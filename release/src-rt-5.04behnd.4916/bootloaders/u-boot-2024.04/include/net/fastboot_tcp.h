/* SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (C) 2023 The Android Open Source Project
 */

#ifndef __NET_FASTBOOT_TCP_H__
#define __NET_FASTBOOT_TCP_H__

/**
 * Wait for incoming tcp fastboot comands.
 */
void fastboot_tcp_start_server(void);

#endif /* __NET_FASTBOOT_TCP_H__ */
