#ifndef FILES_H
#define FILES_H

#include <dirent.h>
#include <stddef.h>
int dir_exists(const char* path);

int dir_is_empty(const char* path);

int is_target(const char* path);
void mkdir_rodzic(const char* path);
int build_trgt(char* out, size_t out_size, const char* src_root, const char* trgt_root, const char* src_path);

#endif
