#if defined(CONFIG_BCM_KF_OPTEE_414_BACKPORTS)
/*
 * Copyright (c) 2015, Linaro Limited
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef OPTEE_PRIVATE_H
#define OPTEE_PRIVATE_H

#include <linux/arm-smccc.h>
#include <linux/semaphore.h>
#include <linux/tee_drv.h>
#include <linux/types.h>
#include "optee_msg.h"

#define OPTEE_MAX_ARG_SIZE	1024

/* Some Global Platform error codes used in this driver */
#define TEEC_SUCCESS			0x00000000
#define TEEC_ERROR_BAD_PARAMETERS	0xFFFF0006
#define TEEC_ERROR_COMMUNICATION	0xFFFF000E
#define TEEC_ERROR_OUT_OF_MEMORY	0xFFFF000C

#define TEEC_ORIGIN_COMMS		0x00000002

typedef void (optee_invoke_fn)(unsigned long, unsigned long, unsigned long,
				unsigned long, unsigned long, unsigned long,
				unsigned long, unsigned long,
				struct arm_smccc_res *);

struct optee_call_queue {
	/* Serializes access to this struct */
	struct mutex mutex;
	struct list_head waiters;
};

struct optee_wait_queue {
	/* Serializes access to this struct */
	struct mutex mu;
	struct list_head db;
};

/**
 * struct optee_supp - supplicant synchronization struct
 * @ctx			the context of current connected supplicant.
 *			if !NULL the supplicant device is available for use,
 *			else busy
 * @ctx_mutex:		held while accessing @ctx
 * @func:		supplicant function id to call
 * @ret:		call return value
 * @num_params:		number of elements in @param
 * @param:		parameters for @func
 * @req_posted:		if true, a request has been posted to the supplicant
 * @supp_next_send:	if true, next step is for supplicant to send response
 * @thrd_mutex:		held by the thread doing a request to supplicant
 * @supp_mutex:		held by supplicant while operating on this struct
 * @data_to_supp:	supplicant is waiting on this for next request
 * @data_from_supp:	requesting thread is waiting on this to get the result
 */
struct optee_supp {
	struct tee_context *ctx;
	/* Serializes access of ctx */
	struct mutex ctx_mutex;

	u32 func;
	u32 ret;
	size_t num_params;
	struct tee_param *param;

	bool req_posted;
	bool supp_next_send;
	/* Serializes access to this struct for requesting thread */
	struct mutex thrd_mutex;
	/* Serializes access to this struct for supplicant threads */
	struct mutex supp_mutex;
	struct completion data_to_supp;
	struct completion data_from_supp;
};

/**
 * struct optee - main service struct
 * @supp_teedev:	supplicant device
 * @teedev:		client device
 * @invoke_fn:		function to issue smc or hvc
 * @call_queue:		queue of threads waiting to call @invoke_fn
 * @wait_queue:		queue of threads from secure world waiting for a
 *			secure world sync object
 * @supp:		supplicant synchronization struct for RPC to supplicant
 * @pool:		shared memory pool
 * @memremaped_shm	virtual address of memory in shared memory pool
 */
struct optee {
	struct tee_device *supp_teedev;
	struct tee_device *teedev;
	optee_invoke_fn *invoke_fn;
	struct optee_call_queue call_queue;
	struct optee_wait_queue wait_queue;
	struct optee_supp supp;
	struct tee_shm_pool *pool;
	void *memremaped_shm;
};

struct optee_session {
	struct list_head list_node;
	u32 session_id;
#if defined(CONFIG_BCM_KF_OPTEE)
	u32 session_pid;
#endif
};

struct optee_context_data {
	/* Serializes access to this struct */
	struct mutex mutex;
	struct list_head sess_list;
};

struct optee_rpc_param {
	u32	a0;
	u32	a1;
	u32	a2;
	u32	a3;
	u32	a4;
	u32	a5;
	u32	a6;
	u32	a7;
};

void optee_handle_rpc(struct tee_context *ctx, struct optee_rpc_param *param);

void optee_wait_queue_init(struct optee_wait_queue *wq);
void optee_wait_queue_exit(struct optee_wait_queue *wq);

u32 optee_supp_thrd_req(struct tee_context *ctx, u32 func, size_t num_params,
			struct tee_param *param);

int optee_supp_read(struct tee_context *ctx, void __user *buf, size_t len);
int optee_supp_write(struct tee_context *ctx, void __user *buf, size_t len);
void optee_supp_init(struct optee_supp *supp);
void optee_supp_uninit(struct optee_supp *supp);

int optee_supp_recv(struct tee_context *ctx, u32 *func, u32 *num_params,
		    struct tee_param *param);
int optee_supp_send(struct tee_context *ctx, u32 ret, u32 num_params,
		    struct tee_param *param);

u32 optee_do_call_with_arg(struct tee_context *ctx, phys_addr_t parg);
int optee_open_session(struct tee_context *ctx,
		       struct tee_ioctl_open_session_arg *arg,
		       struct tee_param *param);
int optee_close_session(struct tee_context *ctx, u32 session);
int optee_invoke_func(struct tee_context *ctx, struct tee_ioctl_invoke_arg *arg,
		      struct tee_param *param);
int optee_cancel_req(struct tee_context *ctx, u32 cancel_id, u32 session);

void optee_enable_shm_cache(struct optee *optee);
void optee_disable_shm_cache(struct optee *optee);

int optee_from_msg_param(struct tee_param *params, size_t num_params,
			 const struct optee_msg_param *msg_params);
int optee_to_msg_param(struct optee_msg_param *msg_params, size_t num_params,
		       const struct tee_param *params);

/*
 * Small helpers
 */

static inline void *reg_pair_to_ptr(u32 reg0, u32 reg1)
{
	return (void *)(unsigned long)(((u64)reg0 << 32) | reg1);
}

static inline void reg_pair_from_64(u32 *reg0, u32 *reg1, u64 val)
{
	*reg0 = val >> 32;
	*reg1 = val;
}

#endif /*OPTEE_PRIVATE_H*/
#endif /* CONFIG_BCM_KF_OPTEE_414_BACKPORTS */
