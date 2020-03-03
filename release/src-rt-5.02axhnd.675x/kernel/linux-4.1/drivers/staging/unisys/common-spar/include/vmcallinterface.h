/* Copyright (C) 2010 - 2013 UNISYS CORPORATION
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, GOOD TITLE or
 * NON INFRINGEMENT.  See the GNU General Public License for more
 * details.
 */

#ifndef __IOMONINTF_H__
#define __IOMONINTF_H__

/*
* This file contains all structures needed to support the VMCALLs for IO
* Virtualization.  The VMCALLs are provided by Monitor and used by IO code
* running on IO Partitions.
*/

#ifdef __GNUC__
#include "iovmcall_gnuc.h"
#endif	/*  */
#include "diagchannel.h"

#ifdef VMCALL_IO_CONTROLVM_ADDR
#undef VMCALL_IO_CONTROLVM_ADDR
#endif	/*  */

/* define subsystem number for AppOS, used in uislib driver  */
#define MDS_APPOS 0x4000000000000000L	/* subsystem = 62 - AppOS */
enum vmcall_monitor_interface_method_tuple { /* VMCALL identification tuples  */
	    /* Note: when a new VMCALL is added:
	     * - the 1st 2 hex digits correspond to one of the
	     *   VMCALL_MONITOR_INTERFACE types and
	     * - the next 2 hex digits are the nth relative instance of within a
	     *   type
	     * E.G. for VMCALL_VIRTPART_RECYCLE_PART,
	     * - the 0x02 identifies it as a VMCALL_VIRTPART type and
	     * - the 0x01 identifies it as the 1st instance of a VMCALL_VIRTPART
	     *   type of VMCALL
	     */

	VMCALL_IO_CONTROLVM_ADDR = 0x0501,	/* used by all Guests, not just
						 * IO */
	VMCALL_IO_DIAG_ADDR = 0x0508,
	VMCALL_IO_VISORSERIAL_ADDR = 0x0509,
	VMCALL_QUERY_GUEST_VIRTUAL_TIME_OFFSET = 0x0708, /* Allow caller to
							  * query virtual time
							  * offset */
	VMCALL_CHANNEL_VERSION_MISMATCH = 0x0709,
	VMCALL_POST_CODE_LOGEVENT = 0x070B,	/* LOGEVENT Post Code (RDX) with
						 * specified subsystem mask (RCX
						 * - monitor_subsystems.h) and
						 * severity (RDX) */
	VMCALL_GENERIC_SURRENDER_QUANTUM_FOREVER = 0x0802, /* Yield the
							    * remainder & all
							    * future quantums of
							    * the caller */
	VMCALL_MEASUREMENT_DO_NOTHING = 0x0901,
	VMCALL_UPDATE_PHYSICAL_TIME = 0x0a02	/* Allow
						 * ULTRA_SERVICE_CAPABILITY_TIME
						 * capable guest to make
						 * VMCALL */
};

#define VMCALL_SUCCESS 0
#define VMCALL_SUCCESSFUL(result)	(result == 0)

#ifdef __GNUC__
#define unisys_vmcall(tuple, reg_ebx, reg_ecx) \
	__unisys_vmcall_gnuc(tuple, reg_ebx, reg_ecx)
#define unisys_extended_vmcall(tuple, reg_ebx, reg_ecx, reg_edx) \
	__unisys_extended_vmcall_gnuc(tuple, reg_ebx, reg_ecx, reg_edx)
#define ISSUE_IO_VMCALL(method, param, result) \
	(result = unisys_vmcall(method, (param) & 0xFFFFFFFF,	\
				(param) >> 32))
#define ISSUE_IO_EXTENDED_VMCALL(method, param1, param2, param3) \
	unisys_extended_vmcall(method, param1, param2, param3)

    /* The following uses VMCALL_POST_CODE_LOGEVENT interface but is currently
     * not used much */
#define ISSUE_IO_VMCALL_POSTCODE_SEVERITY(postcode, severity)		\
do {									\
	ISSUE_IO_EXTENDED_VMCALL(VMCALL_POST_CODE_LOGEVENT, severity,	\
				 MDS_APPOS, postcode);			\
} while (0)
#endif

/* Structures for IO VMCALLs */

/* ///////////// BEGIN PRAGMA PACK PUSH 1 ///////////////////////// */
/* ///////////// ONLY STRUCT TYPE SHOULD BE BELOW */
#pragma pack(push, 1)
struct phys_info {
	u64 pi_pfn;
	u16 pi_off;
	u16 pi_len;
};

#pragma pack(pop)
/* ///////////// END PRAGMA PACK PUSH 1 /////////////////////////// */

/* ///////////// BEGIN PRAGMA PACK PUSH 1 ///////////////////////// */
/* ///////////// ONLY STRUCT TYPE SHOULD BE BELOW */
#pragma pack(push, 1)
/* Parameters to VMCALL_IO_CONTROLVM_ADDR interface */
struct vmcall_io_controlvm_addr_params {
	    /* The Guest-relative physical address of the ControlVm channel.
	    * This VMCall fills this in with the appropriate address. */
	u64 address;	/* contents provided by this VMCALL (OUT) */
	    /* the size of the ControlVm channel in bytes This VMCall fills this
	    * in with the appropriate address. */
	u32 channel_bytes;	/* contents provided by this VMCALL (OUT) */
	u8 unused[4];		/* Unused Bytes in the 64-Bit Aligned Struct */
};

#pragma pack(pop)
/* ///////////// END PRAGMA PACK PUSH 1 /////////////////////////// */

/* ///////////// BEGIN PRAGMA PACK PUSH 1 ///////////////////////// */
/* ///////////// ONLY STRUCT TYPE SHOULD BE BELOW */
#pragma pack(push, 1)
/* Parameters to VMCALL_IO_DIAG_ADDR interface */
struct vmcall_io_diag_addr_params {
	    /* The Guest-relative physical address of the diagnostic channel.
	    * This VMCall fills this in with the appropriate address. */
	u64 address;	/* contents provided by this VMCALL (OUT) */
};

#pragma pack(pop)
/* ///////////// END PRAGMA PACK PUSH 1 /////////////////////////// */

/* ///////////// BEGIN PRAGMA PACK PUSH 1 ///////////////////////// */
/* ///////////// ONLY STRUCT TYPE SHOULD BE BELOW */
#pragma pack(push, 1)
/* Parameters to VMCALL_IO_VISORSERIAL_ADDR interface */
struct vmcall_io_visorserial_addr_params {
	    /* The Guest-relative physical address of the serial console
	    * channel.  This VMCall fills this in with the appropriate
	    * address. */
	u64 address;	/* contents provided by this VMCALL (OUT) */
};

#pragma pack(pop)
/* ///////////// END PRAGMA PACK PUSH 1 /////////////////////////// */

/* Parameters to VMCALL_CHANNEL_MISMATCH interface */
struct vmcall_channel_version_mismatch_params {
	u8 chname[32];	/* Null terminated string giving name of channel
				 * (IN) */
	u8 item_name[32];	/* Null terminated string giving name of
				 * mismatched item (IN) */
	u32 line_no;		/* line# where invoked. (IN) */
	u8 file_name[36];	/* source code where invoked - Null terminated
				 * string (IN) */
};

#endif /* __IOMONINTF_H__ */
