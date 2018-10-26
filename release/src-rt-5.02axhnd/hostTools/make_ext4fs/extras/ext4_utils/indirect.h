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

#ifndef _INDIRECT_H_
#define _INDIRECT_H_

#include "allocate.h"

void inode_allocate_indirect(struct ext4_inode *inode, unsigned long len);
u8 *inode_allocate_data_indirect(struct ext4_inode *inode, unsigned long len,
	unsigned long backing_len);
void inode_attach_resize(struct ext4_inode *inode,
		struct block_allocation *alloc);
void free_indirect_blocks();

#endif
