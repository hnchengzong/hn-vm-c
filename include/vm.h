#pragma once

#include "hn_type.h"
#include <stdbool.h>

#define MEMORY_SIZE 255
#define REG_COUNT 16

// 8-bit VM
#define INSTRUCTION_SIZE 4
#define INSTRUCTION_OP 0
#define INSTRUCTION_SRC1 1
#define INSTRUCTION_SRC2 2
#define INSTRUCTION_DST 3
#define REGISTER_BIT_WIDTH 8  // (op "shl,shr" use)
#define UINT8_VALUE_MASK 0xFF //(op "not" use)

// init
#define VM_PC_START_ADDRESS 0
#define VM_STACK_TOP_OFFSET 1

typedef struct {
  u8 memory[MEMORY_SIZE];
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
