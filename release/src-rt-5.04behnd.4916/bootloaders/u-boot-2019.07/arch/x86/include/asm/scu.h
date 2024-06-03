/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2017 Intel Corporation
 */
#ifndef _X86_ASM_SCU_IPC_H_
#define _X86_ASM_SCU_IPC_H_

/* IPC defines the following message types */
#define IPCMSG_INDIRECT_READ	0x02
#define IPCMSG_INDIRECT_WRITE	0x05
#define IPCMSG_WARM_RESET	0xf0
#define IPCMSG_COLD_RESET	0xf1
#define IPCMSG_SOFT_RESET	0xf2
#define IPCMSG_COLD_BOOT	0xf3
#define IPCMSG_GET_FW_REVISION	0xf4
#define IPCMSG_WATCHDOG_TIMER	0xf8	/* Set Kernel Watchdog Threshold */

struct ipc_ifwi_version {
	u16	minor;
	u8	major;
	u8	hardware_id;
	u32	reserved[3];
};

/* Issue commands to the SCU with or without data */
int scu_ipc_simple_command(u32 cmd, u32 sub);
int scu_ipc_command(u32 cmd, u32 sub, u32 *in, int inlen, u32 *out, int outlen);
int scu_ipc_raw_command(u32 cmd, u32 sub, u32 *in, int inlen, u32 *out,
			int outlen, u32 dptr, u32 sptr);

#endif	/* _X86_ASM_SCU_IPC_H_ */
