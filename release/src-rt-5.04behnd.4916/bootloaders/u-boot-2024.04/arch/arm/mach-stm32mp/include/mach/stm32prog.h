/* SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause */
/*
 * Copyright (C) 2020, STMicroelectronics - All Rights Reserved
 */

#define STM32PROG_VIRT_FIRST_DEV_NUM		0xF1

int stm32prog_write_medium_virt(struct dfu_entity *dfu, u64 offset,
				void *buf, long *len);
int stm32prog_read_medium_virt(struct dfu_entity *dfu, u64 offset,
			       void *buf, long *len);
int stm32prog_get_medium_size_virt(struct dfu_entity *dfu, u64 *size);

bool stm32prog_get_fsbl_nor(void);
