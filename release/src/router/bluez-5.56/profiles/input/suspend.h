/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012  Nordic Semiconductor Inc.
 *  Copyright (C) 2012  Instituto Nokia de Tecnologia - INdT
 *
 *
 */

typedef void (*suspend_event) (void);
typedef void (*resume_event) (void);

int suspend_init(suspend_event suspend, resume_event resume);
void suspend_exit(void);
