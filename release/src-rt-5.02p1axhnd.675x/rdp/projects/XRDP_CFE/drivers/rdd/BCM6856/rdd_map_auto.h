/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
:>
*/



/* This is an automated file. Do not edit its contents. */


#ifndef _RDD_MAP_AUTO_H_
#define _RDD_MAP_AUTO_H_

typedef enum
{
	image_0_runner_image = 0,
	ds_tm_runner_image = image_0_runner_image,
	image_1_runner_image = 1,
	cpu_rx_runner_image = image_1_runner_image,
	wan_direct_runner_image = image_1_runner_image,
	image_2_runner_image = 2,
	cpu_tx_runner_image = image_2_runner_image,
	runner_image_last = 2,
} rdp_runner_image_e;

extern rdp_runner_image_e rdp_core_to_image_map[];
#endif /* _RDD_MAP_AUTO_H_ */
