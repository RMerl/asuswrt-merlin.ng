/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2024 Mathieu Othacehe <m.othacehe@gmail.com>
 */

#ifndef __ARCH_IMX9_MU_H
#define __ARCH_IMX9_MU_H

#include <event.h>

int imx9_probe_mu(void *ctx, struct event *event);

#endif
