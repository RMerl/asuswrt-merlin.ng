/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014 Stefan Roese <sr@denx.de>
 * Copyright (C) 2016 Mario Six <mario.six@gdsys.cc>
 */

#ifndef _CONFIG_CONTROLCENTERDC_H
#define _CONFIG_CONTROLCENTERDC_H

/*
 * TEXT_BASE needs to be below 16MiB, since this area is scrubbed
 * for DDR ECC byte filling in the SPL before loading the main
 * U-Boot into it.
 */

/* Environment in SPI NOR flash */

#define PHY_ANEG_TIMEOUT	8000	/* PHY needs a longer aneg time */

/*
 * Environment Configuration
 */

#define CFG_EXTRA_ENV_SETTINGS						\
	"netdev=eth1\0"						\
	"consoledev=ttyS1\0"							\
	"u-boot=u-boot.bin\0"							\
	"bootfile_addr=1000000\0"						\
	"keyprogram_addr=3000000\0"						\
	"keyprogram_file=keyprogram.img\0"						\
	"fdtfile=controlcenterdc.dtb\0"						\
	"load=tftpboot ${loadaddr} ${u-boot}\0"					\
	"mmcdev=0:2\0"								\
	"update=sf probe 1:0;"							\
		" sf erase 0 +${filesize};"					\
		" sf write ${fileaddr} 0 ${filesize}\0"				\
	"upd=run load update\0"							\
	"fdt_high=0x10000000\0"							\
	"initrd_high=0x10000000\0"						\
	"loadkeyprogram=tpm flush_keys;"					\
		" mmc rescan;"							\
		" ext4load mmc ${mmcdev} ${keyprogram_addr} ${keyprogram_file};"\
		" source ${keyprogram_addr}:script@1\0"				\
	"gpio1=gpio@22_25\0"							\
	"gpio2=A29\0"								\
	"blinkseq='0 0 0 0 2 0 2 2 3 1 3 1 0 0 2 2 3 1 3 3 2 0 2 2 3 1 1 1 "	\
		  "2 0 2 2 3 1 3 1 0 0 2 0 3 3 3 1 2 0 0 0 3 1 1 1 0 0 0 0'\0"	\
	"bootfail=for i in ${blinkseq}; do"					\
		" if test $i -eq 0; then"					\
		" gpio clear ${gpio1}; gpio set ${gpio2};"			\
		" elif test $i -eq 1; then"					\
		" gpio clear ${gpio1}; gpio clear ${gpio2};"			\
		" elif test $i -eq 2; then"					\
		" gpio set ${gpio1}; gpio set ${gpio2};"			\
		" else;"							\
		" gpio clear ${gpio1}; gpio set ${gpio2};"			\
		" fi; sleep 0.12; done\0"

/*
 * mv-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-common.h"

#endif /* _CONFIG_CONTROLCENTERDC_H */
