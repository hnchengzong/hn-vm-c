#include "../include/assembler/assembler.h"
#include "hn_type.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

u8 parse_operand(const char *restrict str) {
  if (is_number(str)) {
    return (u8)atoi(str);
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
  fprintf(stderr, "Invalid operand: %s\n", str);
  return 0;
}

void assemble(const char *restrict asm_file_path,
              const char *restrict bin_file_path) {
  InstrBuffer mem_instr[ASSEMBLE_MAX_SIZE];
  int instr_cnt = 0;

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

  while (line != NULL) {
    line_num++;

    char *restrict const comment_start = strchr(line, ';');
    if (comment_start) {
      *comment_start = '\0';
    }

    while (*line && isspace((unsigned char)*line))
      line++;
    if (*line == '\0') {
      line = strtok(NULL, "\n");
      continue;
    }

    char op_str[16], src1[16], src2[16], dest[16];
    if (sscanf(line, "%s %s %s %s", op_str, src1, src2, dest) != 4) {
      fprintf(stderr, "Line %d: Invalid instruction format\n", line_num);
      goto cleanup;
    }

    mem_instr[instr_cnt][0] = get_opcode(op_str);
    mem_instr[instr_cnt][1] = parse_operand(src1);
    mem_instr[instr_cnt][2] = parse_operand(src2);
    mem_instr[instr_cnt][3] = parse_operand(dest);

    instr_cnt++;

    if (instr_cnt >= ASSEMBLE_MAX_SIZE) {
      fprintf(stderr, "Line %d: Maximum instruction count (%d) reached\n",
              line_num, ASSEMBLE_MAX_SIZE);
      break;
    }

    if (mem_instr[instr_cnt - 1][0] == get_opcode("halt")) {
      printf("Line %d: HALT detected, stop assembling\n", line_num);
      break;
    }

    line = strtok(NULL, "\n");
  }
  if (instr_cnt == 0) {
    fprintf(stderr, "Error: No valid instructions found\n");
    goto cleanup;
  }

  const size_t total_size = instr_cnt * INSTRUCTION_SIZE;
  const size_t bytes_written =
      fwrite(mem_instr, INSTRUCTION_SIZE, instr_cnt, bin_file);
  if (bytes_written != (size_t)instr_cnt) {
    fprintf(stderr, "Error: Failed to write binary file\n");
    goto cleanup;
  }

  fprintf(stdout, "Assemble completed successfully!\n");
  fprintf(stdout, "Total instructions: %d\n", instr_cnt);
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
