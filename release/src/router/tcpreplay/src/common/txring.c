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

#ifdef HAVE_TX_RING

#include "txring.h"
#include "err.h"
#include "utils.h"
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

volatile int shutdown_flag = 0;
int tdata_offset = TPACKET_HDRLEN - sizeof(struct sockaddr_ll);

/**
 * This task will call send() procedure
 */
void *
txring_send(void *arg)
{
    int ec_send;
    static int total = 0;
    int fd_socket = (int)arg;

    do {
        /* send all buffers with TP_STATUS_SEND_REQUEST */
        ec_send = sendto(fd_socket, NULL, 0, MSG_DONTWAIT, (struct sockaddr *)NULL, sizeof(struct sockaddr_ll));

        if (ec_send > 0) {
            total += ec_send;
            dbgx(2, "Sent %d bytes (+%d bytes)", total, ec_send);
        } else {
            /* nothing to do => schedule : useful if no SMP */
            usleep(100);
        }

    } while (!shutdown_flag);

    // if(blocking) printf("end of task send()\n");
    // printf("end of task send(ec=%x)\n", ec_send);

    return (void *)ec_send;
}

/**
 * Put data in TX ring buffer and rotate it if necessary
 */
int
txring_put(txring_t *txp, const void *data, size_t length)
{
    struct tpacket_hdr *ps_header;
    char *to_data;
    int loop = 1;
    int first_loop = 1;
    unsigned int start_index = txp->tx_index;

    do {
        ps_header = ((struct tpacket_hdr *)((void *)txp->tx_head + (txp->treq->tp_frame_size * txp->tx_index)));
        to_data = ((void *)ps_header) + tdata_offset;

        switch ((volatile uint32_t)ps_header->tp_status) {
        case TP_STATUS_WRONG_FORMAT:
            warnx("TP_STATUS_WRONG_FORMAT occuries O_o. Frame %d, pkt len %d\n", txp->tx_index, length);
            break;

        case TP_STATUS_AVAILABLE:
            if (length > txp->treq->tp_frame_size) {
                // TODO Fragment packet
                warnx("[!] %d bytes from %d packet truncated\n", length - txp->treq->tp_frame_size, length);
                length = txp->treq->tp_frame_size;
            }
            memcpy(to_data, data, length);
            ps_header->tp_len = length;
            ps_header->tp_status = TP_STATUS_SEND_REQUEST;
            loop = 0;
            break;

        default:
            dbgx(2,
                 "TPACKET status %u at frame %d with length %d\n",
                 ps_header->tp_status,
                 txp->tx_index,
                 ps_header->tp_len);
            usleep(0);
            break;
        }
        txp->tx_index++;

        if (txp->tx_index >= txp->treq->tp_frame_nr) {
            txp->tx_index = 0;
            first_loop = 0;
        }

        /* check if we've ran over all ring */
        if ((txp->tx_index == start_index) && !first_loop) {
            errno = ENOBUFS;
            return -1;
        }
    } while (loop == 1);

    return ps_header->tp_len;
}

/**
 * \brief Build TX ring buffer request structure
 *
 * This builds a ring buffer request structure making sure
 * that we have buffers big enough so that a frame which
 * is the size of the MTU doesn't get truncated. We also
 * need to structure things with minimum memory wastage
 */
void
txring_mkreq(struct tpacket_req *treq, unsigned int mtu)
{
    unsigned int pg, bs;
    unsigned int s;
    unsigned int mult = 1;
    unsigned nr_blocks = 1000;

    bs = pg = getpagesize();
    s = mtu + TPACKET_HDRLEN;

    memset(treq, 0, sizeof(struct tpacket_req));
    if (bs <= s) {
        while (bs < s) {
            bs += pg;
            mult++;
        }

        treq->tp_block_size = bs;
        treq->tp_frame_size = bs / mult;
        treq->tp_block_nr = nr_blocks;
        treq->tp_frame_nr = mult * nr_blocks;
    } else {
        while ((s * (mult + 1)) <= pg) {
            mult++;
        }
        treq->tp_block_size = pg;
        treq->tp_frame_size = pg / mult;
        treq->tp_block_nr = nr_blocks;
        treq->tp_frame_nr = mult * nr_blocks;
    }
    dbgx(1,
         "txring: block_size=%d block_nr=%d frame_size=%d frame_nr=%d",
         treq->tp_block_size,
         treq->tp_block_nr,
         treq->tp_frame_size,
         treq->tp_frame_nr);
}

/**
 * \brief Create TX ring for socket and init indexes
 *
 * Creates our pthread for sending, currently hardcoded for priority = 20
 */
txring_t *
txring_init(int fd, unsigned int mtu)
{
    pthread_attr_t t_attr_send;
    struct sched_param para_send;
    int mode_loss = 0;
    txring_t *txp;

    /* allocate memory for structure and fill it with different stuff*/
    *txp = (txring_t *)safe_malloc(sizeof(txring_t));
    txp->treq = (struct tpacket_req *)safe_malloc(sizeof(struct tpacket_req));

    txring_mkreq(txp->treq, mtu);
    txp->tx_size = txp->treq->tp_block_size * txp->treq->tp_block_nr;
    txp->tx_index = 0; /* Set index on start*/

    /* Set PACKET_LOSS sockoption */
    if (setsockopt(fd, SOL_PACKET, PACKET_LOSS, (char *)&mode_loss, sizeof(mode_loss)) < 0) {
        perror("setsockopt: PACKET_LOSS");
        return NULL;
    }

    /* Enable TX Ring */
    if (setsockopt(fd, SOL_PACKET, PACKET_TX_RING, (char *)txp->treq, sizeof(struct tpacket_req)) < 0) {
        perror("Can't setsockopt PACKET_TX_RING");
        return NULL;
    }

    /* mmap unswapped memory with TX ring buffer*/
    txp->tx_head = mmap(0, txp->tx_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (txp->tx_head == MAP_FAILED) {
        perror("mmap() failed ");
        return NULL;
    }

    /* Start poll thread*/
    pthread_attr_init(&t_attr_send);
    pthread_attr_setschedpolicy(&t_attr_send, SCHED_RR);
    para_send.sched_priority = 20;
    pthread_attr_setschedparam(&t_attr_send, &para_send);

    if (pthread_create(&txp->tx_send, &t_attr_send, txring_send, (void *)fd) != 0) {
        perror("pthread_create() failed\n");
        abort();
    }

    return txp;
}

#endif /* HAVE_TX_RING */
