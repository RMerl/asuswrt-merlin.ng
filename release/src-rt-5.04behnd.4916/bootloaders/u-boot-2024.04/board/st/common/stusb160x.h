/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020, STMicroelectronics
 */

#ifdef CONFIG_TYPEC_STUSB160X
int stusb160x_cable_connected(void);
#else
int stusb160x_cable_connected(void) { return -ENODEV; }
#endif
