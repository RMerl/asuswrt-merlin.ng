/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 */

#ifndef __OCTEON_FEATURE_H__
#define __OCTEON_FEATURE_H__

#include "cvmx-fuse.h"

/*
 * Octeon models are declared after the macros in octeon-model.h with the
 * suffix _FEATURE. The individual features are declared with the
 * _FEATURE_ infix.
 */
enum octeon_feature {
	/*
	 * Checks on the critical path are moved to the top (8 positions)
	 * so that the compiler generates one less insn than for the rest
	 * of the checks.
	 */
	OCTEON_FEATURE_PKND, /* CN68XX uses port kinds for packet interface */
	/* CN68XX has different fields in word0 - word2 */
	OCTEON_FEATURE_CN68XX_WQE,

	/*
	 * Features
	 */
	/*
	 * Octeon models in the CN5XXX family and higher support atomic
	 * add instructions to memory (saa/saad)
	 */
	OCTEON_FEATURE_SAAD,
	/* Does this Octeon support the ZIP offload engine? */
	OCTEON_FEATURE_ZIP,
	/* Does this Octeon support crypto acceleration using COP2? */
	OCTEON_FEATURE_CRYPTO,
	/* Can crypto be enabled by calling cvmx_crypto_dormant_enable()? */
	OCTEON_FEATURE_DORM_CRYPTO,
	OCTEON_FEATURE_PCIE,	/* Does this Octeon support PCI express? */
	OCTEON_FEATURE_SRIO,	/* Does this Octeon support SRIO */
	OCTEON_FEATURE_ILK,	/* Does this Octeon support Interlaken */
	/*
	 * Some Octeon models support internal memory for storing
	 * cryptographic keys
	 */
	OCTEON_FEATURE_KEY_MEMORY,
	/* Octeon has a LED controller for banks of external LEDs */
	OCTEON_FEATURE_LED_CONTROLLER,
	OCTEON_FEATURE_TRA,	/* Octeon has a trace buffer */
	OCTEON_FEATURE_MGMT_PORT, /* Octeon has a management port */
	OCTEON_FEATURE_RAID,	/* Octeon has a raid unit */
	OCTEON_FEATURE_USB,	/* Octeon has a builtin USB */
	/* Octeon IPD can run without using work queue entries */
	OCTEON_FEATURE_NO_WPTR,
	OCTEON_FEATURE_DFA,	/* Octeon has DFA state machines */
	/*
	 * Octeon MDIO block supports clause 45 transactions for
	 * 10 Gig support
	 */
	OCTEON_FEATURE_MDIO_CLAUSE_45,
	/*
	 * CN52XX and CN56XX used a block named NPEI for PCIe access.
	 * Newer chips replaced this with SLI+DPI
	 */
	OCTEON_FEATURE_NPEI,
	OCTEON_FEATURE_HFA,	/* Octeon has DFA/HFA */
	OCTEON_FEATURE_DFM,	/* Octeon has DFM */
	OCTEON_FEATURE_CIU2,	/* Octeon has CIU2 */
	/* Octeon has DMA Instruction Completion Interrupt mode */
	OCTEON_FEATURE_DICI_MODE,
	/* Octeon has Bit Select Extractor schedulor */
	OCTEON_FEATURE_BIT_EXTRACTOR,
	OCTEON_FEATURE_NAND,	/* Octeon has NAND */
	OCTEON_FEATURE_MMC,	/* Octeon has built-in MMC support */
	OCTEON_FEATURE_ROM,	/* Octeon has built-in ROM support */
	OCTEON_FEATURE_AUTHENTIK, /* Octeon has Authentik ROM support */
	OCTEON_FEATURE_MULTICAST_TIMER, /* Octeon has multi_cast timer */
	OCTEON_FEATURE_MULTINODE, /* Octeon has node support */
	OCTEON_FEATURE_CIU3,	/* Octeon has CIU3 */
	OCTEON_FEATURE_FPA3,	/* Octeon has FPA first seen on 78XX */
	/* CN78XX has different fields in word0 - word2 */
	OCTEON_FEATURE_CN78XX_WQE,
	OCTEON_FEATURE_PKO3,	/* Octeon has enhanced PKO block */
	OCTEON_FEATURE_SPI,	/* Octeon supports SPI interfaces */
	OCTEON_FEATURE_ZIP3,	/* Octeon has zip first seen on 78XX */
	OCTEON_FEATURE_BCH,	/* Octeon supports BCH ECC */
	OCTEON_FEATURE_PKI,	/* Octeon has PKI block */
	OCTEON_FEATURE_OCLA,	/* Octeon has OCLA */
	OCTEON_FEATURE_FAU,	/* Octeon has FAU */
	OCTEON_FEATURE_BGX,	/* Octeon has BGX */
	OCTEON_FEATURE_BGX_MIX,	/* On of the BGX is used for MIX */
	OCTEON_FEATURE_HNA,	/* Octeon has HNA */
	OCTEON_FEATURE_BGX_XCV,	/* Octeon has BGX XCV RGMII support */
	OCTEON_FEATURE_TSO,	/* Octeon has tcp segmentation offload */
	OCTEON_FEATURE_TDM,	/* Octeon has PCM/TDM support */
	OCTEON_FEATURE_PTP,	/* Octeon has PTP support */
	OCTEON_MAX_FEATURE
};

static inline int octeon_has_feature_OCTEON_FEATURE_SAAD(void)
{
	return true;
}

static inline int octeon_has_feature_OCTEON_FEATURE_ZIP(void)
{
	if (OCTEON_IS_MODEL(OCTEON_CNF71XX) ||
	    OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX))
		return 0;
	else
		return !cvmx_fuse_read(121);
}

static inline int octeon_has_feature_OCTEON_FEATURE_ZIP3(void)
{
	return (OCTEON_IS_MODEL(OCTEON_CN78XX) ||
		OCTEON_IS_MODEL(OCTEON_CN73XX));
}

static inline int octeon_has_feature_OCTEON_FEATURE_BCH(void)
{
	return (OCTEON_IS_MODEL(OCTEON_CN70XX) ||
		OCTEON_IS_MODEL(OCTEON_CNF75XX) ||
		OCTEON_IS_MODEL(OCTEON_CN73XX));
}

static inline int octeon_has_feature_OCTEON_FEATURE_CRYPTO(void)
{
	/* OCTEON II and later */
	u64 val;

	val = csr_rd(CVMX_MIO_FUS_DAT2);
	if (val & MIO_FUS_DAT2_NOCRYPTO || val & MIO_FUS_DAT2_NOMUL)
		return 0;
	else if (!(val & MIO_FUS_DAT2_DORM_CRYPTO))
		return 1;

	val = csr_rd(CVMX_RNM_CTL_STATUS);
	return val & RNM_CTL_STATUS_EER_VAL;
}

static inline int octeon_has_feature_OCTEON_FEATURE_DORM_CRYPTO(void)
{
	/* OCTEON II and later */
	u64 val;

	val = csr_rd(CVMX_MIO_FUS_DAT2);
	return !(val & MIO_FUS_DAT2_NOCRYPTO) && !(val & MIO_FUS_DAT2_NOMUL) &&
		(val & MIO_FUS_DAT2_DORM_CRYPTO);
}

static inline int octeon_has_feature_OCTEON_FEATURE_PCIE(void)
{
	/* OCTEON II and later have PCIe */
	return true;
}

static inline int octeon_has_feature_OCTEON_FEATURE_SRIO(void)
{
	if (OCTEON_IS_MODEL(OCTEON_CNF75XX)) {
		if (cvmx_fuse_read(1601) == 0)
			return 0;
		else
			return 1;
	} else {
		return (OCTEON_IS_MODEL(OCTEON_CN63XX) ||
			OCTEON_IS_MODEL(OCTEON_CN66XX));
	}
}

static inline int octeon_has_feature_OCTEON_FEATURE_ILK(void)
{
	return (OCTEON_IS_MODEL(OCTEON_CN68XX) ||
		OCTEON_IS_MODEL(OCTEON_CN78XX));
}

static inline int octeon_has_feature_OCTEON_FEATURE_KEY_MEMORY(void)
{
	/* OCTEON II or later */
	return true;
}

static inline int octeon_has_feature_OCTEON_FEATURE_LED_CONTROLLER(void)
{
	return false;
}

static inline int octeon_has_feature_OCTEON_FEATURE_TRA(void)
{
	return !OCTEON_IS_OCTEON3();
}

static inline int octeon_has_feature_OCTEON_FEATURE_MGMT_PORT(void)
{
	/* OCTEON II or later */
	return true;
}

static inline int octeon_has_feature_OCTEON_FEATURE_RAID(void)
{
	return !OCTEON_IS_MODEL(OCTEON_CNF75XX);
}

static inline int octeon_has_feature_OCTEON_FEATURE_USB(void)
{
	return true;
}

static inline int octeon_has_feature_OCTEON_FEATURE_NO_WPTR(void)
{
	return true;
}

static inline int octeon_has_feature_OCTEON_FEATURE_DFA(void)
{
	return 0;
}

static inline int octeon_has_feature_OCTEON_FEATURE_HFA(void)
{
	if (OCTEON_IS_MODEL(OCTEON_CNF75XX))
		return 0;
	else
		return !cvmx_fuse_read(90);
}

static inline int octeon_has_feature_OCTEON_FEATURE_HNA(void)
{
	if (OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN73XX))
		return !cvmx_fuse_read(134);
	else
		return 0;
}

static inline int octeon_has_feature_OCTEON_FEATURE_DFM(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX)))
		return 0;
	else
		return !cvmx_fuse_read(90);
}

static inline int octeon_has_feature_OCTEON_FEATURE_MDIO_CLAUSE_45(void)
{
	return true;
}

static inline int octeon_has_feature_OCTEON_FEATURE_NPEI(void)
{
	return false;
}

static inline int octeon_has_feature_OCTEON_FEATURE_PKND(void)
{
	return OCTEON_IS_MODEL(OCTEON_CN68XX) ||
		OCTEON_IS_MODEL(OCTEON_CNF75XX) ||
		OCTEON_IS_MODEL(OCTEON_CN73XX) ||
		OCTEON_IS_MODEL(OCTEON_CN78XX);
}

static inline int octeon_has_feature_OCTEON_FEATURE_CN68XX_WQE(void)
{
	return OCTEON_IS_MODEL(OCTEON_CN68XX);
}

static inline int octeon_has_feature_OCTEON_FEATURE_CIU2(void)
{
	return OCTEON_IS_MODEL(OCTEON_CN68XX);
}

static inline int octeon_has_feature_OCTEON_FEATURE_CIU3(void)
{
	return (OCTEON_IS_MODEL(OCTEON_CN78XX) ||
		OCTEON_IS_MODEL(OCTEON_CNF75XX) ||
		OCTEON_IS_MODEL(OCTEON_CN73XX));
}

static inline int octeon_has_feature_OCTEON_FEATURE_FPA3(void)
{
	return (OCTEON_IS_MODEL(OCTEON_CN78XX) ||
		OCTEON_IS_MODEL(OCTEON_CNF75XX) ||
		OCTEON_IS_MODEL(OCTEON_CN73XX));
}

static inline int octeon_has_feature_OCTEON_FEATURE_NAND(void)
{
	return (OCTEON_IS_MODEL(OCTEON_CN63XX) ||
		OCTEON_IS_MODEL(OCTEON_CN66XX) ||
		OCTEON_IS_MODEL(OCTEON_CN68XX) ||
		OCTEON_IS_MODEL(OCTEON_CN73XX) ||
		OCTEON_IS_MODEL(OCTEON_CNF75XX) ||
		OCTEON_IS_MODEL(OCTEON_CN70XX));
}

static inline int octeon_has_feature_OCTEON_FEATURE_DICI_MODE(void)
{
	return (OCTEON_IS_MODEL(OCTEON_CN68XX_PASS2_X) ||
		OCTEON_IS_MODEL(OCTEON_CN61XX) ||
		OCTEON_IS_MODEL(OCTEON_CNF71XX) ||
		OCTEON_IS_MODEL(OCTEON_CN70XX));
}

static inline int octeon_has_feature_OCTEON_FEATURE_BIT_EXTRACTOR(void)
{
	return (OCTEON_IS_MODEL(OCTEON_CN68XX_PASS2_X) ||
		OCTEON_IS_MODEL(OCTEON_CN61XX) ||
		OCTEON_IS_MODEL(OCTEON_CNF71XX) ||
		OCTEON_IS_MODEL(OCTEON_CN70XX));
}

static inline int octeon_has_feature_OCTEON_FEATURE_MMC(void)
{
	return (OCTEON_IS_MODEL(OCTEON_CN61XX) ||
		OCTEON_IS_MODEL(OCTEON_CNF71XX) || OCTEON_IS_OCTEON3());
}

static inline int octeon_has_feature_OCTEON_FEATURE_ROM(void)
{
	return OCTEON_IS_MODEL(OCTEON_CN66XX) ||
		OCTEON_IS_MODEL(OCTEON_CN61XX) ||
		OCTEON_IS_MODEL(OCTEON_CNF71XX);
}

static inline int octeon_has_feature_OCTEON_FEATURE_AUTHENTIK(void)
{
	if (OCTEON_IS_MODEL(OCTEON_CN66XX) ||
	    OCTEON_IS_MODEL(OCTEON_CN61XX) ||
	    OCTEON_IS_MODEL(OCTEON_CNF71XX) ||
	    OCTEON_IS_MODEL(OCTEON_CN70XX)) {
		u64 val;

		val = csr_rd(CVMX_MIO_FUS_DAT2);
		return (val & MIO_FUS_DAT2_NOCRYPTO) &&
			(val & MIO_FUS_DAT2_DORM_CRYPTO);
	}

	return 0;
}

static inline int octeon_has_feature_OCTEON_FEATURE_MULTICAST_TIMER(void)
{
	return (OCTEON_IS_MODEL(OCTEON_CN66XX_PASS1_2) ||
		OCTEON_IS_MODEL(OCTEON_CN61XX) ||
		OCTEON_IS_MODEL(OCTEON_CNF71XX) ||
		OCTEON_IS_MODEL(OCTEON_CN70XX));
}

static inline int octeon_has_feature_OCTEON_FEATURE_MULTINODE(void)
{
	return (!OCTEON_IS_MODEL(OCTEON_CN76XX) &&
		OCTEON_IS_MODEL(OCTEON_CN78XX));
}

static inline int octeon_has_feature_OCTEON_FEATURE_CN78XX_WQE(void)
{
	return (OCTEON_IS_MODEL(OCTEON_CN78XX) ||
		OCTEON_IS_MODEL(OCTEON_CNF75XX) ||
		OCTEON_IS_MODEL(OCTEON_CN73XX));
}

static inline int octeon_has_feature_OCTEON_FEATURE_SPI(void)
{
	return (OCTEON_IS_MODEL(OCTEON_CN66XX) ||
		OCTEON_IS_MODEL(OCTEON_CN61XX) ||
		OCTEON_IS_MODEL(OCTEON_CNF71XX) || OCTEON_IS_OCTEON3());
}

static inline int octeon_has_feature_OCTEON_FEATURE_PKI(void)
{
	return (OCTEON_IS_MODEL(OCTEON_CN78XX) ||
		OCTEON_IS_MODEL(OCTEON_CNF75XX) ||
		OCTEON_IS_MODEL(OCTEON_CN73XX));
}

static inline int octeon_has_feature_OCTEON_FEATURE_PKO3(void)
{
	return (OCTEON_IS_MODEL(OCTEON_CN78XX) ||
		OCTEON_IS_MODEL(OCTEON_CNF75XX) ||
		OCTEON_IS_MODEL(OCTEON_CN73XX));
}

static inline int octeon_has_feature_OCTEON_FEATURE_OCLA(void)
{
	return OCTEON_IS_OCTEON3();
}

static inline int octeon_has_feature_OCTEON_FEATURE_FAU(void)
{
	return (!OCTEON_IS_MODEL(OCTEON_CN78XX) &&
		!OCTEON_IS_MODEL(OCTEON_CNF75XX) &&
		!OCTEON_IS_MODEL(OCTEON_CN73XX));
}

static inline int octeon_has_feature_OCTEON_FEATURE_BGX(void)
{
	return (OCTEON_IS_MODEL(OCTEON_CN78XX) ||
		OCTEON_IS_MODEL(OCTEON_CNF75XX) ||
		OCTEON_IS_MODEL(OCTEON_CN73XX));
}

static inline int octeon_has_feature_OCTEON_FEATURE_BGX_MIX(void)
{
	return (OCTEON_IS_MODEL(OCTEON_CN78XX) ||
		OCTEON_IS_MODEL(OCTEON_CNF75XX) ||
		OCTEON_IS_MODEL(OCTEON_CN73XX));
}

static inline int octeon_has_feature_OCTEON_FEATURE_BGX_XCV(void)
{
	return OCTEON_IS_MODEL(OCTEON_CN73XX);
}

static inline int octeon_has_feature_OCTEON_FEATURE_TSO(void)
{
	return (OCTEON_IS_MODEL(OCTEON_CN73XX) ||
		OCTEON_IS_MODEL(OCTEON_CNF75XX) ||
		OCTEON_IS_MODEL(OCTEON_CN78XX_PASS2_X));
}

static inline int octeon_has_feature_OCTEON_FEATURE_TDM(void)
{
	return OCTEON_IS_MODEL(OCTEON_CN61XX) ||
		OCTEON_IS_MODEL(OCTEON_CNF71XX) ||
		OCTEON_IS_MODEL(OCTEON_CN70XX);
}

static inline int octeon_has_feature_OCTEON_FEATURE_PTP(void)
{
	return OCTEON_IS_MODEL(OCTEON_CN6XXX) ||
		OCTEON_IS_MODEL(OCTEON_CNF7XXX) ||
		OCTEON_IS_MODEL(OCTEON_CN73XX) ||
		OCTEON_IS_MODEL(OCTEON_CNF75XX) ||
		OCTEON_IS_MODEL(OCTEON_CN78XX_PASS2_X);
}

/*
 * Answer ``Is the bit for feature set in the bitmap?''
 * @param feature
 * Return: 1 when the feature is present and 0 otherwise, -1 in case of error.
 */
#define octeon_has_feature(feature_x) octeon_has_feature_##feature_x()

#endif /* __OCTEON_FEATURE_H__ */
