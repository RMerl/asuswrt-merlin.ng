// SPDX-License-Identifier:    GPL-2.0
/*
 * Copyright (C) 2019 Marvell International Ltd.
 */

#include <clk.h>
#include <cpu_func.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <env.h>
#include <errno.h>
#include <fdtdec.h>
#include <log.h>
#include <malloc.h>
#include <memalign.h>
#include <mmc.h>
#include <part.h>
#include <pci.h>
#include <pci_ids.h>
#include <power/regulator.h>
#include <time.h>
#include <watchdog.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/libfdt.h>

#if defined(CONFIG_ARCH_OCTEON)
#include <mach/octeon-model.h>
#include <mach/cvmx-regs.h>
#include <mach/cvmx-mio-emm-defs.h>
#else
#include <asm/arch/board.h>
#include <asm/arch/clock.h>
#include <asm/arch/csrs/csrs-mio_emm.h>
#endif

#include "octeontx_hsmmc.h"

/* Use dummy implementation for MIPS Octeon to always return false */
#if defined(CONFIG_ARCH_OCTEON)
#define otx_is_soc(ver)		0
#endif

#define MMC_TIMEOUT_SHORT	20	/* in ms */
#define MMC_TIMEOUT_LONG	1000
#define MMC_TIMEOUT_ERASE	10000

#define MMC_DEFAULT_DATA_IN_TAP			10
#define MMC_DEFAULT_CMD_IN_TAP			10
#define MMC_DEFAULT_CMD_OUT_TAP			39
#define MMC_DEFAULT_DATA_OUT_TAP		39
#define MMC_DEFAULT_HS200_CMD_IN_TAP		24
#define MMC_DEFAULT_HS200_DATA_IN_TAP		24
#define MMC_DEFAULT_HS200_CMD_OUT_TAP	(otx_is_soc(CN95XX) ? 10 : 5)
#define MMC_DEFAULT_HS200_DATA_OUT_TAP	(otx_is_soc(CN95XX) ? 10 : 5)
#define MMC_DEFAULT_HS400_CMD_OUT_TAP	(otx_is_soc(CN95XX) ? 10 : 5)
#define MMC_DEFAULT_HS400_DATA_OUT_TAP	(otx_is_soc(CN95XX) ? 5 : 3)
#define MMC_DEFAULT_HS200_CMD_OUT_DLY		800	/* Delay in ps */
#define MMC_DEFAULT_HS200_DATA_OUT_DLY		800	/* Delay in ps */
#define MMC_DEFAULT_HS400_CMD_OUT_DLY		800	/* Delay in ps */
#define MMC_DEFAULT_HS400_DATA_OUT_DLY		400	/* Delay in ps */
#define MMC_DEFAULT_SD_UHS_SDR104_CMD_OUT_TAP	MMC_DEFAULT_HS200_CMD_OUT_TAP
#define MMC_DEFAULT_SD_UHS_SDR104_DATA_OUT_TAP	MMC_DEFAULT_HS200_DATA_OUT_TAP
#define MMC_LEGACY_DEFAULT_CMD_OUT_TAP		39
#define MMC_LEGACY_DEFAULT_DATA_OUT_TAP		39
#define MMC_SD_LEGACY_DEFAULT_CMD_OUT_TAP	63
#define MMC_SD_LEGACY_DEFAULT_DATA_OUT_TAP	63
#define MMC_HS_CMD_OUT_TAP			32
#define MMC_HS_DATA_OUT_TAP			32
#define MMC_SD_HS_CMD_OUT_TAP			26
#define MMC_SD_HS_DATA_OUT_TAP			26
#define MMC_SD_UHS_SDR25_CMD_OUT_TAP		26
#define MMC_SD_UHS_SDR25_DATA_OUT_TAP		26
#define MMC_SD_UHS_SDR50_CMD_OUT_TAP		26
#define MMC_SD_UHS_SDR50_DATA_OUT_TAP		26
#define MMC_DEFAULT_TAP_DELAY			4
#define TOTAL_NO_OF_TAPS			512
static void octeontx_mmc_switch_to(struct mmc *mmc);
static void set_wdog(struct mmc *mmc, u64 us);
static void do_switch(struct mmc *mmc, union mio_emm_switch emm_switch);
static int octeontx_mmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd,
				 struct mmc_data *data);
static int octeontx_mmc_configure_delay(struct mmc *mmc);
static int octeontx_mmc_calibrate_delay(struct mmc *mmc);
#if !defined(CONFIG_ARCH_OCTEON)
static int octeontx2_mmc_calc_delay(struct mmc *mmc, int delay);
static void octeontx_mmc_set_timing(struct mmc *mmc);
static int octeontx_mmc_set_input_bus_timing(struct mmc *mmc);
static int octeontx_mmc_set_output_bus_timing(struct mmc *mmc);
#endif

static bool host_probed;

/**
 * Get the slot data structure from a MMC data structure
 */
static inline struct octeontx_mmc_slot *mmc_to_slot(struct mmc *mmc)
{
	return container_of(mmc, struct octeontx_mmc_slot, mmc);
}

static inline struct octeontx_mmc_host *mmc_to_host(struct mmc *mmc)
{
	return mmc_to_slot(mmc)->host;
}

static inline struct octeontx_mmc_slot *dev_to_mmc_slot(struct udevice *dev)
{
	return dev_get_priv(dev);
}

static inline struct mmc *dev_to_mmc(struct udevice *dev)
{
	return &((struct octeontx_mmc_slot *)dev_get_priv(dev))->mmc;
}

#ifdef DEBUG
const char *mmc_reg_str(u64 reg)
{
	if (reg == MIO_EMM_DMA_CFG())
		return "MIO_EMM_DMA_CFG";
	if (reg == MIO_EMM_DMA_ADR())
		return "MIO_EMM_DMA_ADR";
	if (reg == MIO_EMM_DMA_INT())
		return "MIO_EMM_DMA_INT";
	if (reg == MIO_EMM_CFG())
		return "MIO_EMM_CFG";
	if (reg == MIO_EMM_MODEX(0))
		return "MIO_EMM_MODE0";
	if (reg == MIO_EMM_MODEX(1))
		return "MIO_EMM_MODE1";
	if (reg == MIO_EMM_MODEX(2))
		return "MIO_EMM_MODE2";
	if (reg == MIO_EMM_MODEX(3))
		return "MIO_EMM_MODE3";
	if (reg == MIO_EMM_IO_CTL())
		return "MIO_EMM_IO_CTL";
	if (reg == MIO_EMM_SWITCH())
		return "MIO_EMM_SWITCH";
	if (reg == MIO_EMM_DMA())
		return "MIO_EMM_DMA";
	if (reg == MIO_EMM_CMD())
		return "MIO_EMM_CMD";
	if (reg == MIO_EMM_RSP_STS())
		return "MIO_EMM_RSP_STS";
	if (reg == MIO_EMM_RSP_LO())
		return "MIO_EMM_RSP_LO";
	if (reg == MIO_EMM_RSP_HI())
		return "MIO_EMM_RSP_HI";
	if (reg == MIO_EMM_INT())
		return "MIO_EMM_INT";
	if (reg == MIO_EMM_WDOG())
		return "MIO_EMM_WDOG";
	if (reg == MIO_EMM_DMA_ARG())
		return "MIO_EMM_DMA_ARG";
	if (IS_ENABLED(CONFIG_ARCH_OCTEONTX)) {
		if (reg == MIO_EMM_SAMPLE())
			return "MIO_EMM_SAMPLE";
	}
	if (reg == MIO_EMM_STS_MASK())
		return "MIO_EMM_STS_MASK";
	if (reg == MIO_EMM_RCA())
		return "MIO_EMM_RCA";
	if (reg == MIO_EMM_BUF_IDX())
		return "MIO_EMM_BUF_IDX";
	if (reg == MIO_EMM_BUF_DAT())
		return "MIO_EMM_BUF_DAT";
	if (!IS_ENABLED(CONFIG_ARCH_OCTEONTX)) {
		if (reg == MIO_EMM_CALB())
			return "MIO_EMM_CALB";
		if (reg == MIO_EMM_TAP())
			return "MIO_EMM_TAP";
		if (reg == MIO_EMM_TIMING())
			return "MIO_EMM_TIMING";
		if (reg == MIO_EMM_DEBUG())
			return "MIO_EMM_DEBUG";
	}

	return "UNKNOWN";
}
#endif

static void octeontx_print_rsp_sts(struct mmc *mmc)
{
#ifdef DEBUG
	union mio_emm_rsp_sts emm_rsp_sts;
	const struct octeontx_mmc_host *host = mmc_to_host(mmc);
	static const char * const ctype_xor_str[] = {
		"No data",
		"Read data into Dbuf",
		"Write data from Dbuf",
		"Reserved"
	};

	static const char * const rtype_xor_str[] = {
		"No response",
		"R1, 48 bits",
		"R2, 136 bits",
		"R3, 48 bits",
		"R4, 48 bits",
		"R5, 48 bits",
		"Reserved 6",
		"Reserved 7"
	};

	emm_rsp_sts.u = readq(host->base_addr + MIO_EMM_RSP_STS());
	printf("\nMIO_EMM_RSP_STS:              0x%016llx\n", emm_rsp_sts.u);
	printf("    60-61: bus_id:              %u\n", emm_rsp_sts.s.bus_id);
	printf("    59:    cmd_val:             %s\n",
	       emm_rsp_sts.s.cmd_val ? "yes" : "no");
	printf("    58:    switch_val:          %s\n",
	       emm_rsp_sts.s.switch_val ? "yes" : "no");
	printf("    57:    dma_val:             %s\n",
	       emm_rsp_sts.s.dma_val ? "yes" : "no");
	printf("    56:    dma_pend:            %s\n",
	       emm_rsp_sts.s.dma_pend ? "yes" : "no");
	printf("    28:    dbuf_err:            %s\n",
	       emm_rsp_sts.s.dbuf_err ? "yes" : "no");
	printf("    23:    dbuf:                %u\n", emm_rsp_sts.s.dbuf);
	printf("    22:    blk_timeout:         %s\n",
	       emm_rsp_sts.s.blk_timeout ? "yes" : "no");
	printf("    21:    blk_crc_err:         %s\n",
	       emm_rsp_sts.s.blk_crc_err ? "yes" : "no");
	printf("    20:    rsp_busybit:         %s\n",
	       emm_rsp_sts.s.rsp_busybit ? "yes" : "no");
	printf("    19:    stp_timeout:         %s\n",
	       emm_rsp_sts.s.stp_timeout ? "yes" : "no");
	printf("    18:    stp_crc_err:         %s\n",
	       emm_rsp_sts.s.stp_crc_err ? "yes" : "no");
	printf("    17:    stp_bad_sts:         %s\n",
	       emm_rsp_sts.s.stp_bad_sts ? "yes" : "no");
	printf("    16:    stp_val:             %s\n",
	       emm_rsp_sts.s.stp_val ? "yes" : "no");
	printf("    15:    rsp_timeout:         %s\n",
	       emm_rsp_sts.s.rsp_timeout ? "yes" : "no");
	printf("    14:    rsp_crc_err:         %s\n",
	       emm_rsp_sts.s.rsp_crc_err ? "yes" : "no");
	printf("    13:    rsp_bad_sts:         %s\n",
	       emm_rsp_sts.s.rsp_bad_sts ? "yes" : "no");
	printf("    12:    rsp_val:             %s\n",
	       emm_rsp_sts.s.rsp_val ? "yes" : "no");
	printf("    9-11:  rsp_type:            %s\n",
	       rtype_xor_str[emm_rsp_sts.s.rsp_type]);
	printf("    7-8:   cmd_type:            %s\n",
	       ctype_xor_str[emm_rsp_sts.s.cmd_type]);
	printf("    1-6:   cmd_idx:             %u\n",
	       emm_rsp_sts.s.cmd_idx);
	printf("    0:     cmd_done:            %s\n",
	       emm_rsp_sts.s.cmd_done ? "yes" : "no");
#endif
}

static inline u64 read_csr(struct mmc *mmc, u64 reg)
{
	const struct octeontx_mmc_host *host = mmc_to_host(mmc);
	u64 value = readq(host->base_addr + reg);
#ifdef DEBUG_CSR
	printf("        %s: %s(0x%p) => 0x%llx\n", __func__,
	       mmc_reg_str(reg), host->base_addr + reg,
	       value);
#endif
	return value;
}

/**
 * Writes to a CSR register
 *
 * @param[in]	mmc	pointer to mmc data structure
 * @param	reg	register offset
 * @param	value	value to write to register
 */
static inline void write_csr(struct mmc *mmc, u64 reg, u64 value)
{
	const struct octeontx_mmc_host *host = mmc_to_host(mmc);
	void *addr = host->base_addr + reg;

#ifdef DEBUG_CSR
	printf("        %s: %s(0x%p) <= 0x%llx\n", __func__, mmc_reg_str(reg),
	       addr, value);
#endif
	writeq(value, addr);
}

#ifdef DEBUG
static void mmc_print_status(u32 status)
{
#ifdef DEBUG_STATUS
	static const char * const state[] = {
		"Idle",		/* 0 */
		"Ready",	/* 1 */
		"Ident",	/* 2 */
		"Standby",	/* 3 */
		"Tran",		/* 4 */
		"Data",		/* 5 */
		"Receive",	/* 6 */
		"Program",	/* 7 */
		"Dis",		/* 8 */
		"Btst",		/* 9 */
		"Sleep",	/* 10 */
		"reserved",	/* 11 */
		"reserved",	/* 12 */
		"reserved",	/* 13 */
		"reserved",	/* 14 */
		"reserved"	/* 15 */ };
	if (status & R1_APP_CMD)
		puts("MMC ACMD\n");
	if (status & R1_SWITCH_ERROR)
		puts("MMC switch error\n");
	if (status & R1_READY_FOR_DATA)
		puts("MMC ready for data\n");
	printf("MMC %s state\n", state[R1_CURRENT_STATE(status)]);
	if (status & R1_ERASE_RESET)
		puts("MMC erase reset\n");
	if (status & R1_WP_ERASE_SKIP)
		puts("MMC partial erase due to write protected blocks\n");
	if (status & R1_CID_CSD_OVERWRITE)
		puts("MMC CID/CSD overwrite error\n");
	if (status & R1_ERROR)
		puts("MMC undefined device error\n");
	if (status & R1_CC_ERROR)
		puts("MMC device error\n");
	if (status & R1_CARD_ECC_FAILED)
		puts("MMC internal ECC failed to correct data\n");
	if (status & R1_ILLEGAL_COMMAND)
		puts("MMC illegal command\n");
	if (status & R1_COM_CRC_ERROR)
		puts("MMC CRC of previous command failed\n");
	if (status & R1_LOCK_UNLOCK_FAILED)
		puts("MMC sequence or password error in lock/unlock device command\n");
	if (status & R1_CARD_IS_LOCKED)
		puts("MMC device locked by host\n");
	if (status & R1_WP_VIOLATION)
		puts("MMC attempt to program write protected block\n");
	if (status & R1_ERASE_PARAM)
		puts("MMC invalid selection of erase groups for erase\n");
	if (status & R1_ERASE_SEQ_ERROR)
		puts("MMC error in sequence of erase commands\n");
	if (status & R1_BLOCK_LEN_ERROR)
		puts("MMC block length error\n");
	if (status & R1_ADDRESS_ERROR)
		puts("MMC address misalign error\n");
	if (status & R1_OUT_OF_RANGE)
		puts("MMC address out of range\n");
#endif
}
#endif

#if !defined(CONFIG_ARCH_OCTEON)
/**
 * Print out all of the register values where mmc is optional
 *
 * @param mmc	MMC device (can be NULL)
 * @param host	Pointer to host data structure (can be NULL if mmc is !NULL)
 */
static void octeontx_mmc_print_registers2(struct mmc *mmc,
					  struct octeontx_mmc_host *host)
{
	struct octeontx_mmc_slot *slot = mmc ? mmc->priv : NULL;
	union mio_emm_dma_cfg emm_dma_cfg;
	union mio_emm_dma_adr emm_dma_adr;
	union mio_emm_dma_int emm_dma_int;
	union mio_emm_cfg emm_cfg;
	union mio_emm_modex emm_mode;
	union mio_emm_switch emm_switch;
	union mio_emm_dma emm_dma;
	union mio_emm_cmd emm_cmd;
	union mio_emm_rsp_sts emm_rsp_sts;
	union mio_emm_rsp_lo emm_rsp_lo;
	union mio_emm_rsp_hi emm_rsp_hi;
	union mio_emm_int emm_int;
	union mio_emm_wdog emm_wdog;
	union mio_emm_sample emm_sample;
	union mio_emm_calb emm_calb;
	union mio_emm_tap emm_tap;
	union mio_emm_timing emm_timing;
	union mio_emm_io_ctl io_ctl;
	union mio_emm_debug emm_debug;
	union mio_emm_sts_mask emm_sts_mask;
	union mio_emm_rca emm_rca;
	int bus;

	static const char * const bus_width_str[] = {
		"1-bit data bus (power on)",
		"4-bit data bus",
		"8-bit data bus",
		"reserved (3)",
		"reserved (4)",
		"4-bit data bus (dual data rate)",
		"8-bit data bus (dual data rate)",
		"reserved (7)",
		"reserved (8)",
		"invalid (9)",
		"invalid (10)",
		"invalid (11)",
		"invalid (12)",
		"invalid (13)",
		"invalid (14)",
		"invalid (15)",
	};
	static const char * const ctype_xor_str[] = {
		"No data",
		"Read data into Dbuf",
		"Write data from Dbuf",
		"Reserved"
	};

	static const char * const rtype_xor_str[] = {
		"No response",
		"R1, 48 bits",
		"R2, 136 bits",
		"R3, 48 bits",
		"R4, 48 bits",
		"R5, 48 bits",
		"Reserved 6",
		"Reserved 7"
	};

	if (!host && mmc)
		host = mmc_to_host(mmc);

	if (mmc)
		printf("%s: bus id: %u\n", __func__, slot->bus_id);
	emm_dma_cfg.u = readq(host->base_addr + MIO_EMM_DMA_CFG());
	printf("MIO_EMM_DMA_CFG:                0x%016llx\n",
	       emm_dma_cfg.u);
	printf("    63:    en:                  %s\n",
	       emm_dma_cfg.s.en ? "enabled" : "disabled");
	printf("    62:    rw:                  %s\n",
	       emm_dma_cfg.s.rw ? "write" : "read");
	printf("    61:    clr:                 %s\n",
	       emm_dma_cfg.s.clr ? "clear" : "not clear");
	printf("    59:    swap32:              %s\n",
	       emm_dma_cfg.s.swap32 ? "yes" : "no");
	printf("    58:    swap16:              %s\n",
	       emm_dma_cfg.s.swap16 ? "yes" : "no");
	printf("    57:    swap8:               %s\n",
	       emm_dma_cfg.s.swap8 ? "yes" : "no");
	printf("    56:    endian:              %s\n",
	       emm_dma_cfg.s.endian ? "little" : "big");
	printf("    36-55: size:                %u\n",
	       emm_dma_cfg.s.size);

	emm_dma_adr.u = readq(host->base_addr + MIO_EMM_DMA_ADR());
	printf("MIO_EMM_DMA_ADR:              0x%016llx\n", emm_dma_adr.u);
	printf("    0-49:  adr:                 0x%llx\n",
	       (u64)emm_dma_adr.s.adr);

	emm_dma_int.u = readq(host->base_addr + MIO_EMM_DMA_INT());
	printf("\nMIO_EMM_DMA_INT:              0x%016llx\n",
	       emm_dma_int.u);
	printf("    1:     FIFO:                %s\n",
	       emm_dma_int.s.fifo ? "yes" : "no");
	printf("    0:     Done:                %s\n",
	       emm_dma_int.s.done ? "yes" : "no");
		emm_cfg.u = readq(host->base_addr + MIO_EMM_CFG());

	printf("\nMIO_EMM_CFG:                  0x%016llx\n",
	       emm_cfg.u);
	printf("    3:     bus_ena3:            %s\n",
	       emm_cfg.s.bus_ena & 0x08 ? "yes" : "no");
	printf("    2:     bus_ena2:            %s\n",
	       emm_cfg.s.bus_ena & 0x04 ? "yes" : "no");
	printf("    1:     bus_ena1:            %s\n",
	       emm_cfg.s.bus_ena & 0x02 ? "yes" : "no");
	printf("    0:     bus_ena0:            %s\n",
	       emm_cfg.s.bus_ena & 0x01 ? "yes" : "no");
	for (bus = 0; bus < 4; bus++) {
		emm_mode.u = readq(host->base_addr + MIO_EMM_MODEX(bus));
		printf("\nMIO_EMM_MODE%u:               0x%016llx\n",
		       bus, emm_mode.u);
		if (!IS_ENABLED(CONFIG_ARCH_OCTEONTX)) {
			printf("    50:    hs400_timing:        %s\n",
			       emm_mode.s.hs400_timing ? "yes" : "no");
			printf("    49:    hs200_timing:        %s\n",
			       emm_mode.s.hs200_timing ? "yes" : "no");
		}
		printf("    48:    hs_timing:           %s\n",
		       emm_mode.s.hs_timing ? "yes" : "no");
		printf("    40-42: bus_width:           %s\n",
		       bus_width_str[emm_mode.s.bus_width]);
		printf("    32-35: power_class          %u\n",
		       emm_mode.s.power_class);
		printf("    16-31: clk_hi:              %u\n",
		       emm_mode.s.clk_hi);
		printf("    0-15:  clk_lo:              %u\n",
		       emm_mode.s.clk_lo);
	}

	emm_switch.u = readq(host->base_addr + MIO_EMM_SWITCH());
	printf("\nMIO_EMM_SWITCH:               0x%016llx\n", emm_switch.u);
	printf("    60-61: bus_id:              %u\n", emm_switch.s.bus_id);
	printf("    59:    switch_exe:          %s\n",
	       emm_switch.s.switch_exe ? "yes" : "no");
	printf("    58:    switch_err0:         %s\n",
	       emm_switch.s.switch_err0 ? "yes" : "no");
	printf("    57:    switch_err1:         %s\n",
	       emm_switch.s.switch_err1 ? "yes" : "no");
	printf("    56:    switch_err2:         %s\n",
	       emm_switch.s.switch_err2 ? "yes" : "no");
	printf("    48:    hs_timing:           %s\n",
	       emm_switch.s.hs_timing ? "yes" : "no");
	printf("    42-40: bus_width:           %s\n",
	       bus_width_str[emm_switch.s.bus_width]);
	printf("    32-35: power_class:         %u\n",
	       emm_switch.s.power_class);
	printf("    16-31: clk_hi:              %u\n",
	       emm_switch.s.clk_hi);
	printf("    0-15:  clk_lo:              %u\n", emm_switch.s.clk_lo);

	emm_dma.u = readq(host->base_addr + MIO_EMM_DMA());
	printf("\nMIO_EMM_DMA:                  0x%016llx\n", emm_dma.u);
	printf("    60-61: bus_id:              %u\n", emm_dma.s.bus_id);
	printf("    59:    dma_val:             %s\n",
	       emm_dma.s.dma_val ? "yes" : "no");
	printf("    58:    sector:              %s mode\n",
	       emm_dma.s.sector ? "sector" : "byte");
	printf("    57:    dat_null:            %s\n",
	       emm_dma.s.dat_null ? "yes" : "no");
	printf("    51-56: thres:               %u\n", emm_dma.s.thres);
	printf("    50:    rel_wr:              %s\n",
	       emm_dma.s.rel_wr ? "yes" : "no");
	printf("    49:    rw:                  %s\n",
	       emm_dma.s.rw ? "write" : "read");
	printf("    48:    multi:               %s\n",
	       emm_dma.s.multi ? "yes" : "no");
	printf("    32-47: block_cnt:           %u\n",
	       emm_dma.s.block_cnt);
	printf("    0-31:  card_addr:           0x%x\n",
	       emm_dma.s.card_addr);

	emm_cmd.u = readq(host->base_addr + MIO_EMM_CMD());
	printf("\nMIO_EMM_CMD:                  0x%016llx\n", emm_cmd.u);
	printf("\n  62:    skip_busy:           %s\n",
	       emm_cmd.s.skip_busy ? "yes" : "no");
	printf("    60-61: bus_id:              %u\n", emm_cmd.s.bus_id);
	printf("    59:    cmd_val:             %s\n",
	       emm_cmd.s.cmd_val ? "yes" : "no");
	printf("    55:    dbuf:                %u\n", emm_cmd.s.dbuf);
	printf("    49-54: offset:              %u\n", emm_cmd.s.offset);
	printf("    41-42: ctype_xor:           %s\n",
	       ctype_xor_str[emm_cmd.s.ctype_xor]);
	printf("    38-40: rtype_xor:           %s\n",
	       rtype_xor_str[emm_cmd.s.rtype_xor]);
	printf("    32-37: cmd_idx:             %u\n", emm_cmd.s.cmd_idx);
	printf("    0-31:  arg:                 0x%x\n", emm_cmd.s.arg);

	emm_rsp_sts.u = readq(host->base_addr + MIO_EMM_RSP_STS());
	printf("\nMIO_EMM_RSP_STS:              0x%016llx\n", emm_rsp_sts.u);
	printf("    60-61: bus_id:              %u\n", emm_rsp_sts.s.bus_id);
	printf("    59:    cmd_val:             %s\n",
	       emm_rsp_sts.s.cmd_val ? "yes" : "no");
	printf("    58:    switch_val:          %s\n",
	       emm_rsp_sts.s.switch_val ? "yes" : "no");
	printf("    57:    dma_val:             %s\n",
	       emm_rsp_sts.s.dma_val ? "yes" : "no");
	printf("    56:    dma_pend:            %s\n",
	       emm_rsp_sts.s.dma_pend ? "yes" : "no");
	printf("    28:    dbuf_err:            %s\n",
	       emm_rsp_sts.s.dbuf_err ? "yes" : "no");
	printf("    23:    dbuf:                %u\n", emm_rsp_sts.s.dbuf);
	printf("    22:    blk_timeout:         %s\n",
	       emm_rsp_sts.s.blk_timeout ? "yes" : "no");
	printf("    21:    blk_crc_err:         %s\n",
	       emm_rsp_sts.s.blk_crc_err ? "yes" : "no");
	printf("    20:    rsp_busybit:         %s\n",
	       emm_rsp_sts.s.rsp_busybit ? "yes" : "no");
	printf("    19:    stp_timeout:         %s\n",
	       emm_rsp_sts.s.stp_timeout ? "yes" : "no");
	printf("    18:    stp_crc_err:         %s\n",
	       emm_rsp_sts.s.stp_crc_err ? "yes" : "no");
	printf("    17:    stp_bad_sts:         %s\n",
	       emm_rsp_sts.s.stp_bad_sts ? "yes" : "no");
	printf("    16:    stp_val:             %s\n",
	       emm_rsp_sts.s.stp_val ? "yes" : "no");
	printf("    15:    rsp_timeout:         %s\n",
	       emm_rsp_sts.s.rsp_timeout ? "yes" : "no");
	printf("    14:    rsp_crc_err:         %s\n",
	       emm_rsp_sts.s.rsp_crc_err ? "yes" : "no");
	printf("    13:    rsp_bad_sts:         %s\n",
	       emm_rsp_sts.s.rsp_bad_sts ? "yes" : "no");
	printf("    12:    rsp_val:             %s\n",
	       emm_rsp_sts.s.rsp_val ? "yes" : "no");
	printf("    9-11:  rsp_type:            %s\n",
	       rtype_xor_str[emm_rsp_sts.s.rsp_type]);
	printf("    7-8:   cmd_type:            %s\n",
	       ctype_xor_str[emm_rsp_sts.s.cmd_type]);
	printf("    1-6:   cmd_idx:             %u\n",
	       emm_rsp_sts.s.cmd_idx);
	printf("    0:     cmd_done:            %s\n",
	       emm_rsp_sts.s.cmd_done ? "yes" : "no");

	emm_rsp_lo.u = readq(host->base_addr + MIO_EMM_RSP_LO());
	printf("\nMIO_EMM_RSP_STS_LO:           0x%016llx\n", emm_rsp_lo.u);

	emm_rsp_hi.u = readq(host->base_addr + MIO_EMM_RSP_HI());
	printf("\nMIO_EMM_RSP_STS_HI:           0x%016llx\n", emm_rsp_hi.u);

	emm_int.u = readq(host->base_addr + MIO_EMM_INT());
	printf("\nMIO_EMM_INT:                  0x%016llx\n", emm_int.u);
	printf("    6:    switch_err:           %s\n",
	       emm_int.s.switch_err ? "yes" : "no");
	printf("    5:    switch_done:          %s\n",
	       emm_int.s.switch_done ? "yes" : "no");
	printf("    4:    dma_err:              %s\n",
	       emm_int.s.dma_err ? "yes" : "no");
	printf("    3:    cmd_err:              %s\n",
	       emm_int.s.cmd_err ? "yes" : "no");
	printf("    2:    dma_done:             %s\n",
	       emm_int.s.dma_done ? "yes" : "no");
	printf("    1:    cmd_done:             %s\n",
	       emm_int.s.cmd_done ? "yes" : "no");
	printf("    0:    buf_done:             %s\n",
	       emm_int.s.buf_done ? "yes" : "no");

	emm_wdog.u = readq(host->base_addr + MIO_EMM_WDOG());
	printf("\nMIO_EMM_WDOG:                 0x%016llx (%u)\n",
	       emm_wdog.u, emm_wdog.s.clk_cnt);

	if (IS_ENABLED(CONFIG_ARCH_OCTEONTX)) {
		emm_sample.u = readq(host->base_addr + MIO_EMM_SAMPLE());
		printf("\nMIO_EMM_SAMPLE:               0x%016llx\n",
		       emm_sample.u);
		printf("    16-25: cmd_cnt:             %u\n",
		       emm_sample.s.cmd_cnt);
		printf("    0-9:   dat_cnt:             %u\n",
		       emm_sample.s.dat_cnt);
	}

	emm_sts_mask.u = readq(host->base_addr + MIO_EMM_STS_MASK());
	printf("\nMIO_EMM_STS_MASK:             0x%016llx\n", emm_sts_mask.u);

	emm_rca.u = readq(host->base_addr + MIO_EMM_RCA());
	printf("\nMIO_EMM_RCA:                  0x%016llx\n", emm_rca.u);
	printf("    0-15:  card_rca:            0x%04x\n",
	       emm_rca.s.card_rca);
	if (!IS_ENABLED(CONFIG_ARCH_OCTEONTX)) {
		emm_calb.u = readq(host->base_addr + MIO_EMM_CALB());
		printf("\nMIO_EMM_CALB:                 0x%016llx\n",
		       emm_calb.u);
		printf("       0:  start:               %u\n",
		       emm_calb.s.start);
		emm_tap.u = readq(host->base_addr + MIO_EMM_TAP());
		printf("\nMIO_EMM_TAP:                  0x%016llx\n",
		       emm_tap.u);
		printf("     7-0:  delay:               %u\n", emm_tap.s.delay);
		emm_timing.u = readq(host->base_addr + MIO_EMM_TIMING());
		printf("\nMIO_EMM_TIMING:               0x%016llx\n",
		       emm_timing.u);
		printf("   53-48:  cmd_in_tap:          %u\n",
		       emm_timing.s.cmd_in_tap);
		printf("   37-32:  cmd_out_tap:         %u\n",
		       emm_timing.s.cmd_out_tap);
		printf("   21-16:  data_in_tap:         %u\n",
		       emm_timing.s.data_in_tap);
		printf("     5-0:  data_out_tap:        %u\n",
		       emm_timing.s.data_out_tap);
		io_ctl.u = readq(host->base_addr + MIO_EMM_IO_CTL());
		printf("\nMIO_IO_CTL:                   0x%016llx\n", io_ctl.u);
		printf("     3-2:  drive:               %u (%u mA)\n",
		       io_ctl.s.drive, 2 << io_ctl.s.drive);
		printf("       0:  slew:                %u %s\n", io_ctl.s.slew,
		       io_ctl.s.slew ? "high" : "low");
		emm_debug.u = readq(host->base_addr + MIO_EMM_DEBUG());
		printf("\nMIO_EMM_DEBUG:                0x%016llx\n",
		       emm_debug.u);
		printf("      21: rdsync_rst            0x%x\n",
		       emm_debug.s.rdsync_rst);
		printf("      20: emmc_clk_disable      0x%x\n",
		       emm_debug.s.emmc_clk_disable);
		printf("   19-16: dma_sm:               0x%x\n",
		       emm_debug.s.dma_sm);
		printf("   15-12: data_sm:              0x%x\n",
		       emm_debug.s.data_sm);
		printf("    11-8: cmd_sm:               0x%x\n",
		       emm_debug.s.cmd_sm);
		printf("       0: clk_on:               0x%x\n",
		       emm_debug.s.clk_on);
	}

	puts("\n");
}

/**
 * Print out all of the register values
 *
 * @param mmc	MMC device
 */
static void octeontx_mmc_print_registers(struct mmc *mmc)
{
#ifdef DEBUG_REGISTERS
	const int print = 1;
#else
	const int print = 0;
#endif
	if (print)
		octeontx_mmc_print_registers2(mmc, mmc_to_host(mmc));
}
#else
static void octeontx_mmc_print_registers(struct mmc *mmc)
{
	return;
}
#endif

static const struct octeontx_sd_mods octeontx_cr_types[] = {
{ {0, 0}, {0, 0}, {0, 0} },	/* CMD0 */
{ {0, 3}, {0, 3}, {0, 0} },	/* CMD1 */
{ {0, 2}, {0, 2}, {0, 0} },	/* CMD2 */
{ {0, 1}, {0, 3}, {0, 0} },	/* CMD3 SD_CMD_SEND_RELATIVE_ADDR 0, 2 */
{ {0, 0}, {0, 0}, {0, 0} },	/* CMD4 */
{ {0, 1}, {0, 1}, {0, 0} },	/* CMD5 */
{ {0, 1}, {1, 1}, {0, 1} },	/*
				 * CMD6 SD_CMD_SWITCH_FUNC 1,0
				 * (ACMD) SD_APP_SET_BUS_WIDTH
				 */
{ {0, 1}, {0, 1}, {0, 0} },	/* CMD7 */
{ {1, 1}, {0, 3}, {0, 0} },	/* CMD8 SD_CMD_SEND_IF_COND 1,2 */
{ {0, 2}, {0, 2}, {0, 0} },	/* CMD9 */
{ {0, 2}, {0, 2}, {0, 0} },	/* CMD10 */
{ {1, 1}, {0, 1}, {1, 1} },	/* CMD11 SD_CMD_SWITCH_UHS18V 1,0 */
{ {0, 1}, {0, 1}, {0, 0} },	/* CMD12 */
{ {0, 1}, {0, 1}, {1, 3} },	/* CMD13 (ACMD)) SD_CMD_APP_SD_STATUS 1,2 */
{ {1, 1}, {1, 1}, {0, 0} },	/* CMD14 */
{ {0, 0}, {0, 0}, {0, 0} },	/* CMD15 */
{ {0, 1}, {0, 1}, {0, 0} },	/* CMD16 */
{ {1, 1}, {1, 1}, {0, 0} },	/* CMD17 */
{ {1, 1}, {1, 1}, {0, 0} },	/* CMD18 */
{ {3, 1}, {3, 1}, {0, 0} },	/* CMD19 */
{ {2, 1}, {0, 0}, {0, 0} },	/* CMD20 */	/* SD 2,0 */
{ {0, 0}, {0, 0}, {0, 0} },	/* CMD21 */
{ {0, 0}, {0, 0}, {1, 1} },	/* CMD22 (ACMD) SD_APP_SEND_NUM_WR_BLKS 1,0 */
{ {0, 1}, {0, 1}, {0, 1} },	/* CMD23 */	/* SD ACMD 1,0 */
{ {2, 1}, {2, 1}, {2, 1} },	/* CMD24 */
{ {2, 1}, {2, 1}, {2, 1} },	/* CMD25 */
{ {2, 1}, {2, 1}, {2, 1} },	/* CMD26 */
{ {2, 1}, {2, 1}, {2, 1} },	/* CMD27 */
{ {0, 1}, {0, 1}, {0, 1} },	/* CMD28 */
{ {0, 1}, {0, 1}, {0, 1} },	/* CMD29 */
{ {1, 1}, {1, 1}, {1, 1} },	/* CMD30 */
{ {1, 1}, {1, 1}, {1, 1} },	/* CMD31 */
{ {0, 0}, {0, 1}, {0, 0} },	/* CMD32 SD_CMD_ERASE_WR_BLK_START 0,1 */
{ {0, 0}, {0, 1}, {0, 0} },	/* CMD33 SD_CMD_ERASE_WR_BLK_END 0,1 */
{ {0, 0}, {0, 0}, {0, 0} },	/* CMD34 */
{ {0, 1}, {0, 1}, {0, 1} },	/* CMD35 */
{ {0, 1}, {0, 1}, {0, 1} },	/* CMD36 */
{ {0, 0}, {0, 0}, {0, 0} },	/* CMD37 */
{ {0, 1}, {0, 1}, {0, 1} },	/* CMD38 */
{ {0, 4}, {0, 4}, {0, 4} },	/* CMD39 */
{ {0, 5}, {0, 5}, {0, 5} },	/* CMD40 */
{ {0, 0}, {0, 0}, {0, 3} },	/* CMD41 (ACMD) SD_CMD_APP_SEND_OP_COND 0,3 */
{ {2, 1}, {2, 1}, {2, 1} },	/* CMD42 */
{ {0, 0}, {0, 0}, {0, 0} },	/* CMD43 */
{ {0, 0}, {0, 0}, {0, 0} },	/* CMD44 */
{ {0, 0}, {0, 0}, {0, 0} },	/* CMD45 */
{ {0, 0}, {0, 0}, {0, 0} },	/* CMD46 */
{ {0, 0}, {0, 0}, {0, 0} },	/* CMD47 */
{ {0, 0}, {1, 0}, {0, 0} },	/* CMD48 SD_CMD_READ_EXTR_SINGLE */
{ {0, 0}, {2, 0}, {0, 0} },	/* CMD49 SD_CMD_WRITE_EXTR_SINGLE */
{ {0, 0}, {0, 0}, {0, 0} },	/* CMD50 */
{ {0, 0}, {0, 0}, {1, 1} },	/* CMD51 (ACMD) SD_CMD_APP_SEND_SCR 1,1 */
{ {0, 0}, {0, 0}, {0, 0} },	/* CMD52 */
{ {0, 0}, {0, 0}, {0, 0} },	/* CMD53 */
{ {0, 0}, {0, 0}, {0, 0} },	/* CMD54 */
{ {0, 1}, {0, 1}, {0, 1} },	/* CMD55 */
{ {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff} },	/* CMD56 */
{ {0, 0}, {0, 0}, {0, 0} },	/* CMD57 */
{ {0, 0}, {0, 3}, {0, 3} },	/* CMD58 SD_CMD_SPI_READ_OCR 0,3 */
{ {0, 0}, {0, 1}, {0, 0} },	/* CMD59 SD_CMD_SPI_CRC_ON_OFF 0,1 */
{ {0, 0}, {0, 0}, {0, 0} },	/* CMD60 */
{ {0, 0}, {0, 0}, {0, 0} },	/* CMD61 */
{ {0, 0}, {0, 0}, {0, 0} },	/* CMD62 */
{ {0, 0}, {0, 0}, {0, 0} }	/* CMD63 */
};

/**
 * Returns XOR values needed for SD commands and other quirks
 *
 * @param	mmc	mmc device
 * @param	cmd	command information
 *
 * Return: octeontx_mmc_cr_mods data structure with various quirks and flags
 */
static struct octeontx_mmc_cr_mods
octeontx_mmc_get_cr_mods(struct mmc *mmc, const struct mmc_cmd *cmd,
			 const struct mmc_data *data)
{
	struct octeontx_mmc_slot *slot = mmc_to_slot(mmc);
	struct octeontx_mmc_cr_mods cr = {0, 0};
	const struct octeontx_sd_mods *sdm =
					&octeontx_cr_types[cmd->cmdidx & 0x3f];
	u8 c = sdm->mmc.c, r = sdm->mmc.r;
	u8 desired_ctype = 0;

	if (IS_MMC(mmc)) {
#ifdef MMC_SUPPORTS_TUNING
		if (cmd->cmdidx == MMC_CMD_SEND_TUNING_BLOCK_HS200) {
			if (cmd->resp_type == MMC_RSP_R1)
				cr.rtype_xor = 1;
			if (data && data->flags & MMC_DATA_READ)
				cr.ctype_xor = 1;
		}
#endif
		return cr;
	}

	if (cmd->cmdidx == 56)
		c = (cmd->cmdarg & 1) ? 1 : 2;

	if (data) {
		if (data->flags & MMC_DATA_READ)
			desired_ctype = 1;
		else if (data->flags & MMC_DATA_WRITE)
			desired_ctype = 2;
	}

	cr.ctype_xor = c ^ desired_ctype;
	if (slot->is_acmd)
		cr.rtype_xor = r ^ sdm->sdacmd.r;
	else
		cr.rtype_xor = r ^ sdm->sd.r;

	debug("%s(%s): mmc c: %d, mmc r: %d, desired c: %d, xor c: %d, xor r: %d\n",
	      __func__, mmc->dev->name, c, r, desired_ctype,
	      cr.ctype_xor, cr.rtype_xor);
	return cr;
}

/**
 * Keep track of switch commands internally
 */
static void octeontx_mmc_track_switch(struct mmc *mmc, u32 cmd_arg)
{
	struct octeontx_mmc_slot *slot = mmc_to_slot(mmc);
	u8 how = (cmd_arg >> 24) & 3;
	u8 where = (u8)(cmd_arg >> 16);
	u8 val = (u8)(cmd_arg >> 8);

	slot->want_switch = slot->cached_switch;

	if (slot->is_acmd)
		return;

	if (how != 3)
		return;

	switch (where) {
	case EXT_CSD_BUS_WIDTH:
		slot->want_switch.s.bus_width = val;
		break;
	case EXT_CSD_POWER_CLASS:
		slot->want_switch.s.power_class = val;
		break;
	case EXT_CSD_HS_TIMING:
		slot->want_switch.s.hs_timing = 0;
#if !defined(CONFIG_ARCH_OCTEON)
		slot->want_switch.s.hs200_timing = 0;
		slot->want_switch.s.hs400_timing = 0;
#endif
		switch (val & 0xf) {
		case 0:
			break;
		case 1:
			slot->want_switch.s.hs_timing = 1;
			break;
#if !defined(CONFIG_ARCH_OCTEON)
		case 2:
			if (!slot->is_asim && !slot->is_emul)
				slot->want_switch.s.hs200_timing = 1;
			break;
		case 3:
			if (!slot->is_asim && !slot->is_emul)
				slot->want_switch.s.hs400_timing = 1;
			break;
#endif
		default:
			pr_err("%s(%s): Unsupported timing mode 0x%x\n",
			       __func__, mmc->dev->name, val & 0xf);
			break;
		}
		break;
	default:
		break;
	}
}

static int octeontx_mmc_print_rsp_errors(struct mmc *mmc,
					 union mio_emm_rsp_sts rsp_sts)
{
	bool err = false;
	const char *name = mmc->dev->name;

	if (rsp_sts.s.acc_timeout) {
		pr_warn("%s(%s): acc_timeout\n", __func__, name);
		err = true;
	}
	if (rsp_sts.s.dbuf_err) {
		pr_warn("%s(%s): dbuf_err\n", __func__, name);
		err = true;
	}
	if (rsp_sts.s.blk_timeout) {
		pr_warn("%s(%s): blk_timeout\n", __func__, name);
		err = true;
	}
	if (rsp_sts.s.blk_crc_err) {
		pr_warn("%s(%s): blk_crc_err\n", __func__, name);
		err = true;
	}
	if (rsp_sts.s.stp_timeout) {
		pr_warn("%s(%s): stp_timeout\n", __func__, name);
		err = true;
	}
	if (rsp_sts.s.stp_crc_err) {
		pr_warn("%s(%s): stp_crc_err\n", __func__, name);
		err = true;
	}
	if (rsp_sts.s.stp_bad_sts) {
		pr_warn("%s(%s): stp_bad_sts\n", __func__, name);
		err = true;
	}
	if (err)
		pr_warn("  rsp_sts: 0x%llx\n", rsp_sts.u);

	return err ? -1 : 0;
}

/**
 * Starts a DMA operation for block read/write
 *
 * @param	mmc	mmc device
 * @param	write	true if write operation
 * @param	clear	true to clear DMA operation
 * @param	adr	source or destination DMA address
 * @param	size	size in blocks
 * @param	timeout	timeout in ms
 */
static void octeontx_mmc_start_dma(struct mmc *mmc, bool write,
				   bool clear, u32 block, dma_addr_t adr,
				   u32 size, int timeout)
{
	const struct octeontx_mmc_slot *slot = mmc_to_slot(mmc);
	union mio_emm_dma_cfg emm_dma_cfg;
	union mio_emm_dma_adr emm_dma_adr;
	union mio_emm_dma emm_dma;

	/* Clear any interrupts */
	write_csr(mmc, MIO_EMM_DMA_INT(),
		  read_csr(mmc, MIO_EMM_DMA_INT()));

	emm_dma_cfg.u = 0;
	emm_dma_cfg.s.en = 1;
	emm_dma_cfg.s.rw = !!write;
	emm_dma_cfg.s.clr = !!clear;
	emm_dma_cfg.s.size = ((u64)(size * mmc->read_bl_len) / 8) - 1;
#if __BYTE_ORDER != __BIG_ENDIAN
	emm_dma_cfg.s.endian = 1;
#endif
	emm_dma_adr.u = 0;
	emm_dma_adr.s.adr = adr;
	write_csr(mmc, MIO_EMM_DMA_ADR(), emm_dma_adr.u);
	write_csr(mmc, MIO_EMM_DMA_CFG(), emm_dma_cfg.u);

	emm_dma.u = 0;
	emm_dma.s.bus_id = slot->bus_id;
	emm_dma.s.dma_val = 1;
	emm_dma.s.rw = !!write;
	emm_dma.s.sector = mmc->high_capacity ? 1 : 0;

	if (size > 1 && ((IS_SD(mmc) && (mmc->scr[0] & 2)) || !IS_SD(mmc)))
		emm_dma.s.multi = 1;
	else
		emm_dma.s.multi = 0;

	emm_dma.s.block_cnt = size;
	if (!mmc->high_capacity)
		block *= mmc->read_bl_len;
	emm_dma.s.card_addr = block;
	debug("%s(%s): card address: 0x%x, size: %d, multi: %d\n",
	      __func__, mmc->dev->name, block, size, emm_dma.s.multi);

	if (timeout > 0)
		timeout = (timeout * 1000) - 1000;
	set_wdog(mmc, timeout);

	debug("  Writing 0x%llx to mio_emm_dma\n", emm_dma.u);
	write_csr(mmc, MIO_EMM_DMA(), emm_dma.u);
}

/**
 * Waits for a DMA operation to complete
 *
 * @param	mmc	mmc device
 * @param	timeout	timeout in ms
 *
 * Return:	0 for success (could be DMA errors), -ETIMEDOUT on timeout
 */

/**
 * Cleanup DMA engine after a failure
 *
 * @param	mmc	mmc device
 * @param	rsp_sts	rsp status
 */
static void octeontx_mmc_cleanup_dma(struct mmc *mmc,
				     union mio_emm_rsp_sts rsp_sts)
{
	struct octeontx_mmc_slot *slot = mmc_to_slot(mmc);
	union mio_emm_dma emm_dma;
	ulong start;
	int retries = 3;

	do {
		debug("%s(%s): rsp_sts: 0x%llx, rsp_lo: 0x%llx, dma_int: 0x%llx\n",
		      __func__, mmc->dev->name, rsp_sts.u,
		      read_csr(mmc, MIO_EMM_RSP_LO()),
		      read_csr(mmc, MIO_EMM_DMA_INT()));
		emm_dma.u = read_csr(mmc, MIO_EMM_DMA());
		emm_dma.s.dma_val = 1;
		emm_dma.s.dat_null = 1;
		emm_dma.s.bus_id = slot->bus_id;
		write_csr(mmc, MIO_EMM_DMA(), emm_dma.u);
		start = get_timer(0);
		do {
			rsp_sts.u = read_csr(mmc, MIO_EMM_RSP_STS());
			schedule();
		} while (get_timer(start) < 100 &&
			 (rsp_sts.s.dma_val || rsp_sts.s.dma_pend));
	} while (retries-- >= 0 && rsp_sts.s.dma_pend);
	if (rsp_sts.s.dma_val)
		pr_err("%s(%s): Error: could not clean up DMA.  RSP_STS: 0x%llx, RSP_LO: 0x%llx\n",
		       __func__, mmc->dev->name, rsp_sts.u,
		       read_csr(mmc, MIO_EMM_RSP_LO()));
	debug("  rsp_sts after clearing up DMA: 0x%llx\n",
	      read_csr(mmc, MIO_EMM_RSP_STS()));
}

/**
 * Waits for a DMA operation to complete
 *
 * @param	mmc	mmc device
 * @param	timeout	timeout in ms
 * @param	verbose	true to print out error information
 *
 * Return:	0 for success (could be DMA errors), -ETIMEDOUT on timeout
 *		or -EIO if IO error.
 */
static int octeontx_mmc_wait_dma(struct mmc *mmc, bool write, ulong timeout,
				 bool verbose)
{
	struct octeontx_mmc_host *host = mmc_to_host(mmc);
	ulong start_time = get_timer(0);
	union mio_emm_dma_int emm_dma_int;
	union mio_emm_rsp_sts rsp_sts;
	union mio_emm_dma emm_dma;
	bool timed_out = false;
	bool err = false;

	debug("%s(%s, %lu, %d), delay: %uus\n", __func__, mmc->dev->name,
	      timeout, verbose, host->dma_wait_delay);

	udelay(host->dma_wait_delay);
	do {
		emm_dma_int.u = read_csr(mmc, MIO_EMM_DMA_INT());
		rsp_sts.u = read_csr(mmc, MIO_EMM_RSP_STS());
		if (write) {
			if ((rsp_sts.s.dma_pend && !rsp_sts.s.dma_val) ||
			    rsp_sts.s.blk_timeout ||
			    rsp_sts.s.stp_timeout ||
			    rsp_sts.s.rsp_timeout) {
				err = true;
#ifdef DEBUG
				debug("%s: f1\n", __func__);
				octeontx_mmc_print_rsp_errors(mmc, rsp_sts);
#endif
				break;
			}
		} else {
			if (rsp_sts.s.blk_crc_err ||
			    (rsp_sts.s.dma_pend && !rsp_sts.s.dma_val)) {
				err = true;
#if defined(DEBUG)
				octeontx_mmc_print_rsp_errors(mmc, rsp_sts);
#endif
				break;
			}
		}
		if (rsp_sts.s.dma_pend) {
			/*
			 * If this is set then an error has occurred.
			 * Try and restart the DMA operation.
			 */
			emm_dma.u = read_csr(mmc, MIO_EMM_DMA());
			if (verbose) {
				pr_err("%s(%s): DMA pending error: rsp_sts: 0x%llx, dma_int: 0x%llx, emm_dma: 0x%llx\n",
				       __func__, mmc->dev->name, rsp_sts.u,
				       emm_dma_int.u, emm_dma.u);
				octeontx_print_rsp_sts(mmc);
				debug("  MIO_EMM_DEBUG: 0x%llx\n",
				      read_csr(mmc, MIO_EMM_DEBUG()));
				pr_err("%s: Trying DMA resume...\n", __func__);
			}
			emm_dma.s.dma_val = 1;
			emm_dma.s.dat_null = 1;
			write_csr(mmc, MIO_EMM_DMA(), emm_dma.u);
			udelay(10);
		} else if (!rsp_sts.s.dma_val && emm_dma_int.s.done) {
			break;
		}
		schedule();
		timed_out = (get_timer(start_time) > timeout);
	} while (!timed_out);

	if (timed_out || err) {
		if (verbose) {
			pr_err("%s(%s): MMC DMA %s after %lu ms, rsp_sts: 0x%llx, dma_int: 0x%llx, rsp_sts_lo: 0x%llx, emm_dma: 0x%llx\n",
			       __func__, mmc->dev->name,
			       timed_out ? "timed out" : "error",
			       get_timer(start_time), rsp_sts.u,
			       emm_dma_int.u,
			       read_csr(mmc, MIO_EMM_RSP_LO()),
			       read_csr(mmc, MIO_EMM_DMA()));
			octeontx_print_rsp_sts(mmc);
		}
		if (rsp_sts.s.dma_pend)
			octeontx_mmc_cleanup_dma(mmc, rsp_sts);
	} else {
		write_csr(mmc, MIO_EMM_DMA_INT(),
			  read_csr(mmc, MIO_EMM_DMA_INT()));
	}

	return timed_out ? -ETIMEDOUT : (err ? -EIO : 0);
}

/**
 * Read blocks from the MMC/SD device
 *
 * @param	mmc	mmc device
 * @param	cmd	command
 * @param	data	data for read
 * @param	verbose	true to print out error information
 *
 * Return:	number of blocks read or 0 if error
 */
static int octeontx_mmc_read_blocks(struct mmc *mmc, struct mmc_cmd *cmd,
				    struct mmc_data *data, bool verbose)
{
	struct octeontx_mmc_host *host = mmc_to_host(mmc);
	union mio_emm_rsp_sts rsp_sts;
	dma_addr_t dma_addr = (dma_addr_t)dm_pci_virt_to_mem(host->dev,
							     data->dest);
	ulong count;
	ulong blkcnt = data->blocks;
	ulong start = cmd->cmdarg;
	int timeout = 1000 + blkcnt * 20;
	bool timed_out = false;
	bool multi_xfer = cmd->cmdidx == MMC_CMD_READ_MULTIPLE_BLOCK;

	debug("%s(%s): dest: %p, dma address: 0x%llx, blkcnt: %lu, start: %lu\n",
	      __func__, mmc->dev->name, data->dest, dma_addr, blkcnt, start);
	debug("%s: rsp_sts: 0x%llx\n", __func__,
	      read_csr(mmc, MIO_EMM_RSP_STS()));
	/* use max timeout for multi-block transfers */
	/* timeout = 0; */

	/*
	 * If we have a valid SD card in the slot, we set the response bit
	 * mask to check for CRC errors and timeouts only.
	 * Otherwise, use the default power on reset value.
	 */
	write_csr(mmc, MIO_EMM_STS_MASK(),
		  IS_SD(mmc) ? 0x00b00000ull : 0xe4390080ull);
	invalidate_dcache_range((u64)data->dest,
				(u64)data->dest + blkcnt * data->blocksize);

	if (multi_xfer) {
		octeontx_mmc_start_dma(mmc, false, false, start, dma_addr,
				       blkcnt, timeout);
		timed_out = !!octeontx_mmc_wait_dma(mmc, false, timeout,
						    verbose);
		rsp_sts.u = read_csr(mmc, MIO_EMM_RSP_STS());
		if (timed_out || rsp_sts.s.dma_val || rsp_sts.s.dma_pend) {
			if (!verbose)
				return 0;

			pr_err("%s(%s): Error: DMA timed out.  rsp_sts: 0x%llx, emm_int: 0x%llx, dma_int: 0x%llx, rsp_lo: 0x%llx\n",
			       __func__, mmc->dev->name, rsp_sts.u,
			       read_csr(mmc, MIO_EMM_INT()),
			       read_csr(mmc, MIO_EMM_DMA_INT()),
			       read_csr(mmc, MIO_EMM_RSP_LO()));
			pr_err("%s: block count: %lu, start: 0x%lx\n",
			       __func__, blkcnt, start);
			octeontx_mmc_print_registers(mmc);
			return 0;
		}
	} else {
		count = blkcnt;
		timeout = 1000;
		do {
			octeontx_mmc_start_dma(mmc, false, false, start,
					       dma_addr, 1, timeout);
			dma_addr += mmc->read_bl_len;
			start++;

			timed_out = !!octeontx_mmc_wait_dma(mmc, false,
							    timeout, verbose);
			rsp_sts.u = read_csr(mmc, MIO_EMM_RSP_STS());
			if (timed_out || rsp_sts.s.dma_val ||
			    rsp_sts.s.dma_pend) {
				if (verbose) {
					pr_err("%s: Error: DMA timed out.  rsp_sts: 0x%llx, emm_int: 0x%llx, dma_int: 0x%llx, rsp_lo: 0x%llx\n",
					       __func__, rsp_sts.u,
					       read_csr(mmc, MIO_EMM_INT()),
					       read_csr(mmc, MIO_EMM_DMA_INT()),
					       read_csr(mmc, MIO_EMM_RSP_LO()));
					pr_err("%s: block count: 1, start: 0x%lx\n",
					       __func__, start);
					octeontx_mmc_print_registers(mmc);
				}
				return blkcnt - count;
			}
			schedule();
		} while (--count);
	}
#ifdef DEBUG
	debug("%s(%s): Read %lu (0x%lx) blocks starting at block %u (0x%x) to address %p (dma address 0x%llx)\n",
	      __func__, mmc->dev->name, blkcnt, blkcnt,
	      cmd->cmdarg, cmd->cmdarg, data->dest,
	      dm_pci_virt_to_mem(host->dev, data->dest));
	print_buffer(0, data->dest, 1, 0x200, 0);
#endif
	return blkcnt;
}

static int octeontx_mmc_poll_ready(struct mmc *mmc, ulong timeout)
{
	ulong start;
	struct mmc_cmd cmd;
	int err;
	bool not_ready = false;

	memset(&cmd, 0, sizeof(cmd));
	cmd.cmdidx = MMC_CMD_SEND_STATUS;
	cmd.cmdarg = mmc->rca << 16;
	cmd.resp_type = MMC_RSP_R1;
	start = get_timer(0);
	do {
		err = octeontx_mmc_send_cmd(mmc, &cmd, NULL);
		if (err) {
			pr_err("%s(%s): MMC command error: %d; Retry...\n",
			       __func__, mmc->dev->name, err);
			not_ready = true;
		} else if (cmd.response[0] & R1_READY_FOR_DATA) {
			return 0;
		}
		schedule();
	} while (get_timer(start) < timeout);

	if (not_ready)
		pr_err("%s(%s): MMC command error; Retry timeout\n",
		       __func__, mmc->dev->name);
	return -ETIMEDOUT;
}

static ulong octeontx_mmc_write_blocks(struct mmc *mmc, struct mmc_cmd *cmd,
				       struct mmc_data *data)
{
	struct octeontx_mmc_host *host = mmc_to_host(mmc);
	ulong start = cmd->cmdarg;
	ulong blkcnt = data->blocks;
	dma_addr_t dma_addr;
	union mio_emm_rsp_sts rsp_sts;
	union mio_emm_sts_mask emm_sts_mask;
	ulong timeout;
	int count;
	bool timed_out = false;
	bool multi_xfer = (blkcnt > 1) &&
			((IS_SD(mmc) && mmc->scr[0] & 2) || !IS_SD(mmc));

	octeontx_mmc_switch_to(mmc);
	emm_sts_mask.u = 0;
	emm_sts_mask.s.sts_msk = R1_BLOCK_WRITE_MASK;
	write_csr(mmc, MIO_EMM_STS_MASK(), emm_sts_mask.u);

	if (octeontx_mmc_poll_ready(mmc, 10000)) {
		pr_err("%s(%s): Ready timed out\n", __func__, mmc->dev->name);
		return 0;
	}
	flush_dcache_range((u64)data->src,
			   (u64)data->src + blkcnt * mmc->write_bl_len);
	dma_addr = (u64)dm_pci_virt_to_mem(host->dev, (void *)data->src);
	if (multi_xfer) {
		timeout = 5000 + 100 * blkcnt;
		octeontx_mmc_start_dma(mmc, true, false, start, dma_addr,
				       blkcnt, timeout);
		timed_out = !!octeontx_mmc_wait_dma(mmc, true, timeout, true);
		rsp_sts.u = read_csr(mmc, MIO_EMM_RSP_STS());
		if (timed_out || rsp_sts.s.dma_val || rsp_sts.s.dma_pend) {
			pr_err("%s(%s): Error: multi-DMA timed out after %lums.  rsp_sts: 0x%llx, emm_int: 0x%llx, emm_dma_int: 0x%llx, rsp_sts_lo: 0x%llx, emm_dma: 0x%llx\n",
			       __func__, mmc->dev->name, timeout,
			       rsp_sts.u,
			       read_csr(mmc, MIO_EMM_INT()),
			       read_csr(mmc, MIO_EMM_DMA_INT()),
			       read_csr(mmc, MIO_EMM_RSP_LO()),
			       read_csr(mmc, MIO_EMM_DMA()));
			return 0;
		}
	} else {
		timeout = 5000;
		count = blkcnt;
		do {
			octeontx_mmc_start_dma(mmc, true, false, start,
					       dma_addr, 1, timeout);
			dma_addr += mmc->read_bl_len;
			start++;

			timed_out = !!octeontx_mmc_wait_dma(mmc, true, timeout,
							    true);
			rsp_sts.u = read_csr(mmc, MIO_EMM_RSP_STS());
			if (timed_out || rsp_sts.s.dma_val ||
			    rsp_sts.s.dma_pend) {
				pr_err("%s(%s): Error: single-DMA timed out after %lums.  rsp_sts: 0x%llx, emm_int: 0x%llx, emm_dma_int: 0x%llx, rsp_sts_lo: 0x%llx, emm_dma: 0x%llx\n",
				       __func__, mmc->dev->name, timeout,
				       rsp_sts.u,
				       read_csr(mmc, MIO_EMM_RSP_STS()),
				       read_csr(mmc, MIO_EMM_DMA_INT()),
				       read_csr(mmc, MIO_EMM_RSP_LO()),
				       read_csr(mmc, MIO_EMM_DMA()));
				return blkcnt - count;
			}
			schedule();
		} while (--count);
	}

	return blkcnt;
}

/**
 * Send a command to the eMMC/SD device
 *
 * @param mmc	mmc device
 * @param cmd	cmd to send and response
 * @param data	additional data
 * @param flags
 * Return:	0 for success, otherwise error
 */
static int octeontx_mmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd,
				 struct mmc_data *data)
{
	struct octeontx_mmc_slot *slot = mmc_to_slot(mmc);
	const char *name = slot->dev->name;
	struct octeontx_mmc_cr_mods mods = {0, 0};
	union mio_emm_rsp_sts rsp_sts;
	union mio_emm_cmd emm_cmd;
	union mio_emm_rsp_lo rsp_lo;
	union mio_emm_buf_idx emm_buf_idx;
	union mio_emm_buf_dat emm_buf_dat;
	ulong start;
	int i;
	ulong blkcnt;

	/**
	 * This constant has a 1 bit for each command which should have a short
	 * timeout and a 0 for each bit with a long timeout.  Currently the
	 * following commands have a long timeout:
	 *   CMD6, CMD17, CMD18, CMD24, CMD25, CMD32, CMD33, CMD35, CMD36 and
	 *   CMD38.
	 */
	static const u64 timeout_short = 0xFFFFFFA4FCF9FFDFull;
	uint timeout;

	if (cmd->cmdidx == MMC_CMD_SEND_EXT_CSD) {
		union mio_emm_rca emm_rca;

		emm_rca.u = 0;
		emm_rca.s.card_rca = mmc->rca;
		write_csr(mmc, MIO_EMM_RCA(), emm_rca.u);
	}

	if (timeout_short & (1ull << cmd->cmdidx))
		timeout = MMC_TIMEOUT_SHORT;
	else if (cmd->cmdidx == MMC_CMD_SWITCH && IS_SD(mmc))
		timeout = 2560;
	else if (cmd->cmdidx == MMC_CMD_ERASE)
		timeout = MMC_TIMEOUT_ERASE;
	else
		timeout = MMC_TIMEOUT_LONG;

	debug("%s(%s): cmd idx: %u, arg: 0x%x, resp type: 0x%x, timeout: %u\n",
	      __func__, name, cmd->cmdidx, cmd->cmdarg, cmd->resp_type,
	      timeout);
	if (data)
		debug("  data: addr: %p, flags: 0x%x, blocks: %u, blocksize: %u\n",
		      data->dest, data->flags, data->blocks, data->blocksize);

	octeontx_mmc_switch_to(mmc);

	/* Clear any interrupts */
	write_csr(mmc, MIO_EMM_INT(), read_csr(mmc, MIO_EMM_INT()));

	/*
	 * We need to override the default command types and response types
	 * when dealing with SD cards.
	 */
	mods = octeontx_mmc_get_cr_mods(mmc, cmd, data);

	/* Handle block read/write/stop operations */
	switch (cmd->cmdidx) {
	case MMC_CMD_GO_IDLE_STATE:
		slot->tuned = false;
		slot->hs200_tuned = false;
		slot->hs400_tuned = false;
		break;
	case MMC_CMD_STOP_TRANSMISSION:
		return 0;
	case MMC_CMD_READ_MULTIPLE_BLOCK:
	case MMC_CMD_READ_SINGLE_BLOCK:
		pr_debug("%s(%s): Reading blocks\n", __func__, name);
		blkcnt = octeontx_mmc_read_blocks(mmc, cmd, data, true);
		return (blkcnt > 0) ? 0 : -1;
	case MMC_CMD_WRITE_MULTIPLE_BLOCK:
	case MMC_CMD_WRITE_SINGLE_BLOCK:
		blkcnt = octeontx_mmc_write_blocks(mmc, cmd, data);
		return (blkcnt > 0) ? 0 : -1;
	case MMC_CMD_SELECT_CARD:
		/* Set the RCA register (is it set automatically?) */
		if (IS_SD(mmc)) {
			union mio_emm_rca emm_rca;

			emm_rca.u = 0;
			emm_rca.s.card_rca = (cmd->cmdarg >> 16);
			write_csr(mmc, MIO_EMM_RCA(), emm_rca.u);
			debug("%s: Set SD relative address (RCA) to 0x%x\n",
			      __func__, emm_rca.s.card_rca);
		}
		break;

	case MMC_CMD_SWITCH:
		if (!data && !slot->is_acmd)
			octeontx_mmc_track_switch(mmc, cmd->cmdarg);
		break;
	}

	emm_cmd.u = 0;
	emm_cmd.s.cmd_val = 1;
	emm_cmd.s.bus_id = slot->bus_id;
	emm_cmd.s.cmd_idx = cmd->cmdidx;
	emm_cmd.s.arg = cmd->cmdarg;
	emm_cmd.s.ctype_xor = mods.ctype_xor;
	emm_cmd.s.rtype_xor = mods.rtype_xor;
	if (data && data->blocks == 1 && data->blocksize != 512) {
		emm_cmd.s.offset =
			64 - ((data->blocks * data->blocksize) / 8);
		debug("%s: offset set to %u\n", __func__, emm_cmd.s.offset);
	}

	if (data && data->flags & MMC_DATA_WRITE) {
		u8 *src = (u8 *)data->src;

		if (!src) {
			pr_err("%s(%s): Error: data source for cmd 0x%x is NULL!\n",
			       __func__, name, cmd->cmdidx);
			return -1;
		}
		if (data->blocksize > 512) {
			pr_err("%s(%s): Error: data for cmd 0x%x exceeds 512 bytes\n",
			       __func__, name, cmd->cmdidx);
			return -1;
		}
#ifdef DEBUG
		debug("%s: Sending %d bytes data\n", __func__, data->blocksize);
		print_buffer(0, src, 1, data->blocksize, 0);
#endif
		emm_buf_idx.u = 0;
		emm_buf_idx.s.inc = 1;
		write_csr(mmc, MIO_EMM_BUF_IDX(), emm_buf_idx.u);
		for (i = 0; i < (data->blocksize + 7) / 8; i++) {
			memcpy(&emm_buf_dat.u, src, sizeof(emm_buf_dat.u));
			write_csr(mmc, MIO_EMM_BUF_DAT(),
				  cpu_to_be64(emm_buf_dat.u));
			src += sizeof(emm_buf_dat.u);
		}
		write_csr(mmc, MIO_EMM_BUF_IDX(), 0);
	}
	debug("%s(%s): Sending command %u (emm_cmd: 0x%llx)\n", __func__,
	      name, cmd->cmdidx, emm_cmd.u);
	set_wdog(mmc, timeout * 1000);
	write_csr(mmc, MIO_EMM_CMD(), emm_cmd.u);

	/* Wait for command to finish or time out */
	start = get_timer(0);
	do {
		rsp_sts.u = read_csr(mmc, MIO_EMM_RSP_STS());
		schedule();
	} while (!rsp_sts.s.cmd_done && !rsp_sts.s.rsp_timeout &&
		 (get_timer(start) < timeout + 10));
	octeontx_mmc_print_rsp_errors(mmc, rsp_sts);
	if (rsp_sts.s.rsp_timeout || !rsp_sts.s.cmd_done) {
		debug("%s(%s): Error: command %u(0x%x) timed out.  rsp_sts: 0x%llx\n",
		      __func__, name, cmd->cmdidx, cmd->cmdarg, rsp_sts.u);
		octeontx_mmc_print_registers(mmc);
		return -ETIMEDOUT;
	}
	if (rsp_sts.s.rsp_crc_err) {
		debug("%s(%s): RSP CRC error, rsp_sts: 0x%llx, cmdidx: %u, arg: 0x%08x\n",
		      __func__, name, rsp_sts.u, cmd->cmdidx, cmd->cmdarg);
		octeontx_mmc_print_registers(mmc);
		return -1;
	}
	if (slot->bus_id != rsp_sts.s.bus_id) {
		pr_warn("%s(%s): bus id mismatch, got %d, expected %d for command 0x%x(0x%x)\n",
			__func__, name,
			rsp_sts.s.bus_id, slot->bus_id,
			cmd->cmdidx, cmd->cmdarg);
		goto error;
	}
	if (rsp_sts.s.rsp_bad_sts) {
		rsp_lo.u = read_csr(mmc, MIO_EMM_RSP_LO());
		debug("%s: Bad response for bus id %d, cmd id %d:\n"
		      "    rsp_timeout: %d\n"
		      "    rsp_bad_sts: %d\n"
		      "    rsp_crc_err: %d\n",
		      __func__, slot->bus_id, cmd->cmdidx,
		      rsp_sts.s.rsp_timeout,
		      rsp_sts.s.rsp_bad_sts,
		      rsp_sts.s.rsp_crc_err);
		if (rsp_sts.s.rsp_type == 1 && rsp_sts.s.rsp_bad_sts) {
			debug("    Response status: 0x%llx\n",
			      (rsp_lo.u >> 8) & 0xffffffff);
#ifdef DEBUG
			mmc_print_status((rsp_lo.u >> 8) & 0xffffffff);
#endif
		}
		goto error;
	}
	if (rsp_sts.s.cmd_idx != cmd->cmdidx) {
		debug("%s(%s): Command response index %d does not match command index %d\n",
		      __func__, name, rsp_sts.s.cmd_idx, cmd->cmdidx);
		octeontx_print_rsp_sts(mmc);
		debug("%s: rsp_lo: 0x%llx\n", __func__,
		      read_csr(mmc, MIO_EMM_RSP_LO()));

		goto error;
	}

	slot->is_acmd = (cmd->cmdidx == MMC_CMD_APP_CMD);

	if (!cmd->resp_type & MMC_RSP_PRESENT)
		debug("  Response type: 0x%x, no response expected\n",
		      cmd->resp_type);
	/* Get the response if present */
	if (rsp_sts.s.rsp_val && (cmd->resp_type & MMC_RSP_PRESENT)) {
		union mio_emm_rsp_hi rsp_hi;

		rsp_lo.u = read_csr(mmc, MIO_EMM_RSP_LO());

		switch (rsp_sts.s.rsp_type) {
		case 1:
		case 3:
		case 4:
		case 5:
			cmd->response[0] = (rsp_lo.u >> 8) & 0xffffffffull;
			debug("  response: 0x%08x\n",
			      cmd->response[0]);
			cmd->response[1] = 0;
			cmd->response[2] = 0;
			cmd->response[3] = 0;
			break;
		case 2:
			cmd->response[3] = rsp_lo.u & 0xffffffff;
			cmd->response[2] = (rsp_lo.u >> 32) & 0xffffffff;
			rsp_hi.u = read_csr(mmc, MIO_EMM_RSP_HI());
			cmd->response[1] = rsp_hi.u & 0xffffffff;
			cmd->response[0] = (rsp_hi.u >> 32) & 0xffffffff;
			debug("  response: 0x%08x 0x%08x 0x%08x 0x%08x\n",
			      cmd->response[0], cmd->response[1],
			      cmd->response[2], cmd->response[3]);
			break;
		default:
			pr_err("%s(%s): Unknown response type 0x%x for command %d, arg: 0x%x, rsp_sts: 0x%llx\n",
			       __func__, name, rsp_sts.s.rsp_type, cmd->cmdidx,
			       cmd->cmdarg, rsp_sts.u);
			return -1;
		}
	} else {
		debug("  Response not expected\n");
	}

	if (data && data->flags & MMC_DATA_READ) {
		u8 *dest = (u8 *)data->dest;

		if (!dest) {
			pr_err("%s(%s): Error, destination buffer NULL!\n",
			       __func__, mmc->dev->name);
			goto error;
		}
		if (data->blocksize > 512) {
			printf("%s(%s): Error: data size %u exceeds 512\n",
			       __func__, mmc->dev->name,
			       data->blocksize);
			goto error;
		}
		emm_buf_idx.u = 0;
		emm_buf_idx.s.inc = 1;
		write_csr(mmc, MIO_EMM_BUF_IDX(), emm_buf_idx.u);
		for (i = 0; i < (data->blocksize + 7) / 8; i++) {
			emm_buf_dat.u = read_csr(mmc, MIO_EMM_BUF_DAT());
			emm_buf_dat.u = be64_to_cpu(emm_buf_dat.u);
			memcpy(dest, &emm_buf_dat.u, sizeof(emm_buf_dat.u));
			dest += sizeof(emm_buf_dat.u);
		}
		write_csr(mmc, MIO_EMM_BUF_IDX(), 0);
#ifdef DEBUG
		debug("%s: Received %d bytes data\n", __func__,
		      data->blocksize);
		print_buffer(0, data->dest, 1, data->blocksize, 0);
#endif
	}

	return 0;
error:
#ifdef DEBUG
	octeontx_mmc_print_registers(mmc);
#endif
	return -1;
}

static int octeontx_mmc_dev_send_cmd(struct udevice *dev, struct mmc_cmd *cmd,
				     struct mmc_data *data)
{
	return octeontx_mmc_send_cmd(dev_to_mmc(dev), cmd, data);
}

#ifdef MMC_SUPPORTS_TUNING
static int octeontx_mmc_test_cmd(struct mmc *mmc, u32 opcode, int *statp)
{
	struct mmc_cmd cmd;
	int err;

	memset(&cmd, 0, sizeof(cmd));

	debug("%s(%s, %u, %p)\n", __func__, mmc->dev->name, opcode, statp);
	cmd.cmdidx = opcode;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg = mmc->rca << 16;

	err = octeontx_mmc_send_cmd(mmc, &cmd, NULL);
	if (err)
		debug("%s(%s, %u) returned %d\n", __func__,
		      mmc->dev->name, opcode, err);
	if (statp)
		*statp = cmd.response[0];
	return err;
}

static int octeontx_mmc_test_get_ext_csd(struct mmc *mmc, u32 opcode,
					 int *statp)
{
	struct mmc_cmd cmd;
	struct mmc_data data;
	int err;
	u8 ext_csd[MMC_MAX_BLOCK_LEN];

	debug("%s(%s, %u, %p)\n",  __func__, mmc->dev->name, opcode, statp);
	memset(&cmd, 0, sizeof(cmd));

	cmd.cmdidx = MMC_CMD_SEND_EXT_CSD;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg = 0;

	data.dest = (char *)ext_csd;
	data.blocks = 1;
	data.blocksize = MMC_MAX_BLOCK_LEN;
	data.flags = MMC_DATA_READ;

	err = octeontx_mmc_send_cmd(mmc, &cmd, &data);
	if (statp)
		*statp = cmd.response[0];

	return err;
}

/**
 * Wrapper to set the MIO_EMM_TIMING register
 *
 * @param	mmc		pointer to mmc data structure
 * @param	emm_timing	New emm_timing register value
 *
 * On some devices it is possible that changing the data out value can
 * cause a glitch on an internal fifo.  This works around this problem
 * by performing a soft-reset immediately before setting the timing register.
 *
 * Note: this function should not be called from any function that
 * performs DMA or block operations since not all registers are
 * preserved.
 */
static void octeontx_mmc_set_emm_timing(struct mmc *mmc,
					union mio_emm_timing emm_timing)
{
	union mio_emm_cfg emm_cfg;
	struct octeontx_mmc_slot *slot = mmc->priv;
	union mio_emm_debug emm_debug;

	debug("%s(%s, 0x%llx) din: %u\n", __func__, mmc->dev->name,
	      emm_timing.u, emm_timing.s.data_in_tap);

	udelay(1);
	if (slot->host->tap_requires_noclk) {
		/* Turn off the clock */
		emm_debug.u = read_csr(mmc, MIO_EMM_DEBUG());
		emm_debug.s.emmc_clk_disable = 1;
		write_csr(mmc, MIO_EMM_DEBUG(), emm_debug.u);
		udelay(1);
		emm_debug.s.rdsync_rst = 1;
		write_csr(mmc, MIO_EMM_DEBUG(), emm_debug.u);
	}
	emm_cfg.u = read_csr(mmc, MIO_EMM_CFG());
	emm_cfg.s.bus_ena = 1 << 3;
	write_csr(mmc, MIO_EMM_CFG(), emm_cfg.u);

	udelay(1);
	write_csr(mmc, MIO_EMM_TIMING(), emm_timing.u);
	udelay(1);

	if (slot->host->tap_requires_noclk) {
		/* Turn on the clock */
		emm_debug.s.rdsync_rst = 0;
		write_csr(mmc, MIO_EMM_DEBUG(), emm_debug.u);
		udelay(1);
		emm_debug.s.emmc_clk_disable = 0;
		write_csr(mmc, MIO_EMM_DEBUG(), emm_debug.u);
		udelay(1);
	}
	emm_cfg.s.bus_ena = 1 << mmc_to_slot(mmc)->bus_id;
	write_csr(mmc, MIO_EMM_CFG(), emm_cfg.u);
}

static const u8 octeontx_hs400_tuning_block[512] = {
	0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00,
	0xff, 0xff, 0xcc, 0xcc, 0xcc, 0x33, 0xcc, 0xcc,
	0xcc, 0x33, 0x33, 0xcc, 0xcc, 0xcc, 0xff, 0xff,
	0xff, 0xee, 0xff, 0xff, 0xff, 0xee, 0xee, 0xff,
	0xff, 0xff, 0xdd, 0xff, 0xff, 0xff, 0xdd, 0xdd,
	0xff, 0xff, 0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb,
	0xbb, 0xff, 0xff, 0xff, 0x77, 0xff, 0xff, 0xff,
	0x77, 0x77, 0xff, 0x77, 0xbb, 0xdd, 0xee, 0xff,
	0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00,
	0x00, 0xff, 0xff, 0xcc, 0xcc, 0xcc, 0x33, 0xcc,
	0xcc, 0xcc, 0x33, 0x33, 0xcc, 0xcc, 0xcc, 0xff,
	0xff, 0xff, 0xee, 0xff, 0xff, 0xff, 0xee, 0xee,
	0xff, 0xff, 0xff, 0xdd, 0xff, 0xff, 0xff, 0xdd,
	0xdd, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff, 0xff,
	0xbb, 0xbb, 0xff, 0xff, 0xff, 0x77, 0xff, 0xff,
	0xff, 0x77, 0x77, 0xff, 0x77, 0xbb, 0xdd, 0xee,
	0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00,
	0xff, 0xff, 0xcc, 0xcc, 0xcc, 0x33, 0xcc, 0xcc,
	0xcc, 0x33, 0x33, 0xcc, 0xcc, 0xcc, 0xff, 0xff,
	0xff, 0xee, 0xff, 0xff, 0xff, 0xee, 0xee, 0xff,
	0xff, 0xff, 0xdd, 0xff, 0xff, 0xff, 0xdd, 0xdd,
	0xff, 0xff, 0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb,
	0xbb, 0xff, 0xff, 0xff, 0x77, 0xff, 0xff, 0xff,
	0x77, 0x77, 0xff, 0x77, 0xbb, 0xdd, 0xee, 0xff,
	0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00,
	0x00, 0xff, 0xff, 0xcc, 0xcc, 0xcc, 0x33, 0xcc,
	0xcc, 0xcc, 0x33, 0x33, 0xcc, 0xcc, 0xcc, 0xff,
	0xff, 0xff, 0xee, 0xff, 0xff, 0xff, 0xee, 0xee,
	0xff, 0xff, 0xff, 0xdd, 0xff, 0xff, 0xff, 0xdd,
	0xdd, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff, 0xff,
	0xbb, 0xbb, 0xff, 0xff, 0xff, 0x77, 0xff, 0xff,
	0xff, 0x77, 0x77, 0xff, 0x77, 0xbb, 0xdd, 0xee,
	0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00,
	0xff, 0xff, 0xcc, 0xcc, 0xcc, 0x33, 0xcc, 0xcc,
	0xcc, 0x33, 0x33, 0xcc, 0xcc, 0xcc, 0xff, 0xff,
	0xff, 0xee, 0xff, 0xff, 0xff, 0xee, 0xee, 0xff,
	0xff, 0xff, 0xdd, 0xff, 0xff, 0xff, 0xdd, 0xdd,
	0xff, 0xff, 0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb,
	0xbb, 0xff, 0xff, 0xff, 0x77, 0xff, 0xff, 0xff,
	0x77, 0x77, 0xff, 0x77, 0xbb, 0xdd, 0xee, 0xff,
	0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00,
	0x00, 0xff, 0xff, 0xcc, 0xcc, 0xcc, 0x33, 0xcc,
	0xcc, 0xcc, 0x33, 0x33, 0xcc, 0xcc, 0xcc, 0xff,
	0xff, 0xff, 0xee, 0xff, 0xff, 0xff, 0xee, 0xee,
	0xff, 0xff, 0xff, 0xdd, 0xff, 0xff, 0xff, 0xdd,
	0xdd, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff, 0xff,
	0xbb, 0xbb, 0xff, 0xff, 0xff, 0x77, 0xff, 0xff,
	0xff, 0x77, 0x77, 0xff, 0x77, 0xbb, 0xdd, 0xee,
	0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0x00,
	0x00, 0xff, 0x00, 0xff, 0x55, 0xaa, 0x55, 0xaa,
	0xcc, 0x33, 0x33, 0xcc, 0xcc, 0xcc, 0xff, 0xff,
	0xff, 0xee, 0xff, 0xff, 0xff, 0xee, 0xee, 0xff,
	0xff, 0xff, 0xdd, 0xff, 0xff, 0xff, 0xdd, 0xdd,
	0xff, 0xff, 0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb,
	0xbb, 0xff, 0xff, 0xff, 0x77, 0xff, 0xff, 0xff,
	0x77, 0x77, 0xff, 0x77, 0xbb, 0xdd, 0xee, 0xff,
	0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00,
	0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
	0x01, 0xfe, 0x01, 0xfe, 0xcc, 0xcc, 0xcc, 0xff,
	0xff, 0xff, 0xee, 0xff, 0xff, 0xff, 0xee, 0xee,
	0xff, 0xff, 0xff, 0xdd, 0xff, 0xff, 0xff, 0xdd,
	0xdd, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff, 0xff,
	0xbb, 0xbb, 0xff, 0xff, 0xff, 0x77, 0xff, 0xff,
	0xff, 0x77, 0x77, 0xff, 0x77, 0xbb, 0xdd, 0xee,

};

/**
 * Perform tuning in HS400 mode
 *
 * @param[in]	mmc	mmc data structure
 *
 * @ret		0 for success, otherwise error
 */
static int octeontx_tune_hs400(struct mmc *mmc)
{
	struct octeontx_mmc_slot *slot = mmc_to_slot(mmc);
	struct mmc_cmd cmd;
	struct mmc_data data;
	union mio_emm_timing emm_timing;
	u8 buffer[mmc->read_bl_len];
	int tap_adj;
	int err = -1;
	int tap;
	int run = 0;
	int start_run = -1;
	int best_run = 0;
	int best_start = -1;
	bool prev_ok = false;
	char env_name[64];
	char how[MAX_NO_OF_TAPS + 1] = "";

	if (slot->hs400_tuning_block == -1)
		return 0;

	/* The eMMC standard disables all tuning support when operating in
	 * DDR modes like HS400.  The problem with this is that there are
	 * many cases where the HS200 tuning does not work for HS400 mode.
	 * In order to perform this tuning, while in HS200 a block is written
	 * to a block specified in the device tree (marvell,hs400-tuning-block)
	 * which is used for tuning in this function by repeatedly reading
	 * this block and comparing the data and return code.  This function
	 * chooses the data input tap in the middle of the longest run of
	 * successful read operations.
	 */

	emm_timing = slot->hs200_taps;
	debug("%s(%s): Start ci: %d, co: %d, di: %d, do: %d\n",
	      __func__, mmc->dev->name, emm_timing.s.cmd_in_tap,
	      emm_timing.s.cmd_out_tap, emm_timing.s.data_in_tap,
	      emm_timing.s.data_out_tap);
	memset(buffer, 0xdb, sizeof(buffer));

	snprintf(env_name, sizeof(env_name), "emmc%d_data_in_tap_hs400",
		 slot->bus_id);
	tap = env_get_ulong(env_name, 10, -1L);
	if (tap >= 0 && tap < MAX_NO_OF_TAPS) {
		printf("Overriding data input tap for HS400 mode to %d\n", tap);
		emm_timing.s.data_in_tap = tap;
		octeontx_mmc_set_emm_timing(mmc, emm_timing);
		return 0;
	}

	for (tap = 0; tap <= MAX_NO_OF_TAPS; tap++, prev_ok = !err) {
		if (tap < MAX_NO_OF_TAPS) {
			debug("%s: Testing data in tap %d\n", __func__, tap);
			emm_timing.s.data_in_tap = tap;
			octeontx_mmc_set_emm_timing(mmc, emm_timing);

			cmd.cmdidx = MMC_CMD_READ_SINGLE_BLOCK;
			cmd.cmdarg = slot->hs400_tuning_block;
			cmd.resp_type = MMC_RSP_R1;
			data.dest = (void *)buffer;
			data.blocks = 1;
			data.blocksize = mmc->read_bl_len;
			data.flags = MMC_DATA_READ;
			err = !octeontx_mmc_read_blocks(mmc, &cmd, &data,
							false);
			if (err || memcmp(buffer, octeontx_hs400_tuning_block,
					  sizeof(buffer))) {
#ifdef DEBUG
				if (!err) {
					debug("%s: data mismatch.  Read:\n",
					      __func__);
					print_buffer(0, buffer, 1,
						     sizeof(buffer), 0);
					debug("\nExpected:\n");
					print_buffer(0,
					    octeontx_hs400_tuning_block, 1,
					    sizeof(octeontx_hs400_tuning_block),
					    0);
				} else {
					debug("%s: Error %d reading block\n",
					      __func__, err);
				}
#endif
				err = -EINVAL;
			} else {
				debug("%s: tap %d good\n", __func__, tap);
			}
			how[tap] = "-+"[!err];
		} else {
			err = -EINVAL;
		}

		if (!err) {
			if (!prev_ok)
				start_run = tap;
		} else if (prev_ok) {
			run = tap - 1 - start_run;
			if (start_run >= 0 && run > best_run) {
				best_start = start_run;
				best_run = run;
			}
		}
	}

	how[tap - 1] = '\0';
	if (best_start < 0) {
		printf("%s(%s): %lldMHz tuning failed for HS400\n",
		       __func__, mmc->dev->name, slot->clock / 1000000);
		return -EINVAL;
	}
	tap = best_start + best_run / 2;

	snprintf(env_name, sizeof(env_name), "emmc%d_data_in_tap_adj_hs400",
		 slot->bus_id);
	tap_adj = env_get_ulong(env_name, 10, slot->hs400_tap_adj);
	/*
	 * Keep it in range and if out of range force it back in with a small
	 * buffer.
	 */
	if (best_run > 3) {
		tap = tap + tap_adj;
		if (tap >= best_start + best_run)
			tap = best_start + best_run - 2;
		if (tap <= best_start)
			tap = best_start + 2;
	}
	how[tap] = '@';
	debug("Tuning: %s\n", how);
	debug("%s(%s): HS400 tap: best run start: %d, length: %d, tap: %d\n",
	      __func__, mmc->dev->name, best_start, best_run, tap);
	slot->hs400_taps = slot->hs200_taps;
	slot->hs400_taps.s.data_in_tap = tap;
	slot->hs400_tuned = true;
	if (env_get_yesno("emmc_export_hs400_taps") > 0) {
		debug("%s(%s): Exporting HS400 taps\n",
		      __func__, mmc->dev->name);
		env_set_ulong("emmc_timing_tap", slot->host->timing_taps);
		snprintf(env_name, sizeof(env_name),
			 "emmc%d_hs400_data_in_tap_debug",
			 slot->bus_id);
		env_set(env_name, how);
		snprintf(env_name, sizeof(env_name),
			 "emmc%d_hs400_data_in_tap_val",
			 slot->bus_id);
		env_set_ulong(env_name, tap);
		snprintf(env_name, sizeof(env_name),
			 "emmc%d_hs400_data_in_tap_start",
			 slot->bus_id);
		env_set_ulong(env_name, best_start);
		snprintf(env_name, sizeof(env_name),
			 "emmc%d_hs400_data_in_tap_end",
			 slot->bus_id);
		env_set_ulong(env_name, best_start + best_run);
		snprintf(env_name, sizeof(env_name),
			 "emmc%d_hs400_cmd_in_tap",
			 slot->bus_id);
		env_set_ulong(env_name, slot->hs400_taps.s.cmd_in_tap);
		snprintf(env_name, sizeof(env_name),
			 "emmc%d_hs400_cmd_out_tap",
			 slot->bus_id);
		env_set_ulong(env_name, slot->hs400_taps.s.cmd_out_tap);
		snprintf(env_name, sizeof(env_name),
			 "emmc%d_hs400_cmd_out_delay",
			 slot->bus_id);
		env_set_ulong(env_name, slot->cmd_out_hs400_delay);
		snprintf(env_name, sizeof(env_name),
			 "emmc%d_hs400_data_out_tap",
			 slot->bus_id);
		env_set_ulong(env_name, slot->hs400_taps.s.data_out_tap);
		snprintf(env_name, sizeof(env_name),
			 "emmc%d_hs400_data_out_delay",
			 slot->bus_id);
		env_set_ulong(env_name, slot->data_out_hs400_delay);
	} else {
		debug("%s(%s): HS400 environment export disabled\n",
		      __func__, mmc->dev->name);
	}
	octeontx_mmc_set_timing(mmc);

	return 0;
}

struct adj {
	const char *name;
	u8 mask_shift;
	int (*test)(struct mmc *mmc, u32 opcode, int *error);
	u32 opcode;
	bool ddr_only;
	bool hs200_only;
	bool not_hs200_only;
	u8 num_runs;
};

struct adj adj[] = {
	{ "CMD_IN", 48, octeontx_mmc_test_cmd, MMC_CMD_SEND_STATUS,
	  false, false, false, 2, },
/*	{ "CMD_OUT", 32, octeontx_mmc_test_cmd, MMC_CMD_SEND_STATUS, },*/
	{ "DATA_IN(HS200)", 16, mmc_send_tuning,
		MMC_CMD_SEND_TUNING_BLOCK_HS200, false, true, false, 2, },
	{ "DATA_IN", 16, octeontx_mmc_test_get_ext_csd, 0, false, false,
	  true, 2, },
/*	{ "DATA_OUT", 0, octeontx_mmc_test_cmd, 0, true, false},*/
	{ NULL, },
};

/**
 * Perform tuning tests to find optimal timing
 *
 * @param	mmc	mmc device
 * @param	adj	parameter to tune
 * @param	opcode	command opcode to use
 *
 * Return:	0 for success, -1 if tuning failed
 */
static int octeontx_mmc_adjust_tuning(struct mmc *mmc, struct adj *adj,
				      u32 opcode)
{
	struct octeontx_mmc_slot *slot = mmc_to_slot(mmc);
	union mio_emm_timing timing;
	union mio_emm_debug emm_debug;
	int tap;
	int err = -1;
	int run = 0;
	int count;
	int start_run = -1;
	int best_run = 0;
	int best_start = -1;
	bool prev_ok = false;
	u64 tap_status = 0;
	const int tap_adj = slot->hs200_tap_adj;
	char how[MAX_NO_OF_TAPS + 1] = "";
	bool is_hs200 = mmc->selected_mode == MMC_HS_200;

	debug("%s(%s, %s, %d), hs200: %d\n", __func__, mmc->dev->name,
	      adj->name, opcode, is_hs200);
	octeontx_mmc_set_emm_timing(mmc,
				    is_hs200 ? slot->hs200_taps : slot->taps);

#ifdef DEBUG
	if (opcode == MMC_CMD_SEND_TUNING_BLOCK_HS200) {
		printf("%s(%s): Before tuning %s, opcode: %d\n",
		       __func__, mmc->dev->name, adj->name, opcode);
		octeontx_mmc_print_registers2(mmc, NULL);
	}
#endif

	/*
	 * The algorithm to find the optimal timing is to start
	 * at the end and work backwards and select the second
	 * value that passes.  Each test is repeated twice.
	 */
	for (tap = 0; tap <= MAX_NO_OF_TAPS; tap++, prev_ok = !err) {
		if (tap < MAX_NO_OF_TAPS) {
			if (slot->host->tap_requires_noclk) {
				/* Turn off the clock */
				emm_debug.u = read_csr(mmc, MIO_EMM_DEBUG());
				emm_debug.s.emmc_clk_disable = 1;
				write_csr(mmc, MIO_EMM_DEBUG(), emm_debug.u);
				udelay(1);
				emm_debug.s.rdsync_rst = 1;
				write_csr(mmc, MIO_EMM_DEBUG(), emm_debug.u);
				udelay(1);
			}

			timing.u = read_csr(mmc, MIO_EMM_TIMING());
			timing.u &= ~(0x3full << adj->mask_shift);
			timing.u |= (u64)tap << adj->mask_shift;
			write_csr(mmc, MIO_EMM_TIMING(), timing.u);
			debug("%s(%s): Testing ci: %d, co: %d, di: %d, do: %d\n",
			      __func__, mmc->dev->name, timing.s.cmd_in_tap,
			      timing.s.cmd_out_tap, timing.s.data_in_tap,
			      timing.s.data_out_tap);

			if (slot->host->tap_requires_noclk) {
				/* Turn off the clock */
				emm_debug.s.rdsync_rst = 0;
				write_csr(mmc, MIO_EMM_DEBUG(), emm_debug.u);
				udelay(1);
				emm_debug.u = read_csr(mmc, MIO_EMM_DEBUG());
				emm_debug.s.emmc_clk_disable = 0;
				write_csr(mmc, MIO_EMM_DEBUG(), emm_debug.u);
				udelay(1);
			}
			for (count = 0; count < 2; count++) {
				err = adj->test(mmc, opcode, NULL);
				if (err) {
					debug("%s(%s, %s): tap %d failed, count: %d, rsp_sts: 0x%llx, rsp_lo: 0x%llx\n",
					      __func__, mmc->dev->name,
					      adj->name, tap, count,
					      read_csr(mmc,
						       MIO_EMM_RSP_STS()),
					      read_csr(mmc,
						       MIO_EMM_RSP_LO()));
					debug("%s(%s, %s): tap: %d, do: %d, di: %d, co: %d, ci: %d\n",
					      __func__, mmc->dev->name,
					      adj->name, tap,
					      timing.s.data_out_tap,
					      timing.s.data_in_tap,
					      timing.s.cmd_out_tap,
					      timing.s.cmd_in_tap);
					break;
				}
				debug("%s(%s, %s): tap %d passed, count: %d, rsp_sts: 0x%llx, rsp_lo: 0x%llx\n",
				      __func__, mmc->dev->name, adj->name, tap,
				      count,
				      read_csr(mmc, MIO_EMM_RSP_STS()),
				      read_csr(mmc, MIO_EMM_RSP_LO()));
			}
			tap_status |= (u64)(!err) << tap;
			how[tap] = "-+"[!err];
		} else {
			/*
			 * Putting the end+1 case in the loop simplifies
			 * logic, allowing 'prev_ok' to process a sweet
			 * spot in tuning which extends to the wall.
			 */
			err = -EINVAL;
		}
		if (!err) {
			/*
			 * If no CRC/etc errors in the response, but previous
			 * failed, note the start of a new run.
			 */
			debug("  prev_ok: %d\n", prev_ok);
			if (!prev_ok)
				start_run = tap;
		} else if (prev_ok) {
			run = tap - 1 - start_run;
			/* did we just exit a wider sweet spot? */
			if (start_run >= 0 && run > best_run) {
				best_start = start_run;
				best_run = run;
			}
		}
	}
	how[tap - 1] = '\0';
	if (best_start < 0) {
		printf("%s(%s, %s): %lldMHz tuning %s failed\n", __func__,
		       mmc->dev->name, adj->name, slot->clock / 1000000,
		       adj->name);
		return -EINVAL;
	}

	tap = best_start + best_run / 2;
	debug("  tap %d is center, start: %d, run: %d\n", tap,
	      best_start, best_run);
	if (is_hs200) {
		slot->hs200_taps.u &= ~(0x3full << adj->mask_shift);
		slot->hs200_taps.u |= (u64)tap << adj->mask_shift;
	} else {
		slot->taps.u &= ~(0x3full << adj->mask_shift);
		slot->taps.u |= (u64)tap << adj->mask_shift;
	}
	if (best_start < 0) {
		printf("%s(%s, %s): %lldMHz tuning %s failed\n", __func__,
		       mmc->dev->name, adj->name, slot->clock / 1000000,
		       adj->name);
		return -EINVAL;
	}

	tap = best_start + best_run / 2;
	if (is_hs200 && (tap + tap_adj >= 0) && (tap + tap_adj < 64) &&
	    tap_status & (1ULL << (tap + tap_adj))) {
		debug("Adjusting tap from %d by %d to %d\n",
		      tap, tap_adj, tap + tap_adj);
		tap += tap_adj;
	}
	how[tap] = '@';
	debug("%s/%s %d/%d/%d %s\n", mmc->dev->name,
	      adj->name, best_start, tap, best_start + best_run, how);

	if (is_hs200) {
		slot->hs200_taps.u &= ~(0x3full << adj->mask_shift);
		slot->hs200_taps.u |= (u64)tap << adj->mask_shift;
	} else {
		slot->taps.u &= ~(0x3full << adj->mask_shift);
		slot->taps.u |= (u64)tap << adj->mask_shift;
	}

#ifdef DEBUG
	if (opcode == MMC_CMD_SEND_TUNING_BLOCK_HS200) {
		debug("%s(%s, %s): After successful tuning\n",
		      __func__, mmc->dev->name, adj->name);
		debug("%s(%s, %s): tap: %d, new do: %d, di: %d, co: %d, ci: %d\n",
		      __func__, mmc->dev->name, adj->name, tap,
		      slot->taps.s.data_out_tap,
		      slot->taps.s.data_in_tap,
		      slot->taps.s.cmd_out_tap,
		      slot->taps.s.cmd_in_tap);
		debug("%s(%s, %s): tap: %d, new do HS200: %d, di: %d, co: %d, ci: %d\n",
		      __func__, mmc->dev->name, adj->name, tap,
		      slot->hs200_taps.s.data_out_tap,
		      slot->hs200_taps.s.data_in_tap,
		      slot->hs200_taps.s.cmd_out_tap,
		      slot->hs200_taps.s.cmd_in_tap);
	}
#endif
	octeontx_mmc_set_timing(mmc);

	if (is_hs200 && env_get_yesno("emmc_export_hs200_taps")) {
		char env_name[64];

		env_set_ulong("emmc_timing_tap", slot->host->timing_taps);
		switch (opcode) {
		case MMC_CMD_SEND_TUNING_BLOCK:
			snprintf(env_name, sizeof(env_name),
				 "emmc%d_hs200_data_in_tap_debug",
				 slot->bus_id);
			env_set(env_name, how);
			snprintf(env_name, sizeof(env_name),
				 "emmc%d_hs200_data_in_tap_val", slot->bus_id);
			env_set_ulong(env_name, tap);
			snprintf(env_name, sizeof(env_name),
				 "emmc%d_hs200_data_in_tap_start",
				 slot->bus_id);
			env_set_ulong(env_name, best_start);
			snprintf(env_name, sizeof(env_name),
				 "emmc%d_hs200_data_in_tap_end",
				 slot->bus_id);
			env_set_ulong(env_name, best_start + best_run);
			break;
		case MMC_CMD_SEND_STATUS:
			snprintf(env_name, sizeof(env_name),
				 "emmc%d_hs200_cmd_in_tap_debug",
				 slot->bus_id);
			env_set(env_name, how);
			snprintf(env_name, sizeof(env_name),
				 "emmc%d_hs200_cmd_in_tap_val", slot->bus_id);
			env_set_ulong(env_name, tap);
			snprintf(env_name, sizeof(env_name),
				 "emmc%d_hs200_cmd_in_tap_start",
				 slot->bus_id);
			env_set_ulong(env_name, best_start);
			snprintf(env_name, sizeof(env_name),
				 "emmc%d_hs200_cmd_in_tap_end",
				 slot->bus_id);
			env_set_ulong(env_name, best_start + best_run);
			break;
		default:
			snprintf(env_name, sizeof(env_name),
				 "emmc%d_hs200_data_out_tap", slot->bus_id);
			env_set_ulong(env_name, slot->data_out_hs200_delay);
			snprintf(env_name, sizeof(env_name),
				 "emmc%d_hs200_cmd_out_tap", slot->bus_id);
			env_set_ulong(env_name, slot->cmd_out_hs200_delay);
			break;
		}
	}

	return 0;
}

static int octeontx_mmc_execute_tuning(struct udevice *dev, u32 opcode)
{
	struct mmc *mmc = dev_to_mmc(dev);
	struct octeontx_mmc_slot *slot = mmc_to_slot(mmc);
	union mio_emm_timing emm_timing;
	int err;
	struct adj *a;
	bool is_hs200;
	char env_name[64];

	pr_info("%s re-tuning, opcode 0x%x\n", dev->name, opcode);

	if (slot->is_asim || slot->is_emul)
		return 0;

	is_hs200 = (mmc->selected_mode == MMC_HS_200);
	if (is_hs200) {
		slot->hs200_tuned = false;
		slot->hs400_tuned = false;
	} else {
		slot->tuned = false;
	}
	octeontx_mmc_set_output_bus_timing(mmc);
	octeontx_mmc_set_input_bus_timing(mmc);
	emm_timing.u = read_csr(mmc, MIO_EMM_TIMING());
	if (mmc->selected_mode == MMC_HS_200) {
		slot->hs200_taps.s.cmd_out_tap = emm_timing.s.cmd_out_tap;
		slot->hs200_taps.s.data_out_tap = emm_timing.s.data_out_tap;
	} else {
		slot->taps.s.cmd_out_tap = emm_timing.s.cmd_out_tap;
		slot->taps.s.data_out_tap = emm_timing.s.data_out_tap;
	}
	octeontx_mmc_set_input_bus_timing(mmc);
	octeontx_mmc_set_output_bus_timing(mmc);

	for (a = adj; a->name; a++) {
		ulong in_tap;

		if (!strcmp(a->name, "CMD_IN")) {
			snprintf(env_name, sizeof(env_name),
				 "emmc%d_cmd_in_tap", slot->bus_id);
			in_tap = env_get_ulong(env_name, 10, (ulong)-1);
			if (in_tap != (ulong)-1) {
				if (mmc->selected_mode == MMC_HS_200 ||
				    a->hs200_only) {
					slot->hs200_taps.s.cmd_in_tap = in_tap;
					slot->hs400_taps.s.cmd_in_tap = in_tap;
				} else {
					slot->taps.s.cmd_in_tap = in_tap;
				}
				continue;
			}
		} else if (a->hs200_only &&
			   !strcmp(a->name, "DATA_IN(HS200)")) {
			snprintf(env_name, sizeof(env_name),
				 "emmc%d_data_in_tap_hs200", slot->bus_id);
			in_tap = env_get_ulong(env_name, 10, (ulong)-1);
			if (in_tap != (ulong)-1) {
				debug("%s(%s): Overriding HS200 data in tap to %d\n",
				      __func__, dev->name, (int)in_tap);
				slot->hs200_taps.s.data_in_tap = in_tap;
				continue;
			}
		} else if (!a->hs200_only && !strcmp(a->name, "DATA_IN")) {
			snprintf(env_name, sizeof(env_name),
				 "emmc%d_data_in_tap", slot->bus_id);
			in_tap = env_get_ulong(env_name, 10, (ulong)-1);
			if (in_tap != (ulong)-1) {
				debug("%s(%s): Overriding non-HS200 data in tap to %d\n",
				      __func__, dev->name, (int)in_tap);
				slot->taps.s.data_in_tap = in_tap;
				continue;
			}
		}

		debug("%s(%s): Testing: %s, mode: %s, opcode: %u\n", __func__,
		      dev->name, a->name, mmc_mode_name(mmc->selected_mode),
		      opcode);

		/* Skip DDR only test when not in DDR mode */
		if (a->ddr_only && !mmc->ddr_mode) {
			debug("%s(%s): Skipping %s due to non-DDR mode\n",
			      __func__, dev->name, a->name);
			continue;
		}
		/* Skip hs200 tests in non-hs200 mode and
		 * non-hs200 tests in hs200 mode
		 */
		if (is_hs200) {
			if (a->not_hs200_only) {
				debug("%s(%s): Skipping %s\n", __func__,
				      dev->name, a->name);
				continue;
			}
		} else {
			if (a->hs200_only) {
				debug("%s(%s): Skipping %s\n", __func__,
				      dev->name, a->name);
				continue;
			}
		}

		err = octeontx_mmc_adjust_tuning(mmc, a, a->opcode ?
						 a->opcode : opcode);
		if (err) {
			pr_err("%s(%s, %u): tuning %s failed\n", __func__,
			       dev->name, opcode, a->name);
			return err;
		}
	}

	octeontx_mmc_set_timing(mmc);
	if (is_hs200)
		slot->hs200_tuned = true;
	else
		slot->tuned = true;

	if (slot->hs400_tuning_block != -1) {
		struct mmc_cmd cmd;
		struct mmc_data data;
		u8 buffer[mmc->read_bl_len];

		cmd.cmdidx = MMC_CMD_READ_SINGLE_BLOCK;
		cmd.cmdarg = slot->hs400_tuning_block;
		cmd.resp_type = MMC_RSP_R1;
		data.dest = (void *)buffer;
		data.blocks = 1;
		data.blocksize = mmc->read_bl_len;
		data.flags = MMC_DATA_READ;
		err = octeontx_mmc_read_blocks(mmc, &cmd, &data, true) != 1;

		if (err) {
			printf("%s: Cannot read HS400 tuning block %u\n",
			       dev->name, slot->hs400_tuning_block);
			return err;
		}
		if (memcmp(buffer, octeontx_hs400_tuning_block,
			   sizeof(buffer))) {
			debug("%s(%s): Writing new HS400 tuning block to block %d\n",
			      __func__, dev->name, slot->hs400_tuning_block);
			cmd.cmdidx = MMC_CMD_WRITE_SINGLE_BLOCK;
			data.src = (void *)octeontx_hs400_tuning_block;
			data.flags = MMC_DATA_WRITE;
			err = !octeontx_mmc_write_blocks(mmc, &cmd, &data);
			if (err) {
				printf("%s: Cannot write HS400 tuning block %u\n",
				       dev->name, slot->hs400_tuning_block);
				return -EINVAL;
			}
		}
	}

	return 0;
}
#else /* MMC_SUPPORTS_TUNING */
static void octeontx_mmc_set_emm_timing(struct mmc *mmc,
					union mio_emm_timing emm_timing)
{
}
#endif /* MMC_SUPPORTS_TUNING */

/**
 * Calculate the clock period with rounding up
 *
 * @param	mmc	mmc device
 * Return:	clock period in system clocks for clk_lo + clk_hi
 */
static u32 octeontx_mmc_calc_clk_period(struct mmc *mmc)
{
	struct octeontx_mmc_slot *slot = mmc_to_slot(mmc);
	struct octeontx_mmc_host *host = slot->host;

	if (mmc->clock)
		return DIV_ROUND_UP(host->sys_freq, mmc->clock);

	return 0;
}

static int octeontx_mmc_set_ios(struct udevice *dev)
{
	struct octeontx_mmc_slot *slot = dev_to_mmc_slot(dev);
	struct mmc *mmc = &slot->mmc;
	struct octeontx_mmc_host *host = slot->host;
	union mio_emm_switch emm_switch;
	union mio_emm_modex mode;
	uint clock;
	int bus_width = 0;
	int clk_period = 0;
	int power_class = 10;
	int err = 0;
	bool is_hs200 = false;
	bool is_hs400 = false;

	debug("%s(%s): Entry\n", __func__, dev->name);
	debug("  clock: %u, bus width: %u, mode: %u\n", mmc->clock,
	      mmc->bus_width, mmc->selected_mode);
	debug("  host caps: 0x%x, card caps: 0x%x\n", mmc->host_caps,
	      mmc->card_caps);
	octeontx_mmc_switch_to(mmc);

	clock = mmc->clock;
	if (!clock)
		clock = mmc->cfg->f_min;

	switch (mmc->bus_width) {
	case 8:
		bus_width = 2;
		break;
	case 4:
		bus_width = 1;
		break;
	case 1:
		bus_width = 0;
		break;
	default:
		pr_warn("%s(%s): Invalid bus width %d, defaulting to 1\n",
			__func__, dev->name, mmc->bus_width);
		bus_width = 0;
	}

	/* DDR is available for 4/8 bit bus width */
	if (mmc->ddr_mode && bus_width)
		bus_width |= 4;

	debug("%s: sys_freq: %llu\n", __func__, host->sys_freq);
	clk_period = octeontx_mmc_calc_clk_period(mmc);

	emm_switch.u = 0;
	emm_switch.s.bus_width = bus_width;
	emm_switch.s.power_class = power_class;
	emm_switch.s.clk_hi = clk_period / 2;
	emm_switch.s.clk_lo = clk_period / 2;

	debug("%s: last mode: %d, mode: %d, last clock: %u, clock: %u, ddr: %d\n",
	      __func__, slot->last_mode, mmc->selected_mode,
	      slot->last_clock, mmc->clock, mmc->ddr_mode);
	switch (mmc->selected_mode) {
	case MMC_LEGACY:
		break;
	case MMC_HS:
	case SD_HS:
	case MMC_HS_52:
		emm_switch.s.hs_timing = 1;
		break;
	case MMC_HS_200:
		is_hs200 = true;
		fallthrough;
	case UHS_SDR12:
	case UHS_SDR25:
	case UHS_SDR50:
	case UHS_SDR104:
#if !defined(CONFIG_ARCH_OCTEON)
		emm_switch.s.hs200_timing = 1;
#endif
		break;
	case MMC_HS_400:
		is_hs400 = true;
		fallthrough;
	case UHS_DDR50:
	case MMC_DDR_52:
#if !defined(CONFIG_ARCH_OCTEON)
		emm_switch.s.hs400_timing = 1;
#endif
		break;
	default:
		pr_err("%s(%s): Unsupported mode 0x%x\n", __func__, dev->name,
		       mmc->selected_mode);
		return -1;
	}
	emm_switch.s.bus_id = slot->bus_id;

	if (!is_hs200 && !is_hs400 &&
	    (mmc->selected_mode != slot->last_mode ||
	     mmc->clock != slot->last_clock) &&
	    !mmc->ddr_mode) {
		slot->tuned = false;
		slot->last_mode = mmc->selected_mode;
		slot->last_clock = mmc->clock;
	}

	if (CONFIG_IS_ENABLED(MMC_VERBOSE)) {
		debug("%s(%s): Setting bus mode to %s\n", __func__, dev->name,
		      mmc_mode_name(mmc->selected_mode));
	} else {
		debug("%s(%s): Setting bus mode to 0x%x\n", __func__, dev->name,
		      mmc->selected_mode);
	}

#if !defined(CONFIG_ARCH_OCTEON)
	debug(" Trying switch 0x%llx w%d hs:%d hs200:%d hs400:%d\n",
	      emm_switch.u, emm_switch.s.bus_width, emm_switch.s.hs_timing,
	      emm_switch.s.hs200_timing, emm_switch.s.hs400_timing);
#endif

	set_wdog(mmc, 1000);
	do_switch(mmc, emm_switch);
	mdelay(100);
	mode.u = read_csr(mmc, MIO_EMM_MODEX(slot->bus_id));
#if !defined(CONFIG_ARCH_OCTEON)
	debug("%s(%s): mode: 0x%llx w:%d, hs:%d, hs200:%d, hs400:%d\n",
	      __func__, dev->name, mode.u, mode.s.bus_width,
	      mode.s.hs_timing, mode.s.hs200_timing, mode.s.hs400_timing);
#endif

	err = octeontx_mmc_configure_delay(mmc);

#ifdef MMC_SUPPORTS_TUNING
	if (!err && mmc->selected_mode == MMC_HS_400 && !slot->hs400_tuned) {
		debug("%s: Tuning HS400 mode\n", __func__);
		err = octeontx_tune_hs400(mmc);
	}
#endif

	return err;
}

/**
 * Gets the status of the card detect pin
 */
static int octeontx_mmc_get_cd(struct udevice *dev)
{
	struct octeontx_mmc_slot *slot = dev_to_mmc_slot(dev);
	int val = 1;

	if (dm_gpio_is_valid(&slot->cd_gpio)) {
		val = dm_gpio_get_value(&slot->cd_gpio);
		val ^= slot->cd_inverted;
	}
	debug("%s(%s): cd: %d\n", __func__, dev->name, val);
	return val;
}

/**
 * Gets the status of the write protect pin
 */
static int octeontx_mmc_get_wp(struct udevice *dev)
{
	struct octeontx_mmc_slot *slot = dev_to_mmc_slot(dev);
	int val = 0;

	if (dm_gpio_is_valid(&slot->wp_gpio)) {
		val = dm_gpio_get_value(&slot->wp_gpio);
		val ^= slot->wp_inverted;
	}
	debug("%s(%s): wp: %d\n", __func__, dev->name, val);
	return val;
}

#if defined(CONFIG_ARCH_OCTEON)
static int octeontx_mmc_configure_delay(struct mmc *mmc)
{
	struct octeontx_mmc_slot *slot = mmc_to_slot(mmc);
	union mio_emm_sample emm_sample;

	debug("%s(%s)\n", __func__, mmc->dev->name);

	emm_sample.u = 0;
	emm_sample.s.cmd_cnt = slot->cmd_cnt;
	emm_sample.s.dat_cnt = slot->dat_cnt;
	write_csr(mmc, MIO_EMM_SAMPLE(), emm_sample.u);

	return 0;
}

static void octeontx_mmc_io_drive_setup(struct mmc *mmc)
{
}
#else
static void octeontx_mmc_set_timing(struct mmc *mmc)
{
	union mio_emm_timing timing;
	struct octeontx_mmc_slot *slot = mmc_to_slot(mmc);

	switch (mmc->selected_mode) {
	case MMC_HS_200:
		timing = slot->hs200_taps;
		break;
	case MMC_HS_400:
		timing = slot->hs400_tuned ?
				slot->hs400_taps : slot->hs200_taps;
		break;
	default:
		timing = slot->taps;
		break;
	}

	debug("%s(%s):\n  cmd_in_tap: %u\n  cmd_out_tap: %u\n  data_in_tap: %u\n  data_out_tap: %u\n",
	      __func__, mmc->dev->name, timing.s.cmd_in_tap,
	      timing.s.cmd_out_tap, timing.s.data_in_tap,
	      timing.s.data_out_tap);

	octeontx_mmc_set_emm_timing(mmc, timing);
}

static int octeontx_mmc_configure_delay(struct mmc *mmc)
{
	struct octeontx_mmc_slot *slot = mmc_to_slot(mmc);
	struct octeontx_mmc_host *host __maybe_unused = slot->host;
	bool __maybe_unused is_hs200;
	bool __maybe_unused is_hs400;

	debug("%s(%s)\n", __func__, mmc->dev->name);

	if (IS_ENABLED(CONFIG_ARCH_OCTEON) ||
	    IS_ENABLED(CONFIG_ARCH_OCTEONTX)) {
		union mio_emm_sample emm_sample;

		emm_sample.u = 0;
		emm_sample.s.cmd_cnt = slot->cmd_cnt;
		emm_sample.s.dat_cnt = slot->dat_cnt;
		write_csr(mmc, MIO_EMM_SAMPLE(), emm_sample.u);
	} else {
		is_hs200 = (mmc->selected_mode == MMC_HS_200);
		is_hs400 = (mmc->selected_mode == MMC_HS_400);

		if ((is_hs200 && slot->hs200_tuned) ||
		    (is_hs400 && slot->hs400_tuned) ||
		    (!is_hs200 && !is_hs400 && slot->tuned)) {
			octeontx_mmc_set_output_bus_timing(mmc);
		} else {
			int half = MAX_NO_OF_TAPS / 2;
			int dout, cout;

			switch (mmc->selected_mode) {
			case MMC_LEGACY:
				if (IS_SD(mmc)) {
					cout = MMC_SD_LEGACY_DEFAULT_CMD_OUT_TAP;
					dout = MMC_SD_LEGACY_DEFAULT_DATA_OUT_TAP;
				} else {
					cout = MMC_LEGACY_DEFAULT_CMD_OUT_TAP;
					dout = MMC_LEGACY_DEFAULT_DATA_OUT_TAP;
				}
				break;
			case MMC_HS:
				cout = MMC_HS_CMD_OUT_TAP;
				dout = MMC_HS_DATA_OUT_TAP;
				break;
			case SD_HS:
			case UHS_SDR12:
			case UHS_SDR25:
			case UHS_SDR50:
				cout = MMC_SD_HS_CMD_OUT_TAP;
				dout = MMC_SD_HS_DATA_OUT_TAP;
				break;
			case UHS_SDR104:
			case UHS_DDR50:
			case MMC_HS_52:
			case MMC_DDR_52:
				cout = MMC_DEFAULT_CMD_OUT_TAP;
				dout = MMC_DEFAULT_DATA_OUT_TAP;
				break;
			case MMC_HS_200:
				cout = -1;
				dout = -1;
				if (host->timing_calibrated) {
					cout = octeontx2_mmc_calc_delay(
						mmc, slot->cmd_out_hs200_delay);
					dout = octeontx2_mmc_calc_delay(
						mmc,
						slot->data_out_hs200_delay);
					debug("%s(%s): Calibrated HS200/HS400 cmd out delay: %dps tap: %d, data out delay: %d, tap: %d\n",
					      __func__, mmc->dev->name,
					      slot->cmd_out_hs200_delay, cout,
					      slot->data_out_hs200_delay, dout);
				} else {
					cout = MMC_DEFAULT_HS200_CMD_OUT_TAP;
					dout = MMC_DEFAULT_HS200_DATA_OUT_TAP;
				}
				is_hs200 = true;
				break;
			case MMC_HS_400:
				cout = -1;
				dout = -1;
				if (host->timing_calibrated) {
					if (slot->cmd_out_hs400_delay)
						cout = octeontx2_mmc_calc_delay(
							mmc,
							slot->cmd_out_hs400_delay);
					if (slot->data_out_hs400_delay)
						dout = octeontx2_mmc_calc_delay(
							mmc,
							slot->data_out_hs400_delay);
					debug("%s(%s): Calibrated HS200/HS400 cmd out delay: %dps tap: %d, data out delay: %d, tap: %d\n",
					      __func__, mmc->dev->name,
					      slot->cmd_out_hs400_delay, cout,
					      slot->data_out_hs400_delay, dout);
				} else {
					cout = MMC_DEFAULT_HS400_CMD_OUT_TAP;
					dout = MMC_DEFAULT_HS400_DATA_OUT_TAP;
				}
				is_hs400 = true;
				break;
			default:
				pr_err("%s(%s): Invalid mode %d\n", __func__,
				       mmc->dev->name, mmc->selected_mode);
				return -1;
			}
			debug("%s(%s): Not tuned, hs200: %d, hs200 tuned: %d, hs400: %d, hs400 tuned: %d, tuned: %d\n",
			      __func__, mmc->dev->name, is_hs200,
			      slot->hs200_tuned,
			      is_hs400, slot->hs400_tuned, slot->tuned);
			/* Set some defaults */
			if (is_hs200) {
				slot->hs200_taps.u = 0;
				slot->hs200_taps.s.cmd_out_tap = cout;
				slot->hs200_taps.s.data_out_tap = dout;
				slot->hs200_taps.s.cmd_in_tap = half;
				slot->hs200_taps.s.data_in_tap = half;
			} else if (is_hs400) {
				slot->hs400_taps.u = 0;
				slot->hs400_taps.s.cmd_out_tap = cout;
				slot->hs400_taps.s.data_out_tap = dout;
				slot->hs400_taps.s.cmd_in_tap = half;
				slot->hs400_taps.s.data_in_tap = half;
			} else {
				slot->taps.u = 0;
				slot->taps.s.cmd_out_tap = cout;
				slot->taps.s.data_out_tap = dout;
				slot->taps.s.cmd_in_tap = half;
				slot->taps.s.data_in_tap = half;
			}
		}

		if (is_hs200)
			debug("%s(%s): hs200 taps: ci: %u, co: %u, di: %u, do: %u\n",
			      __func__, mmc->dev->name,
			      slot->hs200_taps.s.cmd_in_tap,
			      slot->hs200_taps.s.cmd_out_tap,
			      slot->hs200_taps.s.data_in_tap,
			      slot->hs200_taps.s.data_out_tap);
		else if (is_hs400)
			debug("%s(%s): hs400 taps: ci: %u, co: %u, di: %u, do: %u\n",
			      __func__, mmc->dev->name,
			      slot->hs400_taps.s.cmd_in_tap,
			      slot->hs400_taps.s.cmd_out_tap,
			      slot->hs400_taps.s.data_in_tap,
			      slot->hs400_taps.s.data_out_tap);
		else
			debug("%s(%s): taps: ci: %u, co: %u, di: %u, do: %u\n",
			      __func__, mmc->dev->name, slot->taps.s.cmd_in_tap,
			      slot->taps.s.cmd_out_tap,
			      slot->taps.s.data_in_tap,
			      slot->taps.s.data_out_tap);
		octeontx_mmc_set_timing(mmc);
		debug("%s: Done\n", __func__);
	}

	return 0;
}

/**
 * Set the IO drive strength and slew
 *
 * @param	mmc	mmc device
 */
static void octeontx_mmc_io_drive_setup(struct mmc *mmc)
{
	if (IS_ENABLED(CONFIG_ARCH_OCTEONTX2)) {
		struct octeontx_mmc_slot *slot = mmc_to_slot(mmc);
		union mio_emm_io_ctl io_ctl;

		if (slot->drive < 0 || slot->slew < 0)
			return;

		io_ctl.u = 0;
		io_ctl.s.drive = slot->drive;
		io_ctl.s.slew = slot->slew;
		write_csr(mmc, MIO_EMM_IO_CTL(), io_ctl.u);
	}
}
#endif

/**
 * Sets the MMC watchdog timer in microseconds
 *
 * @param	mmc	mmc device
 * @param	us	timeout in microseconds, 0 for maximum timeout
 */
static void set_wdog(struct mmc *mmc, u64 us)
{
	union mio_emm_wdog wdog;
	u64 val;

	val = (us * mmc->clock) / 1000000;
	if (val >= (1 << 26) || !us) {
		if (us)
			pr_debug("%s: warning: timeout %llu exceeds max value %llu, truncating\n",
				 __func__, us,
				 (u64)(((1ULL << 26) - 1) * 1000000ULL) /
					mmc->clock);
		val = (1 << 26) - 1;
	}
	wdog.u = 0;
	wdog.s.clk_cnt = val;
	write_csr(mmc, MIO_EMM_WDOG(), wdog.u);
}

/**
 * Print switch errors
 *
 * @param	mmc	mmc device
 */
static void check_switch_errors(struct mmc *mmc)
{
	union mio_emm_switch emm_switch;

	emm_switch.u = read_csr(mmc, MIO_EMM_SWITCH());
	if (emm_switch.s.switch_err0)
		pr_err("%s: Switch power class error\n", mmc->cfg->name);
	if (emm_switch.s.switch_err1)
		pr_err("%s: Switch HS timing error\n", mmc->cfg->name);
	if (emm_switch.s.switch_err2)
		pr_err("%s: Switch bus width error\n", mmc->cfg->name);
}

static void do_switch(struct mmc *mmc, union mio_emm_switch emm_switch)
{
	union mio_emm_rsp_sts rsp_sts;
	struct octeontx_mmc_slot *slot = mmc_to_slot(mmc);
	int bus_id = emm_switch.s.bus_id;
	ulong start;

	if (emm_switch.s.bus_id != 0) {
		emm_switch.s.bus_id = 0;
		write_csr(mmc, MIO_EMM_SWITCH(), emm_switch.u);
		udelay(100);
		emm_switch.s.bus_id = bus_id;
	}
	debug("%s(%s, 0x%llx)\n", __func__, mmc->dev->name, emm_switch.u);
	write_csr(mmc, MIO_EMM_SWITCH(), emm_switch.u);

	start = get_timer(0);
	do {
		rsp_sts.u = read_csr(mmc, MIO_EMM_RSP_STS());
		if (!rsp_sts.s.switch_val)
			break;
		udelay(100);
	} while (get_timer(start) < 10);
	if (rsp_sts.s.switch_val) {
		pr_warn("%s(%s): Warning: writing 0x%llx to emm_switch timed out, status: 0x%llx\n",
			__func__, mmc->dev->name, emm_switch.u, rsp_sts.u);
	}
	slot->cached_switch = emm_switch;
	check_switch_errors(mmc);
	slot->cached_switch.u = emm_switch.u;
	debug("%s: emm_switch: 0x%llx, rsp_lo: 0x%llx\n",
	      __func__, read_csr(mmc, MIO_EMM_SWITCH()),
				 read_csr(mmc, MIO_EMM_RSP_LO()));
}

/**
 * Calibrates the delay based on the internal clock
 *
 * @param	mmc	Pointer to mmc data structure
 *
 * Return:	0 for success or -ETIMEDOUT on error
 *
 * NOTE: On error a default value will be calculated.
 */
#if defined(CONFIG_ARCH_OCTEON)
static int octeontx_mmc_set_input_bus_timing(struct mmc *mmc)
{
	return 0;
}

static int octeontx_mmc_set_output_bus_timing(struct mmc *mmc)
{
	return 0;
}

static int octeontx_mmc_calibrate_delay(struct mmc *mmc)
{
	return 0;
}
#else
/**
 * Given a delay in ps, return the tap delay count
 *
 * @param	mmc	mmc data structure
 * @param	delay	delay in picoseconds
 *
 * Return:	Number of tap cycles or error if -1
 */
static int octeontx2_mmc_calc_delay(struct mmc *mmc, int delay)
{
	struct octeontx_mmc_host *host = mmc_to_host(mmc);

	if (host->is_asim || host->is_emul)
		return 63;

	if (!host->timing_taps) {
		pr_err("%s(%s): Error: host timing not calibrated\n",
		       __func__, mmc->dev->name);
		return -1;
	}
	debug("%s(%s, %d) timing taps: %llu\n", __func__, mmc->dev->name,
	      delay, host->timing_taps);
	return min_t(int, DIV_ROUND_UP(delay, host->timing_taps), 63);
}

static int octeontx_mmc_calibrate_delay(struct mmc *mmc)
{
	union mio_emm_calb emm_calb;
	union mio_emm_tap emm_tap;
	union mio_emm_cfg emm_cfg;
	union mio_emm_io_ctl emm_io_ctl;
	union mio_emm_switch emm_switch;
	union mio_emm_wdog emm_wdog;
	union mio_emm_sts_mask emm_sts_mask;
	union mio_emm_debug emm_debug;
	union mio_emm_timing emm_timing;
	struct octeontx_mmc_host *host = mmc_to_host(mmc);
	ulong start;
	u8 bus_id, bus_ena;

	debug("%s: Calibrating delay\n", __func__);
	if (host->is_asim || host->is_emul) {
		debug("  No calibration for ASIM\n");
		return 0;
	}
	emm_tap.u = 0;
	if (host->calibrate_glitch) {
		emm_tap.s.delay = MMC_DEFAULT_TAP_DELAY;
	} else {
		/* Save registers */
		emm_cfg.u = read_csr(mmc, MIO_EMM_CFG());
		emm_io_ctl.u = read_csr(mmc, MIO_EMM_IO_CTL());
		emm_switch.u = read_csr(mmc, MIO_EMM_SWITCH());
		emm_wdog.u = read_csr(mmc, MIO_EMM_WDOG());
		emm_sts_mask.u = read_csr(mmc, MIO_EMM_STS_MASK());
		emm_debug.u = read_csr(mmc, MIO_EMM_DEBUG());
		emm_timing.u = read_csr(mmc, MIO_EMM_TIMING());
		bus_ena = emm_cfg.s.bus_ena;
		bus_id = emm_switch.s.bus_id;
		emm_cfg.s.bus_ena = 0;
		write_csr(mmc, MIO_EMM_CFG(), emm_cfg.u);
		udelay(1);
		emm_cfg.s.bus_ena = 1ULL << 3;
		write_csr(mmc, MIO_EMM_CFG(), emm_cfg.u);
		mdelay(1);
		emm_calb.u = 0;
		write_csr(mmc, MIO_EMM_CALB(), emm_calb.u);
		emm_calb.s.start = 1;
		write_csr(mmc, MIO_EMM_CALB(), emm_calb.u);
		start = get_timer(0);
		/* This should only take 3 microseconds */
		do {
			udelay(5);
			emm_tap.u = read_csr(mmc, MIO_EMM_TAP());
		} while (!emm_tap.s.delay && get_timer(start) < 10);

		emm_calb.s.start = 0;
		write_csr(mmc, MIO_EMM_CALB(), emm_calb.u);

		emm_cfg.s.bus_ena = 0;
		write_csr(mmc, MIO_EMM_CFG(), emm_cfg.u);
		udelay(1);
		/* Restore registers */
		emm_cfg.s.bus_ena = bus_ena;
		write_csr(mmc, MIO_EMM_CFG(), emm_cfg.u);
		if (host->tap_requires_noclk) {
			/* Turn off the clock */
			emm_debug.u = read_csr(mmc, MIO_EMM_DEBUG());
			emm_debug.s.emmc_clk_disable = 1;
			write_csr(mmc, MIO_EMM_DEBUG(), emm_debug.u);
			udelay(1);
			emm_debug.s.rdsync_rst = 1;
			write_csr(mmc, MIO_EMM_DEBUG(), emm_debug.u);
			udelay(1);
		}

		write_csr(mmc, MIO_EMM_TIMING(), emm_timing.u);
		if (host->tap_requires_noclk) {
			/* Turn the clock back on */
			udelay(1);
			emm_debug.s.rdsync_rst = 0;
			write_csr(mmc, MIO_EMM_DEBUG(), emm_debug.u);
			udelay(1);
			emm_debug.s.emmc_clk_disable = 0;
			write_csr(mmc, MIO_EMM_DEBUG(), emm_debug.u);
		}
		udelay(1);
		write_csr(mmc, MIO_EMM_IO_CTL(), emm_io_ctl.u);
		bus_id = emm_switch.s.bus_id;
		emm_switch.s.bus_id = 0;
		write_csr(mmc, MIO_EMM_SWITCH(), emm_switch.u);
		emm_switch.s.bus_id = bus_id;
		write_csr(mmc, MIO_EMM_SWITCH(), emm_switch.u);
		write_csr(mmc, MIO_EMM_WDOG(), emm_wdog.u);
		write_csr(mmc, MIO_EMM_STS_MASK(), emm_sts_mask.u);
		write_csr(mmc, MIO_EMM_RCA(), mmc->rca);
		write_csr(mmc, MIO_EMM_DEBUG(), emm_debug.u);

		if (!emm_tap.s.delay) {
			pr_err("%s: Error: delay calibration failed, timed out.\n",
			       __func__);
			/* Set to default value if timed out */
			emm_tap.s.delay = MMC_DEFAULT_TAP_DELAY;
			return -ETIMEDOUT;
		}
	}
	/* Round up */
	host->timing_taps = (10 * 1000 * emm_tap.s.delay) / TOTAL_NO_OF_TAPS;
	debug("%s(%s): timing taps: %llu, delay: %u\n",
	      __func__, mmc->dev->name, host->timing_taps, emm_tap.s.delay);
	host->timing_calibrated = true;
	return 0;
}

static int octeontx_mmc_set_input_bus_timing(struct mmc *mmc)
{
	struct octeontx_mmc_slot *slot = mmc_to_slot(mmc);

	if (IS_ENABLED(CONFIG_ARCH_OCTEONTX)) {
		union mio_emm_sample sample;

		sample.u = 0;
		sample.s.cmd_cnt = slot->cmd_clk_skew;
		sample.s.dat_cnt = slot->dat_clk_skew;
		write_csr(mmc, MIO_EMM_SAMPLE(), sample.u);
	} else {
		union mio_emm_timing timing;

		timing.u = read_csr(mmc, MIO_EMM_TIMING());
		if (mmc->selected_mode == MMC_HS_200) {
			if (slot->hs200_tuned) {
				timing.s.cmd_in_tap =
					slot->hs200_taps.s.cmd_in_tap;
				timing.s.data_in_tap =
					slot->hs200_taps.s.data_in_tap;
			} else {
				pr_warn("%s(%s): Warning: hs200 timing not tuned\n",
					__func__, mmc->dev->name);
				timing.s.cmd_in_tap =
					MMC_DEFAULT_HS200_CMD_IN_TAP;
				timing.s.data_in_tap =
					MMC_DEFAULT_HS200_DATA_IN_TAP;
			}
		} else if (mmc->selected_mode == MMC_HS_400) {
			if (slot->hs400_tuned) {
				timing.s.cmd_in_tap =
					slot->hs400_taps.s.cmd_in_tap;
				timing.s.data_in_tap =
					slot->hs400_taps.s.data_in_tap;
			} else if (slot->hs200_tuned) {
				timing.s.cmd_in_tap =
					slot->hs200_taps.s.cmd_in_tap;
				timing.s.data_in_tap =
					slot->hs200_taps.s.data_in_tap;
			} else {
				pr_warn("%s(%s): Warning: hs400 timing not tuned\n",
					__func__, mmc->dev->name);
				timing.s.cmd_in_tap =
					MMC_DEFAULT_HS200_CMD_IN_TAP;
				timing.s.data_in_tap =
					MMC_DEFAULT_HS200_DATA_IN_TAP;
			}
		} else if (slot->tuned) {
			timing.s.cmd_in_tap = slot->taps.s.cmd_in_tap;
			timing.s.data_in_tap = slot->taps.s.data_in_tap;
		} else {
			timing.s.cmd_in_tap = MMC_DEFAULT_CMD_IN_TAP;
			timing.s.data_in_tap = MMC_DEFAULT_DATA_IN_TAP;
		}
		octeontx_mmc_set_emm_timing(mmc, timing);
	}

	return 0;
}

/**
 * Sets the default bus timing for the current mode.
 *
 * @param	mmc	mmc data structure
 *
 * Return:	0 for success, error otherwise
 */
static int octeontx_mmc_set_output_bus_timing(struct mmc *mmc)
{
	struct octeontx_mmc_slot *slot = mmc_to_slot(mmc);
	union mio_emm_timing timing;
	int cout_bdelay, dout_bdelay;
	unsigned int cout_delay, dout_delay;
	char env_name[32];

	if (IS_ENABLED(CONFIG_ARCH_OCTEONTX))
		return 0;

	debug("%s(%s)\n", __func__, mmc->dev->name);
	if (slot->is_asim || slot->is_emul)
		return 0;

	octeontx_mmc_calibrate_delay(mmc);

	if (mmc->clock < 26000000) {
		cout_delay = 5000;
		dout_delay = 5000;
	} else if (mmc->clock <= 52000000) {
		cout_delay = 2500;
		dout_delay = 2500;
	} else if (!mmc_is_mode_ddr(mmc->selected_mode)) {
		cout_delay = slot->cmd_out_hs200_delay;
		dout_delay = slot->data_out_hs200_delay;
	} else {
		cout_delay = slot->cmd_out_hs400_delay;
		dout_delay = slot->data_out_hs400_delay;
	}

	snprintf(env_name, sizeof(env_name), "mmc%d_hs200_dout_delay_ps",
		 slot->bus_id);
	dout_delay = env_get_ulong(env_name, 10, dout_delay);
	debug("%s: dout_delay: %u\n", __func__, dout_delay);

	cout_bdelay = octeontx2_mmc_calc_delay(mmc, cout_delay);
	dout_bdelay = octeontx2_mmc_calc_delay(mmc, dout_delay);

	debug("%s: cmd output delay: %u, data output delay: %u, cmd bdelay: %d, data bdelay: %d, clock: %d\n",
	      __func__, cout_delay, dout_delay, cout_bdelay, dout_bdelay,
	      mmc->clock);
	if (cout_bdelay < 0 || dout_bdelay < 0) {
		pr_err("%s: Error: could not calculate command and/or data clock skew\n",
		       __func__);
		return -1;
	}
	timing.u = read_csr(mmc, MIO_EMM_TIMING());
	timing.s.cmd_out_tap = cout_bdelay;
	timing.s.data_out_tap = dout_bdelay;
	if (mmc->selected_mode == MMC_HS_200) {
		slot->hs200_taps.s.cmd_out_tap = cout_bdelay;
		slot->hs200_taps.s.data_out_tap = dout_bdelay;
	} else if (mmc->selected_mode == MMC_HS_400) {
		slot->hs400_taps.s.cmd_out_tap = cout_bdelay;
		slot->hs400_taps.s.data_out_tap = dout_bdelay;
	} else {
		slot->taps.s.cmd_out_tap = cout_bdelay;
		slot->taps.s.data_out_tap = dout_bdelay;
	}
	octeontx_mmc_set_emm_timing(mmc, timing);
	debug("%s(%s): bdelay: %d/%d, clock: %d, ddr: %s, timing taps: %llu, do: %d, di: %d, co: %d, ci: %d\n",
	      __func__, mmc->dev->name, cout_bdelay, dout_bdelay, mmc->clock,
	      mmc->ddr_mode ? "yes" : "no",
	      mmc_to_host(mmc)->timing_taps,
	      timing.s.data_out_tap,
	      timing.s.data_in_tap,
	      timing.s.cmd_out_tap,
	      timing.s.cmd_in_tap);

	return 0;
}
#endif

static void octeontx_mmc_set_clock(struct mmc *mmc)
{
	struct octeontx_mmc_slot *slot = mmc_to_slot(mmc);
	uint clock;

	clock = min(mmc->cfg->f_max, (uint)slot->clock);
	clock = max(mmc->cfg->f_min, clock);
	debug("%s(%s): f_min: %u, f_max: %u, clock: %u\n", __func__,
	      mmc->dev->name, mmc->cfg->f_min, mmc->cfg->f_max, clock);
	slot->clock = clock;
	mmc->clock = clock;
}

/**
 * This switches I/O power as needed when switching between slots.
 *
 * @param	mmc	mmc data structure
 */
static void octeontx_mmc_switch_io(struct mmc *mmc)
{
	struct octeontx_mmc_slot *slot = mmc_to_slot(mmc);
	struct octeontx_mmc_host *host = slot->host;
	struct mmc *last_mmc = host->last_mmc;
	static struct udevice *last_reg;
	union mio_emm_cfg emm_cfg;
	int bus;
	static bool initialized;

	/* First time? */
	if (!initialized || mmc != host->last_mmc) {
		struct mmc *ommc;

		/* Switch to bus 3 which is unused */
		emm_cfg.u = read_csr(mmc, MIO_EMM_CFG());
		emm_cfg.s.bus_ena = 1 << 3;
		write_csr(mmc, MIO_EMM_CFG(), emm_cfg.u);

		/* Turn off all other I/O interfaces with first initialization
		 * if at least one supply was found.
		 */
		for (bus = 0; bus <= OCTEONTX_MAX_MMC_SLOT; bus++) {
			ommc = &host->slots[bus].mmc;

			/* Handle self case later */
			if (ommc == mmc || !ommc->vqmmc_supply)
				continue;

			/* Skip if we're not switching regulators */
			if (last_reg == mmc->vqmmc_supply)
				continue;

			/* Turn off other regulators */
			if (ommc->vqmmc_supply != mmc->vqmmc_supply)
				regulator_set_enable(ommc->vqmmc_supply, false);
		}
		/* Turn ourself on */
		if (mmc->vqmmc_supply && last_reg != mmc->vqmmc_supply)
			regulator_set_enable(mmc->vqmmc_supply, true);
		mdelay(1);	/* Settle time */
		/* Switch to new bus */
		emm_cfg.s.bus_ena = 1 << slot->bus_id;
		write_csr(mmc, MIO_EMM_CFG(), emm_cfg.u);
		last_reg = mmc->vqmmc_supply;
		initialized = true;
		return;
	}

	/* No change in device */
	if (last_mmc == mmc)
		return;

	if (!last_mmc) {
		pr_warn("%s(%s): No previous slot detected in IO slot switch!\n",
			__func__, mmc->dev->name);
		return;
	}

	debug("%s(%s): last: %s, supply: %p\n", __func__, mmc->dev->name,
	      last_mmc->dev->name, mmc->vqmmc_supply);

	/* The supply is the same so we do nothing */
	if (last_mmc->vqmmc_supply == mmc->vqmmc_supply)
		return;

	/* Turn off the old slot I/O supply */
	if (last_mmc->vqmmc_supply) {
		debug("%s(%s): Turning off IO to %s, supply: %s\n",
		      __func__, mmc->dev->name, last_mmc->dev->name,
		      last_mmc->vqmmc_supply->name);
		regulator_set_enable(last_mmc->vqmmc_supply, false);
	}
	/* Turn on the new slot I/O supply */
	if (mmc->vqmmc_supply)  {
		debug("%s(%s): Turning on IO to slot %d, supply: %s\n",
		      __func__, mmc->dev->name, slot->bus_id,
		      mmc->vqmmc_supply->name);
		regulator_set_enable(mmc->vqmmc_supply, true);
	}
	/* Allow power to settle */
	mdelay(1);
}

/**
 * Called to switch between mmc devices
 *
 * @param	mmc	new mmc device
 */
static void octeontx_mmc_switch_to(struct mmc *mmc)
{
	struct octeontx_mmc_slot *slot = mmc_to_slot(mmc);
	struct octeontx_mmc_slot *old_slot;
	struct octeontx_mmc_host *host = slot->host;
	union mio_emm_switch emm_switch;
	union mio_emm_sts_mask emm_sts_mask;
	union mio_emm_rca emm_rca;

	if (slot->bus_id == host->last_slotid)
		return;

	debug("%s(%s) switching from slot %d to slot %d\n", __func__,
	      mmc->dev->name, host->last_slotid, slot->bus_id);
	octeontx_mmc_switch_io(mmc);

	if (host->last_slotid >= 0 && slot->valid) {
		old_slot = &host->slots[host->last_slotid];
		old_slot->cached_switch.u = read_csr(mmc, MIO_EMM_SWITCH());
		old_slot->cached_rca.u = read_csr(mmc, MIO_EMM_RCA());
	}
	if (mmc->rca)
		write_csr(mmc, MIO_EMM_RCA(), mmc->rca);
	emm_switch = slot->cached_switch;
	do_switch(mmc, emm_switch);
	emm_rca.u = 0;
	emm_rca.s.card_rca = mmc->rca;
	write_csr(mmc, MIO_EMM_RCA(), emm_rca.u);
	mdelay(100);

	set_wdog(mmc, 100000);
	if (octeontx_mmc_set_output_bus_timing(mmc) ||
	    octeontx_mmc_set_input_bus_timing(mmc))
		pr_err("%s(%s): Error setting bus timing\n", __func__,
		       mmc->dev->name);
	octeontx_mmc_io_drive_setup(mmc);

	emm_sts_mask.u = 0;
	emm_sts_mask.s.sts_msk = 1 << 7 | 1 << 22 | 1 << 23 | 1 << 19;
	write_csr(mmc, MIO_EMM_STS_MASK(), emm_sts_mask.u);
	host->last_slotid = slot->bus_id;
	host->last_mmc = mmc;
	mdelay(10);
}

/**
 * Perform initial timing configuration
 *
 * @param mmc	mmc device
 *
 * Return: 0 for success
 *
 * NOTE: This will need to be updated when new silicon comes out
 */
static int octeontx_mmc_init_timing(struct mmc *mmc)
{
	union mio_emm_timing timing;

	if (mmc_to_slot(mmc)->is_asim || mmc_to_slot(mmc)->is_emul)
		return 0;

	debug("%s(%s)\n", __func__, mmc->dev->name);
	timing.u = 0;
	timing.s.cmd_out_tap = MMC_DEFAULT_CMD_OUT_TAP;
	timing.s.data_out_tap = MMC_DEFAULT_DATA_OUT_TAP;
	timing.s.cmd_in_tap = MMC_DEFAULT_CMD_IN_TAP;
	timing.s.data_in_tap = MMC_DEFAULT_DATA_IN_TAP;
	octeontx_mmc_set_emm_timing(mmc, timing);
	return 0;
}

/**
 * Perform low-level initialization
 *
 * @param	mmc	mmc device
 *
 * Return:	0 for success, error otherwise
 */
static int octeontx_mmc_init_lowlevel(struct mmc *mmc)
{
	struct octeontx_mmc_slot *slot = mmc_to_slot(mmc);
	struct octeontx_mmc_host *host = slot->host;
	union mio_emm_switch emm_switch;
	u32 clk_period;

	debug("%s(%s): lowlevel init for slot %d\n", __func__,
	      mmc->dev->name, slot->bus_id);
	host->emm_cfg.s.bus_ena &= ~(1 << slot->bus_id);
	write_csr(mmc, MIO_EMM_CFG(), host->emm_cfg.u);
	udelay(100);
	host->emm_cfg.s.bus_ena |= 1 << slot->bus_id;
	write_csr(mmc, MIO_EMM_CFG(), host->emm_cfg.u);
	udelay(10);
	slot->clock = mmc->cfg->f_min;
	octeontx_mmc_set_clock(&slot->mmc);

	if (IS_ENABLED(CONFIG_ARCH_OCTEONTX2)) {
		if (host->cond_clock_glitch) {
			union mio_emm_debug emm_debug;

			emm_debug.u = read_csr(mmc, MIO_EMM_DEBUG());
			emm_debug.s.clk_on = 1;
			write_csr(mmc, MIO_EMM_DEBUG(), emm_debug.u);
		}
		octeontx_mmc_calibrate_delay(&slot->mmc);
	}

	clk_period = octeontx_mmc_calc_clk_period(mmc);
	emm_switch.u = 0;
	emm_switch.s.power_class = 10;
	emm_switch.s.clk_lo = clk_period / 2;
	emm_switch.s.clk_hi = clk_period / 2;

	emm_switch.s.bus_id = slot->bus_id;
	debug("%s: Performing switch\n", __func__);
	do_switch(mmc, emm_switch);
	slot->cached_switch.u = emm_switch.u;

	if (!IS_ENABLED(CONFIG_ARCH_OCTEONTX))
		octeontx_mmc_init_timing(mmc);

	set_wdog(mmc, 1000000); /* Set to 1 second */
	write_csr(mmc, MIO_EMM_STS_MASK(), 0xe4390080ull);
	write_csr(mmc, MIO_EMM_RCA(), 1);
	mdelay(10);
	debug("%s: done\n", __func__);
	return 0;
}

/**
 * Translates a voltage number to bits in MMC register
 *
 * @param	voltage	voltage in microvolts
 *
 * Return:	MMC register value for voltage
 */
static u32 xlate_voltage(u32 voltage)
{
	u32 volt = 0;

	/* Convert to millivolts. Only necessary on ARM Octeon TX/TX2 */
	if (!IS_ENABLED(CONFIG_ARCH_OCTEON))
		voltage /= 1000;

	if (voltage >= 1650 && voltage <= 1950)
		volt |= MMC_VDD_165_195;
	if (voltage >= 2000 && voltage <= 2100)
		volt |= MMC_VDD_20_21;
	if (voltage >= 2100 && voltage <= 2200)
		volt |= MMC_VDD_21_22;
	if (voltage >= 2200 && voltage <= 2300)
		volt |= MMC_VDD_22_23;
	if (voltage >= 2300 && voltage <= 2400)
		volt |= MMC_VDD_23_24;
	if (voltage >= 2400 && voltage <= 2500)
		volt |= MMC_VDD_24_25;
	if (voltage >= 2500 && voltage <= 2600)
		volt |= MMC_VDD_25_26;
	if (voltage >= 2600 && voltage <= 2700)
		volt |= MMC_VDD_26_27;
	if (voltage >= 2700 && voltage <= 2800)
		volt |= MMC_VDD_27_28;
	if (voltage >= 2800 && voltage <= 2900)
		volt |= MMC_VDD_28_29;
	if (voltage >= 2900 && voltage <= 3000)
		volt |= MMC_VDD_29_30;
	if (voltage >= 3000 && voltage <= 3100)
		volt |= MMC_VDD_30_31;
	if (voltage >= 3100 && voltage <= 3200)
		volt |= MMC_VDD_31_32;
	if (voltage >= 3200 && voltage <= 3300)
		volt |= MMC_VDD_32_33;
	if (voltage >= 3300 && voltage <= 3400)
		volt |= MMC_VDD_33_34;
	if (voltage >= 3400 && voltage <= 3500)
		volt |= MMC_VDD_34_35;
	if (voltage >= 3500 && voltage <= 3600)
		volt |= MMC_VDD_35_36;

	return volt;
}

/**
 * Check if a slot is valid in the device tree
 *
 * @param	dev	slot device to check
 *
 * Return:	true if status reports "ok" or "okay" or if no status,
 *		false otherwise.
 */
static bool octeontx_mmc_get_valid(struct udevice *dev)
{
	const char *stat = ofnode_read_string(dev_ofnode(dev), "status");

	if (!stat || !strncmp(stat, "ok", 2))
		return true;
	else
		return false;
}

/**
 * Reads slot configuration from the device tree
 *
 * @param	dev	slot device
 *
 * Return:	0 on success, otherwise error
 */
static int octeontx_mmc_get_config(struct udevice *dev)
{
	struct octeontx_mmc_slot *slot = dev_to_mmc_slot(dev);
	uint voltages[2];
	uint low, high;
	char env_name[32];
	int err;
	ofnode node = dev_ofnode(dev);
	int bus_width = 1;
	ulong new_max_freq;

	debug("%s(%s)", __func__, dev->name);
	slot->cfg.name = dev->name;

	slot->cfg.f_max = ofnode_read_s32_default(dev_ofnode(dev),
						  "max-frequency",
						  26000000);
	snprintf(env_name, sizeof(env_name), "mmc_max_frequency%d",
		 slot->bus_id);

	new_max_freq = env_get_ulong(env_name, 10, slot->cfg.f_max);
	debug("Reading %s, got %lu\n", env_name, new_max_freq);

	if (new_max_freq != slot->cfg.f_max) {
		printf("Overriding device tree MMC maximum frequency %u to %lu\n",
		       slot->cfg.f_max, new_max_freq);
		slot->cfg.f_max = new_max_freq;
	}
	slot->cfg.f_min = 400000;
	slot->cfg.b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;

	if (IS_ENABLED(CONFIG_ARCH_OCTEONTX2)) {
		slot->hs400_tuning_block =
			ofnode_read_s32_default(dev_ofnode(dev),
						"marvell,hs400-tuning-block",
						-1);
		debug("%s(%s): mmc HS400 tuning block: %d\n", __func__,
		      dev->name, slot->hs400_tuning_block);

		slot->hs200_tap_adj =
			ofnode_read_s32_default(dev_ofnode(dev),
						"marvell,hs200-tap-adjust", 0);
		debug("%s(%s): hs200-tap-adjust: %d\n", __func__, dev->name,
		      slot->hs200_tap_adj);
		slot->hs400_tap_adj =
			ofnode_read_s32_default(dev_ofnode(dev),
						"marvell,hs400-tap-adjust", 0);
		debug("%s(%s): hs400-tap-adjust: %d\n", __func__, dev->name,
		      slot->hs400_tap_adj);
	}

	err = ofnode_read_u32_array(dev_ofnode(dev), "voltage-ranges",
				    voltages, 2);
	if (err) {
		slot->cfg.voltages = MMC_VDD_32_33 | MMC_VDD_33_34;
	} else {
		low = xlate_voltage(voltages[0]);
		high = xlate_voltage(voltages[1]);
		debug("  low voltage: 0x%x (%u), high: 0x%x (%u)\n",
		      low, voltages[0], high, voltages[1]);
		if (low > high || !low || !high) {
			pr_err("Invalid MMC voltage range [%u-%u] specified for %s\n",
			       low, high, dev->name);
			return -1;
		}
		slot->cfg.voltages = 0;
		do {
			slot->cfg.voltages |= low;
			low <<= 1;
		} while (low <= high);
	}
	debug("%s: config voltages: 0x%x\n", __func__, slot->cfg.voltages);
	slot->slew = ofnode_read_s32_default(node, "cavium,clk-slew", -1);
	slot->drive = ofnode_read_s32_default(node, "cavium,drv-strength", -1);
	gpio_request_by_name(dev, "cd-gpios", 0, &slot->cd_gpio, GPIOD_IS_IN);
	slot->cd_inverted = ofnode_read_bool(node, "cd-inverted");
	gpio_request_by_name(dev, "wp-gpios", 0, &slot->wp_gpio, GPIOD_IS_IN);
	slot->wp_inverted = ofnode_read_bool(node, "wp-inverted");
	if (slot->cfg.voltages & MMC_VDD_165_195) {
		slot->is_1_8v = true;
		slot->is_3_3v = false;
	} else if (slot->cfg.voltages & (MMC_VDD_30_31 | MMC_VDD_31_32 |
					 MMC_VDD_33_34 | MMC_VDD_34_35 |
					 MMC_VDD_35_36)) {
		slot->is_1_8v = false;
		slot->is_3_3v = true;
	}

	bus_width = ofnode_read_u32_default(node, "bus-width", 1);
	/* Note fall-through */
	switch (bus_width) {
	case 8:
		slot->cfg.host_caps |= MMC_MODE_8BIT;
	case 4:
		slot->cfg.host_caps |= MMC_MODE_4BIT;
	case 1:
		slot->cfg.host_caps |= MMC_MODE_1BIT;
		break;
	}
	if (ofnode_read_bool(node, "no-1-8-v")) {
		slot->is_3_3v = true;
		slot->is_1_8v = false;
		if (!(slot->cfg.voltages & (MMC_VDD_32_33 | MMC_VDD_33_34)))
			pr_warn("%s(%s): voltages indicate 3.3v but 3.3v not supported\n",
				__func__, dev->name);
	}
	if (ofnode_read_bool(node, "mmc-ddr-3-3v")) {
		slot->is_3_3v = true;
		slot->is_1_8v = false;
		if (!(slot->cfg.voltages & (MMC_VDD_32_33 | MMC_VDD_33_34)))
			pr_warn("%s(%s): voltages indicate 3.3v but 3.3v not supported\n",
				__func__, dev->name);
	}
	if (ofnode_read_bool(node, "cap-sd-highspeed") ||
	    ofnode_read_bool(node, "cap-mmc-highspeed") ||
	    ofnode_read_bool(node, "sd-uhs-sdr25"))
		slot->cfg.host_caps |= MMC_MODE_HS;
	if (slot->cfg.f_max >= 50000000 &&
	    slot->cfg.host_caps & MMC_MODE_HS)
		slot->cfg.host_caps |= MMC_MODE_HS_52MHz | MMC_MODE_HS;
	if (ofnode_read_bool(node, "sd-uhs-sdr50"))
		slot->cfg.host_caps |= MMC_MODE_HS_52MHz | MMC_MODE_HS;
	if (ofnode_read_bool(node, "sd-uhs-ddr50"))
		slot->cfg.host_caps |= MMC_MODE_HS | MMC_MODE_HS_52MHz |
				       MMC_MODE_DDR_52MHz;

	if (IS_ENABLED(CONFIG_ARCH_OCTEONTX2)) {
		if (!slot->is_asim && !slot->is_emul) {
			if (ofnode_read_bool(node, "mmc-hs200-1_8v"))
				slot->cfg.host_caps |= MMC_MODE_HS200 |
					MMC_MODE_HS_52MHz;
			if (ofnode_read_bool(node, "mmc-hs400-1_8v"))
				slot->cfg.host_caps |= MMC_MODE_HS400 |
					MMC_MODE_HS_52MHz |
					MMC_MODE_HS200 |
					MMC_MODE_DDR_52MHz;
			slot->cmd_out_hs200_delay =
				ofnode_read_u32_default(node,
					"marvell,cmd-out-hs200-dly",
					MMC_DEFAULT_HS200_CMD_OUT_DLY);
			debug("%s(%s): HS200 cmd out delay: %d\n",
			      __func__, dev->name, slot->cmd_out_hs200_delay);
			slot->data_out_hs200_delay =
				ofnode_read_u32_default(node,
					"marvell,data-out-hs200-dly",
					MMC_DEFAULT_HS200_DATA_OUT_DLY);
			debug("%s(%s): HS200 data out delay: %d\n",
			      __func__, dev->name, slot->data_out_hs200_delay);
			slot->cmd_out_hs400_delay =
				ofnode_read_u32_default(node,
					"marvell,cmd-out-hs400-dly",
					MMC_DEFAULT_HS400_CMD_OUT_DLY);
			debug("%s(%s): HS400 cmd out delay: %d\n",
			      __func__, dev->name, slot->cmd_out_hs400_delay);
			slot->data_out_hs400_delay =
				ofnode_read_u32_default(node,
					"marvell,data-out-hs400-dly",
					MMC_DEFAULT_HS400_DATA_OUT_DLY);
			debug("%s(%s): HS400 data out delay: %d\n",
			      __func__, dev->name, slot->data_out_hs400_delay);
		}
	}

	slot->disable_ddr = ofnode_read_bool(node, "marvell,disable-ddr");
	slot->non_removable = ofnode_read_bool(node, "non-removable");
	slot->cmd_clk_skew = ofnode_read_u32_default(node,
						     "cavium,cmd-clk-skew", 0);
	slot->dat_clk_skew = ofnode_read_u32_default(node,
						     "cavium,dat-clk-skew", 0);
	debug("%s(%s): host caps: 0x%x\n", __func__,
	      dev->name, slot->cfg.host_caps);
	return 0;
}

/**
 * Probes a MMC slot
 *
 * @param	dev	mmc device
 *
 * Return:	0 for success, error otherwise
 */
static int octeontx_mmc_slot_probe(struct udevice *dev)
{
	struct octeontx_mmc_slot *slot;
	struct mmc *mmc;
	int err;

	debug("%s(%s)\n", __func__, dev->name);
	if (!host_probed) {
		pr_err("%s(%s): Error: host not probed yet\n",
		       __func__, dev->name);
	}
	slot = dev_to_mmc_slot(dev);
	mmc = &slot->mmc;
	mmc->dev = dev;

	slot->valid = false;
	if (!octeontx_mmc_get_valid(dev)) {
		debug("%s(%s): slot is invalid\n", __func__, dev->name);
		return -ENODEV;
	}

	debug("%s(%s): Getting config\n", __func__, dev->name);
	err = octeontx_mmc_get_config(dev);
	if (err) {
		pr_err("probe(%s): Error getting config\n", dev->name);
		return err;
	}

	debug("%s(%s): mmc bind, mmc: %p\n", __func__, dev->name, &slot->mmc);
	err = mmc_bind(dev, &slot->mmc, &slot->cfg);
	if (err) {
		pr_err("%s(%s): Error binding mmc\n", __func__, dev->name);
		return -1;
	}

	/* For some reason, mmc_bind always assigns priv to the device */
	slot->mmc.priv = slot;

	debug("%s(%s): lowlevel init\n", __func__, dev->name);
	err = octeontx_mmc_init_lowlevel(mmc);
	if (err) {
		pr_err("probe(%s): Low-level init failed\n", dev->name);
		return err;
	}

	slot->valid = true;

	debug("%s(%s):\n"
	      "  base address : %p\n"
	      "  bus id       : %d\n", __func__, dev->name,
		slot->base_addr, slot->bus_id);

	return err;
}

/**
 * MMC slot driver operations
 */
static const struct dm_mmc_ops octeontx_hsmmc_ops = {
	.send_cmd = octeontx_mmc_dev_send_cmd,
	.set_ios = octeontx_mmc_set_ios,
	.get_cd = octeontx_mmc_get_cd,
	.get_wp = octeontx_mmc_get_wp,
#ifdef MMC_SUPPORTS_TUNING
	.execute_tuning = octeontx_mmc_execute_tuning,
#endif
};

static const struct udevice_id octeontx_hsmmc_ids[] = {
	{ .compatible = "mmc-slot" },
	{ }
};

U_BOOT_DRIVER(octeontx_hsmmc_slot) = {
	.name	= "octeontx_hsmmc_slot",
	.id	= UCLASS_MMC,
	.of_match = of_match_ptr(octeontx_hsmmc_ids),
	.probe = octeontx_mmc_slot_probe,
	.ops = &octeontx_hsmmc_ops,
};

/*****************************************************************
 * PCI host driver
 *
 * The PCI host driver contains the resources used by all of the
 * slot drivers.
 *
 * The slot drivers are pseudo drivers.
 */

/**
 * Probe the MMC host controller
 *
 * @param	dev	mmc host controller device
 *
 * Return:	0 for success, -1 on error
 */
static int octeontx_mmc_host_probe(struct udevice *dev)
{
	struct octeontx_mmc_host *host = dev_get_priv(dev);
	union mio_emm_int emm_int;
	struct clk clk;
	int ret;
	u8 rev;

	debug("%s(%s): Entry host: %p\n", __func__, dev->name, host);

	if (!octeontx_mmc_get_valid(dev)) {
		debug("%s(%s): mmc host not valid\n", __func__, dev->name);
		return -ENODEV;
	}
	memset(host, 0, sizeof(*host));

	/* Octeon TX & TX2 use PCI based probing */
	if (device_is_compatible(dev, "cavium,thunder-8890-mmc")) {
		host->base_addr = dm_pci_map_bar(dev, PCI_BASE_ADDRESS_0, 0, 0, PCI_REGION_TYPE,
						 PCI_REGION_MEM);
		if (!host->base_addr) {
			pr_err("%s: Error: MMC base address not found\n",
			       __func__);
			return -1;
		}
	} else {
		host->base_addr = dev_remap_addr(dev);
	}

	host->dev = dev;
	debug("%s(%s): Base address: %p\n", __func__, dev->name,
	      host->base_addr);
	if (!dev_has_ofnode(dev)) {
		pr_err("%s: No device tree information found\n", __func__);
		return -1;
	}
	host->node = dev_ofnode(dev);
	host->last_slotid = -1;
#if !defined(CONFIG_ARCH_OCTEON)
	if (otx_is_platform(PLATFORM_ASIM))
		host->is_asim = true;
	if (otx_is_platform(PLATFORM_EMULATOR))
		host->is_emul = true;
#endif
	host->dma_wait_delay =
		ofnode_read_u32_default(dev_ofnode(dev),
					"marvell,dma-wait-delay", 1);
	/* Force reset of eMMC */
	writeq(0, host->base_addr + MIO_EMM_CFG());
	debug("%s: Clearing MIO_EMM_CFG\n", __func__);
	udelay(100);
	emm_int.u = readq(host->base_addr + MIO_EMM_INT());
	debug("%s: Writing 0x%llx to MIO_EMM_INT\n", __func__, emm_int.u);
	writeq(emm_int.u, host->base_addr + MIO_EMM_INT());

	debug("%s(%s): Getting I/O clock\n", __func__, dev->name);
	ret = clk_get_by_index(dev, 0, &clk);
	if (ret < 0)
		return ret;

	ret = clk_enable(&clk);
	if (ret)
		return ret;

	host->sys_freq = clk_get_rate(&clk);
	debug("%s(%s): I/O clock %llu\n", __func__, dev->name, host->sys_freq);

	if (IS_ENABLED(CONFIG_ARCH_OCTEONTX2)) {
		/* Flags for issues to work around */
		dm_pci_read_config8(dev, PCI_REVISION_ID, &rev);
		if (otx_is_soc(CN96XX)) {
			debug("%s: CN96XX revision %d\n", __func__, rev);
			switch (rev) {
			case 0:
				host->calibrate_glitch = true;
				host->cond_clock_glitch = true;
				break;
			case 1:
				break;
			case 2:
				break;
			case 0x10:	/* C0 */
				host->hs400_skew_needed = true;
				debug("HS400 skew support enabled\n");
				fallthrough;
			default:
				debug("CN96XX rev C0+ detected\n");
				host->tap_requires_noclk = true;
				break;
			}
		} else if (otx_is_soc(CN95XX)) {
			if (!rev)
				host->cond_clock_glitch = true;
		}
	}

	host_probed = true;

	return 0;
}

/**
 * This performs some initial setup before a probe occurs.
 *
 * @param dev:	MMC slot device
 *
 * Return: 0 for success, -1 on failure
 *
 * Do some pre-initialization before probing a slot.
 */
static int octeontx_mmc_host_child_pre_probe(struct udevice *dev)
{
	struct octeontx_mmc_host *host = dev_get_priv(dev_get_parent(dev));
	struct octeontx_mmc_slot *slot;
	struct mmc_uclass_priv *upriv;
	ofnode node = dev_ofnode(dev);
	u32 bus_id;
	char name[16];
	int err;

	debug("%s(%s) Pre-Probe\n", __func__, dev->name);
	if (ofnode_read_u32(node, "reg", &bus_id)) {
		pr_err("%s(%s): Error: \"reg\" not found in device tree\n",
		       __func__, dev->name);
		return -1;
	}
	if (bus_id > OCTEONTX_MAX_MMC_SLOT) {
		pr_err("%s(%s): Error: \"reg\" out of range of 0..%d\n",
		       __func__, dev->name, OCTEONTX_MAX_MMC_SLOT);
		return -1;
	}

	slot = &host->slots[bus_id];
	dev_set_priv(dev, slot);
	slot->host = host;
	slot->bus_id = bus_id;
	slot->dev = dev;
	slot->base_addr = host->base_addr;
	slot->is_asim = host->is_asim;
	slot->is_emul = host->is_emul;

	snprintf(name, sizeof(name), "octeontx-mmc%d", bus_id);
	err = device_set_name(dev, name);

	/* FIXME: This code should not be needed */
	if (!dev_get_uclass_priv(dev)) {
		debug("%s(%s): Allocating uclass priv\n", __func__,
		      dev->name);
		upriv = calloc(1, sizeof(struct mmc_uclass_priv));
		if (!upriv)
			return -ENOMEM;

		/*
		 * FIXME: This is not allowed
		 * dev_set_uclass_priv(dev, upriv);
		 * uclass_set_priv(dev->uclass, upriv);
		 */
	} else {
		upriv = dev_get_uclass_priv(dev);
	}

	upriv->mmc = &slot->mmc;
	debug("%s: uclass priv: %p, mmc: %p\n", dev->name, upriv, upriv->mmc);

	debug("%s: ret: %d\n", __func__, err);
	return err;
}

static const struct udevice_id octeontx_hsmmc_host_ids[] = {
	{ .compatible = "cavium,thunder-8890-mmc" },
	{ .compatible = "cavium,octeon-7360-mmc" },
	{ }
};

U_BOOT_DRIVER(octeontx_hsmmc_host) = {
	.name	= "octeontx_hsmmc_host",
	/* FIXME: Why is this not UCLASS_MMC? */
	.id	= UCLASS_MISC,
	.of_match = of_match_ptr(octeontx_hsmmc_host_ids),
	.probe	= octeontx_mmc_host_probe,
	.priv_auto	= sizeof(struct octeontx_mmc_host),
	.child_pre_probe = octeontx_mmc_host_child_pre_probe,
	.flags	= DM_FLAG_PRE_RELOC,
};

static struct pci_device_id octeontx_mmc_supported[] = {
	{ PCI_VDEVICE(CAVIUM, PCI_DEVICE_ID_CAVIUM_EMMC) },
	{ },
};

U_BOOT_PCI_DEVICE(octeontx_hsmmc_host, octeontx_mmc_supported);
