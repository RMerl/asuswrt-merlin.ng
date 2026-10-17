/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017 Texas Instruments
 * Written by Franklin Cooper Jr. <fcooper@ti.com>
 */

/**
 * locate_dtb_in_fit - Find a DTB matching the board in a FIT image
 * @fit:	pointer to the FIT image
 *
 * Return: a pointer to a matching DTB blob if found, NULL otherwise
 */
void *locate_dtb_in_fit(const void *fit);
