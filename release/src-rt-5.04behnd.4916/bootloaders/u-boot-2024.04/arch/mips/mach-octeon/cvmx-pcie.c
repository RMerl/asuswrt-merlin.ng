// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Interface to PCIe as a host(RC) or target(EP)
 */

#include <log.h>
#include <linux/delay.h>
#include <linux/libfdt.h>

#include <mach/cvmx-regs.h>
#include <mach/octeon-model.h>
#include <mach/cvmx-fuse.h>
#include <mach/octeon-feature.h>
#include <mach/cvmx-qlm.h>
#include <mach/octeon_qlm.h>

#include <mach/cvmx-helper-fdt.h>

#include <mach/cvmx-regs.h>
#include <mach/octeon-model.h>
#include <mach/cvmx-fuse.h>
#include <mach/octeon-feature.h>
#include <mach/cvmx-qlm.h>
#include <mach/octeon_qlm.h>
#include <mach/cvmx-pcie.h>
#include <mach/cvmx-error.h>
#include <mach/cvmx-helper.h>
#include <mach/cvmx-helper-util.h>
#include <mach/cvmx-bgxx-defs.h>
#include <mach/cvmx-ciu-defs.h>
#include <mach/cvmx-gmxx-defs.h>
#include <mach/cvmx-gserx-defs.h>
#include <mach/cvmx-mio-defs.h>
#include <mach/cvmx-pciercx-defs.h>
#include <mach/cvmx-pcieepx-defs.h>
#include <mach/cvmx-pemx-defs.h>
#include <mach/cvmx-pexp-defs.h>
#include <mach/cvmx-rst-defs.h>
#include <mach/cvmx-sata-defs.h>
#include <mach/cvmx-sli-defs.h>
#include <mach/cvmx-sriomaintx-defs.h>
#include <mach/cvmx-sriox-defs.h>

#include <mach/cvmx-dpi-defs.h>
#include <mach/cvmx-sli-defs.h>
#include <mach/cvmx-dtx-defs.h>

DECLARE_GLOBAL_DATA_PTR;

#define MRRS_CN6XXX 3 /* 1024 byte Max Read Request Size */
#define MPS_CN6XXX  0 /* 128 byte Max Packet Size (Limit of most PCs) */

/* Endian swap mode. */
#define _CVMX_PCIE_ES 1

#define CVMX_READ_CSR(addr)		   csr_rd_node(node, addr)
#define CVMX_WRITE_CSR(addr, val)	   csr_wr_node(node, addr, val)
#define CVMX_PCIE_CFGX_READ(p, addr)	   cvmx_pcie_cfgx_read_node(node, p, addr)
#define CVMX_PCIE_CFGX_WRITE(p, addr, val) cvmx_pcie_cfgx_write_node(node, p, addr, val)

/* #define DEBUG_PCIE */

/* Delay after link up, before issuing first configuration read */
#define PCIE_DEVICE_READY_WAIT_DELAY_MICROSECONDS 700000

/* Recommended Preset Vector: Drop Preset 10    */
int pcie_preset_vec[4] = { 0x593, 0x593, 0x593, 0x593 };

/* Number of LTSSM transitions to record, must be a power of 2 */
#define LTSSM_HISTORY_SIZE 64
#define MAX_RETRIES	   2

bool pcie_link_initialized[CVMX_MAX_NODES][CVMX_PCIE_MAX_PORTS];
int cvmx_primary_pcie_bus_number = 1;

static uint32_t __cvmx_pcie_config_read32(int node, int pcie_port, int bus, int dev, int func,
					  int reg, int lst);

/**
 * Return the Core virtual base address for PCIe IO access. IOs are
 * read/written as an offset from this address.
 *
 * @param pcie_port PCIe port the IO is for
 *
 * Return: 64bit Octeon IO base address for read/write
 */
uint64_t cvmx_pcie_get_io_base_address(int pcie_port)
{
	cvmx_pcie_address_t pcie_addr;

	pcie_addr.u64 = 0;
	pcie_addr.io.upper = 0;
	pcie_addr.io.io = 1;
	pcie_addr.io.did = 3;
	pcie_addr.io.subdid = 2;
	pcie_addr.io.node = (pcie_port >> 4) & 0x3;
	pcie_addr.io.es = _CVMX_PCIE_ES;
	pcie_addr.io.port = (pcie_port & 0x3);
	return pcie_addr.u64;
}

/**
 * Size of the IO address region returned at address
 * cvmx_pcie_get_io_base_address()
 *
 * @param pcie_port PCIe port the IO is for
 *
 * Return: Size of the IO window
 */
uint64_t cvmx_pcie_get_io_size(int pcie_port)
{
	return 1ull << 32;
}

/**
 * Return the Core virtual base address for PCIe MEM access. Memory is
 * read/written as an offset from this address.
 *
 * @param pcie_port PCIe port the IO is for
 *
 * Return: 64bit Octeon IO base address for read/write
 */
uint64_t cvmx_pcie_get_mem_base_address(int pcie_port)
{
	cvmx_pcie_address_t pcie_addr;

	pcie_addr.u64 = 0;
	pcie_addr.mem.upper = 0;
	pcie_addr.mem.io = 1;
	pcie_addr.mem.did = 3;
	pcie_addr.mem.subdid = 3 + (pcie_port & 0x3);
	pcie_addr.mem.node = (pcie_port >> 4) & 0x3;
	return pcie_addr.u64;
}

/**
 * Size of the Mem address region returned at address
 * cvmx_pcie_get_mem_base_address()
 *
 * @param pcie_port PCIe port the IO is for
 *
 * Return: Size of the Mem window
 */
uint64_t cvmx_pcie_get_mem_size(int pcie_port)
{
	return 1ull << 36;
}

/**
 * @INTERNAL
 * Return the QLM number for the PCIE port.
 *
 * @param  pcie_port  QLM number to return for.
 *
 * Return: QLM number.
 */
static int __cvmx_pcie_get_qlm(int node, int pcie_port)
{
	if (OCTEON_IS_MODEL(OCTEON_CN73XX)) {
		cvmx_pemx_cfg_t pem_cfg;
		cvmx_pemx_qlm_t pem_qlm;
		cvmx_gserx_cfg_t gserx_cfg;

		switch (pcie_port) {
		case 0: /* PEM0 */
			gserx_cfg.u64 = CVMX_READ_CSR(CVMX_GSERX_CFG(0));
			if (gserx_cfg.s.pcie)
				return 0; /* PEM0 is on QLM0 and possibly QLM1 */
			else
				return -1; /* PEM0 is disabled */
		case 1:			   /* PEM1 */
			pem_cfg.u64 = CVMX_READ_CSR(CVMX_PEMX_CFG(0));
			gserx_cfg.u64 = CVMX_READ_CSR(CVMX_GSERX_CFG(1));
			if (!pem_cfg.cn78xx.lanes8 && gserx_cfg.s.pcie)
				return 1; /* PEM1 is on QLM 1 */
			else
				return -1; /* PEM1 is disabled */
		case 2:			   /* PEM2 */
			pem_qlm.u64 = CVMX_READ_CSR(CVMX_PEMX_QLM(2));
			if (pem_qlm.cn73xx.pemdlmsel == 1) {
				gserx_cfg.u64 = CVMX_READ_CSR(CVMX_GSERX_CFG(5));
				if (gserx_cfg.s.pcie)
					return 5; /* PEM2 is on DLM5 */
				else
					return -1; /* PEM2 is disabled */
			}
			gserx_cfg.u64 = CVMX_READ_CSR(CVMX_GSERX_CFG(2));
			if (gserx_cfg.s.pcie)
				return 2; /* PEM2 is on QLM2 and possibly QLM3 */
			else
				return -1; /* PEM2 is disabled */
		case 3:			   /* PEM3 */
			pem_qlm.u64 = CVMX_READ_CSR(CVMX_PEMX_QLM(3));
			if (pem_qlm.cn73xx.pemdlmsel == 1) {
				gserx_cfg.u64 = CVMX_READ_CSR(CVMX_GSERX_CFG(6));
				if (gserx_cfg.s.pcie)
					return 6; /* PEM2 is on DLM5 */
				else
					return -1; /* PEM2 is disabled */
			}
			pem_cfg.u64 = CVMX_READ_CSR(CVMX_PEMX_CFG(2));
			gserx_cfg.u64 = CVMX_READ_CSR(CVMX_GSERX_CFG(3));
			if (!pem_cfg.cn78xx.lanes8 && gserx_cfg.s.pcie)
				return 3; /* PEM2 is on QLM2 and possibly QLM3 */
			else
				return -1; /* PEM2 is disabled */
		default:
			printf("Invalid %d PCIe port\n", pcie_port);
			return -2;
		}
	} else if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		cvmx_pemx_cfg_t pem_cfg;
		cvmx_gserx_cfg_t gserx_cfg;

		switch (pcie_port) {
		case 0:
			gserx_cfg.u64 = CVMX_READ_CSR(CVMX_GSERX_CFG(0));
			if (gserx_cfg.s.pcie)
				return 0; /* PEM0 is on QLM0 and possibly QLM1 */
			else
				return -1; /* PEM0 is disabled */
		case 1:			   /* PEM1 */
			pem_cfg.u64 = CVMX_READ_CSR(CVMX_PEMX_CFG(0));
			gserx_cfg.u64 = CVMX_READ_CSR(CVMX_GSERX_CFG(1));
			if (!pem_cfg.cn78xx.lanes8 && gserx_cfg.s.pcie)
				return 1; /* PEM1 is on QLM 1 */
			else
				return -1; /* PEM1 is disabled */
		case 2:			   /* PEM2 */
			gserx_cfg.u64 = CVMX_READ_CSR(CVMX_GSERX_CFG(2));
			if (gserx_cfg.s.pcie)
				return 2; /* PEM2 is on QLM2 and possibly QLM3 */
			else
				return -1; /* PEM2 is disabled */
		case 3:			   /* PEM3 */
		{
			cvmx_gserx_cfg_t gser4_cfg;

			pem_cfg.u64 = CVMX_READ_CSR(CVMX_PEMX_CFG(2));
			gserx_cfg.u64 = CVMX_READ_CSR(CVMX_GSERX_CFG(3));
			gser4_cfg.u64 = CVMX_READ_CSR(CVMX_GSERX_CFG(4));
			if (pem_cfg.cn78xx.lanes8) {
				if (gser4_cfg.s.pcie)
					return 4; /* PEM3 is on QLM4 */
				else
					return -1; /* PEM3 is disabled */
			} else {
				if (gserx_cfg.s.pcie)
					return 3; /* PEM3 is on QLM3 */
				else if (gser4_cfg.s.pcie)
					return 4; /* PEM3 is on QLM4 */
				else
					return -1; /* PEM3 is disabled */
			}
		}
		default:
			printf("Invalid %d PCIe port\n", pcie_port);
			return -1;
		}
	} else if (OCTEON_IS_MODEL(OCTEON_CN70XX)) {
		enum cvmx_qlm_mode mode1 = cvmx_qlm_get_mode(1);
		enum cvmx_qlm_mode mode2 = cvmx_qlm_get_mode(2);

		switch (pcie_port) {
		case 0: /* PCIe0 can be DLM1 with 1, 2 or 4 lanes */
			if (mode1 == CVMX_QLM_MODE_PCIE ||     /* Using DLM 1-2 */
			    mode1 == CVMX_QLM_MODE_PCIE_1X2 || /* Using DLM 1 */
			    mode1 == CVMX_QLM_MODE_PCIE_2X1 || /* Using DLM 1, lane 0 */
			    mode1 == CVMX_QLM_MODE_PCIE_1X1) /* Using DLM 1, l0, l1 not used */
				return 1;
			else
				return -1;
		case 1: /* PCIe1 can be DLM1 1 lane(1), DLM2 1 lane(0) or 2 lanes(0-1) */
			if (mode1 == CVMX_QLM_MODE_PCIE_2X1)
				return 1;
			else if (mode2 == CVMX_QLM_MODE_PCIE_1X2)
				return 2;
			else if (mode2 == CVMX_QLM_MODE_PCIE_2X1)
				return 2;
			else
				return -1;
		case 2: /* PCIe2 can be DLM2 1 lanes(1) */
			if (mode2 == CVMX_QLM_MODE_PCIE_2X1)
				return 2;
			else
				return -1;
		default: /* Only three PEM blocks */
			return -1;
		}
	} else if (OCTEON_IS_MODEL(OCTEON_CNF75XX)) {
		cvmx_gserx_cfg_t gserx_cfg;

		switch (pcie_port) {
		case 0: /* PEM0 */
			gserx_cfg.u64 = CVMX_READ_CSR(CVMX_GSERX_CFG(0));
			if (gserx_cfg.s.pcie)
				return 0; /* PEM0 is on QLM0 and possibly QLM1 */
			else
				return -1; /* PEM0 is disabled */
		case 1:			   /* PEM1 */
			gserx_cfg.u64 = CVMX_READ_CSR(CVMX_GSERX_CFG(1));
			if (gserx_cfg.s.pcie)
				return 1; /* PEM1 is on DLM1 */
			else
				return -1; /* PEM1 is disabled */
		default:
			return -1;
		}
	}
	return -1;
}

/**
 * @INTERNAL
 * Initialize the RC config space CSRs
 *
 * @param node      node
 * @param pcie_port PCIe port to initialize
 */
static void __cvmx_pcie_rc_initialize_config_space(int node, int pcie_port)
{
	/* Max Payload Size (PCIE*_CFG030[MPS]) */
	/* Max Read Request Size (PCIE*_CFG030[MRRS]) */
	/* Relaxed-order, no-snoop enables (PCIE*_CFG030[RO_EN,NS_EN] */
	/* Error Message Enables (PCIE*_CFG030[CE_EN,NFE_EN,FE_EN,UR_EN]) */
	{
		cvmx_pciercx_cfg030_t pciercx_cfg030;

		pciercx_cfg030.u32 = CVMX_PCIE_CFGX_READ(pcie_port,
							 CVMX_PCIERCX_CFG030(pcie_port));
		pciercx_cfg030.s.mps = MPS_CN6XXX;
		pciercx_cfg030.s.mrrs = MRRS_CN6XXX;
		/*
		 * Enable relaxed order processing. This will allow devices
		 * to affect read response ordering
		 */
		pciercx_cfg030.s.ro_en = 1;
		/* Enable no snoop processing. Not used by Octeon */
		pciercx_cfg030.s.ns_en = 1;
		/* Correctable error reporting enable. */
		pciercx_cfg030.s.ce_en = 1;
		/* Non-fatal error reporting enable. */
		pciercx_cfg030.s.nfe_en = 1;
		/* Fatal error reporting enable. */
		pciercx_cfg030.s.fe_en = 1;
		/* Unsupported request reporting enable. */
		pciercx_cfg030.s.ur_en = 1;
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG030(pcie_port),
				     pciercx_cfg030.u32);
	}

	/*
	 * Max Payload Size (DPI_SLI_PRTX_CFG[MPS]) must match
	 * PCIE*_CFG030[MPS]
	 */
	/*
	 * Max Read Request Size (DPI_SLI_PRTX_CFG[MRRS]) must not exceed
	 * PCIE*_CFG030[MRRS]
	 */
	cvmx_dpi_sli_prtx_cfg_t prt_cfg;
	cvmx_sli_s2m_portx_ctl_t sli_s2m_portx_ctl;

	prt_cfg.u64 = CVMX_READ_CSR(CVMX_DPI_SLI_PRTX_CFG(pcie_port));
	prt_cfg.s.mps = MPS_CN6XXX;
	prt_cfg.s.mrrs = MRRS_CN6XXX;
	/* Max outstanding load request. */
	prt_cfg.s.molr = 32;
	CVMX_WRITE_CSR(CVMX_DPI_SLI_PRTX_CFG(pcie_port), prt_cfg.u64);

	sli_s2m_portx_ctl.u64 = CVMX_READ_CSR(CVMX_PEXP_SLI_S2M_PORTX_CTL(pcie_port));
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) ||
	      OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		sli_s2m_portx_ctl.cn61xx.mrrs = MRRS_CN6XXX;
	CVMX_WRITE_CSR(CVMX_PEXP_SLI_S2M_PORTX_CTL(pcie_port), sli_s2m_portx_ctl.u64);

	/* ECRC Generation (PCIE*_CFG070[GE,CE]) */
	{
		cvmx_pciercx_cfg070_t pciercx_cfg070;

		pciercx_cfg070.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG070(pcie_port));
		pciercx_cfg070.s.ge = 1; /* ECRC generation enable. */
		pciercx_cfg070.s.ce = 1; /* ECRC check enable. */
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG070(pcie_port), pciercx_cfg070.u32);
	}

	/* Access Enables (PCIE*_CFG001[MSAE,ME]) */
	/* ME and MSAE should always be set. */
	/* Interrupt Disable (PCIE*_CFG001[I_DIS]) */
	/* System Error Message Enable (PCIE*_CFG001[SEE]) */
	{
		cvmx_pciercx_cfg001_t pciercx_cfg001;

		pciercx_cfg001.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG001(pcie_port));
		pciercx_cfg001.s.msae = 1;  /* Memory space enable. */
		pciercx_cfg001.s.me = 1;    /* Bus master enable. */
		pciercx_cfg001.s.i_dis = 1; /* INTx assertion disable. */
		pciercx_cfg001.s.see = 1;   /* SERR# enable */
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG001(pcie_port), pciercx_cfg001.u32);
	}

	/* Advanced Error Recovery Message Enables */
	/* (PCIE*_CFG066,PCIE*_CFG067,PCIE*_CFG069) */
	CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG066(pcie_port), 0);
	/* Use CVMX_PCIERCX_CFG067 hardware default */
	CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG069(pcie_port), 0);

	/* Active State Power Management (PCIE*_CFG032[ASLPC]) */
	{
		cvmx_pciercx_cfg032_t pciercx_cfg032;

		pciercx_cfg032.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG032(pcie_port));
		pciercx_cfg032.s.aslpc = 0; /* Active state Link PM control. */
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG032(pcie_port), pciercx_cfg032.u32);
	}

	/* Link Width Mode (PCIERCn_CFG452[LME]) - Set during
	 * cvmx_pcie_rc_initialize_link()
	 */
	/* Primary Bus Number (PCIERCn_CFG006[PBNUM]) */
	{
		/* We set the primary bus number to 1 so IDT bridges are happy.
		 * They don't like zero
		 */
		cvmx_pciercx_cfg006_t pciercx_cfg006;

		pciercx_cfg006.u32 = 0;
		pciercx_cfg006.s.pbnum = cvmx_primary_pcie_bus_number;
		pciercx_cfg006.s.sbnum = cvmx_primary_pcie_bus_number;
		pciercx_cfg006.s.subbnum = cvmx_primary_pcie_bus_number;
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG006(pcie_port), pciercx_cfg006.u32);
	}

	/* Memory-mapped I/O BAR (PCIERCn_CFG008) */
	/* Most applications should disable the memory-mapped I/O BAR by */
	/* setting PCIERCn_CFG008[ML_ADDR] < PCIERCn_CFG008[MB_ADDR] */
	{
		cvmx_pciercx_cfg008_t pciercx_cfg008;

		pciercx_cfg008.u32 = 0;
		pciercx_cfg008.s.mb_addr = 0x100;
		pciercx_cfg008.s.ml_addr = 0;
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG008(pcie_port), pciercx_cfg008.u32);
	}

	/* Prefetchable BAR (PCIERCn_CFG009,PCIERCn_CFG010,PCIERCn_CFG011) */
	/* Most applications should disable the prefetchable BAR by setting */
	/* PCIERCn_CFG011[UMEM_LIMIT],PCIERCn_CFG009[LMEM_LIMIT] < */
	/* PCIERCn_CFG010[UMEM_BASE],PCIERCn_CFG009[LMEM_BASE] */
	{
		cvmx_pciercx_cfg009_t pciercx_cfg009;
		cvmx_pciercx_cfg010_t pciercx_cfg010;
		cvmx_pciercx_cfg011_t pciercx_cfg011;

		pciercx_cfg009.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG009(pcie_port));
		pciercx_cfg010.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG010(pcie_port));
		pciercx_cfg011.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG011(pcie_port));
		pciercx_cfg009.s.lmem_base = 0x100;
		pciercx_cfg009.s.lmem_limit = 0;
		pciercx_cfg010.s.umem_base = 0x100;
		pciercx_cfg011.s.umem_limit = 0;
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG009(pcie_port), pciercx_cfg009.u32);
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG010(pcie_port), pciercx_cfg010.u32);
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG011(pcie_port), pciercx_cfg011.u32);
	}

	/* System Error Interrupt Enables (PCIERCn_CFG035[SECEE,SEFEE,SENFEE]) */
	/* PME Interrupt Enables (PCIERCn_CFG035[PMEIE]) */
	{
		cvmx_pciercx_cfg035_t pciercx_cfg035;

		pciercx_cfg035.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG035(pcie_port));
		pciercx_cfg035.s.secee = 1;  /* System error on correctable error enable. */
		pciercx_cfg035.s.sefee = 1;  /* System error on fatal error enable. */
		pciercx_cfg035.s.senfee = 1; /* System error on non-fatal error enable. */
		pciercx_cfg035.s.pmeie = 1;  /* PME interrupt enable. */
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG035(pcie_port), pciercx_cfg035.u32);
	}

	/* Advanced Error Recovery Interrupt Enables */
	/* (PCIERCn_CFG075[CERE,NFERE,FERE]) */
	{
		cvmx_pciercx_cfg075_t pciercx_cfg075;

		pciercx_cfg075.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG075(pcie_port));
		pciercx_cfg075.s.cere = 1;  /* Correctable error reporting enable. */
		pciercx_cfg075.s.nfere = 1; /* Non-fatal error reporting enable. */
		pciercx_cfg075.s.fere = 1;  /* Fatal error reporting enable. */
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG075(pcie_port), pciercx_cfg075.u32);
	}

	/* HP Interrupt Enables (PCIERCn_CFG034[HPINT_EN], */
	/* PCIERCn_CFG034[DLLS_EN,CCINT_EN]) */
	{
		cvmx_pciercx_cfg034_t pciercx_cfg034;

		pciercx_cfg034.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG034(pcie_port));
		pciercx_cfg034.s.hpint_en = 1; /* Hot-plug interrupt enable. */
		pciercx_cfg034.s.dlls_en = 1;  /* Data Link Layer state changed enable */
		pciercx_cfg034.s.ccint_en = 1; /* Command completed interrupt enable. */
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG034(pcie_port), pciercx_cfg034.u32);
	}

	if (OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) ||
	    OCTEON_IS_MODEL(OCTEON_CNF75XX)) {
		int qlm = __cvmx_pcie_get_qlm(node, pcie_port);
		int speed = cvmx_qlm_get_gbaud_mhz(qlm);
		cvmx_pemx_cfg_t pem_cfg;
		cvmx_pciercx_cfg031_t cfg031;
		cvmx_pciercx_cfg040_t cfg040;
		cvmx_pciercx_cfg452_t cfg452;
		cvmx_pciercx_cfg089_t cfg089;
		cvmx_pciercx_cfg090_t cfg090;
		cvmx_pciercx_cfg091_t cfg091;
		cvmx_pciercx_cfg092_t cfg092;
		cvmx_pciercx_cfg554_t cfg554;

		/*
		 * Make sure the PEM agrees with GSERX about the speed
		 * its going to try
		 */
		switch (speed) {
		case 2500: /* Gen1 */
			pem_cfg.u64 = CVMX_READ_CSR(CVMX_PEMX_CFG(pcie_port));
			pem_cfg.s.md = 0;
			CVMX_WRITE_CSR(CVMX_PEMX_CFG(pcie_port), pem_cfg.u64);

			/* Set the target link speed */
			cfg040.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG040(pcie_port));
			cfg040.s.tls = 1;
			CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG040(pcie_port), cfg040.u32);
			break;
		case 5000: /* Gen2 */
			pem_cfg.u64 = CVMX_READ_CSR(CVMX_PEMX_CFG(pcie_port));
			pem_cfg.s.md = 1;
			CVMX_WRITE_CSR(CVMX_PEMX_CFG(pcie_port), pem_cfg.u64);

			/* Set the target link speed */
			cfg040.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG040(pcie_port));
			cfg040.s.tls = 2;
			CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG040(pcie_port), cfg040.u32);
			break;
		case 8000: /* Gen3 */
			pem_cfg.u64 = CVMX_READ_CSR(CVMX_PEMX_CFG(pcie_port));
			pem_cfg.s.md = 2;
			CVMX_WRITE_CSR(CVMX_PEMX_CFG(pcie_port), pem_cfg.u64);

			/* Set the target link speed */
			cfg040.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG040(pcie_port));
			cfg040.s.tls = 3;
			CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG040(pcie_port), cfg040.u32);
			break;
		default:
			break;
		}

		/* Link Width Mode (PCIERCn_CFG452[LME]) */
		pem_cfg.u64 = CVMX_READ_CSR(CVMX_PEMX_CFG(pcie_port));
		cfg452.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG452(pcie_port));
		if (qlm >= 5)
			cfg452.s.lme = 0x3;
		else
			cfg452.s.lme = (pem_cfg.cn78xx.lanes8) ? 0xf : 0x7;
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG452(pcie_port), cfg452.u32);

		/* Errata PEM-25990 - Disable ASLPMS */
		cfg031.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG031(pcie_port));
		cfg031.s.aslpms = 0;
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG031(pcie_port), cfg031.u32);

		/* CFG554.PRV default changed from 16'h7ff to 16'h593. */
		cfg554.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG554(pcie_port));
		cfg554.s.prv = pcie_preset_vec[pcie_port];
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG554(pcie_port), cfg554.u32);
		/* Errata PEM-26189 - Disable the 2ms timer on all chips */
		cfg554.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG554(pcie_port));
		cfg554.s.p23td = 1;
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG554(pcie_port), cfg554.u32);

		/* Errata PEM-21178 - Change the CFG[089-092] LxUTP & LxDTP defaults. */
		cfg089.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG089(pcie_port));
		cfg089.s.l1ddtp = 7;
		cfg089.s.l1utp = 7;
		cfg089.s.l0dtp = 7;
		cfg089.s.l0utp = 7;
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG089(pcie_port), cfg089.u32);
		cfg090.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG090(pcie_port));
		cfg090.s.l3dtp = 7;
		cfg090.s.l3utp = 7;
		cfg090.s.l2dtp = 7;
		cfg090.s.l2utp = 7;
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG090(pcie_port), cfg090.u32);
		cfg091.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG091(pcie_port));
		cfg091.s.l5dtp = 7;
		cfg091.s.l5utp = 7;
		cfg091.s.l4dtp = 7;
		cfg091.s.l4utp = 7;
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG091(pcie_port), cfg091.u32);
		cfg092.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG092(pcie_port));
		cfg092.s.l7dtp = 7;
		cfg092.s.l7utp = 7;
		cfg092.s.l6dtp = 7;
		cfg092.s.l6utp = 7;
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG092(pcie_port), cfg092.u32);
	}
}

static void __cvmx_increment_ba(cvmx_sli_mem_access_subidx_t *pmas)
{
	if (OCTEON_IS_MODEL(OCTEON_CN68XX))
		pmas->cn68xx.ba++;
	else
		pmas->cn63xx.ba++;
}

/*
 * milliseconds to retry PCIe cfg-space access:
 * Value 32(unscaled) was recommended in HRM, but may be too small for
 * some PCIe devices. This 200mS default should cover most devices,
 * but can be extended by bootparam cvmx-pcie.cfg_timeout, or reduced
 * to speed boot if it is known that no devices need so much time.
 */
static int cfg_timeout = 200;

static int cfg_retries(void)
{
	static int cfg_ticks = -1;

	if (cfg_ticks < 0) {
		u64 nS = cfg_timeout * 1000000;
		const int ceiling = 0xffff;

		cfg_ticks = nS / (gd->bus_clk >> 16);
		if (cfg_ticks > ceiling)
			cfg_ticks = ceiling;
	}

	return cfg_ticks;
}

/**
 * @INTERNAL
 * Enable/Disable PEMX_PEMON.pemon based on the direction.
 *
 * @param node      node
 * @param pcie_port PCIe port
 * @param direction 0 to disable, 1 to enable
 */
static void __cvmx_pcie_config_pemon(int node, int pcie_port, bool direction)
{
	cvmx_pemx_on_t pemon;

	pemon.u64 = CVMX_READ_CSR(CVMX_PEMX_ON(pcie_port));
	pemon.s.pemon = direction;
	CVMX_WRITE_CSR(CVMX_PEMX_ON(pcie_port), pemon.u64);
	pemon.u64 = CVMX_READ_CSR(CVMX_PEMX_ON(pcie_port));
}

/**
 * @INTERNAL
 * De-assert GSER_PHY.phy_reset for a given qlm
 *
 * @param node       node
 * @param qlm        qlm for a given PCIe port
 */
static void __cvmx_pcie_gser_phy_config(int node, int pcie_port, int qlm)
{
	cvmx_pemx_cfg_t pem_cfg;
	cvmx_gserx_phy_ctl_t ctrl;
	int has_8lanes = 0;
	int is_gen3 = 0;

	ctrl.u64 = CVMX_READ_CSR(CVMX_GSERX_PHY_CTL(qlm));

	/* Assert the reset */
	ctrl.s.phy_reset = 1;
	CVMX_WRITE_CSR(CVMX_GSERX_PHY_CTL(qlm), ctrl.u64);
	pem_cfg.u64 = CVMX_READ_CSR(CVMX_PEMX_CFG(pcie_port));
	udelay(10);

	has_8lanes = pem_cfg.cn78xx.lanes8;
	is_gen3 = pem_cfg.cn78xx.md >= 2;

	if (has_8lanes) {
		ctrl.u64 = CVMX_READ_CSR(CVMX_GSERX_PHY_CTL(qlm + 1));
		ctrl.s.phy_reset = 1;
		CVMX_WRITE_CSR(CVMX_GSERX_PHY_CTL(qlm + 1), ctrl.u64);
		ctrl.u64 = CVMX_READ_CSR(CVMX_GSERX_PHY_CTL(qlm + 1));
	}
	ctrl.u64 = CVMX_READ_CSR(CVMX_GSERX_PHY_CTL(qlm));
	udelay(10);

	/* Deassert the reset */
	ctrl.s.phy_reset = 0;
	CVMX_WRITE_CSR(CVMX_GSERX_PHY_CTL(qlm), ctrl.u64);
	pem_cfg.u64 = CVMX_READ_CSR(CVMX_PEMX_CFG(pcie_port));
	udelay(500);

	if (has_8lanes) {
		ctrl.u64 = CVMX_READ_CSR(CVMX_GSERX_PHY_CTL(qlm + 1));
		ctrl.s.phy_reset = 0;
		CVMX_WRITE_CSR(CVMX_GSERX_PHY_CTL(qlm + 1), ctrl.u64);
	}
	ctrl.u64 = CVMX_READ_CSR(CVMX_GSERX_PHY_CTL(qlm));
	udelay(500);

	/* Apply some erratas after PHY reset, only applies to PCIe GEN3 */
	if (is_gen3) {
		int i;
		int high_qlm = has_8lanes ? qlm + 1 : qlm;

		/* Apply workaround for Errata GSER-26150 */
		if (OCTEON_IS_MODEL(OCTEON_CN73XX_PASS1_0)) {
			for (i = qlm; i < high_qlm; i++) {
				cvmx_gserx_glbl_pll_cfg_3_t pll_cfg_3;
				cvmx_gserx_glbl_misc_config_1_t misc_config_1;
				/* Update PLL parameters */
				/*
				 * Step 1: Set
				 * GSER()_GLBL_PLL_CFG_3[PLL_VCTRL_SEL_LCVCO_VAL] = 0x2,
				 * and
				 * GSER()_GLBL_PLL_CFG_3[PCS_SDS_PLL_VCO_AMP] = 0
				 */
				pll_cfg_3.u64 = CVMX_READ_CSR(CVMX_GSERX_GLBL_PLL_CFG_3(i));
				pll_cfg_3.s.pcs_sds_pll_vco_amp = 0;
				pll_cfg_3.s.pll_vctrl_sel_lcvco_val = 2;
				CVMX_WRITE_CSR(CVMX_GSERX_GLBL_PLL_CFG_3(i), pll_cfg_3.u64);

				/*
				 * Step 2: Set
				 * GSER()_GLBL_MISC_CONFIG_1[PCS_SDS_TRIM_CHP_REG] = 0x2.
				 */
				misc_config_1.u64 = CVMX_READ_CSR(CVMX_GSERX_GLBL_MISC_CONFIG_1(i));
				misc_config_1.s.pcs_sds_trim_chp_reg = 2;
				CVMX_WRITE_CSR(CVMX_GSERX_GLBL_MISC_CONFIG_1(i), misc_config_1.u64);
			}
		}

		/* Apply workaround for Errata GSER-25992 */
		if (OCTEON_IS_MODEL(OCTEON_CN73XX_PASS1_X) ||
		    OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)) {
			for (i = qlm; i < high_qlm; i++)
				cvmx_qlm_gser_errata_25992(node, i);
		}
	}
}

/* Get the PCIe LTSSM state for the given port
 *
 * @param node      Node to query
 * @param pcie_port PEM to query
 *
 * Return: LTSSM state
 */
static int __cvmx_pcie_rc_get_ltssm_state(int node, int pcie_port)
{
	u64 debug;

	if (OCTEON_IS_MODEL(OCTEON_CN73XX) && pcie_port == 0) {
		CVMX_WRITE_CSR(CVMX_DTX_SPEM_SELX(0), 0);
		CVMX_READ_CSR(CVMX_DTX_SPEM_SELX(0));
		CVMX_WRITE_CSR(CVMX_DTX_SPEM_ENAX(0), 0xfffffffffull);
		CVMX_READ_CSR(CVMX_DTX_SPEM_ENAX(0));

		/* Read the value */
		debug = CVMX_READ_CSR(CVMX_DTX_SPEM_DATX(0));

		/* Disable the PEM from driving OCLA signals */
		CVMX_WRITE_CSR(CVMX_DTX_SPEM_ENAX(0), 0);
		CVMX_READ_CSR(CVMX_DTX_SPEM_ENAX(0));
	} else {
		/* LTSSM state is in debug select 0 */
		CVMX_WRITE_CSR(CVMX_DTX_PEMX_SELX(0, pcie_port), 0);
		CVMX_READ_CSR(CVMX_DTX_PEMX_SELX(0, pcie_port));
		CVMX_WRITE_CSR(CVMX_DTX_PEMX_ENAX(0, pcie_port), 0xfffffffffull);
		CVMX_READ_CSR(CVMX_DTX_PEMX_ENAX(0, pcie_port));

		/* Read the value */
		debug = CVMX_READ_CSR(CVMX_DTX_PEMX_DATX(0, pcie_port));

		/* Disable the PEM from driving OCLA signals */
		CVMX_WRITE_CSR(CVMX_DTX_PEMX_ENAX(0, pcie_port), 0);
		CVMX_READ_CSR(CVMX_DTX_PEMX_ENAX(0, pcie_port));
	}

	/* DBGSEL = 0x0, bits[8:3] */
	return cvmx_bit_extract(debug, 3, 6);
}

/**
 * Get the PCIe LTSSM state for the given port
 *
 * @param node      Node to query
 * @param pcie_port PEM to query
 *
 * Return: LTSSM state
 */
static const char *cvmx_pcie_get_ltssm_string(int ltssm)
{
	switch (ltssm) {
	case 0x00:
		return "DETECT_QUIET";
	case 0x01:
		return "DETECT_ACT";
	case 0x02:
		return "POLL_ACTIVE";
	case 0x03:
		return "POLL_COMPLIANCE";
	case 0x04:
		return "POLL_CONFIG";
	case 0x05:
		return "PRE_DETECT_QUIET";
	case 0x06:
		return "DETECT_WAIT";
	case 0x07:
		return "CFG_LINKWD_START";
	case 0x08:
		return "CFG_LINKWD_ACEPT";
	case 0x09:
		return "CFG_LANENUM_WAIT";
	case 0x0A:
		return "CFG_LANENUM_ACEPT";
	case 0x0B:
		return "CFG_COMPLETE";
	case 0x0C:
		return "CFG_IDLE";
	case 0x0D:
		return "RCVRY_LOCK";
	case 0x0E:
		return "RCVRY_SPEED";
	case 0x0F:
		return "RCVRY_RCVRCFG";
	case 0x10:
		return "RCVRY_IDLE";
	case 0x11:
		return "L0";
	case 0x12:
		return "L0S";
	case 0x13:
		return "L123_SEND_EIDLE";
	case 0x14:
		return "L1_IDLE";
	case 0x15:
		return "L2_IDLE";
	case 0x16:
		return "L2_WAKE";
	case 0x17:
		return "DISABLED_ENTRY";
	case 0x18:
		return "DISABLED_IDLE";
	case 0x19:
		return "DISABLED";
	case 0x1A:
		return "LPBK_ENTRY";
	case 0x1B:
		return "LPBK_ACTIVE";
	case 0x1C:
		return "LPBK_EXIT";
	case 0x1D:
		return "LPBK_EXIT_TIMEOUT";
	case 0x1E:
		return "HOT_RESET_ENTRY";
	case 0x1F:
		return "HOT_RESET";
	case 0x20:
		return "RCVRY_EQ0";
	case 0x21:
		return "RCVRY_EQ1";
	case 0x22:
		return "RCVRY_EQ2";
	case 0x23:
		return "RCVRY_EQ3";
	default:
		return "Unknown";
	}
}

/**
 * During PCIe link initialization we need to make config request to the attached
 * device to verify its speed and width. These config access happen very early
 * after the device is taken out of reset, so may fail for some amount of time.
 * This function automatically retries these config accesses. The normal builtin
 * hardware retry isn't enough for this very early access.
 *
 * @param node      Note to read from
 * @param pcie_port PCIe port to read from
 * @param bus       PCIe bus number
 * @param dev       PCIe device
 * @param func      PCIe function on the device
 * @param reg       Register to read
 *
 * Return: Config register value, or all ones on failure
 */
static uint32_t cvmx_pcie_config_read32_retry(int node, int pcie_port, int bus, int dev, int func,
					      int reg)
{
	/*
	 * Read the PCI config register until we get a valid value. Some cards
	 * require time after link up to return data. Wait at most 3 seconds
	 */
	u64 timeout = 300;
	u32 val;

	do {
		/* Read PCI capability pointer */
		val = __cvmx_pcie_config_read32(node, pcie_port, bus, dev, func, reg, 0);

		/* Check the read succeeded */
		if (val != 0xffffffff)
			return val;
		/* Failed, wait a little and try again */
		mdelay(10);
	} while (--timeout);

	debug("N%d.PCIe%d: Config read failed, can't communicate with device\n",
	      node, pcie_port);

	return -1;
}

/**
 * @INTERNAL
 * Initialize a host mode PCIe gen 2 link. This function takes a PCIe
 * port from reset to a link up state. Software can then begin
 * configuring the rest of the link.
 *
 * @param node	    node
 * @param pcie_port PCIe port to initialize
 *
 * Return: Zero on success
 */
static int __cvmx_pcie_rc_initialize_link_gen2(int node, int pcie_port)
{
	u64 start_cycle;

	cvmx_pemx_ctl_status_t pem_ctl_status;
	cvmx_pciercx_cfg032_t pciercx_cfg032;
	cvmx_pciercx_cfg448_t pciercx_cfg448;

	if (OCTEON_IS_OCTEON3()) {
		if (CVMX_WAIT_FOR_FIELD64_NODE(node, CVMX_PEMX_ON(pcie_port), cvmx_pemx_on_t,
					       pemoor, ==, 1, 100000)) {
			printf("%d:PCIe: Port %d PEM not on, skipping\n", node, pcie_port);
			return -1;
		}
	}

	/* Bring up the link */
	pem_ctl_status.u64 = CVMX_READ_CSR(CVMX_PEMX_CTL_STATUS(pcie_port));
	pem_ctl_status.s.lnk_enb = 1;
	CVMX_WRITE_CSR(CVMX_PEMX_CTL_STATUS(pcie_port), pem_ctl_status.u64);

	/* Wait for the link to come up */
	start_cycle = get_timer(0);
	do {
		if (get_timer(start_cycle) > 1000)
			return -1;

		udelay(1000);
		pciercx_cfg032.u32 = CVMX_PCIE_CFGX_READ(pcie_port,
							 CVMX_PCIERCX_CFG032(pcie_port));
	} while ((pciercx_cfg032.s.dlla == 0) || (pciercx_cfg032.s.lt == 1));

	/* Update the Replay Time Limit.  Empirically, some PCIe devices take a
	 * little longer to respond than expected under load. As a workaround
	 * for this we configure the Replay Time Limit to the value expected
	 * for a 512 byte MPS instead of our actual 256 byte MPS. The numbers
	 * below are directly from the PCIe spec table 3-4
	 */
	pciercx_cfg448.u32 = CVMX_PCIE_CFGX_READ(pcie_port,
						 CVMX_PCIERCX_CFG448(pcie_port));
	switch (pciercx_cfg032.s.nlw) {
	case 1: /* 1 lane */
		pciercx_cfg448.s.rtl = 1677;
		break;
	case 2: /* 2 lanes */
		pciercx_cfg448.s.rtl = 867;
		break;
	case 4: /* 4 lanes */
		pciercx_cfg448.s.rtl = 462;
		break;
	case 8: /* 8 lanes */
		pciercx_cfg448.s.rtl = 258;
		break;
	}
	CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG448(pcie_port),
			     pciercx_cfg448.u32);

	return 0;
}

extern int octeon_pcie_get_qlm_from_fdt(int numa_node, int pcie_port);

static int __cvmx_pcie_check_pcie_port(int node, int pcie_port, enum cvmx_qlm_mode mode)
{
	if (mode == CVMX_QLM_MODE_SRIO_1X4 || mode == CVMX_QLM_MODE_SRIO_2X2 ||
	    mode == CVMX_QLM_MODE_SRIO_4X1) {
		printf("%d:PCIe: Port %d is SRIO, skipping.\n", node, pcie_port);
		return -1;
	} else if (mode == CVMX_QLM_MODE_SGMII) {
		printf("%d:PCIe: Port %d is SGMII, skipping.\n", node, pcie_port);
		return -1;
	} else if (mode == CVMX_QLM_MODE_XAUI || mode == CVMX_QLM_MODE_RXAUI) {
		printf("%d:PCIe: Port %d is XAUI, skipping.\n", node, pcie_port);
		return -1;
	} else if (mode == CVMX_QLM_MODE_ILK) {
		printf("%d:PCIe: Port %d is ILK, skipping.\n", node, pcie_port);
		return -1;
	} else if (mode != CVMX_QLM_MODE_PCIE &&
		   mode != CVMX_QLM_MODE_PCIE_1X8 &&
		   mode != CVMX_QLM_MODE_PCIE_1X2 &&
		   mode != CVMX_QLM_MODE_PCIE_2X1 &&
		   mode != CVMX_QLM_MODE_PCIE_1X1) {
		printf("%d:PCIe: Port %d is unknown, skipping.\n",
		       node, pcie_port);
		return -1;
	}
	return 0;
}

static int __cvmx_pcie_check_qlm_mode(int node, int pcie_port, int qlm)
{
	enum cvmx_qlm_mode mode = CVMX_QLM_MODE_DISABLED;

	if (qlm < 0)
		return -1;

	/* Make sure this interface is PCIe */
	if (OCTEON_IS_MODEL(OCTEON_CN70XX)) {
		if (cvmx_qlm_get_dlm_mode(1, pcie_port) ==
		    CVMX_QLM_MODE_DISABLED) {
			printf("PCIe: Port %d not in PCIe mode, skipping\n",
			       pcie_port);
			return -1;
		}
	} else if (octeon_has_feature(OCTEON_FEATURE_PCIE)) {
		/*
		 * Requires reading the MIO_QLMX_CFG register to figure
		 * out the port type.
		 */
		if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
			qlm = 3 - (pcie_port * 2);
		} else if (OCTEON_IS_MODEL(OCTEON_CN61XX)) {
			cvmx_mio_qlmx_cfg_t qlm_cfg;

			qlm_cfg.u64 = csr_rd(CVMX_MIO_QLMX_CFG(1));
			if (qlm_cfg.s.qlm_cfg == 1)
				qlm = 1;
			else
				qlm = pcie_port;
		} else if (OCTEON_IS_MODEL(OCTEON_CN66XX) ||
			   OCTEON_IS_MODEL(OCTEON_CN63XX)) {
			qlm = pcie_port;
		}

		/*
		 * PCIe is allowed only in QLM1, 1 PCIe port in x2 or
		 * 2 PCIe ports in x1
		 */
		else if (OCTEON_IS_MODEL(OCTEON_CNF71XX))
			qlm = 1;

		mode = cvmx_qlm_get_mode(qlm);

		__cvmx_pcie_check_pcie_port(node, pcie_port, mode);
	}
	return 0;
}

static void __cvmx_pcie_sli_config(int node, int pcie_port)
{
	cvmx_pemx_bar_ctl_t pemx_bar_ctl;
	cvmx_pemx_ctl_status_t pemx_ctl_status;
	cvmx_sli_ctl_portx_t sli_ctl_portx;
	cvmx_sli_mem_access_ctl_t sli_mem_access_ctl;
	cvmx_sli_mem_access_subidx_t mem_access_subid;
	cvmx_pemx_bar1_indexx_t bar1_index;
	int i;

	/* Store merge control (SLI_MEM_ACCESS_CTL[TIMER,MAX_WORD]) */
	sli_mem_access_ctl.u64 = CVMX_READ_CSR(CVMX_PEXP_SLI_MEM_ACCESS_CTL);
	sli_mem_access_ctl.s.max_word = 0; /* Allow 16 words to combine */
	sli_mem_access_ctl.s.timer = 127;  /* Wait up to 127 cycles for more data */
	CVMX_WRITE_CSR(CVMX_PEXP_SLI_MEM_ACCESS_CTL, sli_mem_access_ctl.u64);

	/* Setup Mem access SubDIDs */
	mem_access_subid.u64 = 0;
	mem_access_subid.s.port = pcie_port;	/* Port the request is sent to. */
	mem_access_subid.s.nmerge = 0;		/* Allow merging as it works on CN6XXX. */
	mem_access_subid.s.esr = _CVMX_PCIE_ES; /* Endian-swap for Reads. */
	mem_access_subid.s.esw = _CVMX_PCIE_ES; /* Endian-swap for Writes. */
	mem_access_subid.s.wtype = 0;		/* "No snoop" and "Relaxed ordering" are not set */
	mem_access_subid.s.rtype = 0;		/* "No snoop" and "Relaxed ordering" are not set */
	/* PCIe Address Bits <63:34>. */
	if (OCTEON_IS_MODEL(OCTEON_CN68XX))
		mem_access_subid.cn68xx.ba = 0;
	else
		mem_access_subid.cn63xx.ba = 0;

	/* Setup mem access 12-15 for port 0, 16-19 for port 1, supplying 36
	 * bits of address space
	 */
	for (i = 12 + pcie_port * 4; i < 16 + pcie_port * 4; i++) {
		CVMX_WRITE_CSR(CVMX_PEXP_SLI_MEM_ACCESS_SUBIDX(i), mem_access_subid.u64);
		/* Set each SUBID to extend the addressable range */
		__cvmx_increment_ba(&mem_access_subid);
	}

	if (OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) ||
	    OCTEON_IS_MODEL(OCTEON_CN68XX) ||
	    (OCTEON_IS_OCTEON3() && !OCTEON_IS_MODEL(OCTEON_CN70XX))) {
		/* Disable the peer to peer forwarding register. This must be
		 * setup by the OS after it enumerates the bus and assigns
		 * addresses to the PCIe busses
		 */
		for (i = 0; i < 4; i++) {
			CVMX_WRITE_CSR(CVMX_PEMX_P2P_BARX_START(i, pcie_port), -1);
			CVMX_WRITE_CSR(CVMX_PEMX_P2P_BARX_END(i, pcie_port), -1);
		}
	}

	/* Set Octeon's BAR0 to decode 0-16KB. It overlaps with Bar2 */
	CVMX_WRITE_CSR(CVMX_PEMX_P2N_BAR0_START(pcie_port), 0);

	/* Set Octeon's BAR2 to decode 0-2^41. Bar0 and Bar1 take precedence
	 * where they overlap. It also overlaps with the device addresses, so
	 * make sure the peer to peer forwarding is set right
	 */
	CVMX_WRITE_CSR(CVMX_PEMX_P2N_BAR2_START(pcie_port), 0);

	/* Setup BAR2 attributes */
	/* Relaxed Ordering (NPEI_CTL_PORTn[PTLP_RO,CTLP_RO, WAIT_COM]) */
	/* - PTLP_RO,CTLP_RO should normally be set (except for debug). */
	/* - WAIT_COM=0 will likely work for all applications. */
	/* Load completion relaxed ordering (NPEI_CTL_PORTn[WAITL_COM]) */
	pemx_bar_ctl.u64 = CVMX_READ_CSR(CVMX_PEMX_BAR_CTL(pcie_port));
	pemx_bar_ctl.s.bar1_siz = 3; /* 256MB BAR1 */
	pemx_bar_ctl.s.bar2_enb = 1;
	pemx_bar_ctl.s.bar2_esx = _CVMX_PCIE_ES;
	pemx_bar_ctl.s.bar2_cax = 0;
	CVMX_WRITE_CSR(CVMX_PEMX_BAR_CTL(pcie_port), pemx_bar_ctl.u64);
	sli_ctl_portx.u64 = CVMX_READ_CSR(CVMX_PEXP_SLI_CTL_PORTX(pcie_port));
	sli_ctl_portx.s.ptlp_ro = 1;
	sli_ctl_portx.s.ctlp_ro = 1;
	sli_ctl_portx.s.wait_com = 0;
	sli_ctl_portx.s.waitl_com = 0;
	CVMX_WRITE_CSR(CVMX_PEXP_SLI_CTL_PORTX(pcie_port), sli_ctl_portx.u64);

	/* BAR1 follows BAR2 */
	CVMX_WRITE_CSR(CVMX_PEMX_P2N_BAR1_START(pcie_port),
		       CVMX_PCIE_BAR1_RC_BASE);

	bar1_index.u64 = 0;
	bar1_index.s.addr_idx = (CVMX_PCIE_BAR1_PHYS_BASE >> 22);
	bar1_index.s.ca = 1;		      /* Not Cached */
	bar1_index.s.end_swp = _CVMX_PCIE_ES; /* Endian Swap mode */
	bar1_index.s.addr_v = 1;	      /* Valid entry */

	for (i = 0; i < 16; i++) {
		CVMX_WRITE_CSR(CVMX_PEMX_BAR1_INDEXX(i, pcie_port),
			       bar1_index.u64);
		/* 256MB / 16 >> 22 == 4 */
		bar1_index.s.addr_idx += (((1ull << 28) / 16ull) >> 22);
	}

	/* Wait for 200ms */
	pemx_ctl_status.u64 = CVMX_READ_CSR(CVMX_PEMX_CTL_STATUS(pcie_port));
	pemx_ctl_status.cn63xx.cfg_rtry = cfg_retries();
	CVMX_WRITE_CSR(CVMX_PEMX_CTL_STATUS(pcie_port), pemx_ctl_status.u64);

	/*
	 * Here is the second part of the config retry changes. Wait for 700ms
	 * after setting up the link before continuing. PCIe says the devices
	 * may need up to 900ms to come up. 700ms plus 200ms from above gives
	 * us a total of 900ms
	 */
	if (OCTEON_IS_OCTEON2() || OCTEON_IS_MODEL(OCTEON_CN70XX))
		udelay(PCIE_DEVICE_READY_WAIT_DELAY_MICROSECONDS);
}

/**
 * Initialize a PCIe gen 2 port for use in host(RC) mode. It doesn't enumerate
 * the bus.
 *
 * @param pcie_port PCIe port to initialize
 *
 * Return: Zero on success
 */
static int __cvmx_pcie_rc_initialize_gen2(int pcie_port)
{
	cvmx_ciu_soft_prst_t ciu_soft_prst;
	cvmx_mio_rst_ctlx_t mio_rst_ctl;
	cvmx_pemx_bist_status_t pemx_bist_status;
	cvmx_pemx_bist_status2_t pemx_bist_status2;
	cvmx_pciercx_cfg032_t pciercx_cfg032;
	cvmx_pciercx_cfg515_t pciercx_cfg515;
	u64 ciu_soft_prst_reg, rst_ctl_reg;
	int ep_mode;
	int qlm = 0;
	int node = (pcie_port >> 4) & 0x3;

	pcie_port &= 0x3;

	if (pcie_port >= CVMX_PCIE_PORTS) {
		//debug("Invalid PCIe%d port\n", pcie_port);
		return -1;
	}

	if (__cvmx_pcie_check_qlm_mode(node, pcie_port, qlm))
		return -1;

	/* Make sure we aren't trying to setup a target mode interface in host
	 * mode
	 */
	if (OCTEON_IS_OCTEON3()) {
		ciu_soft_prst_reg = CVMX_RST_SOFT_PRSTX(pcie_port);
		rst_ctl_reg = CVMX_RST_CTLX(pcie_port);
	} else {
		ciu_soft_prst_reg = (pcie_port) ? CVMX_CIU_SOFT_PRST1 : CVMX_CIU_SOFT_PRST;
		rst_ctl_reg = CVMX_MIO_RST_CTLX(pcie_port);
	}
	mio_rst_ctl.u64 = CVMX_READ_CSR(rst_ctl_reg);

	ep_mode = ((OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)) ?
				 (mio_rst_ctl.s.prtmode != 1) :
				 (!mio_rst_ctl.s.host_mode));

	if (OCTEON_IS_MODEL(OCTEON_CN70XX) && pcie_port) {
		cvmx_pemx_cfg_t pemx_cfg;

		pemx_cfg.u64 = csr_rd(CVMX_PEMX_CFG(0));
		if ((pemx_cfg.s.md & 3) == 2) {
			printf("PCIe: Port %d in 1x4 mode.\n", pcie_port);
			return -1;
		}
	}

	if (ep_mode) {
		printf("%d:PCIe: Port %d in endpoint mode.\n", node, pcie_port);
		return -1;
	}

	/* CN63XX Pass 1.0 errata G-14395 requires the QLM De-emphasis be
	 * programmed
	 */
	if (OCTEON_IS_MODEL(OCTEON_CN63XX_PASS1_0)) {
		if (pcie_port) {
			cvmx_ciu_qlm1_t ciu_qlm;

			ciu_qlm.u64 = csr_rd(CVMX_CIU_QLM1);
			ciu_qlm.s.txbypass = 1;
			ciu_qlm.s.txdeemph = 5;
			ciu_qlm.s.txmargin = 0x17;
			csr_wr(CVMX_CIU_QLM1, ciu_qlm.u64);
		} else {
			cvmx_ciu_qlm0_t ciu_qlm;

			ciu_qlm.u64 = csr_rd(CVMX_CIU_QLM0);
			ciu_qlm.s.txbypass = 1;
			ciu_qlm.s.txdeemph = 5;
			ciu_qlm.s.txmargin = 0x17;
			csr_wr(CVMX_CIU_QLM0, ciu_qlm.u64);
		}
	}

	/* Bring the PCIe out of reset */
	ciu_soft_prst.u64 = CVMX_READ_CSR(ciu_soft_prst_reg);
	/* After a chip reset the PCIe will also be in reset. If it
	 * isn't, most likely someone is trying to init it again
	 * without a proper PCIe reset.
	 */
	if (ciu_soft_prst.s.soft_prst == 0) {
		/* Reset the port */
		ciu_soft_prst.s.soft_prst = 1;
		CVMX_WRITE_CSR(ciu_soft_prst_reg, ciu_soft_prst.u64);

		/* Read to make sure write happens */
		ciu_soft_prst.u64 = CVMX_READ_CSR(ciu_soft_prst_reg);

		/* Keep PERST asserted for 2 ms */
		udelay(2000);
	}

	/* Deassert PERST */
	ciu_soft_prst.u64 = CVMX_READ_CSR(ciu_soft_prst_reg);
	ciu_soft_prst.s.soft_prst = 0;
	CVMX_WRITE_CSR(ciu_soft_prst_reg, ciu_soft_prst.u64);
	ciu_soft_prst.u64 = CVMX_READ_CSR(ciu_soft_prst_reg);

	/* Wait 1ms for PCIe reset to complete */
	udelay(1000);

	/* Set MPLL multiplier as per Errata 20669. */
	if (OCTEON_IS_MODEL(OCTEON_CN70XX)) {
		int qlm = __cvmx_pcie_get_qlm(0, pcie_port);
		enum cvmx_qlm_mode mode;
		int old_mult;
		u64 meas_refclock = cvmx_qlm_measure_clock(qlm);

		if (meas_refclock > 99000000 && meas_refclock < 101000000) {
			old_mult = 35;
		} else if (meas_refclock > 124000000 &&
			   meas_refclock < 126000000) {
			old_mult = 56;
		} else if (meas_refclock > 156000000 &&
			   meas_refclock < 156500000) {
			old_mult = 45;
		} else {
			printf("%s: Invalid reference clock for qlm %d\n",
			       __func__, qlm);
			return -1;
		}
		mode = cvmx_qlm_get_mode(qlm);
		__cvmx_qlm_set_mult(qlm, 2500, old_mult);
		/* Adjust mplls for both dlms when configured as pcie 1x4 */
		if (mode == CVMX_QLM_MODE_PCIE && pcie_port == 0)
			__cvmx_qlm_set_mult(qlm + 1, 2500, old_mult);
	}

	/*
	 * Check and make sure PCIe came out of reset. If it doesn't the board
	 * probably hasn't wired the clocks up and the interface should be
	 * skipped
	 */
	if (CVMX_WAIT_FOR_FIELD64_NODE(node, rst_ctl_reg, cvmx_mio_rst_ctlx_t,
				       rst_done, ==, 1, 10000)) {
		printf("%d:PCIe: Port %d stuck in reset, skipping.\n", node, pcie_port);
		return -1;
	}

	/* Check BIST status */
	pemx_bist_status.u64 = CVMX_READ_CSR(CVMX_PEMX_BIST_STATUS(pcie_port));
	if (pemx_bist_status.u64)
		printf("%d:PCIe: BIST FAILED for port %d (0x%016llx)\n", node, pcie_port,
		       CAST64(pemx_bist_status.u64));
	pemx_bist_status2.u64 = CVMX_READ_CSR(CVMX_PEMX_BIST_STATUS2(pcie_port));

	/*
	 * Errata PCIE-14766 may cause the lower 6 bits to be randomly set on
	 * CN63XXp1
	 */
	if (OCTEON_IS_MODEL(OCTEON_CN63XX_PASS1_X))
		pemx_bist_status2.u64 &= ~0x3full;

	if (pemx_bist_status2.u64) {
		printf("%d:PCIe: BIST2 FAILED for port %d (0x%016llx)\n",
		       node, pcie_port, CAST64(pemx_bist_status2.u64));
	}

	/* Initialize the config space CSRs */
	__cvmx_pcie_rc_initialize_config_space(node, pcie_port);

	/* Enable gen2 speed selection */
	pciercx_cfg515.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG515(pcie_port));
	pciercx_cfg515.s.dsc = 1;
	CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG515(pcie_port), pciercx_cfg515.u32);

	/* Bring the link up */
	if (__cvmx_pcie_rc_initialize_link_gen2(node, pcie_port)) {
		/* Some gen1 devices don't handle the gen 2 training correctly.
		 * Disable gen2 and try again with only gen1
		 */
		cvmx_pciercx_cfg031_t pciercx_cfg031;

		pciercx_cfg031.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG031(pcie_port));
		pciercx_cfg031.s.mls = 1;
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG031(pcie_port), pciercx_cfg031.u32);
		if (__cvmx_pcie_rc_initialize_link_gen2(node, pcie_port)) {
			printf("PCIe: Link timeout on port %d, probably the slot is empty\n",
			       pcie_port);
			return -1;
		}
	}

	__cvmx_pcie_sli_config(node, pcie_port);

	/* Display the link status */
	pciercx_cfg032.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG032(pcie_port));
	printf("PCIe: Port %d link active, %d lanes, speed gen%d\n", pcie_port,
	       pciercx_cfg032.s.nlw, pciercx_cfg032.s.ls);

	pcie_link_initialized[node][pcie_port] = true;
	return 0;
}

/**
 * @INTERNAL
 * Initialize a host mode PCIe gen 2 link. This function takes a PCIe
 * port from reset to a link up state. Software can then begin
 * configuring the rest of the link.
 *
 * @param node	    node
 * @param pcie_port PCIe port to initialize
 *
 * Return: Zero on success
 */
static int __cvmx_pcie_rc_initialize_link_gen2_v3(int node, int pcie_port)
{
	u8 ltssm_history[LTSSM_HISTORY_SIZE];
	int ltssm_history_loc;
	cvmx_pemx_ctl_status_t pem_ctl_status;
	cvmx_pciercx_cfg006_t pciercx_cfg006;
	cvmx_pciercx_cfg031_t pciercx_cfg031;
	cvmx_pciercx_cfg032_t pciercx_cfg032;
	cvmx_pciercx_cfg068_t pciercx_cfg068;
	cvmx_pciercx_cfg448_t pciercx_cfg448;
	cvmx_pciercx_cfg515_t pciercx_cfg515;
	int max_gen, max_width;
	u64 hold_time;
	u64 bounce_allow_time;
	u64 timeout, good_time, current_time;
	int neg_gen, neg_width, bus, dev_gen, dev_width;
	unsigned int cap, cap_next;
	int ltssm_state, desired_gen;
	int desired_width;
	int i, need_speed_change, need_lane_change;
	int do_retry_speed = 0;
	int link_up = 0, is_loop_done = 0;

	if (CVMX_WAIT_FOR_FIELD64_NODE(node, CVMX_PEMX_ON(pcie_port), cvmx_pemx_on_t, pemoor, ==, 1,
				       100000)) {
		printf("N%d:PCIe: Port %d PEM not on, skipping\n", node, pcie_port);
		return -1;
	}

	/* Record starting LTSSM state for debug */
	memset(ltssm_history, -1, sizeof(ltssm_history));
	ltssm_history[0] = __cvmx_pcie_rc_get_ltssm_state(node, pcie_port);
	ltssm_history_loc = 0;

	pciercx_cfg031.u32 = CVMX_PCIE_CFGX_READ(pcie_port,
						 CVMX_PCIERCX_CFG031(pcie_port));
	/* Max speed of PEM from config (1-3) */
	max_gen = pciercx_cfg031.s.mls;
	/* Max lane width of PEM (1-3) */
	max_width = pciercx_cfg031.s.mlw;
#ifdef DEBUG_PCIE
	printf("N%d.PCIe%d: Link supports up to %d lanes, speed gen%d\n",
	       node, pcie_port, max_width, max_gen);
#endif

	/* Bring up the link */
#ifdef DEBUG_PCIE
	printf("N%d.PCIe%d: Enabling the link\n", node, pcie_port);
#endif
	pem_ctl_status.u64 = CVMX_READ_CSR(CVMX_PEMX_CTL_STATUS(pcie_port));
	pem_ctl_status.s.lnk_enb = 1;
	CVMX_WRITE_CSR(CVMX_PEMX_CTL_STATUS(pcie_port), pem_ctl_status.u64);

	/*
	 * Configure SLI after enabling PCIe link. Is required for reading
	 * PCIe card capabilities.
	 */
	__cvmx_pcie_sli_config(node, pcie_port);

	/*
	 * After the link is enabled  no prints until link up or error,
	 * Otherwise will miss link state captures
	 */

retry_speed:
	/* Clear RC Correctable Error Status Register */
	CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG068(pcie_port), -1);

	/* Wait for the link to come up and link training to be complete */
#ifdef DEBUG_PCIE
	printf("N%d.PCIe%d: Waiting for link\n", node, pcie_port);
#endif

	/* Timeout of 2 secs */
	timeout = get_timer(0) + 2000;

	/* Records when the link first went good */
	good_time = 0;

	do {
		pciercx_cfg032.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG032(pcie_port));
		/*
		 * Errata PEM-31375 PEM RSL access to PCLK registers can
		 * timeout during speed change. Check for temporary hardware
		 * timeout, and rety if happens
		 */
		if (pciercx_cfg032.u32 == 0xffffffff)
			continue;

		/* Record LTSSM state for debug */
		ltssm_state = __cvmx_pcie_rc_get_ltssm_state(node, pcie_port);

		if (ltssm_history[ltssm_history_loc] != ltssm_state) {
			ltssm_history_loc = (ltssm_history_loc + 1) & (LTSSM_HISTORY_SIZE - 1);
			ltssm_history[ltssm_history_loc] = ltssm_state;
		}

		/* Check if the link is up */
		//		current_time = cvmx_get_cycle();
		current_time = get_timer(0);
		link_up = (pciercx_cfg032.s.dlla && !pciercx_cfg032.s.lt);

		if (link_up) {
			/* Is this the first link up? */
			if (!good_time) {
				/* Mark the time when the link transitioned to good */
				good_time = current_time;
			} else {
				/* Check for a link error */
				pciercx_cfg068.u32 = CVMX_PCIE_CFGX_READ(
					pcie_port, CVMX_PCIERCX_CFG068(pcie_port));
				if (pciercx_cfg068.s.res) {
					/*
					 * Ignore errors before we've been
					 * stable for bounce_allow_time
					 */
					if (good_time + bounce_allow_time <=
					    current_time) {
#ifdef DEBUG_PCIE
						printf("N%d.PCIe%d: Link errors after link up\n",
						       node, pcie_port);
#endif
						/* Link error, signal a retry */
						return 1;
					}

					/*
					 * Clear RC Correctable Error
					 * Status Register
					 */
					CVMX_PCIE_CFGX_WRITE(pcie_port,
							     CVMX_PCIERCX_CFG068(pcie_port),
							     -1);
#ifdef DEBUG_PCIE
					printf("N%d.PCIe%d: Ignored error during settling time\n",
					       node, pcie_port);
#endif
				}
			}
		} else if (good_time) {
			if (good_time + bounce_allow_time <= current_time) {
				/*
				 * We allow bounces for bounce_allow_time after
				 * the link is good. Once this time passes any
				 * bounce requires a retry
				 */
#ifdef DEBUG_PCIE
				printf("N%d.PCIe%d: Link bounce detected\n",
				       node, pcie_port);
#endif
				return 1; /* Link bounce, signal a retry */
			}

#ifdef DEBUG_PCIE
			printf("N%d.PCIe%d: Ignored bounce during settling time\n",
			       node, pcie_port);
#endif
		}

		/* Determine if we've hit the timeout */
		is_loop_done = (current_time >= timeout);

		/*
		 * Determine if we've had a good link for the required hold
		 * time
		 */
		is_loop_done |= link_up && (good_time + hold_time <=
					    current_time);
	} while (!is_loop_done);

	/* Trace the LTSSM state */
#ifdef DEBUG_PCIE
	printf("N%d.PCIe%d: LTSSM History\n", node, pcie_port);
#endif
	for (i = 0; i < LTSSM_HISTORY_SIZE; i++) {
		ltssm_history_loc = (ltssm_history_loc + 1) & (LTSSM_HISTORY_SIZE - 1);
#ifdef DEBUG_PCIE
		if (ltssm_history[ltssm_history_loc] != 0xff)
			printf("N%d.PCIe%d: %s\n", node, pcie_port,
			       cvmx_pcie_get_ltssm_string(ltssm_history[ltssm_history_loc]));
#endif
	}

	if (!link_up) {
		ltssm_state = __cvmx_pcie_rc_get_ltssm_state(node, pcie_port);
#ifdef DEBUG_PCIE
		printf("N%d.PCIe%d: Link down, Data link layer %s(DLLA=%d), Link training %s(LT=%d), LTSSM %s\n",
		       node, pcie_port, pciercx_cfg032.s.dlla ? "active" : "down",
		       pciercx_cfg032.s.dlla, pciercx_cfg032.s.lt ? "active" : "complete",
		       pciercx_cfg032.s.lt, cvmx_pcie_get_ltssm_string(ltssm_state));
#endif
		return 1; /* Link down, signal a retry */
	}

	/* Report the negotiated link speed and width */
	neg_gen = pciercx_cfg032.s.ls;	  /* Current speed of PEM (1-3) */
	neg_width = pciercx_cfg032.s.nlw; /* Current lane width of PEM (1-8) */
#ifdef DEBUG_PCIE
	printf("N%d.PCIe%d: Link negotiated %d lanes, speed gen%d\n", node, pcie_port, neg_width,
	       neg_gen);
#endif
	/* Determine PCIe bus number the directly attached device uses */
	pciercx_cfg006.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG006(pcie_port));
	bus = pciercx_cfg006.s.sbnum;

	/* The SLI has to be initialized so we can read the downstream devices     */
	dev_gen = 1;   /* Device max speed (1-3) */
	dev_width = 1; /* Device max lane width (1-16) */
#ifdef DEBUG_PCIE
	printf("N%d.PCIe%d: Reading Bus %d device max speed and width\n", node, pcie_port, bus);
#endif

	/*
	 * Here is the second part of the config retry changes. Wait for 700ms
	 * after setting up the link before continuing. PCIe says the devices
	 * may need up to 900ms to come up. 700ms plus 200ms from above gives
	 * us a total of 900ms
	 */
	udelay(PCIE_DEVICE_READY_WAIT_DELAY_MICROSECONDS);

	/* Read PCI capability pointer at offset 0x34 of target */
	cap = cvmx_pcie_config_read32_retry(node, pcie_port, bus, 0, 0, 0x34);

	/* Check if we were able to read capabilities pointer */
	if (cap == 0xffffffff)
		return 1; /* Signal retry needed */

	/* Read device max speed and width */
	cap_next = cap & 0xff;
	while (cap_next) {
		cap = cvmx_pcie_config_read32_retry(node, pcie_port, bus,
						    0, 0, cap_next);
		if (cap == 0xffffffff)
			return 1; /* Signal retry needed */

		/* Is this a PCIe capability (0x10)? */
		if ((cap & 0xff) == 0x10) {
#ifdef DEBUG_PCIE
			printf("N%d.PCIe%d: Found PCIe capability at offset 0x%x\n",
			       node, pcie_port, cap_next);
#endif
			/* Offset 0xc contains the max link info */
			cap = cvmx_pcie_config_read32_retry(node, pcie_port, bus, 0, 0,
							    cap_next + 0xc);
			if (cap == 0xffffffff)
				return 1;	       /* Signal retry needed */
			dev_gen = cap & 0xf;	       /* Max speed of PEM from config (1-3) */
			dev_width = (cap >> 4) & 0x3f; /* Max lane width of PEM (1-16) */
#ifdef DEBUG_PCIE
			printf("N%d.PCIe%d: Device supports %d lanes, speed gen%d\n", node,
			       pcie_port, dev_width, dev_gen);
#endif
			break;
		}
		/* Move to next capability */
		cap_next = (cap >> 8) & 0xff;
	}

	/*
	 * Desired link speed and width is either limited by the device or our
	 * PEM configuration. Choose the most restrictive limit
	 */
	desired_gen = (dev_gen < max_gen) ? dev_gen : max_gen;
	desired_width = (dev_width < max_width) ? dev_width : max_width;

	/*
	 * We need a change if we don't match the desired speed or width.
	 * Note that we allow better than expected in case the device lied
	 * about its capabilities
	 */
	need_speed_change = (neg_gen < desired_gen);
	need_lane_change = (neg_width < desired_width);

	if (need_lane_change) {
		/* We didn't get the maximum number of lanes */
#ifdef DEBUG_PCIE
		printf("N%d.PCIe%d: Link width (%d) less that supported (%d)\n",
		       node, pcie_port, neg_width, desired_width);
#endif
		return 2; /* Link wrong width, signal a retry */
	} else if (need_speed_change) {
		if (do_retry_speed) {
#ifdef DEBUG_PCIE
			printf("N%d.PCIe%d: Link speed (gen%d) less that supported (gen%d)\n", node,
			       pcie_port, neg_gen, desired_gen);
#endif
			return 1; /* Link at width, but speed low. Request a retry */
		}

		/* We didn't get the maximum speed. Request a speed change */
#ifdef DEBUG_PCIE
		printf("N%d.PCIe%d: Link speed (gen%d) less that supported (gen%d), requesting a speed change\n",
		       node, pcie_port, neg_gen, desired_gen);
#endif
		pciercx_cfg515.u32 =
			CVMX_PCIE_CFGX_READ(pcie_port,
					    CVMX_PCIERCX_CFG515(pcie_port));
		pciercx_cfg515.s.dsc = 1;
		CVMX_PCIE_CFGX_WRITE(pcie_port,
				     CVMX_PCIERCX_CFG515(pcie_port),
				     pciercx_cfg515.u32);
		mdelay(100);
		do_retry_speed = true;
		goto retry_speed;
	} else {
#ifdef DEBUG_PCIE
		printf("N%d.PCIe%d: Link at best speed and width\n",
		       node, pcie_port);
#endif
		/* For gen3 links check if we are getting errors over the link */
		if (neg_gen == 3) {
			/* Read RC Correctable Error Status Register */
			pciercx_cfg068.u32 =
				CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG068(pcie_port));
			if (pciercx_cfg068.s.res) {
#ifdef DEBUG_PCIE
				printf("N%d.PCIe%d: Link reporting error status\n", node,
				       pcie_port);
#endif
				return 1; /* Getting receiver errors, request a retry */
			}
		}
		return 0; /* Link at correct speed and width */
	}

	/* Update the Replay Time Limit.  Empirically, some PCIe devices take a
	 * little longer to respond than expected under load. As a workaround
	 * for this we configure the Replay Time Limit to the value expected
	 * for a 512 byte MPS instead of our actual 256 byte MPS. The numbers
	 * below are directly from the PCIe spec table 3-4
	 */
	pciercx_cfg448.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG448(pcie_port));
	switch (pciercx_cfg032.s.nlw) {
	case 1: /* 1 lane */
		pciercx_cfg448.s.rtl = 1677;
		break;
	case 2: /* 2 lanes */
		pciercx_cfg448.s.rtl = 867;
		break;
	case 4: /* 4 lanes */
		pciercx_cfg448.s.rtl = 462;
		break;
	case 8: /* 8 lanes */
		pciercx_cfg448.s.rtl = 258;
		break;
	}
	CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG448(pcie_port), pciercx_cfg448.u32);

	return 0;
}

static int __cvmx_pcie_rc_initialize_gen2_v3(int pcie_port)
{
	cvmx_rst_ctlx_t rst_ctl;
	cvmx_rst_soft_prstx_t rst_soft_prst;
	cvmx_pciercx_cfg031_t pciercx_cfg031;
	cvmx_pciercx_cfg032_t pciercx_cfg032;
	cvmx_pciercx_cfg038_t pciercx_cfg038;
	cvmx_pciercx_cfg040_t pciercx_cfg040;
	cvmx_pciercx_cfg515_t pciercx_cfg515;
	cvmx_pciercx_cfg548_t pciercx_cfg548;
	cvmx_pemx_bist_status_t pemx_bist_status;
	u64 rst_soft_prst_reg;
	int qlm;
	int node = (pcie_port >> 4) & 0x3;
	bool requires_pem_reset = 0;
	enum cvmx_qlm_mode mode = CVMX_QLM_MODE_DISABLED;
	int retry_count = 0;
	int result = 0;

	pcie_port &= 0x3;

	/* Assume link down until proven up */
	pcie_link_initialized[node][pcie_port] = false;

	/* Attempt link initialization up to 3 times */
	while (retry_count <= MAX_RETRIES) {
#ifdef DEBUG_PCIE
		if (retry_count)
			printf("N%d:PCIE%d: Starting link retry %d\n", node, pcie_port,
			       retry_count);
#endif
		if (pcie_port >= CVMX_PCIE_PORTS) {
#ifdef DEBUG_PCIE
			printf("Invalid PCIe%d port\n", pcie_port);
#endif
			return -1;
		}

		qlm = __cvmx_pcie_get_qlm(node, pcie_port);

		if (qlm < 0)
			return -1;

		mode = cvmx_qlm_get_mode(qlm);
		if (__cvmx_pcie_check_pcie_port(node, pcie_port, mode))
			return -1;

		rst_soft_prst_reg = CVMX_RST_SOFT_PRSTX(pcie_port);
		rst_ctl.u64 = CVMX_READ_CSR(CVMX_RST_CTLX(pcie_port));

		if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
			CVMX_WRITE_CSR(CVMX_DTX_PEMX_SELX(0, pcie_port), 0x17);
			CVMX_WRITE_CSR(CVMX_DTX_PEMX_SELX(1, pcie_port), 0);
		}

		if (!rst_ctl.s.host_mode) {
			printf("N%d:PCIE: Port %d in endpoint mode.\n",
			       node, pcie_port);
			return -1;
		}

		/* Bring the PCIe out of reset */
		rst_soft_prst.u64 = CVMX_READ_CSR(rst_soft_prst_reg);

		/*
		 * After a chip reset the PCIe will also be in reset. If it
		 * isn't, most likely someone is trying to init it again
		 * without a proper PCIe reset.
		 */
		if (rst_soft_prst.s.soft_prst == 0) {
			/* Disable the MAC controller before resetting */
			__cvmx_pcie_config_pemon(node, pcie_port, 0);

			/* Reset the port */
			rst_soft_prst.s.soft_prst = 1;
			CVMX_WRITE_CSR(rst_soft_prst_reg, rst_soft_prst.u64);

			/* Read to make sure write happens */
			rst_soft_prst.u64 = CVMX_READ_CSR(rst_soft_prst_reg);

			/* Keep PERST asserted for 2 ms */
			udelay(2000);

			/* Reset GSER_PHY to put in a clean state */
			__cvmx_pcie_gser_phy_config(node, pcie_port, qlm);
			requires_pem_reset = 1;

			/* Enable MAC controller before taking pcie out of reset */
			__cvmx_pcie_config_pemon(node, pcie_port, 1);
		}

		/* Deassert PERST */
		rst_soft_prst.u64 = CVMX_READ_CSR(rst_soft_prst_reg);
		rst_soft_prst.s.soft_prst = 0;
		CVMX_WRITE_CSR(rst_soft_prst_reg, rst_soft_prst.u64);
		rst_soft_prst.u64 = CVMX_READ_CSR(rst_soft_prst_reg);

		/* Check if PLLs are locked after GSER_PHY reset. */
		if (requires_pem_reset) {
			cvmx_pemx_cfg_t pemx_cfg;

			pemx_cfg.u64 = csr_rd(CVMX_PEMX_CFG(pcie_port));
			if (CVMX_WAIT_FOR_FIELD64(CVMX_GSERX_QLM_STAT(qlm), cvmx_gserx_qlm_stat_t,
						  rst_rdy, ==, 1, 10000)) {
				printf("QLM%d: Timeout waiting for GSERX_QLM_STAT[rst_rdy]\n", qlm);
				return -1;
			}
			if (pemx_cfg.cn78xx.lanes8 &&
			    (CVMX_WAIT_FOR_FIELD64(CVMX_GSERX_QLM_STAT(qlm + 1),
						   cvmx_gserx_qlm_stat_t, rst_rdy, ==, 1, 10000))) {
				printf("QLM%d: Timeout waiting for GSERX_QLM_STAT[rst_rdy]\n",
				       qlm + 1);
				return -1;
			}
		}

		/* Wait 1ms for PCIe reset to complete */
		udelay(1000);

		/*
		 * Check and make sure PCIe came out of reset. If it doesn't
		 * the board probably hasn't wired the clocks up and the
		 * interface should be skipped
		 */
		if (CVMX_WAIT_FOR_FIELD64_NODE(node, CVMX_RST_CTLX(pcie_port),
					       cvmx_rst_ctlx_t,
					       rst_done, ==, 1, 10000)) {
			printf("N%d:PCIE: Port %d stuck in reset, skipping.\n", node, pcie_port);
			return -1;
		}

		/* Check BIST status */
		pemx_bist_status.u64 = CVMX_READ_CSR(CVMX_PEMX_BIST_STATUS(pcie_port));
		if (pemx_bist_status.u64)
			printf("N%d:PCIE: BIST FAILED for port %d (0x%016llx)\n", node, pcie_port,
			       CAST64(pemx_bist_status.u64));

			/* Initialize the config space CSRs */
#ifdef DEBUG_PCIE
		printf("N%d:PCIE%d Initialize Config Space\n", node, pcie_port);
#endif
		__cvmx_pcie_rc_initialize_config_space(node, pcie_port);

		/* Enable gen2 speed selection */
		pciercx_cfg515.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG515(pcie_port));
		pciercx_cfg515.s.dsc = 1;
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG515(pcie_port), pciercx_cfg515.u32);

		/* Do the link retries on the PCIe interface */
		if (retry_count == MAX_RETRIES) {
			/*
			 * This has to be done AFTER the QLM/PHY interface
			 * initialized
			 */
			pciercx_cfg031.u32 =
				CVMX_PCIE_CFGX_READ(pcie_port,
						    CVMX_PCIERCX_CFG031(pcie_port));
			/*
			 * Drop speed to gen2 if link bouncing
			 * Result = -1  PEM in reset
			 * Result = 0:  link speed and width ok no retry needed
			 * Result = 1:  link errors or speed change needed
			 * Result = 2:  lane width error
			 */
			if (pciercx_cfg031.s.mls == 3 && result != 2) {
#ifdef DEBUG_PCIE
				printf("N%d:PCIE%d: Dropping speed to gen2\n", node, pcie_port);
#endif
				pciercx_cfg031.s.mls = 2;
				CVMX_PCIE_CFGX_WRITE(pcie_port,
						     CVMX_PCIERCX_CFG031(pcie_port),
						     pciercx_cfg031.u32);

				/* Set the target link speed */
				pciercx_cfg040.u32 = CVMX_PCIE_CFGX_READ(
					pcie_port, CVMX_PCIERCX_CFG040(pcie_port));
				pciercx_cfg040.s.tls = 2;
				CVMX_PCIE_CFGX_WRITE(pcie_port,
						     CVMX_PCIERCX_CFG040(pcie_port),
						     pciercx_cfg040.u32);
			}
		}

		/* Bring the link up */
		result = __cvmx_pcie_rc_initialize_link_gen2_v3(node, pcie_port);
		if (result == 0) {
#ifdef DEBUG_PCIE
			printf("N%d:PCIE%d: Link does not need a retry\n", node, pcie_port);
#endif
			break;
		} else if (result > 0) {
			if (retry_count >= MAX_RETRIES) {
				int link_up;
#ifdef DEBUG_PCIE
				printf("N%d:PCIE%d: Link requested a retry, but hit the max retries\n",
				       node, pcie_port);
#endif
				/* If the link is down, report failure */
				pciercx_cfg032.u32 = CVMX_PCIE_CFGX_READ(
					pcie_port,
					CVMX_PCIERCX_CFG032(pcie_port));
				link_up = (pciercx_cfg032.s.dlla && !pciercx_cfg032.s.lt);
				if (!link_up)
					result = -1;
			}
#ifdef DEBUG_PCIE
			else
				printf("N%d.PCIE%d: Link requested a retry\n", node, pcie_port);
#endif
		}
		if (result < 0) {
			int ltssm_state = __cvmx_pcie_rc_get_ltssm_state(node, pcie_port);

			printf("N%d:PCIE%d: Link timeout, probably the slot is empty (LTSSM %s)\n",
			       node, pcie_port, cvmx_pcie_get_ltssm_string(ltssm_state));
			return -1;
		}
		retry_count++;
	}

	pciercx_cfg032.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG032(pcie_port));
	/*
	 * Errata PEM-28816: Link retrain initiated at GEN1 can cause PCIE
	 * link to hang. For Gen1 links we must disable equalization
	 */
	if (pciercx_cfg032.s.ls == 1) {
#ifdef DEBUG_PCIE
		printf("N%d:PCIE%d: Disabling equalization for GEN1 Link\n", node, pcie_port);
#endif
		pciercx_cfg548.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG548(pcie_port));
		pciercx_cfg548.s.ed = 1;
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG548(pcie_port), pciercx_cfg548.u32);
	}

	/* Errata PCIE-29440: Atomic operations to work properly */
	pciercx_cfg038.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG038(pcie_port));
	pciercx_cfg038.s.atom_op_eb = 0;
	pciercx_cfg038.s.atom_op = 1;
	CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG038(pcie_port), pciercx_cfg038.u32);

	/* Errata PCIE-29566 PEM Link Hangs after going into L1 */
	pciercx_cfg548.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG548(pcie_port));
	pciercx_cfg548.s.grizdnc = 0;
	CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIERCX_CFG548(pcie_port), pciercx_cfg548.u32);

	if (result < 0)
		return result;

	/* Display the link status */
	pciercx_cfg032.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIERCX_CFG032(pcie_port));
	printf("N%d:PCIe: Port %d link active, %d lanes, speed gen%d\n", node, pcie_port,
	       pciercx_cfg032.s.nlw, pciercx_cfg032.s.ls);

	pcie_link_initialized[node][pcie_port] = true;
	return 0;
}

/**
 * Initialize a PCIe port for use in host(RC) mode. It doesn't enumerate the bus.
 *
 * @param pcie_port PCIe port to initialize for a node
 *
 * Return: Zero on success
 */
int cvmx_pcie_rc_initialize(int pcie_port)
{
	int result;

	if (OCTEON_IS_OCTEON2() || OCTEON_IS_MODEL(OCTEON_CN70XX))
		result = __cvmx_pcie_rc_initialize_gen2(pcie_port);
	else
		result = __cvmx_pcie_rc_initialize_gen2_v3(pcie_port);

	if (result == 0)
		cvmx_error_enable_group(CVMX_ERROR_GROUP_PCI, pcie_port);
	return result;
}

/**
 * Shutdown a PCIe port and put it in reset
 *
 * @param pcie_port PCIe port to shutdown for a node
 *
 * Return: Zero on success
 */
int cvmx_pcie_rc_shutdown(int pcie_port)
{
	u64 ciu_soft_prst_reg;
	cvmx_ciu_soft_prst_t ciu_soft_prst;
	int node;

	/* Shutdown only if PEM is in RC mode */
	if (!cvmx_pcie_is_host_mode(pcie_port))
		return -1;

	node = (pcie_port >> 4) & 0x3;
	pcie_port &= 0x3;
	cvmx_error_disable_group(CVMX_ERROR_GROUP_PCI, pcie_port);
	/* Wait for all pending operations to complete */
	if (CVMX_WAIT_FOR_FIELD64_NODE(node, CVMX_PEMX_CPL_LUT_VALID(pcie_port),
				       cvmx_pemx_cpl_lut_valid_t, tag, ==,
				       0, 2000))
		debug("PCIe: Port %d shutdown timeout\n", pcie_port);

	if (OCTEON_IS_OCTEON3()) {
		ciu_soft_prst_reg = CVMX_RST_SOFT_PRSTX(pcie_port);
	} else {
		ciu_soft_prst_reg = (pcie_port) ? CVMX_CIU_SOFT_PRST1 :
			CVMX_CIU_SOFT_PRST;
	}

	/* Force reset */
	ciu_soft_prst.u64 = CVMX_READ_CSR(ciu_soft_prst_reg);
	ciu_soft_prst.s.soft_prst = 1;
	CVMX_WRITE_CSR(ciu_soft_prst_reg, ciu_soft_prst.u64);

	return 0;
}

/**
 * @INTERNAL
 * Build a PCIe config space request address for a device
 *
 * @param node	    node
 * @param port	    PCIe port (relative to the node) to access
 * @param bus       Sub bus
 * @param dev       Device ID
 * @param fn        Device sub function
 * @param reg       Register to access
 *
 * Return: 64bit Octeon IO address
 */
static uint64_t __cvmx_pcie_build_config_addr(int node, int port, int bus, int dev, int fn, int reg)
{
	cvmx_pcie_address_t pcie_addr;
	cvmx_pciercx_cfg006_t pciercx_cfg006;

	pciercx_cfg006.u32 = cvmx_pcie_cfgx_read_node(node, port,
						      CVMX_PCIERCX_CFG006(port));
	if (bus <= pciercx_cfg006.s.pbnum && dev != 0)
		return 0;

	pcie_addr.u64 = 0;
	pcie_addr.config.upper = 2;
	pcie_addr.config.io = 1;
	pcie_addr.config.did = 3;
	pcie_addr.config.subdid = 1;
	pcie_addr.config.node = node;
	pcie_addr.config.es = _CVMX_PCIE_ES;
	pcie_addr.config.port = port;
	/* Always use config type 0 */
	if (pciercx_cfg006.s.pbnum == 0)
		pcie_addr.config.ty = (bus > pciercx_cfg006.s.pbnum + 1);
	else
		pcie_addr.config.ty = (bus > pciercx_cfg006.s.pbnum);
	pcie_addr.config.bus = bus;
	pcie_addr.config.dev = dev;
	pcie_addr.config.func = fn;
	pcie_addr.config.reg = reg;
	return pcie_addr.u64;
}

/**
 * Read 8bits from a Device's config space
 *
 * @param pcie_port PCIe port the device is on
 * @param bus       Sub bus
 * @param dev       Device ID
 * @param fn        Device sub function
 * @param reg       Register to access
 *
 * Return: Result of the read
 */
uint8_t cvmx_pcie_config_read8(int pcie_port, int bus, int dev, int fn, int reg)
{
	u64 address;
	int node = (pcie_port >> 4) & 0x3;

	pcie_port &= 0x3;
	address = __cvmx_pcie_build_config_addr(node, pcie_port, bus, dev, fn, reg);
	if (address)
		return cvmx_read64_uint8(address);
	else
		return 0xff;
}

/**
 * Read 16bits from a Device's config space
 *
 * @param pcie_port PCIe port the device is on
 * @param bus       Sub bus
 * @param dev       Device ID
 * @param fn        Device sub function
 * @param reg       Register to access
 *
 * Return: Result of the read
 */
uint16_t cvmx_pcie_config_read16(int pcie_port, int bus, int dev, int fn, int reg)
{
	u64 address;
	int node = (pcie_port >> 4) & 0x3;

	pcie_port &= 0x3;
	address = __cvmx_pcie_build_config_addr(node, pcie_port, bus, dev, fn, reg);
	if (address)
		return le16_to_cpu(cvmx_read64_uint16(address));
	else
		return 0xffff;
}

static uint32_t __cvmx_pcie_config_read32(int node, int pcie_port, int bus, int dev, int func,
					  int reg, int lst)
{
	u64 address;

	address = __cvmx_pcie_build_config_addr(node, pcie_port, bus, dev, func, reg);
	if (lst) {
		if (address && pcie_link_initialized[node][pcie_port])
			return le32_to_cpu(cvmx_read64_uint32(address));
		else
			return 0xffffffff;
	} else if (address) {
		return le32_to_cpu(cvmx_read64_uint32(address));
	} else {
		return 0xffffffff;
	}
}

/**
 * Read 32bits from a Device's config space
 *
 * @param pcie_port PCIe port the device is on
 * @param bus       Sub bus
 * @param dev       Device ID
 * @param fn        Device sub function
 * @param reg       Register to access
 *
 * Return: Result of the read
 */
uint32_t cvmx_pcie_config_read32(int pcie_port, int bus, int dev, int fn, int reg)
{
	int node = (pcie_port >> 4) & 0x3;

	pcie_port &= 0x3;
	return __cvmx_pcie_config_read32(node, pcie_port, bus, dev, fn, reg,
					 pcie_link_initialized[node][pcie_port]);
}

/**
 * Write 8bits to a Device's config space
 *
 * @param pcie_port PCIe port the device is on
 * @param bus       Sub bus
 * @param dev       Device ID
 * @param fn        Device sub function
 * @param reg       Register to access
 * @param val       Value to write
 */
void cvmx_pcie_config_write8(int pcie_port, int bus, int dev, int fn, int reg, uint8_t val)
{
	u64 address;
	int node = (pcie_port >> 4) & 0x3;

	pcie_port &= 0x3;
	address = __cvmx_pcie_build_config_addr(node, pcie_port, bus, dev, fn, reg);
	if (address)
		cvmx_write64_uint8(address, val);
}

/**
 * Write 16bits to a Device's config space
 *
 * @param pcie_port PCIe port the device is on
 * @param bus       Sub bus
 * @param dev       Device ID
 * @param fn        Device sub function
 * @param reg       Register to access
 * @param val       Value to write
 */
void cvmx_pcie_config_write16(int pcie_port, int bus, int dev, int fn, int reg, uint16_t val)
{
	u64 address;
	int node = (pcie_port >> 4) & 0x3;

	pcie_port &= 0x3;
	address = __cvmx_pcie_build_config_addr(node, pcie_port, bus, dev, fn, reg);
	if (address)
		cvmx_write64_uint16(address, cpu_to_le16(val));
}

/**
 * Write 32bits to a Device's config space
 *
 * @param pcie_port PCIe port the device is on
 * @param bus       Sub bus
 * @param dev       Device ID
 * @param fn        Device sub function
 * @param reg       Register to access
 * @param val       Value to write
 */
void cvmx_pcie_config_write32(int pcie_port, int bus, int dev, int fn, int reg, uint32_t val)
{
	u64 address;
	int node = (pcie_port >> 4) & 0x3;

	pcie_port &= 0x3;
	address = __cvmx_pcie_build_config_addr(node, pcie_port, bus, dev, fn, reg);
	if (address)
		cvmx_write64_uint32(address, cpu_to_le32(val));
}

/**
 * Read a PCIe config space register indirectly. This is used for
 * registers of the form PCIEEP_CFG??? and PCIERC?_CFG???.
 *
 * @param pcie_port  PCIe port to read from
 * @param cfg_offset Address to read
 *
 * Return: Value read
 */
uint32_t cvmx_pcie_cfgx_read(int pcie_port, uint32_t cfg_offset)
{
	return cvmx_pcie_cfgx_read_node(0, pcie_port, cfg_offset);
}

uint32_t cvmx_pcie_cfgx_read_node(int node, int pcie_port, uint32_t cfg_offset)
{
	cvmx_pemx_cfg_rd_t pemx_cfg_rd;

	pemx_cfg_rd.u64 = 0;
	pemx_cfg_rd.s.addr = cfg_offset;
	CVMX_WRITE_CSR(CVMX_PEMX_CFG_RD(pcie_port), pemx_cfg_rd.u64);
	pemx_cfg_rd.u64 = CVMX_READ_CSR(CVMX_PEMX_CFG_RD(pcie_port));

	return pemx_cfg_rd.s.data;
}

/**
 * Write a PCIe config space register indirectly. This is used for
 * registers of the form PCIEEP_CFG??? and PCIERC?_CFG???.
 *
 * @param pcie_port  PCIe port to write to
 * @param cfg_offset Address to write
 * @param val        Value to write
 */
void cvmx_pcie_cfgx_write(int pcie_port, uint32_t cfg_offset, uint32_t val)
{
	cvmx_pcie_cfgx_write_node(0, pcie_port, cfg_offset, val);
}

void cvmx_pcie_cfgx_write_node(int node, int pcie_port, uint32_t cfg_offset, uint32_t val)
{
	cvmx_pemx_cfg_wr_t pemx_cfg_wr;

	pemx_cfg_wr.u64 = 0;
	pemx_cfg_wr.s.addr = cfg_offset;
	pemx_cfg_wr.s.data = val;
	CVMX_WRITE_CSR(CVMX_PEMX_CFG_WR(pcie_port), pemx_cfg_wr.u64);
}

extern int cvmx_pcie_is_host_mode(int pcie_port);

/**
 * Initialize a PCIe port for use in target(EP) mode.
 *
 * @param pcie_port PCIe port to initialize for a node
 *
 * Return: Zero on success
 */
int cvmx_pcie_ep_initialize(int pcie_port)
{
	int node = (pcie_port >> 4) & 0x3;

	if (cvmx_pcie_is_host_mode(pcie_port))
		return -1;

	pcie_port &= 0x3;

	/* CN63XX Pass 1.0 errata G-14395 requires the QLM De-emphasis be
	 * programmed
	 */
	if (OCTEON_IS_MODEL(OCTEON_CN63XX_PASS1_0)) {
		if (pcie_port) {
			cvmx_ciu_qlm1_t ciu_qlm;

			ciu_qlm.u64 = csr_rd(CVMX_CIU_QLM1);
			ciu_qlm.s.txbypass = 1;
			ciu_qlm.s.txdeemph = 5;
			ciu_qlm.s.txmargin = 0x17;
			csr_wr(CVMX_CIU_QLM1, ciu_qlm.u64);
		} else {
			cvmx_ciu_qlm0_t ciu_qlm;

			ciu_qlm.u64 = csr_rd(CVMX_CIU_QLM0);
			ciu_qlm.s.txbypass = 1;
			ciu_qlm.s.txdeemph = 5;
			ciu_qlm.s.txmargin = 0x17;
			csr_wr(CVMX_CIU_QLM0, ciu_qlm.u64);
		}
	}

	/* Enable bus master and memory */
	CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIEEPX_CFG001(pcie_port), 0x6);

	/* Max Payload Size (PCIE*_CFG030[MPS]) */
	/* Max Read Request Size (PCIE*_CFG030[MRRS]) */
	/* Relaxed-order, no-snoop enables (PCIE*_CFG030[RO_EN,NS_EN] */
	/* Error Message Enables (PCIE*_CFG030[CE_EN,NFE_EN,FE_EN,UR_EN]) */
	{
		cvmx_pcieepx_cfg030_t pcieepx_cfg030;

		pcieepx_cfg030.u32 = CVMX_PCIE_CFGX_READ(pcie_port, CVMX_PCIEEPX_CFG030(pcie_port));
		pcieepx_cfg030.s.mps = MPS_CN6XXX;
		pcieepx_cfg030.s.mrrs = MRRS_CN6XXX;
		pcieepx_cfg030.s.ro_en = 1;  /* Enable relaxed ordering. */
		pcieepx_cfg030.s.ns_en = 1;  /* Enable no snoop. */
		pcieepx_cfg030.s.ce_en = 1;  /* Correctable error reporting enable. */
		pcieepx_cfg030.s.nfe_en = 1; /* Non-fatal error reporting enable. */
		pcieepx_cfg030.s.fe_en = 1;  /* Fatal error reporting enable. */
		pcieepx_cfg030.s.ur_en = 1;  /* Unsupported request reporting enable. */
		CVMX_PCIE_CFGX_WRITE(pcie_port, CVMX_PCIEEPX_CFG030(pcie_port), pcieepx_cfg030.u32);
	}

	/* Max Payload Size (DPI_SLI_PRTX_CFG[MPS]) must match
	 * PCIE*_CFG030[MPS]
	 */
	/* Max Read Request Size (DPI_SLI_PRTX_CFG[MRRS]) must not
	 * exceed PCIE*_CFG030[MRRS]
	 */
	cvmx_dpi_sli_prtx_cfg_t prt_cfg;
	cvmx_sli_s2m_portx_ctl_t sli_s2m_portx_ctl;

	prt_cfg.u64 = CVMX_READ_CSR(CVMX_DPI_SLI_PRTX_CFG(pcie_port));
	prt_cfg.s.mps = MPS_CN6XXX;
	prt_cfg.s.mrrs = MRRS_CN6XXX;
	/* Max outstanding load request. */
	prt_cfg.s.molr = 32;
	CVMX_WRITE_CSR(CVMX_DPI_SLI_PRTX_CFG(pcie_port), prt_cfg.u64);

	sli_s2m_portx_ctl.u64 = CVMX_READ_CSR(CVMX_PEXP_SLI_S2M_PORTX_CTL(pcie_port));
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) ||
	      OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		sli_s2m_portx_ctl.cn61xx.mrrs = MRRS_CN6XXX;
	CVMX_WRITE_CSR(CVMX_PEXP_SLI_S2M_PORTX_CTL(pcie_port), sli_s2m_portx_ctl.u64);

	/* Setup Mem access SubDID 12 to access Host memory */
	cvmx_sli_mem_access_subidx_t mem_access_subid;

	mem_access_subid.u64 = 0;
	mem_access_subid.s.port = pcie_port; /* Port the request is sent to. */
	mem_access_subid.s.nmerge = 0;	     /* Merging is allowed in this window. */
	mem_access_subid.s.esr = 0;	     /* Endian-swap for Reads. */
	mem_access_subid.s.esw = 0;	     /* Endian-swap for Writes. */
	mem_access_subid.s.wtype = 0;	     /* "No snoop" and "Relaxed ordering" are not set */
	mem_access_subid.s.rtype = 0;	     /* "No snoop" and "Relaxed ordering" are not set */
	/* PCIe Address Bits <63:34>. */
	if (OCTEON_IS_MODEL(OCTEON_CN68XX))
		mem_access_subid.cn68xx.ba = 0;
	else
		mem_access_subid.cn63xx.ba = 0;
	CVMX_WRITE_CSR(CVMX_PEXP_SLI_MEM_ACCESS_SUBIDX(12 + pcie_port * 4), mem_access_subid.u64);

	return 0;
}

/**
 * Wait for posted PCIe read/writes to reach the other side of
 * the internal PCIe switch. This will insure that core
 * read/writes are posted before anything after this function
 * is called. This may be necessary when writing to memory that
 * will later be read using the DMA/PKT engines.
 *
 * @param pcie_port PCIe port to wait for
 */
void cvmx_pcie_wait_for_pending(int pcie_port)
{
	cvmx_sli_data_out_cnt_t sli_data_out_cnt;
	int a;
	int b;
	int c;

	sli_data_out_cnt.u64 = csr_rd(CVMX_PEXP_SLI_DATA_OUT_CNT);
	if (pcie_port) {
		if (!sli_data_out_cnt.s.p1_fcnt)
			return;
		a = sli_data_out_cnt.s.p1_ucnt;
		b = (a + sli_data_out_cnt.s.p1_fcnt - 1) & 0xffff;
	} else {
		if (!sli_data_out_cnt.s.p0_fcnt)
			return;
		a = sli_data_out_cnt.s.p0_ucnt;
		b = (a + sli_data_out_cnt.s.p0_fcnt - 1) & 0xffff;
	}

	while (1) {
		sli_data_out_cnt.u64 = csr_rd(CVMX_PEXP_SLI_DATA_OUT_CNT);
		c = (pcie_port) ? sli_data_out_cnt.s.p1_ucnt :
			sli_data_out_cnt.s.p0_ucnt;
		if (a <= b) {
			if (c < a || c > b)
				return;
		} else {
			if (c > b && c < a)
				return;
		}
	}
}

/**
 * Returns if a PCIe port is in host or target mode.
 *
 * @param pcie_port PCIe port number (PEM number)
 *
 * Return: 0 if PCIe port is in target mode, !0 if in host mode.
 */
int cvmx_pcie_is_host_mode(int pcie_port)
{
	int node = (pcie_port >> 4) & 0x3;
	cvmx_mio_rst_ctlx_t mio_rst_ctl;

	pcie_port &= 0x3;
	if (OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN73XX)) {
		cvmx_pemx_strap_t strap;

		strap.u64 = CVMX_READ_CSR(CVMX_PEMX_STRAP(pcie_port));
		return (strap.cn78xx.pimode == 3);
	} else if (OCTEON_IS_MODEL(OCTEON_CN70XX)) {
		cvmx_rst_ctlx_t rst_ctl;

		rst_ctl.u64 = csr_rd(CVMX_RST_CTLX(pcie_port));
		return !!rst_ctl.s.host_mode;
	}

	mio_rst_ctl.u64 = csr_rd(CVMX_MIO_RST_CTLX(pcie_port));
	if (OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX))
		return mio_rst_ctl.s.prtmode != 0;
	else
		return !!mio_rst_ctl.s.host_mode;
}
