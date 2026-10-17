/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * JZ4780 definitions
 *
 * Copyright (c) 2013 Imagination Technologies
 * Author: Paul Burton <paul.burton@imgtec.com>
 */

#ifndef __JZ4780_H__
#define __JZ4780_H__

/* AHB0 BUS Devices */
#define DDRC_BASE	0xb3010000

/* AHB2 BUS Devices */
#define NEMC_BASE	0xb3410000
#define BCH_BASE	0xb34d0000

/* APB BUS Devices */
#define CPM_BASE	0xb0000000
#define TCU_BASE	0xb0002000
#define WDT_BASE	0xb0002000
#define GPIO_BASE	0xb0010000
#define UART0_BASE	0xb0030000
#define UART1_BASE	0xb0031000
#define UART2_BASE	0xb0032000
#define UART3_BASE	0xb0033000
#define MSC0_BASE	0xb3450000
#define MSC1_BASE	0xb3460000
#define MSC2_BASE	0xb3470000

/*
 * GPIO
 */
/* n = 0,1,2,3,4,5 */
#define GPIO_PXPIN(n)	(0x00 + (n) * 0x100)
#define GPIO_PXINT(n)	(0x10 + (n) * 0x100)
#define GPIO_PXINTS(n)	(0x14 + (n) * 0x100)
#define GPIO_PXINTC(n)	(0x18 + (n) * 0x100)
#define GPIO_PXMASK(n)	(0x20 + (n) * 0x100)
#define GPIO_PXMASKS(n)	(0x24 + (n) * 0x100)
#define GPIO_PXMASKC(n)	(0x28 + (n) * 0x100)
#define GPIO_PXPAT1(n)	(0x30 + (n) * 0x100)
#define GPIO_PXPAT1S(n)	(0x34 + (n) * 0x100)
#define GPIO_PXPAT1C(n)	(0x38 + (n) * 0x100)
#define GPIO_PXPAT0(n)	(0x40 + (n) * 0x100)
#define GPIO_PXPAT0S(n)	(0x44 + (n) * 0x100)
#define GPIO_PXPAT0C(n)	(0x48 + (n) * 0x100)
#define GPIO_PXFLG(n)	(0x50 + (n) * 0x100)
#define GPIO_PXFLGC(n)	(0x54 + (n) * 0x100)
#define GPIO_PXOEN(n)	(0x60 + (n) * 0x100)
#define GPIO_PXOENS(n)	(0x64 + (n) * 0x100)
#define GPIO_PXOENC(n)	(0x68 + (n) * 0x100)
#define GPIO_PXPEN(n)	(0x70 + (n) * 0x100)
#define GPIO_PXPENS(n)	(0x74 + (n) * 0x100)
#define GPIO_PXPENC(n)	(0x78 + (n) * 0x100)
#define GPIO_PXDS(n)	(0x80 + (n) * 0x100)
#define GPIO_PXDSS(n)	(0x84 + (n) * 0x100)
#define GPIO_PXDSC(n)	(0x88 + (n) * 0x100)

/* PLL setup */
#define JZ4780_SYS_EXTAL	48000000
#define JZ4780_SYS_MEM_SPEED	(1200 * 1000000)
#define JZ4780_SYS_MEM_DIV	3
#define JZ4780_SYS_AUDIO_SPEED	(768 * 1000000)

#define JZ4780_APLL_M	1
#define JZ4780_APLL_N	1
#define JZ4780_APLL_OD	1

#define JZ4780_MPLL_M	(JZ4780_SYS_MEM_SPEED / JZ4780_SYS_EXTAL * 2)
#define JZ4780_MPLL_N	2
#define JZ4780_MPLL_OD	1

#define JZ4780_EPLL_M	(JZ4780_SYS_AUDIO_SPEED * 2 / JZ4780_SYS_EXTAL)
#define JZ4780_EPLL_N	1
#define JZ4780_EPLL_OD	2

#define JZ4780_VPLL_M	((888 * 1000000) * 2 / JZ4780_SYS_EXTAL)
#define JZ4780_VPLL_N	1
#define JZ4780_VPLL_OD	2

#ifndef __ASSEMBLY__

u32 sdram_size(int bank);

const u32 jz4780_clk_get_efuse_clk(void);
void jz4780_clk_ungate_ethernet(void);
void jz4780_clk_ungate_mmc(void);
void jz4780_clk_ungate_uart(const unsigned int uart);

void jz4780_efuse_read(size_t addr, size_t count, u8 *buf);
void jz4780_efuse_init(u32 ahb2_rate);

void jz4780_tcu_wdt_start(void);

#ifdef CONFIG_SPL_BUILD
int jz_mmc_init(void __iomem *base);
#endif

#endif /* __ASSEMBLY__ */

#endif	/* __JZ4780_H__ */
