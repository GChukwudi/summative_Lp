# Linux Programming Summative

This repository contains tasks from Linux Programming in fulfilment of the summative assessment. Each project demonstrates different aspects of systems programming, from assembly operations to network programming.

## Table of Contents

- [Requirements](#requirements)
- [Question 1: ELF Binary Analysis](#question-1-binary-file-analysis)
- [Question 2: Assembly Program](#question-2-program-in-assembly-language)
- [Question 3: C Extension](#question-3-c-extension)
- [Question 4: Producer - Consumer](#question-4-producer-&-consumer)
- [Question 5: Chat System](#question-5-chat-system)

## Requirements

### Software Dependencies
- Linux operating system
- GCC compiler
- NASM compiler
- POSIX threads library
- strace utility for system call tracing

### System Requirements
- 64-bit Linux Distribution
- Minimum 4GB RAM
- Processor supporting x86-64 architecture

## Question 1: Binary File Analysis

### Objective
Perform comprehensive analysis of an ELF binary using:
- objdump
- strace
- gdb

### Analysis Tools
```bash
# Know basic file information
file magic
# Check binary architecture and characteristics
readelf -h magic
# Display dynamic library dependencies
ldd magic
```
```bash
# Disassemble binary
objdump -d magic
# Detailed disassembly with source code interleaving
objdump -S magic
```
```bash
# To locate function calls
objdump -d magic | grep -E "call"
# For function imports
objdump -T magic
```
```bash
# Track memory accesses
gdb ./magic
```
```bash
# For dynamic analysis
strace ./magic
```

## Question 2: Program in Assembly Language

### Program Description
A program written in x86-64 assembly that reads a file containing names and counts the number of non-empty lines.

### Compilation
```bash
# Assemble the program
nasm -f elf64 count_names.asm -o count_names.o

# Link the object file
ld -o count_names count_names.o
```

### Usage
```bash
./count_names
```
### Features

- Reads a text file line by line
- Ignores empty lines and lines with only whitespace
- Counts and displays the total number of names found

## Question 3: C Extension

### Program Description
A C extension module for Python that performs statistical operations on arrays of floating-point numbers.

### Compilation
```bash
python3 setup.py build_ext --inplace
```

### Usage
```bash
# Run the extension
python3 test_stat.py
```

### Features
- Sum calculation
- Average calculation
- Standard deviation calculation
- Mode calculation
- Count of array elements

## Question 4: Producer & Consumer

### Program Description
A C program that simulates a producer-consumer model for a factory assembly line.

### Compilation
```bash
# Compile with pthread support
gcc -o producer_consumer producer_consumer.c -lpthread
```

### Usage
```bash
# Run producer-consumer program
./producer_consumer
```

### Features
- Producer adds items to a queue with a 2-second delay
- Consumer removes items with a 3-second delay
- Queue has a maximum capacity of 10 items
- Producer pauses when the queue is full
- Consumer only works when items are available

## Question 5: Chat System

### Program Description
A client-server chat application that allows multiple clients to communicate(4 max).

### Server Compilation
```bash
# Compile server
gcc -o server server.c -pthread
```

### Client Compilation
```bash
# Compile client
gcc -o client client.c -pthread
```

### Execution
1. Start the server
```bash
./server
```

2. Launch clients in separate terminals
```bash
./client
```
