#include "vm.h"

#include "hn_type.h"
#include "op.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
void vm_init(vm *restrict v) {
  v->pc = VM_PC_START_ADDRESS;
  v->sp = MEMORY_SIZE - VM_STACK_TOP_OFFSET;
  v->running = true;
  for (u8 i = 0; i < REG_COUNT; i++) {
    v->reg[i] = 0;
  }
  memset(v->memory, 0, MEMORY_SIZE);
}

bool vm_load_program(vm *restrict v, const char *restrict filename) {
  FILE *fp = fopen(filename, "rb");
  if (fp == NULL) {
    fprintf(stderr, "Failed to open file: %s.\n", filename);
    return false;
  }
  size_t read_bytes = fread(v->memory, 1, MEMORY_SIZE, fp);
  if (read_bytes > MEMORY_SIZE) {
    fprintf(stderr, "Failed to read program: %s.It's too large.\n", filename);
    fclose(fp);
    return false;
  }
  fprintf(stdout, "Loaded %zu bytes from %s.\n", read_bytes, filename);
  fclose(fp);
  return true;
}

void vm_run(vm *restrict vm) {
  while (vm->running == true && vm->pc < MEMORY_SIZE) {
    u8 op = vm->memory[vm->pc + INSTRUCTION_OP];
    u8 src1 = vm->memory[vm->pc + INSTRUCTION_SRC1];
    u8 src2 = vm->memory[vm->pc + INSTRUCTION_SRC2];
    u8 dest = vm->memory[vm->pc + INSTRUCTION_DST];

    vm->pc += INSTRUCTION_SIZE;
    switch (op) {
    case NOP:
      break;
    case ADD:
      vm->reg[dest] = vm->reg[src1] + vm->reg[src2];
      break;
    case SUB:
      vm->reg[dest] = vm->reg[src1] - vm->reg[src2];
      break;
    case MUL:
      vm->reg[dest] = vm->reg[src1] * vm->reg[src2];
      break;
    case DIV:
      if (vm->reg[src2] == 0) {
        fprintf(stderr, "address: 0x%02x.\n", vm->pc - INSTRUCTION_SIZE);
        fprintf(stderr, "Division by zero.\n");
        vm->running = false;
        return;
      }
      vm->reg[dest] = vm->reg[src1] / vm->reg[src2];
      break;
    case MOD:
      if (vm->reg[src2] == 0) {
        fprintf(stderr, "address: 0x%02x.\n", vm->pc - INSTRUCTION_SIZE);
        fprintf(stderr, "Modulo by zero.\n");
        vm->running = false;
        return;
      }
      vm->reg[dest] = vm->reg[src1] % vm->reg[src2];
      break;
    case AND:
      vm->reg[dest] = vm->reg[src1] & vm->reg[src2];
      break;
    case OR:
      vm->reg[dest] = vm->reg[src1] | vm->reg[src2];
      break;
    case XOR:
      vm->reg[dest] = vm->reg[src1] ^ vm->reg[src2];
      break;
    case NOT:
      vm->reg[dest] = ~vm->reg[src1] & UINT8_VALUE_MASK;
      ;
      break;
    case SHL:
      vm->reg[dest] = vm->reg[src1] << (vm->reg[src2] % REGISTER_BIT_WIDTH);
      break;
    case SHR:
      vm->reg[dest] = vm->reg[src1] >> (vm->reg[src2] % REGISTER_BIT_WIDTH);
      break;
    case MOV:
      vm->reg[dest] = vm->reg[src1] + src2;
      break;
    case ALWAYS:
      vm->pc = dest;
      break;
    case EQ:
      if (vm->reg[src1] == vm->reg[src2]) {
        vm->pc = dest;
      }
      break;
    case NEQ:
      if (vm->reg[src1] != vm->reg[src2]) {
        vm->pc = dest;
      }
      break;
    case GT:
      if (vm->reg[src1] > vm->reg[src2]) {
        vm->pc = dest;
      }
      break;
    case LT:
      if (vm->reg[src1] < vm->reg[src2]) {
        vm->pc = dest;
      }
      break;
    case LE:
      if (vm->reg[src1] <= vm->reg[src2]) {
        vm->pc = dest;
      }
      break;
    case GE:
      if (vm->reg[src1] >= vm->reg[src2]) {
        vm->pc = dest;
      }
      break;
    case NEVER:
      break;
    case HALT:
      fprintf(stdout, "final address: 0x%02x.\n", vm->pc - INSTRUCTION_SIZE);
      fprintf(stdout, "Program halted.\n");
      vm->running = false;
      return;
    default:
      fprintf(stderr, "address: 0x%02x.\n", vm->pc - INSTRUCTION_SIZE);
      fprintf(stderr, "Unknown opcode: 0x%02x.\n", op);
      vm->running = false;
    }
  }
}

void vm_print_registers(vm *restrict v) {
  for (u8 i = 0; i < REG_COUNT; i++) {
    fprintf(stdout, "R%d: 0x%u\n", i, v->reg[i]);
  }
  fprintf(stdout, "The PC: 0x%u\n", v->pc);
  fprintf(stdout, "The SP: 0x%u\n", v->sp);
}