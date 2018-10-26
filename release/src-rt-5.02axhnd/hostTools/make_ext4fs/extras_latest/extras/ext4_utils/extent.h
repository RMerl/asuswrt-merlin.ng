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

#ifndef _EXTENT_H_
#define _EXTENT_H_

#include "allocate.h"
#include "ext4_utils.h"

void inode_allocate_extents(struct ext4_inode *inode, u64 len);
struct block_allocation* inode_allocate_file_extents(
	struct ext4_inode *inode, u64 len, const char *filename);
u8 *inode_allocate_data_extents(struct ext4_inode *inode, u64 len,
	u64 backing_len);
void free_extent_blocks();

#endif
