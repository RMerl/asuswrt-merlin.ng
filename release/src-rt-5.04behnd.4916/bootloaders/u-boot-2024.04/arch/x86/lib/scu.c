// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2017 Intel Corporation
 *
 * Intel Mobile Internet Devices (MID) based on Intel Atom SoCs have few
 * microcontrollers inside to do some auxiliary tasks. One of such
 * microcontroller is System Controller Unit (SCU) which, in particular,
 * is servicing watchdog and controlling system reset function.
 *
 * This driver enables IPC channel to SCU.
 */
#include <common.h>
#include <dm.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/cpu.h>
#include <asm/scu.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/kernel.h>

/* SCU register map */
struct ipc_regs {
	u32 cmd;
	u32 status;
	u32 sptr;
	u32 dptr;
	u32 reserved[28];
	u32 wbuf[4];
	u32 rbuf[4];
};

struct scu {
	struct ipc_regs *regs;
};

/**
 * scu_ipc_send_command() - send command to SCU
 * @regs: register map of SCU
 * @cmd: command
 *
 * Command Register (Write Only):
 * A write to this register results in an interrupt to the SCU core processor
 * Format:
 * |rfu2(8) | size(8) | command id(4) | rfu1(3) | ioc(1) | command(8)|
 */
static void scu_ipc_send_command(struct ipc_regs *regs, u32 cmd)
{
	writel(cmd, &regs->cmd);
}

/**
 * scu_ipc_check_status() - check status of last command
 * @regs: register map of SCU
 *
 * Status Register (Read Only):
 * Driver will read this register to get the ready/busy status of the IPC
 * block and error status of the IPC command that was just processed by SCU
 * Format:
 * |rfu3(8)|error code(8)|initiator id(8)|cmd id(4)|rfu1(2)|error(1)|busy(1)|
 */
static int scu_ipc_check_status(struct ipc_regs *regs)
{
	int loop_count = 100000;
	int status;

	do {
		status = readl(&regs->status);
		if (!(status & BIT(0)))
			break;

		udelay(1);
	} while (--loop_count);
	if (!loop_count)
		return -ETIMEDOUT;

	if (status & BIT(1)) {
		printf("%s() status=0x%08x\n", __func__, status);
		return -EIO;
	}

	return 0;
}

static int scu_ipc_cmd(struct ipc_regs *regs, u32 cmd, u32 sub,
		       u32 *in, int inlen, u32 *out, int outlen)
{
	int i, err;

	for (i = 0; i < inlen; i++)
		writel(*in++, &regs->wbuf[i]);

	scu_ipc_send_command(regs, (inlen << 16) | (sub << 12) | cmd);
	err = scu_ipc_check_status(regs);

	if (!err) {
		for (i = 0; i < outlen; i++)
			*out++ = readl(&regs->rbuf[i]);
	}

	return err;
}

/**
 * scu_ipc_raw_command() - IPC command with data and pointers
 * @cmd:    IPC command code
 * @sub:    IPC command sub type
 * @in:     input data of this IPC command
 * @inlen:  input data length in dwords
 * @out:    output data of this IPC command
 * @outlen: output data length in dwords
 * @dptr:   data writing to SPTR register
 * @sptr:   data writing to DPTR register
 *
 * Send an IPC command to SCU with input/output data and source/dest pointers.
 *
 * Return:  an IPC error code or 0 on success.
 */
int scu_ipc_raw_command(u32 cmd, u32 sub, u32 *in, int inlen, u32 *out,
			int outlen, u32 dptr, u32 sptr)
{
	int inbuflen = DIV_ROUND_UP(inlen, 4);
	struct udevice *dev;
	struct scu *scu;
	int ret;

	ret = syscon_get_by_driver_data(X86_SYSCON_SCU, &dev);
	if (ret)
		return ret;

	scu = dev_get_priv(dev);

	/* Up to 16 bytes */
	if (inbuflen > 4)
		return -EINVAL;

	writel(dptr, &scu->regs->dptr);
	writel(sptr, &scu->regs->sptr);

	/*
	 * SRAM controller doesn't support 8-bit writes, it only
	 * supports 32-bit writes, so we have to copy input data into
	 * the temporary buffer, and SCU FW will use the inlen to
	 * determine the actual input data length in the temporary
	 * buffer.
	 */

	u32 inbuf[4] = {0};

	memcpy(inbuf, in, inlen);

	return scu_ipc_cmd(scu->regs, cmd, sub, inbuf, inlen, out, outlen);
}

/**
 * scu_ipc_simple_command() - send a simple command
 * @cmd: command
 * @sub: sub type
 *
 * Issue a simple command to the SCU. Do not use this interface if
 * you must then access data as any data values may be overwritten
 * by another SCU access by the time this function returns.
 *
 * This function may sleep. Locking for SCU accesses is handled for
 * the caller.
 */
int scu_ipc_simple_command(u32 cmd, u32 sub)
{
	struct scu *scu;
	struct udevice *dev;
	int ret;

	ret = syscon_get_by_driver_data(X86_SYSCON_SCU, &dev);
	if (ret)
		return ret;

	scu = dev_get_priv(dev);

	scu_ipc_send_command(scu->regs, sub << 12 | cmd);
	return scu_ipc_check_status(scu->regs);
}

/**
 *  scu_ipc_command - command with data
 *  @cmd: command
 *  @sub: sub type
 *  @in: input data
 *  @inlen: input length in dwords
 *  @out: output data
 *  @outlen: output length in dwords
 *
 *  Issue a command to the SCU which involves data transfers.
 */
int scu_ipc_command(u32 cmd, u32 sub, u32 *in, int inlen, u32 *out, int outlen)
{
	struct scu *scu;
	struct udevice *dev;
	int ret;

	ret = syscon_get_by_driver_data(X86_SYSCON_SCU, &dev);
	if (ret)
		return ret;

	scu = dev_get_priv(dev);

	return scu_ipc_cmd(scu->regs, cmd, sub, in, inlen, out, outlen);
}

static int scu_ipc_probe(struct udevice *dev)
{
	struct scu *scu = dev_get_priv(dev);

	scu->regs = syscon_get_first_range(X86_SYSCON_SCU);

	return 0;
}

static const struct udevice_id scu_ipc_match[] = {
	{ .compatible = "intel,scu-ipc", .data = X86_SYSCON_SCU },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(scu_ipc) = {
	.name		= "scu_ipc",
	.id		= UCLASS_SYSCON,
	.of_match	= scu_ipc_match,
	.probe		= scu_ipc_probe,
	.priv_auto	= sizeof(struct scu),
};
