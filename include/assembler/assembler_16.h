#pragma once

#include "hn_type.h"
#include "op.h"
#include "vm_16.h"

#ifndef VM_16_REGISTER_MAX
#define VM_16_REGISTER_MAX 15
#endif

#ifndef VM_16_ASSEMBLE_MAX_SIZE
#define VM_16_ASSEMBLE_MAX_SIZE 65536
#endif

#ifndef VM_16_INSTRUCTION_SIZE
#define VM_16_INSTRUCTION_SIZE 4
#endif

#ifndef VM_16_MAX_LABELS
#define VM_16_MAX_LABELS 100
#endif

typedef struct {
  char name[16];
  u16 addr;
} VM_16_Label;

typedef u16 VM_16_InstrBuffer[4];

extern op_t vm_16_get_opcode(const char *restrict name);

extern u16 vm_16_parse_op(const char *restrict str);

extern void vm_16_assemble(const char *restrict asm_file_path,
                           const char *restrict bin_file_path);