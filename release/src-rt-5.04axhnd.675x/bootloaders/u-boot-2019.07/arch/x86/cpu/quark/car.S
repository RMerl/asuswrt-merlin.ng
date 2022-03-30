/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 */

#include <config.h>
#include <asm/pci.h>
#include <asm/post.h>
#include <asm/arch/quark.h>
#include <asm/arch/msg_port.h>

.globl car_init
car_init:
	post_code(POST_CAR_START)

	/*
	 * Quark SoC contains an embedded 512KiB SRAM (eSRAM) that is
	 * initialized by hardware. eSRAM is the ideal place to be used
	 * for Cache-As-RAM (CAR) before system memory is available.
	 *
	 * Relocate this eSRAM to a suitable location in the physical
	 * memory map and enable it.
	 */

	/* Host Memory Bound Register P03h:R08h */
	mov	$((MSG_PORT_HOST_BRIDGE << 16) | (HM_BOUND << 8)), %eax
	mov	$(DRAM_BASE + DRAM_MAX_SIZE + ESRAM_SIZE), %edx
	lea	1f, %esp
	jmp	msg_port_write
1:

	/* eSRAM Block Page Control Register P05h:R82h */
	mov	$((MSG_PORT_MEM_MGR << 16) | (ESRAM_BLK_CTRL << 8)), %eax
	mov	$(ESRAM_BLOCK_MODE | (CONFIG_ESRAM_BASE >> 24)), %edx
	lea	2f, %esp
	jmp	msg_port_write
2:

	post_code(POST_CAR_CPU_CACHE)
	jmp	car_init_ret

msg_port_read:
	/*
	 * Parameter:
	 *   eax[23:16] - Message Port ID
	 *   eax[15:08] - Register Address
	 *
	 * Return Value:
	 *   eax - Message Port Register value
	 *
	 * Return Address: esp
	 */

	or	$((MSG_OP_READ << 24) | MSG_BYTE_ENABLE), %eax
	mov	%eax, %ebx

	/* Write MCR B0:D0:F0:RD0 */
	mov	$(PCI_CFG_EN | MSG_CTRL_REG), %eax
	mov	$PCI_REG_ADDR, %dx
	out	%eax, %dx
	mov	$PCI_REG_DATA, %dx
	mov	%ebx, %eax
	out	%eax, %dx

	/* Read MDR B0:D0:F0:RD4 */
	mov	$(PCI_CFG_EN | MSG_DATA_REG), %eax
	mov	$PCI_REG_ADDR, %dx
	out	%eax, %dx
	mov	$PCI_REG_DATA, %dx
	in	%dx, %eax

	jmp	*%esp

msg_port_write:
	/*
	 * Parameter:
	 *   eax[23:16] - Message Port ID
	 *   eax[15:08] - Register Address
	 *   edx        - Message Port Register value to write
	 *
	 * Return Address: esp
	 */

	or	$((MSG_OP_WRITE << 24) | MSG_BYTE_ENABLE), %eax
	mov	%eax, %esi
	mov	%edx, %edi

	/* Write MDR B0:D0:F0:RD4 */
	mov	$(PCI_CFG_EN | MSG_DATA_REG), %eax
	mov	$PCI_REG_ADDR, %dx
	out	%eax, %dx
	mov	$PCI_REG_DATA, %dx
	mov	%edi, %eax
	out	%eax, %dx

	/* Write MCR B0:D0:F0:RD0 */
	mov	$(PCI_CFG_EN | MSG_CTRL_REG), %eax
	mov	$PCI_REG_ADDR, %dx
	out	%eax, %dx
	mov	$PCI_REG_DATA, %dx
	mov	%esi, %eax
	out	%eax, %dx

	jmp	*%esp
