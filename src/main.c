

#include "vm.h"
#include <stdio.h>
int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s program.bin\n", argv[0]);
    return 1;
  }
  vm v;
  vm_init(&v);

  if (!vm_load_program(&v, argv[1])) {
    fprintf(stderr, "load program failed: %s\n", argv[1]);
    return 1;
  }

  vm_run(&v);

  vm_print_registers(&v);

  return 0;
}
