/* SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (C) 2016 The Android Open Source Project
 */

#ifndef __NET_FASTBOOT_H__
#define __NET_FASTBOOT_H__

/**********************************************************************/
/*
 *	Global functions and variables.
 */

/**
 * Wait for incoming fastboot comands.
 */
void fastboot_start_server(void);

/**********************************************************************/

#endif /* __NET_FASTBOOT_H__ */
