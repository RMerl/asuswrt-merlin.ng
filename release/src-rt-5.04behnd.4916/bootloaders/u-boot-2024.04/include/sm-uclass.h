/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2023 SberDevices, Inc.
 *
 * Author: Alexey Romanov <avromanov@salutedevices.com>
 */

#ifndef __SM_UCLASS_H__
#define __SM_UCLASS_H__

#include <asm/types.h>
#include <asm/ptrace.h>

struct udevice;

/**
 * struct sm_ops - The functions that a SM driver must implement.
 *
 * @sm_call: Request a secure monitor call with specified command.
 *
 * @sm_call_read: Request a secure monitor call and retrieve data
 * from secure-monitor (depends on specified command).
 *
 * @sm_call_write: Request a secure monitor call and send data
 * to secure-monitor (depends on specified command).
 *
 * The individual methods are described more fully below.
 */
struct sm_ops {
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
	int (*sm_call)(struct udevice *dev, u32 cmd, s32 *smc_ret,
		       struct pt_regs *args);

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
	int (*sm_call_write)(struct udevice *dev, void *buffer,
			     size_t size, u32 cmd, struct pt_regs *args);

	/**
	 * sm_call_read - retrieve data from secure-monitor
	 *
	 * @dev:	Pointer to UCLASS_SM device
	 * @buffer:	Buffer to store the retrieved data
	 * @size:	Size of the buffer
	 * @cmd:	Index of the SMC function ID
	 * @args:	SMC arguments
	 *
	 * @return:	size of read data on success, a negative value on error
	 */
	int (*sm_call_read)(struct udevice *dev, void *buffer,
			    size_t size, u32 cmd, struct pt_regs *args);
};

#endif /* __SM_UCLASS_H__ */
