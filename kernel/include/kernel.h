#ifndef KERNEL_H
#define KERNEL_H

#include "types.h"
#include "drivers.h"
#include "interrupts.h"
#include "memory.h"
#include "fs.h"
#include "shell.h"

extern int cursor_row;
extern int cursor_col;

void kernel_main(void);

#endif
