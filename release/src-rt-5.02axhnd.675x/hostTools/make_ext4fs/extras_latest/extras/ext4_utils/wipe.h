/*
 * Copyright (C) 2010 The Android Open Source Project
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
 */

#ifndef _WIPE_H_
#define _WIPE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "ext4_utils.h"

/* Set WIPE_IS_SUPPORTED to 1 if the current platform supports
 * wiping of block devices. 0 otherwise. For now, only Linux does.
 */
#ifdef __linux__
#  define WIPE_IS_SUPPORTED 1
#else
#  define WIPE_IS_SUPPORTED 0
#endif

int wipe_block_device(int fd, s64 len);

#ifdef __cplusplus
}
#endif

#endif
