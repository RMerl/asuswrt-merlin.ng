.. SPDX-License-Identifier: GPL-2.0+

.. index::
   single: base (command)

base command
============

Synopsis
--------

::

    base [address]

Description
-----------

The *base* command sets or displays the address offset used by the memory
commands *cmp, cp, md, mdc, mm, ms, mw, mwc*.

All other commands ignore the address defined by *base*.

address
    new base address as hexadecimal number. If no value is provided, the current
    value is displayed.
