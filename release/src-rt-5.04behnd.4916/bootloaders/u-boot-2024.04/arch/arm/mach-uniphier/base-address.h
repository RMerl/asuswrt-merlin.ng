/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2019 Socionext Inc.
 */

#ifndef __UNIPHIER_BASE_ADDRESS_H
#define __UNIPHIER_BASE_ADDRESS_H

#ifdef CONFIG_ARCH_UNIPHIER_V8_MULTI
int uniphier_base_address_init(void);
#else
static inline int uniphier_base_address_init(void)
{
	return 0;
}
#endif

#endif /* __UNIPHIER_BASE_ADDRESS_H */
