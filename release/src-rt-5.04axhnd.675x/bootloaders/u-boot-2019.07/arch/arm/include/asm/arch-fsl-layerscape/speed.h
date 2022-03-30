/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2014-2015, Freescale Semiconductor, Inc.
 */

#ifndef _FSL_LAYERSCAPE_SPEED_H
#define _FSL_LAYERSCAPE_SPEED_H
void get_sys_info(struct sys_info *sys_info);
#ifdef CONFIG_SYS_DPAA_QBMAN
unsigned long get_qman_freq(void);
#endif
#endif /* _FSL_LAYERSCAPE_SPEED_H */
