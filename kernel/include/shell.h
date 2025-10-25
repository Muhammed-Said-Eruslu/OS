#ifndef SHELL_H
#define SHELL_H

#include "types.h"

#define INPUT_BUFFER_SIZE 128

extern int g_should_exit_terminal;

void execute_command(const char *cmd);
void terminal_run(void);
void sysinfo_print(void);
void fssc_run(const char *filename);

#endif
