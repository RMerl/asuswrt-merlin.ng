// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#define LOG_CATEGORY UCLASS_MISC

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <log.h>
#include <misc.h>
#include <tee.h>
#include <asm/io.h>
#include <asm/arch/bsec.h>
#include <asm/arch/stm32mp1_smc.h>
#include <dm/device.h>
#include <dm/device_compat.h>
#include <linux/arm-smccc.h>
#include <linux/iopoll.h>
#include <linux/printk.h>

#define BSEC_OTP_UPPER_START		32
#define BSEC_TIMEOUT_US			10000

/* BSEC REGISTER OFFSET (base relative) */
#define BSEC_OTP_CONF_OFF		0x000
#define BSEC_OTP_CTRL_OFF		0x004
#define BSEC_OTP_WRDATA_OFF		0x008
#define BSEC_OTP_STATUS_OFF		0x00C
#define BSEC_OTP_LOCK_OFF		0x010
#define BSEC_DENABLE_OFF		0x014
#define BSEC_DISTURBED_OFF		0x01C
#define BSEC_ERROR_OFF			0x034
#define BSEC_WRLOCK_OFF			0x04C /* OTP write permananet lock */
#define BSEC_SPLOCK_OFF			0x064 /* OTP write sticky lock */
#define BSEC_SWLOCK_OFF			0x07C /* shadow write sticky lock */
#define BSEC_SRLOCK_OFF			0x094 /* shadow read sticky lock */
#define BSEC_OTP_DATA_OFF		0x200

/* BSEC_CONFIGURATION Register MASK */
#define BSEC_CONF_POWER_UP		0x001

/* BSEC_CONTROL Register */
#define BSEC_READ			0x000
#define BSEC_WRITE			0x100
#define BSEC_LOCK			0x200

/* LOCK Register */
#define OTP_LOCK_MASK			0x1F
#define OTP_LOCK_BANK_SHIFT		0x05
#define OTP_LOCK_BIT_MASK		0x01

/* STATUS Register */
#define BSEC_MODE_BUSY_MASK		0x08
#define BSEC_MODE_PROGFAIL_MASK		0x10
#define BSEC_MODE_PWR_MASK		0x20

/* DENABLE Register */
#define BSEC_DENABLE_DBGSWENABLE	BIT(10)

/*
 * OTP Lock services definition
 * Value must corresponding to the bit number in the register
 */
#define BSEC_LOCK_PROGRAM		0x04

#define PTA_BSEC_UUID { 0x94cf71ad, 0x80e6, 0x40b5, \
	{ 0xa7, 0xc6, 0x3d, 0xc5, 0x01, 0xeb, 0x28, 0x03 } }

/*
 * Read OTP memory
 *
 * [in]		value[0].a		OTP start offset in byte
 * [in]		value[0].b		Access type (0:shadow, 1:fuse, 2:lock)
 * [out]	memref[1].buffer	Output buffer to store read values
 * [out]	memref[1].size		Size of OTP to be read
 *
 * Return codes:
 * TEE_SUCCESS - Invoke command success
 * TEE_ERROR_BAD_PARAMETERS - Incorrect input param
 * TEE_ERROR_ACCESS_DENIED - OTP not accessible by caller
 */
#define PTA_BSEC_READ_MEM		0x0

/*
 * Write OTP memory
 *
 * [in]		value[0].a		OTP start offset in byte
 * [in]		value[0].b		Access type (0:shadow, 1:fuse, 2:lock)
 * [in]		memref[1].buffer	Input buffer to read values
 * [in]		memref[1].size		Size of OTP to be written
 *
 * Return codes:
 * TEE_SUCCESS - Invoke command success
 * TEE_ERROR_BAD_PARAMETERS - Incorrect input param
 * TEE_ERROR_ACCESS_DENIED - OTP not accessible by caller
 */
#define PTA_BSEC_WRITE_MEM		0x1

/* value of PTA_BSEC access type = value[in] b */
#define SHADOW_ACCESS			0
#define FUSE_ACCESS			1
#define LOCK_ACCESS			2

/**
 * bsec_lock() - manage lock for each type SR/SP/SW
 * @address: address of bsec IP register
 * @otp: otp number (0 - BSEC_OTP_MAX_VALUE)
 * Return: true if locked else false
 */
static bool bsec_read_lock(void __iomem *address, u32 otp)
{
	u32 bit;
	u32 bank;

	bit = 1 << (otp & OTP_LOCK_MASK);
	bank = ((otp >> OTP_LOCK_BANK_SHIFT) & OTP_LOCK_MASK) * sizeof(u32);

	return !!(readl((address + bank)) & bit);
}

/**
 * bsec_check_error() - Check status of one otp
 * @base: base address of bsec IP
 * @otp: otp number (0 - BSEC_OTP_MAX_VALUE)
 * Return: 0 if no error, -EAGAIN or -ENOTSUPP
 */
static u32 bsec_check_error(void __iomem *base, u32 otp)
{
	u32 bit;
	u32 bank;

	bit = 1 << (otp & OTP_LOCK_MASK);
	bank = ((otp >> OTP_LOCK_BANK_SHIFT) & OTP_LOCK_MASK) * sizeof(u32);

	if (readl(base + BSEC_DISTURBED_OFF + bank) & bit)
		return -EAGAIN;
	else if (readl(base + BSEC_ERROR_OFF + bank) & bit)
		return -ENOTSUPP;

	return 0;
}

/**
 * bsec_read_SR_lock() - read SR lock (Shadowing)
 * @base: base address of bsec IP
 * @otp: otp number (0 - BSEC_OTP_MAX_VALUE)
 * Return: true if locked else false
 */
static bool bsec_read_SR_lock(void __iomem *base, u32 otp)
{
	return bsec_read_lock(base + BSEC_SRLOCK_OFF, otp);
}

/**
 * bsec_read_SP_lock() - read SP lock (program Lock)
 * @base: base address of bsec IP
 * @otp: otp number (0 - BSEC_OTP_MAX_VALUE)
 * Return: true if locked else false
 */
static bool bsec_read_SP_lock(void __iomem *base, u32 otp)
{
	return bsec_read_lock(base + BSEC_SPLOCK_OFF, otp);
}

/**
 * bsec_SW_lock() - manage SW lock (Write in Shadow)
 * @base: base address of bsec IP
 * @otp: otp number (0 - BSEC_OTP_MAX_VALUE)
 * Return: true if locked else false
 */
static bool bsec_read_SW_lock(void __iomem *base, u32 otp)
{
	return bsec_read_lock(base + BSEC_SWLOCK_OFF, otp);
}

/**
 * bsec_power_safmem() - Activate or deactivate safmem power
 * @base: base address of bsec IP
 * @power: true to power up , false to power down
 * Return: 0 if succeed
 */
static int bsec_power_safmem(void __iomem *base, bool power)
{
	u32 val;
	u32 mask;

	if (power) {
		setbits_le32(base + BSEC_OTP_CONF_OFF, BSEC_CONF_POWER_UP);
		mask = BSEC_MODE_PWR_MASK;
	} else {
		clrbits_le32(base + BSEC_OTP_CONF_OFF, BSEC_CONF_POWER_UP);
		mask = 0;
	}

	/* waiting loop */
	return readl_poll_timeout(base + BSEC_OTP_STATUS_OFF,
				  val, (val & BSEC_MODE_PWR_MASK) == mask,
				  BSEC_TIMEOUT_US);
}

/**
 * bsec_shadow_register() - copy safmen otp to bsec data
 * @dev: bsec IP device
 * @base: base address of bsec IP
 * @otp: otp number (0 - BSEC_OTP_MAX_VALUE)
 * Return: 0 if no error
 */
static int bsec_shadow_register(struct udevice *dev, void __iomem *base, u32 otp)
{
	u32 val;
	int ret;
	bool power_up = false;

	/* check if shadowing of otp is locked */
	if (bsec_read_SR_lock(base, otp))
		dev_dbg(dev, "OTP %d is locked and refreshed with 0\n",
			otp);

	/* check if safemem is power up */
	val = readl(base + BSEC_OTP_STATUS_OFF);
	if (!(val & BSEC_MODE_PWR_MASK)) {
		ret = bsec_power_safmem(base, true);
		if (ret)
			return ret;
		power_up = true;
	}
	/* set BSEC_OTP_CTRL_OFF with the otp value*/
	writel(otp | BSEC_READ, base + BSEC_OTP_CTRL_OFF);

	/* check otp status*/
	ret = readl_poll_timeout(base + BSEC_OTP_STATUS_OFF,
				 val, (val & BSEC_MODE_BUSY_MASK) == 0,
				 BSEC_TIMEOUT_US);
	if (ret)
		return ret;

	ret = bsec_check_error(base, otp);

	if (power_up)
		bsec_power_safmem(base, false);

	return ret;
}

/**
 * bsec_read_shadow() - read an otp data value from shadow
 * @dev: bsec IP device
 * @base: base address of bsec IP
 * @val: read value
 * @otp: otp number (0 - BSEC_OTP_MAX_VALUE)
 * Return: 0 if no error
 */
static int bsec_read_shadow(struct udevice *dev, void __iomem *base, u32 *val,
			    u32 otp)
{
	*val = readl(base + BSEC_OTP_DATA_OFF + otp * sizeof(u32));

	return bsec_check_error(base, otp);
}

/**
 * bsec_write_shadow() - write value in BSEC data register in shadow
 * @dev: bsec IP device
 * @base: base address of bsec IP
 * @val: value to write
 * @otp: otp number (0 - BSEC_OTP_MAX_VALUE)
 * Return: 0 if no error
 */
static int bsec_write_shadow(struct udevice *dev, void __iomem *base, u32 val, u32 otp)
{
	/* check if programming of otp is locked */
	if (bsec_read_SW_lock(base, otp))
		dev_dbg(dev, "OTP %d is lock, write will be ignore\n", otp);

	writel(val, base + BSEC_OTP_DATA_OFF + otp * sizeof(u32));

	return bsec_check_error(base, otp);
}

/**
 * bsec_program_otp() - program a bit in SAFMEM
 * @dev: bsec IP device
 * @base: base address of bsec IP
 * @val: value to program
 * @otp: otp number (0 - BSEC_OTP_MAX_VALUE)
 * after the function the otp data is not refreshed in shadow
 * Return: 0 if no error
 */
static int bsec_program_otp(struct udevice *dev, void __iomem *base, u32 val, u32 otp)
{
	u32 ret;
	bool power_up = false;

	if (bsec_read_SP_lock(base, otp))
		dev_dbg(dev, "OTP %d locked, prog will be ignore\n", otp);

	if (readl(base + BSEC_OTP_LOCK_OFF) & (1 << BSEC_LOCK_PROGRAM))
		dev_dbg(dev, "Global lock, prog will be ignore\n");

	/* check if safemem is power up */
	if (!(readl(base + BSEC_OTP_STATUS_OFF) & BSEC_MODE_PWR_MASK)) {
		ret = bsec_power_safmem(base, true);
		if (ret)
			return ret;

		power_up = true;
	}
	/* set value in write register*/
	writel(val, base + BSEC_OTP_WRDATA_OFF);

	/* set BSEC_OTP_CTRL_OFF with the otp value */
	writel(otp | BSEC_WRITE, base + BSEC_OTP_CTRL_OFF);

	/* check otp status*/
	ret = readl_poll_timeout(base + BSEC_OTP_STATUS_OFF,
				 val, (val & BSEC_MODE_BUSY_MASK) == 0,
				 BSEC_TIMEOUT_US);
	if (ret)
		return ret;

	if (val & BSEC_MODE_PROGFAIL_MASK)
		ret = -EACCES;
	else
		ret = bsec_check_error(base, otp);

	if (power_up)
		bsec_power_safmem(base, false);

	return ret;
}

/**
 * bsec_permanent_lock_otp() - permanent lock of OTP in SAFMEM
 * @dev: bsec IP device
 * @base: base address of bsec IP
 * @otp: otp number (0 - BSEC_OTP_MAX_VALUE)
 * Return: 0 if no error
 */
static int bsec_permanent_lock_otp(struct udevice *dev, void __iomem *base, uint32_t otp)
{
	int ret;
	bool power_up = false;
	u32 val, addr;

	/* check if safemem is power up */
	if (!(readl(base + BSEC_OTP_STATUS_OFF) & BSEC_MODE_PWR_MASK)) {
		ret = bsec_power_safmem(base, true);
		if (ret)
			return ret;

		power_up = true;
	}

	/*
	 * low OTPs = 2 bits word for low OTPs, 1 bits per word for upper OTP
	 * and only 16 bits used in WRDATA
	 */
	if (otp < BSEC_OTP_UPPER_START) {
		addr = otp / 8;
		val = 0x03 << ((otp * 2) & 0xF);
	} else {
		addr = BSEC_OTP_UPPER_START / 8 +
		       ((otp - BSEC_OTP_UPPER_START) / 16);
		val = 0x01 << (otp & 0xF);
	}

	/* set value in write register*/
	writel(val, base + BSEC_OTP_WRDATA_OFF);

	/* set BSEC_OTP_CTRL_OFF with the otp addr and lock request*/
	writel(addr | BSEC_WRITE | BSEC_LOCK, base + BSEC_OTP_CTRL_OFF);

	/* check otp status*/
	ret = readl_poll_timeout(base + BSEC_OTP_STATUS_OFF,
				 val, (val & BSEC_MODE_BUSY_MASK) == 0,
				 BSEC_TIMEOUT_US);
	if (ret)
		return ret;

	if (val & BSEC_MODE_PROGFAIL_MASK)
		ret = -EACCES;
	else
		ret = bsec_check_error(base, otp);

	if (power_up)
		bsec_power_safmem(base, false);

	return ret;
}

/* BSEC MISC driver *******************************************************/
struct stm32mp_bsec_plat {
	void __iomem *base;
};

struct stm32mp_bsec_priv {
	struct udevice *tee;
};

struct stm32mp_bsec_drvdata {
	int size;
	bool ta;
};

static int stm32mp_bsec_read_otp(struct udevice *dev, u32 *val, u32 otp)
{
	struct stm32mp_bsec_plat *plat;
	u32 tmp_data = 0;
	int ret;

	if (IS_ENABLED(CONFIG_ARM_SMCCC) && !IS_ENABLED(CONFIG_SPL_BUILD))
		return stm32_smc(STM32_SMC_BSEC,
				 STM32_SMC_READ_OTP,
				 otp, 0, val);

	plat = dev_get_plat(dev);

	/* read current shadow value */
	ret = bsec_read_shadow(dev, plat->base, &tmp_data, otp);
	if (ret)
		return ret;

	/* copy otp in shadow */
	ret = bsec_shadow_register(dev, plat->base, otp);
	if (ret)
		return ret;

	ret = bsec_read_shadow(dev, plat->base, val, otp);
	if (ret)
		return ret;

	/* restore shadow value */
	ret = bsec_write_shadow(dev, plat->base, tmp_data, otp);

	return ret;
}

static int stm32mp_bsec_read_shadow(struct udevice *dev, u32 *val, u32 otp)
{
	struct stm32mp_bsec_plat *plat;

	if (IS_ENABLED(CONFIG_ARM_SMCCC) && !IS_ENABLED(CONFIG_SPL_BUILD))
		return stm32_smc(STM32_SMC_BSEC,
				 STM32_SMC_READ_SHADOW,
				 otp, 0, val);

	plat = dev_get_plat(dev);

	return bsec_read_shadow(dev, plat->base, val, otp);
}

static int stm32mp_bsec_read_lock(struct udevice *dev, u32 *val, u32 otp)
{
	struct stm32mp_bsec_plat *plat = dev_get_plat(dev);
	u32 wrlock;

	/* return OTP permanent write lock status */
	wrlock = bsec_read_lock(plat->base + BSEC_WRLOCK_OFF, otp);

	*val = 0;
	if (wrlock)
		*val = BSEC_LOCK_PERM;

	return 0;
}

static int stm32mp_bsec_write_otp(struct udevice *dev, u32 val, u32 otp)
{
	struct stm32mp_bsec_plat *plat;

	if (IS_ENABLED(CONFIG_ARM_SMCCC) && !IS_ENABLED(CONFIG_SPL_BUILD))
		return stm32_smc_exec(STM32_SMC_BSEC,
				      STM32_SMC_PROG_OTP,
				      otp, val);

	plat = dev_get_plat(dev);

	return bsec_program_otp(dev, plat->base, val, otp);

}

static int stm32mp_bsec_write_shadow(struct udevice *dev, u32 val, u32 otp)
{
	struct stm32mp_bsec_plat *plat;

	if (IS_ENABLED(CONFIG_ARM_SMCCC) && !IS_ENABLED(CONFIG_SPL_BUILD))
		return stm32_smc_exec(STM32_SMC_BSEC,
				      STM32_SMC_WRITE_SHADOW,
				      otp, val);

	plat = dev_get_plat(dev);

	return bsec_write_shadow(dev, plat->base, val, otp);
}

static int stm32mp_bsec_write_lock(struct udevice *dev, u32 val, u32 otp)
{
	struct stm32mp_bsec_plat *plat;

	/* only permanent write lock is supported in U-Boot */
	if (!(val & BSEC_LOCK_PERM)) {
		dev_dbg(dev, "lock option without BSEC_LOCK_PERM: %x\n", val);
		return 0; /* nothing to do */
	}

	if (IS_ENABLED(CONFIG_ARM_SMCCC) && !IS_ENABLED(CONFIG_SPL_BUILD))
		return stm32_smc_exec(STM32_SMC_BSEC,
				      STM32_SMC_WRLOCK_OTP,
				      otp, 0);

	plat = dev_get_plat(dev);

	return bsec_permanent_lock_otp(dev, plat->base, otp);
}

static int bsec_pta_open_session(struct udevice *tee, u32 *tee_session)
{
	const struct tee_optee_ta_uuid uuid = PTA_BSEC_UUID;
	struct tee_open_session_arg arg;
	int rc;

	memset(&arg, 0, sizeof(arg));
	tee_optee_ta_uuid_to_octets(arg.uuid, &uuid);
	arg.clnt_login = TEE_LOGIN_REE_KERNEL;
	rc = tee_open_session(tee, &arg, 0, NULL);
	if (rc < 0)
		return -ENODEV;

	*tee_session = arg.session;

	return 0;
}

static int bsec_optee_open(struct udevice *dev)
{
	struct stm32mp_bsec_priv *priv = dev_get_priv(dev);
	struct udevice *tee;
	u32 tee_session;
	int rc;

	tee = tee_find_device(NULL, NULL, NULL, NULL);
	if (!tee)
		return -ENODEV;

	/* try to open the STM32 BSEC TA */
	rc = bsec_pta_open_session(tee, &tee_session);
	if (rc)
		return rc;

	tee_close_session(tee, tee_session);

	priv->tee = tee;

	return 0;
}

static int bsec_optee_pta(struct udevice *dev, int cmd, int type, int offset,
			  void *buff, ulong size)
{
	struct stm32mp_bsec_priv *priv = dev_get_priv(dev);
	u32 tee_session;
	struct tee_invoke_arg arg;
	struct tee_param param[2];
	struct tee_shm *fw_shm;
	int rc;

	rc = bsec_pta_open_session(priv->tee, &tee_session);
	if (rc)
		return rc;

	rc = tee_shm_register(priv->tee, buff, size, 0, &fw_shm);
	if (rc)
		goto close_session;

	memset(&arg, 0, sizeof(arg));
	arg.func = cmd;
	arg.session = tee_session;

	memset(param, 0, sizeof(param));

	param[0].attr = TEE_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[0].u.value.a = offset;
	param[0].u.value.b = type;

	if (cmd == PTA_BSEC_WRITE_MEM)
		param[1].attr = TEE_PARAM_ATTR_TYPE_MEMREF_INPUT;
	else
		param[1].attr = TEE_PARAM_ATTR_TYPE_MEMREF_OUTPUT;

	param[1].u.memref.shm = fw_shm;
	param[1].u.memref.size = size;

	rc = tee_invoke_func(priv->tee, &arg, 2, param);
	if (rc < 0 || arg.ret != 0) {
		dev_err(priv->tee,
			"PTA_BSEC invoke failed TEE err: %x, err:%x\n",
			arg.ret, rc);
		if (!rc)
			rc = -EIO;
	}

	tee_shm_free(fw_shm);

close_session:
	tee_close_session(priv->tee, tee_session);

	return rc;
}

static int stm32mp_bsec_read(struct udevice *dev, int offset,
			     void *buf, int size)
{
	struct stm32mp_bsec_priv *priv = dev_get_priv(dev);
	struct stm32mp_bsec_drvdata *data = (struct stm32mp_bsec_drvdata *)dev_get_driver_data(dev);
	int ret;
	int i;
	bool shadow = true, lock = false;
	int nb_otp = size / sizeof(u32);
	int otp, cmd;
	unsigned int offs = offset;

	if (offs >= STM32_BSEC_LOCK_OFFSET) {
		offs -= STM32_BSEC_LOCK_OFFSET;
		lock = true;
	} else if (offs >= STM32_BSEC_OTP_OFFSET) {
		offs -= STM32_BSEC_OTP_OFFSET;
		shadow = false;
	}

	if ((offs % 4) || (size % 4) || !size)
		return -EINVAL;

	if (IS_ENABLED(CONFIG_OPTEE) && priv->tee) {
		cmd = FUSE_ACCESS;
		if (shadow)
			cmd = SHADOW_ACCESS;
		if (lock)
			cmd = LOCK_ACCESS;
		ret = bsec_optee_pta(dev, PTA_BSEC_READ_MEM, cmd, offs, buf, size);
		if (ret)
			return ret;

		return size;
	}

	otp = offs / sizeof(u32);

	for (i = otp; i < (otp + nb_otp) && i < data->size; i++) {
		u32 *addr = &((u32 *)buf)[i - otp];

		if (lock)
			ret = stm32mp_bsec_read_lock(dev, addr, i);
		else if (shadow)
			ret = stm32mp_bsec_read_shadow(dev, addr, i);
		else
			ret = stm32mp_bsec_read_otp(dev, addr, i);

		if (ret)
			break;
	}
	if (ret)
		return ret;
	else
		return (i - otp) * 4;
}

static int stm32mp_bsec_write(struct udevice *dev, int offset,
			      const void *buf, int size)
{
	struct stm32mp_bsec_priv *priv = dev_get_priv(dev);
	struct stm32mp_bsec_drvdata *data = (struct stm32mp_bsec_drvdata *)dev_get_driver_data(dev);
	int ret = 0;
	int i;
	bool shadow = true, lock = false;
	int nb_otp = size / sizeof(u32);
	int otp, cmd;
	unsigned int offs = offset;

	if (offs >= STM32_BSEC_LOCK_OFFSET) {
		offs -= STM32_BSEC_LOCK_OFFSET;
		lock = true;
	} else if (offs >= STM32_BSEC_OTP_OFFSET) {
		offs -= STM32_BSEC_OTP_OFFSET;
		shadow = false;
	}

	if ((offs % 4) || (size % 4) || !size)
		return -EINVAL;

	if (IS_ENABLED(CONFIG_OPTEE) && priv->tee) {
		cmd = FUSE_ACCESS;
		if (shadow)
			cmd = SHADOW_ACCESS;
		if (lock)
			cmd = LOCK_ACCESS;
		ret = bsec_optee_pta(dev, PTA_BSEC_WRITE_MEM, cmd, offs, (void *)buf, size);
		if (ret)
			return ret;

		return size;
	}

	otp = offs / sizeof(u32);

	for (i = otp; i < otp + nb_otp && i < data->size; i++) {
		u32 *val = &((u32 *)buf)[i - otp];

		if (lock)
			ret = stm32mp_bsec_write_lock(dev, *val, i);
		else if (shadow)
			ret = stm32mp_bsec_write_shadow(dev, *val, i);
		else
			ret = stm32mp_bsec_write_otp(dev, *val, i);
		if (ret)
			break;
	}
	if (ret)
		return ret;
	else
		return (i - otp) * 4;
}

static const struct misc_ops stm32mp_bsec_ops = {
	.read = stm32mp_bsec_read,
	.write = stm32mp_bsec_write,
};

static int stm32mp_bsec_of_to_plat(struct udevice *dev)
{
	struct stm32mp_bsec_plat *plat = dev_get_plat(dev);

	plat->base = dev_read_addr_ptr(dev);

	return 0;
}

static int stm32mp_bsec_probe(struct udevice *dev)
{
	struct stm32mp_bsec_drvdata *data = (struct stm32mp_bsec_drvdata *)dev_get_driver_data(dev);
	int otp;
	struct stm32mp_bsec_plat *plat;
	struct clk_bulk clk_bulk;
	int ret;

	ret = clk_get_bulk(dev, &clk_bulk);
	if (!ret) {
		ret = clk_enable_bulk(&clk_bulk);
		if (ret)
			return ret;
	}

	if (IS_ENABLED(CONFIG_OPTEE))
		ret = bsec_optee_open(dev);
	else
		ret = -ENOTSUPP;
	/* failed if OP-TEE TA is required */
	if (data->ta && !ret)
		return ret;

	/*
	 * update unlocked shadow for OTP cleared by the rom code
	 * only executed in SPL, it is done in TF-A for TFABOOT
	 */
	if (IS_ENABLED(CONFIG_SPL_BUILD) && !data->ta) {
		plat = dev_get_plat(dev);

		/* here 57 is the value for STM32MP15x ROM code, only MPU with SPL support*/
		for (otp = 57; otp < data->size; otp++)
			if (!bsec_read_SR_lock(plat->base, otp))
				bsec_shadow_register(dev, plat->base, otp);
	}

	return 0;
}

static const struct stm32mp_bsec_drvdata stm32mp13_data = {
	.size = 96,
	.ta = true,
};

static const struct stm32mp_bsec_drvdata stm32mp15_data = {
	.size = 96,
	.ta = false,
};

static const struct stm32mp_bsec_drvdata stm32mp25_data = {
	.size = 368, /* 384 but no access to HWKEY and STM32PRVKEY */
	.ta = true,
};

static const struct udevice_id stm32mp_bsec_ids[] = {
	{ .compatible = "st,stm32mp13-bsec", .data = (ulong)&stm32mp13_data},
	{ .compatible = "st,stm32mp15-bsec", .data = (ulong)&stm32mp15_data},
	{ .compatible = "st,stm32mp25-bsec", .data = (ulong)&stm32mp25_data},
	{}
};

U_BOOT_DRIVER(stm32mp_bsec) = {
	.name = "stm32mp_bsec",
	.id = UCLASS_MISC,
	.of_match = stm32mp_bsec_ids,
	.of_to_plat = stm32mp_bsec_of_to_plat,
	.plat_auto = sizeof(struct stm32mp_bsec_plat),
	.priv_auto = sizeof(struct stm32mp_bsec_priv),
	.ops = &stm32mp_bsec_ops,
	.probe = stm32mp_bsec_probe,
};

bool bsec_dbgswenable(void)
{
	struct udevice *dev;
	struct stm32mp_bsec_plat *plat;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_MISC,
					  DM_DRIVER_GET(stm32mp_bsec), &dev);
	if (ret || !dev) {
		log_debug("bsec driver not available\n");
		return false;
	}

	plat = dev_get_plat(dev);
	if (readl(plat->base + BSEC_DENABLE_OFF) & BSEC_DENABLE_DBGSWENABLE)
		return true;

	return false;
}

u32 get_otp(int index, int shift, int mask)
{
	int ret;
	struct udevice *dev;
	u32 otp = 0;

	ret = uclass_get_device_by_driver(UCLASS_MISC,
					  DM_DRIVER_GET(stm32mp_bsec),
					  &dev);

	if (!ret)
		ret = misc_read(dev, STM32_BSEC_SHADOW(index),
				&otp, sizeof(otp));

	return (otp >> shift) & mask;
}
