#!/bin/bash

#Compiling the main C code in a position-independant manner
aarch64-linux-gnu-gcc-4.8 -fPIC -O0 -c main.c -o main.o
if [ $? -ne 0 ]; then
  echo "Failed to compile C shellcode"
  exit 1
fi

#Assembling the entry stub and converting it to a binary blob
aarch64-linux-gnu-as entry.S -o entry.elf
if [ $? -ne 0 ]; then
  echo "Failed to assemble complete assembly file"
  exit 1
fi
aarch64-linux-gnu-objcopy -O binary entry.elf entry.bin
if [ $? -ne 0 ]; then
  echo "Failed to copy sections out of ELF file"
  exit 1
fi

#Using our special linker script to make sure the main function is at the beginning of the shellcode file
aarch64-linux-gnu-ld -T ld_script main.o -Map=map.txt -o main.elf
aarch64-linux-gnu-objcopy -O binary main.elf main.bin

#Concatenating the two binary blobs to form our shellcode
cat entry.bin > shellcode.bin
cat main.bin >> shellcode.bin

#Dumping the result
aarch64-linux-gnu-objdump -D -b binary -maarch64 shellcode.bin
