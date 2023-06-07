/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2013 Intel Corporation
 *
 */

#define LOG_TAG "BlueZ"

#ifdef __BIONIC__
#include <cutils/log.h>
#else
#include <stdio.h>
#define LOG_INFO " I"
#define LOG_WARN " W"
#define LOG_ERROR " E"
#define LOG_DEBUG " D"
#define ALOG(pri, tag, fmt, arg...) fprintf(stderr, tag pri": " fmt"\n", ##arg)
#endif

#define info(fmt, arg...) ALOG(LOG_INFO, LOG_TAG, fmt, ##arg)
#define warn(fmt, arg...) ALOG(LOG_WARN, LOG_TAG, fmt, ##arg)
#define error(fmt, arg...) ALOG(LOG_ERROR, LOG_TAG, fmt, ##arg)
#define DBG(fmt, arg...) ALOG(LOG_DEBUG, LOG_TAG, "%s:%s() "fmt, __FILE__, \
							__func__, ##arg)
