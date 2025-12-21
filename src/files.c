#include "files.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>



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
int dir_is_empty(const char* path) //1-pusty , 0-niepusty , -1 - nie istnieje
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
    closedir(dir); //jesli nie znaleznilsimy nic to elo
    return 1;//psuto


}
//tu poprawic usatwic sobie zmienne poprostu zamaist wywolywac funckje po 3 razy
int is_target(const char* trgtpath)
{
    if (dir_exists(trgtpath)==0) //niesitnieje
    {
        if (mkdir(trgtpath, 0777)==0)
        {
            return 0;
        }else
        {
            return -1;
        }

    }else if (dir_exists(trgtpath)==1) //sitnieje
    {

        if (dir_is_empty(trgtpath)==1)
        {
            return 0;
        }else if (dir_is_empty(trgtpath)==0)
        {
            errno=ENOTEMPTY;
            return -1;
        }
        else if (dir_is_empty(trgtpath)==-1)
        {
            return -1;
        }

    }else if (dir_exists(trgtpath)==-1)
    {
        return -1;
    }
    return -1;

}