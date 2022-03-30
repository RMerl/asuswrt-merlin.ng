
U-Boot for Renesas SuperH
	Last update 01/18/2008 by Nobuhiro Iwamatsu

================================================================================
0. What's this?
	This file contains status information for the port of U-Boot to the
	Renesas SuperH series of CPUs.

================================================================================
1. Overview
	SuperH has an original boot loader. However, source code is dirty, and
	maintenance is not done.
	To improve sharing and the maintenance of the code, Nobuhiro Iwamatsu
	started the porting to u-boot in 2007.

================================================================================
2. Supported CPUs

	2.1. Renesas SH7750/SH7750R
		This CPU has the SH4 core.

	2.2. Renesas SH7722
		This CPU has the SH4AL-DSP core.

	2.3. Renesas SH7780
		This CPU has the SH4A core.

================================================================================
3. Supported Boards

	3.1. Hitachi UL MS7750SE01/MS7750RSE01
		Board specific code is in board/ms7750se
		To use this board, type "make ms7750se_config".
		Support devices are :
			- SCIF
			- SDRAM
			- NOR Flash
			- Marubun PCMCIA

	3.2. Hitachi UL MS7722SE01
		Board specific code is in board/ms7722se
		To use this board, type "make ms7722se_config".
		Support devices are :
			- SCIF
			- SDRAM
			- NOR Flash
			- Marubun PCMCIA
			- SMC91x ethernet

	3.2. Hitachi UL MS7720ERP01
		Board specific code is in board/ms7720se
		To use this board, type "make ms7720se_config".
		Support devices are :
			- SCIF
			- SDRAM
			- NOR Flash
			- Marubun PCMCIA

	3.3. Renesas R7780MP
		Board specific code is in board/r7780mp
		To use this board, type "make r7780mp_config".
		Support devices are :
			- SCIF
			- DDR-SDRAM
			- NOR Flash
			- Compact Flash
			- ASIX ethernet
			- SH7780 PCI bridge
			- RTL8110 ethernet

	** README **
		In SuperH, S-record and binary of made u-boot work on the memory.
		When u-boot is written in the flash, it is necessary to change the
		address by using 'objcopy'.
		ex) shX-linux-objcopy -Ibinary -Osrec u-boot.bin u-boot.flash.srec

================================================================================
4. Compiler
	You can use the following of u-boot to compile.
		- SuperH Linux Open site
			http://www.superh-linux.org/
		- KPIT GNU tools
			http://www.kpitgnutools.com/

================================================================================
5. Future
	I plan to support the following CPUs and boards.
		5.1. CPUs
			- SH7751R(SH4)

		5.2. Boards
			- Many boards ;-)

================================================================================
Copyright (c) 2007,2008
    Nobuhiro Iwamatsu <iwamatsu@nigaur.org>
