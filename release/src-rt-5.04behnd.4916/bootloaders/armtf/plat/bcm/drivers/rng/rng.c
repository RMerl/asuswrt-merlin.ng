/*
<:copyright-BRCM:2023:DUAL/GPL:standard

   Copyright (c) 2023 Broadcom 
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
#include <rng.h>
#include <bcm_rng.h>
#include <debug.h>

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
    rng_reset();
    rng_enable();
}


int rng_get_rand(uint8_t *data, int len)
{
    int i;

    if (len > RNG_FIFO_DATA_MAX) {
        len = RNG_FIFO_DATA_MAX;
    }

    if ((RNG->fifoCnt&0xff) < len) {
        if ( RNG->totalBitCnt >= (0xffffffff-(len*8))) {
	    /* reset RBG200 rng since it reached  max bit count*/
            RNG->rbgSoftReset |= 0x1;
            RNG->rbgSoftReset &= ~0x1;
        }
        while((RNG->fifoCnt&0xff) < len/sizeof(uint32_t)) {
        /* time out here */
        }
    }

    for(i = 0; i < len/sizeof(uint32_t); i++) {

        if (RNG->intStatus & (RNG_INT_STATUS_NIST_FAIL | RNG_INT_STATUS_LOCK_FAIL)) {
	    WARN("RESETTING RNG\n");
	    rng_init();
	}

        ((uint32_t*)data)[i] = RNG->rngFifoData;
    }

    if ((len%sizeof(uint32_t))) {
        uint32_t rand_word = RNG->rngFifoData;
        memcpy(&data[i], &rand_word, (len%sizeof(uint32_t)));
    }
    return len;
}

void rng_lock(void)
{
#ifndef CONFIG_BRCM_SMC_BASED
    RNG->perm = RNG_PERM_ALLOW_SECURE_ACCESS;
#endif
}
