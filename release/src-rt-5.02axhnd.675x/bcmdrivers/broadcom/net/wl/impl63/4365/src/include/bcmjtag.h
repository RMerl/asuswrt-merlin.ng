/*
 * JTAG access interface for drivers
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
 * $Id: bcmjtag.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef _bcmjtag_h_
#define _bcmjtag_h_

/*
* Simple high level framework that provides easier driver integration.
*/
/* driver info struct */
typedef struct {
	/* attach to device */
	void *(*attach)(uint16 venid, uint16 devid, void *regsva, void *param);
	/* detach from device */
	void (*detach)(void *ch);
	/* poll device */
	void (*poll)(void *ch);
} bcmjtag_driver_t;

/* platform specific/high level functions */
extern int bcmjtag_register(bcmjtag_driver_t *driver);
extern void bcmjtag_unregister(void);

/*
* More sophisticated low level functions for flexible driver integration.
*/
/* forward declaration */
typedef struct bcmjtag_info bcmjtag_info_t;

/* common/low level functions */
extern bcmjtag_info_t *bcmjtag_attach(osl_t *osh,
                                      uint16 jtmvendorid, uint16 jtmdevid,
                                      void *jtmregs, uint jtmbustype,
                                      void *btparam, bool diffend);
extern int bcmjtag_detach(bcmjtag_info_t *jtih);

extern uint32 bcmjtag_read(bcmjtag_info_t *jtih, uint32 addr, uint size);
extern void bcmjtag_write(bcmjtag_info_t *jtih,
	uint32 addr, uint32 val, uint size);

extern bool bcmjtag_chipmatch(uint16 vendor, uint16 device);

extern int bcmjtag_devattach(bcmjtag_info_t *jtih, uint16 venid, uint16 devid,
	bool (*dcb)(void *arg, uint16 venid, uint16 devid, void *devregs),
	void *arg);

#endif /* _bcmjtag_h_ */
