/*
 * Copyright (C) 2013 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
