/*
 * Implementation of wlc_key algo 'none'
 * Copyright (c) 2012-2013 Broadcom Corporation. All rights reserved.
 * $Id: km_key_none.c 431766 2013-10-24 16:26:34Z $
 */

#include "km_key_pvt.h"

/* internal interface */
static const key_algo_callbacks_t key_none_callbacks = {
	NULL,	/* destroy */
	NULL,   /* get data */
	NULL,	/* set data */
	NULL,	/* rx mpdu */
	NULL,	/* rx msdu */
	NULL,	/* tx mpdu */
	NULL,	/* tx msdu */
	NULL	/* dump */
};

/* public interface */

int
km_key_none_init(wlc_key_t *key)
{
	KM_DBG_ASSERT(key->info.algo == CRYPTO_ALGO_NONE);

	key->info.key_len = 0;
	key->info.iv_len = 0;
	key->info.icv_len = 0;

	key->algo_impl.cb = &key_none_callbacks;
	key->algo_impl.ctx = NULL;

	return BCME_OK;
}
