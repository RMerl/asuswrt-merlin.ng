/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2009 Freescale Semiconductor, Inc.
 */

#ifndef __BCSR_H_
#define __BCSR_H_

#include <common.h>

/* BCSR Bit definitions*/
/****************************************/
/* BCSR defines                         */
/****************************************/
#define BCSR6_UPC1_EN		0x80
#define BCSR6_UPC1_POS_EN	0x40
#define BCSR6_UPC1_ADDR_EN	0x20
#define BCSR6_UPC1_DEV2		0x10
#define BCSR6_SD_CARD_1BIT	0x08
#define BCSR6_SD_CARD_4BITS	0x04
#define BCSR6_TDM2G_EN		0x02
#define BCSR6_UCC7_RMII_EN	0x01

#define BCSR7_UCC1_GETH_EN	0x80
#define BCSR7_UCC1_RGMII_EN	0x40
#define BCSR7_UCC1_RTBI_EN	0x20
#define BCSR7_GETHRST_MRVL	0x04
#define BCSR7_BRD_WRT_PROTECT	0x02

#define BCSR8_UCC2_GETH_EN	0x80
#define BCSR8_UCC2_RGMII_EN	0x40
#define BCSR8_UCC2_RTBI_EN	0x20
#define BCSR8_UEM_MARVEL_RESET	0x02

#define BCSR9_UCC3_GETH_EN	0x80
#define BCSR9_UCC3_RGMII_EN	0x40
#define BCSR9_UCC3_RTBI_EN	0x20
#define BCSR9_UCC3_RMII_EN	0x10
#define BCSR9_UCC3_UEM_MICREL	0x01

#define BCSR10_UCC4_GETH_EN	0x80
#define BCSR10_UCC4_RGMII_EN	0x40
#define BCSR10_UCC4_RTBI_EN	0x20

#define BCSR11_LED0		0x40
#define BCSR11_LED1		0x20
#define BCSR11_LED2		0x10

#define BCSR12_UCC6_RMII_EN	0x20
#define BCSR12_UCC8_RMII_EN	0x20

#define BCSR15_SMII6_DIS	0x08
#define BCSR15_SMII8_DIS	0x04
#define BCSR15_QEUART_EN	0x01

#define BCSR16_UPC1_DEV2	0x02

#define BCSR17_nUSBEN		0x80
#define BCSR17_nUSBLOWSPD	0x40
#define BCSR17_USBVCC		0x20
#define BCSR17_USBMODE		0x10
#define BCSR17_FLASH_nWP	0x01

/*BCSR Utils functions*/

void enable_8569mds_flash_write(void);
void disable_8569mds_flash_write(void);
void enable_8569mds_qe_uec(void);
void disable_8569mds_brd_eeprom_write_protect(void);

#endif	/* __BCSR_H_ */
