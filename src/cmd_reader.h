#ifndef CMD_READER_H
#define CMD_READER_H

#define MAX_CMD_LEN 256
#define MAX_ARGS 16

typedef enum
{
    CMD_EMPTY,   // pusta linia
    CMD_EXIT,    // exit / quit
    CMD_ERROR,
    CMD_ADD,
    CMD_LIST,
    CMD_END
} cmd_typ_t;

typedef struct
{
    cmd_typ_t type;
    char src[MAX_CMD_LEN];
    char trgt[MAX_ARGS][MAX_CMD_LEN];
    int target_count;
} cmd_t;

int cmd_handle(const char* cmd, cmd_t* result);

#endif  // CMD_READER_H