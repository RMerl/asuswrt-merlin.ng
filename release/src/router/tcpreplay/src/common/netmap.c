/*
 *   Copyright (c) 2013 Fred Klassen <fklassen at appneta dot com> - AppNeta
 *
 *   The Tcpreplay Suite of tools is free software: you can redistribute it
 *   and/or modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation, either version 3 of the
 *   License, or with the authors permission any later version.
 *
 *   The Tcpreplay Suite is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with the Tcpreplay Suite.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "netmap.h"
#include "config.h"
#include "common.h"
#include "tcpreplay_api.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

static int nm_do_ioctl(sendpacket_t *sp, u_long what, int subcmd);

/**
 * Method takes an open "/dev/netmap" file descriptor and returns
 * the netmap version.
 *
 * Returns -1 on error
 */
int
get_netmap_version()
{
    u_int32_t netmap_version = -1;
    nmreq_t nmr;
    int fd;

    if ((fd = open("/dev/netmap", O_RDWR)) < 0)
        return -1;

    /* netmap version discovery */
    bzero(&nmr, sizeof(nmr));
    nmr.nr_version = NETMAP_API;

    /* attempt using the netmap API version that this was compiled under */
    if (ioctl(fd, NIOCGINFO, &nmr) == 0) {
        netmap_version = nmr.nr_version;
        dbgx(1, "netmap detected API version %d which matches compiled version\n", netmap_version);
    } else {
        /* failed.
         *
         * Try other versions in an attempt to find the version
         * that matches this system.
         */
        int x;
        for (x = 0; x < 50; ++x) {
            bzero(&nmr, sizeof(nmr));
            nmr.nr_version = x;
            if (ioctl(fd, NIOCGINFO, &nmr) == 0) {
                netmap_version = nmr.nr_version;
                dbgx(1,
                     "netmap detected API version %d which doesn't match compiled version %d\n",
                     netmap_version,
                     NETMAP_API);
                break;
            }
        }
    }

    close(fd);

    return (int)netmap_version;
}

/**
 * ioctl support for netmap
 */
static int
nm_do_ioctl(sendpacket_t *sp, u_long what, int subcmd)
{
    struct ifreq ifr;
    int error;
    int fd;
#ifdef linux
    struct ethtool_value eval;
#endif

    assert(sp);

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        dbg(1, "ioctl error: cannot get device control socket.\n");
        return -1;
    }

    bzero(&ifr, sizeof(ifr));
    strncpy(ifr.ifr_name, sp->device, sizeof(ifr.ifr_name));

    switch (what) {
    case SIOCSIFFLAGS:
        ifr.ifr_flags = sp->if_flags >> 16;
        ifr.ifr_flags = sp->if_flags & 0xffff;
        break;

#ifdef linux
    case SIOCETHTOOL:
        eval.cmd = subcmd;
        eval.data = sp->data;
        ifr.ifr_data = (caddr_t)&eval;
        dbgx(1, "ioctl SIOCETHTOOL subcmd=%d data=%u", subcmd, eval.data);
        break;
#endif
    }

    error = ioctl(fd, what, &ifr);
    if (error)
        goto done;

    switch (what) {
    case SIOCGIFFLAGS:
        sp->if_flags = (ifr.ifr_flags << 16) | (0xffff & ifr.ifr_flags);
        dbgx(1, "SIOCGIFFLAGS flags are 0x%x", sp->if_flags);
        break;

#ifdef linux
    case SIOCETHTOOL:
        switch (subcmd) {
        case ETHTOOL_GGSO:
            sp->gso = eval.data;
            dbgx(1, "ioctl SIOCETHTOOL ETHTOOL_GGSO=%u", eval.data);
            break;

        case ETHTOOL_GTSO:
            sp->tso = eval.data;
            dbgx(1, "ioctl SIOCETHTOOL ETHTOOL_GTSO=%u", eval.data);
            break;

        case ETHTOOL_GRXCSUM:
            sp->rxcsum = eval.data;
            dbgx(1, "ioctl SIOCETHTOOL ETHTOOL_GRXCSUM=%u", eval.data);
            break;

        case ETHTOOL_GTXCSUM:
            sp->txcsum = eval.data;
            dbgx(1, "ioctl SIOCETHTOOL ETHTOOL_GTXCSUM=%u", eval.data);
            break;
        default:
            return -1;
        }
        break;
#endif
    default:
        return -1;
    }

done:
    close(fd);

    if (error)
        warnx("ioctl error %d %lu:%d", error, what, subcmd);
    return error;
}

/**
 * Inner sendpacket_open() method for using netmap
 */
void *
sendpacket_open_netmap(const char *device, char *errbuf, void *arg)
{
    tcpreplay_t *ctx = (tcpreplay_t *)arg;
    sendpacket_t *sp = NULL;
    nmreq_t nmr;
    char ifname_buf[MAX_IFNAMELEN];
    const char *ifname;
    const char *port = NULL;
    size_t namelen;
    u_int32_t nr_ringid = 0;
    u_int32_t nr_flags = NR_REG_DEFAULT;
    int is_default = 0;

    assert(device);
    assert(errbuf);

    dbg(1, "sendpacket_open_netmap: using netmap");

    bzero(&nmr, sizeof(nmr));

    /* prep & return our sp handle */
    sp = (sendpacket_t *)safe_malloc(sizeof(sendpacket_t));

    if (strlen(device) > MAX_IFNAMELEN - 8) {
        snprintf(errbuf, SENDPACKET_ERRBUF_SIZE, "Interface name is to long: %s\n", device);
        goto IFACENAME_INVALID;
    }

    /* get the version of the netmap driver. If < 0, driver is not installed */
    sp->netmap_version = get_netmap_version();
    if (sp->netmap_version < 0) {
        snprintf(errbuf,
                 SENDPACKET_ERRBUF_SIZE,
                 "Unable to determine the running netmap version.\n"
                 "See INSTALL document for details on installing or upgrading netmap.");
        goto NETMAP_NOT_INSTALLED;
    }

    /*
     * Sort out interface names
     *
     * ifname   (foo, netmap:foo or vale:foo) is the port name
     *      foo         bind to a single NIC hardware queue
     *      netmap:foo  bind to a single NIC hardware queue
     *      vale:foo    bind to the Vale virtual interface
     *
     * for netmap version 10+ a suffix can indicate the following:
     *      netmap:foo!     bind to all NIC hardware queues (may cause TX reordering)
     *      netmap:foo^     bind to the host (sw) ring pair
     *      netmap:foo*     bind to the host (sw) and NIC ring pairs (transparent)
     *      netmap:foo-NN   bind to the individual NIC ring pair (queue) where NN = the ring number
     *      netmap:foo{NN   bind to the master side of pipe NN
     *      netmap:foo}NN   bind to the slave side of pipe NN
     */
    if (strncmp(device, "netmap:", 7) && strncmp(device, "vale", 4)) {
        snprintf(ifname_buf, sizeof(ifname_buf), "netmap:%s", device);
        ifname = ifname_buf;
    } else {
        ifname = device;
    }

    if (!strncmp("vale", device, 4))
        sp->is_vale = 1;

    if (ifname[0] == 'n')
        ifname += 7;

    /* scan for a separator */
    for (port = ifname; *port && !index("!-*^{}", *port); port++)
        ;

    namelen = port - ifname;
    if (namelen > sizeof(nmr.nr_name)) {
        snprintf(errbuf, SENDPACKET_ERRBUF_SIZE, "Interface name is to long: %s\n", device);
        goto IFACENAME_INVALID;
    }
    /*
     * Open the netmap device to fetch the number of queues of our
     * interface.
     *
     * The first NIOCREGIF also detaches the card from the
     * protocol stack and may cause a reset of the card,
     * which in turn may take some time for the PHY to
     * reconfigure.
     */
    if ((sp->handle.fd = open("/dev/netmap", O_RDWR)) < 0) {
        dbg(1, "sendpacket_open_netmap: Unable to access netmap");
        snprintf(errbuf,
                 SENDPACKET_ERRBUF_SIZE,
                 "Unable to access netmap.\n"
                 "See INSTALL to learn which NICs are supported and\n"
                 "how to set up netmap-capable network drivers.");
        goto OPEN_FAILED;
    }

    /*
     * The nmreq structure must have the NETMAP_API version for the running machine.
     * However the binary may have been compiled on a different machine than the
     * running machine. Discover the true netmap API version, and be careful to call
     * functions that are available on all netmap versions.
     */
    if (sp->netmap_version >= 10) {
        switch (*port) {
        case '-': /* one NIC */
            nr_flags = NR_REG_ONE_NIC;
            nr_ringid = strtol(port + 1, NULL, 10);
            break;

        case '*': /* NIC and SW, ignore port */
            nr_flags = NR_REG_NIC_SW;
            if (port[1]) {
                snprintf(errbuf, SENDPACKET_ERRBUF_SIZE, "invalid netmap port for nic+sw");
                goto NETMAP_IF_PARSE_FAIL;
            }
            break;

        case '^': /* only sw ring */
            nr_flags = NR_REG_SW;
            if (port[1]) {
                snprintf(errbuf, SENDPACKET_ERRBUF_SIZE, "invalid port for sw ring");
                goto NETMAP_IF_PARSE_FAIL;
            }
            break;

        case '{':
            nr_flags = NR_REG_PIPE_MASTER;
            nr_ringid = strtol(port + 1, NULL, 10);
            break;

        case '}':
            nr_flags = NR_REG_PIPE_SLAVE;
            nr_ringid = strtol(port + 1, NULL, 10);
            break;

        case '!':
            nr_flags = NR_REG_ALL_NIC;
            break;

        default: /* '\0', no suffix */
            nr_flags = NR_REG_ALL_NIC;
            is_default = 1;
            break;
        }

        if (nr_ringid >= NETMAP_RING_MASK) {
            snprintf(errbuf, SENDPACKET_ERRBUF_SIZE, "invalid ringid");
            goto NETMAP_IF_PARSE_FAIL;
        }

        nmr.nr_ringid = nr_ringid;
        nmr.nr_flags = nr_flags;
    }

    nmr.nr_version = sp->netmap_version;
    memcpy(nmr.nr_name, ifname, namelen);
    nmr.nr_name[namelen] = '\0';
    strlcpy(sp->device, nmr.nr_name, sizeof(sp->device));

    /*
     * Register the interface on the netmap device: from now on,
     * we can operate on the network interface without any
     * interference from the legacy network stack.
     *
     * Cards take a long time to reset the PHY.
     */
    fprintf(stderr, "Switching network driver for %s to netmap bypass mode... ", sp->device);
    fflush(NULL);
    sleep(1); /* ensure message prints when user is connected via ssh */

    if (ioctl(sp->handle.fd, NIOCREGIF, &nmr)) {
        snprintf(errbuf,
                 SENDPACKET_ERRBUF_SIZE,
                 "Failure accessing netmap.\n"
                 "\tRequest for netmap version %d failed.\n\tCompiled netmap driver is version %d.\n\tError=%s\n",
                 sp->netmap_version,
                 NETMAP_API,
                 strerror(errno));
        goto NETMAP_IF_FAILED;
    }

    if (!nmr.nr_memsize) {
        snprintf(errbuf, SENDPACKET_ERRBUF_SIZE, "Netmap interface '%s' not configured.\n", device);
        goto NETMAP_IF_FAILED;
    }

    sp->mmap_size = nmr.nr_memsize;
    sp->mmap_addr = (struct netmap_d *)mmap(0, sp->mmap_size, PROT_WRITE | PROT_READ, MAP_SHARED, sp->handle.fd, 0);

    if (!sp->mmap_addr || sp->mmap_addr == MAP_FAILED) {
        snprintf(errbuf, SENDPACKET_ERRBUF_SIZE, "mmap: %s", strerror(errno));
        goto MMAP_FAILED;
    }

    dbgx(1, "sendpacket_open_netmap: mapping %d Kbytes queues=%d", sp->mmap_size >> 10, nmr.nr_tx_rings);

    sp->nm_if = NETMAP_IF(sp->mmap_addr, nmr.nr_offset);
    sp->nmr = nmr;
    sp->handle_type = SP_TYPE_NETMAP;

    /* set up ring IDs */
    sp->cur_tx_ring = 0;
    switch (nr_flags) {
    case NR_REG_DEFAULT: /* only use one queue to prevent TX reordering */
        sp->first_tx_ring = sp->last_tx_ring = sp->cur_tx_ring = 0;
        break;

    case NR_REG_ALL_NIC:
        if (is_default) {
            sp->first_tx_ring = sp->last_tx_ring = sp->cur_tx_ring = 0;
        } else {
            sp->first_tx_ring = sp->cur_tx_ring = 0;
            sp->last_tx_ring = nmr.nr_tx_rings - 1;
        }
        break;

    case NR_REG_SW:
        sp->first_tx_ring = sp->last_tx_ring = sp->cur_tx_ring = nmr.nr_tx_rings;
        break;

    case NR_REG_NIC_SW:
        sp->first_tx_ring = sp->cur_tx_ring = 0;
        sp->last_tx_ring = nmr.nr_tx_rings;
        break;

    case NR_REG_ONE_NIC:
        sp->first_tx_ring = sp->last_tx_ring = sp->cur_tx_ring = nr_ringid;
        break;

    default:
        sp->first_tx_ring = sp->last_tx_ring = sp->cur_tx_ring = 0;
    }
    {
        /* debugging code */
        int i;

        dbgx(1, "%s tx first=%d last=%d  num=%d", ifname, sp->first_tx_ring, sp->last_tx_ring, sp->nmr.nr_tx_rings);
        for (i = 0; i <= sp->nmr.nr_tx_rings; i++) {
#ifdef HAVE_NETMAP_RING_HEAD_TAIL
            dbgx(1,
                 "TX%d 0x%p head=%d cur=%d tail=%d",
                 i,
                 NETMAP_TXRING(sp->nm_if, i),
                 (NETMAP_TXRING(sp->nm_if, i))->head,
                 (NETMAP_TXRING(sp->nm_if, i))->cur,
                 (NETMAP_TXRING(sp->nm_if, i))->tail);
#else
            dbgx(1,
                 "TX%d 0x%p cur=%d avail=%d",
                 i,
                 NETMAP_TXRING(sp->nm_if, i),
                 (NETMAP_TXRING(sp->nm_if, i))->cur,
                 (NETMAP_TXRING(sp->nm_if, i))->avail);
#endif
        }
    }

    dbgx(2, "Waiting %d seconds for phy reset...", ctx->options->netmap_delay);
    sleep(ctx->options->netmap_delay);
    dbg(2, "Ready!");

    if (!sp->is_vale) {
        if (nm_do_ioctl(sp, SIOCGIFFLAGS, 0) < 0)
            goto NM_DO_IOCTL_FAILED;

        if ((sp->if_flags & IFF_RUNNING) == 0) {
            dbgx(1, "sendpacket_open_netmap: %s is not running", sp->device);
            snprintf(errbuf, SENDPACKET_ERRBUF_SIZE, "interface %s is not running - check cables\n", sp->device);
            goto NETMAP_IF_NOT_RUNNING;
        }

        if ((sp->if_flags & IFF_UP) == 0) {
            dbgx(1, "%s is down, bringing up...", sp->device);
            sp->if_flags |= IFF_UP;
        }

        /* set promiscuous mode */
        sp->if_flags |= IFF_PROMISC;
        if (nm_do_ioctl(sp, SIOCSIFFLAGS, 0) < 0)
            goto NM_DO_IOCTL_FAILED;

#ifdef linux
        /* disable:
         * - generic-segmentation-offload
         * - tcp-segmentation-offload
         * - rx-checksumming
         * - tx-checksumming
         */
        if (nm_do_ioctl(sp, SIOCETHTOOL, ETHTOOL_GGSO) < 0 || nm_do_ioctl(sp, SIOCETHTOOL, ETHTOOL_GTSO) < 0 ||
            nm_do_ioctl(sp, SIOCETHTOOL, ETHTOOL_GRXCSUM) < 0 || nm_do_ioctl(sp, SIOCETHTOOL, ETHTOOL_GTXCSUM) < 0)
            goto NM_DO_IOCTL_FAILED;

        sp->data = 0;
        if (nm_do_ioctl(sp, SIOCETHTOOL, ETHTOOL_SGSO) < 0 || nm_do_ioctl(sp, SIOCETHTOOL, ETHTOOL_STSO) < 0 ||
            nm_do_ioctl(sp, SIOCETHTOOL, ETHTOOL_SRXCSUM) < 0 || nm_do_ioctl(sp, SIOCETHTOOL, ETHTOOL_STXCSUM))
            goto NM_DO_IOCTL_FAILED;
#endif
    }

    if (sp->abort)
        goto NETMAP_ABORT;

    notice("done!");

    return sp;

NM_DO_IOCTL_FAILED:
    snprintf(errbuf, SENDPACKET_ERRBUF_SIZE, "nm_do_ioctl: %s", strerror(errno));
NETMAP_IF_NOT_RUNNING:
    notice("failed!");
NETMAP_ABORT:
    fprintf(stderr, " Switching network driver for %s to normal mode... ", sp->device);
    fflush(NULL);
    munmap(sp->mmap_addr, sp->mmap_size);
MMAP_FAILED:
#if NETMAP_API < 10
    ioctl(sp->handle.fd, NIOCUNREGIF, NULL);
#endif
NETMAP_IF_FAILED:
NETMAP_IF_PARSE_FAIL:
    close(sp->handle.fd);
OPEN_FAILED:
    safe_free(sp);
IFACENAME_INVALID:
NETMAP_NOT_INSTALLED:

    return NULL;
}

void
sendpacket_close_netmap(void *p)
{
    sendpacket_t *sp = p;
    fprintf(stderr, "Switching network driver for %s to normal mode... ", sp->device);
    fflush(NULL);

    /* flush any remaining packets */
    ioctl(sp->handle.fd, NIOCTXSYNC, NULL);

    /* wait for traffic to be sent */
    dbgx(2, "Waiting %d seconds for phy reset...", sp->netmap_delay);
    sleep(sp->netmap_delay);
    dbg(2, "Ready!");

#ifdef linux
    if (!sp->is_vale) {
        /* restore original settings:
         * - generic-segmentation-offload
         * - tcp-segmentation-offload
         * - rx-checksumming
         * - tx-checksumming
         */
        sp->data = sp->gso;
        nm_do_ioctl(sp, SIOCETHTOOL, ETHTOOL_SGSO);
        sp->data = sp->tso;
        nm_do_ioctl(sp, SIOCETHTOOL, ETHTOOL_STSO);
        sp->data = sp->rxcsum;
        nm_do_ioctl(sp, SIOCETHTOOL, ETHTOOL_SRXCSUM);
        sp->data = sp->txcsum;
        nm_do_ioctl(sp, SIOCETHTOOL, ETHTOOL_STXCSUM);
    }
#endif /* linux */

    /* restore interface to normal mode */
#if NETMAP_API < 10
    ioctl(sp->handle.fd, NIOCUNREGIF, NULL);
#endif
    if (sp->mmap_addr)
        munmap(sp->mmap_addr, sp->mmap_size);
    close(sp->handle.fd);
    notice("done!");
}

bool
netmap_tx_queues_empty(void *p)
{
    sendpacket_t *sp = p;
    struct netmap_ring *txring;

    assert(sp);

    sp->cur_tx_ring = 0;
    txring = NETMAP_TXRING(sp->nm_if, sp->cur_tx_ring);
    while (NETMAP_TX_RING_EMPTY(txring)) {
        /* current ring is empty- go to next */
        ++sp->cur_tx_ring;
        if (sp->cur_tx_ring > sp->last_tx_ring) {
            /* last ring */
            sp->cur_tx_ring = 0;
            return true;
        }

        txring = NETMAP_TXRING(sp->nm_if, sp->cur_tx_ring);
    }

    /*
     * send TX interrupt signal
     */
    ioctl(sp->handle.fd, NIOCTXSYNC, NULL);

    return false;
}

int
sendpacket_send_netmap(void *p, const u_char *data, size_t len)
{
    sendpacket_t *sp = p;
    struct netmap_ring *txring;
    struct netmap_slot *slot;
    char *pkt;
    uint32_t cur, avail;

    if (sp->abort)
        return 0;

    txring = NETMAP_TXRING(sp->nm_if, sp->cur_tx_ring);
    while ((avail = nm_ring_space(txring)) == 0) {
        /* out of space on current TX queue - go to next */
        ++sp->cur_tx_ring;
        if (sp->cur_tx_ring > sp->last_tx_ring) {
            /*
             * out of space on all queues
             *
             * we have looped through all configured TX queues
             * so we have to reset to the first queue and
             * wait for available space
             */
            sp->cur_tx_ring = sp->first_tx_ring;

            /* send TX interrupt signal
             *
             * On Linux this makes one slot free on the
             * ring, which increases speed by about 10Mbps.
             *
             * But it will never free up all the slots. For
             * that we must poll and call again.
             */
            ioctl(sp->handle.fd, NIOCTXSYNC, NULL);

            /* loop again */
            return -2;
        }

        txring = NETMAP_TXRING(sp->nm_if, sp->cur_tx_ring);
    }

    /*
     * send
     */
    cur = txring->cur;
    slot = &txring->slot[cur];
    slot->flags = 0;
    pkt = NETMAP_BUF(txring, slot->buf_idx);
    memcpy(pkt, data, min(len, txring->nr_buf_size));
    slot->len = len;

    if (avail <= 1)
        slot->flags = NS_REPORT;

    dbgx(3,
         "netmap cur=%d slot index=%d flags=0x%x empty=%d avail=%u bufsize=%d\n",
         cur,
         slot->buf_idx,
         slot->flags,
         NETMAP_TX_RING_EMPTY(txring),
         nm_ring_space(txring),
         txring->nr_buf_size);

    /* let kernel know that packet is available */
    cur = NETMAP_RING_NEXT(txring, cur);
#ifdef HAVE_NETMAP_RING_HEAD_TAIL
    txring->head = cur;
#else
    txring->avail--;
#endif
    txring->cur = cur;

    return len;
}
