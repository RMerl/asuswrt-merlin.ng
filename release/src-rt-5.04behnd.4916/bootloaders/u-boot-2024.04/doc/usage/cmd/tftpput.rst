.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: tftpput (command)

tftpput command
===============

Synopsis
--------

::

    tftpput address size [[hostIPaddr:]filename]

Description
-----------

The tftpput command is used to transfer a file to a TFTP server.

By default the destination port is 69 and the source port is pseudo-random.
If CONFIG_TFTP_PORT=y, the environment variable *tftpsrcp* can be used to set
the source port and the environment variable *tftpdstp* can be used to set
the destination port.

address
    memory address where the data starts

size
    number of bytes to be transferred

hostIPaddr
    IP address of the TFTP server, defaults to the value of environment
    variable *serverip*

filename
    path of the file to be written. If not provided, the client's IP address is
    used to construct a default file name, e.g. C0.A8.00.28.img for IP address
    192.168.0.40.

Example
-------

In the example the following steps are executed:

* setup client network address
* load a file from the SD-card
* send the file via TFTP to a server

::

    => setenv autoload no
    => dhcp
    BOOTP broadcast 1
    DHCP client bound to address 192.168.1.40 (7 ms)
    => load mmc 0:1 $loadaddr test.txt
    260096 bytes read in 13 ms (19.1 MiB/s)
    => tftpput $loadaddr $filesize 192.168.1.3:upload/test.txt
    Using ethernet@1c30000 device
    TFTP to server 192.168.1.3; our IP address is 192.168.1.40
    Filename 'upload/test.txt'.
    Save address: 0x42000000
    Save size:    0x3f800
    Saving: #################
         4.4 MiB/s
    done
    Bytes transferred = 260096 (3f800 hex)
    =>

Configuration
-------------

The command is only available if CONFIG_CMD_TFTPPUT=y.

CONFIG_TFTP_BLOCKSIZE defines the size of the TFTP blocks sent. It defaults
to 1468 matching an ethernet MTU of 1500.

If CONFIG_TFTP_PORT=y, the environment variables *tftpsrcp* and *tftpdstp* can
be used to set the source and the destination ports.

CONFIG_TFTP_WINDOWSIZE can be used to set the TFTP window size of transmits
after which an ACK response is required. The window size defaults to 1.

If CONFIG_TFTP_TSIZE=y, the progress bar is limited to 50 '#' characters.
Otherwise an '#' is written per UDP package which may decrease performance.

Return value
------------

The return value $? is 0 (true) on success and 1 (false) otherwise.
