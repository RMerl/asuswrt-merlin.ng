/*
 * (C) Copyright 2008
 * Texas Instruments, <www.ti.com>
 * Syed Mohammed Khasim <khasim@ti.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation's version 2 of
 * the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef MMC_HOST_DEF_H
#define MMC_HOST_DEF_H

#include <asm/omap_mmc.h>

/* T2 Register definitions */
#define T2_BASE			0x48002000

typedef struct t2 {
	unsigned char res1[0x274];	/* 0x000 */
	unsigned int devconf0;		/* 0x274 */
	unsigned char res2[0x060];	/* 0x278 */
	unsigned int devconf1;		/* 0x2D8 */
	unsigned char res3[0x16C];	/* 0x2DC */
	unsigned int ctl_prog_io1;	/* 0x448 */
	unsigned char res4[0x0D4];	/* 0x44C */
	unsigned int pbias_lite;	/* 0x520 */
} t2_t;

#define MMCSDIO1ADPCLKISEL		(1 << 24)
#define MMCSDIO2ADPCLKISEL		(1 << 6)

#define EN_MMC1				(1 << 24)
#define EN_MMC2				(1 << 25)
#define EN_MMC3				(1 << 30)

#define PBIASLITEPWRDNZ0		(1 << 1)
#define PBIASSPEEDCTRL0			(1 << 2)
#define PBIASLITEPWRDNZ1		(1 << 9)
#define PBIASLITEVMODE1			(1 << 8)
#define PBIASLITEVMODE0			(1 << 0)

#define CTLPROGIO1SPEEDCTRL		(1 << 20)

/*
 * OMAP HSMMC register definitions
 */
#define OMAP_HSMMC1_BASE	0x4809C000
#define OMAP_HSMMC2_BASE	0x480B4000
#define OMAP_HSMMC3_BASE	0x480AD000


#endif /* MMC_HOST_DEF_H */
