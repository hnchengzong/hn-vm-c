#pragma once

#include "hn_type.h"
#include <stdbool.h>

#define VM_16_MEMORY_SIZE 65536
#define VM_16_DATA_MEM_SIZE 65536
#define VM_16_REG_COUNT 16
#define VM_16_STACK_SIZE 65536

#define VM_16_INSTRUCTION_SIZE 4
#define VM_16_INSTRUCTION_OP 0
#define VM_16_INSTRUCTION_SRC1 1
#define VM_16_INSTRUCTION_SRC2 2
#define VM_16_INSTRUCTION_DEST 3

#define VM_16_REGISTER_BIT_WIDTH 16
#define VM_16_UINT16_VALUE_MASK 0xFFFF

#define VM_16_PC_START_ADDRESS 0
#define VM_16_STACK_TOP_OFFSET 1

#define VM_16_IO_MEM 0
#define VM_16_IO_DISK 1
#define VM_16_IO_STDIN 2
#define VM_16_IO_STDOUT 3
#define VM_16_IO_EXT 4

#define VM_16_DEFAULT_DISK "./data/disk_16.bin"
#define VM_16_DISK_MAX_OFFSET 65535

typedef struct {
  u16 program_mem[VM_16_MEMORY_SIZE];
  u16 data_mem[VM_16_DATA_MEM_SIZE];
  u16 stack[VM_16_STACK_SIZE];
  u16 reg[VM_16_REG_COUNT];
  u16 pc;
  u16 sp;
  bool running;
} vm_16;

extern void vm_16_init(vm_16 *restrict v);
extern void vm_16_run(vm_16 *restrict v);
extern void vm_16_load_test_program(vm_16 *restrict v);
extern bool vm_16_load_program(vm_16 *restrict v,
                               const char *restrict filename);
extern void vm_16_print_registers(vm_16 *restrict v);
extern void vm_16_print_memory(vm_16 *restrict v);
extern void vm_16_print_stack(vm_16 *restrict v);