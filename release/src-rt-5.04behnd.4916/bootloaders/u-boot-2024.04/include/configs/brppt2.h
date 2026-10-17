/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Config file for BuR BRPP2_IMX6 board
 *
 * Copyright (C) 2018
 * B&R Industrial Automation GmbH - http://www.br-automation.com/
 */
#ifndef __CONFIG_BRPP2_IMX6_H
#define __CONFIG_BRPP2_IMX6_H

#include <configs/bur_cfg_common.h>
#include <asm/arch/imx-regs.h>

/* -- i.mx6 specifica -- */
#ifndef CONFIG_SYS_L2CACHE_OFF
#define CFG_SYS_PL310_BASE		L2_PL310_BASE
#endif /* !CONFIG_SYS_L2CACHE_OFF */

/* MMC */

/* Boot */

/* misc */

/* Environment */

#define CFG_EXTRA_ENV_SETTINGS \
BUR_COMMON_ENV \
"cfgaddr=0x106F0000\0" \
"scraddr=0x10700000\0" \
"loadaddr=0x10800000\0" \
"dtbaddr=0x12000000\0" \
"ramaddr=0x12100000\0" \
"cfgscr=mw ${loadaddr} 0 128\0" \
"cfgscrl=fdt addr ${dtbaddr} &&"\
" sf probe; sf read ${cfgaddr} 0x40000 0x10000 && source ${cfgaddr}\0" \
"console=ttymxc0,115200n8 consoleblank=0 quiet\0" \
"t50args#0=setenv bootargs b_mode=${b_mode} console=${console} " \
	" root=/dev/mmcblk0p2 rootfstype=ext4 rootwait panic=2 \0" \
"b_t50lgcy#0=" \
	"load ${loaddev}:2 ${loadaddr} /boot/zImage && " \
	"load ${loaddev}:2 ${dtbaddr} /boot/imx6dl-brppt50.dtb; " \
	"run t50args#0; run cfgscrl; bootz ${loadaddr} - ${dtbaddr}\0" \
"t50args#1=setenv bootargs console=${console} b_mode=${b_mode}" \
	" rootwait panic=2\0" \
"b_t50lgcy#1=" \
	"load ${loaddev}:1 ${loadaddr} zImage && " \
	"load ${loaddev}:1 ${dtbaddr} imx6dl-brppt50.dtb && " \
	"load ${loaddev}:1 ${ramaddr} rootfsPPT50.uboot && " \
	"run t50args#1; run cfgscrl; bootz ${loadaddr} ${ramaddr} ${dtbaddr}\0"\
"b_mmc0=load ${loaddev}:1 ${scraddr} bootscr.img && source ${scraddr}\0" \
"b_mmc1=load ${loaddev}:1 ${scraddr} /boot/bootscr.img && source ${scraddr}\0" \
"b_usb0=usb start && load usb 0 ${scraddr} bootscr.img && source ${scraddr}\0" \
"b_net=tftp ${scraddr} netscript.img && source ${scraddr}\0" \
"b_tgts_std=mmc0 mmc1 t50lgcy#0 t50lgcy#1 usb0 net\0" \
"b_tgts_rcy=t50lgcy#1 usb0 net\0" \
"b_tgts_pme=net usb0 mmc0 mmc1\0" \
"b_mode=4\0" \
"b_break=0\0" \
"b_deftgts=if test ${b_mode} = 12; then setenv b_tgts ${b_tgts_pme};" \
" elif test ${b_mode} = 0; then setenv b_tgts ${b_tgts_rcy};" \
" else setenv b_tgts ${b_tgts_std}; fi\0" \
"b_default=run b_deftgts; for target in ${b_tgts};"\
" do echo \"### booting ${target} ###\"; run b_${target};" \
" if test ${b_break} = 1; then; exit; fi; done\0" \
"loaddev=mmc 0\0" \
"altbootcmd=setenv b_mode 0; run b_default;\0" \
"bootlimit=1\0" \
"net2nor=sf probe && dhcp &&" \
" tftp ${loadaddr} SPL && sf erase 0 +${filesize} &&" \
" sf write ${loadaddr} 400 ${filesize} &&" \
" tftp ${loadaddr} u-boot-dtb.img && sf erase 0x100000 +${filesize} &&" \
" sf write ${loadaddr} 0x100000 ${filesize}\0"

/* RAM */
#define PHYS_SDRAM_1			MMDC0_ARB_BASE_ADDR
#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CFG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	IRAM_SIZE

/* USB Configs */
#define CFG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)

#endif	/* __CONFIG_BRPP2_IMX6_H */
