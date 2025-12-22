#define _XOPEN_SOURCE 700
#include "copy.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
// limits do PATH_MAX

static int starts_with_dir(const char* path, const char* root)
{
    size_t n = strlen(root);
    if (strncmp(path, root, n) != 0)
        return 0;
    return path[n] == '\0' || path[n] == '/';
}

int copy_file(const char* src, const char* trgt)
{
    int src_fd = open(src, O_RDONLY);
    if (src_fd < 0)
        return -1;

    int trgt_fd = open(trgt, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (trgt_fd < 0)
    {
        close(src_fd);
        return -1;
    }

    char buffer[1024];

    while (1)
    {
        ssize_t read_bytes = read(src_fd, buffer, sizeof(buffer));
        if (read_bytes < 0)
        {
            // if (errno == EINTR)
            // {
            //     continue;
            // }
            close(src_fd);
            close(trgt_fd);
            return -1;
        }
        if (read_bytes == 0)
            break;  // eof

        ssize_t written_all = 0;
        while (written_all < read_bytes)
        {
            ssize_t written_bytes = write(trgt_fd, buffer + written_all, read_bytes - written_all);
            if (written_bytes < 0)
            {
                close(trgt_fd);
                close(src_fd);
                return -1;
            }
            written_all += written_bytes;
        }
    }
    if (close(src_fd) < 0)
    {
        close(trgt_fd);
        return -1;
    }
    close(trgt_fd);
    return 0;
}

int copy_dir(const char* src, const char* trgt, const char* src_root, const char* trgt_root)
{
    if (mkdir(trgt, 0777) < 0)
    {
        if (errno != EEXIST)
        {
            return -1;
        }
    }

    DIR* dir = opendir(src);
    if (!dir)
        return -1;

    struct dirent* entry;

    while ((entry = readdir(dir)) != NULL)
    {
        const char* name = entry->d_name;
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
            continue;

        char next_src[PATH_MAX];
        char next_trgt[PATH_MAX];

        snprintf(next_src, PATH_MAX, "%s/%s", src, name);
        snprintf(next_trgt, PATH_MAX, "%s/%s", trgt, name);

        if (copy_tree(next_src, next_trgt, src_root, trgt_root) < 0)
        {
            closedir(dir);
            return -1;
        }
    }
    closedir(dir);
    return 0;
}

int copy_symlink(const char* src, const char* trgt, const char* src_root, const char* trgt_root)
{
    char linkie[PATH_MAX];

    ssize_t len = readlink(src, linkie, PATH_MAX - 1);
    if (len < 0)
        return -1;
    linkie[len] = '\0';  // nie dodaje na koncu

    char final_linkie[PATH_MAX];
    snprintf(final_linkie, PATH_MAX, "%s", linkie);
    // char buffer[PATH_MAX];

    // absolute link przepinamy na trgt
    if (linkie[0] == '/')
    {
        char root[PATH_MAX];
        char linkie_root[PATH_MAX];

        if (realpath(src_root, root) != NULL && realpath(linkie, linkie_root) != NULL)
        {
            if (starts_with_dir(linkie_root, root))
            {
                int root_len = strlen(root);
                char sufix[PATH_MAX];
                snprintf(sufix, PATH_MAX, "%s", linkie_root + root_len);
                snprintf(final_linkie, PATH_MAX, "%s%s", trgt_root, sufix);
            }
        }
    }
    // tu zrobic jeszcze edge case check
    unlink(trgt);
    if (symlink(final_linkie, trgt) < 0)
    {
        return -1;
    }
    return 0;
}

int copy_tree(const char* src, const char* trgt, const char* src_root, const char* trgt_root)
{
    struct stat st;
    if (lstat(src, &st) < 0)
    {
        return -1;
    }
    if (S_ISDIR(st.st_mode))
    {
        return copy_dir(src, trgt, src_root, trgt_root);
    }
    if (S_ISREG(st.st_mode))
    {
        return copy_file(src, trgt);
    }
    if (S_ISLNK(st.st_mode))
    {
        return copy_symlink(src, trgt, src_root, trgt_root);
    }

    errno = ENOTSUP;
    return -1;
}
