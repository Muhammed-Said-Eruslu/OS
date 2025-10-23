#ifndef FS_H
#define FS_H

#include "types.h"

void fs_init(void);
void fs_list_files(void);
void fs_mkdir(const char *name);
void fs_cd(const char *name);
void fs_pwd(void);
void fs_write_file(const char *name, const char *text);
char *fs_read_file(const char *filename);
void fs_rm(const char *name);
void fs_mv(const char *oldname, const char *newname);
void fs_save(void);
void fs_load(void);
void fs_set_current_path(const char *path);
char *fs_get_current_path(void);
void fs_touch(const char *filename);

#endif
