.. SPDX-License-Identifier: GPL-2.0+

.. index::
   single: itest (command)

itest command
=============

Synopsis
--------

::

    itest[.b | .w | .l | .q | .s] [*]<value1> <op> [*]<value2>

Description
-----------

The itest command is used to compare two values. The return value $? is set
accordingly.

By default it is assumed that the values are 4 byte integers. By appending a
postfix (.b, .w, .l, .q, .s) the size can be specified:

======= ======================================================
postfix meaning
======= ======================================================
.b      1 byte integer
.w      2 byte integer
.l      4 byte integer
.q      8 byte integer (only available if CONFIG_PHYS_64BIT=y)
.s      string
======= ======================================================

value1, value2
    values to compare. Numeric values are hexadecimal. If '*' is prefixed a
    hexadecimal address is passed, which points to the value to be compared.

op
    operator, see table

    ======== ======================
    operator meaning
    ======== ======================
    -lt      less than
    <        less than
    -le      less or equal
    <=       less or equal
    -eq      equal
    ==       equal
    -ne      not equal
    !=       not equal
    <>       not equal
    -ge      greater or equal
    >=       greater or equal
    -gt      greater than
    >        greater than
    ======== ======================

Examples
========

The itest command sets the result variable $? to true (0) or false (1):

::

    => itest 3 < 4; echo $?
    0
    => itest 3 == 4; echo $?
    1

This value can be used in the :doc:`if <if>` command:

::

    => if itest 0x3002 < 0x4001; then echo true; else echo false; fi
    true

Numbers will be truncated according to the postfix before comparing:

::

    => if itest.b 0x3002 < 0x4001; then echo true; else echo false; fi
    false

Postfix .s causes a string compare. The string '0xa1234' is alphabetically
smaller than '0xb'.

    => if itest.s 0xa1234 < 0xb; then echo true; else echo false; fi
    true

A value prefixed by '*' is a pointer to the value in memory.

::

    => mm 0x4000
    00004000: 00000004 ?
    00004004: 00000003 ? =>
    => if itest *0x4000 == 4; then echo true; else echo false; fi
    true
    => if itest *0x4004 == 3; then echo true; else echo false; fi
    true

Configuration
-------------

The command is only available if CONFIG_CMD_ITEST=y.

Return value
------------

The return value $? is 0 (true) if the condition is true and 1 (false)
otherwise.
