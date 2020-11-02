/*
<:copyright-BRCM:2013:GPL/GPL:standard

   Copyright (c) 2013 Broadcom 
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

#include <linux/module.h>
#include <linux/hw_random.h>
#include <bcm_map_part.h>

// TRNG output is added to a sha256 calcualtion (shastate =0 ) until there are 
// 64 bytes processed.  Then, the RESULTS of the sha256 form the next
// 32 bytes available to /dev/hwrandom
// FIXME: This uses a public domain sha256 which is redundant with the 
//        kernel crypto API.

#include "Sha256.h"
static CSha256 sha;
static char digest[32];
static int shastate ;
static int shacnt ;

static int bcm_rng_data_present(struct hwrng *rng, int wait)
{
    u32 i;
    if ((RNG->fifoCnt & 0xff) != 0) {
        if (shastate == 0) {
            i = RNG->rngFifoData;
            if (shacnt == 0) {
                Sha256_Init(&sha);
            }
            if (shacnt <= 64) {
                Sha256_Update(&sha, (void *)&i,4);
                shacnt = shacnt + 4;
            } else {
                Sha256_Final(&sha, (void *)digest);
                shacnt = 0;
                shastate = 1;
            }
       }
    }
    return((shastate == 1));
}

static int bcm_rng_data_read(struct hwrng *rng, u32 *data)
{
    *data = *((u32 *)(digest + shacnt));
    if (shastate == 1) {
        shacnt = shacnt + 4;
        if (shacnt >= 32) {
            shacnt = 0;
            shastate = 0;
        }
        return 4;
    } else {
        return 0;
    }
}

static struct hwrng bcm_rng = {
    .name           = "bcm_periph",
    .init           = NULL,
    .cleanup        = NULL,
    .data_present   = bcm_rng_data_present,
    .data_read      = bcm_rng_data_read,
};


static int __init mod_init(void)
{
    int err;
    err = hwrng_register(&bcm_rng);
    if (err) {
        printk("failed to register TRNG\n");
    } else {
        printk("BRCM TRNG registered\n");
    }
    shastate = 0;
    shacnt = 0;
    return err;
}

static void __exit mod_exit(void)
{
    hwrng_unregister(&bcm_rng);
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");

