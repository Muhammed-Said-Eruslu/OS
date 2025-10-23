#include "kernel.h"
#include "memory/string_builtin.h"

#define strcpy  my_strcpy

static char current_path[128] = "/";

void fs_set_current_path(const char *path)
{
    my_strcpy(current_path, path);
}

char* fs_get_current_path()
{
    return current_path;
}
