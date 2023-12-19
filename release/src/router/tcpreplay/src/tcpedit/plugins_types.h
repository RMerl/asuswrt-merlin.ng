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


#pragma once

#include "defines.h"
#include "tcpr.h"
#include "tcpedit_types.h"

#ifdef __cplusplus
extern "C" {
#endif


/* 
 * Plugin Requires/Provides Bit Masks 
 * If you add any fields to the provides/requires bitmask,
 * then you also must add appropriate records for
 * tcpeditdlt_bit_map[] and tcpeditdlt_bit_info[]
 * in dlt_plugins.c
 */
typedef enum {
    PLUGIN_MASK_PROTO         = 0x01,
    PLUGIN_MASK_SRCADDR       = 0x02,
    PLUGIN_MASK_DSTADDR       = 0x04
} tcpeditdlt_bit_mask_t;

/* Union of all possible L2 address types */
typedef union {
    u_char ethernet[ETHER_ADDR_LEN]; /* ethernet is 6 bytes long */
    u_int8_t c_hdlc;                 /* Cisco HDLC is a single byte */
} tcpeditdlt_l2address_t;

/* What kind of address is the union? */
typedef enum {
    NONE,           /* DLT has no L2 address */
    ETHERNET,       /* support ethernet */
    C_HDLC,         /* Cisco HDLC uses a 1 byte addr which has only two values 0x0F & 0xBF */
} tcpeditdlt_l2addr_type_t;

/* src or dst mac */
typedef enum {
    SRC_MAC,
    DST_MAC
} tcpeditdlt_mac_type_t;

/* MAC address buffer length */
#define MAX_MAC_LEN 10

typedef struct tcpeditdlt_plugin_s tcpeditdlt_plugin_t;
typedef struct tcpeditdlt_s tcpeditdlt_t;

/* 
 * Each plugin must fill this out so that we know what function
 * to call from the external API
 */
struct tcpeditdlt_plugin_s {
    u_int16_t dlt;  /* dlt to register for */
    char *name;     /* plugin prefix name */
    struct tcpeditdlt_plugin_s *next; /* next in linked list */
    int requires; /* bit mask for which fields this plugin encoder requires */
    int provides; /* bit mask for which fields this plugin decoder provides */
    int (*plugin_init)(tcpeditdlt_t *);
    int (*plugin_post_init)(tcpeditdlt_t *);
    int (*plugin_cleanup)(tcpeditdlt_t *);
    int (*plugin_parse_opts)(tcpeditdlt_t *);
    int (*plugin_decode)(tcpeditdlt_t *, const u_char *, const int);
    int (*plugin_encode)(tcpeditdlt_t *, u_char *, int, tcpr_dir_t);
    int (*plugin_proto)(tcpeditdlt_t *, const u_char *, const int);
    int (*plugin_l2len)(tcpeditdlt_t *, const u_char *, const int);
    u_char *(*plugin_get_layer3)(tcpeditdlt_t *,  u_char *, const int);
    u_char *(*plugin_merge_layer3)(tcpeditdlt_t *, u_char *, const int, u_char *, u_char *);
    tcpeditdlt_l2addr_type_t (*plugin_l2addr_type)(void);
    u_char *(*plugin_get_mac)(tcpeditdlt_t *, tcpeditdlt_mac_type_t, const u_char *, const int);
    void *config; /* user configuration data for the encoder */
    size_t config_size;
};

/*
 * internal DLT plugin context
 */
struct tcpeditdlt_s {
    tcpedit_t *tcpedit;                 /* pointer to our tcpedit context */
#ifdef FORCE_ALIGN
    u_char *l3buff;                     /* pointer for L3 buffer on strictly aligned systems */
#endif
    tcpeditdlt_plugin_t *plugins;       /* registered plugins */
    tcpeditdlt_plugin_t *decoder;       /* Encoder plugin */
    tcpeditdlt_plugin_t *encoder;       /* Decoder plugin */      

    /* decoder validator tells us which kind of address we're processing */
    tcpeditdlt_l2addr_type_t addr_type;    

    /* skip rewriting IP/MAC's which are broadcast or multicast? */
    int skip_broadcast;
    
    /* original DLT */
    u_int16_t dlt;

    /*
     * These variables are filled out for each packet by the decoder
     */
    tcpeditdlt_l2address_t srcaddr;         /* filled out source address */
    tcpeditdlt_l2address_t dstaddr;         /* filled out dst address */
    int l2len;                              /* set by decoder and updated by encoder */
    int l2offset;                           /* offset to L2 - set by decoder and updated by encoder */
    u_int16_t proto;                        /* layer 3 proto type */
    u_int16_t proto_vlan_tag;               /* VLAN tag proto type */
    void *decoded_extra;                    /* any extra L2 data from decoder like VLAN tags */
    size_t decoded_extra_size;              /* size of decode_extra buffer */
    u_char srcmac[MAX_MAC_LEN];             /* buffers to store the src & dst MAC */
    u_char dstmac[MAX_MAC_LEN];
};



#ifdef __cplusplus
}
#endif

