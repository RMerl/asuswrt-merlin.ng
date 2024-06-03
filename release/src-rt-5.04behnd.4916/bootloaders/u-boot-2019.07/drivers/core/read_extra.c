// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <dm/of_addr.h>
#include <dm/read.h>
#include <linux/ioport.h>

/* This file can hold non-inlined dev_read_...() functions */
