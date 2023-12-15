#include "fuzzing.h"
#include <tcpedit/tcpedit.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static unsigned int fuzz_seed;
static unsigned int fuzz_factor;
static unsigned int fuzz_running;

void
fuzzing_init(uint32_t _fuzz_seed, uint32_t _fuzz_factor)
{
    assert(_fuzz_factor);

    fuzz_seed = _fuzz_seed;
    fuzz_factor = _fuzz_factor;
    fuzz_running = 1;
}

#define SGT_MAX_SIZE 16
static inline int
fuzz_get_sgt_size(uint32_t r, uint32_t caplen)
{
    if (0 == caplen)
        return 0;

    if (caplen <= SGT_MAX_SIZE)
        /* packet too small, fuzzing only one byte */
        return 1;

    /* return random value between 1 and SGT_MAX_SIZE */
    return (1 + (r % (SGT_MAX_SIZE - 1)));
}

static inline int
fuzz_reduce_packet_size(tcpedit_t *tcpedit, struct pcap_pkthdr *pkthdr, uint32_t new_len)
{
    if (pkthdr->len < pkthdr->caplen) {
        tcpedit_seterr(tcpedit, "Packet length %u smaller than capture length %u", pkthdr->len, pkthdr->caplen);
        return -1;
    }

    if (new_len > pkthdr->caplen) {
        tcpedit_seterr(tcpedit, "Cannot fuzz packet of capture length %u to length %u", pkthdr->caplen, new_len);
        return -1;
    }

    if (new_len == pkthdr->caplen) {
        return 0;
    }

    pkthdr->len = new_len;
    pkthdr->caplen = pkthdr->len;

    /* do not fix lengths in ip/tcp/udp layers.
     * fixlen option already does so, and can be called with fuzzing option. */

    return 1;
}

int
fuzzing(tcpedit_t *tcpedit, struct pcap_pkthdr *pkthdr, u_char **pktdata)
{
    int chksum_update_required = 0;
    uint32_t r, s;
    uint16_t l2proto;
    uint8_t l4proto;
    u_char *packet, *l3data, *l4data, *end_ptr;
    tcpeditdlt_plugin_t *plugin;
    int l2len, l4len;
    tcpeditdlt_t *ctx;

    assert(tcpedit);
    assert(pkthdr);
    assert(*pktdata);

    if (!fuzz_running)
        goto done;

    assert(fuzz_factor);

    /*
     * Determine if this is one of the packets that is going to be altered.
     * No fuzzing for the other 7 out of 8 packets
     */
    r = tcpr_random(&fuzz_seed);
    if ((r % fuzz_factor) != 0)
        goto done;

    /* initializations */
    ctx = tcpedit->dlt_ctx;
    packet = *pktdata;
    end_ptr = packet + pkthdr->caplen;
    plugin = tcpedit->dlt_ctx->encoder;
    l2len = plugin->plugin_l2len(ctx, packet, pkthdr->caplen);
    l2proto = ntohs(plugin->plugin_proto(ctx, packet, pkthdr->caplen));
    if (l2len == -1 || (int)pkthdr->caplen < l2len)
        goto done;

    /*
     * Get a pointer to the network layer
     *
     * Note that this pointer may be in a working buffer and not on directly
     * to '*pktdata'. All alterations are done in this buffer, which later
     * will be copied back to '*pktdata', if necessary
     */
    l3data = plugin->plugin_get_layer3(ctx, packet, pkthdr->caplen);
    if (!l3data)
        goto done;

    switch (l2proto) {
    case (ETHERTYPE_IP): {
        l4data = get_layer4_v4((ipv4_hdr_t *)(packet + l2len), end_ptr);
        if (!l4data)
            goto done;

        l4len = l4data - packet;
        l4proto = ((ipv4_hdr_t *)l3data)->ip_p;
        break;
    }
    case (ETHERTYPE_IP6): {
        l4data = get_layer4_v6((ipv6_hdr_t *)(packet + l2len), end_ptr);
        if (!l4data)
            goto done;

        l4len = l4data - packet;
        l4proto = ((ipv6_hdr_t *)l3data)->ip_nh;
        break;
    }
    default:
        /* apply fuzzing on unknown packet types */
        l4len = pkthdr->caplen - l2len;
        l4data = packet + l2len;
        l4proto = IPPROTO_RAW;
    }

    /* adjust payload length based on layer 3 protocol */
    switch (l4proto) {
    case IPPROTO_TCP:
        l4len -= sizeof(tcp_hdr_t);
        l4data += sizeof(tcp_hdr_t);
        break;
    case IPPROTO_UDP:
        l4len -= sizeof(udp_hdr_t);
        l4data += sizeof(udp_hdr_t);
        break;
    }

    if (l4len <= 1 || l4data > end_ptr)
        goto done;

    /* add some additional randomization */
    r ^= r >> 16;

    s = r % FUZZING_TOTAL_ACTION_NUMBER;
    switch (s) {
    case FUZZING_DROP_PACKET: {
        /* simulate dropping the packet */
        if (fuzz_reduce_packet_size(tcpedit, pkthdr, 0) < 0)
            /* could not change packet size, so packet left unchanged */
            goto done;

        break;
    }
    case FUZZING_REDUCE_SIZE: {
        /* reduce packet size */
        uint32_t new_len = (r % (l4len - 1)) + 1;
        if (fuzz_reduce_packet_size(tcpedit, pkthdr, new_len) < 0)
            /* could not change packet size, so packet left unchanged */
            goto done;

        chksum_update_required = 1;
        break;
    }
    case FUZZING_CHANGE_START_ZERO: {
        /* fuzz random-size segment at the beginning of the packet with 0x00 */
        uint32_t sgt_size = fuzz_get_sgt_size(r, l4len);
        memset(l4data, 0x00, sgt_size);
        chksum_update_required = 1;
        break;
    }
    case FUZZING_CHANGE_START_RANDOM: {
        /*
         * fuzz random-size segment at the beginning of the packet payload
         * with random bytes
         */
        size_t i;
        uint32_t sgt_size = fuzz_get_sgt_size(r, l4len);
        if (!sgt_size)
            goto done;

        for (i = 0; i < sgt_size; i++)
            l4data[i] = l4data[i] ^ (u_char)(r >> 4);

        chksum_update_required = 1;
        break;
    }
    case FUZZING_CHANGE_START_FF: {
        /*
         * fuzz random-size segment at the beginning of the packet
         * payload with 0xff
         */
        uint32_t sgt_size = fuzz_get_sgt_size(r, l4len);
        if (!sgt_size)
            goto done;

        memset(l4data, 0xff, sgt_size);
        chksum_update_required = 1;
        break;
    }
    case FUZZING_CHANGE_MID_ZERO: {
        /* fuzz random-size segment inside the packet payload with 0x00 */
        if (l4len <= 2)
            goto done;

        uint32_t offset = ((r >> 16) % (l4len - 1)) + 1;
        uint32_t sgt_size = fuzz_get_sgt_size(r, l4len - offset);
        if (!sgt_size)
            goto done;

        memset(l4data + offset, 0x00, sgt_size);
        chksum_update_required = 1;
        break;
    }
    case FUZZING_CHANGE_MID_FF: {
        /* fuzz random-size segment inside the packet payload with 0xff */
        if (l4len <= 2)
            goto done;

        uint32_t offset = ((r >> 16) % (l4len - 1)) + 1;
        uint32_t sgt_size = fuzz_get_sgt_size(r, l4len - offset);
        if (!sgt_size)
            goto done;

        memset(l4data + offset, 0xff, sgt_size);
        chksum_update_required = 1;
        break;
    }
    case FUZZING_CHANGE_END_ZERO: {
        /* fuzz random-sized segment at the end of the packet payload with 0x00 */
        int sgt_size = fuzz_get_sgt_size(r, l4len);
        if (!sgt_size || sgt_size > l4len)
            goto done;

        memset(l4data + l4len - sgt_size, 0x00, sgt_size);
        chksum_update_required = 1;
        break;
    }
    case FUZZING_CHANGE_END_RANDOM: {
        /* fuzz random-sized segment at the end of the packet with random Bytes */
        int i;
        int sgt_size = fuzz_get_sgt_size(r, l4len);
        if (!sgt_size || sgt_size > l4len)
            goto done;

        for (i = (l4len - sgt_size); i < l4len; i++)
            l4data[i] = l4data[i] ^ (u_char)(r >> 4);

        chksum_update_required = 1;
        break;
    }
    case FUZZING_CHANGE_END_FF: {
        /* fuzz random-sized segment at the end of the packet with 0xff00 */
        int sgt_size = fuzz_get_sgt_size(r, l4len);
        if (!sgt_size || sgt_size > l4len)
            goto done;

        memset(l4data + l4len - sgt_size, 0xff, sgt_size);
        chksum_update_required = 1;
        break;
    }

    case FUZZING_CHANGE_MID_RANDOM: {
        /* fuzz random-size segment inside the packet with random Bytes */
        size_t i;
        uint32_t offset = ((r >> 16) % (l4len - 1)) + 1;
        int sgt_size = fuzz_get_sgt_size(r, l4len - offset);
        if (!sgt_size || sgt_size > l4len)
            goto done;

        for (i = offset; i < offset + sgt_size; i++)
            l4data[i] = l4data[i] ^ (u_char)(r >> 4);

        chksum_update_required = 1;
        break;
    }
    default:
        assert(false);
    }

    dbgx(3, "packet %llu fuzzed : %d", tcpedit->runtime.packetnum, s);

done:
    return chksum_update_required;
}
