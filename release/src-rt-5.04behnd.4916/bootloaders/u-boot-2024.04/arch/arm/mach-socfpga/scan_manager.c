// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2013 Altera Corporation <www.altera.com>
 */

#include <common.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/arch/freeze_controller.h>
#include <asm/arch/scan_manager.h>
#include <asm/arch/system_manager.h>
#include <linux/delay.h>

/*
 * Maximum polling loop to wait for IO scan chain engine becomes idle
 * to prevent infinite loop. It is important that this is NOT changed
 * to delay using timer functions, since at the time this function is
 * called, timer might not yet be inited.
 */
#define SCANMGR_MAX_DELAY		100

/*
 * Maximum length of TDI_TDO packet payload is 128 bits,
 * represented by (length - 1) in TDI_TDO header.
 */
#define TDI_TDO_MAX_PAYLOAD		127

#define SCANMGR_STAT_ACTIVE		(1 << 31)
#define SCANMGR_STAT_WFIFOCNT_MASK	0x70000000

static const struct socfpga_scan_manager *scan_manager_base =
		(void *)(SOCFPGA_SCANMGR_ADDRESS);
static const struct socfpga_freeze_controller *freeze_controller_base =
		(void *)(SOCFPGA_SYSMGR_ADDRESS + SYSMGR_FRZCTRL_ADDRESS);

/**
 * scan_chain_engine_is_idle() - Check if the JTAG scan chain is idle
 * @max_iter:	Maximum number of iterations to wait for idle
 *
 * Function to check IO scan chain engine status and wait if the engine is
 * is active. Poll the IO scan chain engine till maximum iteration reached.
 */
static u32 scan_chain_engine_is_idle(u32 max_iter)
{
	const u32 mask = SCANMGR_STAT_ACTIVE | SCANMGR_STAT_WFIFOCNT_MASK;
	u32 status;

	/* Poll the engine until the scan engine is inactive. */
	do {
		status = readl(&scan_manager_base->stat);
		if (!(status & mask))
			return 0;
	} while (max_iter--);

	return -ETIMEDOUT;
}

#define JTAG_BP_INSN		(1 << 0)
#define JTAG_BP_TMS		(1 << 1)
#define JTAG_BP_PAYLOAD		(1 << 2)
#define JTAG_BP_2BYTE		(1 << 3)
#define JTAG_BP_4BYTE		(1 << 4)

/**
 * scan_mgr_jtag_io() - Access the JTAG chain
 * @flags:	Control flags, used to configure the action on the JTAG
 * @iarg:	Instruction argument
 * @parg:	Payload argument or data
 *
 * Perform I/O on the JTAG chain
 */
static void scan_mgr_jtag_io(const u32 flags, const u8 iarg, const u32 parg)
{
	u32 data = parg;

	if (flags & JTAG_BP_INSN) {	/* JTAG instruction */
		/*
		 * The SCC JTAG register is LSB first, so make
		 * space for the instruction at the LSB.
		 */
		data <<= 8;
		if (flags & JTAG_BP_TMS) {
			data |= (0 << 7);	/* TMS instruction. */
			data |= iarg & 0x3f;	/* TMS arg is 6 bits. */
			if (flags & JTAG_BP_PAYLOAD)
				data |= (1 << 6);
		} else {
			data |= (1 << 7);	/* TDI/TDO instruction. */
			data |= iarg & 0xf;	/* TDI/TDO arg is 4 bits. */
			if (flags & JTAG_BP_PAYLOAD)
				data |= (1 << 4);
		}
	}

	if (flags & JTAG_BP_4BYTE)
		writel(data, &scan_manager_base->fifo_quad_byte);
	else if (flags & JTAG_BP_2BYTE)
		writel(data & 0xffff, &scan_manager_base->fifo_double_byte);
	else
		writel(data & 0xff, &scan_manager_base->fifo_single_byte);
}

/**
 * scan_mgr_jtag_insn_data() - Send JTAG instruction and data
 * @iarg:	Instruction argument
 * @data:	Associated data
 * @dlen:	Length of data in bits
 *
 * This function is used when programming the IO chains to submit the
 * instruction followed by variable length payload.
 */
static int
scan_mgr_jtag_insn_data(const u8 iarg, const unsigned long *data,
			const unsigned int dlen)
{
	int i, j;

	scan_mgr_jtag_io(JTAG_BP_INSN | JTAG_BP_2BYTE, iarg, dlen - 1);

	/* 32 bits or more remain */
	for (i = 0; i < dlen / 32; i++)
		scan_mgr_jtag_io(JTAG_BP_4BYTE, 0x0, data[i]);

	if ((dlen % 32) > 24) {	/* 31...24 bits remain */
		scan_mgr_jtag_io(JTAG_BP_4BYTE, 0x0, data[i]);
	} else if (dlen % 32) {	/* 24...1 bit remain */
		for (j = 0; j < dlen % 32; j += 8)
			scan_mgr_jtag_io(0, 0x0, data[i] >> j);
	}

	return scan_chain_engine_is_idle(SCANMGR_MAX_DELAY);
}

/**
 * scan_mgr_io_scan_chain_prg() - Program HPS IO Scan Chain
 * @io_scan_chain_id:		IO scan chain ID
 */
static int scan_mgr_io_scan_chain_prg(const unsigned int io_scan_chain_id)
{
	u32 io_scan_chain_len_in_bits;
	const unsigned long *iocsr_scan_chain;
	unsigned int rem, idx = 0;
	int ret;

	ret = iocsr_get_config_table(io_scan_chain_id, &iocsr_scan_chain,
				     &io_scan_chain_len_in_bits);
	if (ret)
		return 1;

	/*
	 * De-assert reinit if the IO scan chain is intended for HIO. In
	 * this, its the chain 3.
	 */
	if (io_scan_chain_id == 3)
		clrbits_le32(&freeze_controller_base->hioctrl,
			     SYSMGR_FRZCTRL_HIOCTRL_DLLRST_MASK);

	/*
	 * Check if the scan chain engine is inactive and the
	 * WFIFO is empty before enabling the IO scan chain
	 */
	ret = scan_chain_engine_is_idle(SCANMGR_MAX_DELAY);
	if (ret)
		return ret;

	/*
	 * Enable IO Scan chain based on scan chain id
	 * Note: only one chain can be enabled at a time
	 */
	setbits_le32(&scan_manager_base->en, 1 << io_scan_chain_id);

	/* Program IO scan chain. */
	while (io_scan_chain_len_in_bits) {
		if (io_scan_chain_len_in_bits > 128)
			rem = 128;
		else
			rem = io_scan_chain_len_in_bits;

		ret = scan_mgr_jtag_insn_data(0x0, &iocsr_scan_chain[idx], rem);
		if (ret)
			goto error;
		io_scan_chain_len_in_bits -= rem;
		idx += 4;
	}

	/* Disable IO Scan chain when configuration done*/
	clrbits_le32(&scan_manager_base->en, 1 << io_scan_chain_id);
	return 0;

error:
	/* Disable IO Scan chain when error detected */
	clrbits_le32(&scan_manager_base->en, 1 << io_scan_chain_id);
	return ret;
}

int scan_mgr_configure_iocsr(void)
{
	int status = 0;

	/* configure the IOCSR through scan chain */
	status |= scan_mgr_io_scan_chain_prg(0);
	status |= scan_mgr_io_scan_chain_prg(1);
	status |= scan_mgr_io_scan_chain_prg(2);
	status |= scan_mgr_io_scan_chain_prg(3);
	return status;
}

/**
 * scan_mgr_get_fpga_id() - Obtain FPGA JTAG ID
 *
 * This function obtains JTAG ID from the FPGA TAP controller.
 */
u32 scan_mgr_get_fpga_id(void)
{
	const unsigned long data = 0;
	u32 id = 0xffffffff;
	int ret;

	/* Enable HPS to talk to JTAG in the FPGA through the System Manager */
	writel(0x1, socfpga_get_sysmgr_addr() + SYSMGR_GEN5_SCANMGRGRP_CTRL);

	/* Enable port 7 */
	writel(0x80, &scan_manager_base->en);
	/* write to CSW to make s2f_ntrst reset */
	writel(0x02, &scan_manager_base->stat);

	/* Add a pause */
	mdelay(1);

	/* write 0x00 to CSW to clear the s2f_ntrst */
	writel(0, &scan_manager_base->stat);

	/*
	 * Go to Test-Logic-Reset state.
	 * This sets TAP controller into IDCODE mode.
	 */
	scan_mgr_jtag_io(JTAG_BP_INSN | JTAG_BP_TMS, 0x1f | (1 << 5), 0x0);

	/* Go to Run-Test/Idle -> DR-Scan -> Capture-DR -> Shift-DR state. */
	scan_mgr_jtag_io(JTAG_BP_INSN | JTAG_BP_TMS, 0x02 | (1 << 4), 0x0);

	/*
	 * Push 4 bytes of data through TDI->DR->TDO.
	 *
	 * Length of TDI data is 32bits (length - 1) and they are only
	 * zeroes as we care only for TDO data.
	 */
	ret = scan_mgr_jtag_insn_data(0x4, &data, 32);
	/* Read 32 bit from captured JTAG data. */
	if (!ret)
		id = readl(&scan_manager_base->fifo_quad_byte);

	/* Disable all port */
	writel(0, &scan_manager_base->en);
	writel(0, socfpga_get_sysmgr_addr() + SYSMGR_GEN5_SCANMGRGRP_CTRL);

	return id;
}
