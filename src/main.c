#define _XOPEN_SOURCE 700
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "cmd_reader.h"
#include "utils_for_main.h"

volatile sig_atomic_t last_signal = 0;

static void sig_handler(int sig) { last_signal = sig; }

static void sethandler(void (*f)(int), int signum)
{
    struct sigaction act = {0};
    act.sa_handler = f;
    if (-1 == sigaction(signum, &act, NULL))
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}

int main(void)
{
    sethandler(sig_handler, SIGINT);
    sethandler(sig_handler, SIGTERM);

    init_utils();

    char line[4096];
    printf("Available commands:\n");
    printf("\tadd <source> <target1> [target2 ...]\n");
    printf("\tend <source> <target1> [target2 ...]\n");
    printf("\tlist |exit | quit\n");

    while (1)
    {
        if (last_signal != 0)
            break;
        printf("backup> ");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin))
        {
            if (last_signal != 0)
                break;
            if (ferror(stdin))
                perror("fgets");
            break;
        }

        size_t n = strlen(line);
        if (n > 0 && line[n - 1] == '\n')
            line[n - 1] = '\0';

        cmd_t cmd;
        cmd_handle(line, &cmd);

        if (cmd.type == CMD_EMPTY)
            continue;
        if (cmd.type == CMD_EXIT)
            break;
        if (cmd.type == CMD_ERROR)
        {
            fprintf(stderr, "Bad syntax.\n");
            continue;
        }

        if (cmd.type == CMD_ADD)
            start_all_backups(&cmd);
        else if (cmd.type == CMD_LIST)
            list_active_backups();
        else if (cmd.type == CMD_END)
        {
            if (cmd.src[0] == '\0')
            {
                stop_all_backups();
            }
            else
            {
                stop_specific_backups(&cmd);
            }
        }
    }

    printf("\nExit and clean up.\n");
    cleanup_at_exit();
    return 0;
}
