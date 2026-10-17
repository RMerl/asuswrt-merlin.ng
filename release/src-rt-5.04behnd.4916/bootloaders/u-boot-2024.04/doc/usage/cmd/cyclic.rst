.. SPDX-License-Identifier: GPL-2.0+

.. index::
   single: cyclic (command)

cyclic command
==============

Synopsis
--------

::

    cyclic list

Description
-----------

The cyclic list command provides a list of the currently registered
cyclic functions.

This shows the following information:

Function
    Function name

cpu-time
    Total time spent in this cyclic function.

Frequency
    Frequency of execution of this function, e.g. 100 times/s for a
    pediod of 10ms.


See :doc:`../../develop/cyclic` for more information on cyclic functions.

Example
-------

::

    => cyclic list
    function: cyclic_demo, cpu-time: 52906 us, frequency: 99.20 times/s

Configuration
-------------

The cyclic command is only available if CONFIG_CMD_CYCLIC=y.
