/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Common internal memory map for some Freescale SoCs
 *
 * Copyright 2014 Freescale Semiconductor, Inc.
 * Copyright 2018, 2021 NXP
 */

#ifndef __FSL_SEC_H
#define __FSL_SEC_H

#include <asm/io.h>

#ifdef CONFIG_SYS_FSL_SEC_LE
#define sec_in32(a)       in_le32((ulong *)(ulong)(a))
#define sec_out32(a, v)   out_le32((ulong *)(ulong)(a), v)
#define sec_in16(a)       in_le16(a)
#define sec_clrbits32     clrbits_le32
#define sec_setbits32     setbits_le32
#elif defined(CONFIG_SYS_FSL_SEC_BE)
#define sec_in32(a)       in_be32(a)
#define sec_out32(a, v)   out_be32(a, v)
#define sec_in16(a)       in_be16(a)
#define sec_clrbits32     clrbits_be32
#define sec_setbits32     setbits_be32
#elif defined(CONFIG_SYS_FSL_HAS_SEC)
#error Neither CONFIG_SYS_FSL_SEC_LE nor CONFIG_SYS_FSL_SEC_BE is defined
#endif

#define BLOB_SIZE(x)		((x) + 32 + 16) /* Blob buffer size */

/* Security Engine Block (MS = Most Sig., LS = Least Sig.) */
#if CONFIG_SYS_FSL_SEC_COMPAT >= 4
/* RNG4 TRNG test registers */
struct rng4tst {
#define RTMCTL_PRGM 0x00010000	/* 1 -> program mode, 0 -> run mode */
#define RTMCTL_SAMP_MODE_VON_NEUMANN_ES_SC     0 /* use von Neumann data in
						    both entropy shifter and
						    statistical checker */
#define RTMCTL_SAMP_MODE_RAW_ES_SC             1 /* use raw data in both
						    entropy shifter and
						    statistical checker */
#define RTMCTL_SAMP_MODE_VON_NEUMANN_ES_RAW_SC 2 /* use von Neumann data in
						    entropy shifter, raw data
						    in statistical checker */
#define RTMCTL_SAMP_MODE_INVALID               3 /* invalid combination */
	u32 rtmctl;		/* misc. control register */
	u32 rtscmisc;		/* statistical check misc. register */
	u32 rtpkrrng;		/* poker range register */
#ifdef CONFIG_MX6SX
#define RTSDCTL_ENT_DLY		12000
#else
#define RTSDCTL_ENT_DLY		3200
#endif
#define RTSDCTL_ENT_DLY_MAX	12800
	union {
		u32 rtpkrmax;	/* PRGM=1: poker max. limit register */
		u32 rtpkrsq;	/* PRGM=0: poker square calc. result register */
	};
#define RTSDCTL_ENT_DLY_SHIFT 16
#define RTSDCTL_ENT_DLY_MASK (0xffff << RTSDCTL_ENT_DLY_SHIFT)
	u32 rtsdctl;		/* seed control register */
	union {
		u32 rtsblim;	/* PRGM=1: sparse bit limit register */
		u32 rttotsam;	/* PRGM=0: total samples register */
	};
	u32 rtfreqmin;		/* frequency count min. limit register */
#define RTFRQMAX_DISABLE       (1 << 20)
	union {
		u32 rtfreqmax;	/* PRGM=1: freq. count max. limit register */
		u32 rtfreqcnt;	/* PRGM=0: freq. count register */
	};
	u32 rsvd1[40];
#define RDSTA_IF(idx) (0x00000001 << (idx))
#define RDSTA_PR(idx) (0x00000010 << (idx))
#define RDSTA_MASK (RDSTA_PR(1) | RDSTA_PR(0) | RDSTA_IF(1) | RDSTA_IF(0))
#define RDSTA_SKVN 0x40000000
	u32 rdsta;		/*RNG DRNG Status Register*/
	u32 rsvd2[15];
};

/* Version registers (Era 10+) */
struct version_regs {
	u32 crca;	/* CRCA_VERSION */
	u32 afha;	/* AFHA_VERSION */
	u32 kfha;	/* KFHA_VERSION */
	u32 pkha;	/* PKHA_VERSION */
	u32 aesa;	/* AESA_VERSION */
	u32 mdha;	/* MDHA_VERSION */
	u32 desa;	/* DESA_VERSION */
	u32 snw8a;	/* SNW8A_VERSION */
	u32 snw9a;	/* SNW9A_VERSION */
	u32 zuce;	/* ZUCE_VERSION */
	u32 zuca;	/* ZUCA_VERSION */
	u32 ccha;	/* CCHA_VERSION */
	u32 ptha;	/* PTHA_VERSION */
	u32 rng;	/* RNG_VERSION */
	u32 trng;	/* TRNG_VERSION */
	u32 aaha;	/* AAHA_VERSION */
	u32 rsvd[10];
	u32 sr;		/* SR_VERSION */
	u32 dma;	/* DMA_VERSION */
	u32 ai;		/* AI_VERSION */
	u32 qi;		/* QI_VERSION */
	u32 jr;		/* JR_VERSION */
	u32 deco;	/* DECO_VERSION */
};

#define CHA_VER_NUM_MASK	0x000000ff
#define CHA_VER_MISC_SHIFT	8
#define CHA_VER_MISC_MASK	0x0000ff00
#define CHA_VER_REV_SHIFT	16
#define CHA_VER_REV_MASK	0x00ff0000
#define CHA_VER_VID_SHIFT	24
#define CHA_VER_VID_MASK	0xff000000

typedef struct ccsr_sec {
	u32	res0;
	u32	mcfgr;		/* Master CFG Register */
	u8	res1[0x4];
	u32	scfgr;
	struct {
		u32	ms;	/* Job Ring LIODN Register, MS */
		u32	ls;	/* Job Ring LIODN Register, LS */
	} jrliodnr[4];
	u8	res2[0x2c];
	u32	jrstartr;	/* Job Ring Start Register */
	struct {
		u32	ms;	/* RTIC LIODN Register, MS */
		u32	ls;	/* RTIC LIODN Register, LS */
	} rticliodnr[4];
	u8	res3[0x1c];
	u32	decorr;		/* DECO Request Register */
	struct {
		u32	ms;	/* DECO LIODN Register, MS */
		u32	ls;	/* DECO LIODN Register, LS */
	} decoliodnr[16];
	u32	dar;		/* DECO Avail Register */
	u32	drr;		/* DECO Reset Register */
	u8	res5[0x4d8];
	struct rng4tst rng;	/* RNG Registers */
	u8	res6[0x780];
	struct version_regs vreg; /* version registers since era 10 */
	u8	res7[0xa0];
	u32	crnr_ms;	/* CHA Revision Number Register, MS */
	u32	crnr_ls;	/* CHA Revision Number Register, LS */
	u32	ctpr_ms;	/* Compile Time Parameters Register, MS */
	u32	ctpr_ls;	/* Compile Time Parameters Register, LS */
	u8	res8[0x10];
	u32	far_ms;		/* Fault Address Register, MS */
	u32	far_ls;		/* Fault Address Register, LS */
	u32	falr;		/* Fault Address LIODN Register */
	u32	fadr;		/* Fault Address Detail Register */
	u8	res9[0x4];
	u32	csta;		/* CAAM Status Register */
	u32	smpart;		/* Secure Memory Partition Parameters */
	u32	smvid;		/* Secure Memory Version ID */
	u32	rvid;		/* Run Time Integrity Checking Version ID Reg.*/
	u32	ccbvid;		/* CHA Cluster Block Version ID Register */
	u32	chavid_ms;	/* CHA Version ID Register, MS */
	u32	chavid_ls;	/* CHA Version ID Register, LS */
	u32	chanum_ms;	/* CHA Number Register, MS */
	u32	chanum_ls;	/* CHA Number Register, LS */
	u32	secvid_ms;	/* SEC Version ID Register, MS */
	u32	secvid_ls;	/* SEC Version ID Register, LS */
#if defined(CONFIG_FSL_LSCH2) || defined(CONFIG_FSL_LSCH3)
	u8	res10[0x6f020];
#else
	u8	res10[0x6020];
#endif
	u32	qilcr_ms;	/* Queue Interface LIODN CFG Register, MS */
	u32	qilcr_ls;	/* Queue Interface LIODN CFG Register, LS */
#if defined(CONFIG_FSL_LSCH2) || defined(CONFIG_FSL_LSCH3)
	u8	res11[0x8ffd8];
#else
	u8	res11[0x8fd8];
#endif
} ccsr_sec_t;

#define SEC_CTPR_MS_AXI_LIODN		0x08000000
#define SEC_CTPR_MS_QI			0x02000000
#define SEC_CTPR_MS_VIRT_EN_INCL	0x00000001
#define SEC_CTPR_MS_VIRT_EN_POR		0x00000002
#define SEC_RVID_MA			0x0f000000
#define SEC_CHANUM_MS_JRNUM_MASK	0xf0000000
#define SEC_CHANUM_MS_JRNUM_SHIFT	28
#define SEC_CHANUM_MS_DECONUM_MASK	0x0f000000
#define SEC_CHANUM_MS_DECONUM_SHIFT	24
#define SEC_SECVID_MS_IPID_MASK	0xffff0000
#define SEC_SECVID_MS_IPID_SHIFT	16
#define SEC_SECVID_MS_MAJ_REV_MASK	0x0000ff00
#define SEC_SECVID_MS_MAJ_REV_SHIFT	8
#define SEC_CCBVID_ERA_MASK		0xff000000
#define SEC_CCBVID_ERA_SHIFT		24
#define SEC_SCFGR_RDBENABLE		0x00000400
#define SEC_SCFGR_VIRT_EN		0x00008000
#define SEC_CHAVID_LS_RNG_SHIFT		16
#define SEC_CHAVID_RNG_LS_MASK		0x000f0000

struct jr_regs {
#if defined(CONFIG_SYS_FSL_SEC_LE) && \
	!(defined(CONFIG_MX6) || defined(CONFIG_MX7) || \
	  defined(CONFIG_MX7ULP) || defined(CONFIG_IMX8M) || defined(CONFIG_IMX8))
	u32 irba_l;
	u32 irba_h;
#else
	u32 irba_h;
	u32 irba_l;
#endif
	u32 rsvd1;
	u32 irs;
	u32 rsvd2;
	u32 irsa;
	u32 rsvd3;
	u32 irja;
#if defined(CONFIG_SYS_FSL_SEC_LE) && \
	!(defined(CONFIG_MX6) || defined(CONFIG_MX7) || \
	  defined(CONFIG_MX7ULP) || defined(CONFIG_IMX8M) || defined(CONFIG_IMX8))
	u32 orba_l;
	u32 orba_h;
#else
	u32 orba_h;
	u32 orba_l;
#endif
	u32 rsvd4;
	u32 ors;
	u32 rsvd5;
	u32 orjr;
	u32 rsvd6;
	u32 orsf;
	u32 rsvd7;
	u32 jrsta;
	u32 rsvd8;
	u32 jrint;
	u32 jrcfg0;
	u32 jrcfg1;
	u32 rsvd9;
	u32 irri;
	u32 rsvd10;
	u32 orwi;
	u32 rsvd11;
	u32 jrcr;
};

/*
 * Scatter Gather Entry - Specifies the the Scatter Gather Format
 * related information
 */
struct sg_entry {
#if defined(CONFIG_SYS_FSL_SEC_LE) && \
	!(defined(CONFIG_MX6) || defined(CONFIG_MX7) || \
	  defined(CONFIG_MX7ULP) || defined(CONFIG_IMX8M) || defined(CONFIG_IMX8))
	uint32_t addr_lo;	/* Memory Address - lo */
	uint32_t addr_hi;	/* Memory Address of start of buffer - hi */
#else
	uint32_t addr_hi;	/* Memory Address of start of buffer - hi */
	uint32_t addr_lo;	/* Memory Address - lo */
#endif

	uint32_t len_flag;	/* Length of the data in the frame */
#define SG_ENTRY_LENGTH_MASK	0x3FFFFFFF
#define SG_ENTRY_EXTENSION_BIT	0x80000000
#define SG_ENTRY_FINAL_BIT	0x40000000
	uint32_t bpid_offset;
#define SG_ENTRY_BPID_MASK	0x00FF0000
#define SG_ENTRY_BPID_SHIFT	16
#define SG_ENTRY_OFFSET_MASK	0x00001FFF
#define SG_ENTRY_OFFSET_SHIFT	0
};

#if defined(CONFIG_MX6) || defined(CONFIG_MX7) || \
	defined(CONFIG_MX7ULP) || defined(CONFIG_IMX8M) || defined(CONFIG_IMX8)
/* Job Ring Base Address */
#define JR_BASE_ADDR(x) (CFG_SYS_FSL_SEC_ADDR + 0x1000 * (x + 1))
/* Secure Memory Offset varies accross versions */
#define SM_V1_OFFSET 0x0f4
#define SM_V2_OFFSET 0xa00
/*Secure Memory Versioning */
#define SMVID_V2 0x20105
#define SM_VERSION(x)  ({typeof(x) _x = x; \
		_x < SMVID_V2 ? 1 : (_x < 0x20300 ? 2 : 3); })
#define SM_OFFSET(x)  (x == 1 ? SM_V1_OFFSET : SM_V2_OFFSET)
/* CAAM Job Ring 0 Registers */
/* Secure Memory Partition Owner register */
#define SMCSJR_PO		(3 << 6)
/* JR Allocation Error */
#define SMCSJR_AERR		(3 << 12)
/* Secure memory partition 0 page 0 owner register */
#define CAAM_SMPO_0	    (CFG_SYS_FSL_SEC_ADDR + 0x1FBC)
/* Secure memory command register */
#define CAAM_SMCJR(v, jr)   (JR_BASE_ADDR(jr) + SM_OFFSET(v) + SM_CMD(v))
/* Secure memory command status register */
#define CAAM_SMCSJR(v, jr)  (JR_BASE_ADDR(jr) + SM_OFFSET(v) + SM_STATUS(v))
/* Secure memory access permissions register */
#define CAAM_SMAPJR(v, jr, y) \
	(JR_BASE_ADDR(jr) + SM_OFFSET(v) + SM_PERM(v) + y * 16)
/* Secure memory access group 2 register */
#define CAAM_SMAG2JR(v, jr, y) \
	(JR_BASE_ADDR(jr) + SM_OFFSET(v) + SM_GROUP2(v) + y * 16)
/* Secure memory access group 1 register */
#define CAAM_SMAG1JR(v, jr, y)  \
	(JR_BASE_ADDR(jr) + SM_OFFSET(v) + SM_GROUP1(v) + y * 16)

/* Commands and macros for secure memory */
#define SM_CMD(v)		(v == 1 ? 0x0 : 0x1E4)
#define SM_STATUS(v)		(v == 1 ? 0x8 : 0x1EC)
#define SM_PERM(v)		(v == 1 ?  0x10 : 0x4)
#define SM_GROUP2(v)		({typeof(v) _v = v; \
		_v == 1 ? 0x14 : (_v == 2 ? 0x8 : 0xC); })
#define SM_GROUP1(v)		({typeof(v) _v = v; \
		_v == 1 ? 0x18 : (_v == 2 ? 0xC : 0x8); })
#define CMD_PAGE_ALLOC		0x1
#define CMD_PAGE_DEALLOC	0x2
#define CMD_PART_DEALLOC	0x3
#define CMD_INQUIRY		0x5
#define CMD_COMPLETE		(3 << 14)
#define PAGE_AVAILABLE		0
#define PAGE_OWNED		(3 << 6)
#define PAGE(x)			(x << 16)
#define PARTITION(x)		(x << 8)
#define PARTITION_OWNER(x)	(0x3 << (x*2))

/* Address of secure 4kbyte pages */
#define SEC_MEM_PAGE0		CAAM_ARB_BASE_ADDR
#define SEC_MEM_PAGE1		(CAAM_ARB_BASE_ADDR + 0x1000)
#define SEC_MEM_PAGE2		(CAAM_ARB_BASE_ADDR + 0x2000)
#define SEC_MEM_PAGE3		(CAAM_ARB_BASE_ADDR + 0x3000)

#ifdef CONFIG_IMX8M
#define JR_MID    (1)         /* Matches ATF configuration */
#define KS_G1     (0x10000 << JR_MID) /* CAAM only */
#define PERM      (0xB080)    /* CSP, SMAP_LCK, SMAG_LCK, G1_BLOB */
#else
#define JR_MID    (2)         /* Matches ROM configuration */
#define KS_G1     BIT(JR_MID) /* CAAM only */
#define PERM      (0xB008)    /* CSP, SMAP_LCK, SMAG_LCK, G1_BLOB */
#endif /* CONFIG_IMX8M */

/* HAB WRAPPED KEY header */
#define WRP_HDR_SIZE		0x08
#define HDR_TAG			0x81
#define HDR_PAR			0x41
/* HAB WRAPPED KEY Data */
#define HAB_MOD			0x66
#define HAB_ALG			0x55
#define HAB_FLG			0x00

/* Partition and Page IDs */
#define PARTITION_1	1
#define PAGE_1			1

#define ERROR_IN_PAGE_ALLOC	1
#define ECONSTRJDESC   -1

#endif

#define FSL_CAAM_MP_PUBK_BYTES		    64
#define FSL_CAAM_MP_PRVK_BYTES		    32
#define FSL_CAAM_MP_MES_DGST_BYTES	    32

#define FSL_CAAM_ORSR_JRa_OFFSET	0x102c
#define FSL_CAAM_MAX_JR_SIZE		4

/* blob_dek:
 * Encapsulates the src in a secure blob and stores it dst
 * @src: reference to the plaintext
 * @dst: reference to the output adrress
 * @len: size in bytes of src
 * @return: 0 on success, error otherwise
 */
int blob_dek(const u8 *src, u8 *dst, u8 len);

int gen_mppubk(u8 *dst);

int sign_mppubk(const u8 *m, int data_size, u8 *dgst, u8 *c, u8 *d);

#if defined(CONFIG_ARCH_C29X)
int sec_init_idx(uint8_t);
#endif
int sec_init(void);

u8 caam_get_era(void);
#endif

#endif /* __FSL_SEC_H */
