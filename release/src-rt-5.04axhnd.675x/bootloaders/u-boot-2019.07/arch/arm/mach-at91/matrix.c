// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Atmel Corporation
 *		      Wenyou Yang <wenyou.yang@atmel.com>
 */

#include <common.h>
#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/arch/sama5_matrix.h>

void matrix_init(void)
{
	struct atmel_matrix *h64mx = (struct atmel_matrix *)ATMEL_BASE_MATRIX0;
	struct atmel_matrix *h32mx = (struct atmel_matrix *)ATMEL_BASE_MATRIX1;
	int i;

	/* DDR port 1 ~ port 7 */
	for (i = H64MX_SLAVE_DDRC_PORT1; i <= H64MX_SLAVE_DDRC_PORT7; i++) {
		writel(0x000f0f0f, &h64mx->ssr[i]);
		writel(0x0000ffff, &h64mx->sassr[i]);
		writel(0x0000000f, &h64mx->srtsr[i]);
	}

	/* EBI CS3 (NANDFlash 128M) and NFC Command Registers(128M) */
	writel(0x00c0c0c0, &h32mx->ssr[H32MX_SLAVE_EBI]);
	writel(0xff000000, &h32mx->sassr[H32MX_SLAVE_EBI]);
	writel(0xff000000, &h32mx->srtsr[H32MX_SLAVE_EBI]);

	/* NFC SRAM */
	writel(0x00010101, &h32mx->ssr[H32MX_SLAVE_NFC_SRAM]);
	writel(0x00000001, &h32mx->sassr[H32MX_SLAVE_NFC_SRAM]);
	writel(0x00000001, &h32mx->srtsr[H32MX_SLAVE_NFC_SRAM]);
}
