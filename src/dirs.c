#include "dirs.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>


#define PATH_MAX 1024

int dir_exists(const char* path)
{
    struct stat statbuf;  //uzywamy stata bo chcemy sie dowiedziec czy to plik czy nie
    if (stat(path, &statbuf) == -1)//error
    {
        if (errno == ENOENT) //not such a file or dir
        {
            return 0; //ni ma
        }else
        {
            return -1; // w razie wykrzaczy sie gdzies indziej
        }
    }else
    {
        if (S_ISDIR(statbuf.st_mode))
        {
            //istnieje
            return 1;
        }else
        {
            errno=ENOTDIR; //exists but not a dir
            return -1;
        }

    }

}
//sprawdzic ukonczenie i readdir jakie roznice eventually
int dir_is_empty(const char* path)
{
    struct dirent* dp;
    DIR* dir = opendir(path);
    if (dir == NULL)
    {
        return -1;

    }
    while ((dp = readdir(dir)) != NULL)
    {
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
        {
            continue;

        }else
        {
            closedir(dir);
            return 0;//nie pusto
        }

    }
    closedir(dir);
    return 1;//psuto


}



int is_target(const char* trgtpath) {
    int exists = dir_exists(trgtpath);

    if (exists == 0) {
        //niesitnieje robimy
        if (mkdir(trgtpath, 0777) == 0) return 0;
        return -1;
    }

    if (exists == 1) {
        //istnieje - pusty
        int empty = dir_is_empty(trgtpath);
        if (empty == 1) return 0;
        if (empty == 0) {
            errno = ENOTEMPTY;
            return -1;
        }
    }

    return -1;
}

