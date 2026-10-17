/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015 Timesys Corporation
 * Copyright (C) 2015 General Electric Company
 * Copyright (C) 2014 Advantech
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the GE MX6Q Bx50v3 boards.
 */

#ifndef __GE_BX50V3_CONFIG_H
#define __GE_BX50V3_CONFIG_H

#include <asm/arch/imx-regs.h>
#include <asm/mach-imx/gpio.h>

#include "mx6_common.h"
#include <linux/sizes.h>

#ifdef CONFIG_CMD_NFS
#define NETWORKBOOT \
        "setnetworkboot=" \
                "setenv ipaddr 172.16.2.10; setenv serverip 172.16.2.20; " \
                "setenv gatewayip 172.16.2.20; setenv nfsserver 172.16.2.20; " \
                "setenv netmask 255.255.255.0; setenv ethaddr ca:fe:de:ca:f0:11; " \
                "setenv bootargs root=/dev/nfs nfsroot=${nfsserver}:/srv/nfs/,v3,tcp rw rootwait" \
                "setenv bootargs $bootargs ip=${ipaddr}:${nfsserver}:${gatewayip}:${netmask}::eth0:off " \
                "setenv bootargs $bootargs cma=128M bootcause=${bootcause} ${videoargs} " \
                "setenv bootargs $bootargs systemd.mask=helix-network-defaults.service " \
                "setenv bootargs $bootargs watchdog.handle_boot_enabled=1\0" \
        "networkboot=" \
                "run setnetworkboot; " \
                "nfs ${loadaddr} /srv/nfs/fitImage; " \
                "bootm ${loadaddr}\0" \

#define NETWORKBOOTCOMMAND \
	"run networkboot; " \

#else
#define NETWORKBOOT \

#endif

#define CFG_EXTRA_ENV_SETTINGS \
	NETWORKBOOT \
	"image=/boot/fitImage\0" \
	"dev=mmc\0" \
	"devnum=2\0" \
	"rootdev=mmcblk0p\0" \
	"quiet=quiet loglevel=0\0" \
	"setargs=setenv bootargs root=/dev/${rootdev}${partnum} " \
		"ro rootwait cma=128M " \
		"bootcause=${bootcause} " \
		"${quiet} " \
		"${videoargs}" "\0" \
	"doquiet=" \
		"if ext2load ${dev} ${devnum}:5 0x7000A000 /boot/console; " \
			"then setenv quiet; fi\0" \
	"hasfirstboot=" \
		"test -e ${dev} ${devnum}:${partnum} /boot/bootcause/firstboot\0" \
	"swappartitions=" \
		"setexpr partnum 3 - ${partnum}\0" \
	"failbootcmd=" \
		"cls; " \
		"setcurs 5 4; " \
		"lcdputs \"Monitor failed to start. " \
		"Try again, or contact GE Service for support.\"; " \
		"bootcount reset; " \
		"while true; do sleep 1; done; \0" \
	"altbootcmd=" \
		"run doquiet; " \
		"setenv partnum 1; run hasfirstboot || setenv partnum 2; " \
		"run hasfirstboot || setenv partnum 0; " \
		"if test ${partnum} != 0; then " \
			"run swappartitions loadimage doboot; " \
		"fi; " \
		"run failbootcmd\0" \
	"loadimage=" \
		"ext2load ${dev} ${devnum}:${partnum} ${loadaddr} ${image}\0" \
	"doboot=" \
		"echo Booting from ${dev}:${devnum}:${partnum} ...; " \
		"run setargs; " \
		"bootm ${loadaddr}\0" \
	"tryboot=" \
		"setenv partnum 1; run hasfirstboot || setenv partnum 2; " \
		"run loadimage || run swappartitions && run loadimage || " \
			"setenv partnum 0 && echo MISSING IMAGE;" \
		"run doboot; " \
		"run failbootcmd\0" \


/* Physical Memory Map */
#define PHYS_SDRAM                     MMDC0_ARB_BASE_ADDR

#define CFG_SYS_BOOTMAPSZ (256 << 20)     /* 256M */

#define CFG_SYS_SDRAM_BASE          PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR       IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE       IRAM_SIZE

/* environment organization */

#define CFG_SYS_FSL_USDHC_NUM	3

#endif	/* __GE_BX50V3_CONFIG_H */
