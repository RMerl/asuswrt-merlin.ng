/*
 * sdh_ndis.c: SDIO bus driver abstraction layer using
 * Windows SD bus driver interface to provides the bcmsdh API
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
 * $Id: sdh_ndis.c 708017 2017-06-29 14:11:45Z $
 */

#ifdef BCMSPI
#error "SPI defined"
#endif /* BCMSPI */

#include <typedefs.h>
#include <bcmdevs.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <hndsoc.h>

#include <osl.h>
#include <initguid.h>
#include <ntddsd.h>

#include <siutils.h>
#include <bcmsrom.h>

#include <sdio.h>
#include <sbsdio.h>
#include <bcmsdh.h>

#if (NDISVER < 0x0620)
#error "NDIS version not supported"
#endif /* (NDISVER < 0x0620) */

/* SDIO Drive Strength (in milliamps) */
uint dhd_sdiod_drive_strength = 6;
void *def_sdos_info = NULL;
static int regfail = BCME_OK;
extern uint8 dbus_sdos_cfg_read(void *sdos_info, uint func_num, uint32 addr, int *err);
extern void dbus_sdos_cfg_write(void *sdos_info, uint func_num, uint32 addr, uint8 data, int *err);
extern int dbus_sdos_cis_read(void *sdos_info, uint func_num, uint8 *cis, uint32 length);
extern uint32 dbus_sdos_reg_write(void *sdos_info, uint32 addr, uint size, uint32 data);
extern uint32 dbus_sdos_reg_read(void *sdos_info, uint32 addr, uint size, uint32 *data);

uint
bcmsdh_query_iofnum(void *sdh)
{
	UNREFERENCED_PARAMETER(sdh);

	return 1;
}

int
bcmsdh_iovar_op(void *sdh, const char *name,
	void *params, int plen, void *arg, int len, bool set)
{
	return 0;

}

bool
bcmsdh_regfail(void *sdh)
{
	return (regfail != BCME_OK);
}

uint32
bcmsdh_get_dstatus(void *sdh)
{
	return 0;
}

uint8
bcmsdh_cfg_read(void *sdos_info, uint func_num, uint32 addr, int *err)
{
	return dbus_sdos_cfg_read(sdos_info, func_num, addr, err);
}

void
bcmsdh_cfg_write(void *sdos_info, uint func_num, uint32 addr, uint8 data, int *err)
{
	dbus_sdos_cfg_write(sdos_info, func_num, addr, data, err);
}

int
bcmsdh_cis_read(void *sdos_info, uint func_num, uint8 *cis, uint32 length)
{
	if (sdos_info == NULL)
		sdos_info = def_sdos_info;

	return dbus_sdos_cis_read(sdos_info, func_num, cis, length);
}

int
bcmsdh_intr_enable(void *sdos_info)
{
	return 0;
}

uint32
bcmsdh_reg_read(void *sdos_info, uint32 addr, uint length)
{
	uint32 data;

	if (sdos_info == NULL)
		sdos_info = def_sdos_info;

	regfail =  dbus_sdos_reg_read(sdos_info, addr, length, &data);

	return data;
}

uint32
bcmsdh_reg_write(void *sdos_info, uint32 addr, uint length, uint32 data)
{
	if (sdos_info == NULL)
		sdos_info = def_sdos_info;

	return dbus_sdos_reg_write(sdos_info, addr, length, data);
}
