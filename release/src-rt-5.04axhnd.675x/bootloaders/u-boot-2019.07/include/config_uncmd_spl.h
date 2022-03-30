/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2012
 * Ilya Yanok, ilya.yanok@gmail.com
 */

#ifndef __CONFIG_UNCMD_SPL_H__
#define __CONFIG_UNCMD_SPL_H__

#ifdef CONFIG_SPL_BUILD
/* SPL needs only BOOTP + TFTP so undefine other stuff to save space */

#ifndef CONFIG_SPL_DM
#undef CONFIG_DM_SERIAL
#undef CONFIG_DM_GPIO
#undef CONFIG_DM_I2C
#undef CONFIG_DM_SPI
#endif

#undef CONFIG_DM_WARN
#undef CONFIG_DM_STDIO

#endif /* CONFIG_SPL_BUILD */
#endif /* __CONFIG_UNCMD_SPL_H__ */
