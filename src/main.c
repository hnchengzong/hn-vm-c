#include "vm.h"
#include "vm_16.h"
#include <stdio.h>

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s program.bin\n", argv[0]);
    return 1;
  }

  int mode;
  printf("choose mode(8=8bit 16=16bit): ");
  scanf("%d", &mode);

  switch (mode) {
  case 8: {
    vm v;
    vm_init(&v);
    if (!vm_load_program(&v, argv[1])) {
      fprintf(stderr, "load program failed: %s\n", argv[1]);
      return 1;
    }
    vm_run(&v);
    vm_print_registers(&v);
    break;
  }
  case 16: {
    vm_16 v;
    vm_16_init(&v);
    if (!vm_16_load_program(&v, argv[1])) {
      fprintf(stderr, "load program failed: %s\n", argv[1]);
      return 1;
    }
    vm_16_run(&v);
    vm_16_print_registers(&v);
    break;
  }
  default:
    fprintf(stderr, "\ninvalid mode\n");
    return 1;
  }

  return 0;
}