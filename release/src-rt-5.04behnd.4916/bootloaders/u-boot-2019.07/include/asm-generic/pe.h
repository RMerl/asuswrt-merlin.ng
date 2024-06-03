/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  Portable Executable and Common Object Constants
 *
 *  Copyright (c) 2018 Heinrich Schuchardt
 *
 *  based on the "Microsoft Portable Executable and Common Object File Format
 *  Specification", revision 11, 2017-01-23
 */

#ifndef _ASM_PE_H
#define _ASM_PE_H

/* Characteristics */
#define IMAGE_FILE_RELOCS_STRIPPED		0x0001
#define IMAGE_FILE_EXECUTABLE_IMAGE		0x0002
#define IMAGE_FILE_LINE_NUMS_STRIPPED		0x0004
#define IMAGE_FILE_LOCAL_SYMS_STRIPPED		0x0008
#define IMAGE_FILE_AGGRESSIVE_WS_TRIM		0x0010
#define IMAGE_FILE_LARGE_ADDRESS_AWARE		0x0020
/* Reserved					0x0040 */
#define IMAGE_FILE_BYTES_REVERSED_LO		0x0080
#define IMAGE_FILE_32BIT_MACHINE		0x0100
#define IMAGE_FILE_DEBUG_STRIPPED		0x0200
#define IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP	0x0400
#define IMAGE_FILE_NET_RUN_FROM_SWAP		0x0800
#define IMAGE_FILE_SYSTEM			0x1000
#define IMAGE_FILE_DLL				0x2000
#define IMAGE_FILE_UP_SYSTEM_ONLY		0x4000
#define IMAGE_FILE_BYTES_REVERSED_HI		0x8000

/* Subsystem type */
#define IMAGE_SUBSYSTEM_EFI_APPLICATION		10
#define IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER	11
#define IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER	12
#define IMAGE_SUBSYSTEM_EFI_ROM			13

#endif /* _ASM_PE_H */
