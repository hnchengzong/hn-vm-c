#pragma once

#include "hn_type.h"
#include <stdbool.h>

#define MEMORY_SIZE 256
#define DATA_MEM_SIZE 256
#define REG_COUNT 16
#define STACK_SIZE 256

// 8-bit VM
#define INSTRUCTION_SIZE 4
#define INSTRUCTION_OP 0
#define INSTRUCTION_SRC1 1
#define INSTRUCTION_SRC2 2
#define INSTRUCTION_DEST 3
#define REGISTER_BIT_WIDTH 8  // (op "shl,shr" use)
#define UINT8_VALUE_MASK 0xFF //(op "not" use)

// init
#define VM_PC_START_ADDRESS 0
#define VM_STACK_TOP_OFFSET 1

// I/O
#define IO_MEM 0
#define IO_DISK 1
#define IO_STDIN 2
#define IO_STDOUT 3
#define IO_EXT 4

// disk
#define DEFAULT_DISK "./data/disk0.bin"
#define DISK_MAX_OFFSET 255

typedef struct {
  u8 program_mem[MEMORY_SIZE];
  u8 data_mem[DATA_MEM_SIZE];
  u8 stack[STACK_SIZE];
  u8 reg[REG_COUNT];
  u8 pc;
  u8 sp;
  bool running;
} vm;

extern void vm_init(vm *restrict v);
extern void vm_run(vm *restrict v);
extern void vm_load_test_program(vm *restrict v);
extern bool vm_load_program(vm *restrict v, const char *restrict filename);
extern void vm_print_registers(vm *restrict v);
extern void vm_print_memory(vm *restrict v);
extern void vm_print_stack(vm *restrict v);
