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


#define RNG_FIFO_DATA_BIT_MAX		(16*32) 
#define RNG_FIFO_DATA_MAX		(RNG_FIFO_DATA_BIT_MAX/8) 
#define RNG_FIFO_CNT_EMPTY_SHIFT	31
#define RNG_FIFO_CNT_FULL_SHIFT		30


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

#ifndef CONFIG_SMC_BASED
int rng_pac_lock(uint32_t perm)
{
    switch(perm)
    {
        case RNG_PERM_NSEC_ACCESS_ENABLE:
            RNG->perm |= RNG_PERM_NSEC_ENABLE;
#if ENABLE_DUAL_NONSEC_RNG
#include <asm/arch/trng.h>
            TRNG->perm |= TRNG_PERM_NSEC_ENABLE;
#endif	    
        break;

        case RNG_PERM_SEC_ACCESS_ENABLE:
            RNG->perm |= RNG_PERM_SEC_ENABLE;
        break;

        case RNG_PERM_ALL_ACCESS_DISABLE:
            RNG->perm = RNG_PERM_DISABLE_ALL; 
        break;

        default:
        break;
    }
    return 0;
}
#endif
