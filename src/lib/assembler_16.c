#include "../include/assembler/assembler_16.h"
#include "hn_type.h"
#include "vm_16.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static VM_16_Label vm_16_labels[VM_16_MAX_LABELS];
static int vm_16_label_count = 0;

static bool is_number(const char *restrict str) {
  if (!str || *str == '\0')
    return false;
  for (int i = 0; str[i] != '\0'; i++) {
    if (!isdigit((unsigned char)str[i])) {
      return false;
    }
  }
  return true;
}

static u16 vm_16_find_label(const char *name) {
  for (int i = 0; i < vm_16_label_count; i++) {
    if (strcmp(vm_16_labels[i].name, name) == 0) {
      return vm_16_labels[i].addr;
    }
  }
  if (is_number(name)) {
    return (u16)atoi(name);
  }
  fprintf(stderr, "undefined label: %s\n", name);
  exit(EXIT_FAILURE);
}

op_t vm_16_get_opcode(const char *restrict name) {
  if (is_number(name)) {
    return (op_t)atoi(name);
  }
  for (int i = 0; i < OP_NUM; i++) {
    if (strcmp(name, op_table[i].name) == 0) {
      return op_table[i].code;
    }
  }
  fprintf(stderr, "Invalid opcode: %s\n", name);
  return OP_NUM;
}

u16 vm_16_parse_op(const char *restrict str) {
  if (strcmp(str, "nop") == 0 || strcmp(str, "null") == 0 ||
      strcmp(str, "nil") == 0 || strcmp(str, "none") == 0) {
    return 0;
  }
  if (str[0] == 'r' || str[0] == 'R') {
    u16 reg_num = (u16)atoi(str + 1);
    if (reg_num > VM_16_REGISTER_MAX) {
      fprintf(stderr, "error:register must be r0~r%d,you input:%s\n",
              VM_16_REGISTER_MAX, str);
      exit(EXIT_FAILURE);
    }
    return reg_num;
  }
  return vm_16_find_label(str);
}

void vm_16_assemble(const char *restrict asm_file_path,
                    const char *restrict bin_file_path) {
  VM_16_InstrBuffer mem_instr[VM_16_ASSEMBLE_MAX_SIZE];
  int instr_count = 0;

  FILE *const asm_file = fopen(asm_file_path, "rb");
  FILE *const bin_file = fopen(bin_file_path, "wb");

  if (!asm_file) {
    fprintf(stderr, "Failed to open asm file: %s\n", asm_file_path);
    goto cleanup;
  }
  if (!bin_file) {
    fprintf(stderr, "Failed to create bin file: %s\n", bin_file_path);
    goto cleanup;
  }
  const int seek_end = fseek(asm_file, 0, SEEK_END);
  if (seek_end != 0) {
    fprintf(stderr, "Failed to seek to end of file: %s\n", asm_file_path);
    goto cleanup;
  }
  const long asm_file_size = ftell(asm_file);
  if (asm_file_size < 0) {
    fprintf(stderr, "Failed to get file size: %s\n", asm_file_path);
    goto cleanup;
  }
  const int seek_begin = fseek(asm_file, 0, SEEK_SET);
  if (seek_begin != 0) {
    fprintf(stderr, "Failed to seek to beginning of file: %s\n", asm_file_path);
    goto cleanup;
  }

  char *file_content = malloc(asm_file_size + 1);
  if (!file_content) {
    fprintf(stderr, "Failed to allocate memory for file content\n");
    goto cleanup;
  }
  const size_t bytes_read = fread(file_content, 1, asm_file_size, asm_file);
  if (bytes_read != (size_t)asm_file_size) {
    fprintf(stderr, "Failed to read entire asm file: read %zu of %ld bytes\n",
            bytes_read, asm_file_size);
    goto cleanup;
  }
  file_content[asm_file_size] = '\0';

  char *restrict line = strtok(file_content, "\n");
  int line_num = 0;
  int temp_instr_count = 0;
  vm_16_label_count = 0;

  while (line != NULL) {
    line_num++;
    char *comment_start = strchr(line, ';');
    if (comment_start)
      *comment_start = '\0';

    while (*line && isspace((unsigned char)*line))
      line++;
    if (*line == '\0') {
      line = strtok(NULL, "\n");
      continue;
    }

    char op_str[16], src1[16], src2[16], dest[16];
    if (sscanf(line, "%s %s %s %s", op_str, src1, src2, dest) != 4) {
      fprintf(stderr, "Line %d: invalid line format\n", line_num);
      goto cleanup;
    }

    if (strcmp(op_str, "label") == 0) {
      if (vm_16_label_count >= VM_16_MAX_LABELS) {
        fprintf(stderr, "Line %d: too many labels (max %d)\n", line_num,
                VM_16_MAX_LABELS);
        goto cleanup;
      }
      for (int i = 0; i < vm_16_label_count; i++) {
        if (strcmp(vm_16_labels[i].name, src1) == 0) {
          fprintf(stderr, "Line %d: duplicate label: %s\n", line_num, src1);
          goto cleanup;
        }
      }
      strncpy(vm_16_labels[vm_16_label_count].name, src1, 15);
      vm_16_labels[vm_16_label_count].name[15] = '\0';
      vm_16_labels[vm_16_label_count].addr =
          temp_instr_count * VM_16_INSTRUCTION_SIZE;
      vm_16_label_count++;
      line = strtok(NULL, "\n");
      continue;
    }

    if (strcmp(op_str, "pusha") == 0 || strcmp(op_str, "popa") == 0) {
      temp_instr_count += VM_16_REGISTER_MAX + 1;
      line = strtok(NULL, "\n");
      continue;
    }

    temp_instr_count++;
    line = strtok(NULL, "\n");
  }

  fseek(asm_file, 0, SEEK_SET);
  fread(file_content, 1, asm_file_size, asm_file);
  line = strtok(file_content, "\n");
  line_num = 0;
  instr_count = 0;

  while (line != NULL) {
    line_num++;
    char *comment_start = strchr(line, ';');
    if (comment_start)
      *comment_start = '\0';

    while (*line && isspace((unsigned char)*line))
      line++;
    if (*line == '\0') {
      line = strtok(NULL, "\n");
      continue;
    }

    char op_str[16], src1[16], src2[16], dest[16];
    if (sscanf(line, "%s %s %s %s", op_str, src1, src2, dest) != 4) {
      fprintf(stderr, "Line %d: invalid line format\n", line_num);
      goto cleanup;
    }

    if (instr_count > 0 &&
        mem_instr[instr_count - 1][0] == (u16)vm_16_get_opcode("end")) {
      printf("Line %d: detect 'end', stop assembling\n", line_num);
      break;
    }

    if (strcmp(op_str, "label") == 0) {
      line = strtok(NULL, "\n");
      continue;
    }

    if (strcmp(op_str, "pusha") == 0) {
      if (instr_count + VM_16_REGISTER_MAX + 1 > VM_16_ASSEMBLE_MAX_SIZE) {
        fprintf(stderr, "Line %d: instruction buffer overflow\n", line_num);
        goto cleanup;
      }
      for (int i = 0; i <= VM_16_REGISTER_MAX; i++) {
        mem_instr[instr_count][0] = (u16)vm_16_get_opcode("push");
        mem_instr[instr_count][1] = (u16)i;
        mem_instr[instr_count][2] = 0;
        mem_instr[instr_count][3] = 0;
        instr_count++;
      }
      line = strtok(NULL, "\n");
      continue;
    }

    if (strcmp(op_str, "popa") == 0) {
      if (instr_count + VM_16_REGISTER_MAX + 1 > VM_16_ASSEMBLE_MAX_SIZE) {
        fprintf(stderr, "Line %d: instruction buffer overflow\n", line_num);
        goto cleanup;
      }
      for (int i = VM_16_REGISTER_MAX; i >= 0; i--) {
        mem_instr[instr_count][0] = (u16)vm_16_get_opcode("pop");
        mem_instr[instr_count][1] = 0;
        mem_instr[instr_count][2] = 0;
        mem_instr[instr_count][3] = (u16)i;
        instr_count++;
      }
      line = strtok(NULL, "\n");
      continue;
    }

    mem_instr[instr_count][0] = (u16)vm_16_get_opcode(op_str);
    mem_instr[instr_count][1] = vm_16_parse_op(src1);
    mem_instr[instr_count][2] = vm_16_parse_op(src2);
    mem_instr[instr_count][3] = vm_16_parse_op(dest);

    instr_count++;

    if (instr_count >= VM_16_ASSEMBLE_MAX_SIZE) {
      fprintf(stderr, "Line %d: Maximum instruction count (%d) reached\n",
              line_num, VM_16_ASSEMBLE_MAX_SIZE);
      break;
    }

    line = strtok(NULL, "\n");
  }
  if (instr_count == 0) {
    fprintf(stderr, "Error: No valid instructions found\n");
    goto cleanup;
  }

  const size_t total_size = instr_count * sizeof(VM_16_InstrBuffer);
  const size_t bytes_written =
      fwrite(mem_instr, sizeof(VM_16_InstrBuffer), instr_count, bin_file);
  if (bytes_written != (size_t)instr_count) {
    fprintf(stderr, "Error: Failed to write binary file\n");
    goto cleanup;
  }

  fprintf(stdout, "Assemble completed successfully!\n");
  fprintf(stdout, "Total instructions: %d\n", instr_count);
  fprintf(stdout, "Output binary size: %zu bytes\n", total_size);
  goto cleanup;
cleanup:
  if (asm_file)
    fclose(asm_file);
  if (bin_file)
    fclose(bin_file);
  if (file_content)
    free(file_content);
  return;
}