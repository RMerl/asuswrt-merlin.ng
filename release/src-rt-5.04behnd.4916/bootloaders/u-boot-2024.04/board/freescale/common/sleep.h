/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 */

#ifndef __SLEEP_H
#define __SLEEP_H

#define DCFG_CCSR_CRSTSR_WDRFR	(1 << 3)
#define DDR_BUFF_LEN			128

/* determine if it is a wakeup from deep sleep */
bool is_warm_boot(void);

/* disable console output */
void fsl_dp_disable_console(void);

/* clean up everything and jump to kernel */
int fsl_dp_resume(void);
#endif
