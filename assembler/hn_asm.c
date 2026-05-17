#include "assembler/assembler.h"
#include <stdio.h>
int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s source.asm output.bin\n", argv[0]);
    return -1;
  }

  const char *src_asm = argv[1];
  const char *dst_bin = argv[2];

  assemble(src_asm, dst_bin);

  return 0;
}