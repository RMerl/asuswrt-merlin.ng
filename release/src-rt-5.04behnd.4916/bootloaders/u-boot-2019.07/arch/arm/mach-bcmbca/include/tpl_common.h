/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#ifndef _TPL_COMMON_H
#define _TPL_COMMON_H


#include <ubispl.h>
#include <part.h>
#include <image.h>

typedef union
{
	struct ubispl_info ubi_info;
	disk_partition_t gpt_info;
} bcaspl_part_info;


int tpl_get_ubi_info( bcaspl_part_info * part_info);
char * tpl_get_imgdev_name(void);
#endif



