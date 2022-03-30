// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Broadcom
 */
/*
    
*/

#ifndef _RDD_INIT_H
#define _RDD_INIT_H

typedef struct
{
	uint8_t *ddr0_runner_base_ptr;
	int is_basic;
} rdd_init_params_t;

int rdd_init(void);
void rdd_exit(void);

int rdd_data_structures_init(rdd_init_params_t *init_params);


#endif /* _RDD_INIT_H */
