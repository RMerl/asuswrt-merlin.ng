/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015, Freescale Semiconductor, Inc.
 */

#ifndef __ARCH_ARM_MACH_S32V234_MCCGM_REGS_H__
#define __ARCH_ARM_MACH_S32V234_MCCGM_REGS_H__

#ifndef __ASSEMBLY__

/* MC_CGM registers definitions */
/* MC_CGM_SC_SS */
#define CGM_SC_SS(cgm_addr)			( ((cgm_addr) + 0x000007E4) )
#define MC_CGM_SC_SEL_FIRC			(0x0)
#define MC_CGM_SC_SEL_XOSC			(0x1)
#define MC_CGM_SC_SEL_ARMPLL		(0x2)
#define MC_CGM_SC_SEL_CLKDISABLE	(0xF)

/* MC_CGM_SC_DCn */
#define CGM_SC_DCn(cgm_addr,dc)		( ((cgm_addr) + 0x000007E8) + ((dc) * 0x4) )
#define MC_CGM_SC_DCn_PREDIV(val)	(MC_CGM_SC_DCn_PREDIV_MASK & ((val) << MC_CGM_SC_DCn_PREDIV_OFFSET))
#define MC_CGM_SC_DCn_PREDIV_MASK	(0x00070000)
#define MC_CGM_SC_DCn_PREDIV_OFFSET	(16)
#define MC_CGM_SC_DCn_DE			(1 << 31)
#define MC_CGM_SC_SEL_MASK			(0x0F000000)
#define MC_CGM_SC_SEL_OFFSET		(24)

/* MC_CGM_ACn_DCm */
#define CGM_ACn_DCm(cgm_addr,ac,dc)		( ((cgm_addr) + 0x00000808) + ((ac) * 0x20) + ((dc) * 0x4) )
#define MC_CGM_ACn_DCm_PREDIV(val)		(MC_CGM_ACn_DCm_PREDIV_MASK & ((val) << MC_CGM_ACn_DCm_PREDIV_OFFSET))

/*
 * MC_CGM_ACn_DCm_PREDIV_MASK is on 5 bits because practical test has shown
 * that the 5th bit is always ignored during writes if the current
 * MC_CGM_ACn_DCm_PREDIV field has only 4 bits
 *
 * The manual states only selectors 1, 5 and 15 have DC0_PREDIV on 5 bits
 *
 * This should be changed if any problems occur.
 */
#define MC_CGM_ACn_DCm_PREDIV_MASK		(0x001F0000)
#define MC_CGM_ACn_DCm_PREDIV_OFFSET	(16)
#define MC_CGM_ACn_DCm_DE				(1 << 31)

/*
 * MC_CGM_ACn_SC/MC_CGM_ACn_SS
 */
#define CGM_ACn_SC(cgm_addr,ac)			((cgm_addr + 0x00000800) + ((ac) * 0x20))
#define CGM_ACn_SS(cgm_addr,ac)			((cgm_addr + 0x00000804) + ((ac) * 0x20))
#define MC_CGM_ACn_SEL_MASK				(0x07000000)
#define MC_CGM_ACn_SEL_SET(source)		(MC_CGM_ACn_SEL_MASK & (((source) & 0x7) << MC_CGM_ACn_SEL_OFFSET))
#define MC_CGM_ACn_SEL_OFFSET			(24)

#define MC_CGM_ACn_SEL_FIRC				(0x0)
#define MC_CGM_ACn_SEL_XOSC				(0x1)
#define MC_CGM_ACn_SEL_ARMPLL			(0x2)
/*
 * According to the manual some PLL can be divided by X (X={1,3,5}):
 * PERPLLDIVX, VIDEOPLLDIVX.
 */
#define MC_CGM_ACn_SEL_PERPLLDIVX		(0x3)
#define MC_CGM_ACn_SEL_ENETPLL			(0x4)
#define MC_CGM_ACn_SEL_DDRPLL			(0x5)
#define MC_CGM_ACn_SEL_EXTSRCPAD		(0x7)
#define MC_CGM_ACn_SEL_SYSCLK			(0x8)
#define MC_CGM_ACn_SEL_VIDEOPLLDIVX		(0x9)
#define MC_CGM_ACn_SEL_PERCLK			(0xA)

/* PLLDIG PLL Divider Register (PLLDIG_PLLDV) */
#define PLLDIG_PLLDV(pll)				((MC_CGM0_BASE_ADDR + 0x00000028) + ((pll) * 0x80))
#define PLLDIG_PLLDV_MFD(div)			(PLLDIG_PLLDV_MFD_MASK & (div))
#define PLLDIG_PLLDV_MFD_MASK			(0x000000FF)

/*
 * PLLDIG_PLLDV_RFDPHIB has a different format for /32 according to
 * the reference manual. This other value respect the formula 2^[RFDPHIBY+1]
 */
#define PLLDIG_PLLDV_RFDPHI_SET(val)	(PLLDIG_PLLDV_RFDPHI_MASK & (((val) & PLLDIG_PLLDV_RFDPHI_MAXVALUE) << PLLDIG_PLLDV_RFDPHI_OFFSET))
#define PLLDIG_PLLDV_RFDPHI_MASK		(0x003F0000)
#define PLLDIG_PLLDV_RFDPHI_MAXVALUE	(0x3F)
#define PLLDIG_PLLDV_RFDPHI_OFFSET		(16)

#define PLLDIG_PLLDV_RFDPHI1_SET(val)	(PLLDIG_PLLDV_RFDPHI1_MASK & (((val) & PLLDIG_PLLDV_RFDPHI1_MAXVALUE) << PLLDIG_PLLDV_RFDPHI1_OFFSET))
#define PLLDIG_PLLDV_RFDPHI1_MASK		(0x7E000000)
#define PLLDIG_PLLDV_RFDPHI1_MAXVALUE	(0x3F)
#define PLLDIG_PLLDV_RFDPHI1_OFFSET		(25)

#define PLLDIG_PLLDV_PREDIV_SET(val)	(PLLDIG_PLLDV_PREDIV_MASK & (((val) & PLLDIG_PLLDV_PREDIV_MAXVALUE) << PLLDIG_PLLDV_PREDIV_OFFSET))
#define PLLDIG_PLLDV_PREDIV_MASK		(0x00007000)
#define PLLDIG_PLLDV_PREDIV_MAXVALUE	(0x7)
#define PLLDIG_PLLDV_PREDIV_OFFSET		(12)

/* PLLDIG PLL Fractional  Divide Register (PLLDIG_PLLFD) */
#define PLLDIG_PLLFD(pll)				((MC_CGM0_BASE_ADDR + 0x00000030) + ((pll) * 0x80))
#define PLLDIG_PLLFD_MFN_SET(val)		(PLLDIG_PLLFD_MFN_MASK & (val))
#define PLLDIG_PLLFD_MFN_MASK			(0x00007FFF)
#define PLLDIG_PLLFD_SMDEN				(1 << 30)

/* PLL Calibration Register 1 (PLLDIG_PLLCAL1) */
#define PLLDIG_PLLCAL1(pll)				((MC_CGM0_BASE_ADDR + 0x00000038) + ((pll) * 0x80))
#define PLLDIG_PLLCAL1_NDAC1_SET(val)	(PLLDIG_PLLCAL1_NDAC1_MASK & ((val) << PLLDIG_PLLCAL1_NDAC1_OFFSET))
#define PLLDIG_PLLCAL1_NDAC1_OFFSET		(24)
#define PLLDIG_PLLCAL1_NDAC1_MASK		(0x7F000000)

/* Digital Frequency Synthesizer (DFS) */
/* According to the manual there are 3 DFS modules only for ARM_PLL, DDR_PLL, ENET_PLL */
#define DFS0_BASE_ADDR				(MC_CGM0_BASE_ADDR + 0x00000040)

/* DFS DLL Program Register 1 */
#define DFS_DLLPRG1(pll)			(DFS0_BASE_ADDR + 0x00000000 + ((pll) * 0x80))

#define DFS_DLLPRG1_V2IGC_SET(val)	(DFS_DLLPRG1_V2IGC_MASK & ((val) << DFS_DLLPRG1_V2IGC_OFFSET))
#define DFS_DLLPRG1_V2IGC_OFFSET	(0)
#define DFS_DLLPRG1_V2IGC_MASK		(0x00000007)

#define DFS_DLLPRG1_LCKWT_SET(val)		(DFS_DLLPRG1_LCKWT_MASK & ((val) << DFS_DLLPRG1_LCKWT_OFFSET))
#define DFS_DLLPRG1_LCKWT_OFFSET		(4)
#define DFS_DLLPRG1_LCKWT_MASK			(0x00000030)

#define DFS_DLLPRG1_DACIN_SET(val)		(DFS_DLLPRG1_DACIN_MASK & ((val) << DFS_DLLPRG1_DACIN_OFFSET))
#define DFS_DLLPRG1_DACIN_OFFSET		(6)
#define DFS_DLLPRG1_DACIN_MASK			(0x000001C0)

#define DFS_DLLPRG1_CALBYPEN_SET(val)	(DFS_DLLPRG1_CALBYPEN_MASK & ((val) << DFS_DLLPRG1_CALBYPEN_OFFSET))
#define DFS_DLLPRG1_CALBYPEN_OFFSET		(9)
#define DFS_DLLPRG1_CALBYPEN_MASK		(0x00000200)

#define DFS_DLLPRG1_VSETTLCTRL_SET(val)	(DFS_DLLPRG1_VSETTLCTRL_MASK & ((val) << DFS_DLLPRG1_VSETTLCTRL_OFFSET))
#define DFS_DLLPRG1_VSETTLCTRL_OFFSET	(10)
#define DFS_DLLPRG1_VSETTLCTRL_MASK		(0x00000C00)

#define DFS_DLLPRG1_CPICTRL_SET(val)	(DFS_DLLPRG1_CPICTRL_MASK & ((val) << DFS_DLLPRG1_CPICTRL_OFFSET))
#define DFS_DLLPRG1_CPICTRL_OFFSET		(12)
#define DFS_DLLPRG1_CPICTRL_MASK		(0x00007000)

/* DFS Control Register (DFS_CTRL) */
#define DFS_CTRL(pll)					(DFS0_BASE_ADDR + 0x00000018 + ((pll) * 0x80))
#define DFS_CTRL_DLL_LOLIE				(1 << 0)
#define DFS_CTRL_DLL_RESET				(1 << 1)

/* DFS Port Status Register (DFS_PORTSR) */
#define DFS_PORTSR(pll)						(DFS0_BASE_ADDR + 0x0000000C +((pll) * 0x80))
/* DFS Port Reset Register (DFS_PORTRESET) */
#define DFS_PORTRESET(pll)					(DFS0_BASE_ADDR + 0x00000014 + ((pll) * 0x80))
#define DFS_PORTRESET_PORTRESET_SET(val)	(DFS_PORTRESET_PORTRESET_MASK | (((val) & DFS_PORTRESET_PORTRESET_MAXVAL) << DFS_PORTRESET_PORTRESET_OFFSET))
#define DFS_PORTRESET_PORTRESET_MAXVAL		(0xF)
#define DFS_PORTRESET_PORTRESET_MASK		(0x0000000F)
#define DFS_PORTRESET_PORTRESET_OFFSET		(0)

/* DFS Divide Register Portn (DFS_DVPORTn) */
#define DFS_DVPORTn(pll,n)			(DFS0_BASE_ADDR + ((pll) * 0x80) + (0x0000001C + ((n) * 0x4)))

/*
 * The mathematical formula for fdfs_clockout is the following:
 * fdfs_clckout = fdfs_clkin / ( DFS_DVPORTn[MFI] + (DFS_DVPORTn[MFN]/256) )
 */
#define DFS_DVPORTn_MFI_SET(val)	(DFS_DVPORTn_MFI_MASK & (((val) & DFS_DVPORTn_MFI_MAXVAL) << DFS_DVPORTn_MFI_OFFSET) )
#define DFS_DVPORTn_MFN_SET(val)	(DFS_DVPORTn_MFN_MASK & (((val) & DFS_DVPORTn_MFN_MAXVAL) << DFS_DVPORTn_MFN_OFFSET) )
#define DFS_DVPORTn_MFI_MASK		(0x0000FF00)
#define DFS_DVPORTn_MFN_MASK		(0x000000FF)
#define DFS_DVPORTn_MFI_MAXVAL		(0xFF)
#define DFS_DVPORTn_MFN_MAXVAL		(0xFF)
#define DFS_DVPORTn_MFI_OFFSET		(8)
#define DFS_DVPORTn_MFN_OFFSET		(0)
#define DFS_MAXNUMBER				(4)

#define DFS_PARAMS_Nr				(3)

/* Frequencies are in Hz */
#define FIRC_CLK_FREQ				(48000000)
#define XOSC_CLK_FREQ				(40000000)

#define PLL_MIN_FREQ				(650000000)
#define PLL_MAX_FREQ				(1300000000)

#define ARM_PLL_PHI0_FREQ			(1000000000)
#define ARM_PLL_PHI1_FREQ			(1000000000)
/* ARM_PLL_PHI1_DFS1_FREQ - 266 Mhz */
#define ARM_PLL_PHI1_DFS1_EN		(1)
#define ARM_PLL_PHI1_DFS1_MFI		(3)
#define ARM_PLL_PHI1_DFS1_MFN		(194)
/* ARM_PLL_PHI1_DFS2_REQ - 600 Mhz */
#define ARM_PLL_PHI1_DFS2_EN		(1)
#define ARM_PLL_PHI1_DFS2_MFI		(1)
#define ARM_PLL_PHI1_DFS2_MFN		(170)
/* ARM_PLL_PHI1_DFS3_FREQ - 600 Mhz */
#define ARM_PLL_PHI1_DFS3_EN		(1)
#define ARM_PLL_PHI1_DFS3_MFI		(1)
#define ARM_PLL_PHI1_DFS3_MFN		(170)
#define ARM_PLL_PHI1_DFS_Nr			(3)
#define ARM_PLL_PLLDV_PREDIV		(2)
#define ARM_PLL_PLLDV_MFD			(50)
#define ARM_PLL_PLLDV_MFN			(0)

#define PERIPH_PLL_PHI0_FREQ		(400000000)
#define PERIPH_PLL_PHI1_FREQ		(100000000)
#define PERIPH_PLL_PHI1_DFS_Nr		(0)
#define PERIPH_PLL_PLLDV_PREDIV		(1)
#define PERIPH_PLL_PLLDV_MFD		(30)
#define PERIPH_PLL_PLLDV_MFN		(0)

#define ENET_PLL_PHI0_FREQ			(500000000)
#define ENET_PLL_PHI1_FREQ			(1000000000)
/* ENET_PLL_PHI1_DFS1_FREQ - 350 Mhz*/
#define ENET_PLL_PHI1_DFS1_EN		(1)
#define ENET_PLL_PHI1_DFS1_MFI		(2)
#define ENET_PLL_PHI1_DFS1_MFN		(219)
/* ENET_PLL_PHI1_DFS2_FREQ - 350 Mhz*/
#define ENET_PLL_PHI1_DFS2_EN		(1)
#define ENET_PLL_PHI1_DFS2_MFI		(2)
#define ENET_PLL_PHI1_DFS2_MFN		(219)
/* ENET_PLL_PHI1_DFS3_FREQ - 320 Mhz*/
#define ENET_PLL_PHI1_DFS3_EN		(1)
#define ENET_PLL_PHI1_DFS3_MFI		(3)
#define ENET_PLL_PHI1_DFS3_MFN		(32)
/* ENET_PLL_PHI1_DFS1_FREQ - 50 Mhz*/
#define ENET_PLL_PHI1_DFS4_EN		(1)
#define ENET_PLL_PHI1_DFS4_MFI		(2)
#define ENET_PLL_PHI1_DFS4_MFN		(0)
#define ENET_PLL_PHI1_DFS_Nr		(4)
#define ENET_PLL_PLLDV_PREDIV		(2)
#define ENET_PLL_PLLDV_MFD			(50)
#define ENET_PLL_PLLDV_MFN			(0)

#define DDR_PLL_PHI0_FREQ			(533000000)
#define DDR_PLL_PHI1_FREQ			(1066000000)
/* DDR_PLL_PHI1_DFS1_FREQ - 500 Mhz */
#define DDR_PLL_PHI1_DFS1_EN		(1)
#define DDR_PLL_PHI1_DFS1_MFI		(2)
#define DDR_PLL_PHI1_DFS1_MFN		(33)
/* DDR_PLL_PHI1_DFS2_REQ - 500 Mhz */
#define DDR_PLL_PHI1_DFS2_EN		(1)
#define DDR_PLL_PHI1_DFS2_MFI		(2)
#define DDR_PLL_PHI1_DFS2_MFN		(33)
/* DDR_PLL_PHI1_DFS3_FREQ - 350 Mhz */
#define DDR_PLL_PHI1_DFS3_EN		(1)
#define DDR_PLL_PHI1_DFS3_MFI		(3)
#define DDR_PLL_PHI1_DFS3_MFN		(11)
#define DDR_PLL_PHI1_DFS_Nr			(3)
#define DDR_PLL_PLLDV_PREDIV		(2)
#define DDR_PLL_PLLDV_MFD			(53)
#define DDR_PLL_PLLDV_MFN			(6144)

#define VIDEO_PLL_PHI0_FREQ			(600000000)
#define VIDEO_PLL_PHI1_FREQ			(0)
#define VIDEO_PLL_PHI1_DFS_Nr		(0)
#define VIDEO_PLL_PLLDV_PREDIV		(1)
#define VIDEO_PLL_PLLDV_MFD			(30)
#define VIDEO_PLL_PLLDV_MFN			(0)

#endif

#endif /*__ARCH_ARM_MACH_S32V234_MCCGM_REGS_H__ */
