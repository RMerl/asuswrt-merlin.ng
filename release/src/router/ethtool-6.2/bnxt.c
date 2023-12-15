/* Code to dump registers for NetXtreme-E/NetXtreme-C Broadcom devices.
 *
 * Copyright (c) 2020 Broadcom Inc.
 */
#include <stdio.h>
#include "internal.h"

#define BNXT_PXP_REG_LEN	0x3110
#define BNXT_PCIE_STATS_LEN	(12 * sizeof(u64))

struct bnxt_pcie_stat {
	const char *name;
	u16 offset;
	u8 size;
	const char *format;
};

static const struct bnxt_pcie_stat bnxt_pcie_stats[] = {
	{ .name = "PL Signal integrity errors", .offset = 0, .size = 4, .format = "%llu" },
	{ .name = "DL Signal integrity errors", .offset = 4, .size = 4, .format = "%llu" },
	{ .name = "TLP Signal integrity errors", .offset = 8, .size = 4, .format = "%llu" },
	{ .name = "Link integrity", .offset = 12, .size = 4, .format = "%llu" },
	{ .name = "TX TLP traffic rate", .offset = 16, .size = 4, .format = "%llu" },
	{ .name = "RX TLP traffic rate", .offset = 20, .size = 4, .format = "%llu" },
	{ .name = "TX DLLP traffic rate", .offset = 24, .size = 4, .format = "%llu" },
	{ .name = "RX DLLP traffic rate", .offset = 28, .size = 4, .format = "%llu" },
	{ .name = "Equalization Phase 0 time(ms)", .offset = 33, .size = 1, .format = "0x%x" },
	{ .name = "Equalization Phase 1 time(ms)", .offset = 32, .size = 1, .format = "0x%x" },
	{ .name = "Equalization Phase 2 time(ms)", .offset = 35, .size = 1, .format = "0x%x" },
	{ .name = "Equalization Phase 3 time(ms)", .offset = 34, .size = 1, .format = "0x%x" },
	{ .name = "PHY LTSSM Histogram 0", .offset = 36, .size = 2, .format = "0x%x"},
	{ .name = "PHY LTSSM Histogram 1", .offset = 38, .size = 2, .format = "0x%x"},
	{ .name = "PHY LTSSM Histogram 2", .offset = 40, .size = 2, .format = "0x%x"},
	{ .name = "PHY LTSSM Histogram 3", .offset = 42, .size = 2, .format = "0x%x"},
	{ .name = "Recovery Histogram 0", .offset = 44, .size = 2, .format = "0x%x"},
	{ .name = "Recovery Histogram 1", .offset = 46, .size = 2, .format = "0x%x"},
};

int bnxt_dump_regs(struct ethtool_drvinfo *info __maybe_unused, struct ethtool_regs *regs)
{
	const struct bnxt_pcie_stat *stats = bnxt_pcie_stats;
	u16 *pcie_stats, pcie_stat16;
	u32 reg, i, pcie_stat32;
	u64 pcie_stat64;

	if (regs->len < BNXT_PXP_REG_LEN) {
		fprintf(stdout, "Length too short, expected at least 0x%x\n",
			BNXT_PXP_REG_LEN);
		return -1;
	}

	fprintf(stdout, "PXP Registers\n");
	fprintf(stdout, "Offset\tValue\n");
	fprintf(stdout, "------\t-------\n");
	for (i = 0; i < BNXT_PXP_REG_LEN; i += sizeof(reg)) {
		memcpy(&reg, &regs->data[i], sizeof(reg));
		if (reg)
			fprintf(stdout, "0x%04x\t0x%08x\n", i, reg);
	}
	fprintf(stdout, "\n");

	if (!regs->version)
		return 0;

	if (regs->len < (BNXT_PXP_REG_LEN + BNXT_PCIE_STATS_LEN)) {
		fprintf(stdout, "Length is too short, expected 0x%zx\n",
			BNXT_PXP_REG_LEN + BNXT_PCIE_STATS_LEN);
		return -1;
	}

	pcie_stats = (u16 *)(regs->data + BNXT_PXP_REG_LEN);
	fprintf(stdout, "PCIe statistics:\n");
	fprintf(stdout, "----------------\n");
	for (i = 0; i < ARRAY_SIZE(bnxt_pcie_stats); i++) {
		fprintf(stdout, "%-30s : ", stats[i].name);
		switch (stats[i].size) {
		case 1:
			pcie_stat16 = 0;
			memcpy(&pcie_stat16, &pcie_stats[stats[i].offset], sizeof(u16));
			fprintf(stdout, stats[i].format, pcie_stat16);
			break;
		case 2:
			pcie_stat32 = 0;
			memcpy(&pcie_stat32, &pcie_stats[stats[i].offset], sizeof(u32));
			fprintf(stdout, stats[i].format, pcie_stat32);
			break;
		case 4:
			pcie_stat64 = 0;
			memcpy(&pcie_stat64, &pcie_stats[stats[i].offset], sizeof(u64));
			fprintf(stdout, stats[i].format, pcie_stat64);
			break;
		}
		fprintf(stdout, "\n");
	}

	return 0;
}
