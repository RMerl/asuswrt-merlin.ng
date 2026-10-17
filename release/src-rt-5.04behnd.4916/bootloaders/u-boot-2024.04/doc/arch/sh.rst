.. SPDX-License-Identifier: GPL-2.0+
.. Copyright (c) 2007,2008 Nobuhiro Iwamatsu <iwamatsu@nigaur.org>

SuperH
======

What's this?
------------
This file contains status information for the port of U-Boot to the
Renesas SuperH series of CPUs.

Overview
--------
SuperH has an original boot loader. However, source code is dirty, and
maintenance is not done. To improve sharing and the maintenance of the code,
Nobuhiro Iwamatsu started the porting to U-Boot in 2007.

Supported CPUs
--------------

Renesas SH7750/SH7750R
^^^^^^^^^^^^^^^^^^^^^^
This CPU has the SH4 core.

Renesas SH7722
^^^^^^^^^^^^^^
This CPU has the SH4AL-DSP core.

Supported Boards
----------------

Hitachi UL MS7750SE01/MS7750RSE01
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Board specific code is in board/ms7750se
To use this board, type "make ms7750se_config".
Support devices are:

   - SCIF
   - SDRAM
   - NOR Flash
   - Marubun PCMCIA

Hitachi UL MS7722SE01
^^^^^^^^^^^^^^^^^^^^^
Board specific code is in board/ms7722se
To use this board, type "make ms7722se_config".
Support devices are:

   - SCIF
   - SDRAM
   - NOR Flash
   - Marubun PCMCIA
   - SMC91x ethernet

Hitachi UL MS7720ERP01
^^^^^^^^^^^^^^^^^^^^^^
Board specific code is in board/ms7720se
To use this board, type "make ms7720se_config".
Support devices are:

   - SCIF
   - SDRAM
   - NOR Flash
   - Marubun PCMCIA

In SuperH, S-record and binary of made u-boot work on the memory.
When u-boot is written in the flash, it is necessary to change the
address by using 'objcopy'::

   ex) shX-linux-objcopy -Ibinary -Osrec u-boot.bin u-boot.flash.srec

Compiler
--------
You can use the following of u-boot to compile.
   - `SuperH Linux Open site <http://www.superh-linux.org/>`_
   - `KPIT GNU tools <http://www.kpitgnutools.com/>`_

Future
------
I plan to support the following CPUs and boards.

CPUs
^^^^
- SH7751R(SH4)

Boards
^^^^^^
Many boards ;-)
