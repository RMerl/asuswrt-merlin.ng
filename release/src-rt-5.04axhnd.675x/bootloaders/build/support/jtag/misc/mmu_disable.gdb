restore armv8_disable_mmu.bin binary $pc+0x1000
set $pc=$pc+0x1000
b $pc+0x14c
c
