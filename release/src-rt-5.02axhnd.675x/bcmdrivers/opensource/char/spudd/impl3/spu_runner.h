/*
<:copyright-BRCM:2015:GPL/GPL:spu

   Copyright (c) 2015 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/
#ifndef __SPU_RUNNER_H__
#define __SPU_RUNNER_H__

#include <rdpa_api.h>
#include <rdpa_ipsec.h>
#include <rdpa_ag_ipsec.h>

struct spu_desc
{
    rdpa_sa_desc_t     sa_descr;
    struct list_head   entry;
    uint32_t           index;
    uint32_t           reserved;
};

__init int  spu_runner_init( void );
__exit void spu_runner_deinit( void );

int spu_runner_descr_key_validate(int enckeylen, int authkeylen);
int spu_runner_descr_config(struct spu_ctx *pCtx);
struct spu_desc *spu_runner_descr_get(void);
void spu_runner_descr_put(struct spu_desc *pDescr);
int spu_runner_process_ipsec(struct spu_trans_req *pReq, pNBuff_t pNBuf, uint32_t offset);

#endif /* __SPU_RUNNER_H__ */