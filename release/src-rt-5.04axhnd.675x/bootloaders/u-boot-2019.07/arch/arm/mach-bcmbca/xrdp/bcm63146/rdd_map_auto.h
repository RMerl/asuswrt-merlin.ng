// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    
*/



/* This is an automated file. Do not edit its contents. */


#ifndef _RDD_MAP_AUTO_H_
#define _RDD_MAP_AUTO_H_

typedef enum
{
	image_0_runner_image = 0,
	cfe_core_runner_image = image_0_runner_image,
	runner_image_last = 0,
} rdp_runner_image_e;

extern rdp_runner_image_e rdp_core_to_image_map[];
#endif /* _RDD_MAP_AUTO_H_ */
