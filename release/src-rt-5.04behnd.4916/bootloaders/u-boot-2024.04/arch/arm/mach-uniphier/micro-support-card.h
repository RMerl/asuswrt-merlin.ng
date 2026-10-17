/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2012-2014 Panasonic Corporation
 * Copyright (C) 2015-2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#ifndef MICRO_SUPPORT_CARD_H
#define MICRO_SUPPORT_CARD_H

#if defined(CONFIG_MICRO_SUPPORT_CARD)
void support_card_init(void);
void support_card_late_init(void);
void led_puts(const char *s);
#else
static inline void support_card_init(void)
{
}

static inline void support_card_late_init(void)
{
}

static inline void led_puts(const char *s)
{
}
#endif

#endif /* MICRO_SUPPORT_CARD_H */
