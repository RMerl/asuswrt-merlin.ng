.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: loadx (command)

loadx command
=============

Synopsis
--------

::

    loadx [addr [baud]]

Description
-----------

The loadx command is used to transfer a file to the device via the serial line
using the XMODEM protocol.

The number of transferred bytes is saved in environment variable filesize.

addr
    load address, defaults to environment variable loadaddr or if loadaddr is
    not set to configuration variable CONFIG_SYS_LOAD_ADDR

baud
    baud rate for the ymodem transmission. After the transmission the baud
    rate is reset to the original value.

Example
-------

In the example below the terminal emulation program picocom was used to
transfer a file to the device.

.. code-block::

    picocom --send-cmd 'sx -b vv' --baud 115200 /dev/ttyUSB0

After entering the loadx command the key sequence <CTRL-A><CTRL-S> is used to
let picocom prompt for the file name. Picocom invokes the program sx for the
file transfer.

::

    => loadx 60800000 115200
    ## Ready for binary (xmodem) download to 0x60800000 at 115200 bps...
    C
    *** file: helloworld.efi
    $ sx -b vv helloworld.efi
    sx: cannot open vv: No such file or directory
    Sending helloworld.efi, 24 blocks: Give your local XMODEM receive command now.
    Xmodem sectors/kbytes sent:   0/ 0kRetry 0: NAK on sector
    Bytes Sent:   3072   BPS:1147

    Transfer incomplete

    *** exit status: 1 ***
    ## Total Size      = 0x00000c00 = 3072 Bytes
    ## Start Addr      = 0x60800000
    =>

The transfer can be cancelled by pressing 3 times <CTRL+C> after two seconds
of inactivity on terminal.

Configuration
-------------

The command is only available if CONFIG_CMD_LOADB=y.

Initial timeout in seconds while waiting for transfer is configured by
config option CMD_LOADXY_TIMEOUT or by env variable $loadxy_timeout.
Setting it to 0 means infinite timeout.

Return value
------------

The return value $? is 0 (true) on success, 1 (false) otherwise.
