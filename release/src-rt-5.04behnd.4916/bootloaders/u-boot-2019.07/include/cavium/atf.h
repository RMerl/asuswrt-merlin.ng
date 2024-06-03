/* SPDX-License-Identifier: GPL-2.0+ */
/**
 * (C) Copyright 2014, Cavium Inc.
**/
#ifndef __ATF_H__
#define __ATF_H__
#include <cavium/atf_part.h>

ssize_t atf_read_mmc(uintptr_t offset, void *buffer, size_t size);
ssize_t atf_read_nor(uintptr_t offset, void *buffer, size_t size);
ssize_t atf_get_pcount(void);
ssize_t atf_get_part(struct storage_partition *part, unsigned int index);
ssize_t atf_erase_nor(uintptr_t offset, size_t size);
ssize_t atf_write_nor(uintptr_t offset, const void *buffer, size_t size);
ssize_t atf_write_mmc(uintptr_t offset, const void *buffer, size_t size);
ssize_t atf_dram_size(unsigned int node);
ssize_t atf_node_count(void);
ssize_t atf_env_count(void);
ssize_t atf_env_string(size_t index, char *str);

#endif
