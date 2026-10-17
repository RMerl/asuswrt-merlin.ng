.. index::
   single: exception (command)

exception command
=================

Synopsis
--------

::

    exception <type>

Description
-----------

The exception command is used to test the handling of exceptions like undefined
instructions, segmentation faults or alignment faults.

type
  type of exception to be generated. The available types are architecture
  dependent. Use 'help exception' to determine which are available.

  **ARM:**

  breakpoint
    prefetch abort

  unaligned
    data abort

  undefined
    undefined instruction

  **RISC-V:**

  ebreak
    breakpoint exception

  unaligned
    load address misaligned

  undefined
    undefined instruction

  **Sandbox:**

  sigsegv
    illegal memory access

  undefined
    undefined instruction

  **x86:**

  undefined
    undefined instruction

Examples
--------

::

    => exception undefined

    Illegal instruction
    pc = 0x56076dd1a0f9, pc_reloc = 0x540f9

    resetting ...
