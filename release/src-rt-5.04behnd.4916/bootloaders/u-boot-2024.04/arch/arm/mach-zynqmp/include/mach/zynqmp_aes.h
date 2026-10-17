/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023 Weidmueller Interface GmbH & Co. KG <oss@weidmueller.com>
 * Christian Taedcke <christian.taedcke@weidmueller.com>
 *
 * Declaration of AES operation functionality for ZynqMP.
 */

#ifndef ZYNQMP_AES_H
#define ZYNQMP_AES_H

struct zynqmp_aes {
	u64 srcaddr;
	u64 ivaddr;
	u64 keyaddr;
	u64 dstaddr;
	u64 len;
	u64 op;
	u64 keysrc;
};

/**
 * zynqmp_aes_operation() - Performs an aes operation using the pmu firmware
 *
 * @aes: The aes operation buffer that must have been allocated using
 *       ALLOC_CACHE_ALIGN_BUFFER(struct zynqmp_aes, aes, 1)
 *
 * Return: 0 in case of success, in case of an error any other value
 */
int zynqmp_aes_operation(struct zynqmp_aes *aes);

#endif /* ZYNQMP_AES_H */
