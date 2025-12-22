#include "cmd_reader.h"
#include <stdio.h>
#include <string.h>


#define PATH_MAX 1024

//handling spacji
static char* token(char** ptr)
{
    while (**ptr == ' ' || **ptr == '\t') (*ptr)++;
    if (**ptr == '\0') return NULL;

    char* start;
    if (**ptr == '"')
    {
        (*ptr)++;
        start = *ptr;
        while (**ptr != '\0' && **ptr != '"') (*ptr)++;
        if (**ptr == '"')
        {
            **ptr = '\0';
            (*ptr)++;
        }
    }
    else
    {
        start = *ptr;
        while (**ptr != '\0' && **ptr != ' ' && **ptr != '\t') (*ptr)++;
        if (**ptr != '\0')
        {
            **ptr = '\0';
            (*ptr)++;
        }
    }
    return start;
}

int cmd_handle(const char* cmd, cmd_t* result)
{
    result->type = CMD_ERROR;
    result->src[0]='\0';
    result->target_count = 0;

    char buf[MAX_CMD_LEN * 2];
    strncpy(buf, cmd, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

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

    if (strcmp(word, "list") == 0)
    {
        result->type = CMD_LIST;
        return 0;
    }

    if (strcmp(word, "add")==0 || strcmp(word, "end")==0)
    {
        if (strcmp(word, "add") == 0){ result->type = CMD_ADD;}
        else if (strcmp(word, "end") == 0) {result->type = CMD_END;}


        char* source = token(&ptr);
        if (source==NULL) return 0; //brak src

        strncpy(result->src, source, MAX_CMD_LEN-1);
        result->src[MAX_CMD_LEN-1] = '\0';

        int trgt_count = 0;
        while (trgt_count < MAX_ARGS)
        {
            char* target = token(&ptr);
            if (target == NULL) break;
            strncpy(result->trgt[trgt_count], target, MAX_CMD_LEN-1);
            result->trgt[trgt_count][MAX_CMD_LEN-1] = '\0';
            trgt_count++;
        }
        result->target_count = trgt_count;

        // add ma cel, end nie - roznica
        if (result->type == CMD_ADD && result->target_count == 0)
        {
            result->type = CMD_ERROR;
        }
        return 0;
    }
    return 0;
}