/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 Texas Instruments Incorporated - https://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 */
#ifndef _ASM_ARCH_HARDWARE_H_
#define _ASM_ARCH_HARDWARE_H_

#include <asm/io.h>

#ifdef CONFIG_SOC_K3_AM654
#include "am6_hardware.h"
#endif

#ifdef CONFIG_SOC_K3_J721E
#include "j721e_hardware.h"
#endif

#ifdef CONFIG_SOC_K3_J721S2
#include "j721s2_hardware.h"
#endif

#ifdef CONFIG_SOC_K3_AM642
#include "am64_hardware.h"
#endif

#ifdef CONFIG_SOC_K3_AM625
#include "am62_hardware.h"
#endif

#ifdef CONFIG_SOC_K3_AM62A7
#include "am62a_hardware.h"
#endif

/* Assuming these addresses and definitions stay common across K3 devices */
#define CTRLMMR_WKUP_JTAG_ID	(WKUP_CTRL_MMR0_BASE + 0x14)
#define JTAG_ID_VARIANT_SHIFT	28
#define JTAG_ID_VARIANT_MASK	(0xf << 28)
#define JTAG_ID_PARTNO_SHIFT	12
#define JTAG_ID_PARTNO_MASK	(0xffff << 12)
#define JTAG_ID_PARTNO_AM65X	0xbb5a
#define JTAG_ID_PARTNO_J721E	0xbb64
#define JTAG_ID_PARTNO_J7200	0xbb6d
#define JTAG_ID_PARTNO_AM64X	0xbb38
#define JTAG_ID_PARTNO_J721S2	0xbb75
#define JTAG_ID_PARTNO_AM62X	0xbb7e
#define JTAG_ID_PARTNO_AM62AX   0xbb8d

#define K3_SOC_ID(id, ID) \
static inline bool soc_is_##id(void) \
{ \
	u32 soc = (readl(CTRLMMR_WKUP_JTAG_ID) & \
		JTAG_ID_PARTNO_MASK) >> JTAG_ID_PARTNO_SHIFT; \
	return soc == JTAG_ID_PARTNO_##ID; \
}
K3_SOC_ID(am65x, AM65X)
K3_SOC_ID(j721e, J721E)
K3_SOC_ID(j7200, J7200)
K3_SOC_ID(am64x, AM64X)
K3_SOC_ID(j721s2, J721S2)
K3_SOC_ID(am62x, AM62X)
K3_SOC_ID(am62ax, AM62AX)

#define K3_SEC_MGR_SYS_STATUS		0x44234100
#define SYS_STATUS_DEV_TYPE_SHIFT	0
#define SYS_STATUS_DEV_TYPE_MASK	(0xf)
#define SYS_STATUS_DEV_TYPE_GP		0x3
#define SYS_STATUS_DEV_TYPE_TEST	0x5
#define SYS_STATUS_DEV_TYPE_EMU		0x9
#define SYS_STATUS_DEV_TYPE_HS		0xa
#define SYS_STATUS_SUB_TYPE_SHIFT	8
#define SYS_STATUS_SUB_TYPE_MASK	(0xf << 8)
#define SYS_STATUS_SUB_TYPE_VAL_FS	0xa

/*
 * The CTRL_MMR0 memory space is divided into several equally-spaced
 * partitions, so defining the partition size allows us to determine
 * register addresses common to those partitions.
 */
#define CTRL_MMR0_PARTITION_SIZE		0x4000

/*
 * CTRL_MMR0, WKUP_CTRL_MMR0, and MCU_CTRL_MMR0 lock/kick-mechanism
 * shared register definitions. The same registers are also used for
 * PADCFG_MMR lock/kick-mechanism.
 */
#define CTRLMMR_LOCK_KICK0			0x1008
#define CTRLMMR_LOCK_KICK0_UNLOCK_VAL		0x68ef3490
#define CTRLMMR_LOCK_KICK1			0x100c
#define CTRLMMR_LOCK_KICK1_UNLOCK_VAL		0xd172bc5a

#define K3_ROM_BOOT_HEADER_MAGIC	"EXTBOOT"

struct rom_extended_boot_data {
	char header[8];
	u32 num_components;
};

#endif /* _ASM_ARCH_HARDWARE_H_ */
