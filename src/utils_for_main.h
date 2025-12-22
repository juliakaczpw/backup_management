#ifndef UTILS_FOR_MAIN_H
#define UTILS_FOR_MAIN_H

#include "cmd_reader.h"
#include <sys/types.h>

//struct do procesow
typedef struct {
    pid_t pid;
    char src[MAX_CMD_LEN];
    char trgt[MAX_CMD_LEN];
    int active;
} backup_task_t;

//procesy
void init_utils();
void start_all_backups(cmd_t *cmd);
void list_active_backups();
void stop_specific_backups(cmd_t *cmd);
void cleanup_at_exit();
void stop_all_backups();

#endif