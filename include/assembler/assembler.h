#pragma once

#include "hn_type.h"
#include "op.h"

#ifndef REGISTER_MAX
#define REGISTER_MAX 15
#endif

#ifndef ASSEMBLE_MAX_SIZE
#define ASSEMBLE_MAX_SIZE 256
#endif

#ifndef INSTRUCTION_SIZE 4
#define INSTRUCTION_SIZE 4
#endif

typedef u8 InstrBuffer[4];

extern op_t get_opcode(const char *restrict name);

extern u8 parse_operand(const char *restrict str);

void assemble(const char *restrict asm_file_path,
              const char *restrict bin_file_path);