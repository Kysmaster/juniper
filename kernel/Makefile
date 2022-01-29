SHELL = /bin/bash

PATH := $(shell pwd)/host/cross-root/bin:$(PATH)

# Globals and files to compile.
KERNEL    := kernel
KERNELELF := $(KERNEL).elf
KERNELHDD := $(KERNEL).hdd

CFILES    := $(wildcard **/**/**/*.c) $(wildcard **/**/*.c) $(wildcard **/*.c) $(wildcard *.c)
ASMFILES  := $(wildcard **/**/**/*.S) $(wildcard **/**/*.S) $(wildcard **/*.S) $(wildcard *.S)
OBJ       := $(CFILES:.c=.o) $(ASMFILES:.S=.o)
LIBS	  := 

PREFIX = $(shell pwd)

CC      = aarch64-elf-gcc

CFLAGS    = -O2 -std=gnu11 -Werror-implicit-function-declaration -Wstrict-prototypes -Wwrite-strings -fno-omit-frame-pointer -I../include \
			-lgcc -mno-outline-atomics -Wall
ASFLAGS   = -DASSEMBLY -I. -I../include
LDFLAGS   = -T link.ld -z max-page-size=4096 -nostdlib -nostartfiles -lgcc -mno-outline-atomics 

all: $(KERNELELF)

$(KERNELELF): $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ) $(LIBS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	$(CC) $< $(ASFLAGS) -c -o $@

clean:
	rm -f $(OBJ) $(BINS) $(KERNELELF) $(KERNELHDD)
