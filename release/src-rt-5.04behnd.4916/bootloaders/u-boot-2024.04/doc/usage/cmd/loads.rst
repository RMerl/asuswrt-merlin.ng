.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: loads (command)

loads command
=============

Synopsis
--------

::

    loads [offset [baud]]

Description
-----------

The loads command is used to transfer a file to the device via the serial line
using the Motorola S-record file format.

offset
    offset added to the addresses in the S-record file

baud
    baud rate to use for download. This parameter is only available if
    CONFIG_SYS_LOADS_BAUD_CHANGE=y

Example
-------

As example file to be transferred we use a script printing 'hello s-record'.
Here are the commands to create the S-record file:

.. code-block:: bash

    $ echo 'echo hello s-record' > script.txt
    $ mkimage -T script -d script.txt script.scr
    Image Name:
    Created:      Sun Jun 25 10:35:02 2023
    Image Type:   PowerPC Linux Script (gzip compressed)
    Data Size:    28 Bytes = 0.03 KiB = 0.00 MiB
    Load Address: 00000000
    Entry Point:  00000000
    Contents:
       Image 0: 20 Bytes = 0.02 KiB = 0.00 MiB
    $ srec_cat script.scr -binary -CRLF -Output script.srec
    $ echo -e "S9030000FC\r" >> script.srec
    $ cat script.srec
    S0220000687474703A2F2F737265636F72642E736F75726365666F7267652E6E65742F1D
    S1230000270519566D773EB6649815E30000001700000000000000003DE3D97005070601E2
    S12300200000000000000000000000000000000000000000000000000000000000000000BC
    S11A00400000000F0000000068656C6C6F20732D7265636F72640A39
    S5030003F9
    S9030000FC
    $

The load address in the first S1 record is 0x0000.

The terminal emulation program picocom is invoked with *cat* as the send
command to transfer the file.

.. code-block::

    picocom --send-cmd 'cat' --baud 115200 /dev/ttyUSB0

After entering the *loads* command the key sequence <CTRL-A><CTRL-S> is used to
let picocom prompt for the file name. Picocom invokes the program *cat* for the
file transfer. The loaded script is executed using the *source* command.

.. code-block::

    => loads $scriptaddr
    ## Ready for S-Record download ...

    *** file: script.srec
    $ cat script.srec

    *** exit status: 0 ***

    ## First Load Addr = 0x4FC00000
    ## Last  Load Addr = 0x4FC0005B
    ## Total Size      = 0x0000005C = 92 Bytes
    ## Start Addr      = 0x00000000
    => source $scriptaddr
    ## Executing script at 4fc00000
    hello s-record
    =>

Configuration
-------------

The command is only available if CONFIG_CMD_LOADS=y. The parameter to set the
baud rate is only available if CONFIG_SYS_LOADS_BAUD_CHANGE=y

Return value
------------

The return value $? is 0 (true) on success, 1 (false) otherwise.
