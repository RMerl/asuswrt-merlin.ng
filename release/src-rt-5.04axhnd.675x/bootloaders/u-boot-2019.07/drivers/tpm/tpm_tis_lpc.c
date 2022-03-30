// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 */

/*
 * The code in this file is based on the article "Writing a TPM Device Driver"
 * published on http://ptgmedia.pearsoncmg.com.
 *
 * One principal difference is that in the simplest config the other than 0
 * TPM localities do not get mapped by some devices (for instance, by Infineon
 * slb9635), so this driver provides access to locality 0 only.
 */

#include <common.h>
#include <dm.h>
#include <mapmem.h>
#include <tpm-v1.h>
#include <asm/io.h>

#define PREFIX "lpc_tpm: "

enum i2c_chip_type {
	SLB9635,
	AT97SC3204,
};

static const char * const chip_name[] = {
	[SLB9635] = "Infineon SLB9635 TT 1.2",
	[AT97SC3204] = "Atmel AT97SC3204",
};

static const u32 chip_didvid[] = {
	[SLB9635] = 0xb15d1,
	[AT97SC3204] = 0x32041114,
};

struct tpm_locality {
	u32 access;
	u8 padding0[4];
	u32 int_enable;
	u8 vector;
	u8 padding1[3];
	u32 int_status;
	u32 int_capability;
	u32 tpm_status;
	u8 padding2[8];
	u8 data;
	u8 padding3[3803];
	u32 did_vid;
	u8 rid;
	u8 padding4[251];
};

struct tpm_tis_lpc_priv {
	struct tpm_locality *regs;
};

/*
 * This pointer refers to the TPM chip, 5 of its localities are mapped as an
 * array.
 */
#define TPM_TOTAL_LOCALITIES	5

/* Some registers' bit field definitions */
#define TIS_STS_VALID                  (1 << 7) /* 0x80 */
#define TIS_STS_COMMAND_READY          (1 << 6) /* 0x40 */
#define TIS_STS_TPM_GO                 (1 << 5) /* 0x20 */
#define TIS_STS_DATA_AVAILABLE         (1 << 4) /* 0x10 */
#define TIS_STS_EXPECT                 (1 << 3) /* 0x08 */
#define TIS_STS_RESPONSE_RETRY         (1 << 1) /* 0x02 */

#define TIS_ACCESS_TPM_REG_VALID_STS   (1 << 7) /* 0x80 */
#define TIS_ACCESS_ACTIVE_LOCALITY     (1 << 5) /* 0x20 */
#define TIS_ACCESS_BEEN_SEIZED         (1 << 4) /* 0x10 */
#define TIS_ACCESS_SEIZE               (1 << 3) /* 0x08 */
#define TIS_ACCESS_PENDING_REQUEST     (1 << 2) /* 0x04 */
#define TIS_ACCESS_REQUEST_USE         (1 << 1) /* 0x02 */
#define TIS_ACCESS_TPM_ESTABLISHMENT   (1 << 0) /* 0x01 */

#define TIS_STS_BURST_COUNT_MASK       (0xffff)
#define TIS_STS_BURST_COUNT_SHIFT      (8)

 /* 1 second is plenty for anything TPM does. */
#define MAX_DELAY_US	(1000 * 1000)

/* Retrieve burst count value out of the status register contents. */
static u16 burst_count(u32 status)
{
	return (status >> TIS_STS_BURST_COUNT_SHIFT) &
			TIS_STS_BURST_COUNT_MASK;
}

/* TPM access wrappers to support tracing */
static u8 tpm_read_byte(struct tpm_tis_lpc_priv *priv, const u8 *ptr)
{
	u8  ret = readb(ptr);
	debug(PREFIX "Read reg 0x%4.4x returns 0x%2.2x\n",
	      (u32)(uintptr_t)ptr - (u32)(uintptr_t)priv->regs, ret);
	return ret;
}

static u32 tpm_read_word(struct tpm_tis_lpc_priv *priv, const u32 *ptr)
{
	u32  ret = readl(ptr);
	debug(PREFIX "Read reg 0x%4.4x returns 0x%8.8x\n",
	      (u32)(uintptr_t)ptr - (u32)(uintptr_t)priv->regs, ret);
	return ret;
}

static void tpm_write_byte(struct tpm_tis_lpc_priv *priv, u8 value, u8 *ptr)
{
	debug(PREFIX "Write reg 0x%4.4x with 0x%2.2x\n",
	      (u32)(uintptr_t)ptr - (u32)(uintptr_t)priv->regs, value);
	writeb(value, ptr);
}

static void tpm_write_word(struct tpm_tis_lpc_priv *priv, u32 value,
			   u32 *ptr)
{
	debug(PREFIX "Write reg 0x%4.4x with 0x%8.8x\n",
	      (u32)(uintptr_t)ptr - (u32)(uintptr_t)priv->regs, value);
	writel(value, ptr);
}

/*
 * tis_wait_reg()
 *
 * Wait for at least a second for a register to change its state to match the
 * expected state. Normally the transition happens within microseconds.
 *
 * @reg - pointer to the TPM register
 * @mask - bitmask for the bitfield(s) to watch
 * @expected - value the field(s) are supposed to be set to
 *
 * Returns the register contents in case the expected value was found in the
 * appropriate register bits, or -ETIMEDOUT on timeout.
 */
static int tis_wait_reg(struct tpm_tis_lpc_priv *priv, u32 *reg, u8 mask,
			u8 expected)
{
	u32 time_us = MAX_DELAY_US;

	while (time_us > 0) {
		u32 value = tpm_read_word(priv, reg);
		if ((value & mask) == expected)
			return value;
		udelay(1); /* 1 us */
		time_us--;
	}

	return -ETIMEDOUT;
}

/*
 * Probe the TPM device and try determining its manufacturer/device name.
 *
 * Returns 0 on success, -ve on error
 */
static int tpm_tis_lpc_probe(struct udevice *dev)
{
	struct tpm_tis_lpc_priv *priv = dev_get_priv(dev);
	fdt_addr_t addr;
	u32 didvid;
	ulong chip_type = dev_get_driver_data(dev);

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;
	priv->regs = map_sysmem(addr, 0);
	didvid = tpm_read_word(priv, &priv->regs[0].did_vid);

	if (didvid != chip_didvid[chip_type]) {
		u32 vid, did;
		vid = didvid & 0xffff;
		did = (didvid >> 16) & 0xffff;
		debug("Invalid vendor/device ID %04x/%04x\n", vid, did);
		return -ENODEV;
	}

	debug("Found TPM: %s\n", chip_name[chip_type]);

	return 0;
}

/*
 * tis_senddata()
 *
 * send the passed in data to the TPM device.
 *
 * @data - address of the data to send, byte by byte
 * @len - length of the data to send
 *
 * Returns 0 on success, -ve on error (in case the device does not accept
 * the entire command).
 */
static int tis_senddata(struct udevice *dev, const u8 *data, size_t len)
{
	struct tpm_tis_lpc_priv *priv = dev_get_priv(dev);
	struct tpm_locality *regs = priv->regs;
	u32 offset = 0;
	u16 burst = 0;
	u32 max_cycles = 0;
	u8 locality = 0;
	u32 value;

	value = tis_wait_reg(priv, &regs[locality].tpm_status,
			     TIS_STS_COMMAND_READY, TIS_STS_COMMAND_READY);
	if (value == -ETIMEDOUT) {
		printf("%s:%d - failed to get 'command_ready' status\n",
		       __FILE__, __LINE__);
		return value;
	}
	burst = burst_count(value);

	while (1) {
		unsigned count;

		/* Wait till the device is ready to accept more data. */
		while (!burst) {
			if (max_cycles++ == MAX_DELAY_US) {
				printf("%s:%d failed to feed %zd bytes of %zd\n",
				       __FILE__, __LINE__, len - offset, len);
				return -ETIMEDOUT;
			}
			udelay(1);
			burst = burst_count(tpm_read_word(priv,
					&regs[locality].tpm_status));
		}

		max_cycles = 0;

		/*
		 * Calculate number of bytes the TPM is ready to accept in one
		 * shot.
		 *
		 * We want to send the last byte outside of the loop (hence
		 * the -1 below) to make sure that the 'expected' status bit
		 * changes to zero exactly after the last byte is fed into the
		 * FIFO.
		 */
		count = min((size_t)burst, len - offset - 1);
		while (count--)
			tpm_write_byte(priv, data[offset++],
				       &regs[locality].data);

		value = tis_wait_reg(priv, &regs[locality].tpm_status,
				     TIS_STS_VALID, TIS_STS_VALID);

		if ((value == -ETIMEDOUT) || !(value & TIS_STS_EXPECT)) {
			printf("%s:%d TPM command feed overflow\n",
			       __FILE__, __LINE__);
			return value == -ETIMEDOUT ? value : -EIO;
		}

		burst = burst_count(value);
		if ((offset == (len - 1)) && burst) {
			/*
			 * We need to be able to send the last byte to the
			 * device, so burst size must be nonzero before we
			 * break out.
			 */
			break;
		}
	}

	/* Send the last byte. */
	tpm_write_byte(priv, data[offset++], &regs[locality].data);
	/*
	 * Verify that TPM does not expect any more data as part of this
	 * command.
	 */
	value = tis_wait_reg(priv, &regs[locality].tpm_status,
			     TIS_STS_VALID, TIS_STS_VALID);
	if ((value == -ETIMEDOUT) || (value & TIS_STS_EXPECT)) {
		printf("%s:%d unexpected TPM status 0x%x\n",
		       __FILE__, __LINE__, value);
		return value == -ETIMEDOUT ? value : -EIO;
	}

	/* OK, sitting pretty, let's start the command execution. */
	tpm_write_word(priv, TIS_STS_TPM_GO, &regs[locality].tpm_status);
	return 0;
}

/*
 * tis_readresponse()
 *
 * read the TPM device response after a command was issued.
 *
 * @buffer - address where to read the response, byte by byte.
 * @len - pointer to the size of buffer
 *
 * On success stores the number of received bytes to len and returns 0. On
 * errors (misformatted TPM data or synchronization problems) returns
 * -ve value.
 */
static int tis_readresponse(struct udevice *dev, u8 *buffer, size_t len)
{
	struct tpm_tis_lpc_priv *priv = dev_get_priv(dev);
	struct tpm_locality *regs = priv->regs;
	u16 burst;
	u32 value;
	u32 offset = 0;
	u8 locality = 0;
	const u32 has_data = TIS_STS_DATA_AVAILABLE | TIS_STS_VALID;
	u32 expected_count = len;
	int max_cycles = 0;

	/* Wait for the TPM to process the command. */
	value = tis_wait_reg(priv, &regs[locality].tpm_status,
			      has_data, has_data);
	if (value == -ETIMEDOUT) {
		printf("%s:%d failed processing command\n",
		       __FILE__, __LINE__);
		return value;
	}

	do {
		while ((burst = burst_count(value)) == 0) {
			if (max_cycles++ == MAX_DELAY_US) {
				printf("%s:%d TPM stuck on read\n",
				       __FILE__, __LINE__);
				return -EIO;
			}
			udelay(1);
			value = tpm_read_word(priv, &regs[locality].tpm_status);
		}

		max_cycles = 0;

		while (burst-- && (offset < expected_count)) {
			buffer[offset++] = tpm_read_byte(priv,
						&regs[locality].data);

			if (offset == 6) {
				/*
				 * We got the first six bytes of the reply,
				 * let's figure out how many bytes to expect
				 * total - it is stored as a 4 byte number in
				 * network order, starting with offset 2 into
				 * the body of the reply.
				 */
				u32 real_length;
				memcpy(&real_length,
				       buffer + 2,
				       sizeof(real_length));
				expected_count = be32_to_cpu(real_length);

				if ((expected_count < offset) ||
				    (expected_count > len)) {
					printf("%s:%d bad response size %d\n",
					       __FILE__, __LINE__,
					       expected_count);
					return -ENOSPC;
				}
			}
		}

		/* Wait for the next portion. */
		value = tis_wait_reg(priv, &regs[locality].tpm_status,
				     TIS_STS_VALID, TIS_STS_VALID);
		if (value == -ETIMEDOUT) {
			printf("%s:%d failed to read response\n",
			       __FILE__, __LINE__);
			return value;
		}

		if (offset == expected_count)
			break;	/* We got all we needed. */

	} while ((value & has_data) == has_data);

	/*
	 * Make sure we indeed read all there was. The TIS_STS_VALID bit is
	 * known to be set.
	 */
	if (value & TIS_STS_DATA_AVAILABLE) {
		printf("%s:%d wrong receive status %x\n",
		       __FILE__, __LINE__, value);
		return -EBADMSG;
	}

	/* Tell the TPM that we are done. */
	tpm_write_word(priv, TIS_STS_COMMAND_READY,
		       &regs[locality].tpm_status);

	return offset;
}

static int tpm_tis_lpc_close(struct udevice *dev)
{
	struct tpm_tis_lpc_priv *priv = dev_get_priv(dev);
	struct tpm_locality *regs = priv->regs;
	u8 locality = 0;

	if (tpm_read_word(priv, &regs[locality].access) &
	    TIS_ACCESS_ACTIVE_LOCALITY) {
		tpm_write_word(priv, TIS_ACCESS_ACTIVE_LOCALITY,
			       &regs[locality].access);

		if (tis_wait_reg(priv, &regs[locality].access,
				 TIS_ACCESS_ACTIVE_LOCALITY, 0) == -ETIMEDOUT) {
			printf("%s:%d - failed to release locality %d\n",
			       __FILE__, __LINE__, locality);
			return -ETIMEDOUT;
		}
	}
	return 0;
}

static int tpm_tis_lpc_open(struct udevice *dev)
{
	struct tpm_tis_lpc_priv *priv = dev_get_priv(dev);
	struct tpm_locality *regs = priv->regs;
	u8 locality = 0; /* we use locality zero for everything. */
	int ret;

	ret = tpm_tis_lpc_close(dev);
	if (ret) {
		printf("%s: Failed to close TPM\n", __func__);
		return ret;
	}

	/* now request access to locality. */
	tpm_write_word(priv, TIS_ACCESS_REQUEST_USE, &regs[locality].access);

	/* did we get a lock? */
	ret = tis_wait_reg(priv, &regs[locality].access,
			 TIS_ACCESS_ACTIVE_LOCALITY,
			 TIS_ACCESS_ACTIVE_LOCALITY);
	if (ret == -ETIMEDOUT) {
		printf("%s:%d - failed to lock locality %d\n",
		       __FILE__, __LINE__, locality);
		return ret;
	}

	tpm_write_word(priv, TIS_STS_COMMAND_READY,
		       &regs[locality].tpm_status);

	return 0;
}

static int tpm_tis_get_desc(struct udevice *dev, char *buf, int size)
{
	ulong chip_type = dev_get_driver_data(dev);

	if (size < 50)
		return -ENOSPC;

	return snprintf(buf, size, "1.2 TPM (%s)",
			chip_name[chip_type]);
}


static const struct tpm_ops tpm_tis_lpc_ops = {
	.open		= tpm_tis_lpc_open,
	.close		= tpm_tis_lpc_close,
	.get_desc	= tpm_tis_get_desc,
	.send		= tis_senddata,
	.recv		= tis_readresponse,
};

static const struct udevice_id tpm_tis_lpc_ids[] = {
	{ .compatible = "infineon,slb9635lpc", .data = SLB9635 },
	{ .compatible = "atmel,at97sc3204", .data = AT97SC3204 },
	{ }
};

U_BOOT_DRIVER(tpm_tis_lpc) = {
	.name   = "tpm_tis_lpc",
	.id     = UCLASS_TPM,
	.of_match = tpm_tis_lpc_ids,
	.ops    = &tpm_tis_lpc_ops,
	.probe	= tpm_tis_lpc_probe,
	.priv_auto_alloc_size = sizeof(struct tpm_tis_lpc_priv),
};
