/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * ./arch/arm/mach-rmobile/include/mach/rcar-gen3-base.h
 *
 * Copyright (C) 2015 Renesas Electronics Corporation
 */

#ifndef __ASM_ARCH_RCAR_GEN3_BASE_H
#define __ASM_ARCH_RCAR_GEN3_BASE_H

/*
 * R-Car (R8A7750) I/O Addresses
 */
#define RWDT_BASE		0xE6020000
#define SWDT_BASE		0xE6030000
#define LBSC_BASE		0xEE220200
#define TMU_BASE		0xE61E0000
#define GPIO5_BASE		0xE6055000

/* SCIF */
#define SCIF0_BASE		0xE6E60000
#define SCIF1_BASE		0xE6E68000
#define SCIF2_BASE		0xE6E88000
#define SCIF3_BASE		0xE6C50000
#define SCIF4_BASE		0xE6C40000
#define SCIF5_BASE		0xE6F30000

/* Module stop status register */
#define MSTPSR0			0xE6150030
#define MSTPSR1			0xE6150038
#define MSTPSR2			0xE6150040
#define MSTPSR3			0xE6150048
#define MSTPSR4			0xE615004C
#define MSTPSR5			0xE615003C
#define MSTPSR6			0xE61501C0
#define MSTPSR7			0xE61501C4
#define MSTPSR8			0xE61509A0
#define MSTPSR9			0xE61509A4
#define MSTPSR10		0xE61509A8
#define MSTPSR11		0xE61509AC

/* Realtime module stop control register */
#define RMSTPCR0		0xE6150110
#define RMSTPCR1		0xE6150114
#define RMSTPCR2		0xE6150118
#define RMSTPCR3		0xE615011C
#define RMSTPCR4		0xE6150120
#define RMSTPCR5		0xE6150124
#define RMSTPCR6		0xE6150128
#define RMSTPCR7		0xE615012C
#define RMSTPCR8		0xE6150980
#define RMSTPCR9		0xE6150984
#define RMSTPCR10		0xE6150988
#define RMSTPCR11		0xE615098C

/* System module stop control register */
#define SMSTPCR0		0xE6150130
#define SMSTPCR1		0xE6150134
#define SMSTPCR2		0xE6150138
#define SMSTPCR3		0xE615013C
#define SMSTPCR4		0xE6150140
#define SMSTPCR5		0xE6150144
#define SMSTPCR6		0xE6150148
#define SMSTPCR7		0xE615014C
#define SMSTPCR8		0xE6150990
#define SMSTPCR9		0xE6150994
#define SMSTPCR10		0xE6150998
#define SMSTPCR11		0xE615099C

/* PFC */
#define PFC_PUEN5	0xE6060414
#define PUEN_SSI_SDATA4	BIT(17)
#define PFC_PUEN6       0xE6060418
#define PUEN_USB1_OVC   (1 << 2)
#define PUEN_USB1_PWEN  (1 << 1)

/* IICDVFS (I2C) */
#define CONFIG_SYS_I2C_SH_BASE0	0xE60B0000

#ifndef __ASSEMBLY__
#include <asm/types.h>

/* RWDT */
struct rcar_rwdt {
	u32 rwtcnt;
	u32 rwtcsra;
	u32 rwtcsrb;
};

/* SWDT */
struct rcar_swdt {
	u32 swtcnt;
	u32 swtcsra;
	u32 swtcsrb;
};
#endif

#endif /* __ASM_ARCH_RCAR_GEN3_BASE_H */
