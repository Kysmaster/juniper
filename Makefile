SHELL = /bin/bash

PATH := $(shell pwd)/host/cross-root/bin:$(PATH)
PREFIX = $(shell pwd)/root

KERNEL    := kernel
KERNELELF := $(KERNEL).elf

QEMU    = qemu-system-aarch64

QEMUFLAGS = -cpu cortex-a53 -machine raspi3 -kernel kernel/$(KERNELELF) -smp 4 -m 256M -serial null -serial stdio

all: 
	#@# Build and install libc
	#cp -r ./libc/include/* ./root/usr/include/

	@# Build and install kernel
	$(MAKE) -C kernel



clean:
	$(MAKE) -C kernel clean
	rm -rf ./root/usr/include/*
	
run: all
	$(QEMU) $(QEMUFLAGS)
