/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2016
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Based on:
 * am335x_evm.h
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef __CONFIG_AM335X_SHC_H
#define __CONFIG_AM335X_SHC_H

#include <configs/ti_am335x_common.h>

/* settings we don;t want on this board */

/* Clock Defines */
#define V_OSCK				24000000  /* Clock output from T2 */
#define V_SCLK				(V_OSCK)

#ifndef CONFIG_SPL_BUILD
#define CFG_EXTRA_ENV_SETTINGS \
	"loadaddr=0x80200000\0" \
	"kloadaddr=0x84000000\0" \
	"fdtaddr=0x85000000\0" \
	"fdt_high=0xffffffff\0" \
	"rdaddr=0x81000000\0" \
	"bootfile=uImage\0" \
	"fdtfile=am335x-shc.dtb\0" \
	"verify=no\0" \
	"serverip=10.55.152.184\0" \
	"rootpath=/srv/nfs/shc-rootfs\0" \
	"console=ttyO0,115200n8\0" \
	"optargs=quiet\0" \
	"mmcdev=1\0" \
	"harakiri=0\0" \
	"mmcpart=2\0" \
	"active_root=root1\0" \
	"inactive_root=root2\0" \
	"mmcrootfstype=ext4 rootwait\0" \
	"nfsopts=nolock\0" \
	"static_ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}" \
		"::off\0" \
	"ip_method=none\0" \
	"bootargs_defaults=setenv bootargs " \
		"console=${console} " \
		"${optargs}\0" \
	"mmcargs=run bootargs_defaults;" \
		"setenv bootargs ${bootargs} " \
		"root=${mmcroot} " \
		"rootfstype=${mmcrootfstype} ip=${ip_method}\0" \
	"netargs=setenv bootargs console=${console} " \
		"${optargs} " \
		"root=/dev/nfs " \
		"nfsroot=${serverip}:${rootpath},${nfsopts} rw " \
		"ip=dhcp\0" \
	"bootenv=uEnv.txt\0" \
	"loadbootenv=if fatload mmc ${mmcdev} ${loadaddr} ${bootenv}; then " \
			"echo Loaded environment from ${bootenv}; " \
			"run importbootenv; " \
		"fi;\0" \
	"importbootenv=echo Importing environment variables from uEnv.txt ...; " \
		"env import -t $loadaddr $filesize\0" \
	"loaduimagefat=fatload mmc ${mmcdev} ${kloadaddr} ${bootfile}\0" \
	"loaduimage=ext2load mmc ${mmcdev}:${mmcpart} ${kloadaddr} /boot/${bootfile}\0" \
	"loadfdt=ext2load mmc ${mmcdev}:${mmcpart} ${fdtaddr} /boot/${fdtfile}\0" \
	"netloaduimage=tftp ${loadaddr} ${bootfile}\0" \
	"netloadfdt=tftp ${fdtaddr} ${fdtfile}\0" \
	"mmcboot=echo Booting Linux from ${mmcdevice} ...; " \
		"run mmcargs; " \
		"if run loadfdt; then " \
			"echo device tree detected; " \
			"bootm ${kloadaddr} - ${fdtaddr}; " \
		"else " \
			"bootm ${kloadaddr}; " \
		"fi; \0" \
	"netboot=echo Booting from network ...; " \
		"setenv autoload no; " \
		"dhcp; " \
		"run netloaduimage; " \
		"run netargs; " \
		"echo NFS path: ${serverip}:${rootpath};" \
		"if run netloadfdt; then " \
			"echo device tree detected; " \
			"bootm ${loadaddr} - ${fdtaddr}; " \
		"else " \
			"bootm ${loadaddr}; " \
		"fi; \0" \
	"emmc_erase=if test ${harakiri} = 1 ; then echo erase emmc ...; setenv mmcdev 1; mmc erase 0 200; reset; fi; \0" \
	"mmcpart_gp=mmcpart gp 1 40; \0" \
	"mmcpart_enhance=mmcpart enhance 0 64; \0" \
	"mmcpart_rel_write=mmcpart rel_write 1f; \0" \
	"mmcpart_commit=mmcpart commit 1; \0" \
	"mmc_hw_part=run mmcpart_gp; run mmcpart_enhance; run mmcpart_rel_write; run mmcpart_commit; \0" \
	"led_success=gpio set 22; \0" \
	"fusecmd=mmc dev 1; if mmcpart iscommitted; then echo HW Partitioning already committed; mmcpart list; else run mmc_hw_part; fi; run led_success; \0" \
	"uenv_exec=if test -n $uenvcmd; then " \
			"echo Running uenvcmd ...; " \
			"run uenvcmd; " \
		"fi;\0" \
	"sd_setup=echo SD/MMC-Card detected on device 0; " \
		"setenv mmcdevice SD; " \
		"setenv mmcdev 0; " \
		"setenv mmcpart 2; " \
		"setenv mmcroot /dev/mmcblk${mmcdev}p${mmcpart};\0" \
	"emmc_setup=echo eMMC detected on device 1; " \
		"setenv mmcdevice eMMC; " \
		"setenv mmcdev 1; " \
		"run emmc_erase; " \
		"if test ${active_root} = root2; then " \
			"echo Active root is partition 6 (root2); " \
			"setenv mmcpart 6; " \
		"else " \
			"echo Active root is partition 5 (root1); " \
			"setenv mmcpart 5; " \
		"fi; " \
		"setenv mmcroot /dev/mmcblk${mmcdev}p${mmcpart};\0"
#endif /* #ifndef CONFIG_SPL_BUILD */

#if defined CONFIG_SHC_NETBOOT
/* Network Boot */

#elif defined CONFIG_SHC_SDBOOT /* !defined CONFIG_SHC_NETBOOT */
/* SD-Card Boot */

#elif defined CONFIG_SHC_ICT
/* ICT adapter boots only u-boot and does HW partitioning */

#else /* !defined CONFIG_SHC_NETBOOT, !defined CONFIG_SHC_SDBOOT */
/* Regular Boot from internal eMMC */

#endif /* Regular Boot */

/* NS16550 Configuration */
#define CFG_SYS_NS16550_COM1		0x44e09000	/* UART0 */
#define CFG_SYS_NS16550_COM2		0x48022000	/* UART1 */
#define CFG_SYS_NS16550_COM3		0x48024000	/* UART2 */
#define CFG_SYS_NS16550_COM4		0x481a6000	/* UART3 */
#define CFG_SYS_NS16550_COM5		0x481a8000	/* UART4 */
#define CFG_SYS_NS16550_COM6		0x481aa000	/* UART5 */

#endif	/* ! __CONFIG_AM335X_SHC_H */
