/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2019 Linaro
 * Author: Manivannan Sadhasivam <manivannan.sadhasivam@linaro.org>
 */

#ifndef __HI3660_H__
#define __HI3660_H__

#ifndef __ASSEMBLY__
#include <linux/bitops.h>
#endif

#define HI3660_UART6_BASE			0xfff32000

#define PMU_REG_BASE                            0xfff34000
#define PMIC_HARDWARE_CTRL0                     (PMU_REG_BASE + (0x0C5 << 2))

#define SCTRL_REG_BASE                          0xfff0a000
#define SCTRL_SCFPLLCTRL0                       (SCTRL_REG_BASE + 0x120)
#define SCTRL_SCFPLLCTRL0_FPLL0_EN              BIT(0)

#define CRG_REG_BASE                            0xfff35000
#define CRG_PEREN2                              (CRG_REG_BASE + 0x020)
#define CRG_PERDIS2                             (CRG_REG_BASE + 0x024)
#define CRG_PERCLKEN2                           (CRG_REG_BASE + 0x028)
#define CRG_PERSTAT2                            (CRG_REG_BASE + 0x02C)
#define CRG_PEREN4                              (CRG_REG_BASE + 0x040)
#define CRG_PERDIS4                             (CRG_REG_BASE + 0x044)
#define CRG_PERCLKEN4                           (CRG_REG_BASE + 0x048)
#define CRG_PERSTAT4                            (CRG_REG_BASE + 0x04C)
#define CRG_PERRSTEN2                           (CRG_REG_BASE + 0x078)
#define CRG_PERRSTDIS2                          (CRG_REG_BASE + 0x07C)
#define CRG_PERRSTSTAT2                         (CRG_REG_BASE + 0x080)
#define CRG_PERRSTEN3                           (CRG_REG_BASE + 0x084)
#define CRG_PERRSTDIS3                          (CRG_REG_BASE + 0x088)
#define CRG_PERRSTSTAT3                         (CRG_REG_BASE + 0x08C)
#define CRG_PERRSTEN4                           (CRG_REG_BASE + 0x090)
#define CRG_PERRSTDIS4                          (CRG_REG_BASE + 0x094)
#define CRG_PERRSTSTAT4                         (CRG_REG_BASE + 0x098)
#define CRG_ISOEN                               (CRG_REG_BASE + 0x144)
#define CRG_ISODIS                              (CRG_REG_BASE + 0x148)
#define CRG_ISOSTAT                             (CRG_REG_BASE + 0x14C)

#define PINMUX4_BASE				0xfff11000
#define PINMUX4_SDDET				(PINMUX4_BASE + 0x60)

#define PINCONF3_BASE				0xff37e800
#define PINCONF3_SDCLK				(PINCONF3_BASE + 0x00)
#define PINCONF3_SDCMD				(PINCONF3_BASE + 0x04)
#define PINCONF3_SDDATA0			(PINCONF3_BASE + 0x08)
#define PINCONF3_SDDATA1			(PINCONF3_BASE + 0x0c)
#define PINCONF3_SDDATA2			(PINCONF3_BASE + 0x10)
#define PINCONF3_SDDATA3			(PINCONF3_BASE + 0x14)

#endif /*__HI3660_H__*/
