/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 NXP
 *
 */

#ifndef CRYPTO_FSL_TYPE_H
#define CRYPTO_FSL_TYPE_H

#ifdef CONFIG_CAAM_64BIT
typedef unsigned long long caam_dma_addr_t;
#else
typedef u32 caam_dma_addr_t;
#endif

#endif
