#define _XOPEN_SOURCE 700
#include <stdlib.h>   // realpath


#include "utils_for_main.h"
#include "copy.h"
#include "dirs.h"
#include "watcher.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <limits.h>


//#define PATH_MAX 1024

static backup_task_t tasks[64];

void init_utils() {
    for (int i = 0; i < 64; i++) tasks[i].active = 0;
}

static int is_inside(const char *src, const char *trgt) {
    char r_src[PATH_MAX], r_trgt[PATH_MAX];
    if (realpath(src, r_src) == NULL || realpath(trgt, r_trgt) == NULL) return 0;
    size_t len = strlen(r_src);
    if (strncmp(r_src, r_trgt, len) == 0) {
        if (r_trgt[len] == '/' || r_trgt[len] == '\0') return 1;
    }
    return 0;
}

void start_all_backups(cmd_t *cmd) {
    char abs_src[PATH_MAX];
    if (realpath(cmd->src, abs_src) == NULL) { perror("realpath source"); return; }

    for (int i = 0; i < cmd->target_count; i++) {
        if (is_target(cmd->trgt[i]) < 0) {
            fprintf(stderr, "Target invalid: %s\n", cmd->trgt[i]);
            continue;
        }

        char abs_trgt[PATH_MAX];
        if (realpath(cmd->trgt[i], abs_trgt) == NULL) continue;

        if (is_inside(abs_src, abs_trgt)) {
            printf("Error - loop on  %s inside %s\n", abs_trgt, abs_src);
            continue;
        }

        //check duplikatow (też na abs)
        int exists = 0;
        for (int j = 0; j < 64; j++) {
            if (tasks[j].active && strcmp(tasks[j].src, abs_src) == 0 && strcmp(tasks[j].trgt, abs_trgt) == 0) {
                exists = 1; break;
            }
        }
        if (exists) { printf("Already running: %s -> %s\n", abs_src, abs_trgt); continue; }

        int slot = -1;
        for (int j = 0; j < 64; j++) if (!tasks[j].active) { slot = j; break; }
        if (slot == -1) return;

        pid_t pid = fork();
        if (pid == 0) {
            start_watcher(abs_src, abs_trgt);
            exit(0);
        } else if (pid > 0) {
            tasks[slot].pid = pid;
            strncpy(tasks[slot].src, abs_src, MAX_CMD_LEN - 1);
            tasks[slot].src[MAX_CMD_LEN - 1] = '\0';
            strncpy(tasks[slot].trgt, abs_trgt, MAX_CMD_LEN - 1);
            tasks[slot].trgt[MAX_CMD_LEN - 1] = '\0';
            tasks[slot].active = 1;
            printf("Backup started [PID %d]\n", pid);
        }
    }
}

void stop_all_backups()
{
    for (int j = 0; j < 64; j++) {
        if (!tasks[j].active) continue;

        kill(tasks[j].pid, SIGTERM);
        waitpid(tasks[j].pid, NULL, 0);
        printf("Stopped: %s -> %s\n", tasks[j].src, tasks[j].trgt);
        tasks[j].active = 0;
    }
}


void list_active_backups() {
    printf("\n Active Backups\n");
    for (int i = 0; i < 64; i++) {
        if (tasks[i].active) {
            if (waitpid(tasks[i].pid, NULL, WNOHANG) == 0) {
                printf("[%d] %s -> %s\n", tasks[i].pid, tasks[i].src, tasks[i].trgt);
            } else { tasks[i].active = 0; }
        }
    }
}

void stop_specific_backups(cmd_t *cmd) {
    char abs_src[PATH_MAX];
    //src do tablicy
    if (realpath(cmd->src, abs_src) == NULL) strncpy(abs_src, cmd->src, PATH_MAX-1);

    for (int j = 0; j < 64; j++) {
        if (!tasks[j].active) continue;
        if (strcmp(tasks[j].src, abs_src) != 0) continue;

        int should_kill = 0;
        if (cmd->target_count == 0) {
            should_kill = 1; //kill dla tego src
        } else {
            for (int i = 0; i < cmd->target_count; i++) {
                char abs_trgt[PATH_MAX];
                //jesli target nie istnieje -realpath wykrzaczy sie
                if (realpath(cmd->trgt[i], abs_trgt) == NULL) strncpy(abs_trgt, cmd->trgt[i], PATH_MAX-1);
                if (strcmp(tasks[j].trgt, abs_trgt) == 0) { should_kill = 1; break; }
            }
        }

        if (should_kill) {
            kill(tasks[j].pid, SIGTERM);
            waitpid(tasks[j].pid, NULL, 0);
            tasks[j].active = 0;
            printf("Stopped monitoring: %s -> %s\n", tasks[j].src, tasks[j].trgt);
        }
    }
}

void cleanup_at_exit() {
    for (int i = 0; i < 64; i++)
    {
        if (tasks[i].active)
        {
            kill(tasks[i].pid, SIGKILL);
            waitpid(tasks[i].pid, NULL, 0);
        }

    }
}