/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 Linaro Limited
 */

#ifndef __SANDBOXTEE_H
#define __SANDBOXTEE_H

#include <search.h>
#include <tee/optee_ta_avb.h>

/**
 * struct sandbox_tee_state - internal state of the sandbox TEE
 * @session:			current open session
 * @num_shms:			number of registered shared memory objects
 * @ta:				Trusted Application of current session
 * @ta_avb_rollback_indexes	TA avb rollback indexes storage
 * @ta_avb_lock_state		TA avb lock state storage
 * @pstorage_htab		named persistent values storage
 */
struct sandbox_tee_state {
	u32 session;
	int num_shms;
	void *ta;
	u64 ta_avb_rollback_indexes[TA_AVB_MAX_ROLLBACK_LOCATIONS];
	u32 ta_avb_lock_state;
	struct hsearch_data pstorage_htab;
};

#endif /*__SANDBOXTEE_H*/
