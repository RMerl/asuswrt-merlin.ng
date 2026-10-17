/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2022 Gateworks Corporation
 */

#ifndef _GSC_H_
#define _GSC_H_

/*
 * board_gsc_info - Display additional board info
 */
void board_gsc_info(void);

/*
 * gsc_boot_wd_disable - disable the BOOT watchdog
 *
 * Return: 0 on success or negative error on failure
 */
int gsc_boot_wd_disable(void);

#endif
