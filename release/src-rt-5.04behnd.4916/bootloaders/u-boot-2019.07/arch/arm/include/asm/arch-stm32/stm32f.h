/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 * Author(s): Patrice Chotard, <patrice.chotard@st.com> for STMicroelectronics.
 */

#ifndef _ASM_ARCH_STM32F_H
#define _ASM_ARCH_STM32F_H

#define STM32_PERIPH_BASE	0x40000000UL

#define STM32_APB2_PERIPH_BASE	(STM32_PERIPH_BASE + 0x00010000)
#define STM32_AHB1_PERIPH_BASE	(STM32_PERIPH_BASE + 0x00020000)

#define STM32_SYSCFG_BASE	(STM32_APB2_PERIPH_BASE + 0x3800)
#define STM32_FLASH_CNTL_BASE	(STM32_AHB1_PERIPH_BASE + 0x3C00)

void stm32_flash_latency_cfg(int latency);

#endif /* _ASM_ARCH_STM32F_H */

