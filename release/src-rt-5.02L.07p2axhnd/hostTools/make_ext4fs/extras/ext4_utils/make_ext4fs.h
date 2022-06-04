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

#ifndef _MAKE_EXT4FS_H_
#define _MAKE_EXT4FS_H_

#ifdef __cplusplus
extern "C" {
#endif

struct selabel_handle;

int make_ext4fs(const char *filename, long long len,
                const char *mountpoint, struct selabel_handle *sehnd);
int make_ext4fs_sparse_fd(int fd, long long len,
                const char *mountpoint, struct selabel_handle *sehnd);

#ifdef __cplusplus
}
#endif

#endif
