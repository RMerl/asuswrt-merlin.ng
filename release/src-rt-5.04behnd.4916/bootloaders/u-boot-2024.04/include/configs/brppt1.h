/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * brtpp1.h
 *
 * specific parts for B&R T-Series Motherboard
 *
 * Copyright (C) 2013 Hannes Schmelzer <oe5hpm@oevsv.at> -
 * Bernecker & Rainer Industrieelektronik GmbH - http://www.br-automation.com
 */

#ifndef __CONFIG_BRPPT1_H__
#define __CONFIG_BRPPT1_H__

#include <configs/bur_cfg_common.h>
#include <configs/bur_am335x_common.h>
#include <linux/stringify.h>
/* ------------------------------------------------------------------------- */
/* memory */

/* Clock Defines */
#define V_OSCK				26000000  /* Clock output from T2 */
#define V_SCLK				(V_OSCK)

/*
 * When we have NAND flash we expect to be making use of mtdparts,
 * both for ease of use in U-Boot and for passing information on to
 * the Linux kernel.
 */

#define MMC_TGTS \
"t30args#0=setenv bootargs ${optargs_rot} ${optargs} console=${console} " \
	"b_mode=${b_mode} root=${root_dev} rootfstype=ext4 rootwait\0" \
"b_t30lgcy#0=" \
	"load ${loaddev}:2 ${loadaddr} /boot/zImage && " \
	"run load_dtb && " \
	"run t30args#0; run cfgscr; bootz ${loadaddr} - ${dtbaddr}\0" \
"t30args#1=setenv bootargs ${optargs_rot} ${optargs} console=${console} " \
	"b_mode=${b_mode}\0" \
"b_t30lgcy#1=" \
	"load ${loaddev}:1 ${loadaddr} zImage && " \
	"load ${loaddev}:1 ${dtbaddr} am335x-brppt30.dtb && " \
	"load ${loaddev}:1 ${ramaddr} rootfsPPT30.uboot && " \
	"run t30args#1; run cfgscr; bootz ${loadaddr} ${ramaddr} ${dtbaddr}\0" \
"b_mmc0=load ${loaddev}:1 ${scraddr} bootscr.img && source ${scraddr}\0" \
"b_mmc1=load ${loaddev}:1 ${scraddr} /boot/bootscr.img && source ${scraddr}\0" \
"b_tgts_std=mmc0 mmc1 t30lgcy#0 t30lgcy#1 usb0 net\0" \
"b_tgts_rcy=t30lgcy#1 usb0 net\0" \
"b_tgts_pme=net usb0 mmc0 mmc1\0" \
"loaddev=mmc 1\0" \
"root_dev=/dev/mmcblk0p2\0" \
"load_dtb=load ${loaddev}:2 ${dtbaddr} /boot/am335x-brppt30.dtb; " \
	 "if test $? -eq 0; then " \
	     "setenv root_dev /dev/mmcblk1p2; " \
	 "else; " \
	     "load ${loaddev}:1 ${dtbaddr} am335x-brppt30-legacy.dtb; " \
	 "fi;\0"

#ifdef CONFIG_ENV_IS_IN_MMC
#define MMCTGTS \
MMC_TGTS \
"cfgscr=mw ${cfgaddr} 0;" \
" mmc dev 1; mmc read ${cfgaddr} 200 80; source ${cfgaddr};" \
" fdt addr ${dtbaddr} || cp ${fdtcontroladdr} ${dtbaddr} 4000\0"
#else
#define MMCTGTS ""
#endif /* CONFIG_MMC */

#define LOAD_OFFSET(x)			0x8##x

#define CFG_EXTRA_ENV_SETTINGS \
BUR_COMMON_ENV \
"verify=no\0" \
"scraddr=" __stringify(LOAD_OFFSET(0000000)) "\0" \
"cfgaddr=" __stringify(LOAD_OFFSET(0020000)) "\0" \
"dtbaddr=" __stringify(LOAD_OFFSET(0040000)) "\0" \
"loadaddr=" __stringify(LOAD_OFFSET(0100000)) "\0" \
"ramaddr=" __stringify(LOAD_OFFSET(2000000)) "\0" \
"console=ttyO0,115200n8\0" \
"optargs=consoleblank=0 quiet panic=2\0" \
"b_break=0\0" \
"b_usb0=usb start && load usb 0 ${scraddr} bootscr.img && source ${scraddr}\0" \
"b_net=tftp ${scraddr} netscript.img && source ${scraddr}\0" \
MMCTGTS \
"b_deftgts=if test ${b_mode} = 12; then setenv b_tgts ${b_tgts_pme};" \
" elif test ${b_mode} = 0; then setenv b_tgts ${b_tgts_rcy};" \
" else setenv b_tgts ${b_tgts_std}; fi\0" \
"b_default=run b_deftgts; for target in ${b_tgts};"\
" do echo \"### booting ${target} ###\"; run b_${target};" \
" if test ${b_break} = 1; then; exit; fi; done\0"

#endif	/* ! __CONFIG_BRPPT1_H__ */
