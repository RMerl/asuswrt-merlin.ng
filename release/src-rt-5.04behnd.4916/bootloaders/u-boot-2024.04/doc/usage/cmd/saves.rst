.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: saves (command)

saves command
=============

Synopsis
--------

::

    saves [offset [size [baud]]]

Description
-----------

The *saves* command is used to transfer a file from the device via the serial
line using the Motorola S-record file format.

offset
    start address of memory area to save, defaults to 0x0

size
    size of memory area to save, defaults to 0x0

baud
    baud rate to use for upload. This parameter is only available if
    CONFIG_SYS_LOADS_BAUD_CHANGE=y

Example
-------

In the example the *screen* command is used to connect to the U-Boot serial
console.

In a first screen session a file is loaded from the SD-card and the *saves*
command is invoked. <CTRL+A><k> is used to kill the screen session.

A new screen session is started which logs the output to a file and the
<ENTER> key is hit to start the file output. <CTRL+A><k> is issued to kill the
screen session.

The log file is converted to a binary file using the *srec_cat* command.
A negative offset of -1337982976 (= -0x4c000000) is applied to compensate for
the offset used in the *saves* command.

.. code-block::

    $ screen /dev/ttyUSB0 115200
    => echo $scriptaddr
    0x4FC00000
    => load mmc 0:1 $scriptaddr boot.txt
    124 bytes read in 1 ms (121.1 KiB/s)
    => saves $scriptaddr $filesize
    ## Ready for S-Record upload, press ENTER to proceed ...
    Really kill this window [y/n]
    $ screen -Logfile out.srec -L /dev/ttyUSB0 115200
    S0030000FC
    S3154FC00000736574656E76206175746F6C6F616420AD
    S3154FC000106E6F0A646863700A6C6F6164206D6D633E
    S3154FC0002020303A3120246664745F616464725F72B3
    S3154FC00030206474620A6C6F6164206D6D6320303AC0
    S3154FC000403120246B65726E656C5F616464725F72DA
    S3154FC0005020736E702E6566690A626F6F74656669C6
    S3154FC0006020246B65726E656C5F616464725F7220CB
    S3114FC00070246664745F616464725F720A38
    S70500000000FA
    ## S-Record upload complete
    =>
    Really kill this window [y/n]
    $ srec_cat out.srec -offset -1337982976 -Output out.txt -binary 2>/dev/null
    $ cat out.txt
    setenv autoload no
    dhcp
    load mmc 0:1 $fdt_addr_r dtb
    load mmc 0:1 $kernel_addr_r snp.efi
    bootefi $kernel_addr_r $fdt_addr_r
    $

Configuration
-------------

The command is only available if CONFIG_CMD_SAVES=y. The parameter to set the
baud rate is only available if CONFIG_SYS_LOADS_BAUD_CHANGE=y

Return value
------------

The return value $? is 0 (true) on success, 1 (false) otherwise.
