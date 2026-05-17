# hn-vm-c

[![简体中文](https://img.shields.io/badge/简体中文-green)](README.zh_CN.md) [![English](https://img.shields.io/badge/English-red)](README.md)

A virtual machine and a corresponding assembler written in C, built with xmake. It uses a custom RISC-like instruction set with a fixed instruction length of 4 bytes.

## Instruction Set

### Instructions

- No-operation: `nop` — does nothing, used as a placeholder. (opcode `0x00`; the assembler also accepts `null`, `none`, `nil`)
- Basic arithmetic: `add`, `sub`, `mul`, `div`, `mod` (remainder); `and`, `or`, `xor`, `not`, `shl`, `shr` (logical left/right shift). These operate only between registers, no immediate operands. (opcodes `0x01`–`0x0B`)
- Move: `mov` (`0x0C`)
- Immediate load: `set` (`0x0D`)
- Conditional branches: `always`, `eq`, `nq`, `lt`, `gt`, `le`, `ge`, `never` (`0x10`–`0x17`)
- Stack: `push`, `pop` (`0x18`, `0x19`)
- I/O: `read`, `write` (`0x20`, `0x21`)
- Function calls: `call`, `ret` (`0x40`, `0x41`)
- Halt: `halt` — stops the program and exits. (`0x80`)

### Sources / Destinations

The directly accessible registers are `r0` through `r15`, encoded as `0x00`–`0x0F`. The program counter is `pc` and the stack pointer is `sp`.
To modify `pc` you must use conditional branches or function calls. To access data memory or the disk you must use `read` or `write`.
The program memory, stack memory, and data memory are isolated from each other (all offsets start from 0).
When interacting with the virtual disk, you need to provide a filename; the file should be placed in the same directory.

### Detailed Description

1. Basic arithmetic instructions follow the format: `instruction src1 src2 dst`. For example, `add r0 r1 r2` adds the values stored in `r1` and `r2` and stores the result in `r2`.

2. The immediate instruction `set` uses the format: `set src nop dst value`. For example, `set 4 nop r0` loads the immediate value 4 into `r0`. (Writing `set 4 0 r0` is also correct.)

3. Immediates can also be used with `mov`: `mov r0, 4 r0` adds 4 to the value in `r0`. `mov r0, 0 r1` or `mov r0, nop r1` moves `r0` into `r1` (i.e., places `r0 + 0` into `r1`).

4. Conditional branches have the format: `instruction src1 src2 target`. If the condition is true, `pc` is set to the target address. For example, `always nop nop 0x08` jumps unconditionally to the beginning. `eq r1 r2 0x00` checks if `r1` and `r2` are equal; if so, it jumps to address `0x08`. Labels can also be used as jump targets.

5. Stack instructions: `push src nop nop` and `pop nop nop dst`. For example, `push r0 nop nop` pushes the value of `r0` onto the stack, and `pop nop nop r0` pops the top of the stack into `r0`.

6. The `read` instruction format is `read device_type offset/mode dst`. It reads data from the specified device and stores it into the destination. Examples:
   - `read 0 10 r0` – reads from data memory at offset 10 into `r0`.
   - `read 2 0 r1` – reads a character from `stdin` into `r1`.
   - `read 2 1 r2` – reads a character from the console and writes it into data memory (using `r2` as the address).
   - `read 1 5 r3` – reads from the virtual disk at offset 5 into `r3`.

7. The `write` instruction format is `write device_type offset/mode src`. It writes data from the source to the specified device. Examples:
   - `write 0 20 r0` – writes `r0` to data memory at offset 20.
   - `write 1 8 r2` – writes `r2` to the virtual disk at offset 8.
   - `write 3 0 r1` – outputs `r1` as a character.
   - `write 3 1 r1` – outputs `r1` as a number.
   - `write 3 2 nop` – prints all registers.
   - `write 3 3 nop` – prints data memory.
   - `write 3 4 nop` – prints the stack.

8. `call` format: `call nop nop target_address`. It pushes the address of the next instruction onto the stack and jumps to the target address. Example: `call nop nop 0x20` jumps to address `0x20` and saves the return address.
   `ret` format: `ret nop nop nop`. It pops the return address from the stack into `pc`, performing a function return.

9. The `halt` instruction terminates the program. Every program should end with a `halt`; otherwise, when `pc` reaches the end of program memory, an abnormal exit occurs. The assembler also stops processing code after encountering `halt`.

## Data Width and Memory Size

The VM currently supports only 8-bit registers. Program memory holds at most 64 instructions (256 bytes). The stack, data memory, and disk space are each 256 bytes. Every instruction is exactly 4 bytes long.

## Assembler

The assembler converts mnemonics like `add` and `r0` into their corresponding `u8` values. Text after `;` is treated as a comment. The assembler automatically skips whitespace and assembles line by line (separated by `\n`) in 4-byte blocks. Labels are supported: `label lab1 nop nop` does not occupy program space; a later `call nop nop lab1` will jump to the location of `lab1`.
`popa nop nop nop` and `pusha nop nop nop` are macros that automatically push all registers onto the stack or pop all registers from the stack, respectively.

When the assembler runs, the first pass removes comments and empty lines and checks for 4-byte alignment (except for labels and macros). The second pass expands macros and translates assembly instructions into binary machine code. Mnemonics like `nop` and `null` are directly written as `0`. The assembler stops processing any code after encountering `halt`, so `halt` must be placed at the very end.

## Running (requires xmake)

```bash
git clone --depth 1 https://github.com/hnchengzong/hn-vm-c.git
cd ./hn-vm-c

# Build both the VM and assembler
xmake build

# Or build individually
xmake build hn-vm-c
xmake build hn-assembler

# Switch build mode
xmake f -m debug
xmake f -m release

# Test run
xmake build
./build/assembler/release/hn-assembler ./tests/asm/test_basic_asm.asm ./tests/bin/test_basic_asm.bin
./build/hn_vm_c/release/hn-vm-c ./tests/bin/test_basic_asm.bin

```
