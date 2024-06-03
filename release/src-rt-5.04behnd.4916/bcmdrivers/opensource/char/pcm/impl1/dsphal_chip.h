/*
<:copyright-BRCM:2021:DUAL/GPL:standard

   Copyright (c) 2021 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/
/***************************************************************************/
#ifndef _DSPHAL_CHIP_H
#define _DSPHAL_CHIP_H

#include "bcmtypes.h"

extern int iudma_get_irq_number;
extern int dect_get_irq_number;
/*****************************************************************************
** BCM 63138 (impl2)
*****************************************************************************/
#if defined(CONFIG_BCM963138)
   /* General settings & supported components */
   #define PCM_DPLL_SUPPORT         1
   #define iudma_get_irq_num()      iudma_get_irq_number
   #define dect_get_irq_num()       dect_get_irq_number
   #define IUDMA_NUM_CHANNELS       6
   #define APM_PCM_CTRL             0x200 
   #define APM_DMA_CTRL             0x800 

   #define DECT_SHIM_CTRL_BASE      (IO_ADDRESS(0x80000000) + 0x00050000)
   #define DECT_SHIM_DMA_CTRL_BASE  (IO_ADDRESS(0x80000000) + 0x00050050)
   #define DECT_AHB_SHARED_RAM_BASE (IO_ADDRESS(0x80000000) + 0x00040000)

   /* DPLL settings */
   #define DPLL                     APM
   #define DPLL_CTRL_K0_CLEAR       ~0xF
   #define DPLL_PHASE_THRESH        5
   #define DPLL_K0                  12
   #define DPLL_NOM_FREQ            0xa7c5ac47

   /* NTR control settings */
   #define PCM_NTR_DSL                       1
   #define PCM_NTR_CTRL_CLEAR                (~0xF0000000)
   #define PCM_NTR_IN_CTRL_SHIFT             29 /* shift for PCM_NTR_IN clock routing field */
   #define PCM_NTR_IN_CTRL_SEL_DECT_NTR      3  /* Selection for routing DECT refclk as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_NTR_0    2  /* Selection for routing VDSL_NTR[0] as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_NTR_1    1  /* Selection for routing VDSL_NTR[1] as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_GPIO     0  /* Selection for routing NTR_PULES_IN as PCM_NTR_IN */

   /* PCM settings */
   #define PCM_DMA_CHANNEL_RX       4
   #define PCM_DMA_CHANNEL_TX       5
   #define P_NCO_FCW_MISC           0xa7c5ac47  /* 0xa7c5ac47 for DPLL */
   #define P_PCM_NCO_SHIFT          0x0         /* 0x0 for DPLL, 0x1 for MISC */
   #define P_PCM_NCO_MUX_CNTL_DPLL  0x2         /* 0x2 for DPLL */
   #define P_PCM_NCO_MUX_CNTL_MISC  0x3         /* 0x3 for MISC */
   #define PCM_CLK_CNTL_1_ISI       0x60000000  /* PCM_NCO clk_out = 24.576Mhz. */
   #define PCM_CLK_CNTL_1_ZSI       0xc0000000  /* PCM_NCO clk_out = 49.152Mhz. */
   #define PCM_CLK_CNTL_1_PCM       0x40000000  /* PCM_NCO clk_out = 16.384Mhz. */
   #define PCM_MSIF_ENABLE          0x00000001  /* R_MSIF_ENABLE = 1 */
   #define PCM_ZDS_ENABLE           0x00000001  /* R_ZDS_ENABLE = 1 */
   #define PCM_NCO_MUX_SHIFT        4
   #define PCM_CLOCK_SEL_ISI        0x7         /* div/12 clock divider for ISI mode input clock of 24.576Mhz */
   #define PCM_CLOCK_SEL_PCM        0x2         /* div/8 clock divider for PCM/ZSI mode input clock of 16.384Mhz */
   /* Due to a quirk in the 63138/63148, the timeslots are shifted forward by
    * 16bits and wrap around at the end of the frame */
   #define pcm_get_rx_ch_index(x)   (((x) + 1) & 0x7)

   /* DECT settings */
   #define DECT_AHB_BASE_ADDR       DECT_AHB_REG_PHYS_BASE


/*****************************************************************************
** BCM 63148 (impl2)
*****************************************************************************/
#elif defined(CONFIG_BCM963148)
   /* General settings & supported components */
   #define PCM_DPLL_SUPPORT         1
   #define iudma_get_irq_num()      iudma_get_irq_number
   #define dect_get_irq_num()       dect_get_irq_number
   #define IUDMA_NUM_CHANNELS       6
   #define APM_PCM_CTRL             0x200 
   #define APM_DMA_CTRL             0x800 

   #define DECT_SHIM_CTRL_BASE      (IO_ADDRESS(0x80000000) + 0x00050000)
   #define DECT_SHIM_DMA_CTRL_BASE  (IO_ADDRESS(0x80000000) + 0x00050050)
   #define DECT_AHB_SHARED_RAM_BASE (IO_ADDRESS(0x80000000) + 0x00040000)

   /* DPLL settings */
   #define DPLL                     APM
   #define DPLL_CTRL_K0_CLEAR       ~0xF
   #define DPLL_PHASE_THRESH        5
   #define DPLL_K0                  12
   #define DPLL_NOM_FREQ            0xa7c5ac47

   /* NTR control settings */
   #define PCM_NTR_DSL                       1
   #define PCM_NTR_CTRL_CLEAR                (~0xF0000000)
   #define PCM_NTR_IN_CTRL_SHIFT             29 /* shift for PCM_NTR_IN clock routing field */
   #define PCM_NTR_IN_CTRL_SEL_DECT_NTR      3  /* Selection for routing DECT refclk as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_NTR_0    2  /* Selection for routing VDSL_NTR[0] as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_NTR_1    1  /* Selection for routing VDSL_NTR[1] as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_GPIO     0  /* Selection for routing NTR_PULES_IN as PCM_NTR_IN */

   /* PCM settings */
   #define PCM_DMA_CHANNEL_RX       4
   #define PCM_DMA_CHANNEL_TX       5
   #define P_NCO_FCW_MISC           0xa7c5ac47  /* 0xa7c5ac47 for DPLL */
   #define P_PCM_NCO_SHIFT          0x0         /* 0x0 for DPLL, 0x1 for MISC */
   #define P_PCM_NCO_MUX_CNTL_DPLL  0x2         /* 0x2 for DPLL */
   #define P_PCM_NCO_MUX_CNTL_MISC  0x3         /* 0x3 for MISC */
   #define PCM_CLK_CNTL_1_ISI       0x60000000  /* PCM_NCO clk_out = 24.576Mhz. */
   #define PCM_CLK_CNTL_1_ZSI       0xc0000000  /* PCM_NCO clk_out = 49.152Mhz. */
   #define PCM_CLK_CNTL_1_PCM       0x40000000  /* PCM_NCO clk_out = 16.384Mhz. */
   #define PCM_MSIF_ENABLE          0x00000001  /* R_MSIF_ENABLE = 1 */
   #define PCM_ZDS_ENABLE           0x00000001  /* R_ZDS_ENABLE = 1 */
   #define PCM_NCO_MUX_SHIFT        4
   #define PCM_CLOCK_SEL_ISI        0x7         /* div/12 clock divider for ISI mode input clock of 24.576Mhz */
   #define PCM_CLOCK_SEL_PCM        0x2         /* div/8 clock divider for PCM/ZSI mode input clock of 16.384Mhz */
   /* Due to a quirk in the 63138/63148, the rx timeslots are shifted forward
    * by 16bits and wrap around at the end of the frame */
   #define pcm_get_rx_ch_index(x)   (((x) + 1) & 0x7)

   /* DECT settings */
   #define DECT_AHB_BASE_ADDR       DECT_AHB_REG_PHYS_BASE


/*****************************************************************************
** BCM 6211x (impl2)
*****************************************************************************/
#elif defined(CONFIG_BCM94908)
   /* General settings & supported components */
   #define PCM_DPLL_SUPPORT         1
   #define iudma_get_irq_num()      iudma_get_irq_number
   #define IUDMA_NUM_CHANNELS       2
   #define APM_PCM_CTRL             0xC00 
   #define APM_DMA_CTRL             0x1000

   /* DPLL settings */
   #define DPLL                     PCM
   #define DPLL_CTRL_K0_CLEAR       ~0xF
   #define DPLL_PHASE_THRESH        5
   #define DPLL_K0                  12
   #define DPLL_NOM_FREQ            0xa7c5ac47

   /* NTR control settings */
   #define PCM_NTR_RTR                       1  /* NTR for Router */
   #define PCM_NTR_CTRL_CLEAR                (~0x70000000)
   #define PCM_NTR_IN_CTRL_SHIFT             29 /* shift for PCM_NTR_IN clock routing field */
   #define PCM_NTR_IN_CTRL_SEL_DECT_NTR      3  /* Selection for routing DECT refclk as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_NTR_0    2  /* Selection for routing VDSL_NTR[0] as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_NTR_1    1  /* Selection for routing VDSL_NTR[1] as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_GPIO     0  /* Selection for routing NTR_PULES_IN as PCM_NTR_IN */

   /* PCM settings */
   #define PCM_DMA_CHANNEL_RX       0
   #define PCM_DMA_CHANNEL_TX       1
   #define P_NCO_FCW_MISC           0xa7c5ac47  /* 0xa7c5ac47 for DPLL */
   #define P_PCM_NCO_SHIFT          0x0         /* 0x0 for DPLL, 0x1 for MISC */
   #define P_PCM_NCO_MUX_CNTL_DPLL  0x2         /* 0x2 for DPLL */
   #define P_PCM_NCO_MUX_CNTL_MISC  0x3         /* 0x3 for MISC */
   #define PCM_CLK_CNTL_1_ISI       0x60000000  /* PCM_NCO clk_out = 24.576Mhz. */
   #define PCM_CLK_CNTL_1_ZSI       0xc0000000  /* PCM_NCO clk_out = 49.152Mhz. */
   #define PCM_CLK_CNTL_1_PCM       0x40000000  /* PCM_NCO clk_out = 16.384Mhz. */
   #define PCM_MSIF_ENABLE          0x00000001  /* R_MSIF_ENABLE = 1 */
   #define PCM_ZDS_ENABLE           0x00000001  /* R_ZDS_ENABLE = 1 */
   #define PCM_NCO_MUX_SHIFT        4
   #define PCM_CLOCK_SEL_ISI        0x7         /* div/12 clock divider for ISI mode input clock of 24.576Mhz */
   #define PCM_CLOCK_SEL_PCM        0x2         /* div/8 clock divider for PCM/ZSI mode input clock of 16.384Mhz */

   /* Due to a quirk in the 6211x, the rx timeslots are shifted forward by
    * 16bits and wrap around at the end of the frame */
   #define pcm_get_rx_ch_index(x)   (((x) + 1) & 0x7)

/*****************************************************************************
** BCM 6858 (impl2)
*****************************************************************************/
#elif defined(CONFIG_BCM96858)
   /* General settings & supported components */
   #define PCM_DPLL_SUPPORT         1
   #define iudma_get_irq_num()      iudma_get_irq_number
   #define IUDMA_NUM_CHANNELS       6
   #define APM_PCM_CTRL             0xC00 
   #define APM_DMA_CTRL             0x1800

   #define PCM_LOW_VOICE_DENSITY    1   /*6858B0 PCM channels are limited by OTP */
   #define PCM_SAMPLE_SIZE          0x00C00000
   #define PCM_SAMPLE_SIZE_SHIFT    22
   #define PCM_SAMPLE_SIZE_8        0
   #define PCM_SAMPLE_SIZE_16       1
   #define PCM_SAMPLE_SIZE_32       2
   #define PCM_CLK_DIV              0x0001C000
   #define PCM_CLK_DIV_SHIFT        14
   #define PCM_FRAME_SIZE           0x000000FF
   #define PCM_FRAME_SIZE_SHIFT     0

   /* DPLL settings */
   #define DPLL                     APM
   #define DPLL_CTRL_K0_CLEAR       ~0xF
   #define DPLL_PHASE_THRESH        5
   #define DPLL_K0                  12
   #define DPLL_NOM_FREQ            0xa7c5ac47

   /* NTR control settings */
   #define PCM_NTR_PON                       1
   #define PCM_NTR_CTRL_CLEAR                (~0x70000000)
   #define PCM_NTR_IN_CTRL_SHIFT             29 /* shift for PCM_NTR_IN clock routing field */
   #define PCM_NTR_IN_CTRL_SEL_DECT_NTR      3  /* Selection for routing DECT refclk as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_NTR_0    2  /* Selection for routing VDSL_NTR[0] as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_NTR_1    1  /* Selection for routing VDSL_NTR[1] as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_GPIO     0  /* Selection for routing NTR_PULES_IN as PCM_NTR_IN */

   /* PCM settings */
   #define PCM_DMA_CHANNEL_RX       4
   #define PCM_DMA_CHANNEL_TX       5
   #define P_NCO_FCW_MISC           0xa7c5ac47  /* 0xa7c5ac47 for DPLL */
   #define P_PCM_NCO_SHIFT          0x0         /* 0x0 for DPLL, 0x1 for MISC */
   #define P_PCM_NCO_MUX_CNTL_DPLL  0x2         /* 0x2 for DPLL */
   #define P_PCM_NCO_MUX_CNTL_MISC  0x3         /* 0x3 for MISC */
   #define PCM_CLK_CNTL_1_ISI       0x60000000  /* PCM_NCO clk_out = 24.576Mhz. */
   #define PCM_CLK_CNTL_1_ZSI       0xc0000000  /* PCM_NCO clk_out = 49.152Mhz. */
   #define PCM_CLK_CNTL_1_PCM       0x40000000  /* PCM_NCO clk_out = 16.384Mhz. */
   #define PCM_MSIF_ENABLE          0x00000001  /* R_MSIF_ENABLE = 1 */
   #define PCM_ZDS_ENABLE           0x00000001  /* R_ZDS_ENABLE = 1 */
   #define PCM_NCO_MUX_SHIFT        4
   #define PCM_CLOCK_SEL_ISI        0x7         /* div/12 clock divider for ISI mode input clock of 24.576Mhz */
   #define PCM_CLOCK_SEL_ZSI        0x7         /* div/?? clock divider for ZSI mode input clock of 24.576Mhz */
   #define PCM_CLOCK_SEL_PCM        0x2         /* div/8 clock divider for PCM/ZSI mode input clock of 16.384Mhz */

/*****************************************************************************
** BCM 63158 (impl2)
*****************************************************************************/
#elif defined(CONFIG_BCM963158)
   /* General settings & supported components */
   #define PCM_DPLL_SUPPORT         1
   #define iudma_get_irq_num()      iudma_get_irq_number
   #define IUDMA_NUM_CHANNELS       6
   #define APM_PCM_CTRL             0xC00 
   #define APM_DMA_CTRL             0x1800

   #define PCM_SAMPLE_SIZE          0x00C00000
   #define PCM_SAMPLE_SIZE_SHIFT    22
   #define PCM_SAMPLE_SIZE_8        0
   #define PCM_SAMPLE_SIZE_16       1
   #define PCM_SAMPLE_SIZE_32       2
   #define PCM_CLK_DIV              0x0001C000
   #define PCM_CLK_DIV_SHIFT        14
   #define PCM_FRAME_SIZE           0x000000FF
   #define PCM_FRAME_SIZE_SHIFT     0

   /* DPLL settings */
   #define DPLL                     APM
   #define DPLL_CTRL_K0_CLEAR       ~0xF
   #define DPLL_PHASE_THRESH        5
   #define DPLL_K0                  12
   #define DPLL_NOM_FREQ            0xa7c5ac47

   /* NTR control settings */
   #define PCM_NTR_PON                       1
   #define PCM_NTR_CTRL_CLEAR                (~0x70000000)
   #define PCM_NTR_IN_CTRL_SHIFT             29 /* shift for PCM_NTR_IN clock routing field */
   #define PCM_NTR_IN_CTRL_SEL_DECT_NTR      3  /* Selection for routing DECT refclk as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_NTR_0    2  /* Selection for routing VDSL_NTR[0] as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_NTR_1    1  /* Selection for routing VDSL_NTR[1] as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_GPIO     0  /* Selection for routing NTR_PULES_IN as PCM_NTR_IN */

   /* PCM settings */
   #define PCM_DMA_CHANNEL_RX       4
   #define PCM_DMA_CHANNEL_TX       5
   #define P_NCO_FCW_MISC           0xa7c5ac47  /* 0xa7c5ac47 for DPLL */
   #define P_PCM_NCO_SHIFT          0x0         /* 0x0 for DPLL, 0x1 for MISC */
   #define P_PCM_NCO_MUX_CNTL_DPLL  0x2         /* 0x2 for DPLL */
   #define P_PCM_NCO_MUX_CNTL_MISC  0x3         /* 0x3 for MISC */
   #define PCM_CLK_CNTL_1_ISI       0x60000000  /* PCM_NCO clk_out = 24.576Mhz. */
   #define PCM_CLK_CNTL_1_ZSI       0xc0000000  /* PCM_NCO clk_out = 49.152Mhz. */
   #define PCM_CLK_CNTL_1_PCM       0x40000000  /* PCM_NCO clk_out = 16.384Mhz. */
   #define PCM_MSIF_ENABLE          0x00000001  /* R_MSIF_ENABLE = 1 */
   #define PCM_ZDS_ENABLE           0x00000001  /* R_ZDS_ENABLE = 1 */
   #define PCM_NCO_MUX_SHIFT        4
   #define PCM_CLOCK_SEL_ISI        0x7         /* div/12 clock divider for ISI mode input clock of 24.576Mhz */
   #define PCM_CLOCK_SEL_ZSI        0x7         /* div/?? clock divider for ZSI mode input clock of 24.576Mhz */
   #define PCM_CLOCK_SEL_PCM        0x2         /* div/8 clock divider for PCM/ZSI mode input clock of 16.384Mhz */

/*****************************************************************************
** BCM 68460 (impl2)
*****************************************************************************/
#elif defined(CONFIG_BCM96846)

   /* General settings & supported components */
   #define PCM_DPLL_SUPPORT         1
   #define iudma_get_irq_num()      iudma_get_irq_number
   #define IUDMA_NUM_CHANNELS       6
   #define APM_PCM_CTRL             0xC00 
   #define APM_DMA_CTRL             0x1800

   #define PCM_SAMPLE_SIZE          0x00C00000
   #define PCM_SAMPLE_SIZE_SHIFT    22
   #define PCM_SAMPLE_SIZE_8        0
   #define PCM_SAMPLE_SIZE_16       1
   #define PCM_SAMPLE_SIZE_32       2
   #define PCM_CLK_DIV              0x0001C000
   #define PCM_CLK_DIV_SHIFT        14
   #define PCM_FRAME_SIZE           0x000000FF
   #define PCM_FRAME_SIZE_SHIFT     0

   /* DPLL settings */
   #define DPLL                     APM
   #define DPLL_CTRL_K0_CLEAR       ~0xF
   #define DPLL_PHASE_THRESH        5
   #define DPLL_K0                  12
   #define DPLL_NOM_FREQ            0xa7c5ac47

   /* NTR control settings */
   #define PCM_NTR_PON                       1
   #define PCM_NTR_CTRL_CLEAR                (~0x70000000)
   #define PCM_NTR_IN_CTRL_SHIFT             29 /* shift for PCM_NTR_IN clock routing field */
   #define PCM_NTR_IN_CTRL_SEL_DECT_NTR      3  /* Selection for routing DECT refclk as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_NTR_0    2  /* Selection for routing VDSL_NTR[0] as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_NTR_1    1  /* Selection for routing VDSL_NTR[1] as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_GPIO     0  /* Selection for routing NTR_PULES_IN as PCM_NTR_IN */

   /* PCM settings */
   #define PCM_DMA_CHANNEL_RX       4
   #define PCM_DMA_CHANNEL_TX       5
   #define P_NCO_FCW_MISC           0xa7c5ac47  /* 0xa7c5ac47 for DPLL */
   #define P_PCM_NCO_SHIFT          0x0         /* 0x0 for DPLL, 0x1 for MISC */
   #define P_PCM_NCO_MUX_CNTL_DPLL  0x2         /* 0x2 for DPLL */
   #define P_PCM_NCO_MUX_CNTL_MISC  0x3         /* 0x3 for MISC */
   #define PCM_CLK_CNTL_1_ISI       0x60000000  /* PCM_NCO clk_out = 24.576Mhz. */
   #define PCM_CLK_CNTL_1_ZSI       0xc0000000  /* PCM_NCO clk_out = 49.152Mhz. */
   #define PCM_CLK_CNTL_1_PCM       0x40000000  /* PCM_NCO clk_out = 16.384Mhz. */
   #define PCM_MSIF_ENABLE          0x00000001  /* R_MSIF_ENABLE = 1 */
   #define PCM_ZDS_ENABLE           0x00000001  /* R_ZDS_ENABLE = 1 */
   #define PCM_NCO_MUX_SHIFT        4
   #define PCM_CLOCK_SEL_ISI        0x7         /* div/12 clock divider for ISI mode input clock of 24.576Mhz */
   #define PCM_CLOCK_SEL_ZSI        0x7         /* div/?? clock divider for ZSI mode input clock of 24.576Mhz */
   #define PCM_CLOCK_SEL_PCM        0x2         /* div/8 clock divider for PCM/ZSI mode input clock of 16.384Mhz */

/*****************************************************************************
** BCM 68560 (impl2)
*****************************************************************************/
#elif defined(CONFIG_BCM96856)

   /* General settings & supported components */
   #define PCM_DPLL_SUPPORT         1
   #define iudma_get_irq_num()      iudma_get_irq_number
   #define IUDMA_NUM_CHANNELS       6
   #define APM_PCM_CTRL             0xC00 
   #define APM_DMA_CTRL             0x1800

   #define PCM_SAMPLE_SIZE          0x00C00000
   #define PCM_SAMPLE_SIZE_SHIFT    22
   #define PCM_SAMPLE_SIZE_8        0
   #define PCM_SAMPLE_SIZE_16       1
   #define PCM_SAMPLE_SIZE_32       2
   #define PCM_CLK_DIV              0x0001C000
   #define PCM_CLK_DIV_SHIFT        14
   #define PCM_FRAME_SIZE           0x000000FF
   #define PCM_FRAME_SIZE_SHIFT     0

   /* DPLL settings */
   #define DPLL                     APM
   #define DPLL_CTRL_K0_CLEAR       ~0xF
   #define DPLL_PHASE_THRESH        5
   #define DPLL_K0                  12
   #define DPLL_NOM_FREQ            0xa7c5ac47

   /* NTR control settings */
   #define PCM_NTR_PON                       1
   #define PCM_NTR_CTRL_CLEAR                (~0x70000000)
   #define PCM_NTR_IN_CTRL_SHIFT             29 /* shift for PCM_NTR_IN clock routing field */
   #define PCM_NTR_IN_CTRL_SEL_DECT_NTR      3  /* Selection for routing DECT refclk as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_NTR_0    2  /* Selection for routing VDSL_NTR[0] as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_NTR_1    1  /* Selection for routing VDSL_NTR[1] as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_GPIO     0  /* Selection for routing NTR_PULES_IN as PCM_NTR_IN */

   /* PCM settings */
   #define PCM_DMA_CHANNEL_RX       4
   #define PCM_DMA_CHANNEL_TX       5
   #define P_NCO_FCW_MISC           0xa7c5ac47  /* 0xa7c5ac47 for DPLL */
   #define P_PCM_NCO_SHIFT          0x0         /* 0x0 for DPLL, 0x1 for MISC */
   #define P_PCM_NCO_MUX_CNTL_DPLL  0x2         /* 0x2 for DPLL */
   #define P_PCM_NCO_MUX_CNTL_MISC  0x3         /* 0x3 for MISC */
   #define PCM_CLK_CNTL_1_ISI       0x60000000  /* PCM_NCO clk_out = 24.576Mhz. */
   #define PCM_CLK_CNTL_1_ZSI       0xc0000000  /* PCM_NCO clk_out = 49.152Mhz. */
   #define PCM_CLK_CNTL_1_PCM       0x40000000  /* PCM_NCO clk_out = 16.384Mhz. */
   #define PCM_MSIF_ENABLE          0x00000001  /* R_MSIF_ENABLE = 1 */
   #define PCM_ZDS_ENABLE           0x00000001  /* R_ZDS_ENABLE = 1 */
   #define PCM_NCO_MUX_SHIFT        4
   #define PCM_CLOCK_SEL_ISI        0x7         /* div/12 clock divider for ISI mode input clock of 24.576Mhz */
   #define PCM_CLOCK_SEL_ZSI        0x7         /* div/?? clock divider for ZSI mode input clock of 24.576Mhz */
   #define PCM_CLOCK_SEL_PCM        0x2         /* div/8 clock divider for PCM/ZSI mode input clock of 16.384Mhz */
/*****************************************************************************
** BCM 6855 (impl2)
*****************************************************************************/
#elif defined(CONFIG_BCM96855)

   /* General settings & supported components */
   #define PCM_DPLL_SUPPORT         1
   #define iudma_get_irq_num()      iudma_get_irq_number
   #define IUDMA_NUM_CHANNELS       6
   #define APM_PCM_CTRL             0xC00 
   #define APM_DMA_CTRL             0x1800

   #define PCM_SAMPLE_SIZE          0x00C00000
   #define PCM_SAMPLE_SIZE_SHIFT    22
   #define PCM_SAMPLE_SIZE_8        0
   #define PCM_SAMPLE_SIZE_16       1
   #define PCM_SAMPLE_SIZE_32       2
   #define PCM_CLK_DIV              0x0001C000
   #define PCM_CLK_DIV_SHIFT        14
   #define PCM_FRAME_SIZE           0x000000FF
   #define PCM_FRAME_SIZE_SHIFT     0

   /* DPLL settings */
   #define DPLL                     APM
   #define DPLL_CTRL_K0_CLEAR       ~0xF
   #define DPLL_PHASE_THRESH        5
   #define DPLL_K0                  12
   #define DPLL_NOM_FREQ            0xa7c5ac47

   /* NTR control settings */
   #define PCM_NTR_PON                       1
   #define PCM_NTR_CTRL_CLEAR                (~0x70000000)
   #define PCM_NTR_IN_CTRL_SHIFT             29 /* shift for PCM_NTR_IN clock routing field */
   #define PCM_NTR_IN_CTRL_SEL_DECT_NTR      3  /* Selection for routing DECT refclk as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_NTR_0    2  /* Selection for routing VDSL_NTR[0] as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_NTR_1    1  /* Selection for routing VDSL_NTR[1] as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_GPIO     0  /* Selection for routing NTR_PULES_IN as PCM_NTR_IN */

   /* PCM settings */
   #define PCM_DMA_CHANNEL_RX       4
   #define PCM_DMA_CHANNEL_TX       5
   #define P_NCO_FCW_MISC           0xa7c5ac47  /* 0xa7c5ac47 for DPLL */
   #define P_PCM_NCO_SHIFT          0x0         /* 0x0 for DPLL, 0x1 for MISC */
   #define P_PCM_NCO_MUX_CNTL_DPLL  0x2         /* 0x2 for DPLL */
   #define P_PCM_NCO_MUX_CNTL_MISC  0x3         /* 0x3 for MISC */
   #define PCM_CLK_CNTL_1_ISI       0x60000000  /* PCM_NCO clk_out = 24.576Mhz. */
   #define PCM_CLK_CNTL_1_ZSI       0xc0000000  /* PCM_NCO clk_out = 49.152Mhz. */
   #define PCM_CLK_CNTL_1_PCM       0x40000000  /* PCM_NCO clk_out = 16.384Mhz. */
   #define PCM_MSIF_ENABLE          0x00000001  /* R_MSIF_ENABLE = 1 */
   #define PCM_ZDS_ENABLE           0x00000001  /* R_ZDS_ENABLE = 1 */
   #define PCM_NCO_MUX_SHIFT        4
   #define PCM_CLOCK_SEL_ISI        0x7         /* div/12 clock divider for ISI mode input clock of 24.576Mhz */
   #define PCM_CLOCK_SEL_ZSI        0x7         /* div/?? clock divider for ZSI mode input clock of 24.576Mhz */
   #define PCM_CLOCK_SEL_PCM        0x2         /* div/8 clock divider for PCM/ZSI mode input clock of 16.384Mhz */

/*****************************************************************************
** BCM 63178 (impl2)
*****************************************************************************/
#elif defined(CONFIG_BCM963178)
   /* General settings & supported components */
   #define PCM_DPLL_SUPPORT         1
   #define iudma_get_irq_num()      iudma_get_irq_number
   #define IUDMA_NUM_CHANNELS       6
   #define APM_PCM_CTRL             0xC00 
   #define APM_DMA_CTRL             0x1800

   #define PCM_SAMPLE_SIZE          0x00C00000
   #define PCM_SAMPLE_SIZE_SHIFT    22
   #define PCM_SAMPLE_SIZE_8        0
   #define PCM_SAMPLE_SIZE_16       1
   #define PCM_SAMPLE_SIZE_32       2
   #define PCM_CLK_DIV              0x0001C000
   #define PCM_CLK_DIV_SHIFT        14
   #define PCM_FRAME_SIZE           0x000000FF
   #define PCM_FRAME_SIZE_SHIFT     0

   /* DPLL settings */
   #define DPLL                     APM
   #define DPLL_CTRL_K0_CLEAR       ~0xF
   #define DPLL_PHASE_THRESH        5
   #define DPLL_K0                  12
   #define DPLL_NOM_FREQ            0xa7c5ac47

   /* NTR control settings */
   #define PCM_NTR_DSL                       0  /* Set PCM_NTR_DSL to 1 when BRCM_NTR_SUPPORT enabled */
   #define PCM_NTR_CTRL_CLEAR                (~0x70000000)
   #define PCM_NTR_IN_CTRL_SHIFT             29 /* shift for PCM_NTR_IN clock routing field */
   #define PCM_NTR_IN_CTRL_SEL_DECT_NTR      3  /* Selection for routing DECT refclk as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_NTR_0    2  /* Selection for routing VDSL_NTR[0] as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_NTR_1    1  /* Selection for routing VDSL_NTR[1] as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_GPIO     0  /* Selection for routing NTR_PULES_IN as PCM_NTR_IN */

   /* PCM settings */
   #define PCM_DMA_CHANNEL_RX       4
   #define PCM_DMA_CHANNEL_TX       5
   #define P_NCO_FCW_MISC           0xa7c5ac47  /* 0xa7c5ac47 for DPLL */
   #define P_PCM_NCO_SHIFT          0x0         /* 0x0 for DPLL, 0x1 for MISC */
   #define P_PCM_NCO_MUX_CNTL_DPLL  0x2         /* 0x2 for DPLL */
   #define P_PCM_NCO_MUX_CNTL_MISC  0x3         /* 0x3 for MISC */
   #define PCM_CLK_CNTL_1_ISI       0x60000000  /* PCM_NCO clk_out = 24.576Mhz. */
   #define PCM_CLK_CNTL_1_ZSI       0xc0000000  /* PCM_NCO clk_out = 49.152Mhz. */
   #define PCM_CLK_CNTL_1_PCM       0x40000000  /* PCM_NCO clk_out = 16.384Mhz. */
   #define PCM_MSIF_ENABLE          0x00000001  /* R_MSIF_ENABLE = 1 */
   #define PCM_ZDS_ENABLE           0x00000001  /* R_ZDS_ENABLE = 1 */
   #define PCM_NCO_MUX_SHIFT        4
   #define PCM_CLOCK_SEL_ISI        0x7         /* div/12 clock divider for ISI mode input clock of 24.576Mhz */
   #define PCM_CLOCK_SEL_ZSI        0x7         /* div/?? clock divider for ZSI mode input clock of 24.576Mhz */
   #define PCM_CLOCK_SEL_PCM        0x2         /* div/8 clock divider for PCM/ZSI mode input clock of 16.384Mhz */

/*****************************************************************************
** BCM 47622 (impl2)
*****************************************************************************/
#elif defined(CONFIG_BCM947622)
   /* General settings & supported components */
   #define PCM_DPLL_SUPPORT         1
   #define iudma_get_irq_num()      iudma_get_irq_number
   #define IUDMA_NUM_CHANNELS       6
   #define APM_PCM_CTRL             0xC00 
   #define APM_DMA_CTRL             0x1800

   #define PCM_SAMPLE_SIZE          0x00C00000
   #define PCM_SAMPLE_SIZE_SHIFT    22
   #define PCM_SAMPLE_SIZE_8        0
   #define PCM_SAMPLE_SIZE_16       1
   #define PCM_SAMPLE_SIZE_32       2
   #define PCM_CLK_DIV              0x0001C000
   #define PCM_CLK_DIV_SHIFT        14
   #define PCM_FRAME_SIZE           0x000000FF
   #define PCM_FRAME_SIZE_SHIFT     0

   /* DPLL settings */
   #define DPLL                     APM
   #define DPLL_CTRL_K0_CLEAR       ~0xF
   #define DPLL_PHASE_THRESH        5
   #define DPLL_K0                  12
   #define DPLL_NOM_FREQ            0xa7c5ac47

   /* NTR control settings */
   #define PCM_NTR_PON                       1
   #define PCM_NTR_CTRL_CLEAR                (~0x70000000)
   #define PCM_NTR_IN_CTRL_SHIFT             29 /* shift for PCM_NTR_IN clock routing field */
   #define PCM_NTR_IN_CTRL_SEL_DECT_NTR      3  /* Selection for routing DECT refclk as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_NTR_0    2  /* Selection for routing VDSL_NTR[0] as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_NTR_1    1  /* Selection for routing VDSL_NTR[1] as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_GPIO     0  /* Selection for routing NTR_PULES_IN as PCM_NTR_IN */

   /* PCM settings */
   #define PCM_DMA_CHANNEL_RX       4
   #define PCM_DMA_CHANNEL_TX       5
   #define P_NCO_FCW_MISC           0xa7c5ac47  /* 0xa7c5ac47 for DPLL */
   #define P_PCM_NCO_SHIFT          0x0         /* 0x0 for DPLL, 0x1 for MISC */
   #define P_PCM_NCO_MUX_CNTL_DPLL  0x2         /* 0x2 for DPLL */
   #define P_PCM_NCO_MUX_CNTL_MISC  0x3         /* 0x3 for MISC */
   #define PCM_CLK_CNTL_1_ISI       0x60000000  /* PCM_NCO clk_out = 24.576Mhz. */
   #define PCM_CLK_CNTL_1_ZSI       0xc0000000  /* PCM_NCO clk_out = 49.152Mhz. */
   #define PCM_CLK_CNTL_1_PCM       0x40000000  /* PCM_NCO clk_out = 16.384Mhz. */
   #define PCM_MSIF_ENABLE          0x00000001  /* R_MSIF_ENABLE = 1 */
   #define PCM_ZDS_ENABLE           0x00000001  /* R_ZDS_ENABLE = 1 */
   #define PCM_NCO_MUX_SHIFT        4
   #define PCM_CLOCK_SEL_ISI        0x7         /* div/12 clock divider for ISI mode input clock of 24.576Mhz */
   #define PCM_CLOCK_SEL_ZSI        0x7         /* div/?? clock divider for ZSI mode input clock of 24.576Mhz */
   #define PCM_CLOCK_SEL_PCM        0x2         /* div/8 clock divider for PCM/ZSI mode input clock of 16.384Mhz */

/*****************************************************************************
** BCM 6878 (impl2)
*****************************************************************************/
#elif defined(CONFIG_BCM96878)

   /* General settings & supported components */
   #define PCM_DPLL_SUPPORT         1
   #define iudma_get_irq_num()      iudma_get_irq_number
   #define IUDMA_NUM_CHANNELS       6
   #define APM_PCM_CTRL             0xC00 
   #define APM_DMA_CTRL             0x1800

   #define PCM_SAMPLE_SIZE          0x00C00000
   #define PCM_SAMPLE_SIZE_SHIFT    22
   #define PCM_SAMPLE_SIZE_8        0
   #define PCM_SAMPLE_SIZE_16       1
   #define PCM_SAMPLE_SIZE_32       2
   #define PCM_CLK_DIV              0x0001C000
   #define PCM_CLK_DIV_SHIFT        14
   #define PCM_FRAME_SIZE           0x000000FF
   #define PCM_FRAME_SIZE_SHIFT     0

   /* DPLL settings */
   #define DPLL                     APM
   #define DPLL_CTRL_K0_CLEAR       ~0xF
   #define DPLL_PHASE_THRESH        5
   #define DPLL_K0                  12
   #define DPLL_NOM_FREQ            0xa7c5ac47

   /* NTR control settings */
   #define PCM_NTR_PON                       1
   #define PCM_NTR_CTRL_CLEAR                (~0x70000000)
   #define PCM_NTR_IN_CTRL_SHIFT             29 /* shift for PCM_NTR_IN clock routing field */
   #define PCM_NTR_IN_CTRL_SEL_DECT_NTR      3  /* Selection for routing DECT refclk as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_NTR_0    2  /* Selection for routing VDSL_NTR[0] as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_NTR_1    1  /* Selection for routing VDSL_NTR[1] as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_GPIO     0  /* Selection for routing NTR_PULES_IN as PCM_NTR_IN */

   /* PCM settings */
   #define PCM_DMA_CHANNEL_RX       4
   #define PCM_DMA_CHANNEL_TX       5
   #define P_NCO_FCW_MISC           0xa7c5ac47  /* 0xa7c5ac47 for DPLL */
   #define P_PCM_NCO_SHIFT          0x0         /* 0x0 for DPLL, 0x1 for MISC */
   #define P_PCM_NCO_MUX_CNTL_DPLL  0x2         /* 0x2 for DPLL */
   #define P_PCM_NCO_MUX_CNTL_MISC  0x3         /* 0x3 for MISC */
   #define PCM_CLK_CNTL_1_ISI       0x60000000  /* PCM_NCO clk_out = 24.576Mhz. */
   #define PCM_CLK_CNTL_1_ZSI       0xc0000000  /* PCM_NCO clk_out = 49.152Mhz. */
   #define PCM_CLK_CNTL_1_PCM       0x40000000  /* PCM_NCO clk_out = 16.384Mhz. */
   #define PCM_MSIF_ENABLE          0x00000001  /* R_MSIF_ENABLE = 1 */
   #define PCM_ZDS_ENABLE           0x00000001  /* R_ZDS_ENABLE = 1 */
   #define PCM_NCO_MUX_SHIFT        4
   #define PCM_CLOCK_SEL_ISI        0x7         /* div/12 clock divider for ISI mode input clock of 24.576Mhz */
   #define PCM_CLOCK_SEL_ZSI        0x7         /* div/?? clock divider for ZSI mode input clock of 24.576Mhz */
   #define PCM_CLOCK_SEL_PCM        0x2         /* div/8 clock divider for PCM/ZSI mode input clock of 16.384Mhz */

/*****************************************************************************
** BCM 63146 (impl2)
*****************************************************************************/
#elif defined(CONFIG_BCM963146)
   /* General settings & supported components */
   #define PCM_DPLL_SUPPORT         1
   #define iudma_get_irq_num()      iudma_get_irq_number
   #define IUDMA_NUM_CHANNELS       6
   #define APM_PCM_CTRL             0xC00 
   #define APM_DMA_CTRL             0x1800

   #define PCM_SAMPLE_SIZE          0x00C00000
   #define PCM_SAMPLE_SIZE_SHIFT    22
   #define PCM_SAMPLE_SIZE_8        0
   #define PCM_SAMPLE_SIZE_16       1
   #define PCM_SAMPLE_SIZE_32       2
   #define PCM_CLK_DIV              0x0001C000
   #define PCM_CLK_DIV_SHIFT        14
   #define PCM_FRAME_SIZE           0x000000FF
   #define PCM_FRAME_SIZE_SHIFT     0

   /* DPLL settings */
   #define DPLL                     APM
   #define DPLL_CTRL_K0_CLEAR       ~0xF
   #define DPLL_PHASE_THRESH        5
   #define DPLL_K0                  12
   #define DPLL_NOM_FREQ            0xa7c5ac47

   /* NTR control settings */
   // #define BRCM_NTR_SUPPORT                  1 /* enble this for ntr test */
   #define PCM_NTR_PON                       1 
   #define PCM_NTR_CTRL_CLEAR                (~0x70000000)
   #define PCM_NTR_IN_CTRL_SHIFT             29 /* shift for PCM_NTR_IN clock routing field */
   #define PCM_NTR_IN_CTRL_SEL_DECT_NTR      3  /* Selection for routing DECT refclk as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_NTR_0    2  /* Selection for routing VDSL_NTR[0] as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_NTR_1    1  /* Selection for routing VDSL_NTR[1] as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_GPIO     0  /* Selection for routing NTR_PULES_IN as PCM_NTR_IN */

   /* PCM settings */
   #define PCM_DMA_CHANNEL_RX       4
   #define PCM_DMA_CHANNEL_TX       5
   #define P_NCO_FCW_MISC           0xa7c5ac47  /* 0xa7c5ac47 for DPLL */
   #define P_PCM_NCO_SHIFT          0x0         /* 0x0 for DPLL, 0x1 for MISC */
   #define P_PCM_NCO_MUX_CNTL_DPLL  0x2         /* 0x2 for DPLL */
   #define P_PCM_NCO_MUX_CNTL_MISC  0x3         /* 0x3 for MISC */
   #define PCM_CLK_CNTL_1_ISI       0x60000000  /* PCM_NCO clk_out = 24.576Mhz. */
   #define PCM_CLK_CNTL_1_ZSI       0xc0000000  /* PCM_NCO clk_out = 49.152Mhz. */
   #define PCM_CLK_CNTL_1_PCM       0x40000000  /* PCM_NCO clk_out = 16.384Mhz. */
   #define PCM_MSIF_ENABLE          0x00000001  /* R_MSIF_ENABLE = 1 */
   #define PCM_ZDS_ENABLE           0x00000001  /* R_ZDS_ENABLE = 1 */
   #define PCM_NCO_MUX_SHIFT        4
   #define PCM_CLOCK_SEL_ISI        0x7         /* div/12 clock divider for ISI mode input clock of 24.576Mhz */
   #define PCM_CLOCK_SEL_ZSI        0x7         /* div/?? clock divider for ZSI mode input clock of 24.576Mhz */
   #define PCM_CLOCK_SEL_PCM        0x2         /* div/8 clock divider for PCM/ZSI mode input clock of 16.384Mhz */

/*****************************************************************************
** BCM 4912 (impl2)
*****************************************************************************/
#elif defined(CONFIG_BCM94912)
   /* General settings & supported components */
   #define PCM_DPLL_SUPPORT         1
   #define iudma_get_irq_num()      iudma_get_irq_number
   #define IUDMA_NUM_CHANNELS       6
   #define APM_PCM_CTRL             0xC00 
   #define APM_DMA_CTRL             0x1800

   #define PCM_SAMPLE_SIZE          0x00C00000
   #define PCM_SAMPLE_SIZE_SHIFT    22
   #define PCM_SAMPLE_SIZE_8        0
   #define PCM_SAMPLE_SIZE_16       1
   #define PCM_SAMPLE_SIZE_32       2
   #define PCM_CLK_DIV              0x0001C000
   #define PCM_CLK_DIV_SHIFT        14
   #define PCM_FRAME_SIZE           0x000000FF
   #define PCM_FRAME_SIZE_SHIFT     0

   /* DPLL settings */
   #define DPLL                     APM
   #define DPLL_CTRL_K0_CLEAR       ~0xF
   #define DPLL_PHASE_THRESH        5
   #define DPLL_K0                  12
   #define DPLL_NOM_FREQ            0xa7c5ac47

   /* NTR control settings */
   //#define BRCM_NTR_SUPPORT                  1 /* enable it for ntr verification */ 
   #define PCM_NTR_RTR                       1 
   #define PCM_NTR_CTRL_CLEAR                (~0x70000000)
   #define PCM_NTR_IN_CTRL_SHIFT             29 /* shift for PCM_NTR_IN clock routing field */
   #define PCM_NTR_IN_CTRL_SEL_DECT_NTR      3  /* Selection for routing DECT refclk as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_NTR_0    2  /* Selection for routing VDSL_NTR[0] as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_NTR_1    1  /* Selection for routing VDSL_NTR[1] as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_GPIO     0  /* Selection for routing NTR_PULES_IN as PCM_NTR_IN */

   /* PCM settings */
   #define PCM_DMA_CHANNEL_RX       4
   #define PCM_DMA_CHANNEL_TX       5
   #define P_NCO_FCW_MISC           0xa7c5ac47  /* 0xa7c5ac47 for DPLL */
   #define P_PCM_NCO_SHIFT          0x0         /* 0x0 for DPLL, 0x1 for MISC */
   #define P_PCM_NCO_MUX_CNTL_DPLL  0x2         /* 0x2 for DPLL */
   #define P_PCM_NCO_MUX_CNTL_MISC  0x3         /* 0x3 for MISC */
   #define PCM_CLK_CNTL_1_ISI       0x60000000  /* PCM_NCO clk_out = 24.576Mhz. */
   #define PCM_CLK_CNTL_1_ZSI       0xc0000000  /* PCM_NCO clk_out = 49.152Mhz. */
   #define PCM_CLK_CNTL_1_PCM       0x40000000  /* PCM_NCO clk_out = 16.384Mhz. */
   #define PCM_MSIF_ENABLE          0x00000001  /* R_MSIF_ENABLE = 1 */
   #define PCM_ZDS_ENABLE           0x00000001  /* R_ZDS_ENABLE = 1 */
   #define PCM_NCO_MUX_SHIFT        4
   #define PCM_CLOCK_SEL_ISI        0x7         /* div/12 clock divider for ISI mode input clock of 24.576Mhz */
   #define PCM_CLOCK_SEL_ZSI        0x7         /* div/?? clock divider for ZSI mode input clock of 24.576Mhz */
   #define PCM_CLOCK_SEL_PCM        0x2         /* div/8 clock divider for PCM/ZSI mode input clock of 16.384Mhz */

/*****************************************************************************
** BCM 6756 (impl2)
*****************************************************************************/
#elif defined(CONFIG_BCM96756)
   /* General settings & supported components */
   #define PCM_DPLL_SUPPORT         1
   #define iudma_get_irq_num()      iudma_get_irq_number
   #define IUDMA_NUM_CHANNELS       6
   #define APM_PCM_CTRL             0xC00 
   #define APM_DMA_CTRL             0x1800

   #define PCM_SAMPLE_SIZE          0x00C00000
   #define PCM_SAMPLE_SIZE_SHIFT    22
   #define PCM_SAMPLE_SIZE_8        0
   #define PCM_SAMPLE_SIZE_16       1
   #define PCM_SAMPLE_SIZE_32       2
   #define PCM_CLK_DIV              0x0001C000
   #define PCM_CLK_DIV_SHIFT        14
   #define PCM_FRAME_SIZE           0x000000FF
   #define PCM_FRAME_SIZE_SHIFT     0

   /* DPLL settings */
   #define DPLL                     APM
   #define DPLL_CTRL_K0_CLEAR       ~0xF
   #define DPLL_PHASE_THRESH        5
   #define DPLL_K0                  12
   #define DPLL_NOM_FREQ            0xa7c5ac47

   /* NTR control settings */
   //#define BRCM_NTR_SUPPORT                  1 /* enble this for ntr test */
   #define PCM_NTR_PON                       1
   #define PCM_NTR_CTRL_CLEAR                (~0x70000000)
   #define PCM_NTR_IN_CTRL_SHIFT             29 /* shift for PCM_NTR_IN clock routing field */
   #define PCM_NTR_IN_CTRL_SEL_DECT_NTR      3  /* Selection for routing DECT refclk as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_NTR_0    2  /* Selection for routing VDSL_NTR[0] as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_NTR_1    1  /* Selection for routing VDSL_NTR[1] as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_GPIO     0  /* Selection for routing NTR_PULES_IN as PCM_NTR_IN */

   /* PCM settings */
   #define PCM_DMA_CHANNEL_RX       4
   #define PCM_DMA_CHANNEL_TX       5
   #define P_NCO_FCW_MISC           0xa7c5ac47  /* 0xa7c5ac47 for DPLL */
   #define P_PCM_NCO_SHIFT          0x0         /* 0x0 for DPLL, 0x1 for MISC */
   #define P_PCM_NCO_MUX_CNTL_DPLL  0x2         /* 0x2 for DPLL */
   #define P_PCM_NCO_MUX_CNTL_MISC  0x3         /* 0x3 for MISC */
   #define PCM_CLK_CNTL_1_ISI       0x60000000  /* PCM_NCO clk_out = 24.576Mhz. */
   #define PCM_CLK_CNTL_1_ZSI       0xc0000000  /* PCM_NCO clk_out = 49.152Mhz. */
   #define PCM_CLK_CNTL_1_PCM       0x40000000  /* PCM_NCO clk_out = 16.384Mhz. */
   #define PCM_MSIF_ENABLE          0x00000001  /* R_MSIF_ENABLE = 1 */
   #define PCM_ZDS_ENABLE           0x00000001  /* R_ZDS_ENABLE = 1 */
   #define PCM_NCO_MUX_SHIFT        4
   #define PCM_CLOCK_SEL_ISI        0x7         /* div/12 clock divider for ISI mode input clock of 24.576Mhz */
   #define PCM_CLOCK_SEL_ZSI        0x7         /* div/?? clock divider for ZSI mode input clock of 24.576Mhz */
   #define PCM_CLOCK_SEL_PCM        0x2         /* div/8 clock divider for PCM/ZSI mode input clock of 16.384Mhz */
/*****************************************************************************
** BCM 6888 (impl2)
*****************************************************************************/
#elif defined(CONFIG_BCM96888) || defined(CONFIG_BCM968880) || defined(CONFIG_BCM96837) || defined(CONFIG_BCM96765)
   /* General settings & supported components */
   #define PCM_DPLL_SUPPORT         1
   #define iudma_get_irq_num()      iudma_get_irq_number
   #define IUDMA_NUM_CHANNELS       6
   #define APM_PCM_CTRL             0xC00 
   #define APM_DMA_CTRL             0x1800

   #define PCM_SAMPLE_SIZE          0x00C00000
   #define PCM_SAMPLE_SIZE_SHIFT    22
   #define PCM_SAMPLE_SIZE_8        0
   #define PCM_SAMPLE_SIZE_16       1
   #define PCM_SAMPLE_SIZE_32       2
   #define PCM_CLK_DIV              0x0001C000
   #define PCM_CLK_DIV_SHIFT        14
   #define PCM_FRAME_SIZE           0x000000FF
   #define PCM_FRAME_SIZE_SHIFT     0

   /* DPLL settings */
   #define DPLL                     APM
   #define DPLL_CTRL_K0_CLEAR       ~0xF
   #define DPLL_PHASE_THRESH        5
   #define DPLL_K0                  12
   #define DPLL_NOM_FREQ            0xa7c5ac47

   /* NTR control settings */
   //#define BRCM_NTR_SUPPORT                  1 /* enble this for ntr test */
   #define PCM_NTR_PON                       1
   #define PCM_NTR_CTRL_CLEAR                (~0x70000000)
   #define PCM_NTR_IN_CTRL_SHIFT             29 /* shift for PCM_NTR_IN clock routing field */
   #define PCM_NTR_IN_CTRL_SEL_DECT_NTR      3  /* Selection for routing DECT refclk as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_NTR_0    2  /* Selection for routing VDSL_NTR[0] as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_NTR_1    1  /* Selection for routing VDSL_NTR[1] as PCM_NTR_IN */
   #define PCM_NTR_IN_CTRL_SEL_VDSL_GPIO     0  /* Selection for routing NTR_PULES_IN as PCM_NTR_IN */

   /* PCM settings */
   #define PCM_DMA_CHANNEL_RX       4
   #define PCM_DMA_CHANNEL_TX       5
   #define P_NCO_FCW_MISC           0xa7c5ac47  /* 0xa7c5ac47 for DPLL */
   #define P_PCM_NCO_SHIFT          0x0         /* 0x0 for DPLL, 0x1 for MISC */
   #define P_PCM_NCO_MUX_CNTL_DPLL  0x2         /* 0x2 for DPLL */
   #define P_PCM_NCO_MUX_CNTL_MISC  0x3         /* 0x3 for MISC */
   #define PCM_CLK_CNTL_1_ISI       0x60000000  /* PCM_NCO clk_out = 24.576Mhz. */
   #define PCM_CLK_CNTL_1_ZSI       0xc0000000  /* PCM_NCO clk_out = 49.152Mhz. */
   #define PCM_CLK_CNTL_1_PCM       0x40000000  /* PCM_NCO clk_out = 16.384Mhz. */
   #define PCM_MSIF_ENABLE          0x00000001  /* R_MSIF_ENABLE = 1 */
   #define PCM_ZDS_ENABLE           0x00000001  /* R_ZDS_ENABLE = 1 */
   #define PCM_NCO_MUX_SHIFT        4
   #define PCM_CLOCK_SEL_ISI        0x7         /* div/12 clock divider for ISI mode input clock of 24.576Mhz */
   #define PCM_CLOCK_SEL_ZSI        0x7         /* div/?? clock divider for ZSI mode input clock of 24.576Mhz */
   #define PCM_CLOCK_SEL_PCM        0x2         /* div/8 clock divider for PCM/ZSI mode input clock of 16.384Mhz */
#else
   #error "No DSPHAL supported on this chip"
#endif

/* Common defines */
#ifndef DMA_COHERENT_BITS
#   define DMA_COHERENT_BITS        32
#endif
#ifndef pcm_get_tx_ch_index
#   define pcm_get_tx_ch_index(x)   (x)
#endif
#ifndef pcm_get_rx_ch_index
#   define pcm_get_rx_ch_index(x)   (x)
#endif


#endif /* _DSPHAL_CHIP_H */
