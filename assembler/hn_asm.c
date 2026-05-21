#include "assembler/assembler.h"
#include "assembler/assembler_16.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s source.asm output.bin\n", argv[0]);
    return -1;
  }

  const char *src_asm = argv[1];
  const char *dst_bin = argv[2];

  int mode;
  printf("choose mode(8=8bit 16=16bit): ");
  scanf("%d", &mode);

  switch (mode) {
  case 8:
    assemble(src_asm, dst_bin);
    break;
  case 16:
    vm_16_assemble(src_asm, dst_bin);
    break;
  default:
    fprintf(stderr, "\ninvalid mode\n");
    return -1;
  }

  return 0;
}