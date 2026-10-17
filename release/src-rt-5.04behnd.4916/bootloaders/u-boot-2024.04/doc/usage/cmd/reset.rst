.. SPDX-License-Identifier: GPL-2.0+

.. index::
   single: reset (command)

reset command
=============

Synopsis
--------

::

    reset [-w]

Description
-----------

Perform reset of the CPU. By default does COLD reset, which resets CPU,
DDR and peripherals, on some boards also resets external PMIC.

-w
    Do warm WARM, reset CPU but keep peripheral/DDR/PMIC active.


Return value
------------

The return value $? is always set to 0 (true).
