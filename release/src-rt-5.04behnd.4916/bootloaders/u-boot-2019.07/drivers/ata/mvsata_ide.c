// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2010 Albert ARIBAUD <albert.u.boot@aribaud.net>
 *
 * Written-by: Albert ARIBAUD <albert.u.boot@aribaud.net>
 */

#include <common.h>
#include <asm/io.h>

#if defined(CONFIG_ORION5X)
#include <asm/arch/orion5x.h>
#elif defined(CONFIG_KIRKWOOD)
#include <asm/arch/soc.h>
#elif defined(CONFIG_ARCH_MVEBU)
#include <linux/mbus.h>
#endif

/* SATA port registers */
struct mvsata_port_registers {
	u32 reserved0[10];
	u32 edma_cmd;
	u32 reserved1[181];
	/* offset 0x300 : ATA Interface registers */
	u32 sstatus;
	u32 serror;
	u32 scontrol;
	u32 ltmode;
	u32 phymode3;
	u32 phymode4;
	u32 reserved2[5];
	u32 phymode1;
	u32 phymode2;
	u32 bist_cr;
	u32 bist_dw1;
	u32 bist_dw2;
	u32 serrorintrmask;
};

/*
 * Sanity checks:
 * - to compile at all, we need CONFIG_SYS_ATA_BASE_ADDR.
 * - for ide_preinit to make sense, we need at least one of
 *   CONFIG_SYS_ATA_IDE0_OFFSET or CONFIG_SYS_ATA_IDE1_OFFSET;
 * - for ide_preinit to be called, we need CONFIG_IDE_PREINIT.
 * Fail with an explanation message if these conditions are not met.
 * This is particularly important for CONFIG_IDE_PREINIT, because
 * its lack would not cause a build error.
 */

#if !defined(CONFIG_SYS_ATA_BASE_ADDR)
#error CONFIG_SYS_ATA_BASE_ADDR must be defined
#elif !defined(CONFIG_SYS_ATA_IDE0_OFFSET) \
   && !defined(CONFIG_SYS_ATA_IDE1_OFFSET)
#error CONFIG_SYS_ATA_IDE0_OFFSET or CONFIG_SYS_ATA_IDE1_OFFSET \
   must be defined
#elif !defined(CONFIG_IDE_PREINIT)
#error CONFIG_IDE_PREINIT must be defined
#endif

/*
 * Masks and values for SControl DETection and Interface Power Management,
 * and for SStatus DETection.
 */

#define MVSATA_EDMA_CMD_ATA_RST		0x00000004
#define MVSATA_SCONTROL_DET_MASK		0x0000000F
#define MVSATA_SCONTROL_DET_NONE		0x00000000
#define MVSATA_SCONTROL_DET_INIT		0x00000001
#define MVSATA_SCONTROL_IPM_MASK		0x00000F00
#define MVSATA_SCONTROL_IPM_NO_LP_ALLOWED	0x00000300
#define MVSATA_SCONTROL_MASK \
	(MVSATA_SCONTROL_DET_MASK|MVSATA_SCONTROL_IPM_MASK)
#define MVSATA_PORT_INIT \
	(MVSATA_SCONTROL_DET_INIT|MVSATA_SCONTROL_IPM_NO_LP_ALLOWED)
#define MVSATA_PORT_USE \
	(MVSATA_SCONTROL_DET_NONE|MVSATA_SCONTROL_IPM_NO_LP_ALLOWED)
#define MVSATA_SSTATUS_DET_MASK			0x0000000F
#define MVSATA_SSTATUS_DET_DEVCOMM		0x00000003

/*
 * Status codes to return to client callers. Currently, callers ignore
 * exact value and only care for zero or nonzero, so no need to make this
 * public, it is only #define'd for clarity.
 * If/when standard negative codes are implemented in U-Boot, then these
 * #defines should be moved to, or replaced by ones from, the common list
 * of status codes.
 */

#define MVSATA_STATUS_OK	0
#define MVSATA_STATUS_TIMEOUT	-1

/*
 * Registers for SATA MBUS memory windows
 */

#define MVSATA_WIN_CONTROL(w)	(MVEBU_AXP_SATA_BASE + 0x30 + ((w) << 4))
#define MVSATA_WIN_BASE(w)	(MVEBU_AXP_SATA_BASE + 0x34 + ((w) << 4))

/*
 * Initialize SATA memory windows for Armada XP
 */

#ifdef CONFIG_ARCH_MVEBU
static void mvsata_ide_conf_mbus_windows(void)
{
	const struct mbus_dram_target_info *dram;
	int i;

	dram = mvebu_mbus_dram_info();

	/* Disable windows, Set Size/Base to 0  */
	for (i = 0; i < 4; i++) {
		writel(0, MVSATA_WIN_CONTROL(i));
		writel(0, MVSATA_WIN_BASE(i));
	}

	for (i = 0; i < dram->num_cs; i++) {
		const struct mbus_dram_window *cs = dram->cs + i;
		writel(((cs->size - 1) & 0xffff0000) | (cs->mbus_attr << 8) |
				(dram->mbus_dram_target_id << 4) | 1,
				MVSATA_WIN_CONTROL(i));
		writel(cs->base & 0xffff0000, MVSATA_WIN_BASE(i));
	}
}
#endif

/*
 * Initialize one MVSATAHC port: set SControl's IPM to "always active"
 * and DET to "reset", then wait for SStatus's DET to become "device and
 * comm ok" (or time out after 50 us if no device), then set SControl's
 * DET back to "no action".
 */

static int mvsata_ide_initialize_port(struct mvsata_port_registers *port)
{
	u32 control;
	u32 status;
	u32 timeleft = 10000; /* wait at most 10 ms for SATA reset to complete */

	/* Hard reset */
	writel(MVSATA_EDMA_CMD_ATA_RST, &port->edma_cmd);
	udelay(25); /* taken from original marvell port */
	writel(0, &port->edma_cmd);

	/* Set control IPM to 3 (no low power) and DET to 1 (initialize) */
	control = readl(&port->scontrol);
	control = (control & ~MVSATA_SCONTROL_MASK) | MVSATA_PORT_INIT;
	writel(control, &port->scontrol);
	/* Toggle control DET back to 0 (normal operation) */
	control = (control & ~MVSATA_SCONTROL_MASK) | MVSATA_PORT_USE;
	writel(control, &port->scontrol);
	/* wait for status DET to become 3 (device and communication OK) */
	while (--timeleft) {
		status = readl(&port->sstatus) & MVSATA_SSTATUS_DET_MASK;
		if (status == MVSATA_SSTATUS_DET_DEVCOMM)
			break;
		udelay(1);
	}
	/* return success or time-out error depending on time left */
	if (!timeleft)
		return MVSATA_STATUS_TIMEOUT;
	return MVSATA_STATUS_OK;
}

/*
 * ide_preinit() will be called by ide_init in cmd_ide.c and will
 * reset the MVSTATHC ports needed by the board.
 */

int ide_preinit(void)
{
	int ret = MVSATA_STATUS_TIMEOUT;
	int status;

#ifdef CONFIG_ARCH_MVEBU
	mvsata_ide_conf_mbus_windows();
#endif

	/* Enable ATA port 0 (could be SATA port 0 or 1) if declared */
#if defined(CONFIG_SYS_ATA_IDE0_OFFSET)
	status = mvsata_ide_initialize_port(
		(struct mvsata_port_registers *)
		(CONFIG_SYS_ATA_BASE_ADDR + CONFIG_SYS_ATA_IDE0_OFFSET));
	if (status == MVSATA_STATUS_OK)
		ret = MVSATA_STATUS_OK;
#endif
	/* Enable ATA port 1 (could be SATA port 0 or 1) if declared */
#if defined(CONFIG_SYS_ATA_IDE1_OFFSET)
	status = mvsata_ide_initialize_port(
		(struct mvsata_port_registers *)
		(CONFIG_SYS_ATA_BASE_ADDR + CONFIG_SYS_ATA_IDE1_OFFSET));
	if (status == MVSATA_STATUS_OK)
		ret = MVSATA_STATUS_OK;
#endif
	/* Return success if at least one port initialization succeeded */
	return ret;
}
