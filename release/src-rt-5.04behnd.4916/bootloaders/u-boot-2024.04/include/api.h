/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __API_H
#define __API_H

/**
 * api_init() - Initialize API for external applications
 *
 * Initialize API for external (standalone) applications running on top of
 * U-Boot. It is called during the generic post-relocation init sequence.
 *
 * Return: 0 if OK
 */
int api_init(void);

#endif
