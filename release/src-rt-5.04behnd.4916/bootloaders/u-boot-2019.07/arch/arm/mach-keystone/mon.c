// SPDX-License-Identifier: GPL-2.0+
/*
 * K2HK: secure kernel command file
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#include <common.h>
#include <command.h>
#include <mach/mon.h>
#include <spl.h>
asm(".arch_extension sec\n\t");

int mon_install(u32 addr, u32 dpsc, u32 freq, u32 bm_addr)
{
	int result;

	__asm__ __volatile__ (
		"stmfd r13!, {lr}\n"
		"mov r0, %1\n"
		"mov r1, %2\n"
		"mov r2, %3\n"
		"mov r3, %4\n"
		"blx r0\n"
		"mov %0, r0\n"
		"ldmfd r13!, {lr}\n"
		: "=&r" (result)
		: "r" (addr), "r" (dpsc), "r" (freq), "r" (bm_addr)
		: "cc", "r0", "r1", "r2", "r3", "memory");
	return result;
}

int mon_power_on(int core_id, void *ep)
{
	int result;

	asm volatile (
		"stmfd  r13!, {lr}\n"
		"mov r1, %1\n"
		"mov r2, %2\n"
		"mov r0, #0\n"
		"smc	#0\n"
		"mov %0, r0\n"
		"ldmfd  r13!, {lr}\n"
		: "=&r" (result)
		: "r" (core_id), "r" (ep)
		: "cc", "r0", "r1", "r2", "memory");
	return  result;
}

int mon_power_off(int core_id)
{
	int result;

	asm volatile (
		"stmfd  r13!, {lr}\n"
		"mov r1, %1\n"
		"mov r0, #1\n"
		"smc	#1\n"
		"mov %0, r0\n"
		"ldmfd  r13!, {lr}\n"
		: "=&r" (result)
		: "r" (core_id)
		: "cc", "r0", "r1", "memory");
	return  result;
}

#ifdef CONFIG_TI_SECURE_DEVICE
#define KS2_HS_SEC_HEADER_LEN	0x60
#define KS2_HS_SEC_TAG_OFFSET	0x34
#define KS2_AUTH_CMD		130

/**
 * k2_hs_bm_auth() - Invokes security functions using a
 * proprietary TI interface. This binary and source for
 * this is available in the secure development package or
 * SECDEV. For details on how to access this please refer
 * doc/README.ti-secure
 *
 * @cmd: Secure monitor command
 * @arg1: Argument for command
 *
 * returns non-zero value on success, zero on error
 */
static int k2_hs_bm_auth(int cmd, void *arg1)
{
	int result;

	asm volatile (
		"stmfd  r13!, {r4-r12, lr}\n"
		"mov r0, %1\n"
		"mov r1, %2\n"
		"smc #2\n"
		"mov %0, r0\n"
		"ldmfd r13!, {r4-r12, lr}\n"
		: "=&r" (result)
		: "r" (cmd), "r" (arg1)
		: "cc", "r0", "r1", "memory");

	return  result;
}

void board_fit_image_post_process(void **p_image, size_t *p_size)
{
	int result = 0;
	void *image = *p_image;

	if (strncmp(image + KS2_HS_SEC_TAG_OFFSET, "KEYS", 4)) {
		printf("No signature found in image!\n");
		hang();
	}

	result = k2_hs_bm_auth(KS2_AUTH_CMD, image);
	if (result == 0) {
		printf("Authentication failed!\n");
		hang();
	}

	/*
	 * Overwrite the image headers after authentication
	 * and decryption. Update size to reflect removal
	 * of header.
	 */
	*p_size -= KS2_HS_SEC_HEADER_LEN;
	memcpy(image, image + KS2_HS_SEC_HEADER_LEN, *p_size);

	/*
	 * Output notification of successful authentication to re-assure the
	 * user that the secure code is being processed as expected. However
	 * suppress any such log output in case of building for SPL and booting
	 * via YMODEM. This is done to avoid disturbing the YMODEM serial
	 * protocol transactions.
	 */
	if (!(IS_ENABLED(CONFIG_SPL_BUILD) &&
	      IS_ENABLED(CONFIG_SPL_YMODEM_SUPPORT) &&
	      spl_boot_device() == BOOT_DEVICE_UART))
		printf("Authentication passed\n");
}
#endif
