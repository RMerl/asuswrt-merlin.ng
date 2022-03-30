// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2015-2016 Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
 */

/*
 * @file
 * @brief PFE utility commands
 */

#include <net/pfe_eth/pfe_eth.h>

static inline void pfe_command_help(void)
{
	printf("Usage: pfe [pe | status | expt ] <options>\n");
}

static void pfe_command_pe(int argc, char * const argv[])
{
	if (argc >= 3 && strcmp(argv[2], "pmem") == 0) {
		if (argc >= 4 && strcmp(argv[3], "read") == 0) {
			int i;
			int num;
			int id;
			u32 addr;
			u32 size;
			u32 val;

			if (argc == 7) {
				num = simple_strtoul(argv[6], NULL, 0);
			} else if (argc == 6) {
				num = 1;
			} else {
				printf("Usage: pfe pe pmem read <id> <addr> [<num>]\n");
				return;
			}

			id = simple_strtoul(argv[4], NULL, 0);
			addr = simple_strtoul(argv[5], NULL, 16);
			size = 4;

			for (i = 0; i < num; i++, addr += 4) {
				val = pe_pmem_read(id, addr, size);
				val = be32_to_cpu(val);
				if (!(i & 3))
					printf("%08x: ", addr);
				printf("%08x%s", val, i == num - 1 || (i & 3)
				       == 3 ? "\n" : " ");
			}

		} else {
			printf("Usage: pfe pe pmem read <parameters>\n");
		}
	} else if (argc >= 3 && strcmp(argv[2], "dmem") == 0) {
		if (argc >= 4 && strcmp(argv[3], "read") == 0) {
			int i;
			int num;
			int id;
			u32 addr;
			u32 size;
			u32 val;

			if (argc == 7) {
				num = simple_strtoul(argv[6], NULL, 0);
			} else if (argc == 6) {
				num = 1;
			} else {
				printf("Usage: pfe pe dmem read <id> <addr> [<num>]\n");
				return;
			}

			id = simple_strtoul(argv[4], NULL, 0);
			addr = simple_strtoul(argv[5], NULL, 16);
			size = 4;

			for (i = 0; i < num; i++, addr += 4) {
				val = pe_dmem_read(id, addr, size);
				val = be32_to_cpu(val);
				if (!(i & 3))
					printf("%08x: ", addr);
				printf("%08x%s", val, i == num - 1 || (i & 3)
				       == 3 ? "\n" : " ");
			}

		} else if (argc >= 4 && strcmp(argv[3], "write") == 0) {
			int id;
			u32 val;
			u32 addr;
			u32 size;

			if (argc != 7) {
				printf("Usage: pfe pe dmem write <id> <val> <addr>\n");
				return;
			}

			id = simple_strtoul(argv[4], NULL, 0);
			val = simple_strtoul(argv[5], NULL, 16);
			val = cpu_to_be32(val);
			addr = simple_strtoul(argv[6], NULL, 16);
			size = 4;
			pe_dmem_write(id, val, addr, size);
		} else {
			printf("Usage: pfe pe dmem [read | write] <parameters>\n");
		}
	} else if (argc >= 3 && strcmp(argv[2], "lmem") == 0) {
		if (argc >= 4 && strcmp(argv[3], "read") == 0) {
			int i;
			int num;
			u32 val;
			u32 offset;

			if (argc == 6) {
				num = simple_strtoul(argv[5], NULL, 0);
			} else if (argc == 5) {
				num = 1;
			} else {
				printf("Usage: pfe pe lmem read <offset> [<num>]\n");
				return;
			}

			offset = simple_strtoul(argv[4], NULL, 16);

			for (i = 0; i < num; i++, offset += 4) {
				pe_lmem_read(&val, 4, offset);
				val = be32_to_cpu(val);
				printf("%08x%s", val, i == num - 1 || (i & 7)
				       == 7 ? "\n" : " ");
			}

		} else if (argc >= 4 && strcmp(argv[3], "write") == 0)	{
			u32 val;
			u32 offset;

			if (argc != 6) {
				printf("Usage: pfe pe lmem write <val> <offset>\n");
				return;
			}

			val = simple_strtoul(argv[4], NULL, 16);
			val = cpu_to_be32(val);
			offset = simple_strtoul(argv[5], NULL, 16);
			pe_lmem_write(&val, 4, offset);
		} else {
			printf("Usage: pfe pe lmem [read | write] <parameters>\n");
		}
	} else {
		if (strcmp(argv[2], "help") != 0)
			printf("Unknown option: %s\n", argv[2]);

		printf("Usage: pfe pe <parameters>\n");
	}
}

#define NUM_QUEUES		16

/*
 * qm_read_drop_stat
 * This function is used to read the drop statistics from the TMU
 * hw drop counter.  Since the hw counter is always cleared afer
 * reading, this function maintains the previous drop count, and
 * adds the new value to it.  That value can be retrieved by
 * passing a pointer to it with the total_drops arg.
 *
 * @param tmu           TMU number (0 - 3)
 * @param queue         queue number (0 - 15)
 * @param total_drops   pointer to location to store total drops (or NULL)
 * @param do_reset      if TRUE, clear total drops after updating
 *
 */
u32 qm_read_drop_stat(u32 tmu, u32 queue, u32 *total_drops, int do_reset)
{
	static u32 qtotal[TMU_MAX_ID + 1][NUM_QUEUES];
	u32 val;

	writel((tmu << 8) | queue, TMU_TEQ_CTRL);
	writel((tmu << 8) | queue, TMU_LLM_CTRL);
	val = readl(TMU_TEQ_DROP_STAT);
	qtotal[tmu][queue] += val;
	if (total_drops)
		*total_drops = qtotal[tmu][queue];
	if (do_reset)
		qtotal[tmu][queue] = 0;
	return val;
}

static ssize_t tmu_queue_stats(char *buf, int tmu, int queue)
{
	ssize_t len = 0;
	u32 drops;

	printf("%d-%02d, ", tmu, queue);

	drops = qm_read_drop_stat(tmu, queue, NULL, 0);

	/* Select queue */
	writel((tmu << 8) | queue, TMU_TEQ_CTRL);
	writel((tmu << 8) | queue, TMU_LLM_CTRL);

	printf("(teq) drop: %10u, tx: %10u (llm) head: %08x, tail: %08x, drop: %10u\n",
	       drops, readl(TMU_TEQ_TRANS_STAT),
	       readl(TMU_LLM_QUE_HEADPTR), readl(TMU_LLM_QUE_TAILPTR),
	       readl(TMU_LLM_QUE_DROPCNT));

	return len;
}

static ssize_t tmu_queues(char *buf, int tmu)
{
	ssize_t len = 0;
	int queue;

	for (queue = 0; queue < 16; queue++)
		len += tmu_queue_stats(buf + len, tmu, queue);

	return len;
}

static inline void hif_status(void)
{
	printf("hif:\n");

	printf("  tx curr bd:    %x\n", readl(HIF_TX_CURR_BD_ADDR));
	printf("  tx status:     %x\n", readl(HIF_TX_STATUS));
	printf("  tx dma status: %x\n", readl(HIF_TX_DMA_STATUS));

	printf("  rx curr bd:    %x\n", readl(HIF_RX_CURR_BD_ADDR));
	printf("  rx status:     %x\n", readl(HIF_RX_STATUS));
	printf("  rx dma status: %x\n", readl(HIF_RX_DMA_STATUS));

	printf("hif nocopy:\n");

	printf("  tx curr bd:    %x\n", readl(HIF_NOCPY_TX_CURR_BD_ADDR));
	printf("  tx status:     %x\n", readl(HIF_NOCPY_TX_STATUS));
	printf("  tx dma status: %x\n", readl(HIF_NOCPY_TX_DMA_STATUS));

	printf("  rx curr bd:    %x\n", readl(HIF_NOCPY_RX_CURR_BD_ADDR));
	printf("  rx status:     %x\n", readl(HIF_NOCPY_RX_STATUS));
	printf("  rx dma status: %x\n", readl(HIF_NOCPY_RX_DMA_STATUS));
}

static void gpi(int id, void *base)
{
	u32 val;

	printf("%s%d:\n", __func__, id);

	printf("  tx under stick: %x\n", readl(base + GPI_FIFO_STATUS));
	val = readl(base + GPI_FIFO_DEBUG);
	printf("  tx pkts:        %x\n", (val >> 23) & 0x3f);
	printf("  rx pkts:        %x\n", (val >> 18) & 0x3f);
	printf("  tx bytes:       %x\n", (val >> 9) & 0x1ff);
	printf("  rx bytes:       %x\n", (val >> 0) & 0x1ff);
	printf("  overrun:        %x\n", readl(base + GPI_OVERRUN_DROPCNT));
}

static void  bmu(int id, void *base)
{
	printf("%s%d:\n", __func__, id);

	printf("  buf size:  %x\n", (1 << readl(base + BMU_BUF_SIZE)));
	printf("  buf count: %x\n", readl(base + BMU_BUF_CNT));
	printf("  buf rem:   %x\n", readl(base + BMU_REM_BUF_CNT));
	printf("  buf curr:  %x\n", readl(base + BMU_CURR_BUF_CNT));
	printf("  free err:  %x\n", readl(base + BMU_FREE_ERR_ADDR));
}

#define	PESTATUS_ADDR_CLASS	0x800
#define PEMBOX_ADDR_CLASS	0x890
#define	PESTATUS_ADDR_TMU	0x80
#define PEMBOX_ADDR_TMU		0x290
#define	PESTATUS_ADDR_UTIL	0x0

static void pfe_pe_status(int argc, char * const argv[])
{
	int do_clear = 0;
	u32 id;
	u32 dmem_addr;
	u32 cpu_state;
	u32 activity_counter;
	u32 rx;
	u32 tx;
	u32 drop;
	char statebuf[5];
	u32 class_debug_reg = 0;

	if (argc == 4 && strcmp(argv[3], "clear") == 0)
		do_clear = 1;

	for (id = CLASS0_ID; id < MAX_PE; id++) {
		if (id >= TMU0_ID) {
			if (id == TMU2_ID)
				continue;
			if (id == TMU0_ID)
				printf("tmu:\n");
			dmem_addr = PESTATUS_ADDR_TMU;
		} else {
			if (id == CLASS0_ID)
				printf("class:\n");
			dmem_addr = PESTATUS_ADDR_CLASS;
			class_debug_reg = readl(CLASS_PE0_DEBUG + id * 4);
		}

		cpu_state = pe_dmem_read(id, dmem_addr, 4);
		dmem_addr += 4;
		memcpy(statebuf, (char *)&cpu_state, 4);
		statebuf[4] = '\0';
		activity_counter = pe_dmem_read(id, dmem_addr, 4);
		dmem_addr += 4;
		rx = pe_dmem_read(id, dmem_addr, 4);
		if (do_clear)
			pe_dmem_write(id, 0, dmem_addr, 4);
		dmem_addr += 4;
		tx = pe_dmem_read(id, dmem_addr, 4);
		if (do_clear)
			pe_dmem_write(id, 0, dmem_addr, 4);
		dmem_addr += 4;
		drop = pe_dmem_read(id, dmem_addr, 4);
		if (do_clear)
			pe_dmem_write(id, 0, dmem_addr, 4);
		dmem_addr += 4;

		if (id >= TMU0_ID) {
			printf("%d: state=%4s ctr=%08x rx=%x qstatus=%x\n",
			       id - TMU0_ID, statebuf,
			       cpu_to_be32(activity_counter),
			       cpu_to_be32(rx), cpu_to_be32(tx));
		} else {
			printf("%d: pc=1%04x ldst=%04x state=%4s ctr=%08x rx=%x tx=%x drop=%x\n",
			       id - CLASS0_ID, class_debug_reg & 0xFFFF,
			       class_debug_reg >> 16,
			       statebuf, cpu_to_be32(activity_counter),
			       cpu_to_be32(rx), cpu_to_be32(tx),
			       cpu_to_be32(drop));
		}
	}
}

static void pfe_command_status(int argc, char * const argv[])
{
	if (argc >= 3 && strcmp(argv[2], "pe") == 0) {
		pfe_pe_status(argc, argv);
	} else if (argc == 3 && strcmp(argv[2], "bmu") == 0) {
		bmu(1, BMU1_BASE_ADDR);
		bmu(2, BMU2_BASE_ADDR);
	} else if (argc == 3 && strcmp(argv[2], "hif") == 0) {
		hif_status();
	} else if (argc == 3 && strcmp(argv[2], "gpi") == 0) {
		gpi(0, EGPI1_BASE_ADDR);
		gpi(1, EGPI2_BASE_ADDR);
		gpi(3, HGPI_BASE_ADDR);
	} else if (argc == 3 && strcmp(argv[2], "tmu0_queues") == 0) {
		tmu_queues(NULL, 0);
	} else if (argc == 3 && strcmp(argv[2], "tmu1_queues") == 0) {
		tmu_queues(NULL, 1);
	} else if (argc == 3 && strcmp(argv[2], "tmu3_queues") == 0) {
		tmu_queues(NULL, 3);
	} else {
		printf("Usage: pfe status [pe <clear> | bmu | gpi | hif | tmuX_queues ]\n");
	}
}

#define EXPT_DUMP_ADDR 0x1fa8
#define EXPT_REG_COUNT 20
static const char *register_names[EXPT_REG_COUNT] = {
		"  pc", "ECAS", " EID", "  ED",
		"  sp", "  r1", "  r2", "  r3",
		"  r4", "  r5", "  r6", "  r7",
		"  r8", "  r9", " r10", " r11",
		" r12", " r13", " r14", " r15"
};

static void pfe_command_expt(int argc, char * const argv[])
{
	unsigned int id, i, val, addr;

	if (argc == 3) {
		id = simple_strtoul(argv[2], NULL, 0);
		addr = EXPT_DUMP_ADDR;
		printf("Exception information for PE %d:\n", id);
		for (i = 0; i < EXPT_REG_COUNT; i++) {
			val = pe_dmem_read(id, addr, 4);
			val = be32_to_cpu(val);
			printf("%s:%08x%s", register_names[i], val,
			       (i & 3) == 3 ? "\n" : " ");
			addr += 4;
		}
	} else {
		printf("Usage: pfe expt <id>\n");
	}
}

#ifdef PFE_RESET_WA
/*This function sends a dummy packet to HIF through TMU3 */
static void send_dummy_pkt_to_hif(void)
{
	u32 buf;
	static u32 dummy_pkt[] =  {
		0x4200800a, 0x01000003, 0x00018100, 0x00000000,
		0x33221100, 0x2b785544, 0xd73093cb, 0x01000608,
		0x04060008, 0x2b780200, 0xd73093cb, 0x0a01a8c0,
		0x33221100, 0xa8c05544, 0x00000301, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0xbe86c51f };

	/*Allocate BMU2 buffer */
	buf = readl(BMU2_BASE_ADDR + BMU_ALLOC_CTRL);

	debug("Sending a dummy pkt to HIF %x\n", buf);
	buf += 0x80;
	memcpy((void *)DDR_PFE_TO_VIRT(buf), dummy_pkt, sizeof(dummy_pkt));

	/*Write length and pkt to TMU*/
	writel(0x03000042, TMU_PHY_INQ_PKTPTR);
	writel(buf, TMU_PHY_INQ_PKTINFO);
}

static void pfe_command_stop(int argc, char * const argv[])
{
	int pfe_pe_id, hif_stop_loop = 10;
	u32 rx_status;

	printf("Stopping PFE...\n");

	/*Mark all descriptors as LAST_BD */
	hif_rx_desc_disable();

	/*If HIF Rx BDP is busy send a dummy packet */
	do {
		rx_status = readl(HIF_RX_STATUS);
		if (rx_status & BDP_CSR_RX_DMA_ACTV)
			send_dummy_pkt_to_hif();
		udelay(10);
	} while (hif_stop_loop--);

	if (readl(HIF_RX_STATUS) & BDP_CSR_RX_DMA_ACTV)
		printf("Unable to stop HIF\n");

	/*Disable Class PEs */
	for (pfe_pe_id = CLASS0_ID; pfe_pe_id <= CLASS_MAX_ID; pfe_pe_id++) {
		/*Inform PE to stop */
		pe_dmem_write(pfe_pe_id, cpu_to_be32(1), PEMBOX_ADDR_CLASS, 4);
		udelay(10);

		/*Read status */
		if (!pe_dmem_read(pfe_pe_id, PEMBOX_ADDR_CLASS + 4, 4))
			printf("Failed to stop PE%d\n", pfe_pe_id);
	}

	/*Disable TMU PEs */
	for (pfe_pe_id = TMU0_ID; pfe_pe_id <= TMU_MAX_ID; pfe_pe_id++) {
		if (pfe_pe_id == TMU2_ID)
			continue;

		/*Inform PE to stop */
		pe_dmem_write(pfe_pe_id, 1, PEMBOX_ADDR_TMU, 4);
		udelay(10);

		/*Read status */
		if (!pe_dmem_read(pfe_pe_id, PEMBOX_ADDR_TMU + 4, 4))
			printf("Failed to stop PE%d\n", pfe_pe_id);
	}
}
#endif

static int pfe_command(cmd_tbl_t *cmdtp, int flag, int argc,
		       char * const argv[])
{
	if (argc == 1 || strcmp(argv[1], "help") == 0) {
		pfe_command_help();
		return CMD_RET_SUCCESS;
	}

	if (strcmp(argv[1], "pe") == 0) {
		pfe_command_pe(argc, argv);
	} else if (strcmp(argv[1], "status") == 0) {
		pfe_command_status(argc, argv);
	} else if (strcmp(argv[1], "expt") == 0) {
		pfe_command_expt(argc, argv);
#ifdef PFE_RESET_WA
	} else if (strcmp(argv[1], "stop") == 0) {
		pfe_command_stop(argc, argv);
#endif
	} else {
		printf("Unknown option: %s\n", argv[1]);
		pfe_command_help();
		return CMD_RET_FAILURE;
	}
	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	pfe,	7,	1,	pfe_command,
	"Performs PFE lib utility functions",
	"Usage:\n"
	"pfe <options>"
);
