#include "vm.h"

#include "hn_type.h"
#include "op.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
void vm_init(vm *restrict v) {
  v->pc = VM_PC_START_ADDRESS;
  v->sp = STACK_SIZE - VM_STACK_TOP_OFFSET;
  v->running = true;
  for (u8 i = 0; i < REG_COUNT; i++) {
    v->reg[i] = 0;
  }
  memset(v->program_mem, 0, MEMORY_SIZE);
  memset(v->data_mem, 0, DATA_MEM_SIZE);
  memset(v->stack, 0, STACK_SIZE);
}

bool vm_load_program(vm *restrict v, const char *restrict filename) {
  FILE *fp = fopen(filename, "rb");
  if (fp == NULL) {
    fprintf(stderr, "Failed to open file: %s.\n", filename);
    return false;
  }
  size_t read_bytes = fread(v->program_mem, 1, MEMORY_SIZE, fp);
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
  while (vm->running == true) {
    u8 op = vm->program_mem[vm->pc + INSTRUCTION_OP];
    u8 src1 = vm->program_mem[vm->pc + INSTRUCTION_SRC1];
    u8 src2 = vm->program_mem[vm->pc + INSTRUCTION_SRC2];
    u8 dest = vm->program_mem[vm->pc + INSTRUCTION_DST];

    vm->pc += INSTRUCTION_SIZE;
    if (vm->pc >= MEMORY_SIZE) {
      fprintf(stderr, "Program end.\n");
      vm->running = false;
      return;
    }

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
    case SET:
      vm->reg[dest] = src1;
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
    case PUSH:
      if (vm->sp == 0) {
        fprintf(stderr, "stack overflow!\n");
        vm->running = false;
        break;
      }
      vm->stack[vm->sp] = vm->reg[src1];
      vm->sp--;
      break;
    case POP:
      if (vm->sp == STACK_SIZE - VM_STACK_TOP_OFFSET) {
        fprintf(stderr, "stack underflow!\n");
        vm->running = false;
        break;
      }
      vm->sp++;
      vm->reg[dest] = vm->stack[vm->sp];
      break;
    case READ:
      u8 in_type = src1;
      u8 in_offset = src2;
      switch (in_type) {
      case IO_MEM:
        vm->reg[dest] = vm->data_mem[in_offset % DATA_MEM_SIZE];
        break;
      case IO_DISK: {
        FILE *fp = fopen(DISK_FILE_PATH, "rb");
        if (fp) {
          u8 off = in_offset % DISK_MAX_OFFSET;
          fseek(fp, off, SEEK_SET);
          fread(&vm->reg[dest], 1, 1, fp);
          fclose(fp);
        }
        break;
      }
      case IO_STDIN:
        if (in_offset == 0) {
          vm->reg[dest] = getchar();
        }
        if (in_offset == 1) {
          vm->data_mem[dest % DATA_MEM_SIZE] = getchar();
        } else {
          vm->running = false;
          fprintf(stderr, "Unknown offset: 0x%02x.\n", in_offset);
        }
        break;
      case IO_EXT:
        break;
      }
      break;
    case WRITE:
      u8 out_type = src1;
      u8 out_offset = src2;
      switch (out_type) {
      case IO_MEM:
        vm->data_mem[out_offset % DATA_MEM_SIZE] = vm->reg[dest];
        break;
      case IO_DISK: {
        FILE *fp = fopen(DISK_FILE_PATH, "r+b");
        if (!fp)
          fp = fopen(DISK_FILE_PATH, "wb");
        if (fp) {
          u8 off = out_offset % DISK_MAX_OFFSET;
          fseek(fp, off, SEEK_SET);
          u8 dat = vm->reg[dest];
          fwrite(&dat, 1, 1, fp);
          fclose(fp);
        }
        break;
      }
      case IO_STDOUT:
        if (out_offset == 0) {
          fprintf(stdout, "%c", vm->reg[dest]);
        }
        if (out_offset == 1) {
          fprintf(stderr, "%d\n", vm->reg[dest]);
        }
        if (out_offset == 2) {
          vm_print_registers(vm);
        }
        if (out_offset == 3) {
          vm_print_memory(vm);
        }
        if (out_offset == 4) {
          vm_print_stack(vm);
        } else {
          vm->running = false;
          fprintf(stderr, "Unknown offset: 0x%02x.\n", out_offset);
        }
        break;
      case IO_EXT:
        break;
      }
      break;
    case CALL:
      if (vm->sp == 0) {
        fprintf(stderr, "function's stack overflow!\n");
        vm->running = false;
        break;
      }
      vm->stack[vm->sp] = vm->pc;
      vm->sp--;
      vm->pc = dest;
      break;
    case RET:
      if (vm->sp >= STACK_SIZE - VM_STACK_TOP_OFFSET) {
        fprintf(stderr, "function,s stack underflow!\n");
        vm->running = false;
        break;
      }
      vm->sp++;
      vm->pc = vm->stack[vm->sp];
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
    fprintf(stdout, "R%d: %u\n", i, v->reg[i]);
  }
  fprintf(stdout, "The PC: %u\n", v->pc);
  fprintf(stdout, "The SP: %u\n", v->sp);
}

void vm_print_memory(vm *restrict v) {
  printf("\n===== Memory =====\n");
  for (u8 i = 0; i < DATA_MEM_SIZE; i++) {
    if (i % 16 == 0)
      printf("\n%3u: ", i);
    printf("%3u ", v->data_mem[i]);
  }
  printf("\n==================\n");
}

void vm_print_stack(vm *restrict v) {
  printf("\n===== Stack =====\n");
  for (u8 i = 0; i < STACK_SIZE; i++) {
    if (i % 16 == 0)
      printf("\n%3u: ", i);
    if (i == v->sp)
      printf("[%3u] ", v->stack[i]);
    else
      printf("%3u ", v->stack[i]);
  }
  printf("\n=================\n");
}