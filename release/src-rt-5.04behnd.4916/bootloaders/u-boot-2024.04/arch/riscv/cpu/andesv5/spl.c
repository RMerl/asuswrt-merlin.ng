// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023 Andes Technology Corporation
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 */
#include <cpu_func.h>
#include <hang.h>
#include <init.h>
#include <log.h>
#include <spl.h>
#include <asm/global_data.h>
#include <asm/system.h>

DECLARE_GLOBAL_DATA_PTR;

#if CONFIG_IS_ENABLED(RAM_SUPPORT)
struct legacy_img_hdr *spl_get_load_buffer(ssize_t offset, size_t size)
{
	return (void *)(CONFIG_SPL_LOAD_FIT_ADDRESS + offset);
}

void *board_spl_fit_buffer_addr(ulong fit_size, int sectors, int bl_len)
{
	return spl_get_load_buffer(0, sectors * bl_len);
}
#endif
