SHELL = /bin/bash

PATH := $(shell pwd)/host/cross-root/bin:$(PATH)

# Globals and files to compile.
KERNEL    := kernel
KERNELELF := $(KERNEL).elf
KERNELHDD := $(KERNEL).hdd

CFILES    := $(shell find $(SOURCEDIR) -type f -name '*.c')
ASMFILES  := $(shell find $(SOURCEDIR) -type f -name '*.S')
OBJ       := $(CFILES:.c=.o) $(ASMFILES:.S=.o)

PREFIX = $(shell pwd)

CC      = aarch64-juniper-gcc

CFLAGS    = -O2 -std=gnu11 -Werror-implicit-function-declaration -Wstrict-prototypes -Wwrite-strings -fno-omit-frame-pointer -I./include \
			-include "config.h" -lgcc -mno-outline-atomics
ASFLAGS   = -DASSEMBLY -I. -I./include -include "config.h"
LDFLAGS   = -T link.ld -z max-page-size=4096 -nostdlib -nostartfiles -lgcc -mno-outline-atomics

all: $(KERNELELF)

$(KERNELELF): $(OBJ) font_sfn.o
	$(CC) $(LDFLAGS) $(OBJ) font_sfn.o -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	$(CC) $< $(ASFLAGS)  -c -o $@

font_sfn.o: font.sfn
	aarch64-juniper-ld -r -b binary -o font_sfn.o font.sfn

clean:
	rm -f $(OBJ) $(BINS) $(KERNELELF) $(KERNELHDD)
