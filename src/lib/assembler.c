#include "../include/assembler/assembler.h"
#include "hn_type.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Label labels[MAX_LABELS];
static int label_count = 0;

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

static u16 find_label(const char *name) {
  for (int i = 0; i < label_count; i++) {
    if (strcmp(labels[i].name, name) == 0) {
      return labels[i].addr;
    }
  }
  if (is_number(name)) {
    return (u16)atoi(name);
  }
  fprintf(stderr, "undefined label: %s\n", name);
  exit(EXIT_FAILURE);
}

op_t get_opcode(const char *restrict name) {
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

u8 parse_op(const char *restrict str) {
  if (strcmp(str, "nop") == 0 || strcmp(str, "null") == 0 ||
      strcmp(str, "nil") == 0 || strcmp(str, "none") == 0) {
    return 0;
  }
  if (str[0] == 'r' || str[0] == 'R') {
    u8 reg_num = (u8)atoi(str + 1);
    if (reg_num > REGISTER_MAX) {
      fprintf(stderr, "error:register must be r0~r%d,you input:%s\n",
              REGISTER_MAX, str);
      exit(EXIT_FAILURE);
    }
    return reg_num;
  }
  return (u8)find_label(str);
}

void assemble(const char *restrict asm_file_path,
              const char *restrict bin_file_path) {
  InstrBuffer mem_instr[ASSEMBLE_MAX_SIZE];
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
  label_count = 0;

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
      if (label_count >= MAX_LABELS) {
        fprintf(stderr, "Line %d: too many labels (max %d)\n", line_num,
                MAX_LABELS);
        goto cleanup;
      }
      for (int i = 0; i < label_count; i++) {
        if (strcmp(labels[i].name, src1) == 0) {
          fprintf(stderr, "Line %d: duplicate label: %s\n", line_num, src1);
          goto cleanup;
        }
      }
      strncpy(labels[label_count].name, src1, 15);
      labels[label_count].name[15] = '\0';
      labels[label_count].addr = temp_instr_count * 4;
      label_count++;
      line = strtok(NULL, "\n");
      continue;
    }

    if (strcmp(op_str, "pusha") == 0 || strcmp(op_str, "popa") == 0) {
      temp_instr_count += REGISTER_MAX + 1;
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

    if (mem_instr[instr_count - 1][0] == get_opcode("end")) {
      printf("Line %d: detect 'end', stop assembling\n", line_num);
      break;
    }

    if (strcmp(op_str, "label") == 0) {
      line = strtok(NULL, "\n");
      continue;
    }

    if (strcmp(op_str, "pusha") == 0) {
      if (instr_count + REGISTER_MAX + 1 > ASSEMBLE_MAX_SIZE) {
        fprintf(stderr, "Line %d: instruction buffer overflow\n", line_num);
        goto cleanup;
      }
      for (int i = 0; i <= REGISTER_MAX; i++) {
        mem_instr[instr_count][0] = get_opcode("push");
        mem_instr[instr_count][1] = i;
        mem_instr[instr_count][2] = 0;
        mem_instr[instr_count][3] = 0;
        instr_count++;
      }
      line = strtok(NULL, "\n");
      continue;
    }

    if (strcmp(op_str, "popa") == 0) {
      if (instr_count + REGISTER_MAX + 1 > ASSEMBLE_MAX_SIZE) {
        fprintf(stderr, "Line %d: instruction buffer overflow\n", line_num);
        goto cleanup;
      }
      for (int i = REGISTER_MAX; i >= 0; i--) {
        mem_instr[instr_count][0] = get_opcode("pop");
        mem_instr[instr_count][1] = 0;
        mem_instr[instr_count][2] = 0;
        mem_instr[instr_count][3] = i;
        instr_count++;
      }
      line = strtok(NULL, "\n");
      continue;
    }

    mem_instr[instr_count][0] = get_opcode(op_str);
    mem_instr[instr_count][1] = parse_op(src1);
    mem_instr[instr_count][2] = parse_op(src2);
    mem_instr[instr_count][3] = parse_op(dest);

    instr_count++;

    if (instr_count >= ASSEMBLE_MAX_SIZE) {
      fprintf(stderr, "Line %d: Maximum instruction count (%d) reached\n",
              line_num, ASSEMBLE_MAX_SIZE);
      break;
    }

    line = strtok(NULL, "\n");
  }
  if (instr_count == 0) {
    fprintf(stderr, "Error: No valid instructions found\n");
    goto cleanup;
  }

  const size_t total_size = instr_count * INSTRUCTION_SIZE;
  const size_t bytes_written =
      fwrite(mem_instr, INSTRUCTION_SIZE, instr_count, bin_file);
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