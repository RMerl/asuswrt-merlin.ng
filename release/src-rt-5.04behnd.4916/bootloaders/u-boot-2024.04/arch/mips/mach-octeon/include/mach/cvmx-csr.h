/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Configuration and status register (CSR) address and type definitions for
 * Octoen.
 */

#ifndef __CVMX_CSR_H__
#define __CVMX_CSR_H__

#include "cvmx-csr-enums.h"
#include "cvmx-pip-defs.h"

typedef cvmx_pip_prt_cfgx_t cvmx_pip_port_cfg_t;

/* The CSRs for bootbus region zero used to be independent of the
    other 1-7. As of SDK 1.7.0 these were combined. These macros
    are for backwards compactability */
#define CVMX_MIO_BOOT_REG_CFG0 CVMX_MIO_BOOT_REG_CFGX(0)
#define CVMX_MIO_BOOT_REG_TIM0 CVMX_MIO_BOOT_REG_TIMX(0)

/* The CN3XXX and CN58XX chips used to not have a LMC number
    passed to the address macros. These are here to supply backwards
    compatibility with old code. Code should really use the new addresses
    with bus arguments for support on other chips */
#define CVMX_LMC_BIST_CTL	  CVMX_LMCX_BIST_CTL(0)
#define CVMX_LMC_BIST_RESULT	  CVMX_LMCX_BIST_RESULT(0)
#define CVMX_LMC_COMP_CTL	  CVMX_LMCX_COMP_CTL(0)
#define CVMX_LMC_CTL		  CVMX_LMCX_CTL(0)
#define CVMX_LMC_CTL1		  CVMX_LMCX_CTL1(0)
#define CVMX_LMC_DCLK_CNT_HI	  CVMX_LMCX_DCLK_CNT_HI(0)
#define CVMX_LMC_DCLK_CNT_LO	  CVMX_LMCX_DCLK_CNT_LO(0)
#define CVMX_LMC_DCLK_CTL	  CVMX_LMCX_DCLK_CTL(0)
#define CVMX_LMC_DDR2_CTL	  CVMX_LMCX_DDR2_CTL(0)
#define CVMX_LMC_DELAY_CFG	  CVMX_LMCX_DELAY_CFG(0)
#define CVMX_LMC_DLL_CTL	  CVMX_LMCX_DLL_CTL(0)
#define CVMX_LMC_DUAL_MEMCFG	  CVMX_LMCX_DUAL_MEMCFG(0)
#define CVMX_LMC_ECC_SYND	  CVMX_LMCX_ECC_SYND(0)
#define CVMX_LMC_FADR		  CVMX_LMCX_FADR(0)
#define CVMX_LMC_IFB_CNT_HI	  CVMX_LMCX_IFB_CNT_HI(0)
#define CVMX_LMC_IFB_CNT_LO	  CVMX_LMCX_IFB_CNT_LO(0)
#define CVMX_LMC_MEM_CFG0	  CVMX_LMCX_MEM_CFG0(0)
#define CVMX_LMC_MEM_CFG1	  CVMX_LMCX_MEM_CFG1(0)
#define CVMX_LMC_OPS_CNT_HI	  CVMX_LMCX_OPS_CNT_HI(0)
#define CVMX_LMC_OPS_CNT_LO	  CVMX_LMCX_OPS_CNT_LO(0)
#define CVMX_LMC_PLL_BWCTL	  CVMX_LMCX_PLL_BWCTL(0)
#define CVMX_LMC_PLL_CTL	  CVMX_LMCX_PLL_CTL(0)
#define CVMX_LMC_PLL_STATUS	  CVMX_LMCX_PLL_STATUS(0)
#define CVMX_LMC_READ_LEVEL_CTL	  CVMX_LMCX_READ_LEVEL_CTL(0)
#define CVMX_LMC_READ_LEVEL_DBG	  CVMX_LMCX_READ_LEVEL_DBG(0)
#define CVMX_LMC_READ_LEVEL_RANKX CVMX_LMCX_READ_LEVEL_RANKX(0)
#define CVMX_LMC_RODT_COMP_CTL	  CVMX_LMCX_RODT_COMP_CTL(0)
#define CVMX_LMC_RODT_CTL	  CVMX_LMCX_RODT_CTL(0)
#define CVMX_LMC_WODT_CTL	  CVMX_LMCX_WODT_CTL0(0)
#define CVMX_LMC_WODT_CTL0	  CVMX_LMCX_WODT_CTL0(0)
#define CVMX_LMC_WODT_CTL1	  CVMX_LMCX_WODT_CTL1(0)

/* The CN3XXX and CN58XX chips used to not have a TWSI bus number
    passed to the address macros. These are here to supply backwards
    compatibility with old code. Code should really use the new addresses
    with bus arguments for support on other chips */
#define CVMX_MIO_TWS_INT	 CVMX_MIO_TWSX_INT(0)
#define CVMX_MIO_TWS_SW_TWSI	 CVMX_MIO_TWSX_SW_TWSI(0)
#define CVMX_MIO_TWS_SW_TWSI_EXT CVMX_MIO_TWSX_SW_TWSI_EXT(0)
#define CVMX_MIO_TWS_TWSI_SW	 CVMX_MIO_TWSX_TWSI_SW(0)

/* The CN3XXX and CN58XX chips used to not have a SMI/MDIO bus number
    passed to the address macros. These are here to supply backwards
    compatibility with old code. Code should really use the new addresses
    with bus arguments for support on other chips */
#define CVMX_SMI_CLK	CVMX_SMIX_CLK(0)
#define CVMX_SMI_CMD	CVMX_SMIX_CMD(0)
#define CVMX_SMI_EN	CVMX_SMIX_EN(0)
#define CVMX_SMI_RD_DAT CVMX_SMIX_RD_DAT(0)
#define CVMX_SMI_WR_DAT CVMX_SMIX_WR_DAT(0)

#endif /* __CVMX_CSR_H__ */
