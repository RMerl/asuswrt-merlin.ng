/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Generic QEMU header
 *
 * Copyright 2023 Google LLC
 */

#ifndef __QEMU_H
#define __QEMU_H

/* set up the chipset for QEMU so that video can be used */
void qemu_chipset_init(void);

#endif
