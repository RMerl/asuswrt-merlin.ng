/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * sh_eth.h - Driver for Renesas SuperH ethernet controller.
 *
 * Copyright (C) 2008 - 2012 Renesas Solutions Corp.
 * Copyright (c) 2008 - 2012 Nobuhiro Iwamatsu
 * Copyright (c) 2007 Carlos Munoz <carlos@kenati.com>
 */

#include <netdev.h>
#include <asm/types.h>

#define SHETHER_NAME "sh_eth"

#if defined(CONFIG_SH)
/* Malloc returns addresses in the P1 area (cacheable). However we need to
   use area P2 (non-cacheable) */
#define ADDR_TO_P2(addr)	((((int)(addr) & ~0xe0000000) | 0xa0000000))

/* The ethernet controller needs to use physical addresses */
#if defined(CONFIG_SH_32BIT)
#define ADDR_TO_PHY(addr)	((((int)(addr) & ~0xe0000000) | 0x40000000))
#else
#define ADDR_TO_PHY(addr)	((int)(addr) & ~0xe0000000)
#endif
#elif defined(CONFIG_ARM)
#ifndef inl
#define inl	readl
#define outl	writel
#endif
#define ADDR_TO_PHY(addr)	((int)(addr))
#define ADDR_TO_P2(addr)	(addr)
#endif /* defined(CONFIG_SH) */

/* base padding size is 16 */
#ifndef CONFIG_SH_ETHER_ALIGNE_SIZE
#define CONFIG_SH_ETHER_ALIGNE_SIZE 16
#endif

/* Number of supported ports */
#define MAX_PORT_NUM	2

/* Buffers must be big enough to hold the largest ethernet frame. Also, rx
   buffers must be a multiple of 32 bytes */
#define MAX_BUF_SIZE	(48 * 32)

/* The number of tx descriptors must be large enough to point to 5 or more
   frames. If each frame uses 2 descriptors, at least 10 descriptors are needed.
   We use one descriptor per frame */
#define NUM_TX_DESC		8

/* The size of the tx descriptor is determined by how much padding is used.
   4, 20, or 52 bytes of padding can be used */
#define TX_DESC_PADDING	(CONFIG_SH_ETHER_ALIGNE_SIZE - 12)

/* Tx descriptor. We always use 3 bytes of padding */
struct tx_desc_s {
	volatile u32 td0;
	u32 td1;
	u32 td2;		/* Buffer start */
	u8 padding[TX_DESC_PADDING];	/* aligned cache line size */
};

/* There is no limitation in the number of rx descriptors */
#define NUM_RX_DESC	8

/* The size of the rx descriptor is determined by how much padding is used.
   4, 20, or 52 bytes of padding can be used */
#define RX_DESC_PADDING	(CONFIG_SH_ETHER_ALIGNE_SIZE - 12)
/* aligned cache line size */
#define RX_BUF_ALIGNE_SIZE	(CONFIG_SH_ETHER_ALIGNE_SIZE > 32 ? 64 : 32)

/* Rx descriptor. We always use 4 bytes of padding */
struct rx_desc_s {
	volatile u32 rd0;
	volatile u32 rd1;
	u32 rd2;		/* Buffer start */
	u8 padding[TX_DESC_PADDING];	/* aligned cache line size */
};

struct sh_eth_info {
	struct tx_desc_s *tx_desc_alloc;
	struct tx_desc_s *tx_desc_base;
	struct tx_desc_s *tx_desc_cur;
	struct rx_desc_s *rx_desc_alloc;
	struct rx_desc_s *rx_desc_base;
	struct rx_desc_s *rx_desc_cur;
	u8 *rx_buf_alloc;
	u8 *rx_buf_base;
	u8 mac_addr[6];
	u8 phy_addr;
	struct eth_device *dev;
	struct phy_device *phydev;
	void __iomem *iobase;
};

struct sh_eth_dev {
	int port;
	struct sh_eth_info port_info[MAX_PORT_NUM];
};

/* from linux/drivers/net/ethernet/renesas/sh_eth.h */
enum {
	/* E-DMAC registers */
	EDSR = 0,
	EDMR,
	EDTRR,
	EDRRR,
	EESR,
	EESIPR,
	TDLAR,
	TDFAR,
	TDFXR,
	TDFFR,
	RDLAR,
	RDFAR,
	RDFXR,
	RDFFR,
	TRSCER,
	RMFCR,
	TFTR,
	FDR,
	RMCR,
	EDOCR,
	TFUCR,
	RFOCR,
	FCFTR,
	RPADIR,
	TRIMD,
	RBWAR,
	TBRAR,

	/* Ether registers */
	ECMR,
	ECSR,
	ECSIPR,
	PIR,
	PSR,
	RDMLR,
	PIPR,
	RFLR,
	IPGR,
	APR,
	MPR,
	PFTCR,
	PFRCR,
	RFCR,
	RFCF,
	TPAUSER,
	TPAUSECR,
	BCFR,
	BCFRR,
	GECMR,
	BCULR,
	MAHR,
	MALR,
	TROCR,
	CDCR,
	LCCR,
	CNDCR,
	CEFCR,
	FRECR,
	TSFRCR,
	TLFRCR,
	CERCR,
	CEECR,
	RMIIMR, /* R8A7790 */
	MAFCR,
	RTRATE,
	CSMR,
	RMII_MII,

	/* This value must be written at last. */
	SH_ETH_MAX_REGISTER_OFFSET,
};

static const u16 sh_eth_offset_gigabit[SH_ETH_MAX_REGISTER_OFFSET] = {
	[EDSR]	= 0x0000,
	[EDMR]	= 0x0400,
	[EDTRR]	= 0x0408,
	[EDRRR]	= 0x0410,
	[EESR]	= 0x0428,
	[EESIPR]	= 0x0430,
	[TDLAR]	= 0x0010,
	[TDFAR]	= 0x0014,
	[TDFXR]	= 0x0018,
	[TDFFR]	= 0x001c,
	[RDLAR]	= 0x0030,
	[RDFAR]	= 0x0034,
	[RDFXR]	= 0x0038,
	[RDFFR]	= 0x003c,
	[TRSCER]	= 0x0438,
	[RMFCR]	= 0x0440,
	[TFTR]	= 0x0448,
	[FDR]	= 0x0450,
	[RMCR]	= 0x0458,
	[RPADIR]	= 0x0460,
	[FCFTR]	= 0x0468,
	[CSMR] = 0x04E4,

	[ECMR]	= 0x0500,
	[ECSR]	= 0x0510,
	[ECSIPR]	= 0x0518,
	[PIR]	= 0x0520,
	[PSR]	= 0x0528,
	[PIPR]	= 0x052c,
	[RFLR]	= 0x0508,
	[APR]	= 0x0554,
	[MPR]	= 0x0558,
	[PFTCR]	= 0x055c,
	[PFRCR]	= 0x0560,
	[TPAUSER]	= 0x0564,
	[GECMR]	= 0x05b0,
	[BCULR]	= 0x05b4,
	[MAHR]	= 0x05c0,
	[MALR]	= 0x05c8,
	[TROCR]	= 0x0700,
	[CDCR]	= 0x0708,
	[LCCR]	= 0x0710,
	[CEFCR]	= 0x0740,
	[FRECR]	= 0x0748,
	[TSFRCR]	= 0x0750,
	[TLFRCR]	= 0x0758,
	[RFCR]	= 0x0760,
	[CERCR]	= 0x0768,
	[CEECR]	= 0x0770,
	[MAFCR]	= 0x0778,
	[RMII_MII] =  0x0790,
};

static const u16 sh_eth_offset_rz[SH_ETH_MAX_REGISTER_OFFSET] = {
	[EDSR]	= 0x0000,
	[EDMR]	= 0x0400,
	[EDTRR]	= 0x0408,
	[EDRRR]	= 0x0410,
	[EESR]	= 0x0428,
	[EESIPR]	= 0x0430,
	[TDLAR]	= 0x0010,
	[TDFAR]	= 0x0014,
	[TDFXR]	= 0x0018,
	[TDFFR]	= 0x001c,
	[RDLAR]	= 0x0030,
	[RDFAR]	= 0x0034,
	[RDFXR]	= 0x0038,
	[RDFFR]	= 0x003c,
	[TRSCER]	= 0x0438,
	[RMFCR]	= 0x0440,
	[TFTR]	= 0x0448,
	[FDR]	= 0x0450,
	[RMCR]	= 0x0458,
	[RPADIR]	= 0x0460,
	[FCFTR]	= 0x0468,
	[CSMR] = 0x04E4,

	[ECMR]	= 0x0500,
	[ECSR]	= 0x0510,
	[ECSIPR]	= 0x0518,
	[PIR]	= 0x0520,
	[PSR]	= 0x0528,
	[PIPR]	= 0x052c,
	[RFLR]	= 0x0508,
	[APR]	= 0x0554,
	[MPR]	= 0x0558,
	[PFTCR]	= 0x055c,
	[PFRCR]	= 0x0560,
	[TPAUSER]	= 0x0564,
	[GECMR]	= 0x05b0,
	[BCULR]	= 0x05b4,
	[MAHR]	= 0x05c0,
	[MALR]	= 0x05c8,
	[TROCR]	= 0x0700,
	[CDCR]	= 0x0708,
	[LCCR]	= 0x0710,
	[CEFCR]	= 0x0740,
	[FRECR]	= 0x0748,
	[TSFRCR]	= 0x0750,
	[TLFRCR]	= 0x0758,
	[RFCR]	= 0x0760,
	[CERCR]	= 0x0768,
	[CEECR]	= 0x0770,
	[MAFCR]	= 0x0778,
	[RMII_MII] =  0x0790,
};

static const u16 sh_eth_offset_fast_sh4[SH_ETH_MAX_REGISTER_OFFSET] = {
	[ECMR]	= 0x0100,
	[RFLR]	= 0x0108,
	[ECSR]	= 0x0110,
	[ECSIPR]	= 0x0118,
	[PIR]	= 0x0120,
	[PSR]	= 0x0128,
	[RDMLR]	= 0x0140,
	[IPGR]	= 0x0150,
	[APR]	= 0x0154,
	[MPR]	= 0x0158,
	[TPAUSER]	= 0x0164,
	[RFCF]	= 0x0160,
	[TPAUSECR]	= 0x0168,
	[BCFRR]	= 0x016c,
	[MAHR]	= 0x01c0,
	[MALR]	= 0x01c8,
	[TROCR]	= 0x01d0,
	[CDCR]	= 0x01d4,
	[LCCR]	= 0x01d8,
	[CNDCR]	= 0x01dc,
	[CEFCR]	= 0x01e4,
	[FRECR]	= 0x01e8,
	[TSFRCR]	= 0x01ec,
	[TLFRCR]	= 0x01f0,
	[RFCR]	= 0x01f4,
	[MAFCR]	= 0x01f8,
	[RTRATE]	= 0x01fc,

	[EDMR]	= 0x0000,
	[EDTRR]	= 0x0008,
	[EDRRR]	= 0x0010,
	[TDLAR]	= 0x0018,
	[RDLAR]	= 0x0020,
	[EESR]	= 0x0028,
	[EESIPR]	= 0x0030,
	[TRSCER]	= 0x0038,
	[RMFCR]	= 0x0040,
	[TFTR]	= 0x0048,
	[FDR]	= 0x0050,
	[RMCR]	= 0x0058,
	[TFUCR]	= 0x0064,
	[RFOCR]	= 0x0068,
	[RMIIMR] = 0x006C,
	[FCFTR]	= 0x0070,
	[RPADIR]	= 0x0078,
	[TRIMD]	= 0x007c,
	[RBWAR]	= 0x00c8,
	[RDFAR]	= 0x00cc,
	[TBRAR]	= 0x00d4,
	[TDFAR]	= 0x00d8,
};

/* Register Address */
#if defined(CONFIG_CPU_SH7763) || defined(CONFIG_CPU_SH7734)
#define SH_ETH_TYPE_GETHER
#define BASE_IO_ADDR	0xfee00000
#elif defined(CONFIG_CPU_SH7757) || \
	defined(CONFIG_CPU_SH7752) || \
	defined(CONFIG_CPU_SH7753)
#if defined(CONFIG_SH_ETHER_USE_GETHER)
#define SH_ETH_TYPE_GETHER
#define BASE_IO_ADDR	0xfee00000
#else
#define SH_ETH_TYPE_ETHER
#define BASE_IO_ADDR	0xfef00000
#endif
#elif defined(CONFIG_R8A7740)
#define SH_ETH_TYPE_GETHER
#define BASE_IO_ADDR	0xE9A00000
#elif defined(CONFIG_RCAR_GEN2)
#define SH_ETH_TYPE_ETHER
#define BASE_IO_ADDR	0xEE700200
#elif defined(CONFIG_R7S72100)
#define SH_ETH_TYPE_RZ
#define BASE_IO_ADDR	0xE8203000
#endif

/*
 * Register's bits
 * Copy from Linux driver source code
 */
#if defined(SH_ETH_TYPE_GETHER) || defined(SH_ETH_TYPE_RZ)
/* EDSR */
enum EDSR_BIT {
	EDSR_ENT = 0x01, EDSR_ENR = 0x02,
};
#define EDSR_ENALL (EDSR_ENT|EDSR_ENR)
#endif

/* EDMR */
enum DMAC_M_BIT {
	EDMR_DL1 = 0x20, EDMR_DL0 = 0x10,
#if defined(SH_ETH_TYPE_GETHER) || defined(SH_ETH_TYPE_RZ)
	EDMR_SRST	= 0x03, /* Receive/Send reset */
	EMDR_DESC_R	= 0x30, /* Descriptor reserve size */
	EDMR_EL		= 0x40, /* Litte endian */
#elif defined(SH_ETH_TYPE_ETHER)
	EDMR_SRST	= 0x01,
	EMDR_DESC_R	= 0x30, /* Descriptor reserve size */
	EDMR_EL		= 0x40, /* Litte endian */
#else
	EDMR_SRST = 0x01,
#endif
};

#if CONFIG_SH_ETHER_ALIGNE_SIZE == 64
# define EMDR_DESC EDMR_DL1
#elif CONFIG_SH_ETHER_ALIGNE_SIZE == 32
# define EMDR_DESC EDMR_DL0
#elif CONFIG_SH_ETHER_ALIGNE_SIZE == 16 /* Default */
# define EMDR_DESC 0
#endif

/* RFLR */
#define RFLR_RFL_MIN	0x05EE	/* Recv Frame length 1518 byte */

/* EDTRR */
enum DMAC_T_BIT {
#if defined(SH_ETH_TYPE_GETHER) || defined(SH_ETH_TYPE_RZ)
	EDTRR_TRNS = 0x03,
#else
	EDTRR_TRNS = 0x01,
#endif
};

/* GECMR */
enum GECMR_BIT {
#if defined(CONFIG_CPU_SH7757) || \
	defined(CONFIG_CPU_SH7752) || \
	defined(CONFIG_CPU_SH7753)
	GECMR_1000B = 0x20, GECMR_100B = 0x01, GECMR_10B = 0x00,
#else
	GECMR_1000B = 0x01, GECMR_100B = 0x04, GECMR_10B = 0x00,
#endif
};

/* EDRRR*/
enum EDRRR_R_BIT {
	EDRRR_R = 0x01,
};

/* TPAUSER */
enum TPAUSER_BIT {
	TPAUSER_TPAUSE = 0x0000ffff,
	TPAUSER_UNLIMITED = 0,
};

/* BCFR */
enum BCFR_BIT {
	BCFR_RPAUSE = 0x0000ffff,
	BCFR_UNLIMITED = 0,
};

/* PIR */
enum PIR_BIT {
	PIR_MDI = 0x08, PIR_MDO = 0x04, PIR_MMD = 0x02, PIR_MDC = 0x01,
};

/* PSR */
enum PHY_STATUS_BIT { PHY_ST_LINK = 0x01, };

/* EESR */
enum EESR_BIT {
#if defined(SH_ETH_TYPE_ETHER)
	EESR_TWB  = 0x40000000,
#else
	EESR_TWB  = 0xC0000000,
	EESR_TC1  = 0x20000000,
	EESR_TUC  = 0x10000000,
	EESR_ROC  = 0x80000000,
#endif
	EESR_TABT = 0x04000000,
	EESR_RABT = 0x02000000, EESR_RFRMER = 0x01000000,
#if defined(SH_ETH_TYPE_ETHER)
	EESR_ADE  = 0x00800000,
#endif
	EESR_ECI  = 0x00400000,
	EESR_FTC  = 0x00200000, EESR_TDE  = 0x00100000,
	EESR_TFE  = 0x00080000, EESR_FRC  = 0x00040000,
	EESR_RDE  = 0x00020000, EESR_RFE  = 0x00010000,
#if defined(SH_ETH_TYPE_ETHER)
	EESR_CND  = 0x00000800,
#endif
	EESR_DLC  = 0x00000400,
	EESR_CD   = 0x00000200, EESR_RTO  = 0x00000100,
	EESR_RMAF = 0x00000080, EESR_CEEF = 0x00000040,
	EESR_CELF = 0x00000020, EESR_RRF  = 0x00000010,
	EESR_RTLF = 0x00000008, EESR_RTSF = 0x00000004,
	EESR_PRE  = 0x00000002, EESR_CERF = 0x00000001,
};


#if defined(SH_ETH_TYPE_GETHER) || defined(SH_ETH_TYPE_RZ)
# define TX_CHECK (EESR_TC1 | EESR_FTC)
# define EESR_ERR_CHECK	(EESR_TWB | EESR_TABT | EESR_RABT | EESR_RDE \
		| EESR_RFRMER | EESR_TFE | EESR_TDE | EESR_ECI)
# define TX_ERROR_CEHCK (EESR_TWB | EESR_TABT | EESR_TDE | EESR_TFE)

#else
# define TX_CHECK (EESR_FTC | EESR_CND | EESR_DLC | EESR_CD | EESR_RTO)
# define EESR_ERR_CHECK	(EESR_TWB | EESR_TABT | EESR_RABT | EESR_RDE \
		| EESR_RFRMER | EESR_ADE | EESR_TFE | EESR_TDE | EESR_ECI)
# define TX_ERROR_CEHCK (EESR_TWB | EESR_TABT | EESR_ADE | EESR_TDE | EESR_TFE)
#endif

/* EESIPR */
enum DMAC_IM_BIT {
	DMAC_M_TWB = 0x40000000, DMAC_M_TABT = 0x04000000,
	DMAC_M_RABT = 0x02000000,
	DMAC_M_RFRMER = 0x01000000, DMAC_M_ADF = 0x00800000,
	DMAC_M_ECI = 0x00400000, DMAC_M_FTC = 0x00200000,
	DMAC_M_TDE = 0x00100000, DMAC_M_TFE = 0x00080000,
	DMAC_M_FRC = 0x00040000, DMAC_M_RDE = 0x00020000,
	DMAC_M_RFE = 0x00010000, DMAC_M_TINT4 = 0x00000800,
	DMAC_M_TINT3 = 0x00000400, DMAC_M_TINT2 = 0x00000200,
	DMAC_M_TINT1 = 0x00000100, DMAC_M_RINT8 = 0x00000080,
	DMAC_M_RINT5 = 0x00000010, DMAC_M_RINT4 = 0x00000008,
	DMAC_M_RINT3 = 0x00000004, DMAC_M_RINT2 = 0x00000002,
	DMAC_M_RINT1 = 0x00000001,
};

/* Receive descriptor bit */
enum RD_STS_BIT {
	RD_RACT = 0x80000000, RD_RDLE = 0x40000000,
	RD_RFP1 = 0x20000000, RD_RFP0 = 0x10000000,
	RD_RFE = 0x08000000, RD_RFS10 = 0x00000200,
	RD_RFS9 = 0x00000100, RD_RFS8 = 0x00000080,
	RD_RFS7 = 0x00000040, RD_RFS6 = 0x00000020,
	RD_RFS5 = 0x00000010, RD_RFS4 = 0x00000008,
	RD_RFS3 = 0x00000004, RD_RFS2 = 0x00000002,
	RD_RFS1 = 0x00000001,
};
#define RDF1ST	RD_RFP1
#define RDFEND	RD_RFP0
#define RD_RFP	(RD_RFP1|RD_RFP0)

/* RDFFR*/
enum RDFFR_BIT {
	RDFFR_RDLF = 0x01,
};

/* FCFTR */
enum FCFTR_BIT {
	FCFTR_RFF2 = 0x00040000, FCFTR_RFF1 = 0x00020000,
	FCFTR_RFF0 = 0x00010000, FCFTR_RFD2 = 0x00000004,
	FCFTR_RFD1 = 0x00000002, FCFTR_RFD0 = 0x00000001,
};
#define FIFO_F_D_RFF	(FCFTR_RFF2|FCFTR_RFF1|FCFTR_RFF0)
#define FIFO_F_D_RFD	(FCFTR_RFD2|FCFTR_RFD1|FCFTR_RFD0)

/* Transfer descriptor bit */
enum TD_STS_BIT {
#if defined(SH_ETH_TYPE_GETHER) || defined(SH_ETH_TYPE_ETHER) || \
	defined(SH_ETH_TYPE_RZ)
	TD_TACT = 0x80000000,
#else
	TD_TACT = 0x7fffffff,
#endif
	TD_TDLE = 0x40000000, TD_TFP1 = 0x20000000,
	TD_TFP0 = 0x10000000,
};
#define TDF1ST	TD_TFP1
#define TDFEND	TD_TFP0
#define TD_TFP	(TD_TFP1|TD_TFP0)

/* RMCR */
enum RECV_RST_BIT { RMCR_RST = 0x01, };
/* ECMR */
enum FELIC_MODE_BIT {
#if defined(SH_ETH_TYPE_GETHER) || defined(SH_ETH_TYPE_RZ)
	ECMR_TRCCM = 0x04000000, ECMR_RCSC = 0x00800000,
	ECMR_DPAD = 0x00200000, ECMR_RZPF = 0x00100000,
#endif
	ECMR_ZPF = 0x00080000, ECMR_PFR = 0x00040000, ECMR_RXF = 0x00020000,
	ECMR_TXF = 0x00010000, ECMR_MCT = 0x00002000, ECMR_PRCEF = 0x00001000,
	ECMR_PMDE = 0x00000200, ECMR_RE = 0x00000040, ECMR_TE = 0x00000020,
	ECMR_ILB = 0x00000008, ECMR_ELB = 0x00000004, ECMR_DM = 0x00000002,
	ECMR_PRM = 0x00000001,
#ifdef CONFIG_CPU_SH7724
	ECMR_RTM = 0x00000010,
#elif defined(CONFIG_RCAR_GEN2)
	ECMR_RTM = 0x00000004,
#endif

};

#if defined(SH_ETH_TYPE_GETHER) || defined(SH_ETH_TYPE_RZ)
#define ECMR_CHG_DM (ECMR_TRCCM | ECMR_RZPF | ECMR_ZPF | ECMR_PFR | \
			ECMR_RXF | ECMR_TXF | ECMR_MCT)
#elif defined(SH_ETH_TYPE_ETHER)
#define ECMR_CHG_DM (ECMR_ZPF | ECMR_PFR | ECMR_RXF | ECMR_TXF)
#else
#define ECMR_CHG_DM	(ECMR_ZPF | ECMR_PFR | ECMR_RXF | ECMR_TXF | ECMR_MCT)
#endif

/* ECSR */
enum ECSR_STATUS_BIT {
#if defined(SH_ETH_TYPE_ETHER)
	ECSR_BRCRX = 0x20, ECSR_PSRTO = 0x10,
#endif
	ECSR_LCHNG = 0x04,
	ECSR_MPD = 0x02, ECSR_ICD = 0x01,
};

#if defined(SH_ETH_TYPE_GETHER) || defined(SH_ETH_TYPE_RZ)
# define ECSR_INIT (ECSR_ICD | ECSIPR_MPDIP)
#else
# define ECSR_INIT (ECSR_BRCRX | ECSR_PSRTO | \
			ECSR_LCHNG | ECSR_ICD | ECSIPR_MPDIP)
#endif

/* ECSIPR */
enum ECSIPR_STATUS_MASK_BIT {
#if defined(SH_ETH_TYPE_ETHER)
	ECSIPR_BRCRXIP = 0x20,
	ECSIPR_PSRTOIP = 0x10,
#elif defined(SH_ETY_TYPE_GETHER)
	ECSIPR_PSRTOIP = 0x10,
	ECSIPR_PHYIP = 0x08,
#endif
	ECSIPR_LCHNGIP = 0x04,
	ECSIPR_MPDIP = 0x02,
	ECSIPR_ICDIP = 0x01,
};

#if defined(SH_ETH_TYPE_GETHER) || defined(SH_ETH_TYPE_RZ)
# define ECSIPR_INIT (ECSIPR_LCHNGIP | ECSIPR_ICDIP | ECSIPR_MPDIP)
#else
# define ECSIPR_INIT (ECSIPR_BRCRXIP | ECSIPR_PSRTOIP | ECSIPR_LCHNGIP | \
				ECSIPR_ICDIP | ECSIPR_MPDIP)
#endif

/* APR */
enum APR_BIT {
	APR_AP = 0x00000004,
};

/* MPR */
enum MPR_BIT {
	MPR_MP = 0x00000006,
};

/* TRSCER */
enum DESC_I_BIT {
	DESC_I_TINT4 = 0x0800, DESC_I_TINT3 = 0x0400, DESC_I_TINT2 = 0x0200,
	DESC_I_TINT1 = 0x0100, DESC_I_RINT8 = 0x0080, DESC_I_RINT5 = 0x0010,
	DESC_I_RINT4 = 0x0008, DESC_I_RINT3 = 0x0004, DESC_I_RINT2 = 0x0002,
	DESC_I_RINT1 = 0x0001,
};

/* RPADIR */
enum RPADIR_BIT {
	RPADIR_PADS1 = 0x20000, RPADIR_PADS0 = 0x10000,
	RPADIR_PADR = 0x0003f,
};

#if defined(SH_ETH_TYPE_GETHER) || defined(SH_ETH_TYPE_RZ)
# define RPADIR_INIT (0x00)
#else
# define RPADIR_INIT (RPADIR_PADS1)
#endif

/* FDR */
enum FIFO_SIZE_BIT {
	FIFO_SIZE_T = 0x00000700, FIFO_SIZE_R = 0x00000007,
};

static inline unsigned long sh_eth_reg_addr(struct sh_eth_info *port,
					    int enum_index)
{
#if defined(SH_ETH_TYPE_GETHER) || defined(SH_ETH_TYPE_RZ)
	const u16 *reg_offset = sh_eth_offset_gigabit;
#elif defined(SH_ETH_TYPE_ETHER)
	const u16 *reg_offset = sh_eth_offset_fast_sh4;
#elif defined(SH_ETH_TYPE_RZ)
	const u16 *reg_offset = sh_eth_offset_rz;
#else
#error
#endif
	return (unsigned long)port->iobase + reg_offset[enum_index];
}

static inline void sh_eth_write(struct sh_eth_info *port, unsigned long data,
				int enum_index)
{
	outl(data, sh_eth_reg_addr(port, enum_index));
}

static inline unsigned long sh_eth_read(struct sh_eth_info *port,
					int enum_index)
{
	return inl(sh_eth_reg_addr(port, enum_index));
}
