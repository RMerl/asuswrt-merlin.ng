/* $Id$ */

/*
 *   Copyright (c) 2001-2010 Aaron Turner <aturner at synfin dot net>
 *   Copyright (c) 2013-2022 Fred Klassen <tcpreplay at appneta dot com> - AppNeta
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

#include "defines.h"
#include "config.h"
#include "common.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

static tcpr_cache_t *new_cache(void);

/**
 * Takes a single char and returns a ptr to a string representation of the
 * 8 bits that make up that char.  Use BIT_STR() to print it out
 */
#ifdef DEBUG
static char *
byte2bits(char byte, char *bitstring)
{
    int i, j = 7;

    for (i = 1; i <= 255; i = i << 1) {
        if (byte & i)
            bitstring[j] = '\061';
        j--;
    }

    return bitstring;
}
#endif

/**
 * simple function to read in a cache file created with tcpprep this let's us
 * be really damn fast in picking an interface to send the packet out returns
 * number of cache entries read
 *
 * now also checks for the cache magic and version
 */

COUNTER
read_cache(char **cachedata, const char *cachefile, char **comment)
{
    int cachefd;
    tcpr_cache_file_hdr_t header;
    ssize_t read_size;
    COUNTER cache_size;

    assert(cachedata);
    assert(comment);

    /* open the file or abort */
    if ((cachefd = open(cachefile, O_RDONLY)) == -1)
        errx(-1, "unable to open %s:%s", cachefile, strerror(errno));

    /* read the cache header and determine compatibility */
    if ((read_size = read(cachefd, &header, sizeof(header))) < 0)
        errx(-1, "unable to read from %s:%s,", cachefile, strerror(errno));

    if (read_size < (ssize_t)sizeof(header))
        errx(-1, "Cache file %s doesn't contain a full header", cachefile);

    /* verify our magic: tcpprep\0 */
    if (memcmp(header.magic, CACHEMAGIC, sizeof(CACHEMAGIC)) != 0)
        errx(-1, "Unable to process %s: not a tcpprep cache file", cachefile);

    /* verify version */
    if (strtol(header.version, NULL, 10) != strtol(CACHEVERSION, NULL, 10))
        errx(-1, "Unable to process %s: cache file version mismatch", cachefile);

    /* read the comment */
    header.comment_len = ntohs(header.comment_len);
    if (header.comment_len > 65534)
        errx(-1, "Unable to process %s: invalid comment length %u", cachefile, header.comment_len);

    *comment = (char *)safe_malloc(header.comment_len + 1);

    dbgx(1, "Comment length: %d", header.comment_len);

    if ((read_size = read(cachefd, *comment, header.comment_len)) < 0)
        errx(-1, "Error reading comment: %s", strerror(errno));

    if (read_size != (ssize_t)header.comment_len)
        errx(-1, "Invalid comment read: expected=%u actual=%zd bytes", header.comment_len, read_size);

    dbgx(1, "Cache file comment: %s", *comment);

    /* malloc our cache block */
    header.num_packets = ntohll(header.num_packets);
    header.packets_per_byte = ntohs(header.packets_per_byte);
    cache_size = header.num_packets / header.packets_per_byte;

    /* deal with any remainder, because above division is integer */
    if (header.num_packets % header.packets_per_byte)
        cache_size++;

    dbgx(1, "Cache file contains %" PRIu64 " packets in " COUNTER_SPEC " bytes", header.num_packets, cache_size);

    dbgx(1, "Cache uses %d packets per byte", header.packets_per_byte);

    *cachedata = (char *)safe_malloc(cache_size);

    /* read in the cache */
    if ((COUNTER)(read_size = read(cachefd, *cachedata, cache_size)) != cache_size)
        errx(-1,
             "Cache data length (%zu bytes) doesn't match "
             "cache header (" COUNTER_SPEC " bytes)",
             read_size,
             cache_size);

    dbgx(1, "Loaded in %" PRIu64 " packets from cache.", header.num_packets);

    close(cachefd);
    return (header.num_packets);
}

/**
 * writes out the cache file header, comment and then the
 * contents of *cachedata to out_file and then returns the number
 * of cache entries written
 */
COUNTER
write_cache(tcpr_cache_t *cachedata, const int out_file, COUNTER numpackets, char *comment)
{
    tcpr_cache_t *mycache = NULL;
    tcpr_cache_file_hdr_t *cache_header = NULL;
    uint32_t chars, last = 0;
    COUNTER packets = 0;
    ssize_t written;

    assert(out_file);

    /* write a header to our file */
    cache_header = (tcpr_cache_file_hdr_t *)safe_malloc(sizeof(tcpr_cache_file_hdr_t));
    strncpy(cache_header->magic, CACHEMAGIC, strlen(CACHEMAGIC) + 1);
    strncpy(cache_header->version, CACHEVERSION, strlen(CACHEVERSION) + 1);
    cache_header->packets_per_byte = htons(CACHE_PACKETS_PER_BYTE);
    cache_header->num_packets = htonll((u_int64_t)numpackets);

    /* we can't strlen(NULL) so ... */
    if (comment != NULL) {
        cache_header->comment_len = htons((uint16_t)strlen(comment));
    } else {
        cache_header->comment_len = 0;
    }

    written = write(out_file, cache_header, sizeof(tcpr_cache_file_hdr_t));
    dbgx(1, "Wrote %zu bytes of cache file header", written);

    if (written != sizeof(tcpr_cache_file_hdr_t))
        errx(-1,
             "Only wrote %zu of %zu bytes of the cache file header!\n%s",
             written,
             sizeof(tcpr_cache_file_hdr_t),
             written == -1 ? strerror(errno) : "");

    /* don't write comment if there is none */
    if (comment != NULL) {
        written = write(out_file, comment, strlen(comment));
        dbgx(1, "Wrote %zu bytes of comment", written);

        if (written != (ssize_t)strlen(comment))
            errx(-1,
                 "Only wrote %zu of %zu bytes of the comment!\n%s",
                 written,
                 strlen(comment),
                 written == -1 ? strerror(errno) : "");
    }

    if (cachedata) {
        mycache = cachedata;

        while (!last) {
            /* increment total packets */
            packets += mycache->packets;

            /* calculate how many chars to write */
            chars = mycache->packets / CACHE_PACKETS_PER_BYTE;
            if (mycache->packets % CACHE_PACKETS_PER_BYTE) {
                chars++;
                dbgx(1, "Bumping up to the next byte: %d %% %d", mycache->packets, CACHE_PACKETS_PER_BYTE);
            }

            /* write to file, and verify it wrote properly */
            written = write(out_file, mycache->data, chars);
            dbgx(1, "Wrote %zu bytes of cache data", written);
            if (written != (ssize_t)chars)
                errx(-1, "Only wrote %zu of %i bytes to cache file!", written, chars);

            /*
             * if that was the last, stop processing, otherwise wash,
             * rinse, repeat
             */
            if (mycache->next != NULL) {
                mycache = mycache->next;
            } else {
                last = 1;
            }
        }
    }
    safe_free(cache_header);
    /* return number of packets written */
    return (packets);
}

/**
 * mallocs a new CACHE struct all pre-set to sane defaults
 */

static tcpr_cache_t *
new_cache(void)
{
    tcpr_cache_t *newcache;

    /* malloc mem */
    newcache = (tcpr_cache_t *)safe_malloc(sizeof(tcpr_cache_t));
    return (newcache);
}

/**
 * adds the cache data for a packet to the given cachedata
 */

tcpr_dir_t
add_cache(tcpr_cache_t **cachedata, const int send, const tcpr_dir_t interface)
{
    static tcpr_cache_t *lastcache = NULL;
    tcpr_dir_t result;
#ifdef DEBUG
    char bitstring[9] = EIGHT_ZEROS;
#endif

    assert(cachedata);

    /* first run?  malloc our first entry, set bit count to 0 */
    if (*cachedata == NULL || lastcache == NULL) {
        *cachedata = new_cache();
        lastcache = *cachedata;
    } else {
        /* check to see if this is the last bit in this struct */
        if ((lastcache->packets + 1) > (CACHEDATASIZE * CACHE_PACKETS_PER_BYTE)) {
            /*
             * if so, we have to malloc a new one and set bit to 0
             */
            dbg(1, "Adding to cachedata linked list");
            lastcache->next = new_cache();
            lastcache = lastcache->next;
        }
    }

    /* always increment our bit count */
    lastcache->packets++;
    dbgx(1, "Cache array packet %d", lastcache->packets);

    /* send packet ? */
    if (send == SEND) {
        COUNTER index;
        uint32_t bit;
        u_char *byte;

        index = (lastcache->packets - 1) / (COUNTER)CACHE_PACKETS_PER_BYTE;
        bit = (((lastcache->packets - 1) % (COUNTER)CACHE_PACKETS_PER_BYTE) * (COUNTER)CACHE_BITS_PER_PACKET) + 1;
        dbgx(3, "Bit: %d", bit);

        byte = (u_char *)&lastcache->data[index];
        *byte += (u_char)(1 << bit);

        dbgx(2, "set send bit: byte " COUNTER_SPEC " = 0x%x", index, *byte);

        /* if true, set low order bit. else, do squat */
        if (interface == TCPR_DIR_C2S) {
            *byte += (u_char)(1 << (bit - 1));

            dbgx(2, "set interface bit: byte " COUNTER_SPEC " = 0x%x", index, *byte);
            result = TCPR_DIR_C2S;
        } else {
            dbgx(2, "don't set interface bit: byte " COUNTER_SPEC " = 0x%x", index, *byte);
            result = TCPR_DIR_S2C;
        }

#ifdef DEBUG
        /*
         * only build the byte string when not in debug mode since
         * the calculation is a bit expensive
         */
        dbgx(3, "Current cache byte: %c%c%c%c%c%c%c%c", BIT_STR(byte2bits(*byte, bitstring)));
#endif
    } else {
        dbg(1, "not setting send bit");
        result = TCPR_DIR_NOSEND;
    }

    return result;
}

/**
 * returns the action for a given packet based on the CACHE
 */
tcpr_dir_t
check_cache(char *cachedata, COUNTER packetid)
{
    COUNTER index;
    uint32_t bit;

    assert(cachedata);

    if (packetid == 0)
        err(-1, "packetid must be > 0");

    index = (packetid - 1) / (COUNTER)CACHE_PACKETS_PER_BYTE;
    bit = (uint32_t)(((packetid - 1) % (COUNTER)CACHE_PACKETS_PER_BYTE) * (COUNTER)CACHE_BITS_PER_PACKET) + 1;

#ifdef DEBUG
    dbgx(3,
         "Index: " COUNTER_SPEC "\tBit: %d\tByte: %hhu\tMask: %hhu",
         index,
         bit,
         cachedata[index],
         (uint8_t)(cachedata[index] & (char)(1 << bit)));
#endif

    if (!(cachedata[index] & (char)(1 << bit))) {
        return TCPR_DIR_NOSEND;
    }

    /* go back a bit to get the interface */
    bit--;
    if (cachedata[index] & (char)(1 << bit)) {
        return TCPR_DIR_C2S;
    } else {
        return TCPR_DIR_S2C;
    }
}
