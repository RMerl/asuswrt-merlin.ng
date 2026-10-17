/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Patrice Chotard, <patrice.chotard@foss.st.com> for STMicroelectronics.
 */

#ifndef __STM32_PWR_H_
#define __STM32_PWR_H_

#ifndef __ASSEMBLY__
#include <linux/bitops.h>
#endif

/*
 * Offsets of some PWR registers
 */
#define PWR_CR1_ODEN			BIT(16)
#define PWR_CR1_ODSWEN			BIT(17)
#define PWR_CSR1_ODRDY			BIT(16)
#define PWR_CSR1_ODSWRDY		BIT(17)

struct stm32_pwr_regs {
	u32 cr1;   /* power control register 1 */
	u32 csr1;  /* power control/status register 2 */
	u32 cr2;   /* power control register 2 */
	u32 csr2;  /* power control/status register 2 */
};

#endif /* __STM32_PWR_H_ */
