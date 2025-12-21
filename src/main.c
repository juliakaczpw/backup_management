#define _XOPEN_SOURCE 700
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "cmd_reader.h"
#include "copy.h"
#include "files.h"

int main(void)
{
    char line[4096];

    printf("Available commands:\n");
    printf("\tadd <source> <target1> [target2 ...]\n");
    printf("\texit | quit\n");

    while (1)
    {
        printf("backup> ");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin))
        {

            if (ferror(stdin))
                perror("fgets");
            break;
        }

        // ucinamy '\n'
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
            fprintf(stderr, "Bad command. Use: add <src> <target...> or exit\n");
            continue;
        }

        if (cmd.type == CMD_ADD)
        {

            for (int i = 0; i < cmd.target_count; i++)
            {
                const char *src = cmd.src;
                const char *trgt_root = cmd.trgt[i];


                if (is_target(trgt_root) < 0)
                {
                    fprintf(stderr, "Target invalid: %s: ", trgt_root);
                    perror("is_target");
                    continue;
                }


                const char *base = strrchr(src, '/');
                base = (base ? base + 1 : src);

                if (base[0] == '\0')
                {
                    fprintf(stderr, "Bad source path (ends with '/'): %s\n", src);
                    continue;
                }

                char dest[1024];
                if (snprintf(dest, sizeof(dest), "%s/%s", trgt_root, base) >= (int)sizeof(dest))
                {
                    fprintf(stderr, "Path too long for dest\n");
                    continue;
                }


                if (copy_tree(src, dest, src, trgt_root) < 0)
                {
                    fprintf(stderr, "Copy failed %s -> %s: ", src, dest);
                }

                printf("OK: %s -> %s\n", src, dest);
            }

            continue;
        }

        fprintf(stderr, "Unknown command type\n");
    }

    printf("Exiting.\n");
    return 0;
}
