.. SPDX-License-Identifier: GPL-2.0-or-later

.. index::
   single: if (command)

if command
==========

Synopsis
--------

::

    if <test statement>
    then
        <statements>
    fi

    if <test statement>
    then
        <statements>
    else
        <statements>
    fi

Description
-----------

The if command is used to conditionally execute statements.

test statement
    Any command. The test statement set the $? variable. If the value of
    $? becomes 0 (true) the statements after the **then** statement will
    be executed. Otherwise the statements after the **else** statement.

Examples
--------

The examples shows how the value of a numeric variable can be tested with
the :doc:`itest <itest>` command.

::

    => a=1; if itest $a == 0; then echo true; else echo false; fi
    false
    => a=0; if itest $a == 0; then echo true; else echo false; fi
    true

In the following example we try to load an EFI binary via TFTP. If loading
succeeds, the binary is executed.

::

    if tftp $kernel_addr_r shellriscv64.efi; then bootefi $kernel_addr_r; fi

Return value
------------

The value of $? is the return value of the last executed statement.

::

    => if true; then true; else true; fi; echo $?
    0
    => if false; then true; else true; fi; echo $?
    0
    => if false; then false; else false; fi; echo $?
    1
    => if true; then false; else false; fi; echo $?
    1
    => if false; then true; fi; echo $?
    1
