/*
 * NDIS-specific portion of
 * Broadcom 802.11abg Networking Device Driver
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: bcmwlsdioif.h,v 1.1.2.1 2011-01-25 23:24:48 $
 */

#ifndef BCMWLSDIOIF_H
#define BCMWLSDIOIF_H

#include <wdf.h>
#include <WdfMiniport.h>
#include <initguid.h>
#include <ntddsd.h>

/*
 * Define an Interface Guid to access the proprietary sdio interface.
 * This guid is used to identify a specific interface in IRP_MN_QUERY_INTERFACE
 * handler.
 */
/* {5C279E39-D180-4e39-A1D4-65FB02B92472} */
DEFINE_GUID(GUID_BCMWLSDIO_INTERFACE_STANDARD,
0x5c279e39, 0xd180, 0x4e39, 0xa1, 0xd4, 0x65, 0xfb, 0x2, 0xb9, 0x24, 0x72);

#define VERSION_BCMWLSDIO_INTERFACE_STANDARD	1

/*
 * Define Interface reference/dereference routines for
 *  Interfaces exported by IRP_MN_QUERY_INTERFACE
 */

typedef VOID (*PINTERFACE_REFERENCE)(PVOID Context);
typedef VOID (*PINTERFACE_DEREFERENCE)(PVOID Context);

typedef
NTSTATUS
(*PSDBUSOPENINTERFACE)(
	PDEVICE_OBJECT  Pdo,
	PSDBUS_INTERFACE_STANDARD InterfaceStandard,
	USHORT Size,
	USHORT Version);

typedef
NTSTATUS
(*PSDBUSSUBMITREQUEST)(
	PVOID InterfaceContext,
	PSDBUS_REQUEST_PACKET Packet);

typedef
NTSTATUS
(*PSDBUSSUBMITREQUESTASYNC)(
	PVOID InterfaceContext,
	PSDBUS_REQUEST_PACKET Packet,
	PIRP Irp,
	PIO_COMPLETION_ROUTINE CompletionRoutine,
	PVOID UserContext);

/*
 * Interface for getting and setting power level etc.,
 */
typedef struct _BCMWLSDIO_INTERFACE_STANDARD {
    INTERFACE                   InterfaceHeader;
    PDEVICE_OBJECT		Pdo;
    PDEVICE_OBJECT		TargetObject;
    PSDBUSOPENINTERFACE		SdBusOpenInterface;
    PSDBUSSUBMITREQUEST		SdBusSubmitRequest;
    PSDBUSSUBMITREQUESTASYNC    SdBusSubmitRequestAsync;
} BCMWLSDIO_INTERFACE_STANDARD, *PBCMWLSDIO_INTERFACE_STANDARD;

#endif /* BCMWLSDIOIF_H */
