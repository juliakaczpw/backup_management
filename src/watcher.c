#define _XOPEN_SOURCE 700
#include "watcher.h"
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <unistd.h>
#include "copy.h"
#include "dirs.h"

#define MAX_WATCHES 8192
#define EVENT_BUF_LEN (64 * (sizeof(struct inotify_event) + NAME_MAX + 1))

// robione na podstawie watch_tree.c z plikow z lecture

struct Watch
{
    int wd;
    char* path;
};
struct WatchMap
{
    struct Watch watch_map[MAX_WATCHES];
    int watch_count;
};

static void add_to_map(struct WatchMap* map, int wd, const char* path)
{
    if (map->watch_count >= MAX_WATCHES)
        return;
    map->watch_map[map->watch_count].wd = wd;
    map->watch_map[map->watch_count].path = strdup(path);
    map->watch_count++;
}

static struct Watch* find_watch(struct WatchMap* map, int wd)
{
    for (int i = 0; i < map->watch_count; i++)
        if (map->watch_map[i].wd == wd)
            return &map->watch_map[i];
    return NULL;
}

static void remove_from_map(struct WatchMap* map, int wd)
{
    for (int i = 0; i < map->watch_count; i++)
    {
        if (map->watch_map[i].wd == wd)
        {
            free(map->watch_map[i].path);
            map->watch_map[i] = map->watch_map[map->watch_count - 1];
            map->watch_count--;
            return;
        }
    }
}

static void remove_recursive(const char* path)
{
    struct stat st;
    if (lstat(path, &st) < 0)
        return;
    if (S_ISDIR(st.st_mode))
    {
        DIR* d = opendir(path);
        if (d)
        {
            struct dirent* p;
            while ((p = readdir(d)))
            {
                if (strcmp(p->d_name, ".") == 0 || strcmp(p->d_name, "..") == 0)
                    continue;
                char sub[PATH_MAX];
                snprintf(sub, sizeof(sub), "%s/%s", path, p->d_name);
                remove_recursive(sub);
            }
            closedir(d);
        }
        rmdir(path);
    }
    else
    {
        unlink(path);
    }
}

static void add_watch_recursive(int notify_fd, struct WatchMap* map, const char* base_path)
{
    uint32_t mask = IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO | IN_DELETE_SELF;
    int wd = inotify_add_watch(notify_fd, base_path, mask);
    if (wd < 0)
        return;
    add_to_map(map, wd, base_path);
    DIR* dir = opendir(base_path);
    if (!dir)
        return;
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s", base_path, entry->d_name);
        struct stat st;
        if (lstat(full_path, &st) == 0 && S_ISDIR(st.st_mode))
            add_watch_recursive(notify_fd, map, full_path);
    }
    closedir(dir);
}

void start_watcher(const char* source_path, const char* target_root)
{
    char abs_source[PATH_MAX];
    if (realpath(source_path, abs_source) == NULL)
        exit(1);

    int last_slash = -1;
    for (int k = 0; abs_source[k] != '\0'; k++)
        if (abs_source[k] == '/')
            last_slash = k;
    char* base_name = abs_source + (last_slash + 1);

    // initial syncing
    char initial_dest[PATH_MAX];
    snprintf(initial_dest, sizeof(initial_dest), "%s/%s", target_root, base_name);
    copy_tree(abs_source, initial_dest, abs_source, target_root);

    // monitoring
    int notify_fd = inotify_init();
    if (notify_fd < 0)
        exit(1);
    struct WatchMap map = {0};
    add_watch_recursive(notify_fd, &map, abs_source);

    char buffer[EVENT_BUF_LEN];
    while (1)
    {
        ssize_t len = read(notify_fd, buffer, EVENT_BUF_LEN);
        if (len < 0)
            break;
        ssize_t i = 0;
        while (i < len)
        {
            struct inotify_event* event = (struct inotify_event*)&buffer[i];
            struct Watch* watch = find_watch(&map, event->wd);
            if (watch)
            {
                // jeslis rc deleted to koniec
                if ((event->mask & IN_DELETE_SELF) && strcmp(watch->path, abs_source) == 0)
                {
                    close(notify_fd);
                    exit(0);
                }

                if (event->len > 0)
                {
                    char s_path[PATH_MAX], t_path[PATH_MAX];
                    snprintf(s_path, sizeof(s_path), "%s/%s", watch->path, event->name);
                    const char* relative = s_path + strlen(abs_source);
                    snprintf(t_path, sizeof(t_path), "%s/%s%s", target_root, base_name, relative);

                    if (event->mask & (IN_CREATE | IN_MOVED_TO))
                    {
                        copy_tree(s_path, t_path, abs_source, target_root);
                        struct stat st;
                        if (lstat(s_path, &st) == 0 && S_ISDIR(st.st_mode))
                            add_watch_recursive(notify_fd, &map, s_path);
                    }
                    else if (event->mask & IN_MODIFY)
                    {
                        struct stat st;
                        if (lstat(s_path, &st) == 0 && S_ISREG(st.st_mode))
                            copy_file(s_path, t_path);
                    }
                    else if (event->mask & (IN_DELETE | IN_MOVED_FROM))
                    {
                        remove_recursive(t_path);
                    }
                }
            }
            if (event->mask & IN_IGNORED)
                remove_from_map(&map, event->wd);
            i += sizeof(struct inotify_event) + event->len;
        }
    }
    close(notify_fd);
    exit(0);
}
