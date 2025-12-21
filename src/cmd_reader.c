#include "cmd_reader.h"
#include <stdio.h>
#include <string.h>

//bierzemy ptr zeby go przesuwac, pomijamy spacje/taby, zwracamy poczatek tokena, przesuwamy *ptr za token
static char* token(char** ptr)
{
    //skipujemy spacje
    while (**ptr == ' ' || **ptr == '\t') (*ptr)++;

    if (**ptr == '\0') return NULL;
    char* start = *ptr;

    //idziemy do konca tokena
    while (**ptr != '\0' && **ptr != ' ' && **ptr != '\t') (*ptr)++;

    if (**ptr != '\0')
    {
        **ptr = '\0';
        (*ptr)++;
    }
    return start;


}
//przyjmujemy cmd tylko do odczytu
int cmd_handle(const char* cmd, cmd_t* result)
{
    result->type = CMD_ERROR;
    result->src[0]='\0';
    result->target_count = 0;

    //kopiujemy do bufora
    char buf[MAX_CMD_LEN];
    strncpy(buf, cmd, MAX_CMD_LEN-1);
    buf[MAX_CMD_LEN-1]='\0';

    char* ptr = buf;

    char* word = token(&ptr);
    if (word==NULL)
    {
        result->type = CMD_EMPTY;
        return 0;
    }

    if (strcmp(word, "exit")==0 || strcmp(word, "quit")==0)
    {
        result->type= CMD_EXIT;
        return 0;

    }

    if (strcmp(word, "add")==0)
    {
        char* source = token(&ptr);
        if (source==NULL)
        {
            result->type = CMD_ERROR; //brak src
            return 0;
        }

        strncpy(result->src, source, MAX_CMD_LEN-1);
        result->src[MAX_CMD_LEN-1] = '\0';

        int trgt_count =0;
        while (trgt_count<MAX_ARGS)
        {
            char* target = token(&ptr);
            if (target == NULL)
            {
                break;
            }
            strncpy(result->trgt[trgt_count], target, MAX_CMD_LEN-1);
            result->trgt[trgt_count][MAX_CMD_LEN-1] = '\0';
            trgt_count++;
        }

        result->target_count = trgt_count;

        if (result->target_count == 0)
        {
            result->type = CMD_ERROR;
            return 0;
        }
        result->type= CMD_ADD;
        return 0;

    }
    result->type = CMD_ERROR;
    return 0;
}