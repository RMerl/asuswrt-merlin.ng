/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2007
 * DENX Software Engineering, Anatolij Gustschin, agust@denx.de
 */

/*
 * mb862xx.h - Graphic interface for Fujitsu CoralP/Lime
 */

#ifndef _MB862XX_H_
#define _MB862XX_H_

#define PCI_VENDOR_ID_FUJITSU	0x10CF
#define PCI_DEVICE_ID_CORAL_P	0x2019
#define PCI_DEVICE_ID_CORAL_PA	0x201E

#define MB862XX_TYPE_LIME	0x1

#define GC_HOST_BASE		0x01fc0000
#define GC_DISP_BASE		0x01fd0000
#define GC_DRAW_BASE		0x01ff0000

/* Host interface registers */
#define GC_SRST			0x0000002c
#define GC_CCF			0x00000038
#define GC_CID			0x000000f0
#define GC_MMR			0x0000fffc

/*
 * Display Controller registers
 * _A means the offset is aligned, we use these for boards
 * with 8-/16-bit GDC access not working or buggy.
 */
#define GC_DCM0			0x00000000
#define GC_HTP_A		0x00000004
#define GC_HTP			0x00000006
#define GC_HDB_HDP_A		0x00000008
#define GC_HDP			0x00000008
#define GC_HDB			0x0000000a
#define GC_VSW_HSW_HSP_A	0x0000000c
#define GC_HSP			0x0000000c
#define GC_HSW			0x0000000e
#define GC_VSW			0x0000000f
#define GC_VTR_A		0x00000010
#define GC_VTR			0x00000012
#define GC_VDP_VSP_A		0x00000014
#define GC_VSP			0x00000014
#define GC_VDP			0x00000016
#define GC_WY_WX		0x00000018
#define GC_WH_WW		0x0000001c
#define GC_L0M			0x00000020
#define GC_L0OA0		0x00000024
#define GC_L0DA0		0x00000028
#define GC_L0DY_L0DX		0x0000002c
#define GC_L2M			0x00000040
#define GC_L2OA0		0x00000044
#define GC_L2DA0		0x00000048
#define GC_L2OA1		0x0000004c
#define GC_L2DA1		0x00000050
#define GC_L2DX			0x00000054
#define GC_L2DY			0x00000056
#define GC_DCM1			0x00000100
#define GC_DCM2			0x00000104
#define GC_DCM3			0x00000108
#define GC_L0EM			0x00000110
#define GC_L0WY_L0WX		0x00000114
#define GC_L0WH_L0WW		0x00000118
#define GC_L2EM			0x00000130
#define GC_L2WX			0x00000134
#define GC_L2WY			0x00000136
#define GC_L2WW			0x00000138
#define GC_L2WH			0x0000013a
#define GC_L0PAL0		0x00000400

/* Drawing registers */
#define GC_CTR			0x00000400
#define GC_IFCNT		0x00000408
#define GC_FBR			0x00000440
#define GC_XRES			0x00000444
#define GC_CXMIN		0x00000454
#define GC_CXMAX		0x00000458
#define GC_CYMIN		0x0000045c
#define GC_CYMAX		0x00000460
#define GC_FC			0x00000480
#define GC_BC			0x00000484
#define GC_FIFO			0x000004a0
#define GC_REV			0x00008084
#define GC_GEO_FIFO		0x00008400

typedef struct {
	unsigned int index;
	unsigned int value;
} gdc_regs;

int mb862xx_probe(unsigned int addr);
const gdc_regs *board_get_regs (void);
unsigned int board_video_init (void);
void board_backlight_switch(int);

#endif /* _MB862XX_H_ */
