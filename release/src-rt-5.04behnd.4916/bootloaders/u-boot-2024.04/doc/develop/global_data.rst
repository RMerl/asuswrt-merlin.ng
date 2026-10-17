.. SPDX-License-Identifier: GPL-2.0+

Global data
===========

Globally required fields are held in the global data structure. A pointer to the
structure is available as symbol gd. The symbol is made available by the macro
%DECLARE_GLOBAL_DATA_PTR.

Register pointing to global data
--------------------------------

On most architectures the global data pointer is stored in a register.

+------------+----------+
| ARC        | r25      |
+------------+----------+
| ARM 32bit  | r9       |
+------------+----------+
| ARM 64bit  | x18      |
+------------+----------+
| M68000     | d7       |
+------------+----------+
| MicroBlaze | r31      |
+------------+----------+
| Nios II    | gp       |
+------------+----------+
| PowerPC    | r2       |
+------------+----------+
| RISC-V     | gp (x3)  |
+------------+----------+
| SuperH     | r13      |
+------------+----------+
| x86 32bit  | fs       |
+------------+----------+

The sandbox, x86_64, and Xtensa are notable exceptions.

Current implementation uses a register for the GD pointer because this results
in smaller code. However, using plain global data for the GD pointer would be
possible too (and simpler, as it does not require the reservation of a specific
register for it), but the resulting code is bigger.

Clang for ARM does not support assigning a global register. When using Clang
gd is defined as an inline function using assembly code. This adds a few bytes
to the code size.

Binaries called by U-Boot are not aware of the register usage and will not
conserve gd. UEFI binaries call the API provided by U-Boot and may return to
U-Boot. The value of gd has to be saved every time U-Boot is left and restored
whenever U-Boot is reentered. This is also relevant for the implementation of
function tracing. For setting the value of gd function set_gd() can be used.

Global data structure
---------------------

.. kernel-doc:: include/asm-generic/global_data.h
   :internal:
