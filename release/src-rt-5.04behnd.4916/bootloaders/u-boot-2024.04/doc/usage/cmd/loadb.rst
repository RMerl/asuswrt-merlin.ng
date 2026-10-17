.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: loadb (command)

loadb command
=============

Synopsis
--------

::

    loadb [addr [baud]]

Description
-----------

The loadb command is used to transfer a file to the device via the serial line
using the Kermit protocol.

The number of transferred bytes is saved in environment variable filesize.

addr
    load address, defaults to environment variable loadaddr or if loadaddr is
    not set to configuration variable CONFIG_SYS_LOAD_ADDR

baud
    baud rate for the Kermit transmission. After the transmission the baud
    rate is reset to the original value.

Example
-------

In the example below the terminal emulation program picocom and G-Kermit
serve to transfer a file to a device.

.. code-block:: bash

    picocom --baud 115200 --send-cmd "gkermit -iXvs" /dev/ttyUSB0

After entering the loadb command the key sequence <CTRL-A><CTRL-S> is used to
let picocom prompt for the file name. Picocom invokes G-Kermit for the file
transfer.

::

    => loadb 60800000 115200
    ## Ready for binary (kermit) download to 0x60800000 at 115200 bps...

    *** file: helloworld.efi
    $ gkermit -iXvs helloworld.efi
    G-Kermit 2.01, The Kermit Project, 2021-11-15
    Escape back to your local Kermit and give a RECEIVE command.

    KERMIT READY TO SEND...
    |
    *** exit status: 0 ***
    ## Total Size      = 0x00000c00 = 3072 Bytes
    ## Start Addr      = 0x60800000
    =>

The transfer can be cancelled by pressing <CTRL+C>.

Configuration
-------------

The command is only available if CONFIG_CMD_LOADB=y.

Return value
------------

The return value $? is 0 (true) on success, 1 (false) on error.
