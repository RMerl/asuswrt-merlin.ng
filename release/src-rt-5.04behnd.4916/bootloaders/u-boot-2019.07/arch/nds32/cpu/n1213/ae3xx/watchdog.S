/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011 Andes Technology Corporation
 * Macpaul Lin, Andes Technology Corporation <macpaul@andestech.com>
 */

#include <asm/arch-ag101/ag101.h>
#include <linux/linkage.h>

.text

#ifndef CONFIG_SKIP_TRUNOFF_WATCHDOG
ENTRY(turnoff_watchdog)
#error "AE3XX not support wdt yet"
ENDPROC(turnoff_watchdog)
#endif
