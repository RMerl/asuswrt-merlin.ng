// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2024 Broadcom Corporation
   All Rights Reserved
*/

#ifndef _MEMORY_ACCESS_H_
#define _MEMORY_ACCESS_H_

#define WRITE_32(a, r)  (*(volatile uint32_t*)(a) = *(uint32_t*)&(r))
#define READ_32(a, r)   (*(volatile uint32_t*)&(r) = *(volatile uint32_t*)(a))
#define WRITE_64(a, r)  (*(volatile uint64_t*)(a) = *(uint64_t*)&(r))
#define READ_64(a, r)   (*(volatile uint64_t*)&(r) = *(volatile uint64_t*)(a))

#define VAL32(_a)       ( *(volatile uint32_t*)(_a))
#define VAL64(_a)       ( *(volatile uint64_t*)(_a))

#endif
