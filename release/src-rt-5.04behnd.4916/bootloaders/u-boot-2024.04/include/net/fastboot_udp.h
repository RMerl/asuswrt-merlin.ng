/* SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (C) 2016 The Android Open Source Project
 */

#ifndef __NET_FASTBOOT_H__
#define __NET_FASTBOOT_H__

/**
 * Wait for incoming UDP fastboot comands.
 */
void fastboot_udp_start_server(void);

#endif /* __NET_FASTBOOT_H__ */
