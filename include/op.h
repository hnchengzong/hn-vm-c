#pragma once

// #ifndef nop
// #define nop 0b00000000
// #endif

// #ifndef add
// #define add 0b00000001
// #endif

// #ifndef sub
// #define sub 0b00000010
// #endif

// #ifndef mul
// #define mul 0b00000011
// #endif

// #ifndef div
// #define div 0b00000100
// #endif

// #ifndef mod
// #define mod 0b00000101
// #endif

// #ifndef AND // Can only be capitalized because and is a keyword.
// #define AND 0b00000110
// #endif

// #ifndef OR
// #define OR 0b00000111
// #endif

// #ifndef XOR
// #define XOR 0b00001000
// #endif

// #ifndef NOT
// #define NOT 0b00001001
// #endif

// #ifndef shl
// #define shl 0b00001010
// #endif

// #ifndef shr
// #define shr 0b00001011
// #endif

// #ifndef mov
// #define mov 0b00001100
// #endif

// #ifndef always
// #define always 0b00010000
// #endif

// #ifndef eq
// #define eq 0b00010001
// #endif

// #ifndef neq
// #define neq 0b00010010
// #endif

// #ifndef lt
// #define lt 0b00010011
// #endif

// #ifndef gt
// #define gt 0b00010100
// #endif

// #ifndef le
// #define le 0b00010101
// #endif

// #ifndef ge
// #define ge 0b00010110
// #endif

// #ifndef never
// #define never 0b00010111
// #endif

// #ifndef push
// #define push 0b00011000
// #endif

// #ifndef pop
// #define pop 0b00011001
// #endif

// #ifndef write
// #define write 0b00100000
// #endif

// #ifndef read
// #define read 0b00100001
// #endif

// #ifndef mem
// #define mem 0b00000000
// #endif

// // read=read|mem=read_mem
// #ifndef read_mem
// #define read_mem 0b00100001
// #endif

// #ifndef write_mem
// #define write_mem 0b00100000
// #endif

// #ifndef disk
// #define disk 0b00110000
// #endif

// // read|disk=read_disk
// #ifndef read_disk
// #define read_disk 0b00110000
// #endif

// #ifndef write_disk
// #define write_disk 0b00110001
// #endif

// #ifndef io
// #define io 0b00101000
// #endif

// #ifndef stdio
// #define stdio 0b00101100
// #endif

// #ifndef call
// #define call 0b01000000
// #endif

// #ifndef ret
// #define ret 0b01000001
// #endif

// #ifndef halt
// #define halt 0b10000000
// #endif

#include "hn_type.h"
typedef u8 op_t;

typedef enum {
  NOP = 0b00000000,    // 0x00
  ADD = 0b00000001,    // 0x01
  SUB = 0b00000010,    // 0x02
  MUL = 0b00000011,    // 0x03
  DIV = 0b00000100,    // 0x04
  MOD = 0b00000101,    // 0x05
  AND = 0b00000110,    // 0x06
  OR = 0b00000111,     // 0x07
  XOR = 0b00001000,    // 0x08
  NOT = 0b00001001,    // 0x09
  SHL = 0b00001010,    // 0x0A
  SHR = 0b00001011,    // 0x0B
  MOV = 0b00001100,    // 0x0C
  ALWAYS = 0b00010000, // 0x10
  EQ = 0b00010001,     // 0x11
  NEQ = 0b00010010,    // 0x12
  LT = 0b00010011,     // 0x13
  GT = 0b00010100,     // 0x14
  LE = 0b00010101,     // 0x15
  GE = 0b00010110,     // 0x16
  NEVER = 0b00010111,  // 0x17
  PUSH = 0b00011000,   // 0x18
  POP = 0b00011001,    // 0x19
  WRITE = 0b00100000,  // 0x20
  READ = 0b00100001,   // 0x21
  MEM = 0b00000000,    // 0x00
  DISK = 0b00110000,   // 0x30
  IO = 0b00101000,     // 0x28
  STDIO = 0b00101100,  // 0x2C
  CALL = 0b01000000,   // 0x40
  RET = 0b01000001,    // 0x41
  HALT = 0b10000000    // 0x80
} OpCode;

typedef struct {
  const char *name;
  const op_t code;
} OpInfo;

static const OpInfo op_table[] = {
    {"nop", NOP},     {"add", ADD},       {"sub", SUB},   {"mul", MUL},
    {"div", DIV},     {"mod", MOD},       {"and", AND},   {"or", OR},
    {"xor", XOR},     {"not", NOT},       {"shl", SHL},   {"shr", SHR},
    {"mov", MOV},     {"always", ALWAYS}, {"eq", EQ},     {"neq", NEQ},
    {"lt", LT},       {"gt", GT},         {"le", LE},     {"ge", GE},
    {"never", NEVER}, {"push", PUSH},     {"pop", POP},   {"write", WRITE},
    {"read", READ},   {"mem", MEM},       {"disk", DISK}, {"io", IO},
    {"stdio", STDIO}, {"call", CALL},     {"ret", RET},   {"halt", HALT},
    {NULL, 0}};

#ifndef OP_NUM
#define OP_NUM (sizeof(op_table) / sizeof(op_table[0]) - 1)
#endif