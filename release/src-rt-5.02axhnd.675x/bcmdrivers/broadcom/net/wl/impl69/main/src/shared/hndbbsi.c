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

#include <bcm_cfg.h>
#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <hndsoc.h>
#include <bcmdevs.h>
#include <bcmendian.h>
#include <hndbbsi.h>

#ifndef _CFE_
#include <linux/spinlock.h>
#endif // endif
#ifdef BCMDBG
#define	BBSI_MSG(args)	printf args
#else
#define	BBSI_MSG(args)
#endif	/* BCMDBG */

#define MAX_STATUS_RETRY 5

/* Private hnadler */
static bbsi_t bbsi = {0};

static int
spi_write(bbsi_t *bbsi, uint8 *buf, uint32 buflen)
{
	uint32 spi_clk = 1 << bbsi->spi_clk;
	uint32 spi_cs = 1 << bbsi->spi_cs;
	uint32 spi_mosi = 1 << bbsi->spi_mosi;
	uint32 i, j, output;
	uint8 data;

	/* set CS low (activate) */
	si_gpioout(bbsi->sih, spi_cs, 0, GPIO_DRV_PRIORITY);

	for (i = 0; i < buflen; i++) {
		data = buf[i];

		/* send data from MSB to LSB */
		for (j = 0; j < 8; j++) {
			output = (data & 0x80) ? spi_mosi : 0;

			/* clock falling and output data (mode 3) */
			si_gpioout(bbsi->sih, spi_clk | spi_mosi, 0 | output, GPIO_DRV_PRIORITY);
			/* clock rising */
			si_gpioout(bbsi->sih, spi_clk, spi_clk, GPIO_DRV_PRIORITY);
			/* shift to next bit to be send */
			data <<= 1;
		}
	}

	/* Set CS high (deactivate) */
	si_gpioout(bbsi->sih, spi_cs, spi_cs, GPIO_DRV_PRIORITY);

	return 0;
}

static int
spi_write_then_read(bbsi_t *bbsi, uint8 *buf, uint32 buflen, uint8 *rbuf, uint32 rbuflen)
{
	uint32 spi_clk = 1 << bbsi->spi_clk;
	uint32 spi_cs = 1 << bbsi->spi_cs;
	uint32 spi_mosi = 1 << bbsi->spi_mosi;
	uint32 spi_miso = 1 << bbsi->spi_miso;
	uint32 i, j, output;
	uint8 data;

	/* set CS low (activate) */
	si_gpioout(bbsi->sih, spi_cs, 0, GPIO_DRV_PRIORITY);

	for (i = 0; i < buflen; i++) {
		data = buf[i];

		/* send data from MSB to LSB */
		for (j = 0; j < 8; j++) {
			output = (data & 0x80) ? spi_mosi : 0;

			/* clock falling and output data (mode 3) */
			si_gpioout(bbsi->sih, (spi_clk | spi_mosi), output, GPIO_DRV_PRIORITY);
			/* clock rising */
			si_gpioout(bbsi->sih, spi_clk, spi_clk, GPIO_DRV_PRIORITY);
			/* shift to next bit to be send */
			data <<= 1;
		}
	}

	for (i = 0; i < rbuflen; i++) {
		/* receive data from MSB to LSB */
		for (j = 0, data = 0; j < 8; j++) {
			/* shift to next bit */
			data <<= 1;

			/* clock falling */
			si_gpioout(bbsi->sih, spi_clk, 0, GPIO_DRV_PRIORITY);
			/* sample input data */
			data |= ((si_gpioin(bbsi->sih) & spi_miso) > 0) ? 1: 0;
			/* clock rising */
			si_gpioout(bbsi->sih, spi_clk, spi_clk, GPIO_DRV_PRIORITY);
		}

		rbuf[i] = data;
	}

	/* Set CS high (deactivate) */
	si_gpioout(bbsi->sih, spi_cs, spi_cs, GPIO_DRV_PRIORITY);

	return 0;
}

/* Poll for command completion. Returns zero when complete. */
static int
is_bbsi_done(bbsi_t *bbsi)
{
	uint8 read_status[2] = {BBSI_CMD_READ, BBSI_STATUS_REG_ADDR};
	uint8 read_rx;
	int status, i, ret = -1;

	for (i = 0; i < MAX_STATUS_RETRY; i++) {
		/* issue command to spi */
		status = spi_write_then_read(bbsi, read_status, 2, &read_rx, 1);

		/* status return from spi operation */
		if (status != 0) {
			printf("%s: spi returned error\n", __func__);
			break;
		}

		/* transaction error reported by BBSI slave device */
		if (read_rx & BBSI_STATUS_FAILED) {
			printf("%s: BBSI transaction error, status=0x%x\n", __func__, read_rx);
			break;
		} else if ((read_rx & (1 << BBSI_STATUS_BUSY_SHIFT)) == 0) {
			/* transaction completed */
			ret = 0;
			break;
		}
	}

	return ret;
}

static int
bbsi_read(bbsi_t *bbsi, uint32 addr, uint32 readlen, uint32 *data)
{
	uint8 buf[7] = {0}, rbuf[4] = {0};
	int status, ret = -1;
#ifndef _CFE_
	unsigned long flags;
	/* Disable irq */
	spin_lock_irqsave(&bbsi->irq_lock, flags);
#endif /* !CFE */
	if (readlen > 4) {
		printf("%s: incorrect read length: %d\n", __func__, readlen);
#ifndef _CFE_
		spin_unlock_irqrestore(&bbsi->irq_lock, flags);
#endif /* !CFE */
		return ret;
	}

	buf[0] = BBSI_CMD_WRITE;
	buf[1] = BBSI_CONFIG_REG_ADDR;
	buf[2] = ((4 - readlen) << BBSI_CONFIG_XFER_MODE_SHIFT) |
		(BBSI_CONFIG_READ_RBUS_MASK << BBSI_CONFIG_READ_RBUS_SHIFT);
	buf[3] = (addr >> 24) & 0xFF;	/* Assuming MSB bytes are always sent first */
	buf[4] = (addr >> 16) & 0xFF;
	buf[5] = (addr >> 8) & 0xFF;
	buf[6] = (addr >> 0) & 0xFF;

	status = spi_write(bbsi, buf, 7);

	if (status != 0) {
		printf("%s: Spi returned error\n", __func__);
#ifndef _CFE_
		spin_unlock_irqrestore(&bbsi->irq_lock, flags);
#endif /* !CFE */
		return ret;
	}

	if (is_bbsi_done(bbsi) != 0) {
		printf("%s: read to addr:0x%08x failed\n", __func__, addr);
#ifndef _CFE_
		spin_unlock_irqrestore(&bbsi->irq_lock, flags);
#endif /* !CFE */
		return ret;
	}

	buf[0] = BBSI_CMD_READ;
	buf[1] = BBSI_DATA0_REG_ADDR;

	status = spi_write_then_read(bbsi, buf, 2, rbuf, 4);

	if (status != 0) {
		printf("%s: spi returned error\n", __func__);
#ifndef _CFE_
		spin_unlock_irqrestore(&bbsi->irq_lock, flags);
#endif /* !CFE */
		return ret;
	}

	*data = (rbuf[0] << 24) | (rbuf[1] << 16) | (rbuf[2] << 8) | rbuf[3];
	ret = 0;

#ifndef _CFE_
	/* Enable irq */
	spin_unlock_irqrestore(&bbsi->irq_lock, flags);
#endif /* !CFE */
	return ret;
}

static int
bbsi_write(bbsi_t *bbsi, uint32 addr, uint32 writelen, uint32 data)
{
	uint8 buf[12];
	int status, ret = -1;
#ifndef _CFE_
	unsigned long flags;
	/* Disable irq */
	spin_lock_irqsave(&bbsi->irq_lock, flags);
#endif /* !CFE */
	if (writelen > 4) {
		printf("%s: incorrect write length: %d\n", __func__, writelen);
#ifndef _CFE_
		spin_unlock_irqrestore(&bbsi->irq_lock, flags);
#endif /* !CFE */
		return ret;
	}

	data <<= (8 * (4 - writelen));

	buf[0] = BBSI_CMD_WRITE;
	buf[1] = BBSI_CONFIG_REG_ADDR;
	buf[2] = (4 - writelen) << BBSI_CONFIG_XFER_MODE_SHIFT;
	buf[3] = (addr >> 24) & 0xFF;	/* Assuming MSB bytes are always sent first */
	buf[4] = (addr >> 16) & 0xFF;
	buf[5] = (addr >> 8) & 0xFF;
	buf[6] = (addr >> 0) & 0xFF;
	buf[7] = (data >> 24) & 0xFF;
	buf[8] = (data >> 16) & 0xFF;
	buf[9] = (data >> 8) & 0xFF;
	buf[10] = (data >> 0) & 0xFF;

	status = spi_write(bbsi, buf, 11);

	if (status != 0) {
		printf("%s: Spi returned error\n", __func__);
#ifndef _CFE_
		spin_unlock_irqrestore(&bbsi->irq_lock, flags);
#endif /* !CFE */
		return ret;
	}

	if (is_bbsi_done(bbsi) != 0) {
		printf("%s: write to addr:0x%08x failed\n", __func__, addr);
#ifndef _CFE_
		spin_unlock_irqrestore(&bbsi->irq_lock, flags);
#endif /* !CFE */
		return ret;
	}

	ret = 0;
#ifndef _CFE_
	/* Enable irq */
	spin_unlock_irqrestore(&bbsi->irq_lock, flags);
#endif /* !CFE */
	return ret;
}

#ifndef _CFE_
uint32
bbsi_readregs32(bbsi_t *bbsih, uint32 addr)
{
	unsigned long data = 0;
	BUG_ON(addr & 3);
	addr &= 0x1fffffff;

	if (bbsih->read(bbsih, addr, 4, &data) < 0) {
		printk("bbsi_read: can't read %08x\n",
		       (unsigned int)addr);
	}

	return (data);
}

void bbsi_writeregs32(bbsi_t *bbsih, uint32 addr, uint32 data)
{
	BUG_ON(addr & 3);
	addr &= 0x1fffffff;

	if (bbsih->write(bbsih, addr, 4, data) < 0) {
		printk(KERN_ERR
		       "bbsi_write: can't write %08x (data %08x)\n",
		       (unsigned int)addr, (unsigned int)data);
	}
}

int
bbsi_readbuf(bbsi_t *bbsi, uint32 addr, uint32 readlen, uint32 *data)
{
	uint8 buf[7] = {0};
	int status, ret = -1;
	unsigned long flags;

	/* Disable irq */
	spin_lock_irqsave(&bbsi->irq_lock, flags);

	buf[0] = BBSI_CMD_WRITE;
	buf[1] = BBSI_CONFIG_REG_ADDR;
	buf[2] = (BBSI_CONFIG_SPEC_READ_MASK << BBSI_CONFIG_SPEC_READ_SHIFT) |
		(BBSI_CONFIG_READ_RBUS_MASK << BBSI_CONFIG_READ_RBUS_SHIFT);
	buf[3] = (addr >> 24) & 0xFF;
	buf[4] = (addr >> 16) & 0xFF;
	buf[5] = (addr >> 8) & 0xFF;
	buf[6] = (addr >> 0) & 0xFF;

	status = spi_write(bbsi, buf, 7);

	if (status != 0) {
		printf("%s: Spi returned error\n", __func__);
		spin_unlock_irqrestore(&bbsi->irq_lock, flags);
		return ret;
	}

	if (is_bbsi_done(bbsi) != 0) {
		printf("%s: read to addr:0x%08x failed\n", __func__, addr);
		spin_unlock_irqrestore(&bbsi->irq_lock, flags);
		return ret;
	}

	buf[0] = BBSI_CMD_READ;
	buf[1] = BBSI_DATA0_REG_ADDR;

	while (readlen) {
		uint32 count;

		count = (readlen >= 4 ? 4 : readlen);
		status = spi_write_then_read(bbsi, buf, 2, data, count);

		/* status return from spi operation */
		if (status != 0) {
			printf("%s: consecutive read command failed\n", __func__);
			spin_unlock_irqrestore(&bbsi->irq_lock, flags);
			return ret;
		}

		/* check if bbsi transaction completed */
		if (is_bbsi_done(bbsi) != 0) {
			printf("%s consecutive read not ready %d\n", __func__, readlen);
			spin_unlock_irqrestore(&bbsi->irq_lock, flags);
			return ret;
		}

		data += count/4;
		readlen -= count;
	}

	ret = 0;
	spin_unlock_irqrestore(&bbsi->irq_lock, flags);
	return ret;
}

int
bbsi_writebuf(bbsi_t *bbsi, uint32 addr, uint32 writelen, uint32 *data)
{
	uint32 start_addr, blocksize, *p = data;
	int status, ret = -1;
	uint8 buf[MAX_SPISLAVE_WRITE_BUFLEN + 12] = {0};
	unsigned long flags;

	/* Disable irq */
	spin_lock_irqsave(&bbsi->irq_lock, flags);

	addr &= 0x1fffffff;

	ASSERT(writelen);

	/* limit each write transaction under 512 byte? */
	while (writelen) {
		/* check writing block size */
		blocksize = (writelen >= MAX_SPISLAVE_WRITE_BUFLEN?
			MAX_SPISLAVE_WRITE_BUFLEN: writelen);
		/* length remain to be written */
		writelen -= blocksize;
		/* shift start address */
		start_addr = addr;

		buf[0] = BBSI_CMD_WRITE;
		buf[1] = BBSI_CONFIG_REG_ADDR;
		buf[2] = 0;
		buf[3] = (addr >> 24) & 0xFF;
		buf[4] = (addr >> 16) & 0xFF;
		buf[5] = (addr >> 8) & 0xFF;
		buf[6] = (addr >> 0) & 0xFF;
		memcpy(&buf[7], p, blocksize);

		status = spi_write(bbsi, buf, blocksize + 7);

		if (status != 0) {
			printf("%s: Spi returned error\n", __func__);
			spin_unlock_irqrestore(&bbsi->irq_lock, flags);
			return ret;
		}

		if (is_bbsi_done(bbsi) != 0) {
			printf("%s: write to addr:0x%08x failed\n", __func__, addr);
			spin_unlock_irqrestore(&bbsi->irq_lock, flags);
			return ret;
		}
		addr += blocksize;
		p = (uint32 *)((char*)p + blocksize);
	}

	ret = 0;
	spin_unlock_irqrestore(&bbsi->irq_lock, flags);
	return ret;
}
#endif /* !CFE */

static void
bbsi_reset_moca_slavedev(bbsi_t *bbsi)
{
	uint moca_reset = 1 << bbsi->moca_reset;

	/* using GPIO to reset moca device */
	si_gpioouten(bbsi->sih, moca_reset, moca_reset, GPIO_DRV_PRIORITY);

	/* Keep RESET high for 50ms */
	si_gpioout(bbsi->sih, moca_reset, moca_reset, GPIO_DRV_PRIORITY);
	OSL_DELAY(50000);

	/* Keep RESET low for 300ms */
	si_gpioout(bbsi->sih, moca_reset, 0, GPIO_DRV_PRIORITY);
	OSL_DELAY(300000);

	/* Keep RESET high for 300ms */
	si_gpioout(bbsi->sih, moca_reset, moca_reset, GPIO_DRV_PRIORITY);
	OSL_DELAY(300000);
}

static void
bbsi_init_gphy(bbsi_t *bbsi)
{
	/* write 1's except to bit 26 (gphy sw_init) */
	bbsi_write(bbsi, 0x1040431c, 4, 0xFBFFFFFF);
	/* clear bits 2 and 0 */
	bbsi_write(bbsi, 0x10800004, 4, 0x02a4c000);

	/* DELAY 10MS */
	OSL_DELAY(10000);

	bbsi_write(bbsi,  0x1040431c, 4, 0xFFFFFFFF);
	/* take unimac out of reset */
	bbsi_write(bbsi, 0x10800000, 4, 0);

	/* Pin muxing for rgmii 0 and 1 */
	bbsi_write(bbsi, 0x10404100, 4, 0x11110000);
	bbsi_write(bbsi, 0x10404104, 4, 0x11111111);
	bbsi_write(bbsi, 0x10404108, 4, 0x11111111);
	bbsi_write(bbsi, 0x1040410c, 4, 0x00001111);
	/* Pin mux for MDIO */
	bbsi_write(bbsi, 0x10404118, 4, 0x00001100);

	bbsi_write(bbsi, 0x10800024, 4, 0x0000930d);
	/* enable rgmii 0 */
	bbsi_write(bbsi, 0x1080000c, 4, 0x00000011);
	/* enable rgmii 1 */
	bbsi_write(bbsi, 0x10800018, 4, 0x00000011);

	bbsi_write(bbsi, 0x10800808, 4, 0x010000d8);
	/* port mode for gphy and moca from rgmii */
	bbsi_write(bbsi, 0x10800000, 4, 2);

	/* tx and rx enable (0x3) */
	bbsi_write(bbsi, 0x10800808, 4, 0x0100000b);
	/* Link/ACT LED */
	bbsi_write(bbsi, 0x10800024, 4, 0x0000934d);

	/* Set 6802 mdio slave */
	bbsi_write(bbsi, 0x10800000, 4, 0xE);
	/* enable ID mode on RGMII 1 */
	bbsi_write(bbsi, 0x1080000c, 4, 0x0000001f);
	/* enable ID mode on RGMII 0 */
	bbsi_write(bbsi, 0x10800018, 4, 0x0000001f);
	/* Set rgmii to 2.5V CMOS */
	bbsi_write(bbsi, 0x104040a4, 4, 0x11);
}

/* Initialize gsio bbsi interface */
bbsi_t *
bbsi_init(si_t *sih)
{
	ASSERT(sih);

	/* Already initialized ? */
	if (bbsi.sih != NULL)
		return &bbsi;

	/* 53573/47189 series */
	if (sih->ccrev == 54) {
		uint32 data = 0;
		uint32 mask, val;

		bzero(&bbsi, sizeof(bbsi));

		bbsi.spi_clk = getgpiopin(NULL, "bbsi_clk", GPIO_PIN_NOTDEFINED);
		bbsi.spi_cs = getgpiopin(NULL, "bbsi_cs", GPIO_PIN_NOTDEFINED);
		bbsi.spi_mosi = getgpiopin(NULL, "bbsi_mosi", GPIO_PIN_NOTDEFINED);
		bbsi.spi_miso = getgpiopin(NULL, "bbsi_miso", GPIO_PIN_NOTDEFINED);
		bbsi.moca_reset = getgpiopin(NULL, "moca_reset", GPIO_PIN_NOTDEFINED);
		bbsi.moca_intr = getgpiopin(NULL, "moca_intr", GPIO_PIN_NOTDEFINED);

		if (bbsi.spi_clk == GPIO_PIN_NOTDEFINED || bbsi.spi_cs == GPIO_PIN_NOTDEFINED ||
		    bbsi.spi_mosi == GPIO_PIN_NOTDEFINED || bbsi.spi_miso == GPIO_PIN_NOTDEFINED ||
		    bbsi.moca_reset == GPIO_PIN_NOTDEFINED) {
			printf("No GPIO defined for BBSI interface \n");
			return NULL;
		}
		BBSI_MSG(("BBSI GPIO: clk: %d, cs: %d, mosi: %d, miso: %d, reset: %d intr: %d\n",
			bbsi.spi_clk, bbsi.spi_cs, bbsi.spi_mosi, bbsi.spi_miso,
			bbsi.moca_reset, bbsi.moca_intr));

		bbsi.sih = sih;
		bbsi.poll = NULL;
		bbsi.read = bbsi_read;
		bbsi.write = bbsi_write;
#ifndef _CFE_
		bbsi.read32 = bbsi_readregs32;
		bbsi.write32 = bbsi_writeregs32;
		bbsi.readbuf = bbsi_readbuf;
		bbsi.writebuf = bbsi_writebuf;
		bbsi.irq_lock = SPIN_LOCK_UNLOCKED;
#endif /* !_CFE_ */

		/* BCM6802 need spi on mode3.  set clk, cs to 1, mosi to 0 */
		mask = (1 << bbsi.spi_clk) | (1 << bbsi.spi_cs) | (1 << bbsi.spi_mosi);
		val = (1 << bbsi.spi_clk) | (1 << bbsi.spi_cs);
		si_gpioout(sih, mask, val, GPIO_DRV_PRIORITY);

		/* Setup GPIO output */
		mask = (1 << bbsi.spi_clk) | (1 << bbsi.spi_mosi) | (1 << bbsi.spi_cs);
		val = (1 << bbsi.spi_clk) | (1 << bbsi.spi_mosi) | (1 << bbsi.spi_cs);
		si_gpioouten(sih, mask, val, GPIO_DRV_PRIORITY);

		bbsi_reset_moca_slavedev(&bbsi);

		if ((bbsi_read(&bbsi, SUN_TOP_CTRL_CHIP_FAMILY_ID, 4, &data) < 0) ||
		    data == 0 || data == 0xffffffff) {
			printf("Failed to get SUN_TOP_CTRL_CHIP_FAMILY_ID: 0x%08x\n", data);
			return NULL;
		} else {
			bbsi.slave_fid = data;
		}

		if ((bbsi_read(&bbsi, SUN_TOP_CTRL_PRODUCT_ID, 4, &data) < 0) ||
		    data == 0 || data == 0xffffffff) {
			printf("Failed to get SUN_TOP_CTRL_PRODUCT_ID: 0x%08x\n", data);
			return NULL;
		} else {
			bbsi.slave_pid = data;
		}

		printf("BBSI slave device SUN_TOP_CTRL_CHIP_FAMILY_ID: 0x%08x,"
			" SUN_TOP_CTRL_PRODUCT_ID: 0x%08x\n", bbsi.slave_fid, bbsi.slave_pid);

		/* init gphy */
		bbsi_init_gphy(&bbsi);
	}

	return &bbsi;
}
