/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#ifndef __UNIPHIER_SOC_INFO_H__
#define __UNIPHIER_SOC_INFO_H__

#include <linux/kernel.h>
#include <linux/stddef.h>

#define UNIPHIER_LD4_ID		0x26
#define UNIPHIER_PRO4_ID	0x28
#define UNIPHIER_SLD8_ID	0x29
#define UNIPHIER_PRO5_ID	0x2a
#define UNIPHIER_PXS2_ID	0x2e
#define UNIPHIER_LD6B_ID	0x2f
#define UNIPHIER_LD11_ID	0x31
#define UNIPHIER_LD20_ID	0x32
#define UNIPHIER_PXS3_ID	0x35

unsigned int uniphier_get_soc_id(void);
unsigned int uniphier_get_soc_model(void);
unsigned int uniphier_get_soc_revision(void);

#define UNIPHIER_DEFINE_SOCDATA_FUNC(__func_name, __table)	\
static typeof(&__table[0]) __func_name(void)			\
{								\
	unsigned int soc_id;					\
	int i;							\
								\
	soc_id = uniphier_get_soc_id();				\
	for (i = 0; i < ARRAY_SIZE(__table); i++) {		\
		if (__table[i].soc_id == soc_id)		\
			return &__table[i];			\
	}							\
								\
	return NULL;						\
}

#endif /* __UNIPHIER_SOC_INFO_H__ */
