// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2018 Xilinx, Inc.
 * Siva Durga Prasad Paladugu <siva.durga.paladugu@xilinx.com>
 *
 * Copyright (C) 2023 Weidmueller Interface GmbH & Co. KG <oss@weidmueller.com>
 * Christian Taedcke <christian.taedcke@weidmueller.com>
 */

#include <common.h>
#include <mach/zynqmp_aes.h>

#include <asm/arch/sys_proto.h>
#include <cpu_func.h>
#include <memalign.h>
#include <zynqmp_firmware.h>

int zynqmp_aes_operation(struct zynqmp_aes *aes)
{
	u32 ret_payload[PAYLOAD_ARG_CNT];
	int ret;

	if (zynqmp_firmware_version() <= PMUFW_V1_0)
		return -ENOENT;

	if (aes->srcaddr && aes->ivaddr && aes->dstaddr) {
		flush_dcache_range(aes->srcaddr,
				   aes->srcaddr +
				   roundup(aes->len, ARCH_DMA_MINALIGN));
		flush_dcache_range(aes->ivaddr,
				   aes->ivaddr +
				   roundup(IV_SIZE, ARCH_DMA_MINALIGN));
		flush_dcache_range(aes->dstaddr,
				   aes->dstaddr +
				   roundup(aes->len, ARCH_DMA_MINALIGN));
	}

	if (aes->keysrc == 0) {
		if (aes->keyaddr == 0)
			return -EINVAL;

		flush_dcache_range(aes->keyaddr,
				   aes->keyaddr +
				   roundup(KEY_PTR_LEN, ARCH_DMA_MINALIGN));
	}

	flush_dcache_range((ulong)aes, (ulong)(aes) +
			   roundup(sizeof(struct zynqmp_aes), ARCH_DMA_MINALIGN));

	ret = xilinx_pm_request(PM_SECURE_AES, upper_32_bits((ulong)aes),
				lower_32_bits((ulong)aes), 0, 0, ret_payload);
	if (ret || ret_payload[1]) {
		printf("Failed: AES op status:0x%x, errcode:0x%x\n",
		       ret, ret_payload[1]);
		return -EIO;
	}

	return 0;
}
