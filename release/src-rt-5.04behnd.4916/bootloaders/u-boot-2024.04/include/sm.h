/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2023 SberDevices, Inc.
 *
 * Author: Alexey Romanov <avromanov@salutedevices.ru>
 */

#ifndef __SM_H__
#define __SM_H__

/*
 * NOTE: UCLASS_SM is designed with the idea that
 * each driver should convert @cmd to some raw
 * value, which is known only for driver, and set this
 * value to the first element of the @args->regs array.
 * Therefore, it is necessary to pass the remaining
 * arguments starting at index = 1. Anyway, driver
 * implementation may vary, so, please, check the specific
 * implementation of the driver you are using.
 */

#include <asm/types.h>
#include <asm/ptrace.h>

struct udevice;

/**
 * sm_call - generic SMC call to the secure-monitor
 *
 * @dev:	Pointer to UCLASS_SM device
 * @cmd_index:	Index of the SMC function ID
 * @smc_ret:	Returned value from secure world
 * @args:	SMC arguments
 *
 * @return:	0 on success, a negative value on error
 */
int sm_call(struct udevice *dev, u32 cmd, s32 *ret, struct pt_regs *args);

/**
 * sm_call_read - retrieve data from secure-monitor
 *
 * @dev:	Pointer to UCLASS_MESON_SM device
 * @buffer:	Buffer to store the retrieved data
 * @size:	Size of the buffer
 * @cmd:	Index of the SMC function ID
 * @args:	SMC arguments
 *
 * @return:	size of read data on success, a negative value on error
 */
int sm_call_read(struct udevice *dev, void *buffer, size_t size,
		 u32 cmd, struct pt_regs *args);

/**
 * sm_call_write - send data to secure-monitor
 *
 * @dev:	Pointer to UCLASS_SM device
 * @buffer:	Buffer containing data to send
 * @size:	Size of the buffer
 * @cmd:	Index of the SMC function ID
 * @args:	SMC arguments
 *
 * @return:	size of sent data on success, a negative value on error
 */
int sm_call_write(struct udevice *dev, void *buffer, size_t size,
		  u32 cmd, struct pt_regs *args);

#endif /* __SM_H__ */
