# hn-vm-c

一个使用c写到的虚拟机和汇编器，使用xmake构建。自定义类RISC指令集，指令为4字节固定长度。

## 指令集

### 指令

- 空指令：nop，无任何操作，占位。（0x00）
- 基础运算：add、sub、mul、div、mod（取余）；and/or/xor/not、shl/shr（逻辑左移/右移），仅支持寄存器间计算，不支持立即数。（分别对应0x01-0x0B
- 移动：mov。（0x0C）
- 立即数：set。（0x0D）
- 条件判断：always、eq、nq、lt、gt、le、ge、never（0x10-0x17）
- I/O：read，write
- 栈：push/pop。（0x18，0x19）
- 函数调用：call、ret。（0x40，0x41）
- 停机指令：halt(停止程序并退出)。（0x80）

### 源/目标

是寄存器`r0`-`r15`。对应0x00-0x0F.程序计数器为`pc`，栈指针为`sp`。

### 详细说明

基础运算的指令为： 指令 源1 源2 目标。比如`add r0 r1 r2`是把`r1`，`r2`存储的值相加，结果存到`r2`中。
条件判断中，
立即数`set`的指令为：set 源 nop 目标 值。比如 `set 4 nop r0` 是把立即数4设置到r0中。（当然，写`set 4 0 r0`也对）
立即数也可以通过`mov`实现，比如：`mov r0, 4 r0` 相当于把r0储存的值加入4。`mov r0, 0 r1`或则`mov r0, nop r1`是移动r0到r1。
条件判断的指令为：指令 源1 源2 跳转目标。条件满足则会把`pc`的值设置为跳转目标。比如`always nop nop 0x08`无条件跳转到开头。`eq r1 r2 0x00`是判断r1和r2的值是否相等，相等则跳转到0x08。
栈指令为 `push 源1 nop nop`和`pop nop nop 目标`，比如`push r0 nop nop`是将r0的值压入栈中，`pop nop nop r0`是将栈顶的值弹出到r0中。
停机指令`halt`会让程序退出。每个程序都应当要在结尾处有`halt`指令，否则会一直回到开头无限循环。汇编器也会在遇到`halt`后不再汇编后面的代码。

## 汇编器

汇编器就是把`add`、`r0`等词汇转换成对应的`u8`数字。`;`后面的内容会被视作注释。汇编会自动跳过空格，通过`\n`换行来4字节汇编。

## 运行

```bash

git clone --depth 1 https://github.com/hnchengzong/hn-vm-c.git
cd ./hn-vm-c

# 构建虚拟机和汇编器
xmake build

# 单独构建
xmake build hn-vm-c
xmake build hn-assembler

# 切换模式
xmake f -m debug
xmake f -m release

# 测试运行
xmake build
./build/assembler/release/hn-assembler ./tests/asm/test_basic_asm.asm ./tests/bin/test_basic_asm.bin
./build/hn_vm_c/release/hn-vm-c ./tests/bin/test_basic_asm.bin

```
