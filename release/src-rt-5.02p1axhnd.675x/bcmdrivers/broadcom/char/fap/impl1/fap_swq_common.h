#ifndef __FAP_SWQ_COMMON_H_INCLUDED__
#define __FAP_SWQ_COMMON_H_INCLUDED__
/*
<:copyright-BRCM:2014:DUAL/GPL:standard 

   Copyright (c) 2014 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/

#include "fap4ke_swq.h"

/* this structure holds information about swq  which is not changed
*  once swq is initialized, Storing this information in cached memory
*  avoids some uncached memory accesses
*/

typedef struct {
    fap4ke_SWQueue_t *swq;
    uint32  *qStart;
    uint32  *qEnd;
    uint8   msgSize;
    uint8   fapId;
    uint8   dqm;
    uint8   rsvd1;
} SWQInfo_t;

#endif /* __FAP_SWQ_COMMON_H_INCLUDED__ */
