/*
 * Broadcom BBSI interface over SiliconBackplane chipcommon gsio interface
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id$
 */

#ifndef _hndbbsi_h_
#define _hndbbsi_h_

#include <typedefs.h>
#include <sbchipc.h>

#ifndef _CFE_
#include <linux/spinlock.h>
#endif // endif
#define BBSI_CMD_READ				0x80
#define BBSI_CMD_WRITE				0x81

#define BBSI_STATUS_REG_ADDR			6
#define BBSI_STATUS_CPURUNNING_SHIFT		0x6
#define BBSI_STATUS_CPURUNNING_MASK		0x1
#define BBSI_STATUS_HABREQ_SHIFT		0x5
#define BBSI_STATUS_HABREQ_MASK			0x1
#define BBSI_STATUS_BUSY_SHIFT			0x4
#define BBSI_STATUS_BUSY_MASK			0x1
#define BBSI_STATUS_RBUS_UNEXPTX_SHIFT		0x3
#define BBSI_STATUS_RBUS_UNEXPTX_MASK		0x1
#define BBSI_STATUS_RBUS_TIMEOUT_SHIFT		0x2
#define BBSI_STATUS_RBUS_TIMEOUT_MASK		0x1
#define BBSI_STATUS_ERROR_SHIFT			0x0
#define BBSI_STATUS_ERROR_MASK			0x1
#define BBSI_STATUS_FAILED			0xf

#define BBSI_CONFIG_REG_ADDR			7
#define BBSI_CONFIG_XFER_MODE_SHIFT		0x3
#define BBSI_CONFIG_XFER_MODE_MASK		0x3
#define BBSI_CONFIG_NO_RBUS_ADDR_INC_SHIFT	0x2
#define BBSI_CONFIG_NO_RBUS_ADDR_INC_MASK	0x1
#define BBSI_CONFIG_SPEC_READ_SHIFT		0x1
#define BBSI_CONFIG_SPEC_READ_MASK		0x1
#define BBSI_CONFIG_READ_RBUS_SHIFT		0x0
#define BBSI_CONFIG_READ_RBUS_MASK		0x1

#define BBSI_DATA0_REG_ADDR			0xc

#define MAX_SPISLAVE_DEV_NUM	   1
#define MAX_SPISLAVE_WRITE_BUFLEN		500

/* BCM6802 MoCA chip series register address */
#define SUN_TOP_CTRL_PIN_MUX_CTRL_0		0x10404100
#define SUN_TOP_CTRL_PIN_MUX_CTRL_1		0x10404104
#define SUN_TOP_CTRL_PIN_MUX_CTRL_2		0x10404108
#define SUN_TOP_CTRL_PIN_MUX_CTRL_3		0x1040410c
#define SUN_TOP_CTRL_PIN_MUX_CTRL_4		0x10404110
#define SUN_TOP_CTRL_PIN_MUX_CTRL_5		0x10404114
#define SUN_TOP_CTRL_PIN_MUX_CTRL_6		0x10404118

#define SUN_TOP_CTRL_CHIP_FAMILY_ID		0x10404000
#define SUN_TOP_CTRL_PRODUCT_ID			0x10404004
#define SUN_TOP_CTRL_GENERAL_CTRL_NO_SCAN_0	0x104040a4
#define SUN_TOP_CTRL_GENERAL_CTRL_NO_SCAN_1	0x104040a8
#define SUN_TOP_CTRL_GENERAL_CTRL_NO_SCAN_5	0x104040b8
#define SUN_TOP_CTRL_PIN_MUX_PAD_CTRL_3		0x10404128
#define SUN_TOP_CTRL_SW_INIT_0_CLEAR		0x1040431C
#define SUN_TOP_CTRL_SW_INIT_1_CLEAR		0x10404334

#define CLKGEN_PAD_CLOCK_DISABLE		0x1010013c
#define CLKGEN_LEAP_TOP_INST_CLOCK_DISABLE	0x101000d4

#define PM_CLK_CTRL		0x10406184
#define PM_CONFIG		0x10406180

#define EPORT_REG_EMUX_CNTRL			0x10800000
#define EPORT_REG_GPHY_CNTRL			0x10800004
#define EPORT_REG_RGMII_0_CNTRL			0x1080000c
#define EPORT_REG_RGMII_0_RX_CLOCK_DELAY_CNTRL	0x10800014
#define EPORT_REG_RGMII_1_CNTRL			0x10800018
#define EPORT_REG_RGMII_1_RX_CLOCK_DELAY_CNTRL	0x10800020
#define EPORT_REG_LED_CNTRL			0x10800024

struct bbsi;
typedef struct bbsi bbsi_t;

struct bbsi {
	si_t *sih;
	uint32 slave_fid;
	uint32 slave_pid;

	uint32 spi_clk;		/* gpio pin numbers */
	uint32 spi_cs;
	uint32 spi_mosi;
	uint32 spi_miso;
	uint32 moca_reset;
	uint32 moca_intr;

	int (*poll)(bbsi_t *bbsi);
	int (*read)(bbsi_t *bbsi, uint32 addr, uint32 len, uint32 *data);
	int (*write)(bbsi_t *bbsi, uint32 addr, uint32 len, uint32 data);
#ifndef _CFE_
	spinlock_t irq_lock;
	int (*readbuf)(bbsi_t *bbsi, uint32 addr, uint32 len, uint32 *data);
	int (*writebuf)(bbsi_t *bbsi, uint32 addr, uint32 len, uint32 *data);
	int (*read32)(bbsi_t *bbsi, uint32 addr);
	int (*write32)(bbsi_t *bbsi, uint32 addr, uint32 data);
#endif /* !_CFE_ */
};

bbsi_t *bbsi_init(si_t *sih);

#endif /* _hndbbsi_h_ */
