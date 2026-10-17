.. SPDX-License-Identifier: GPL-2.0+

Xtensa
======

Xtensa Architecture and Diamond Cores
-------------------------------------

Xtensa is a configurable processor architecture from Tensilica, Inc.
Diamond Cores are pre-configured instances available for license and
SoC cores in the same manner as ARM, MIPS, etc.

Xtensa licensees create their own Xtensa cores with selected features
and custom instructions, registers and co-processors. The custom core
is configured with Tensilica tools and built with Tensilica's Xtensa
Processor Generator.

There are an effectively infinite number of CPUs in the Xtensa
architecture family. It is, however, not feasible to support individual
Xtensa CPUs in U-Boot. Therefore, there is only a single 'xtensa' CPU
in the cpu tree of U-Boot.

In the same manner as the Linux port to Xtensa, U-Boot adapts to an
individual Xtensa core configuration using a set of macros provided with
the particular core. This is part of what is known as the hardware
abstraction layer (HAL). For the purpose of U-Boot, the HAL consists only
of a few header files. These provide CPP macros that customize sources,
Makefiles, and the linker script.


Adding support for an additional processor configuration
--------------------------------------------------------

The header files for one particular processor configuration are inside
a variant-specific directory located in the arch/xtensa/include/asm
directory. The name of that directory starts with 'arch-' followed by
the name for the processor configuration, for example, arch-dc233c for
the Diamond DC233 processor.

core.h:
  Definitions for the core itself.

The following files are part of the overlay but not used by U-Boot.

tie.h:
  Co-processors and custom extensions defined in the Tensilica Instruction
  Extension (TIE) language.
tie-asm.h:
  Assembly macros to access custom-defined registers and states.


Global Data Pointer, Exported Function Stubs, and the ABI
---------------------------------------------------------

To support standalone applications launched with the "go" command,
U-Boot provides a jump table of entrypoints to exported functions
(grep for EXPORT_FUNC). The implementation for Xtensa depends on
which ABI (or function calling convention) is used.

Windowed ABI presents unique difficulties with the approach based on
keeping global data pointer in dedicated register. Because the register
window rotates during a call, there is no register that is constantly
available for the gd pointer. Therefore, on xtensa gd is a simple
global variable. Another difficulty arises from the requirement to have
an 'entry' at the beginning of a function, which rotates the register
file and reserves a stack frame. This is an integral part of the
windowed ABI implemented in hardware. It makes using a jump table to an
arbitrary (separately compiled) function a bit tricky. Use of a simple
wrapper is also very tedious due to the need to move all possible
register arguments and adjust the stack to handle arguments that cannot
be passed in registers. The most efficient approach is to have the jump
table perform the 'entry' so as to pretend it's the start of the real
function. This requires decoding the target function's 'entry'
instruction to determine the stack frame size, and adjusting the stack
pointer accordingly, then jumping into the target function just after
the 'entry'. Decoding depends on the processor's endianness so uses the
HAL. The implementation (12 instructions) is in examples/stubs.c.


Access to Invalid Memory Addresses
----------------------------------

U-Boot does not check if memory addresses given as arguments to commands
such as "md" are valid. There are two possible types of invalid
addresses: an area of physical address space may not be mapped to RAM
or peripherals, or in the presence of MMU an area of virtual address
space may not be mapped to physical addresses.

Accessing first type of invalid addresses may result in hardware lockup,
reading of meaningless data, written data being ignored or an exception,
depending on the CPU wiring to the system. Accessing second type of
invalid addresses always ends with an exception.

U-Boot for Xtensa provides a special memory exception handler that
reports such access attempts and resets the board.


.. Chris Zankel
.. Ross Morley
