#include "vm_16.h"

#include "hn_type.h"
#include "op.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

void vm_16_init(vm_16 *restrict v) {
  v->pc = VM_16_PC_START_ADDRESS;
  v->sp = VM_16_STACK_SIZE - VM_16_STACK_TOP_OFFSET;
  v->running = true;
  for (u16 i = 0; i < VM_16_REG_COUNT; i++) {
    v->reg[i] = 0;
  }
  memset(v->program_mem, 0, VM_16_MEMORY_SIZE * sizeof(u16));
  memset(v->data_mem, 0, VM_16_DATA_MEM_SIZE * sizeof(u16));
  memset(v->stack, 0, VM_16_STACK_SIZE * sizeof(u16));
}

bool vm_16_load_program(vm_16 *restrict v, const char *restrict filename) {
  FILE *fp = fopen(filename, "rb");
  if (fp == NULL) {
    fprintf(stderr, "Failed to open file: %s.\n", filename);
    return false;
  }
  size_t read_units = fread(v->program_mem, sizeof(u16), VM_16_MEMORY_SIZE, fp);
  fprintf(stdout, "Loaded %zu bytes from %s.\n", read_units * sizeof(u16),
          filename);
  fclose(fp);
  return true;
}

void vm_16_run(vm_16 *restrict vm) {
  while (vm->running == true) {
    u16 op = vm->program_mem[vm->pc + VM_16_INSTRUCTION_OP];
    u16 src1 = vm->program_mem[vm->pc + VM_16_INSTRUCTION_SRC1];
    u16 src2 = vm->program_mem[vm->pc + VM_16_INSTRUCTION_SRC2];
    u16 dest = vm->program_mem[vm->pc + VM_16_INSTRUCTION_DEST];

    vm->pc += VM_16_INSTRUCTION_SIZE;
    if (vm->pc >= VM_16_MEMORY_SIZE) {
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
        fprintf(stderr, "address: 0x%04x.\n", vm->pc - VM_16_INSTRUCTION_SIZE);
        fprintf(stderr, "Division by zero.\n");
        vm->running = false;
        return;
      }
      vm->reg[dest] = vm->reg[src1] / vm->reg[src2];
      break;
    case MOD:
      if (vm->reg[src2] == 0) {
        fprintf(stderr, "address: 0x%04x.\n", vm->pc - VM_16_INSTRUCTION_SIZE);
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
      vm->reg[dest] = ~vm->reg[src1] & VM_16_UINT16_VALUE_MASK;
      break;
    case SHL:
      vm->reg[dest] = vm->reg[src1]
                      << (vm->reg[src2] % VM_16_REGISTER_BIT_WIDTH);
      break;
    case SHR:
      vm->reg[dest] =
          vm->reg[src1] >> (vm->reg[src2] % VM_16_REGISTER_BIT_WIDTH);
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
      if (vm->sp == VM_16_STACK_SIZE - VM_16_STACK_TOP_OFFSET) {
        fprintf(stderr, "stack underflow!\n");
        vm->running = false;
        break;
      }
      vm->sp++;
      vm->reg[dest] = vm->stack[vm->sp];
      break;
    case READ:
      u16 in_type = src1;
      u16 in_offset = src2;
      switch (in_type) {
      case VM_16_IO_MEM:
        vm->reg[dest] = vm->data_mem[in_offset % VM_16_DATA_MEM_SIZE];
        break;
      case VM_16_IO_DISK: {
        char disk_name[64] = {0};
        printf("input disk name (empty = default.img): ");
        fgets(disk_name, 64, stdin);

        for (int i = 0; disk_name[i]; i++) {
          if (disk_name[i] == ' ' || disk_name[i] == '\n')
            disk_name[i] = 0;
        }

        if (strlen(disk_name) == 0) {
          strcpy(disk_name, VM_16_DEFAULT_DISK);
        }

        FILE *fp = fopen(disk_name, "rb");
        if (!fp) {
          fprintf(stderr, "disk open failed\n");
          vm->running = false;
          break;
        }

        u16 off = in_offset % VM_16_DISK_MAX_OFFSET;
        fseek(fp, off * sizeof(u16), SEEK_SET);
        fread(&vm->reg[dest], sizeof(u16), 1, fp);
        fclose(fp);
        break;
      }
      case VM_16_IO_STDIN:
        if (in_offset == 0) {
          vm->reg[dest] = getchar();
        } else if (in_offset == 1) {
          vm->data_mem[dest % VM_16_DATA_MEM_SIZE] = getchar();
        } else {
          vm->running = false;
          fprintf(stderr, "Unknown offset: 0x%04x.\n", in_offset);
        }
        break;
      case VM_16_IO_EXT:
        break;
      }
      break;
    case WRITE:
      u16 out_type = src1;
      u16 out_offset = src2;
      switch (out_type) {
      case VM_16_IO_MEM:
        vm->data_mem[out_offset % VM_16_DATA_MEM_SIZE] = vm->reg[dest];
        break;
      case VM_16_IO_DISK: {
        char disk_name[64] = {0};
        printf("input disk name (empty = default.img): ");
        fgets(disk_name, 64, stdin);

        for (int i = 0; disk_name[i]; i++) {
          if (disk_name[i] == ' ' || disk_name[i] == '\n')
            disk_name[i] = 0;
        }

        if (strlen(disk_name) == 0) {
          strcpy(disk_name, VM_16_DEFAULT_DISK);
        }

        FILE *fp = fopen(disk_name, "r+b");
        if (!fp)
          fp = fopen(disk_name, "wb");
        if (!fp) {
          fprintf(stderr, "disk create failed\n");
          vm->running = false;
          break;
        }

        u16 off = out_offset % VM_16_DISK_MAX_OFFSET;
        fseek(fp, off * sizeof(u16), SEEK_SET);
        u16 dat = vm->reg[dest];
        fwrite(&dat, sizeof(u16), 1, fp);
        fclose(fp);
        break;
      }

      case VM_16_IO_STDOUT:
        if (out_offset == 0) {
          fprintf(stdout, "%c", vm->reg[dest]);
        } else if (out_offset == 1) {
          fprintf(stderr, "%d\n", vm->reg[dest]);
        } else if (out_offset == 2) {
          vm_16_print_registers(vm);
        } else if (out_offset == 3) {
          vm_16_print_memory(vm);
        } else if (out_offset == 4) {
          vm_16_print_stack(vm);
        } else {
          vm->running = false;
          fprintf(stderr, "Unknown offset: 0x%04x.\n", out_offset);
        }
        break;
      case VM_16_IO_EXT:
        break;
      }
      break;
    case CALL:
      if (vm->sp == 0) {
        fprintf(stderr, "function's stack overflow!\n");
        vm->running = false;
        break;
      }
      vm->stack[vm->sp] = vm->pc + VM_16_INSTRUCTION_SIZE;
      vm->sp--;
      vm->pc = dest;
      break;
    case RET:
      if (vm->sp >= VM_16_STACK_SIZE - VM_16_STACK_TOP_OFFSET) {
        fprintf(stderr, "function's stack underflow!\n");
        vm->running = false;
        break;
      }
      vm->sp++;
      vm->pc = vm->stack[vm->sp];
      break;

    case HALT:
      fprintf(stdout, "final address: 0x%04x.\n",
              vm->pc - VM_16_INSTRUCTION_SIZE);
      fprintf(stdout, "Program halted.\n");
      vm->running = false;
      break;
    case END:
      fprintf(stdout, "final address: 0x%04x.\n",
              vm->pc - VM_16_INSTRUCTION_SIZE);
      fprintf(stdout, "Program halted.\n");
      vm->running = false;
      return;
    default:
      fprintf(stderr, "address: 0x%04x.\n", vm->pc - VM_16_INSTRUCTION_SIZE);
      fprintf(stderr, "Unknown opcode: 0x%04x.\n", op);
      vm->running = false;
    }
  }
}

void vm_16_print_registers(vm_16 *restrict v) {
  for (u16 i = 0; i < VM_16_REG_COUNT; i++) {
    fprintf(stdout, "R%d: %u\n", i, v->reg[i]);
  }
  fprintf(stdout, "The PC: %u\n", v->pc);
  fprintf(stdout, "The SP: %u\n", v->sp);
}

void vm_16_print_memory(vm_16 *restrict v) {
  printf("\n===== Memory =====\n");
  for (int i = 0; i < VM_16_DATA_MEM_SIZE; i++) {
    if (i % 16 == 0)
      printf("\n%3u: ", i);
    printf("%5u ", v->data_mem[i]);
  }
  printf("\n==================\n");
}

void vm_16_print_stack(vm_16 *restrict v) {
  printf("\n===== Stack =====\n");
  for (int i = 0; i < VM_16_STACK_SIZE; i++) {
    if (i % 16 == 0)
      printf("\n%3u: ", i);
    if (i == v->sp)
      printf("[%5u] ", v->stack[i]);
    else
      printf("%5u ", v->stack[i]);
  }
  printf("\n=================\n");
}