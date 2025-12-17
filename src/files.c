#include "files.h"
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>


#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

int dir_exists(const char* path)
{
    struct stat statbuf;
    if (stat(path, &statbuf) == -1)
    {
        if (errno == ENOENT)
        {
            return 0; //ni ma
        }else
        {
            return -1; // w razie wykrzaczy sie gdzies indziej
        }

        if (S_ISDIR(statbuf.st_mode))
            return 1;

    }


}