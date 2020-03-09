/***********************************************************************
 *
 *  Copyright (c) 2014  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2006:DUAL/GPL:standard
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 *
 ************************************************************************/


#include "httpdshared.h"

static struct { 
    unsigned long keyHash;
    unsigned long  sig;
} gCtx = {0, 0};

static unsigned long power(unsigned long base, unsigned long exp) {
    if (exp == 0)
        return 1;
    else if (exp % 2)
        return base * power(base, exp - 1);
    else {
        int temp = power(base, exp / 2);
        return temp * temp;
    }
}

static unsigned long encrypt(unsigned long key, unsigned long sig ) {
    // a simple one-way encryption algorithm which prevents us from having 
    // to store the session key in the clear.
    unsigned long hash = 0;
    int i;
    int bits = sizeof(sig)*8;
    for (i = 0; i < bits; i++) {
        if ( (sig >> i) & 0x1 ) {
            hash = (((hash >> 3)+(hash&0x7 << (bits-3)))) ^ power(key, i);
        } else {
            hash ^= 0x50505050;
        }
    }
    return hash;
}

void hsl_setSessionKey(unsigned long key, unsigned long sig) {
    gCtx.sig = sig;
    gCtx.keyHash = encrypt(key, sig);
}

int hsl_checkSessionKey(unsigned long key) {
    return (key && (encrypt(key, gCtx.sig) == gCtx.keyHash));
}


