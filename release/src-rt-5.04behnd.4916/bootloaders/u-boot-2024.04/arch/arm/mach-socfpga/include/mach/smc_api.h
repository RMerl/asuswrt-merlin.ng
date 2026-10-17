/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2020 Intel Corporation
 */

#ifndef _SMC_API_H_
#define _SMC_API_H_

int invoke_smc(u32 func_id, u64 *args, int arg_len, u64 *ret_arg, int ret_len);
int smc_send_mailbox(u32 cmd, u32 len, u32 *arg, u8 urgent, u32 *resp_buf_len,
		     u32 *resp_buf);
int smc_get_usercode(u32 *usercode);

#endif /* _SMC_API_H_ */
