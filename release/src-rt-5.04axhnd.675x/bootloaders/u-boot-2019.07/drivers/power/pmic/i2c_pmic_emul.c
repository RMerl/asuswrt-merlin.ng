// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2015 Samsung Electronics
 *  Przemyslaw Marczak  <p.marczak@samsung.com>
 */

#include <common.h>
#include <errno.h>
#include <dm.h>
#include <i2c.h>
#include <power/pmic.h>
#include <power/sandbox_pmic.h>

/**
 * struct sandbox_i2c_pmic_plat_data - platform data for the PMIC
 *
 * @rw_reg: PMICs register of the chip I/O transaction
 * @reg:    PMICs registers array
 */
struct sandbox_i2c_pmic_plat_data {
	u8 rw_reg, rw_idx;
	u8 reg_count;
	u8 trans_len;
	u8 buf_size;
	u8 *reg;
};

static int sandbox_i2c_pmic_read_data(struct udevice *emul, uchar chip,
				      uchar *buffer, int len)
{
	struct sandbox_i2c_pmic_plat_data *plat = dev_get_platdata(emul);

	if (plat->rw_idx + len > plat->buf_size) {
		pr_err("Request exceeds PMIC register range! Max register: %#x",
		      plat->reg_count);
		return -EFAULT;
	}

	debug("Read PMIC: %#x at register: %#x idx: %#x count: %d\n",
	      (unsigned int)chip & 0xff, plat->rw_reg, plat->rw_idx, len);

	memcpy(buffer, plat->reg + plat->rw_idx, len);

	return 0;
}

static int sandbox_i2c_pmic_write_data(struct udevice *emul, uchar chip,
				       uchar *buffer, int len,
				       bool next_is_read)
{
	struct sandbox_i2c_pmic_plat_data *plat = dev_get_platdata(emul);

	/* Probe only */
	if (!len)
		return 0;

	/* Set PMIC register for I/O */
	plat->rw_reg = *buffer;
	plat->rw_idx = plat->rw_reg * plat->trans_len;

	debug("Write PMIC: %#x at register: %#x idx: %#x count: %d\n",
	      (unsigned int)chip & 0xff, plat->rw_reg, plat->rw_idx, len);

	/* For read operation, set (write) only chip reg */
	if (next_is_read)
		return 0;

	buffer++;
	len--;

	if (plat->rw_idx + len > plat->buf_size) {
		pr_err("Request exceeds PMIC register range! Max register: %#x",
		      plat->reg_count);
	}

	memcpy(plat->reg + plat->rw_idx, buffer, len);

	return 0;
}

static int sandbox_i2c_pmic_xfer(struct udevice *emul, struct i2c_msg *msg,
				 int nmsgs)
{
	int ret = 0;

	for (; nmsgs > 0; nmsgs--, msg++) {
		bool next_is_read = nmsgs > 1 && (msg[1].flags & I2C_M_RD);
		if (msg->flags & I2C_M_RD) {
			ret = sandbox_i2c_pmic_read_data(emul, msg->addr,
							 msg->buf, msg->len);
		} else {
			ret = sandbox_i2c_pmic_write_data(emul, msg->addr,
							  msg->buf, msg->len,
							  next_is_read);
		}

		if (ret)
			break;
	}

	return ret;
}

static int sandbox_i2c_pmic_ofdata_to_platdata(struct udevice *emul)
{
	struct sandbox_i2c_pmic_plat_data *plat = dev_get_platdata(emul);
	struct udevice *pmic_dev = i2c_emul_get_device(emul);
	struct uc_pmic_priv *priv = dev_get_uclass_priv(pmic_dev);
	const u8 *reg_defaults;

	debug("%s:%d Setting PMIC default registers\n", __func__, __LINE__);
	plat->reg_count = pmic_reg_count(pmic_dev);
	plat->trans_len = priv->trans_len;
	plat->buf_size = plat->reg_count * plat->trans_len;

	plat->reg = calloc(1, plat->buf_size);
	if (!plat->reg) {
		debug("Canot allocate memory (%d B) for PMIC I2C emulation!\n",
		      plat->buf_size);
		return -ENOMEM;
	}

	reg_defaults = dev_read_u8_array_ptr(emul, "reg-defaults",
					     plat->buf_size);

	if (!reg_defaults) {
		pr_err("Property \"reg-defaults\" not found for device: %s!",
		      emul->name);
		free(plat->reg);
		return -EINVAL;
	}

	memcpy(plat->reg, reg_defaults, plat->buf_size);

	return 0;
}

struct dm_i2c_ops sandbox_i2c_pmic_emul_ops = {
	.xfer = sandbox_i2c_pmic_xfer,
};

static const struct udevice_id sandbox_i2c_pmic_ids[] = {
	{ .compatible = "sandbox,i2c-pmic" },
	{ }
};

U_BOOT_DRIVER(sandbox_i2c_pmic_emul) = {
	.name		= "sandbox_i2c_pmic_emul",
	.id		= UCLASS_I2C_EMUL,
	.of_match	= sandbox_i2c_pmic_ids,
	.ofdata_to_platdata = sandbox_i2c_pmic_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct sandbox_i2c_pmic_plat_data),
	.ops		= &sandbox_i2c_pmic_emul_ops,
};
