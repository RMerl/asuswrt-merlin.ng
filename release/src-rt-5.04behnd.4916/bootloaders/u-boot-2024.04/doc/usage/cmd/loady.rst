.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: loady (command)

loady command
=============

Synopsis
--------

::

    loady [addr [baud]]

Description
-----------

The loady command is used to transfer a file to the device via the serial line
using the YMODEM protocol.

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

After entering the loady command the key sequence <CTRL-A><CTRL-S> is used to
let picocom prompt for the file name. Picocom invokes the program sz for the
file transfer.

::

    => loady 80064000 115200
    ## Ready for binary (ymodem) download to 0x80064000 at 115200 bps...
    C
    *** file: BOOTRISCV64.EFI
    $ sz -b -vv BOOTRISCV64.EFI
    Sending: BOOTRISCV64.EFI
    Bytes Sent: 398976   BPS:7883
    Sending:
    Ymodem sectors/kbytes sent:   0/ 0k
    Transfer complete

    *** exit status: 0 ***
    /1(CAN) packets, 4 retries
    ## Total Size      = 0x0006165f = 398943 Bytes
    => echo ${filesize}
    6165f
    =>

Transfer can be cancelled by pressing 3 times <CTRL+C> after two seconds
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
