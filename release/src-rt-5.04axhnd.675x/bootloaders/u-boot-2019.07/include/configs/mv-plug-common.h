/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009-2015
 * Marvell Semiconductor <www.marvell.com>
 */

#ifndef _CONFIG_MARVELL_PLUG_H
#define _CONFIG_MARVELL_PLUG_H

/*
 * High Level Configuration Options (easy to change)
 */
#define CONFIG_KW88F6281	1	/* SOC Name */
#define CONFIG_SKIP_LOWLEVEL_INIT	/* disable board lowlevel_init */

/*
 * mv-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-common.h"

#endif /* _CONFIG_MARVELL_PLUG_H */
