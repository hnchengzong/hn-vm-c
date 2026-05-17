set 11 nop r0
set 22 nop r1
set 33 nop r2
set 44 nop r3

write 3 0 r0
set 10 nop r0
write 3 0 r0
write 3 2 nop

pusha nop nop nop
write 3 4 nop ; 打印栈

set 0 nop r0
set 0 nop r1
set 0 nop r2
set 0 nop r3
write 3 2 nop ; 打印寄存器

popa nop nop nop
write 3 2 nop

halt nop nop nop