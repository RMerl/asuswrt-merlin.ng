/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 NXP
 */

#ifndef __IMX_AHAB_H__
#define __IMX_AHAB_H__

#include <imx_container.h>

int ahab_auth_cntr_hdr(struct container_hdr *container, u16 length);
int ahab_auth_release(void);
int ahab_verify_cntr_image(struct boot_img_t *img, int image_index);

#endif
