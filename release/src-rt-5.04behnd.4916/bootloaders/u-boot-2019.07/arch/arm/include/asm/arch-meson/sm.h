/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2016 - Beniamino Galvani <b.galvani@gmail.com>
 */

#ifndef __MESON_SM_H__
#define __MESON_SM_H__

ssize_t meson_sm_read_efuse(uintptr_t offset, void *buffer, size_t size);

#endif /* __MESON_SM_H__ */
