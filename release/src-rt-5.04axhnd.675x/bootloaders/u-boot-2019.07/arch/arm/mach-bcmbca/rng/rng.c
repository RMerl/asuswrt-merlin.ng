// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Broadcom
 */
/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright 2019 Broadcom Ltd.
 */
/*
 * 
 */
// BCMFORMAT: notabs reindent:uncrustify:bcm_minimal_i4.cfg

#include <common.h>
#include <linux/types.h>
#include "linux/printk.h"
#include <asm/arch/rng.h>
#include <asm/arch/misc.h>
#include "bcm_rng.h"



static void rng_enable(void)
{
    RNG->ctrl0 |= 0x1;
}

static void rng_reset(void)
{
    RNG->rngSoftReset |= 0x1;
    RNG->rngSoftReset &= ~0x1;
    RNG->rbgSoftReset |= 0x1;
    RNG->rbgSoftReset &= ~0x1;
}

void rng_init(void)
{
    rng_enable();
    rng_reset();
}


int rng_get_rand(u8* data, int len)
{
    int i;
    if (len > RNG_FIFO_DATA_MAX) {
        len = RNG_FIFO_DATA_MAX; 
    }
    if ((RNG->fifoCnt&0xff) < len) {
    //printf("fifo cnt 0x%x  total bits 0x%x\n",RNG->fifoCnt,RNG->totalBitCnt);
        if ( RNG->totalBitCnt >= (0xffffffff-(len*8))) { 
        /* reset RBG200 rng since it reached  max bit count*/
            RNG->rbgSoftReset |= 0x1;
            RNG->rbgSoftReset &= ~0x1;
        }
        while((RNG->fifoCnt&0xff) < len/sizeof(u32)) {
        /* time out here */
        }
    }
    for(i = 0; i < len/sizeof(u32); i++) {
        ((u32*)data)[i] = RNG->rngFifoData;
    }
    if ((len%sizeof(u32))) {
        u32 rand_word = RNG->rngFifoData;
        memcpy(&data[i], &rand_word, (len%sizeof(u32)));
    }
    return len;
}

int rng_pac_lock(uint32_t perm)
{
    uint32_t startRegion;
    if (perm == RNG_PERM_DISABLE_ALL)  {
        RNG->perm = perm; 
    } else {
        RNG->perm |= perm; 
    }
}
