.. SPDX-License-Identifier: GPL-2.0+

.. index::
   single: rng (command)

rng command
===========

Synopsis
--------

::

    rng [devnum [n]]

Description
-----------

The *rng* command reads the random number generator(RNG) device and
prints the random bytes read on the console. A maximum of 64 bytes can
be read in one invocation of the command.

devnum
    The RNG device from which the random bytes are to be
    read. Defaults to 0.

n
    Number of random bytes to be read and displayed on the
    console. Default value is 0x40. Max value is 0x40.
