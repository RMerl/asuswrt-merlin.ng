// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <dm.h>
#include <misc.h>
#include <asm/io.h>
#include <asm/arch/stm32mp1_smc.h>
#include <linux/arm-smccc.h>
#include <linux/iopoll.h>

#define BSEC_OTP_MAX_VALUE		95

#ifndef CONFIG_STM32MP1_TRUSTED
#define BSEC_TIMEOUT_US			10000

/* BSEC REGISTER OFFSET (base relative) */
#define BSEC_OTP_CONF_OFF		0x000
#define BSEC_OTP_CTRL_OFF		0x004
#define BSEC_OTP_WRDATA_OFF		0x008
#define BSEC_OTP_STATUS_OFF		0x00C
#define BSEC_OTP_LOCK_OFF		0x010
#define BSEC_DISTURBED_OFF		0x01C
#define BSEC_ERROR_OFF			0x034
#define BSEC_SPLOCK_OFF			0x064 /* Program safmem sticky lock */
#define BSEC_SWLOCK_OFF			0x07C /* write in OTP sticky lock */
#define BSEC_SRLOCK_OFF			0x094 /* shadowing sticky lock */
#define BSEC_OTP_DATA_OFF		0x200

/* BSEC_CONFIGURATION Register MASK */
#define BSEC_CONF_POWER_UP		0x001

/* BSEC_CONTROL Register */
#define BSEC_READ			0x000
#define BSEC_WRITE			0x100

/* LOCK Register */
#define OTP_LOCK_MASK			0x1F
#define OTP_LOCK_BANK_SHIFT		0x05
#define OTP_LOCK_BIT_MASK		0x01

/* STATUS Register */
#define BSEC_MODE_BUSY_MASK		0x08
#define BSEC_MODE_PROGFAIL_MASK		0x10
#define BSEC_MODE_PWR_MASK		0x20

/*
 * OTP Lock services definition
 * Value must corresponding to the bit number in the register
 */
#define BSEC_LOCK_PROGRAM		0x04

/**
 * bsec_check_error() - Check status of one otp
 * @base: base address of bsec IP
 * @otp: otp number (0 - BSEC_OTP_MAX_VALUE)
 * Return: 0 if no error, -EAGAIN or -ENOTSUPP
 */
static u32 bsec_check_error(u32 base, u32 otp)
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
 * bsec_lock() - manage lock for each type SR/SP/SW
 * @address: address of bsec IP register
 * @otp: otp number (0 - BSEC_OTP_MAX_VALUE)
 * Return: true if locked else false
 */
static bool bsec_read_lock(u32 address, u32 otp)
{
	u32 bit;
	u32 bank;

	bit = 1 << (otp & OTP_LOCK_MASK);
	bank = ((otp >> OTP_LOCK_BANK_SHIFT) & OTP_LOCK_MASK) * sizeof(u32);

	return !!(readl(address + bank) & bit);
}

/**
 * bsec_read_SR_lock() - read SR lock (Shadowing)
 * @base: base address of bsec IP
 * @otp: otp number (0 - BSEC_OTP_MAX_VALUE)
 * Return: true if locked else false
 */
static bool bsec_read_SR_lock(u32 base, u32 otp)
{
	return bsec_read_lock(base + BSEC_SRLOCK_OFF, otp);
}

/**
 * bsec_read_SP_lock() - read SP lock (program Lock)
 * @base: base address of bsec IP
 * @otp: otp number (0 - BSEC_OTP_MAX_VALUE)
 * Return: true if locked else false
 */
static bool bsec_read_SP_lock(u32 base, u32 otp)
{
	return bsec_read_lock(base + BSEC_SPLOCK_OFF, otp);
}

/**
 * bsec_SW_lock() - manage SW lock (Write in Shadow)
 * @base: base address of bsec IP
 * @otp: otp number (0 - BSEC_OTP_MAX_VALUE)
 * Return: true if locked else false
 */
static bool bsec_read_SW_lock(u32 base, u32 otp)
{
	return bsec_read_lock(base + BSEC_SWLOCK_OFF, otp);
}

/**
 * bsec_power_safmem() - Activate or deactivate safmem power
 * @base: base address of bsec IP
 * @power: true to power up , false to power down
 * Return: 0 if succeed
 */
static int bsec_power_safmem(u32 base, bool power)
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
 * @base: base address of bsec IP
 * @otp: otp number (0 - BSEC_OTP_MAX_VALUE)
 * Return: 0 if no error
 */
static int bsec_shadow_register(u32 base, u32 otp)
{
	u32 val;
	int ret;
	bool power_up = false;

	/* check if shadowing of otp is locked */
	if (bsec_read_SR_lock(base, otp))
		pr_debug("bsec : OTP %d is locked and refreshed with 0\n", otp);

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
 * @base: base address of bsec IP
 * @val: read value
 * @otp: otp number (0 - BSEC_OTP_MAX_VALUE)
 * Return: 0 if no error
 */
static int bsec_read_shadow(u32 base, u32 *val, u32 otp)
{
	*val = readl(base + BSEC_OTP_DATA_OFF + otp * sizeof(u32));

	return bsec_check_error(base, otp);
}

/**
 * bsec_write_shadow() - write value in BSEC data register in shadow
 * @base: base address of bsec IP
 * @val: value to write
 * @otp: otp number (0 - BSEC_OTP_MAX_VALUE)
 * Return: 0 if no error
 */
static int bsec_write_shadow(u32 base, u32 val, u32 otp)
{
	/* check if programming of otp is locked */
	if (bsec_read_SW_lock(base, otp))
		pr_debug("bsec : OTP %d is lock, write will be ignore\n", otp);

	writel(val, base + BSEC_OTP_DATA_OFF + otp * sizeof(u32));

	return bsec_check_error(base, otp);
}

/**
 * bsec_program_otp() - program a bit in SAFMEM
 * @base: base address of bsec IP
 * @val: value to program
 * @otp: otp number (0 - BSEC_OTP_MAX_VALUE)
 * after the function the otp data is not refreshed in shadow
 * Return: 0 if no error
 */
static int bsec_program_otp(long base, u32 val, u32 otp)
{
	u32 ret;
	bool power_up = false;

	if (bsec_read_SP_lock(base, otp))
		pr_debug("bsec : OTP %d locked, prog will be ignore\n", otp);

	if (readl(base + BSEC_OTP_LOCK_OFF) & (1 << BSEC_LOCK_PROGRAM))
		pr_debug("bsec : Global lock, prog will be ignore\n");

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
#endif /* CONFIG_STM32MP1_TRUSTED */

/* BSEC MISC driver *******************************************************/
struct stm32mp_bsec_platdata {
	u32 base;
};

static int stm32mp_bsec_read_otp(struct udevice *dev, u32 *val, u32 otp)
{
#ifdef CONFIG_STM32MP1_TRUSTED
	return stm32_smc(STM32_SMC_BSEC,
			 STM32_SMC_READ_OTP,
			 otp, 0, val);
#else
	struct stm32mp_bsec_platdata *plat = dev_get_platdata(dev);
	u32 tmp_data = 0;
	int ret;

	/* read current shadow value */
	ret = bsec_read_shadow(plat->base, &tmp_data, otp);
	if (ret)
		return ret;

	/* copy otp in shadow */
	ret = bsec_shadow_register(plat->base, otp);
	if (ret)
		return ret;

	ret = bsec_read_shadow(plat->base, val, otp);
	if (ret)
		return ret;

	/* restore shadow value */
	ret = bsec_write_shadow(plat->base, tmp_data, otp);
	return ret;
#endif
}

static int stm32mp_bsec_read_shadow(struct udevice *dev, u32 *val, u32 otp)
{
#ifdef CONFIG_STM32MP1_TRUSTED
	return stm32_smc(STM32_SMC_BSEC,
			 STM32_SMC_READ_SHADOW,
			 otp, 0, val);
#else
	struct stm32mp_bsec_platdata *plat = dev_get_platdata(dev);

	return bsec_read_shadow(plat->base, val, otp);
#endif
}

static int stm32mp_bsec_write_otp(struct udevice *dev, u32 val, u32 otp)
{
#ifdef CONFIG_STM32MP1_TRUSTED
	return stm32_smc_exec(STM32_SMC_BSEC,
			      STM32_SMC_PROG_OTP,
			      otp, val);
#else
	struct stm32mp_bsec_platdata *plat = dev_get_platdata(dev);

	return bsec_program_otp(plat->base, val, otp);
#endif
}

static int stm32mp_bsec_write_shadow(struct udevice *dev, u32 val, u32 otp)
{
#ifdef CONFIG_STM32MP1_TRUSTED
	return stm32_smc_exec(STM32_SMC_BSEC,
			      STM32_SMC_WRITE_SHADOW,
			      otp, val);
#else
	struct stm32mp_bsec_platdata *plat = dev_get_platdata(dev);

	return bsec_write_shadow(plat->base, val, otp);
#endif
}

static int stm32mp_bsec_read(struct udevice *dev, int offset,
			     void *buf, int size)
{
	int ret;
	int i;
	bool shadow = true;
	int nb_otp = size / sizeof(u32);
	int otp;

	if (offset >= STM32_BSEC_OTP_OFFSET) {
		offset -= STM32_BSEC_OTP_OFFSET;
		shadow = false;
	}
	otp = offset / sizeof(u32);

	if (otp < 0 || (otp + nb_otp - 1) > BSEC_OTP_MAX_VALUE) {
		dev_err(dev, "wrong value for otp, max value : %i\n",
			BSEC_OTP_MAX_VALUE);
		return -EINVAL;
	}

	for (i = otp; i < (otp + nb_otp); i++) {
		u32 *addr = &((u32 *)buf)[i - otp];

		if (shadow)
			ret = stm32mp_bsec_read_shadow(dev, addr, i);
		else
			ret = stm32mp_bsec_read_otp(dev, addr, i);

		if (ret)
			break;
	}
	return ret;
}

static int stm32mp_bsec_write(struct udevice *dev, int offset,
			      const void *buf, int size)
{
	int ret = 0;
	int i;
	bool shadow = true;
	int nb_otp = size / sizeof(u32);
	int otp;

	if (offset >= STM32_BSEC_OTP_OFFSET) {
		offset -= STM32_BSEC_OTP_OFFSET;
		shadow = false;
	}
	otp = offset / sizeof(u32);

	if (otp < 0 || (otp + nb_otp - 1) > BSEC_OTP_MAX_VALUE) {
		dev_err(dev, "wrong value for otp, max value : %d\n",
			BSEC_OTP_MAX_VALUE);
		return -EINVAL;
	}

	for (i = otp; i < otp + nb_otp; i++) {
		u32 *val = &((u32 *)buf)[i - otp];

		if (shadow)
			ret = stm32mp_bsec_write_shadow(dev, *val, i);
		else
			ret = stm32mp_bsec_write_otp(dev, *val, i);
		if (ret)
			break;
	}
	return ret;
}

static const struct misc_ops stm32mp_bsec_ops = {
	.read = stm32mp_bsec_read,
	.write = stm32mp_bsec_write,
};

static int stm32mp_bsec_ofdata_to_platdata(struct udevice *dev)
{
	struct stm32mp_bsec_platdata *plat = dev_get_platdata(dev);

	plat->base = (u32)dev_read_addr_ptr(dev);

	return 0;
}

#ifndef CONFIG_STM32MP1_TRUSTED
static int stm32mp_bsec_probe(struct udevice *dev)
{
	int otp;
	struct stm32mp_bsec_platdata *plat = dev_get_platdata(dev);

	/* update unlocked shadow for OTP cleared by the rom code */
	for (otp = 57; otp <= BSEC_OTP_MAX_VALUE; otp++)
		if (!bsec_read_SR_lock(plat->base, otp))
			bsec_shadow_register(plat->base, otp);

	return 0;
}
#endif

static const struct udevice_id stm32mp_bsec_ids[] = {
	{ .compatible = "st,stm32mp15-bsec" },
	{}
};

U_BOOT_DRIVER(stm32mp_bsec) = {
	.name = "stm32mp_bsec",
	.id = UCLASS_MISC,
	.of_match = stm32mp_bsec_ids,
	.ofdata_to_platdata = stm32mp_bsec_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct stm32mp_bsec_platdata),
	.ops = &stm32mp_bsec_ops,
#ifndef CONFIG_STM32MP1_TRUSTED
	.probe = stm32mp_bsec_probe,
#endif
};
