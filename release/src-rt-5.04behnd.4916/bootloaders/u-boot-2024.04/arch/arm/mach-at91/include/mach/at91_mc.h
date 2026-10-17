/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2009 Jens Scharsig (js_at_ng@scharsoft.de)
 */

#ifndef AT91_MC_H
#define AT91_MC_H

#define AT91_ASM_MC_EBI_CSA	(ATMEL_BASE_MC + 0x60)
#define AT91_ASM_MC_EBI_CFG	(ATMEL_BASE_MC + 0x64)
#define AT91_ASM_MC_SMC_CSR0	(ATMEL_BASE_MC + 0x70)
#define AT91_ASM_MC_SDRAMC_MR	(ATMEL_BASE_MC + 0x90)
#define AT91_ASM_MC_SDRAMC_TR	(ATMEL_BASE_MC + 0x94)
#define AT91_ASM_MC_SDRAMC_CR	(ATMEL_BASE_MC + 0x98)

#ifndef __ASSEMBLY__

typedef struct at91_ebi {
	u32	csa;		/* 0x00 Chip Select Assignment Register */
	u32	cfgr;		/* 0x04 Configuration Register */
	u32	reserved[2];
} at91_ebi_t;

#define AT91_EBI_CSA_CS0A	0x0001
#define AT91_EBI_CSA_CS1A	0x0002

#define AT91_EBI_CSA_CS3A	0x0008
#define AT91_EBI_CSA_CS4A	0x0010

typedef struct at91_sdramc {
	u32	mr;	/* 0x00 SDRAMC Mode Register */
	u32	tr;	/* 0x04 SDRAMC Refresh Timer Register */
	u32	cr;	/* 0x08 SDRAMC Configuration Register */
	u32	ssr;	/* 0x0C SDRAMC Self Refresh Register */
	u32	lpr;	/* 0x10 SDRAMC Low Power Register */
	u32	ier;	/* 0x14 SDRAMC Interrupt Enable Register */
	u32	idr;	/* 0x18 SDRAMC Interrupt Disable Register */
	u32	imr;	/* 0x1C SDRAMC Interrupt Mask Register */
	u32	icr;	/* 0x20 SDRAMC Interrupt Status Register */
	u32	reserved[3];
} at91_sdramc_t;

typedef struct at91_smc {
	u32	csr[8];		/* 0x00 SDRAMC Mode Register */
} at91_smc_t;

#define AT91_SMC_CSR_RWHOLD(x)		((x & 0x7) << 28)
#define AT91_SMC_CSR_RWSETUP(x)		((x & 0x7) << 24)
#define AT91_SMC_CSR_ACSS_STANDARD	0x00000000
#define AT91_SMC_CSR_ACSS_1CYCLE	0x00010000
#define AT91_SMC_CSR_ACSS_2CYCLE	0x00020000
#define AT91_SMC_CSR_ACSS_3CYCLE	0x00030000
#define AT91_SMC_CSR_DRP		0x00008000
#define AT91_SMC_CSR_DBW_8		0x00004000
#define AT91_SMC_CSR_DBW_16		0x00002000
#define AT91_SMC_CSR_BAT_8		0x00000000
#define AT91_SMC_CSR_BAT_16		0x00001000
#define AT91_SMC_CSR_TDF(x)		((x & 0xF) << 8)
#define AT91_SMC_CSR_WSEN		0x00000080
#define AT91_SMC_CSR_NWS(x)		(x & 0x7F)

typedef struct at91_bfc {
	u32	mr;	/* 0x00 SDRAMC Mode Register */
} at91_bfc_t;

typedef struct at91_mc {
	u32		rcr;		/* 0x00 MC Remap Control Register */
	u32		asr;		/* 0x04 MC Abort Status Register */
	u32		aasr;		/* 0x08 MC Abort Address Status Reg */
	u32		mpr;		/* 0x0C MC Master Priority Register */
	u32		reserved1[20];	/* 0x10-0x5C */
	at91_ebi_t	ebi;		/* 0x60	- 0x6C EBI */
	at91_smc_t	smc;		/* 0x70 - 0x8C SMC User Interface */
	at91_sdramc_t	sdramc;		/* 0x90 - 0xBC SDRAMC User Interface */
	at91_bfc_t	bfc;		/* 0xC0 BFC User Interface */
	u32		reserved2[15];
} at91_mc_t;

#endif
#endif
