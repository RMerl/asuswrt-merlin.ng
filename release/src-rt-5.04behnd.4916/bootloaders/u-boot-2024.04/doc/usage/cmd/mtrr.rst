.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: mtrr (command)

mtrr command
============

Synopsis
--------

    mtrr [list]
    mtrr set <reg> <type> <start> <size>
    mtrr disable <reg>
    mtrr enable


Description
-----------

The *mtrr* command is used to dump the Memory Type Range Registers (MTRRs) on
an x86 machine. These register control cache behaviour in selected memory
ranges.

Note that the number of registers can vary between CPUs.


mtrr [list]
~~~~~~~~~~~

List the MTRRs. The table shows the following information:

Reg
    Register number (the first is register 0)

Valid
    Shows Y if the register is valid (has bit 11 set), N if not

Write-type
    Shows the behaviour when writing to the memory region. The types are
    abbreviated to fit a reasonable line length. Valid types shown below.

    ======  ==============  ====================================================
    Value   Type            Meaning
    ======  ==============  ====================================================
    0       Uncacheable     Skip cache and write directly to memory
    1       Combine         Multiple writes can be combined into one transaction
    4       Through         Update cache and also write to memory
    5       Protect         Writes are prohibited
    6       Back            Update cache but don't write to memory
    ======  ==============  ====================================================

Base
    Base memory address from which the register controls behaviour

Mask
    Mask value, which also indicates the size

Size
    Length of memory region within which the register controls behaviour


mtrr set
~~~~~~~~

This sets the value of a particular MTRR. Parameters are:

reg
    Register number to set, with 0 being the first

type
    Access type to set. See Write-type above for valid types. This uses the name
    rather than its numeric value.

start
    Base memory address from which the register should control behaviour

size
    Length of memory region within which the register controls behaviour


mtrr disable
~~~~~~~~~~~~

This disables a particular register, by clearing its `valid` bit (11).


mtrr enable
~~~~~~~~~~~

This enables a particular register, by setting its `valid` bit (11).


Example
-------

This shows disabling and enabling an MTRR, as well as setting its type::

    => mtrr
    CPU 0:
    Reg Valid Write-type   Base   ||        Mask   ||        Size   ||
    0   Y     Back         0000000000000000 0000000f80000000 0000000080000000
    1   Y     Back         0000000080000000 0000000fe0000000 0000000020000000
    2   Y     Back         00000000a0000000 0000000ff0000000 0000000010000000
    3   Y     Uncacheable  00000000ad000000 0000000fff000000 0000000001000000
    4   Y     Uncacheable  00000000ae000000 0000000ffe000000 0000000002000000
    5   Y     Combine      00000000d0000000 0000000ff0000000 0000000010000000
    6   N     Uncacheable  0000000000000000 0000000000000000 0000001000000000
    7   N     Uncacheable  0000000000000000 0000000000000000 0000001000000000
    8   N     Uncacheable  0000000000000000 0000000000000000 0000001000000000
    9   N     Uncacheable  0000000000000000 0000000000000000 0000001000000000
    => mtrr d 5
    => mtrr
    CPU 0:
    Reg Valid Write-type   Base   ||        Mask   ||        Size   ||
    0   Y     Back         0000000000000000 0000000f80000000 0000000080000000
    1   Y     Back         0000000080000000 0000000fe0000000 0000000020000000
    2   Y     Back         00000000a0000000 0000000ff0000000 0000000010000000
    3   Y     Uncacheable  00000000ad000000 0000000fff000000 0000000001000000
    4   Y     Uncacheable  00000000ae000000 0000000ffe000000 0000000002000000
    5   N     Combine      00000000d0000000 0000000ff0000000 0000000010000000
    6   N     Uncacheable  0000000000000000 0000000000000000 0000001000000000
    7   N     Uncacheable  0000000000000000 0000000000000000 0000001000000000
    8   N     Uncacheable  0000000000000000 0000000000000000 0000001000000000
    9   N     Uncacheable  0000000000000000 0000000000000000 0000001000000000
    => mtrr e 5
    => mtrr
    CPU 0:
    Reg Valid Write-type   Base   ||        Mask   ||        Size   ||
    0   Y     Back         0000000000000000 0000000f80000000 0000000080000000
    1   Y     Back         0000000080000000 0000000fe0000000 0000000020000000
    2   Y     Back         00000000a0000000 0000000ff0000000 0000000010000000
    3   Y     Uncacheable  00000000ad000000 0000000fff000000 0000000001000000
    4   Y     Uncacheable  00000000ae000000 0000000ffe000000 0000000002000000
    5   Y     Combine      00000000d0000000 0000000ff0000000 0000000010000000
    6   N     Uncacheable  0000000000000000 0000000000000000 0000001000000000
    7   N     Uncacheable  0000000000000000 0000000000000000 0000001000000000
    8   N     Uncacheable  0000000000000000 0000000000000000 0000001000000000
    9   N     Uncacheable  0000000000000000 0000000000000000 0000001000000000
    => mtrr set 5 Uncacheable d0000000 10000000
    => mtrr
    CPU 0:
    Reg Valid Write-type   Base   ||        Mask   ||        Size   ||
    0   Y     Back         0000000000000000 0000000f80000000 0000000080000000
    1   Y     Back         0000000080000000 0000000fe0000000 0000000020000000
    2   Y     Back         00000000a0000000 0000000ff0000000 0000000010000000
    3   Y     Uncacheable  00000000ad000000 0000000fff000000 0000000001000000
    4   Y     Uncacheable  00000000ae000000 0000000ffe000000 0000000002000000
    5   Y     Uncacheable  00000000d0000000 0000000ff0000000 0000000010000000
    6   N     Uncacheable  0000000000000000 0000000000000000 0000001000000000
    7   N     Uncacheable  0000000000000000 0000000000000000 0000001000000000
    8   N     Uncacheable  0000000000000000 0000000000000000 0000001000000000
    9   N     Uncacheable  0000000000000000 0000000000000000 0000001000000000
    =>
