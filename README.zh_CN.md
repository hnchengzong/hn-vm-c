# hn-vm-c

[![简体中文](https://img.shields.io/badge/简体中文-green)](README.zh_CN.md) [![English](https://img.shields.io/badge/English-red)](README.md)

一个使用c写的虚拟机和相应汇编器，使用xmake构建。自定义类RISC指令集，指令为4字节固定长度。

## 指令集

### 指令

- 空指令：nop，无任何操作，占位。（0x00，汇编器也支持写成null，none，nil等形式）
- 基础运算：add、sub、mul、div、mod（取余）；and/or/xor/not、shl/shr（逻辑左移/右移），仅支持寄存器间计算，不支持立即数。（分别对应0x01-0x0B
- 移动：mov。（0x0C）
- 立即数：set。（0x0D）
- 条件判断：always、eq、nq、lt、gt、le、ge、never（0x10-0x17）
- 栈：push/pop。（0x18，0x19）
- I/O：read，write（0x20，0x21）
- 函数调用：call、ret。（0x40，0x41）
- 停机指令：halt(停止程序并退出)。（0x80）

### 源/目标

能直接操作的是寄存器`r0`-`r15`。对应0x00-0x0F.程序计数器为`pc`，栈指针为`sp`。
要想改变`pc`，必须使用条件判断和函数。要使用数据内存或磁盘必须使用`read`或`write`。
运行程序的内存，栈的内存和存放数据的内存是隔离的。（这些偏移都是从0开始）
与磁盘交互时需要输入名称，放在同级文件夹。

### 详细说明

1. 基础运算的指令为： 指令 源1 源2 目标。比如`add r0 r1 r2`是把`r1`，`r2`存储的值相加，结果存到`r2`中。

2. 立即数`set`的指令为：set 源 nop 目标 值。比如 `set 4 nop r0` 是把立即数4设置到r0中。（当然，写`set 4 0 r0`也对）

3. 立即数也可以通过`mov`实现，比如：`mov r0, 4 r0` 相当于把r0储存的值加入4。`mov r0, 0 r1`或则`mov r0, nop r1`是移动`r0`到`r1`，即把`r0+0`放到`r1`中。

4. 条件判断的指令为：指令 源1 源2 跳转目标。条件满足则会把`pc`的值设置为跳转目标。比如`always nop nop 0x08`无条件跳转到开头。`eq r1 r2 0x00`是判断r1和r2的值是否相等，相等则跳转到0x08。你也可以使用标签跳转。

5. 栈指令为 `push 源1 nop nop`和`pop nop nop 目标`，比如`push r0 nop nop`是将r0的值压入栈中，`pop nop nop r0`是将栈顶的值弹出到r0中。

6. read 指令格式为 read 设备类型 偏移 / 模式 目标，功能是从指定设备读取数据并存入目标。比如 `read 0 10 r0` 是从数据内存偏移 10 读取数据到 r0，`read 2 0 r1` 是从`stdin`读取字符到 `r1`，`read 2 1 r2` 是从控制台读取字符写入数据内存，`read 1 5 r3` 是从虚拟磁盘偏移 5 读取数据到 `r3`。

7. write 指令格式为 write 设备类型 偏移 / 模式 源，功能是将源数据写入指定设备。比如 `write 0 20 r0` 是将 r0 写入数据内存偏移 20，`write 1 8 r2` 是将 `r2` 写入虚拟磁盘偏移 8，`write 3 0 r1` 以字符形式输出 r1，`write 3 1 r1` 以数字形式输出 r1，`write 3 2 nop` 打印寄存器，`write 3 3 nop` 打印数据内存，`write 3 4 nop` 打印栈。

8. call 指令格式为 call nop nop  目标地址，功能是将下一条指令的地址压入栈中，并跳转到目标地址执行函数。比如 `call nop nop 0x20` 是跳转到地址 0x20 执行函数，并保存返回地址。
ret 指令格式为 `ret nop nop nop`，会从栈中弹出返回地址并赋值给 pc，实现函数返回。

9. 停机指令`halt`会让程序退出。每个程序都应当要在结尾处有`halt`指令，否则`pc`指向程序内存结尾后会异常退出。汇编器也会在遇到`halt`后不再汇编后面的代码。

## 比特位

目前虚拟机只支持8位寄存器，最多写 64 条指令，栈、内存、磁盘空间只有256字节。每条指令都长4字节。

## 汇编器

汇编器就是把`add`、`r0`等词汇转换成对应的`u8`数字。`;`后面的内容会被视作注释。汇编会自动跳过空格，并通过`\n`换行来逐4字节汇编。支持`label`标签。比如`label lab1 nop nop`不占用程序空间，之后`call nop nop lab1`会跳转到`lab1`所在位置。`popa nop nop nop`和`pusha nop nop nop`是宏，会自动将所有寄存器压入栈中或将栈中的所有寄存器弹出。

当汇编器运行时，第一遍会删除注释和空行，并检查是否满足4字节对齐。之后除了label标签和宏。第二遍会展开宏，并将汇编指令转换为二进制机器码。对于`nop`，`null`等值会直接写入0。汇编器遇到`halt`后自动停止分析代码，所以`halt`要写到最后面。

## 运行（需要xmake）

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
