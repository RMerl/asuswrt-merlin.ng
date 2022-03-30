/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2016
 * Texas Instruments, <www.ti.com>
 *
 * Andreas Dannenberg <dannenberg@ti.com>
 */
#ifndef	_OMAP_SEC_COMMON_H_
#define	_OMAP_SEC_COMMON_H_

#include <common.h>

/*
 * Invoke secure ROM API on high-security (HS) device variants. It formats
 * the variable argument list into the format expected by the ROM code before
 * triggering the actual low-level smc entry.
 */
u32 secure_rom_call(u32 service, u32 proc_id, u32 flag, ...);

/*
 * Invoke a secure ROM API on high-secure (HS) device variants that can be used
 * to verify a secure blob by authenticating and optionally decrypting it. The
 * exact operation performed depends on how the certificate that was embedded
 * into the blob during the signing/encryption step when the secure blob was
 * first created.
 */
int secure_boot_verify_image(void **p_image, size_t *p_size);

/*
 * Return the start of secure reserved RAM, if a default start address has
 * not been configured then return a region at the end of the external DRAM.
 */
u32 get_sec_mem_start(void);

/*
 * Invoke a secure HAL API that allows configuration of the external memory
 * firewall regions.
 */
int secure_emif_firewall_setup(uint8_t region_num, uint32_t start_addr,
			       uint32_t size, uint32_t access_perm,
			       uint32_t initiator_perm);

/*
 * Invoke a secure HAL API on high-secure (HS) device variants that reserves a
 * region of external memory for secure world use, and protects it using memory
 * firewalls that prevent public world access. This API is intended to setaside
 * memory that will be used for a secure world OS/TEE.
 */
int secure_emif_reserve(void);

/*
 * Invoke a secure HAL API to lock the external memory firewall configurations.
 * After this API is called, none of the HAL APIs for configuring the that
 * firewall will be usable (calls to those APIs will return failure and have
 * no effect).
 */
int secure_emif_firewall_lock(void);

/*
 * Invoke a secure HAL API to authenticate and install a Trusted Execution
 * Environment (TEE) image.
 */
int secure_tee_install(u32 tee_image);

#endif /* _OMAP_SEC_COMMON_H_ */
