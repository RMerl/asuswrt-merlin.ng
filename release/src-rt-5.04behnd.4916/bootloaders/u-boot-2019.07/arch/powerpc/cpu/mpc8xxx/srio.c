// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2011 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <config.h>
#include <asm/fsl_law.h>
#include <asm/fsl_serdes.h>
#include <asm/fsl_srio.h>
#include <linux/errno.h>

#ifdef CONFIG_SRIO_PCIE_BOOT_MASTER
#define SRIO_PORT_ACCEPT_ALL 0x10000001
#define SRIO_IB_ATMU_AR 0x80f55000
#define SRIO_OB_ATMU_AR_MAINT 0x80077000
#define SRIO_OB_ATMU_AR_RW 0x80045000
#define SRIO_LCSBA1CSR_OFFSET 0x5c
#define SRIO_MAINT_WIN_SIZE 0x1000000 /* 16M */
#define SRIO_RW_WIN_SIZE 0x100000 /* 1M */
#define SRIO_LCSBA1CSR 0x60000000
#endif

#if defined(CONFIG_FSL_CORENET)
#ifdef CONFIG_SYS_FSL_QORIQ_CHASSIS2
	#define _DEVDISR_SRIO1 FSL_CORENET_DEVDISR3_SRIO1
	#define _DEVDISR_SRIO2 FSL_CORENET_DEVDISR3_SRIO2
#else
	#define _DEVDISR_SRIO1 FSL_CORENET_DEVDISR_SRIO1
	#define _DEVDISR_SRIO2 FSL_CORENET_DEVDISR_SRIO2
#endif
	#define _DEVDISR_RMU   FSL_CORENET_DEVDISR_RMU
	#define CONFIG_SYS_MPC8xxx_GUTS_ADDR CONFIG_SYS_MPC85xx_GUTS_ADDR
#elif defined(CONFIG_MPC85xx)
	#define _DEVDISR_SRIO1 MPC85xx_DEVDISR_SRIO
	#define _DEVDISR_SRIO2 MPC85xx_DEVDISR_SRIO
	#define _DEVDISR_RMU   MPC85xx_DEVDISR_RMSG
	#define CONFIG_SYS_MPC8xxx_GUTS_ADDR CONFIG_SYS_MPC85xx_GUTS_ADDR
#elif defined(CONFIG_MPC86xx)
	#define _DEVDISR_SRIO1 MPC86xx_DEVDISR_SRIO
	#define _DEVDISR_SRIO2 MPC86xx_DEVDISR_SRIO
	#define _DEVDISR_RMU   MPC86xx_DEVDISR_RMSG
	#define CONFIG_SYS_MPC8xxx_GUTS_ADDR \
		(&((immap_t *)CONFIG_SYS_IMMR)->im_gur)
#else
#error "No defines for DEVDISR_SRIO"
#endif

#ifdef CONFIG_SYS_FSL_ERRATUM_SRIO_A004034
/*
 * Erratum A-004034
 * Affects: SRIO
 * Description: During port initialization, the SRIO port performs
 * lane synchronization (detecting valid symbols on a lane) and
 * lane alignment (coordinating multiple lanes to receive valid data
 * across lanes). Internal errors in lane synchronization and lane
 * alignment may cause failure to achieve link initialization at
 * the configured port width.
 * An SRIO port configured as a 4x port may see one of these scenarios:
 * 1. One or more lanes fails to achieve lane synchronization. Depending
 * on which lanes fail, this may result in downtraining from 4x to 1x
 * on lane 0, 4x to 1x on lane R (redundant lane).
 * 2. The link may fail to achieve lane alignment as a 4x, even though
 * all 4 lanes achieve lane synchronization, and downtrain to a 1x.
 * An SRIO port configured as a 1x port may fail to complete port
 * initialization (PnESCSR[PU] never deasserts) because of scenario 1.
 * Impact: SRIO port may downtrain to 1x, or may fail to complete
 * link initialization. Once a port completes link initialization
 * successfully, it will operate normally.
 */
static int srio_erratum_a004034(u8 port)
{
	serdes_corenet_t *srds_regs;
	u32 conf_lane;
	u32 init_lane;
	int idx, first, last;
	u32 i;
	unsigned long long end_tick;
	struct ccsr_rio *srio_regs = (void *)CONFIG_SYS_FSL_SRIO_ADDR;

	srds_regs = (void *)(CONFIG_SYS_FSL_CORENET_SERDES_ADDR);
	conf_lane = (in_be32((void *)&srds_regs->srdspccr0)
			>> (12 - port * 4)) & 0x3;
	init_lane = (in_be32((void *)&srio_regs->lp_serial
			.port[port].pccsr) >> 27) & 0x7;

	/*
	 * Start a counter set to ~2 ms after the SERDES reset is
	 * complete (SERDES SRDSBnRSTCTL[RST_DONE]=1 for n
	 * corresponding to the SERDES bank/PLL for the SRIO port).
	 */
	 if (in_be32((void *)&srds_regs->bank[0].rstctl)
		& SRDS_RSTCTL_RSTDONE) {
		/*
		 * Poll the port uninitialized status (SRIO PnESCSR[PO]) until
		 * PO=1 or the counter expires. If the counter expires, the
		 * port has failed initialization: go to recover steps. If PO=1
		 * and the desired port width is 1x, go to normal steps. If
		 * PO = 1 and the desired port width is 4x, go to recover steps.
		 */
		end_tick = usec2ticks(2000) + get_ticks();
		do {
			if (in_be32((void *)&srio_regs->lp_serial
				.port[port].pescsr) & 0x2) {
				if (conf_lane == 0x1)
					goto host_ok;
				else {
					if (init_lane == 0x2)
						goto host_ok;
					else
						break;
				}
			}
		} while (end_tick > get_ticks());

		/* recover at most 3 times */
		for (i = 0; i < 3; i++) {
			/* Set SRIO PnCCSR[PD]=1 */
			setbits_be32((void *)&srio_regs->lp_serial
					.port[port].pccsr,
					0x800000);
			/*
			* Set SRIO PnPCR[OBDEN] on the host to
			* enable the discarding of any pending packets.
			*/
			setbits_be32((void *)&srio_regs->impl.port[port].pcr,
				0x04);
			/* Wait 50 us */
			udelay(50);
			/* Run sync command */
			isync();

			if (port)
				first = serdes_get_first_lane(SRIO2);
			else
				first = serdes_get_first_lane(SRIO1);
			if (unlikely(first < 0))
				return -ENODEV;
			if (conf_lane == 0x1)
				last = first;
			else
				last = first + 3;
			/*
			 * Set SERDES BnGCRm0[RRST]=0 for each SRIO
			 * bank n and lane m.
			 */
			for (idx = first; idx <= last; idx++)
				clrbits_be32(&srds_regs->lane[idx].gcr0,
				SRDS_GCR0_RRST);
			/*
			 * Read SERDES BnGCRm0 for each SRIO
			 * bank n and lane m
			 */
			for (idx = first; idx <= last; idx++)
				in_be32(&srds_regs->lane[idx].gcr0);
			/* Run sync command */
			isync();
			/* Wait >= 100 ns */
			udelay(1);
			/*
			 * Set SERDES BnGCRm0[RRST]=1 for each SRIO
			 * bank n and lane m.
			 */
			for (idx = first; idx <= last; idx++)
				setbits_be32(&srds_regs->lane[idx].gcr0,
				SRDS_GCR0_RRST);
			/*
			 * Read SERDES BnGCRm0 for each SRIO
			 * bank n and lane m
			 */
			for (idx = first; idx <= last; idx++)
				in_be32(&srds_regs->lane[idx].gcr0);
			/* Run sync command */
			isync();
			/* Wait >= 300 ns */
			udelay(1);

			/* Write 1 to clear all bits in SRIO PnSLCSR */
			out_be32((void *)&srio_regs->impl.port[port].slcsr,
				0xffffffff);
			/* Clear SRIO PnPCR[OBDEN] on the host */
			clrbits_be32((void *)&srio_regs->impl.port[port].pcr,
				0x04);
			/* Set SRIO PnCCSR[PD]=0 */
			clrbits_be32((void *)&srio_regs->lp_serial
				.port[port].pccsr,
				0x800000);
			/* Wait >= 24 ms */
			udelay(24000);
			/* Poll the state of the port again */
			init_lane =
				(in_be32((void *)&srio_regs->lp_serial
					.port[port].pccsr) >> 27) & 0x7;
			if (in_be32((void *)&srio_regs->lp_serial
				.port[port].pescsr) & 0x2) {
				if (conf_lane == 0x1)
					goto host_ok;
				else {
					if (init_lane == 0x2)
						goto host_ok;
				}
			}
			if (i == 2)
				return -ENODEV;
		}
	} else
		return -ENODEV;

host_ok:
	/* Poll PnESCSR[OES] on the host until it is clear */
	end_tick = usec2ticks(1000000) + get_ticks();
	do {
		if (!(in_be32((void *)&srio_regs->lp_serial.port[port].pescsr)
			& 0x10000)) {
			out_be32(((void *)&srio_regs->lp_serial
				.port[port].pescsr), 0xffffffff);
			out_be32(((void *)&srio_regs->phys_err
				.port[port].edcsr), 0);
			out_be32(((void *)&srio_regs->logical_err.ltledcsr), 0);
			return 0;
		}
	} while (end_tick > get_ticks());

	return -ENODEV;
}
#endif

void srio_init(void)
{
	ccsr_gur_t *gur = (void *)CONFIG_SYS_MPC8xxx_GUTS_ADDR;
	int srio1_used = 0, srio2_used = 0;
	u32 *devdisr;

#ifdef CONFIG_SYS_FSL_QORIQ_CHASSIS2
	devdisr = &gur->devdisr3;
#else
	devdisr = &gur->devdisr;
#endif
	if (is_serdes_configured(SRIO1)) {
		set_next_law(CONFIG_SYS_SRIO1_MEM_PHYS,
				law_size_bits(CONFIG_SYS_SRIO1_MEM_SIZE),
				LAW_TRGT_IF_RIO_1);
		srio1_used = 1;
#ifdef CONFIG_SYS_FSL_ERRATUM_SRIO_A004034
		if (srio_erratum_a004034(0) < 0)
			printf("SRIO1: enabled but port error\n");
		else
#endif
		printf("SRIO1: enabled\n");
	} else {
		printf("SRIO1: disabled\n");
	}

#ifdef CONFIG_SRIO2
	if (is_serdes_configured(SRIO2)) {
		set_next_law(CONFIG_SYS_SRIO2_MEM_PHYS,
				law_size_bits(CONFIG_SYS_SRIO2_MEM_SIZE),
				LAW_TRGT_IF_RIO_2);
		srio2_used = 1;
#ifdef CONFIG_SYS_FSL_ERRATUM_SRIO_A004034
		if (srio_erratum_a004034(1) < 0)
			printf("SRIO2: enabled but port error\n");
		else
#endif
		printf("SRIO2: enabled\n");

	} else {
		printf("SRIO2: disabled\n");
	}
#endif

#ifdef CONFIG_FSL_CORENET
	/* On FSL_CORENET devices we can disable individual ports */
	if (!srio1_used)
		setbits_be32(devdisr, _DEVDISR_SRIO1);
	if (!srio2_used)
		setbits_be32(devdisr, _DEVDISR_SRIO2);
#endif

	/* neither port is used - disable everything */
	if (!srio1_used && !srio2_used) {
		setbits_be32(devdisr, _DEVDISR_SRIO1);
		setbits_be32(devdisr, _DEVDISR_SRIO2);
		setbits_be32(devdisr, _DEVDISR_RMU);
	}
}

#ifdef CONFIG_SRIO_PCIE_BOOT_MASTER
void srio_boot_master(int port)
{
	struct ccsr_rio *srio = (void *)CONFIG_SYS_FSL_SRIO_ADDR;

	/* set port accept-all */
	out_be32((void *)&srio->impl.port[port - 1].ptaacr,
				SRIO_PORT_ACCEPT_ALL);

	debug("SRIOBOOT - MASTER: Master port [ %d ] for srio boot.\n", port);
	/* configure inbound window for slave's u-boot image */
	debug("SRIOBOOT - MASTER: Inbound window for slave's image; "
			"Local = 0x%llx, Srio = 0x%llx, Size = 0x%x\n",
			(u64)CONFIG_SRIO_PCIE_BOOT_IMAGE_MEM_PHYS,
			(u64)CONFIG_SRIO_PCIE_BOOT_IMAGE_MEM_BUS1,
			CONFIG_SRIO_PCIE_BOOT_IMAGE_SIZE);
	out_be32((void *)&srio->atmu.port[port - 1].inbw[0].riwtar,
			CONFIG_SRIO_PCIE_BOOT_IMAGE_MEM_PHYS >> 12);
	out_be32((void *)&srio->atmu.port[port - 1].inbw[0].riwbar,
			CONFIG_SRIO_PCIE_BOOT_IMAGE_MEM_BUS1 >> 12);
	out_be32((void *)&srio->atmu.port[port - 1].inbw[0].riwar,
			SRIO_IB_ATMU_AR
			| atmu_size_mask(CONFIG_SRIO_PCIE_BOOT_IMAGE_SIZE));

	/* configure inbound window for slave's u-boot image */
	debug("SRIOBOOT - MASTER: Inbound window for slave's image; "
			"Local = 0x%llx, Srio = 0x%llx, Size = 0x%x\n",
			(u64)CONFIG_SRIO_PCIE_BOOT_IMAGE_MEM_PHYS,
			(u64)CONFIG_SRIO_PCIE_BOOT_IMAGE_MEM_BUS2,
			CONFIG_SRIO_PCIE_BOOT_IMAGE_SIZE);
	out_be32((void *)&srio->atmu.port[port - 1].inbw[1].riwtar,
			CONFIG_SRIO_PCIE_BOOT_IMAGE_MEM_PHYS >> 12);
	out_be32((void *)&srio->atmu.port[port - 1].inbw[1].riwbar,
			CONFIG_SRIO_PCIE_BOOT_IMAGE_MEM_BUS2 >> 12);
	out_be32((void *)&srio->atmu.port[port - 1].inbw[1].riwar,
			SRIO_IB_ATMU_AR
			| atmu_size_mask(CONFIG_SRIO_PCIE_BOOT_IMAGE_SIZE));

	/* configure inbound window for slave's ucode and ENV */
	debug("SRIOBOOT - MASTER: Inbound window for slave's ucode and ENV; "
			"Local = 0x%llx, Srio = 0x%llx, Size = 0x%x\n",
			(u64)CONFIG_SRIO_PCIE_BOOT_UCODE_ENV_MEM_PHYS,
			(u64)CONFIG_SRIO_PCIE_BOOT_UCODE_ENV_MEM_BUS,
			CONFIG_SRIO_PCIE_BOOT_UCODE_ENV_SIZE);
	out_be32((void *)&srio->atmu.port[port - 1].inbw[2].riwtar,
			CONFIG_SRIO_PCIE_BOOT_UCODE_ENV_MEM_PHYS >> 12);
	out_be32((void *)&srio->atmu.port[port - 1].inbw[2].riwbar,
			CONFIG_SRIO_PCIE_BOOT_UCODE_ENV_MEM_BUS >> 12);
	out_be32((void *)&srio->atmu.port[port - 1].inbw[2].riwar,
			SRIO_IB_ATMU_AR
			| atmu_size_mask(CONFIG_SRIO_PCIE_BOOT_UCODE_ENV_SIZE));
}

void srio_boot_master_release_slave(int port)
{
	struct ccsr_rio *srio = (void *)CONFIG_SYS_FSL_SRIO_ADDR;
	u32 escsr;
	debug("SRIOBOOT - MASTER: "
			"Check the port status and release slave core ...\n");

	escsr = in_be32((void *)&srio->lp_serial.port[port - 1].pescsr);
	if (escsr & 0x2) {
		if (escsr & 0x10100) {
			debug("SRIOBOOT - MASTER: Port [ %d ] is error.\n",
				port);
		} else {
			debug("SRIOBOOT - MASTER: "
				"Port [ %d ] is ready, now release slave's core ...\n",
				port);
			/*
			 * configure outbound window
			 * with maintenance attribute to set slave's LCSBA1CSR
			 */
			out_be32((void *)&srio->atmu.port[port - 1]
				.outbw[1].rowtar, 0);
			out_be32((void *)&srio->atmu.port[port - 1]
				.outbw[1].rowtear, 0);
			if (port - 1)
				out_be32((void *)&srio->atmu.port[port - 1]
					.outbw[1].rowbar,
					CONFIG_SYS_SRIO2_MEM_PHYS >> 12);
			else
				out_be32((void *)&srio->atmu.port[port - 1]
					.outbw[1].rowbar,
					CONFIG_SYS_SRIO1_MEM_PHYS >> 12);
			out_be32((void *)&srio->atmu.port[port - 1]
					.outbw[1].rowar,
					SRIO_OB_ATMU_AR_MAINT
					| atmu_size_mask(SRIO_MAINT_WIN_SIZE));

			/*
			 * configure outbound window
			 * with R/W attribute to set slave's BRR
			 */
			out_be32((void *)&srio->atmu.port[port - 1]
				.outbw[2].rowtar,
				SRIO_LCSBA1CSR >> 9);
			out_be32((void *)&srio->atmu.port[port - 1]
				.outbw[2].rowtear, 0);
			if (port - 1)
				out_be32((void *)&srio->atmu.port[port - 1]
					.outbw[2].rowbar,
					(CONFIG_SYS_SRIO2_MEM_PHYS
					+ SRIO_MAINT_WIN_SIZE) >> 12);
			else
				out_be32((void *)&srio->atmu.port[port - 1]
					.outbw[2].rowbar,
					(CONFIG_SYS_SRIO1_MEM_PHYS
					+ SRIO_MAINT_WIN_SIZE) >> 12);
			out_be32((void *)&srio->atmu.port[port - 1]
				.outbw[2].rowar,
				SRIO_OB_ATMU_AR_RW
				| atmu_size_mask(SRIO_RW_WIN_SIZE));

			/*
			 * Set the LCSBA1CSR register in slave
			 * by the maint-outbound window
			 */
			if (port - 1) {
				out_be32((void *)CONFIG_SYS_SRIO2_MEM_VIRT
					+ SRIO_LCSBA1CSR_OFFSET,
					SRIO_LCSBA1CSR);
				while (in_be32((void *)CONFIG_SYS_SRIO2_MEM_VIRT
					+ SRIO_LCSBA1CSR_OFFSET)
					!= SRIO_LCSBA1CSR)
					;
				/*
				 * And then set the BRR register
				 * to release slave core
				 */
				out_be32((void *)CONFIG_SYS_SRIO2_MEM_VIRT
					+ SRIO_MAINT_WIN_SIZE
					+ CONFIG_SRIO_PCIE_BOOT_BRR_OFFSET,
					CONFIG_SRIO_PCIE_BOOT_RELEASE_MASK);
			} else {
				out_be32((void *)CONFIG_SYS_SRIO1_MEM_VIRT
					+ SRIO_LCSBA1CSR_OFFSET,
					SRIO_LCSBA1CSR);
				while (in_be32((void *)CONFIG_SYS_SRIO1_MEM_VIRT
					+ SRIO_LCSBA1CSR_OFFSET)
					!= SRIO_LCSBA1CSR)
					;
				/*
				 * And then set the BRR register
				 * to release slave core
				 */
				out_be32((void *)CONFIG_SYS_SRIO1_MEM_VIRT
					+ SRIO_MAINT_WIN_SIZE
					+ CONFIG_SRIO_PCIE_BOOT_BRR_OFFSET,
					CONFIG_SRIO_PCIE_BOOT_RELEASE_MASK);
			}
			debug("SRIOBOOT - MASTER: "
					"Release slave successfully! Now the slave should start up!\n");
		}
	} else
		debug("SRIOBOOT - MASTER: Port [ %d ] is not ready.\n", port);
}
#endif
