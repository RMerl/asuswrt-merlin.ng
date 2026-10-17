/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef __BCM_NS_H
#define __BCM_NS_H

#include <linux/sizes.h>

/* Physical Memory Map */
#define V2M_BASE			0x00000000
#define PHYS_SDRAM_1			V2M_BASE

#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM_1

/* Called "periph_clk" in Linux, used by the global timer */
#define CFG_SYS_HZ_CLOCK		500000000

/* Called "iprocslow" in Linux */
#define CFG_SYS_NS16550_CLK		125000000

/* console configuration */
#define CONSOLE_ARGS "console_args=console=ttyS0,115200n8\0"
#define MAX_CPUS "max_cpus=maxcpus=2\0"
#define EXTRA_ARGS "extra_args=earlycon=uart8250,mmio32,0x18000300\0"

#define BASE_ARGS "${console_args} ${extra_args} ${pcie_args}"	\
		  " ${max_cpus}  ${log_level} ${reserved_mem}"
#define SETBOOTARGS "setbootargs=setenv bootargs " BASE_ARGS "\0"

#define KERNEL_LOADADDR_CFG \
	"loadaddr=0x01000000\0" \
	"dtb_loadaddr=0x02000000\0"

/*
 * Hardcoded for the only boards we support, if you add more
 * boards, add a more clever bootcmd!
 */
#define NS_BOOTCMD "bootcmd_dlink_dir8xxl=seama 0x00fe0000; go 0x01000000"

#define ARCH_ENV_SETTINGS \
	CONSOLE_ARGS \
	MAX_CPUS \
	EXTRA_ARGS \
	KERNEL_LOADADDR_CFG \
	NS_BOOTCMD

#define CFG_EXTRA_ENV_SETTINGS \
	ARCH_ENV_SETTINGS

#endif /* __BCM_NS_H */
