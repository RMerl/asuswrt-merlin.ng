/* $Id$ */

/* Copyright (c) 2010 Dmitriy Gerasimov <gesser@demlabs.ru>
 * Copyright (c) 2010 Aaron Turner.
 * Copyright (c) 2023 Fred Klassen - AppNet Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the names of the copyright owners nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#ifdef HAVE_TX_RING

#include "defines.h"
#include "config.h"

#if __GLIBC__ >= 2 && __GLIBC_MINOR >= 1
#include <net/ethernet.h> /* the L2 protocols */
#include <netpacket/packet.h>
#else
#include <asm/types.h>
#include <linux/if_ether.h> /* The L2 protocols */
#include <linux/if_packet.h>
#endif
#include <pthread.h>

struct txring_s {
    pthread_t tx_send; /*Poll TX thread*/

    volatile struct tpacket_hdr *tx_head; /* Pointer to mmaped memory with TX ring */
    struct tpacket_req *treq;             /* TX ring parametrs */
    volatile unsigned int tx_index;       /* TX index */
    int tx_size;                          /* Size of mmaped TX ring */
};
typedef struct txring_s txring_t;

int txring_put(txring_t *txp, const void *data, size_t length);
txring_t *txring_init(int fd, unsigned int mtu);
#endif /* HAVE_TX_RING */
