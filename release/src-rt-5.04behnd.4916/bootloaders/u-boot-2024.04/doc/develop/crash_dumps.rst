.. SPDX-License-Identifier: GPL-2.0+
.. Copyright (c) 2020 Heinrich Schuchardt

Analyzing crash dumps
=====================

When the CPU detects an instruction that it cannot execute it raises an
interrupt. U-Boot then writes a crash dump. This chapter describes how such
dump can be analyzed.

Creating a crash dump voluntarily
---------------------------------

For describing the analysis of a crash dump we need an example. U-Boot comes
with a command :doc:`exception <../usage/cmd/exception>` that comes in handy
here. The command is enabled by::

    CONFIG_CMD_EXCEPTION=y

The example output below was recorded when running qemu\_arm64\_defconfig on
QEMU::

    => exception undefined
    "Synchronous Abort" handler, esr 0x02000000
    elr: 00000000000101fc lr : 00000000000214ec (reloc)
    elr: 000000007ff291fc lr : 000000007ff3a4ec
    x0 : 000000007ffbd7f8 x1 : 0000000000000000
    x2 : 0000000000000001 x3 : 000000007eedce18
    x4 : 000000007ff291fc x5 : 000000007eedce50
    x6 : 0000000000000064 x7 : 000000007eedce10
    x8 : 0000000000000000 x9 : 0000000000000004
    x10: 6db6db6db6db6db7 x11: 000000000000000d
    x12: 0000000000000006 x13: 000000000001869f
    x14: 000000007edd7dc0 x15: 0000000000000002
    x16: 000000007ff291fc x17: 0000000000000000
    x18: 000000007eed8dc0 x19: 0000000000000000
    x20: 000000007ffbd7f8 x21: 0000000000000000
    x22: 000000007eedce10 x23: 0000000000000002
    x24: 000000007ffd4c80 x25: 0000000000000000
    x26: 0000000000000000 x27: 0000000000000000
    x28: 000000007eedce70 x29: 000000007edd7b40

    Code: b00003c0 912ad000 940029d6 17ffff52 (e7f7defb)
    Resetting CPU ...

    resetting ...

The first line provides us with the type of interrupt that occurred.
On ARMv8 a synchronous abort is an exception thrown when hitting an unallocated
instruction. The exception syndrome register ESR register contains information
describing the reason for the exception. Bit 25 set here indicates that a 32 bit
instruction led to the exception.

The second line provides the contents of the elr and the lr register after
subtracting the relocation offset. - U-Boot relocates itself after being
loaded. - The relocation offset can also be displayed using the bdinfo command.

After the contents of the registers we get a line indicating the machine
code of the instructions preceding the crash and in parentheses the instruction
leading to the dump.

Analyzing the code location
---------------------------

We can convert the instructions in the line starting with 'Code:' into mnemonics
using the objdump command. To make things easier scripts/decodecode is
supplied::

    $echo 'Code: b00003c0 912ad000 940029d6 17ffff52 (e7f7defb)' | \
      CROSS_COMPILE=aarch64-linux-gnu- ARCH=arm64 scripts/decodecode
    Code: b00003c0 912ad000 940029d6 17ffff52 (e7f7defb)
    All code
    ========
       0:   b00003c0     adrp   x0, 0x79000
       4:   912ad000     add    x0, x0, #0xab4
       8:   940029d6     bl     0xa760
       c:   17ffff52     b      0xfffffffffffffd54
      10:*  e7f7defb     .inst  0xe7f7defb ; undefined <-- trapping instruction

    Code starting with the faulting instruction
    ===========================================
       0:   e7f7defb     .inst  0xe7f7defb ; undefined

Now lets use the locations provided by the elr and lr registers after
subtracting the relocation offset to find out where in the code the crash
occurred and from where it was invoked.

File u-boot.map contains the memory layout of the U-Boot binary. Here we find
these lines::

   .text.do_undefined
                  0x00000000000101fc        0xc cmd/built-in.o
   .text.exception_complete
                  0x0000000000010208       0x90 cmd/built-in.o
   ...
   .text.cmd_process
                  0x00000000000213b8      0x164 common/built-in.o
                  0x00000000000213b8                cmd_process
   .text.cmd_process_error
                  0x000000000002151c       0x40 common/built-in.o
                  0x000000000002151c                cmd_process_error

So the error occurred at the start of function do\_undefined() and this
function was invoked from somewhere inside function cmd\_process().

If we want to dive deeper, we can disassemble the U-Boot binary::

    $ aarch64-linux-gnu-objdump -S -D u-boot | less

    00000000000101fc <do_undefined>:
    {
            /*
             * 0xe7f...f.   is undefined in ARM mode
             * 0xde..       is undefined in Thumb mode
            */
            asm volatile (".word 0xe7f7defb\n");
       101fc:       e7f7defb        .inst   0xe7f7defb ; undefined
            return CMD_RET_FAILURE;
    }
    10200:       52800020        mov     w0, #0x1        // #1
    10204:       d65f03c0        ret

This example is based on the ARMv8 architecture but the same procedures can be
used on other architectures as well.

Crashs in UEFI binaries
-----------------------

If UEFI images are loaded when a crash occurs, their load addresses are
displayed. If the process counter points to an address in a loaded UEFI
binary, the relative process counter position is indicated. Here is an
example executed on the U-Boot sandbox::

    => load host 0:1 $kernel_addr_r buggy.efi
    5632 bytes read in 0 ms
    => bootefi $kernel_addr_r
    Booting /buggy.efi
    Buggy world!

    Segmentation violation
    pc = 0x19fc264c, pc_reloc = 0xffffaa4688b1664c

    UEFI image [0x0000000019fc0000:0x0000000019fc6137] pc=0x264c '/buggy.efi'

The crash occured in UEFI binary buggy.efi at relative position 0x264c.
Disassembly may be used to find the actual source code location::

    $ x86_64-linux-gnu-objdump -S -D buggy_efi.so

    0000000000002640 <memset>:
        2640:       f3 0f 1e fa             endbr64
        2644:       48 89 f8                mov    %rdi,%rax
        2647:       48 89 f9                mov    %rdi,%rcx
        264a:       eb 0b                   jmp    2657 <memset+0x17>
        264c:       40 88 31                mov    %sil,(%rcx)

Architecture specific details
-----------------------------

ARMv8
~~~~~

On the ARM 64-bit architecture CONFIG_ARMV8_SPL_EXCEPTION_VECTORS controls
if the exception vector tables are set up in the Secondary Program Loader (SPL).
Without initialization of the tables crash dumps cannot be shown. The feature is
disabled by default on most boards to reduce the size of the SPL.

RISC-V
~~~~~~

On the RISC-V architecture CONFIG_SHOW_REGS=y has to be specified to show
all registers in crash dumps.

Sandbox
~~~~~~~

The sandbox U-Boot binary must be invoked with parameter *-S* to display crash
dumps:

.. code-block:: bash

    ./u-boot -S -T

Only with CONFIG_SANDBOX_CRASH_RESET=y the sandbox reboots after a crash.
