// SPDX-License-Identifier: GPL-2.0
/*
 * Cr50 / H1 TPM support
 *
 * Copyright 2018 Google LLC
 */

#define LOG_CATEGORY UCLASS_TPM

#include <common.h>
#include <dm.h>
#include <i2c.h>
#include <irq.h>
#include <log.h>
#include <spl.h>
#include <tpm-common.h>
#include <tpm-v2.h>
#include <acpi/acpigen.h>
#include <acpi/acpi_device.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/unaligned.h>
#include <linux/delay.h>
#include <dm/acpi.h>

enum {
	TIMEOUT_INIT_MS		= 30000, /* Very long timeout for TPM init */
	TIMEOUT_LONG_US		= 2 * 1000 * 1000,
	TIMEOUT_SHORT_US	= 2 * 1000,
	TIMEOUT_NO_IRQ_US	= 20 * 1000,
	TIMEOUT_IRQ_US		= 100 * 1000,
};

enum {
	CR50_DID_VID = 0x00281ae0L
};

enum {
	CR50_MAX_BUF_SIZE = 63,
};

/*
 * Operations specific to the Cr50 TPM used on Chromium OS and Android devices
 *
 * FIXME: below is not enough to differentiate between vendors commands
 * of numerous devices. However, the current tpm2 APIs aren't very amenable
 * to extending generically because the marshaling code is assuming all
 * knowledge of all commands.
 */
#define TPM2_CC_VENDOR_BIT_MASK			0x20000000

#define TPM2_CR50_VENDOR_COMMAND		(TPM2_CC_VENDOR_BIT_MASK | 0)
#define TPM2_CR50_SUB_CMD_IMMEDIATE_RESET	19
#define TPM2_CR50_SUB_CMD_NVMEM_ENABLE_COMMITS	21
#define TPM2_CR50_SUB_CMD_REPORT_TPM_STATE	23
#define TPM2_CR50_SUB_CMD_TURN_UPDATE_ON	24
#define TPM2_CR50_SUB_CMD_GET_REC_BTN		29
#define TPM2_CR50_SUB_CMD_TPM_MODE		40
#define TPM2_CR50_SUB_CMD_GET_BOOT_MODE		52
#define TPM2_CR50_SUB_CMD_RESET_EC		53

/* Cr50 vendor-specific error codes. */
#define VENDOR_RC_ERR              0x00000500
enum cr50_vendor_rc {
	VENDOR_RC_INTERNAL_ERROR	= (VENDOR_RC_ERR | 6),
	VENDOR_RC_NO_SUCH_SUBCOMMAND	= (VENDOR_RC_ERR | 8),
	VENDOR_RC_NO_SUCH_COMMAND	= (VENDOR_RC_ERR | 127),
};

enum cr50_tpm_mode {
	/*
	 * Default state: TPM is enabled, and may be set to either
	 * TPM_MODE_ENABLED or TPM_MODE_DISABLED.
	 */
	TPM_MODE_ENABLED_TENTATIVE = 0,

	/* TPM is enabled, and mode may not be changed. */
	TPM_MODE_ENABLED = 1,

	/* TPM is disabled, and mode may not be changed. */
	TPM_MODE_DISABLED = 2,

	TPM_MODE_INVALID,
};

/**
 * struct cr50_priv - Private driver data
 *
 * @ready_gpio: GPIO to use to check if the TPM is ready
 * @irq: IRQ to use check if the TPM is ready (has priority over @ready_gpio)
 * @locality: Currenttly claimed locality (-1 if none)
 * @vendor: vendor: Vendor ID for TPM
 * @use_irq: true to use @irq, false to use @ready if available
 */
struct cr50_priv {
	struct gpio_desc ready_gpio;
	struct irq irq;
	int locality;
	uint vendor;
	bool use_irq;
};

/*
 * The below structure represents the body of the response to the 'report tpm
 * state' vendor command.
 *
 * It is transferred over the wire, so it needs to be serialized/deserialized,
 * and it is likely to change, so its contents must be versioned.
 */
#define TPM_STATE_VERSION	1
struct tpm_vendor_state {
	u32 version;
	/*
	 * The following three fields are set by the TPM in case of an assert.
	 * There is no other processing than setting the source code line
	 * number, error code and the first 4 characters of the function name.
	 *
	 * We don't expect this happening, but it is included in the report
	 * just in case.
	 */
	u32 fail_line;	/* s_failLIne */
	u32 fail_code;	/* s_failCode */
	char func_name[4];	/* s_failFunction, limited to 4 chars */

	/*
	 * The following two fields are the current time filtered value of the
	 * 'failed tries' TPM counter, and the maximum allowed value of the
	 * counter.
	 *
	 * failed_tries == max_tries is the definition of the TPM lockout
	 * condition.
	 */
	u32 failed_tries;	/* gp.failedTries */
	u32 max_tries;	/* gp.maxTries */
	/* The below fields are present in version 2 and above */
};

/* Wait for interrupt to indicate TPM is ready */
static int cr50_i2c_wait_tpm_ready(struct udevice *dev)
{
	struct cr50_priv *priv = dev_get_priv(dev);
	ulong timeout, base;
	int i;

	if (!priv->use_irq && !dm_gpio_is_valid(&priv->ready_gpio)) {
		/* Fixed delay if interrupt not supported */
		udelay(TIMEOUT_NO_IRQ_US);
		return 0;
	}

	base = timer_get_us();
	timeout = base + TIMEOUT_IRQ_US;

	i = 0;
	while (priv->use_irq ? !irq_read_and_clear(&priv->irq) :
	       !dm_gpio_get_value(&priv->ready_gpio)) {
		i++;
		if ((int)(timer_get_us() - timeout) >= 0) {
			log_warning("Timeout\n");
			/* Use this instead of the -ETIMEDOUT used by i2c */
			return -ETIME;
		}
	}
	log_debug("i=%d\n", i);

	return 0;
}

/* Clear pending interrupts */
static void cr50_i2c_clear_tpm_irq(struct udevice *dev)
{
	struct cr50_priv *priv = dev_get_priv(dev);

	if (priv->use_irq)
		irq_read_and_clear(&priv->irq);
}

/*
 * cr50_i2c_read() - read from TPM register
 *
 * @dev: TPM chip information
 * @addr: register address to read from
 * @buffer: provided by caller
 * @len: number of bytes to read
 *
 * 1) send register address byte 'addr' to the TPM
 * 2) wait for TPM to indicate it is ready
 * 3) read 'len' bytes of TPM response into the provided 'buffer'
 *
 * Return 0 on success. -ve on error
 */
static int cr50_i2c_read(struct udevice *dev, u8 addr, u8 *buffer,
			 size_t len)
{
	int ret;

	/* Clear interrupt before starting transaction */
	cr50_i2c_clear_tpm_irq(dev);

	/* Send the register address byte to the TPM */
	ret = dm_i2c_write(dev, 0, &addr, 1);
	if (ret) {
		log_err("Address write failed (err=%d)\n", ret);
		return ret;
	}

	/* Wait for TPM to be ready with response data */
	ret = cr50_i2c_wait_tpm_ready(dev);
	if (ret)
		return ret;

	/* Read response data frrom the TPM */
	ret = dm_i2c_read(dev, 0, buffer, len);
	if (ret) {
		log_err("Read response failed (err=%d)\n", ret);
		return ret;
	}

	return 0;
}

/*
 * cr50_i2c_write() - write to TPM register
 *
 * @dev: TPM chip information
 * @addr: register address to write to
 * @buffer: data to write
 * @len: number of bytes to write
 *
 * 1) prepend the provided address to the provided data
 * 2) send the address+data to the TPM
 * 3) wait for TPM to indicate it is done writing
 *
 * Returns -1 on error, 0 on success.
 */
static int cr50_i2c_write(struct udevice *dev, u8 addr, const u8 *buffer,
			  size_t len)
{
	u8 buf[len + 1];
	int ret;

	if (len > CR50_MAX_BUF_SIZE) {
		log_err("Length %zd is too large\n", len);
		return -E2BIG;
	}

	/* Prepend the 'register address' to the buffer */
	buf[0] = addr;
	memcpy(buf + 1, buffer, len);

	/* Clear interrupt before starting transaction */
	cr50_i2c_clear_tpm_irq(dev);

	/* Send write request buffer with address */
	ret = dm_i2c_write(dev, 0, buf, len + 1);
	if (ret) {
		log_err("Error writing to TPM (err=%d)\n", ret);
		return ret;
	}

	/* Wait for TPM to be ready */
	return cr50_i2c_wait_tpm_ready(dev);
}

static inline u8 tpm_access(int locality)
{
	if (locality == -1)
		locality = 0;
	return 0x0 | (locality << 4);
}

static inline u8 tpm_sts(int locality)
{
	if (locality == -1)
		locality = 0;
	return 0x1 | (locality << 4);
}

static inline u8 tpm_data_fifo(int locality)
{
	if (locality == -1)
		locality = 0;
	return 0x5 | (locality << 4);
}

static inline u8 tpm_did_vid(int locality)
{
	if (locality == -1)
		locality = 0;
	return 0x6 | (locality << 4);
}

static int release_locality(struct udevice *dev, int force)
{
	struct cr50_priv *priv = dev_get_priv(dev);
	u8 mask = TPM_ACCESS_VALID | TPM_ACCESS_REQUEST_PENDING;
	u8 addr = tpm_access(priv->locality);
	int ret;
	u8 buf;

	ret = cr50_i2c_read(dev, addr, &buf, 1);
	if (ret)
		return ret;

	if (force || (buf & mask) == mask) {
		buf = TPM_ACCESS_ACTIVE_LOCALITY;
		cr50_i2c_write(dev, addr, &buf, 1);
	}

	priv->locality = -1;

	return 0;
}

/* cr50 requires all 4 bytes of status register to be read */
static int cr50_i2c_status(struct udevice *dev)
{
	struct cr50_priv *priv = dev_get_priv(dev);
	u8 buf[4];
	int ret;

	ret = cr50_i2c_read(dev, tpm_sts(priv->locality), buf, sizeof(buf));
	if (ret) {
		log_warning("%s: Failed to read status\n", __func__);
		return ret;
	}

	return buf[0];
}

/* cr50 requires all 4 bytes of status register to be written */
static int cr50_i2c_ready(struct udevice *dev)
{
	struct cr50_priv *priv = dev_get_priv(dev);
	u8 buf[4] = { TPM_STS_COMMAND_READY };
	int ret;

	ret = cr50_i2c_write(dev, tpm_sts(priv->locality), buf, sizeof(buf));
	if (ret)
		return ret;

	udelay(TIMEOUT_SHORT_US);

	return 0;
}

static int cr50_i2c_wait_burststs(struct udevice *dev, u8 mask,
				  size_t *burst, int *status)
{
	struct cr50_priv *priv = dev_get_priv(dev);
	ulong timeout;
	u32 buf;

	/*
	 * cr50 uses bytes 3:2 of status register for burst count and all 4
	 * bytes must be read
	 */
	timeout = timer_get_us() + TIMEOUT_LONG_US;
	while (timer_get_us() < timeout) {
		if (cr50_i2c_read(dev, tpm_sts(priv->locality),
				  (u8 *)&buf, sizeof(buf)) < 0) {
			udelay(TIMEOUT_SHORT_US);
			continue;
		}

		*status = buf & 0xff;
		*burst = le16_to_cpu((buf >> 8) & 0xffff);

		if ((*status & mask) == mask &&
		    *burst > 0 && *burst <= CR50_MAX_BUF_SIZE)
			return 0;

		udelay(TIMEOUT_SHORT_US);
	}

	log_warning("Timeout reading burst and status\n");

	return -ETIMEDOUT;
}

static int cr50_i2c_recv(struct udevice *dev, u8 *buf, size_t buf_len)
{
	struct cr50_priv *priv = dev_get_priv(dev);
	size_t burstcnt, expected, current, len;
	u8 addr = tpm_data_fifo(priv->locality);
	u8 mask = TPM_STS_VALID | TPM_STS_DATA_AVAIL;
	u32 expected_buf;
	int status;
	int ret;

	log_debug("%s: buf_len=%x\n", __func__, buf_len);
	if (buf_len < TPM_HEADER_SIZE)
		return -E2BIG;

	ret = cr50_i2c_wait_burststs(dev, mask, &burstcnt, &status);
	if (ret < 0) {
		log_warning("First chunk not available\n");
		goto out_err;
	}

	/* Read first chunk of burstcnt bytes */
	if (cr50_i2c_read(dev, addr, buf, burstcnt) < 0) {
		log_warning("Read failed\n");
		goto out_err;
	}

	/* Determine expected data in the return buffer */
	memcpy(&expected_buf, buf + TPM_CMD_COUNT_OFFSET, sizeof(expected_buf));
	expected = be32_to_cpu(expected_buf);
	if (expected > buf_len) {
		log_warning("Too much data: %zu > %zu\n", expected, buf_len);
		goto out_err;
	}

	/* Now read the rest of the data */
	current = burstcnt;
	while (current < expected) {
		/* Read updated burst count and check status */
		if (cr50_i2c_wait_burststs(dev, mask, &burstcnt, &status) < 0) {
			log_warning("- burst failure1\n");
			goto out_err;
			}

		len = min(burstcnt, expected - current);
		if (cr50_i2c_read(dev, addr, buf + current, len) != 0) {
			log_warning("Read failed\n");
			goto out_err;
		}

		current += len;
	}

	if (cr50_i2c_wait_burststs(dev, TPM_STS_VALID, &burstcnt,
				   &status) < 0) {
		log_warning("- burst failure2\n");
		goto out_err;
	}
	if (status & TPM_STS_DATA_AVAIL) {
		log_warning("Data still available\n");
		goto out_err;
	}

	return current;

out_err:
	/* Abort current transaction if still pending */
	ret = cr50_i2c_status(dev);
	if (ret < 0)
		return ret;
	if (ret & TPM_STS_COMMAND_READY) {
		ret = cr50_i2c_ready(dev);
		if (ret)
			return ret;
	}

	return -EIO;
}

static int cr50_i2c_send(struct udevice *dev, const u8 *buf, size_t len)
{
	struct cr50_priv *priv = dev_get_priv(dev);
	int status;
	size_t burstcnt, limit, sent = 0;
	u8 tpm_go[4] = { TPM_STS_GO };
	ulong timeout;
	int ret;

	log_debug("len=%x\n", len);
	timeout = timer_get_us() + TIMEOUT_LONG_US;
	do {
		ret = cr50_i2c_status(dev);
		if (ret < 0)
			goto out_err;
		if (ret & TPM_STS_COMMAND_READY)
			break;

		if (timer_get_us() > timeout)
			goto out_err;

		ret = cr50_i2c_ready(dev);
		if (ret)
			goto out_err;
	} while (1);

	while (len > 0) {
		u8 mask = TPM_STS_VALID;

		/* Wait for data if this is not the first chunk */
		if (sent > 0)
			mask |= TPM_STS_DATA_EXPECT;

		if (cr50_i2c_wait_burststs(dev, mask, &burstcnt, &status) < 0)
			goto out_err;

		/*
		 * Use burstcnt - 1 to account for the address byte
		 * that is inserted by cr50_i2c_write()
		 */
		limit = min(burstcnt - 1, len);
		if (cr50_i2c_write(dev, tpm_data_fifo(priv->locality),
				   &buf[sent], limit) != 0) {
			log_warning("Write failed\n");
			goto out_err;
		}

		sent += limit;
		len -= limit;
	}

	/* Ensure TPM is not expecting more data */
	if (cr50_i2c_wait_burststs(dev, TPM_STS_VALID, &burstcnt, &status) < 0)
		goto out_err;
	if (status & TPM_STS_DATA_EXPECT) {
		log_warning("Data still expected\n");
		goto out_err;
	}

	/* Start the TPM command */
	ret = cr50_i2c_write(dev, tpm_sts(priv->locality), tpm_go,
			     sizeof(tpm_go));
	if (ret) {
		log_warning("Start command failed\n");
		goto out_err;
	}

	return sent;

out_err:
	/* Abort current transaction if still pending */
	ret = cr50_i2c_status(dev);

	if (ret < 0 || (ret & TPM_STS_COMMAND_READY)) {
		ret = cr50_i2c_ready(dev);
		if (ret)
			return ret;
	}

	return -EIO;
}

/**
 * process_reset() - Wait for the Cr50 to reset
 *
 * Cr50 processes reset requests asynchronously and conceivably could be busy
 * executing a long command and not reacting to the reset pulse for a while.
 *
 * This function will make sure that the AP does not proceed with boot until
 * TPM finished reset processing.
 *
 * @dev: Cr50 device
 * Return: 0 if OK, -EPERM if locality could not be taken
 */
static int process_reset(struct udevice *dev)
{
	const int loc = 0;
	u8 access;
	ulong start;

	/*
	 * Locality is released by TPM reset.
	 *
	 * If locality is taken at this point, this could be due to the fact
	 * that the TPM is performing a long operation and has not processed
	 * reset request yet. We'll wait up to CR50_TIMEOUT_INIT_MS and see if
	 * it releases locality when reset is processed.
	 */
	start = get_timer(0);
	do {
		const u8 mask = TPM_ACCESS_VALID | TPM_ACCESS_ACTIVE_LOCALITY;
		int ret;

		ret = cr50_i2c_read(dev, tpm_access(loc),
				    &access, sizeof(access));
		if (ret || ((access & mask) == mask)) {
			/*
			 * Don't bombard the chip with traffic; let it keep
			 * processing the command.
			 */
			mdelay(2);
			continue;
		}

		log_debug("TPM ready after %ld ms\n", get_timer(start));

		return 0;
	} while (get_timer(start) < TIMEOUT_INIT_MS);

	log_err("TPM failed to reset after %ld ms, status: %#x\n",
		get_timer(start), access);

	return -EPERM;
}

/*
 * Locality could be already claimed (if this is a later U-Boot phase and the
 * read-only U-Boot did not release it), or not yet claimed, if this is TPL or
 * the older read-only U-Boot did release it.
 */
static int claim_locality(struct udevice *dev, int loc)
{
	const u8 mask = TPM_ACCESS_VALID | TPM_ACCESS_ACTIVE_LOCALITY;
	struct cr50_priv *priv = dev_get_priv(dev);
	u8 access;
	int ret;

	ret = cr50_i2c_read(dev, tpm_access(loc), &access, sizeof(access));
	if (ret)
		return log_msg_ret("read1", ret);

	if ((access & mask) == mask) {
		log_warning("Locality already claimed\n");
		return 0;
	}

	access = TPM_ACCESS_REQUEST_USE;
	ret = cr50_i2c_write(dev, tpm_access(loc), &access, sizeof(access));
	if (ret)
		return log_msg_ret("write", ret);

	ret = cr50_i2c_read(dev, tpm_access(loc), &access, sizeof(access));
	if (ret)
		return log_msg_ret("read2", ret);

	if ((access & mask) != mask) {
		log_err("Failed to claim locality\n");
		return -EPERM;
	}
	log_debug("Claimed locality %d\n", loc);
	priv->locality = loc;

	return 0;
}

static int cr50_i2c_get_desc(struct udevice *dev, char *buf, int size)
{
	struct dm_i2c_chip *chip = dev_get_parent_plat(dev);
	struct cr50_priv *priv = dev_get_priv(dev);
	int len;

	len = snprintf(buf, size, "cr50 TPM 2.0 (i2c %02x id %x), ",
		       chip->chip_addr, priv->vendor >> 16);
	if (priv->use_irq) {
		len += snprintf(buf + len, size - len, "irq=%s/%ld",
				priv->irq.dev->name, priv->irq.id);
	} else if (dm_gpio_is_valid(&priv->ready_gpio)) {
		len += snprintf(buf + len, size - len, "gpio=%s/%u",
				priv->ready_gpio.dev->name,
				priv->ready_gpio.offset);
	} else {
		len += snprintf(buf + len, size - len, "delay=%d",
				TIMEOUT_NO_IRQ_US);
	}

	return len;
}

static int stringify_state(char *buf, int len, char *str, size_t max_size)
{
	struct tpm_vendor_state state;
	size_t text_size = 0;

	state.version = get_unaligned_be32(buf +
		offsetof(struct tpm_vendor_state, version));
	state.fail_line = get_unaligned_be32(buf +
		offsetof(struct tpm_vendor_state, fail_line));
	state.fail_code = get_unaligned_be32(buf +
		offsetof(struct tpm_vendor_state, fail_code));
	memcpy(state.func_name,
	       buf + offsetof(struct tpm_vendor_state, func_name),
	       sizeof(state.func_name));
	state.failed_tries = get_unaligned_be32(buf +
		offsetof(struct tpm_vendor_state, failed_tries));
	state.max_tries = get_unaligned_be32(buf +
		offsetof(struct tpm_vendor_state, max_tries));

	text_size += snprintf(str + text_size, max_size - text_size,
			      "v=%d", state.version);
	if (text_size >= max_size)
		return -ENOSPC;

	if (state.version > TPM_STATE_VERSION)
		text_size += snprintf(str + text_size,
				      max_size - text_size,
				      " not fully supported\n");
	if (text_size >= max_size)
		return -ENOSPC;

	if (state.version == 0)
		return -EINVAL;	/* This should never happen */

	text_size += snprintf(str + text_size,
			      max_size - text_size,
			      " failed_tries=%d max_tries=%d\n",
			      state.failed_tries, state.max_tries);
	if (text_size >= max_size)
		return -ENOSPC;

	if (state.fail_line) {
		/* make sure function name is zero terminated. */
		char func_name[sizeof(state.func_name) + 1];

		memcpy(func_name, state.func_name, sizeof(state.func_name));
		func_name[sizeof(state.func_name)] = '\0';

		text_size += snprintf(str + text_size,
				      max_size - text_size,
				      "tpm failed: f %s line %d code %d",
				      func_name,
				      state.fail_line,
				      state.fail_code);
		if (text_size >= max_size)
			return -ENOSPC;
	}

	return 0;
}

static int cr50_i2c_report_state(struct udevice *dev, char *str, int str_max)
{
	char buf[50];
	size_t buf_size = sizeof(buf);
	int ret;

	ret = tpm2_report_state(dev, TPM2_CR50_VENDOR_COMMAND,
				TPM2_CR50_SUB_CMD_REPORT_TPM_STATE,
				buf, &buf_size);
	if (ret)
		return ret;

	/* TPM responded as expected */
	ret = stringify_state(buf, buf_size, str, str_max);
	if (ret)
		return ret;

	return 0;
}

static int cr50_i2c_open(struct udevice *dev)
{
	char buf[80];
	int ret;

	ret = process_reset(dev);
	if (ret)
		return log_msg_ret("reset", ret);

	ret = claim_locality(dev, 0);
	if (ret)
		return log_msg_ret("claim", ret);

	cr50_i2c_get_desc(dev, buf, sizeof(buf));
	log_debug("%s\n", buf);

	return 0;
}

static int cr50_i2c_cleanup(struct udevice *dev)
{
	struct cr50_priv *priv = dev_get_priv(dev);

	log_debug("cleanup %d\n", priv->locality);
	if (priv->locality != -1)
		release_locality(dev, 1);

	return 0;
}

static int cr50_acpi_fill_ssdt(const struct udevice *dev, struct acpi_ctx *ctx)
{
	char scope[ACPI_PATH_MAX];
	char name[ACPI_NAME_MAX];
	const char *hid;
	int ret;

	ret = acpi_device_scope(dev, scope, sizeof(scope));
	if (ret)
		return log_msg_ret("scope", ret);
	ret = acpi_get_name(dev, name);
	if (ret)
		return log_msg_ret("name", ret);

	hid = dev_read_string(dev, "acpi,hid");
	if (!hid)
		return log_msg_ret("hid", ret);

	/* Device */
	acpigen_write_scope(ctx, scope);
	acpigen_write_device(ctx, name);
	acpigen_write_name_string(ctx, "_HID", hid);
	acpigen_write_name_integer(ctx, "_UID",
				   dev_read_u32_default(dev, "acpi,uid", 0));
	acpigen_write_name_string(ctx, "_DDN",
				  dev_read_string(dev, "acpi,ddn"));
	acpigen_write_sta(ctx, acpi_device_status(dev));

	/* Resources */
	acpigen_write_name(ctx, "_CRS");
	acpigen_write_resourcetemplate_header(ctx);
	ret = acpi_device_write_i2c_dev(ctx, dev);
	if (ret < 0)
		return log_msg_ret("i2c", ret);
	ret = acpi_device_write_interrupt_or_gpio(ctx, (struct udevice *)dev,
						  "ready-gpios");
	if (ret < 0)
		return log_msg_ret("irq_gpio", ret);

	acpigen_write_resourcetemplate_footer(ctx);

	acpigen_pop_len(ctx); /* Device */
	acpigen_pop_len(ctx); /* Scope */

	return 0;
}

enum {
	TPM_TIMEOUT_MS		= 5,
	SHORT_TIMEOUT_MS	= 750,
	LONG_TIMEOUT_MS		= 2000,
};

static int cr50_i2c_of_to_plat(struct udevice *dev)
{
	struct tpm_chip_priv *upriv = dev_get_uclass_priv(dev);
	struct cr50_priv *priv = dev_get_priv(dev);
	struct irq irq;
	int ret;

	upriv->version = TPM_V2;
	upriv->duration_ms[TPM_SHORT] = SHORT_TIMEOUT_MS;
	upriv->duration_ms[TPM_MEDIUM] = LONG_TIMEOUT_MS;
	upriv->duration_ms[TPM_LONG] = LONG_TIMEOUT_MS;
	upriv->retry_time_ms = TPM_TIMEOUT_MS;

	upriv->pcr_count = 32;
	upriv->pcr_select_min = 2;

	/* Optional GPIO to track when cr50 is ready */
	ret = irq_get_by_index(dev, 0, &irq);
	if (!ret) {
		priv->irq = irq;
		priv->use_irq = true;
	} else {
		ret = gpio_request_by_name(dev, "ready-gpios", 0,
					   &priv->ready_gpio, GPIOD_IS_IN);
		if (ret) {
			log_warning("Cr50 does not have an ready GPIO/interrupt (err=%d)\n",
				    ret);
		}
	}

	return 0;
}

static int cr50_i2c_probe(struct udevice *dev)
{
	struct cr50_priv *priv = dev_get_priv(dev);
	u32 vendor = 0;
	ulong start;

	/*
	 * 150ms should be enough to synchronise with the TPM even under the
	 * worst nested-reset-request conditions. In the vast majority of cases
	 * there will be no wait at all.
	 */
	start = get_timer(0);
	while (get_timer(start) < 150) {
		int ret;

		/* Exit once DID and VID verified */
		ret = cr50_i2c_read(dev, tpm_did_vid(0), (u8 *)&vendor, 4);
		if (!ret && vendor == CR50_DID_VID)
			break;

		/* TPM might be resetting; let's retry in a bit */
		mdelay(10);
	}
	if (vendor != CR50_DID_VID) {
		log_warning("DID_VID %08x not recognised\n", vendor);
		return log_msg_ret("vendor-id", -EXDEV);
	}
	priv->vendor = vendor;
	priv->locality = -1;
	log_debug("Cr50 ready\n");

	return 0;
}

struct acpi_ops cr50_acpi_ops = {
	.fill_ssdt	= cr50_acpi_fill_ssdt,
};

static const struct tpm_ops cr50_i2c_ops = {
	.open		= cr50_i2c_open,
	.get_desc	= cr50_i2c_get_desc,
	.report_state	= cr50_i2c_report_state,
	.send		= cr50_i2c_send,
	.recv		= cr50_i2c_recv,
	.cleanup	= cr50_i2c_cleanup,
};

static const struct udevice_id cr50_i2c_ids[] = {
	{ .compatible = "google,cr50" },
	{ }
};

U_BOOT_DRIVER(google_cr50) = {
	.name   = "google_cr50",
	.id     = UCLASS_TPM,
	.of_match = cr50_i2c_ids,
	.ops    = &cr50_i2c_ops,
	.of_to_plat	= cr50_i2c_of_to_plat,
	.probe	= cr50_i2c_probe,
	.remove	= cr50_i2c_cleanup,
	.priv_auto	= sizeof(struct cr50_priv),
	ACPI_OPS_PTR(&cr50_acpi_ops)
	.flags		= DM_FLAG_OS_PREPARE,
};
