ASM     = nasm
ASFLAGS = -f elf
CC      = gcc
LD      = ld

# ============================================================
# Derleme Ayarları
# ============================================================

INCLUDES = -Ikernel/include \
           -Ikernel/include/memory \
           -Ikernel/fs \
           -Ikernel/drivers \
           -Ikernel/editor \
           -Ikernel/keyboard \
           -Ikernel/interrupts \
           -Ikernel/memory \
           -Ikernel/shell

CFLAGS  = -m32 -ffreestanding -nostdlib -O2 -Wall -Wextra -fno-builtin $(INCLUDES)

# ============================================================
# Kaynak Dosyalar
# ============================================================

SRC_C   := $(shell find kernel -name '*.c')
SRC_ASM := $(shell find kernel -name '*.asm')

OBJ_C   := $(SRC_C:.c=.o)
OBJ_ASM := $(SRC_ASM:.asm=.o)

OBJS    := $(OBJ_C) $(OBJ_ASM)

# ============================================================
# Hedefler
# ============================================================

all: myos.bin

myos.bin: $(OBJS)
	$(LD) -m elf_i386 -T linker.ld -nostdlib -o $@ $(OBJS)

# ============================================================
# Derleme Kuralları
# ============================================================

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	$(ASM) $(ASFLAGS) $< -o $@

# ============================================================
# Temizlik
# ============================================================

clean:
	rm -f $(OBJS) myos.bin

# ============================================================
# QEMU Çalıştırma
# ============================================================

run: myos.bin
	qemu-system-i386 -drive file=fs.img,format=raw,index=0,media=disk -kernel myos.bin -serial stdio
