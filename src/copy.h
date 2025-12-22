#ifndef COPY_H
#define COPY_H

int copy_file(const char* src, const char* trgt);
int copy_dir(const char* src, const char* trgt, const char* src_root, const char* trgt_root);
int copy_symlink(const char* src, const char* trgt, const char* src_root, const char* trgt_root);
int copy_tree(const char* src, const char* trgt, const char* src_root, const char* trgt_root);

#endif
