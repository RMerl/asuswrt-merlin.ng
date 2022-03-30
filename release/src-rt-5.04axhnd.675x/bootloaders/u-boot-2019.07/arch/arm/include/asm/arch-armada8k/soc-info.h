/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2016 Marvell International Ltd.
 * https://spdx.org/licenses
 */

#ifndef _SOC_INFO_H_
#define _SOC_INFO_H_

/* Pin Ctrl driver definitions */
#define BITS_PER_PIN		4
#define PIN_FUNC_MASK		((1 << BITS_PER_PIN) - 1)
#define PIN_REG_SHIFT		3
#define PIN_FIELD_MASK		((1 << PIN_REG_SHIFT) - 1)

#endif	/* _SOC_INFO_H_ */
