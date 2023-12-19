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
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

static tcpr_cidr_t *cidr2cidr(char *);

/**
 * prints to the given fd all the entries in mycidr
 */
void
print_cidr(tcpr_cidr_t *mycidr)
{
    tcpr_cidr_t *cidr_ptr;

    fprintf(stderr, "Cidr List: ");

    cidr_ptr = mycidr;
    while (cidr_ptr != NULL) {
        /* print it */
        fprintf(stderr, "%s/%d, ", get_cidr2name(cidr_ptr, RESOLVE), cidr_ptr->masklen);

        /* go to the next */
        if (cidr_ptr->next != NULL) {
            cidr_ptr = cidr_ptr->next;
        } else {
            break;
        }
    }
    fprintf(stderr, "\n");
}

/**
 * deletes all entries in a cidr and destroys the datastructure
 */
void
destroy_cidr(tcpr_cidr_t *cidr)
{
    if (cidr != NULL) {
        if (cidr->next != NULL)
            destroy_cidr(cidr->next);

        safe_free(cidr);
    }
}

/**
 * adds a new tcpr_cidr_t entry to cidrdata
 */
void
add_cidr(tcpr_cidr_t **cidrdata, tcpr_cidr_t **newcidr)
{
    tcpr_cidr_t *cidr_ptr;
    dbg(1, "Running new_cidr()");

    if (*cidrdata == NULL) {
        *cidrdata = *newcidr;
    } else {
        cidr_ptr = *cidrdata;

        while (cidr_ptr->next != NULL)
            cidr_ptr = cidr_ptr->next;

        cidr_ptr->next = *newcidr;
    }
}

/**
 * Mallocs and sets to sane defaults a tcpr_cidr_t structure
 */

tcpr_cidr_t *
new_cidr(void)
{
    tcpr_cidr_t *newcidr;

    newcidr = (tcpr_cidr_t *)safe_malloc(sizeof(tcpr_cidr_t));

    memset(newcidr, '\0', sizeof(tcpr_cidr_t));
    newcidr->masklen = 99;
    newcidr->next = NULL;

    return (newcidr);
}

/**
 * Creates a new tcpr_cidrmap_t structure.  Malloc's memory
 */
tcpr_cidrmap_t *
new_cidr_map(void)
{
    tcpr_cidrmap_t *new;

    new = (tcpr_cidrmap_t *)safe_malloc(sizeof(tcpr_cidrmap_t));

    memset(new, '\0', sizeof(tcpr_cidrmap_t));
    new->next = NULL;

    return (new);
}

/**
 * Converts a single cidr (string) in the form of x.x.x.x/y into a
 * tcpr_cidr_t structure.  Will malloc the tcpr_cidr_t structure.
 */
static tcpr_cidr_t *
cidr2cidr(char *cidr)
{
    int count;
    unsigned int octets[4]; /* used in sscanf */
    tcpr_cidr_t *newcidr;
    char networkip[16], tempoctet[4];
    int family;
    char *p;

    assert(cidr);

    newcidr = new_cidr();

    for (p = cidr; *p; ++p) {
        if (*p == '#') {
            *p = ':';
        } else if (*p == ']') {
            *p = 0;
            break;
        }
    }

    /*
     * scan it, and make sure it scanned correctly, also copy over the
     * masklen
     */
    count = sscanf(cidr, "%u.%u.%u.%u/%d", &octets[0], &octets[1], &octets[2], &octets[3], &newcidr->masklen);

    if (count == 4) {
        newcidr->masklen = 32;
        family = AF_INET;
    } else if (count == 5) {
        family = AF_INET;
    } else {
        p = strstr(cidr, "/");
        if (p) {
            *p = 0;
            ++p;
            sscanf(p, "%d", &newcidr->masklen);
        } else {
            newcidr->masklen = 128;
        }

        if (newcidr->masklen < 0 || newcidr->masklen > 128)
            goto error;

        /* skip past the opening [ */
        if (*cidr == '[')
            cidr++;

        if (get_name2addr6(cidr, RESOLVE, &newcidr->u.network6) > 0) {
            family = AF_INET6;
        } else {
            goto error;
        }
    }

    if (family == AF_INET) {
        /* masklen better be 0 =< masklen <= 32 */
        if (newcidr->masklen > 32)
            goto error;

        /* copy in the ip address */
        memset(networkip, '\0', 16);
        for (count = 0; count < 4; count++) {
            if (octets[count] > 255)
                goto error;

            snprintf(tempoctet, sizeof(octets[count]), "%u", octets[count]);
            strcat(networkip, tempoctet);
            /* we don't want a '.' at the end of the last octet */
            if (count < 3)
                strcat(networkip, ".");
        }

        /* copy over the network address and return */
#ifdef HAVE_INET_ATON
        inet_aton(networkip, (struct in_addr *)&newcidr->u.network);
#elif HAVE_INET_ADDR
        newcidr->network = inet_addr(networkip);
#endif
    } else {
        /* Everything's done */
    }

    newcidr->family = family;
    return (newcidr);

    /* we only get here on error parsing input */
error:
    errx(-1, "%s: %s", "Unable to parse as a valid CIDR", cidr);
}

static void
mask_cidr6(char **cidrin, const char *delim)
{
    if (**cidrin == '[' && *delim == ':') {
        char *p;
        ++*cidrin;
        /* make strtok happy */
        for (p = *cidrin; *p && *p != ']'; ++p) {
            if (*p == ':')
                *p = '#';
        }
    }
}

/**
 * parses a list of tcpr_cidr_t's input from the user which should be in the form
 * of x.x.x.x/y,x.x.x.x/y...
 * returns 1 for success, or fails to return on failure (exit 1)
 * since we use strtok to process cidr, it gets zeroed out.
 */
int
parse_cidr(tcpr_cidr_t **cidrdata, char *cidrin, char *delim)
{
    tcpr_cidr_t *cidr_ptr; /* ptr to current cidr record */
    char *network;
    char *token = NULL;

    mask_cidr6(&cidrin, delim);

    /* first iteration of input using strtok */
    network = strtok_r(cidrin, delim, &token);
    if (network == NULL)
        return 0;

    *cidrdata = cidr2cidr(network);
    cidr_ptr = *cidrdata;

    /* do the same with the rest of the input */
    while (1) {
        if (token)
            mask_cidr6(&token, delim);

        network = strtok_r(NULL, delim, &token);
        /* if that was the last CIDR, then kickout */
        if (network == NULL)
            break;

        /* next record */
        cidr_ptr->next = cidr2cidr(network);
        cidr_ptr = cidr_ptr->next;
    }

    return 1;
}

/**
 * parses a pair of IP addresses: <IP1>:<IP2> and processes it like:
 * -N 0.0.0.0/0:<IP1> -N 0.0.0.0/0:<IP2>
 * returns 1 for success or returns 0 on failure
 * since we use strtok to process optarg, it gets zeroed out
 */
int
parse_endpoints(tcpr_cidrmap_t **cidrmap1, tcpr_cidrmap_t **cidrmap2, const char *optarg)
{
#define NEWMAP_LEN (INET6_ADDRSTRLEN * 2)
    char *map = NULL, newmap[NEWMAP_LEN];
    char *token = NULL;
    char *string;
    char *p;
    int res = 0;

    string = safe_strdup(optarg);

    if (*string == '[') {
        /* ipv6 mode */
        memset(newmap, '\0', NEWMAP_LEN);
        p = strstr(string, "]:[");
        if (!p)
            goto done;

        *p = 0;
        strlcpy(newmap, "[::/0]:", NEWMAP_LEN);
        strlcat(newmap, string, NEWMAP_LEN);
        strlcat(newmap, "]", NEWMAP_LEN);

        if (!parse_cidr_map(cidrmap1, newmap))
            goto done;

        /* do again with the second IP */
        memset(newmap, '\0', NEWMAP_LEN);
        strlcpy(newmap, "[::/0]:", NEWMAP_LEN);
        strlcat(newmap, p + 2, NEWMAP_LEN);

        if (!parse_cidr_map(cidrmap2, newmap))
            goto done;

    } else {
        /* ipv4 mode */
        memset(newmap, '\0', NEWMAP_LEN);
        map = strtok_r(string, ":", &token);
        if (map == NULL)
            goto done;

        strlcpy(newmap, "0.0.0.0/0:", NEWMAP_LEN);
        strlcat(newmap, map, NEWMAP_LEN);
        if (!parse_cidr_map(cidrmap1, newmap))
            goto done;

        /* do again with the second IP */
        memset(newmap, '\0', NEWMAP_LEN);
        map = strtok_r(NULL, ":", &token);

        strlcpy(newmap, "0.0.0.0/0:", NEWMAP_LEN);
        strlcat(newmap, map, NEWMAP_LEN);
        if (!parse_cidr_map(cidrmap2, newmap))
            goto done;
    }

    /* success */
    res = 1;

done:
    safe_free(string);
    return res;
}

/**
 * parses a list of tcpr_cidrmap_t's input from the user which should be in the form
 * of x.x.x.x/y:x.x.x.x/y,...
 * IPv6 syntax: [addr/y]:[addr/y],...
 * returns 1 for success, or returns 0 on failure
 * since we use strtok to process optarg, it gets zeroed out.
 */
int
parse_cidr_map(tcpr_cidrmap_t **cidrmap, const char *optarg)
{
    tcpr_cidr_t *cidr = NULL;
    char *map;
    char *token = NULL, *string;
    tcpr_cidrmap_t *ptr;
    int res = 0;

    string = safe_strdup(optarg);

    /* first iteration */
    map = strtok_r(string, ",", &token);
    if (!parse_cidr(&cidr, map, ":"))
        goto done;

    /* must return a linked list of two */
    if (cidr->next == NULL)
        goto done;

    /* copy over */
    *cidrmap = new_cidr_map();
    ptr = *cidrmap;

    ptr->from = cidr;
    ptr->to = cidr->next;
    ptr->from->next = NULL;

    /* do the same with the reset of the input */
    while (1) {
        map = strtok_r(NULL, ",", &token);
        if (map == NULL)
            break;

        if (!parse_cidr(&cidr, map, ":"))
            goto done;

        /* must return a linked list of two */
        if (cidr->next == NULL)
            goto done;

        /* copy over */
        ptr->next = new_cidr_map();
        ptr = ptr->next;
        ptr->from = cidr;
        ptr->to = cidr->next;
        ptr->from->next = NULL;
    }

    /* success */
    res = 1;

done:
    safe_free(string);
    return res;
}

/**
 * checks to see if the ip address is in the cidr
 * returns 1 for true, 0 for false
 */
int
ip_in_cidr(const tcpr_cidr_t *mycidr, const unsigned long ip)
{
    unsigned long ipaddr, network, mask;
    int ret;
#ifdef DEBUG
    char netstr[20];
#endif

    if (mycidr->family != AF_INET)
        return 0;

    /* always return 1 if 0.0.0.0/0 */
    if (mycidr->masklen == 0 && mycidr->u.network == 0)
        return 1;

    mask = ~0; /* turn on all the bits */

    /* shift over by the correct number of bits */
    mask = mask << (32 - mycidr->masklen);

    /* apply the mask to the network and ip */
    ipaddr = ntohl(ip) & mask;

    network = htonl(mycidr->u.network) & mask;

#ifdef DEBUG
    /* copy this for debug purposes, since it's not re-entrant */
    strlcpy(netstr, get_addr2name4(mycidr->u.network, RESOLVE), 20);
#endif

    /* if they're the same, then ip is in network */
    if (network == ipaddr) {
#ifdef DEBUG
        dbgx(1, "The ip %s is inside of %s/%d", get_addr2name4(ip, RESOLVE), netstr, mycidr->masklen);
#endif
        ret = 1;
    } else {
#ifdef DEBUG
        dbgx(1, "The ip %s is not inside of %s/%d", get_addr2name4(ip, RESOLVE), netstr, mycidr->masklen);
#endif
        ret = 0;
    }
    return ret;
}

static int
ip6_addr_is_unspec(const struct tcpr_in6_addr *addr)
{
    return addr->tcpr_s6_addr32[0] == 0 && addr->tcpr_s6_addr32[1] == 0 && addr->tcpr_s6_addr32[2] == 0 &&
           addr->tcpr_s6_addr32[3] == 0;
}

int
ip6_in_cidr(const tcpr_cidr_t *mycidr, const struct tcpr_in6_addr *addr)
{
    int ret;
#ifdef DEBUG
    char netstr[INET6_ADDRSTRLEN];
#endif
    uint32_t i, j, k;

    if (mycidr->family != AF_INET6)
        return 0;

    /* always return 1 if ::/0 */
    if (mycidr->masklen == 0 && ip6_addr_is_unspec(addr))
        return 1;

    j = mycidr->masklen / 8;

    for (i = 0; i < j; i++) {
        if (addr->tcpr_s6_addr[i] != mycidr->u.network6.tcpr_s6_addr[i]) {
            ret = 0;
            goto out;
        }
    }

    if ((k = mycidr->masklen % 8) == 0) {
        ret = 1;
        goto out;
    }

    k = (uint32_t)~0 << (8 - k);
    i = addr->tcpr_s6_addr[j] & k;
    j = mycidr->u.network6.tcpr_s6_addr[j] & k;
    ret = i == j;
out:

#ifdef DEBUG
    /* copy this for debug purposes, since it's not re-entrant */
    strlcpy(netstr, get_addr2name6(&mycidr->u.network6, RESOLVE), INET6_ADDRSTRLEN);
#endif

    /* if they're the same, then ip is in network */
    if (ret) {
#ifdef DEBUG
        dbgx(1, "The ip %s is inside of %s/%d", get_addr2name6(addr, RESOLVE), netstr, mycidr->masklen);
#endif
    } else {
#ifdef DEBUG
        dbgx(1, "The ip %s is not inside of %s/%d", get_addr2name6(addr, RESOLVE), netstr, mycidr->masklen);
#endif
    }
    return ret;
}

/**
 * iterates over cidrdata to find if a given ip matches
 * returns 1 for true, 0 for false
 */

int
check_ip_cidr(tcpr_cidr_t *cidrdata, const unsigned long ip)
{
    tcpr_cidr_t *mycidr;

    /* if we have no cidrdata, of course it isn't in there
     * this actually should happen occasionally, so don't put an assert here
     */
    if (cidrdata == NULL)
        return 1;

    mycidr = cidrdata;

    /* loop through cidr */
    while (1) {
        /* if match, return 1 */
        if (ip_in_cidr(mycidr, ip)) {
            dbgx(3, "Found %s in cidr", get_addr2name4(ip, RESOLVE));
            return 1;
        }

        /* check for next record */
        if (mycidr->next != NULL) {
            mycidr = mycidr->next;
        } else {
            break;
        }
    }

    /* if we get here, no match */
    dbgx(3, "Didn't find %s in cidr", get_addr2name4(ip, RESOLVE));
    return 0;
}

int
check_ip6_cidr(tcpr_cidr_t *cidrdata, const struct tcpr_in6_addr *addr)
{
    tcpr_cidr_t *mycidr;

    /* if we have no cidrdata, of course it isn't in there
     * this actually should happen occasionally, so don't put an assert here
     */
    if (cidrdata == NULL) {
        return 1;
    }

    mycidr = cidrdata;

    /* loop through cidr */
    while (1) {
        /* if match, return 1 */
        if (ip6_in_cidr(mycidr, addr)) {
            dbgx(3, "Found %s in cidr", get_addr2name6(addr, RESOLVE));
            return 1;
        }

        /* check for next record */
        if (mycidr->next != NULL) {
            mycidr = mycidr->next;
        } else {
            break;
        }
    }

    /* if we get here, no match */
    dbgx(3, "Didn't find %s in cidr", get_addr2name6(addr, RESOLVE));
    return 0;
}
