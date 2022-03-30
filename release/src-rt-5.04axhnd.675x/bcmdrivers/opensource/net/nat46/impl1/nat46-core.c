/* 
 * core nat46 functionality.
 * It does not know about network devices, modules or anything similar: 
 * those are abstracted away by other layers.
 *
 * Copyright (c) 2013-2014 Andrew Yourtchenko <ayourtch@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <net/route.h>
#include <linux/version.h>

#include "nat46-glue.h"
#include "nat46-core.h"
#include "nat46-module.h"

void
nat46debug_dump(nat46_instance_t *nat46, int level, void *addr, int len)
{
  char tohex[] = "0123456789ABCDEF";
  int i = 0;
  int k = 0;
  unsigned char *pc = addr;

  char buf0[32];                // offset
  char buf1[64];                // hex
  char buf2[64];                // literal

  char *pc1 = buf1;
  char *pc2 = buf2;

  while(--len >= 0) {
    if(i % 16 == 0) {
      for(k=0; k<9; k++) {
        buf0[k] = 0;
      }
      for(k=0; k<8; k++) {
        buf0[7-k] = tohex[ 0xf & (i >> k) ];
      }
      buf0[8] = 0;
      buf1[0] = 0;
      buf2[0] = 0;
      pc1 = buf1;
      pc2 = buf2;
    }
    *pc1++ = tohex[*pc >> 4];
    *pc1++ = tohex[*pc & 15];
    *pc1++ = ' ';

    if(*pc >= 32 && *pc < 127) {
      *pc2++ = *pc;
    } else {
      *pc2++ = '.';
    }
    i++;
    pc++;
    if(i % 16 == 0) {
      *pc1 = 0;
      *pc2 = 0;
      nat46debug(level, "%s:   %s  %s", buf0, buf1, buf2);
    }

  }
  if(i % 16 != 0) {
    while(i % 16 != 0) {
      *pc1++ = ' ';
      *pc1++ = ' ';
      *pc1++ = ' ';
      *pc2++ = ' ';
      i++;
    }
    *pc1 = 0;
    *pc2 = 0;
    nat46debug(level, "%s:   %s  %s", buf0, buf1, buf2);
  }
}

/* return the current arg, and advance the tail to the next space-separated word */
char *get_next_arg(char **ptail) {
  char *pc = NULL;
  while ((*ptail) && (**ptail) && ((**ptail == ' ') || (**ptail == '\n'))) {
    **ptail = 0;
    (*ptail)++;
  }
  pc = *ptail;

  while ((*ptail) && (**ptail) && ((**ptail != ' ') && (**ptail != '\n'))) {
    (*ptail)++;
  }

  while ((*ptail) && (**ptail) && ((**ptail == ' ') || (**ptail == '\n'))) {
    **ptail = 0;
    (*ptail)++;
  }

  if ((pc) && (0 == *pc)) {
    pc = NULL;
  }
  return pc;
}

/* 
 * Parse an IPv6 address (if pref_len is NULL), or prefix (if it isn't).
 * parses destructively (places \0 between address and prefix len)
 */
int try_parse_ipv6_prefix(struct in6_addr *pref, int *pref_len, char *arg) {
  int err = 0;
  char *arg_plen = strchr(arg, '/');
  if (arg_plen) {
    *arg_plen++ = 0;
    if (pref_len) {
      *pref_len = simple_strtol(arg_plen, NULL, 10);
    }
  }
  err = (1 != in6_pton(arg, -1, (u8 *)pref, '\0', NULL));
  return err;
}

int try_parse_ipv4_prefix(u32 *v4addr, int *pref_len, char *arg) {
  int err = 0;
  char *arg_plen = strchr(arg, '/');
  if (arg_plen) {
    *arg_plen++ = 0;
    if (pref_len) {
      *pref_len = simple_strtol(arg_plen, NULL, 10);
    }
  }
  err = (1 != in4_pton(arg, -1, (u8 *)v4addr, '/', NULL));
  return err;
}


/* 
 * parse a rule argument and put config into a rule.
 * advance the tail to prepare for the next arg parsing.
 * destructive.
 */

int try_parse_rule_arg(nat46_xlate_rule_t *rule, char *arg_name, char **ptail) {
  int err = 0;
  char *val = get_next_arg(ptail);
  if (NULL == val) {
    err = -1;
  } else if (0 == strcmp(arg_name, "v6")) {
    err = try_parse_ipv6_prefix(&rule->v6_pref, &rule->v6_pref_len, val);
  } else if (0 == strcmp(arg_name, "v4")) {
    err = try_parse_ipv4_prefix(&rule->v4_pref, &rule->v4_pref_len, val);
  } else if (0 == strcmp(arg_name, "ea-len")) {
    rule->ea_len = simple_strtol(val, NULL, 10);
  } else if (0 == strcmp(arg_name, "psid-offset")) {
    rule->psid_offset = simple_strtol(val, NULL, 10);
  } else if (0 == strcmp(arg_name, "style")) {
    if (0 == strcmp("MAP", val)) {
      rule->style = NAT46_XLATE_MAP;
    } else if (0 == strcmp("MAP0", val)) {
      rule->style = NAT46_XLATE_MAP0;
    } else if (0 == strcmp("RFC6052", val)) {
      rule->style = NAT46_XLATE_RFC6052;
    } else if (0 == strcmp("NONE", val)) {
      rule->style = NAT46_XLATE_NONE;
    } else {
      err = 1;
    }
  }
  return err;
}

/* 
 * Parse the config commands in the buffer, 
 * destructive (puts zero between the args) 
 */

int nat46_set_ipair_config(nat46_instance_t *nat46, int ipair, char *buf, int count) {
  char *tail = buf;
  char *arg_name;
  int err = 0;
  char *val;
  nat46_xlate_rulepair_t *apair = NULL;

  if ((ipair < 0) || (ipair >= nat46->npairs)) {
    return -1;
  }

  apair = &nat46->pairs[ipair];

  while ((0 == err) && (NULL != (arg_name = get_next_arg(&tail)))) {
    if (0 == strcmp(arg_name, "debug")) {
      val = get_next_arg(&tail);
      if (val) {
        nat46->debug = simple_strtol(val, NULL, 10);
      }
    } else if (arg_name == strstr(arg_name, "local.")) {
      arg_name += strlen("local.");
      nat46debug(13, "Setting local xlate parameter");
      err = try_parse_rule_arg(&apair->local, arg_name, &tail);
    } else if (arg_name == strstr(arg_name, "remote.")) {
      arg_name += strlen("remote.");
      nat46debug(13, "Setting remote xlate parameter");
      err = try_parse_rule_arg(&apair->remote, arg_name, &tail);
    }
  }
  return err;
}

int nat46_set_config(nat46_instance_t *nat46, char *buf, int count) {
  int ret = -1;
  if (nat46->npairs > 0) {
    ret = nat46_set_ipair_config(nat46, nat46->npairs-1, buf, count);
  }
  return ret;
}

char *xlate_style_to_string(nat46_xlate_style_t style) {
  switch(style) {
    case NAT46_XLATE_NONE:
      return "NONE";
    case NAT46_XLATE_MAP:
      return "MAP";
    case NAT46_XLATE_MAP0:
      return "MAP0";
    case NAT46_XLATE_RFC6052:
      return "RFC6052";
  }
  return "unknown";
}

/* 
 * Get the nat46 configuration into a supplied buffer (if non-null).
 */
int nat46_get_ipair_config(nat46_instance_t *nat46, int ipair, char *buf, int count) {
  int ret = 0;
  nat46_xlate_rulepair_t *apair = NULL;
  char *format = "local.v4 %pI4/%d local.v6 %pI6c/%d local.style %s local.ea-len %d local.psid-offset %d remote.v4 %pI4/%d remote.v6 %pI6c/%d remote.style %s remote.ea-len %d remote.psid-offset %d debug %d";

  if ((ipair < 0) || (ipair >= nat46->npairs)) {
    return ret;
  }
  apair = &nat46->pairs[ipair];

  ret = snprintf(buf, count, format,
		&apair->local.v4_pref, apair->local.v4_pref_len,
		&apair->local.v6_pref, apair->local.v6_pref_len,
		xlate_style_to_string(apair->local.style),
		apair->local.ea_len, apair->local.psid_offset,

		&apair->remote.v4_pref, apair->remote.v4_pref_len,
		&apair->remote.v6_pref, apair->remote.v6_pref_len,
		xlate_style_to_string(apair->remote.style),
		apair->remote.ea_len, apair->remote.psid_offset,

		nat46->debug);
  return ret;
}

int nat46_get_config(nat46_instance_t *nat46, char *buf, int count) {
  int ret = 0;
  if (nat46->npairs > 0) {
    ret = nat46_get_ipair_config(nat46, nat46->npairs-1, buf, count);
  } else {
    nat46debug(0, "nat46_get_config: npairs is 0");
  }
  return ret;
}


/********************************************************************

From RFC6052, section 2.2:

    +--+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
    |PL| 0-------------32--40--48--56--64--72--80--88--96--104---------|
    +--+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
    |32|     prefix    |v4(32)         | u | suffix                    |
    +--+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
    |40|     prefix        |v4(24)     | u |(8)| suffix                |
    +--+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
    |48|     prefix            |v4(16) | u | (16)  | suffix            |
    +--+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
    |56|     prefix                |(8)| u |  v4(24)   | suffix        |
    +--+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
    |64|     prefix                    | u |   v4(32)      | suffix    |
    +--+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
    |96|     prefix                                    |    v4(32)     |
    +--+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

********************************************************************/

void xlate_v4_to_nat64(nat46_instance_t *nat46, nat46_xlate_rule_t *rule, void *pipv4, void *pipv6) {
  char *ipv4 = pipv4;
  char *ipv6 = pipv6;

  /* 'u' byte and suffix are zero */
  memset(&ipv6[8], 0, 8);
  switch(rule->v6_pref_len) {
    case 32:
      memcpy(ipv6, &rule->v6_pref, 4);
      memcpy(&ipv6[4], ipv4, 4);
      break;
    case 40:
      memcpy(ipv6, &rule->v6_pref, 5);
      memcpy(&ipv6[5], ipv4, 3);
      ipv6[9] = ipv4[3];
      break;
    case 48:
      memcpy(ipv6, &rule->v6_pref, 6);
      ipv6[6] = ipv4[0];
      ipv6[7] = ipv4[1];
      ipv6[9] = ipv4[2];
      ipv6[10] = ipv4[3];
      break;
    case 56:
      memcpy(ipv6, &rule->v6_pref, 7);
      ipv6[7] = ipv4[0];
      ipv6[9] = ipv4[1];
      ipv6[10] = ipv4[2];
      ipv6[11] = ipv4[3];
      break;
    case 64:
      memcpy(ipv6, &rule->v6_pref, 8);
      memcpy(&ipv6[9], ipv4, 4);
      break;
    case 96:
      memcpy(ipv6, &rule->v6_pref, 12);
      memcpy(&ipv6[12], ipv4, 4);
      break;
  }
}

int xlate_nat64_to_v4(nat46_instance_t *nat46, nat46_xlate_rule_t *rule, void *pipv6, void *pipv4) {
  char *ipv4 = pipv4;
  char *ipv6 = pipv6;
  int cmp = -1;
  int v6_pref_len = rule->v6_pref_len;

  switch(v6_pref_len) {
    case 32:
      cmp = memcmp(ipv6, &rule->v6_pref, 4);
      break;
    case 40:
      cmp = memcmp(ipv6, &rule->v6_pref, 5);
      break;
    case 48:
      cmp = memcmp(ipv6, &rule->v6_pref, 6);
      break;
    case 56:
      cmp = memcmp(ipv6, &rule->v6_pref, 7);
      break;
    case 64:
      cmp = memcmp(ipv6, &rule->v6_pref, 8);
      break;
    case 96:
      cmp = memcmp(ipv6, &rule->v6_pref, 12);
      break;
  }
  if (cmp) {
    /* Not in NAT64 prefix */
    return 0;
  }
  switch(v6_pref_len) {
    case 32:
      memcpy(ipv4, &ipv6[4], 4);
      break;
    case 40:
      memcpy(ipv4, &ipv6[5], 3);
      ipv4[3] = ipv6[9];
      break;
    case 48:
      ipv4[0] = ipv6[6];
      ipv4[1] = ipv6[7];
      ipv4[2] = ipv6[9];
      ipv4[3] = ipv6[10];
      break;
    case 56:
      ipv4[0] = ipv6[7];
      ipv4[1] = ipv6[9];
      ipv4[2] = ipv6[10];
      ipv4[3] = ipv6[11];
      break;
    case 64:
      memcpy(ipv4, &ipv6[9], 4);
      break;
    case 96:
      memcpy(ipv4, &ipv6[12], 4);
      break;
  }
  return 1;
}

/*

The below bitarray copy code is from

http://stackoverflow.com/questions/3534535/whats-a-time-efficient-algorithm-to-copy-unaligned-bit-arrays

*/

#define CHAR_BIT 8
#define PREPARE_FIRST_COPY()                                      \
    do {                                                          \
    if (src_len >= (CHAR_BIT - dst_offset_modulo)) {              \
        *dst     &= reverse_mask[dst_offset_modulo];              \
        src_len -= CHAR_BIT - dst_offset_modulo;                  \
    } else {                                                      \
        *dst     &= reverse_mask[dst_offset_modulo]               \
              | reverse_mask_xor[dst_offset_modulo + src_len + 1];\
         c       &= reverse_mask[dst_offset_modulo + src_len    ];\
        src_len = 0;                                              \
    } } while (0)


static void
bitarray_copy(const void *src_org, int src_offset, int src_len,
                    void *dst_org, int dst_offset)
{
/*
    static const unsigned char mask[] =
        { 0x55, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff };
*/
    static const unsigned char reverse_mask[] =
        { 0x55, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff };
    static const unsigned char reverse_mask_xor[] =
        { 0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01, 0x00 };

    if (src_len) {
        const unsigned char *src;
              unsigned char *dst;
        int                  src_offset_modulo,
                             dst_offset_modulo;

        src = src_org + (src_offset / CHAR_BIT);
        dst = dst_org + (dst_offset / CHAR_BIT);

        src_offset_modulo = src_offset % CHAR_BIT;
        dst_offset_modulo = dst_offset % CHAR_BIT;

        if (src_offset_modulo == dst_offset_modulo) {
            int              byte_len;
            int              src_len_modulo;
            if (src_offset_modulo) {
                unsigned char   c;

                c = reverse_mask_xor[dst_offset_modulo]     & *src++;

                PREPARE_FIRST_COPY();
                *dst++ |= c;
            }

            byte_len = src_len / CHAR_BIT;
            src_len_modulo = src_len % CHAR_BIT;

            if (byte_len) {
                memcpy(dst, src, byte_len);
                src += byte_len;
                dst += byte_len;
            }
            if (src_len_modulo) {
                *dst     &= reverse_mask_xor[src_len_modulo];
                *dst |= reverse_mask[src_len_modulo]     & *src;
            }
        } else {
            int             bit_diff_ls,
                            bit_diff_rs;
            int             byte_len;
            int             src_len_modulo;
            unsigned char   c;
            /*
             * Begin: Line things up on destination.
             */
            if (src_offset_modulo > dst_offset_modulo) {
                bit_diff_ls = src_offset_modulo - dst_offset_modulo;
                bit_diff_rs = CHAR_BIT - bit_diff_ls;

                c = *src++ << bit_diff_ls;
                c |= *src >> bit_diff_rs;
                c     &= reverse_mask_xor[dst_offset_modulo];
            } else {
                bit_diff_rs = dst_offset_modulo - src_offset_modulo;
                bit_diff_ls = CHAR_BIT - bit_diff_rs;

                c = *src >> bit_diff_rs     &
                    reverse_mask_xor[dst_offset_modulo];
            }
            PREPARE_FIRST_COPY();
            *dst++ |= c;

            /*
             * Middle: copy with only shifting the source.
             */
            byte_len = src_len / CHAR_BIT;

            while (--byte_len >= 0) {
                c = *src++ << bit_diff_ls;
                c |= *src >> bit_diff_rs;
                *dst++ = c;
            }

            /*
             * End: copy the remaining bits;
             */
            src_len_modulo = src_len % CHAR_BIT;
            if (src_len_modulo) {
                c = *src++ << bit_diff_ls;
                c |= *src >> bit_diff_rs;
                c     &= reverse_mask[src_len_modulo];

                *dst     &= reverse_mask_xor[src_len_modulo];
                *dst |= c;
            }
        }
    }
}

int xlate_map_v4_to_v6(nat46_instance_t *nat46, nat46_xlate_rule_t *rule, void *pipv4, void *pipv6, uint16_t *pl4id, int map_version) {
  int ret = 0;
  u32 *pv4u32 = pipv4;
  uint8_t *p6 = pipv6;

  uint16_t psid;
  uint16_t l4id = pl4id ? *pl4id : 0;
  uint8_t psid_bits_len = rule->ea_len - (32 - rule->v4_pref_len);
  uint8_t v4_lsb_bits_len = 32 - rule->v4_pref_len;

  /* check that the ipv4 address is within the IPv4 map domain and reject if not */

  if ( (ntohl(*pv4u32) & (0xffffffff << v4_lsb_bits_len)) != ntohl(rule->v4_pref) ) {
    nat46debug(5, "xlate_map_v4_to_v6: IPv4 address %pI4 outside of MAP domain %pI4/%d", pipv4, &rule->v4_pref, rule->v4_pref_len);
    return 0;
  }

  if (rule->ea_len < (32 - rule->v4_pref_len) ) {
    nat46debug(0, "xlate_map_v4_to_v6: rule->ea_len < (32 - rule->v4_pref_len)");
    return 0;
  }

  if (!pl4id && psid_bits_len) {
    nat46debug(5, "xlate_map_v4_to_v6: l4id required for MAP domain %pI4/%d (ea-len %d)", &rule->v4_pref, rule->v4_pref_len, rule->ea_len);
    return 0;
  }

  /* zero out the IPv6 address */
  memset(pipv6, 0, 16);

  psid = (ntohs(l4id) >> (16 - psid_bits_len - rule->psid_offset)) & (0xffff >> (16 - psid_bits_len));
  nat46debug(10, "xlate_map_v4_to_v6: ntohs(l4id): %04x psid_bits_len: %d, rule psid-offset: %d, psid: %d\n", ntohs(l4id), psid_bits_len, rule->psid_offset, psid);

  /*
   *     create the IID. pay the attention there can be two formats:
   *
   *     draft-ietf-softwire-map-t-00:
   *
   *
   *   +--+---+---+---+---+---+---+---+---+
   *   |PL|   8  16  24  32  40  48  56   |
   *   +--+---+---+---+---+---+---+---+---+
   *   |64| u | IPv4 address  |  PSID | 0 |
   *   +--+---+---+---+---+---+---+---+---+
   *
   *
   *     latest draft-ietf-softwire-map-t:
   *
   *   |        128-n-o-s bits            |
   *   | 16 bits|    32 bits     | 16 bits|
   *   +--------+----------------+--------+
   *   |   0    |  IPv4 address  |  PSID  |
   *   +--------+----------------+--------+
   *
   *   In the case of an IPv4 prefix, the IPv4 address field is right-padded
   *   with zeros up to 32 bits.  The PSID is zero left-padded to create a
   *   16 bit field.  For an IPv4 prefix or a complete IPv4 address, the
   *   PSID field is zero.
   *
   *   If the End-user IPv6 prefix length is larger than 64, the most
   *   significant parts of the interface identifier is overwritten by the
   *   prefix.
   *
   */
  if (map_version) {
    p6[8] = p6[9] = 0;
    p6[10] = 0xff & (ntohl(*pv4u32) >> 24);
    p6[11] = 0xff & (ntohl(*pv4u32) >> 16);
    p6[12] = 0xff & (ntohl(*pv4u32) >> 8);
    p6[13] = 0xff & (ntohl(*pv4u32));
    p6[14] = 0xff & (psid >> 8);
    p6[15] = 0xff & (psid);
  } else {
    p6[8]  = 0;
    p6[9]  = 0xff & (ntohl(*pv4u32) >> 24);
    p6[10] = 0xff & (ntohl(*pv4u32) >> 16);
    p6[11] = 0xff & (ntohl(*pv4u32) >> 8);
    p6[12] = 0xff & (ntohl(*pv4u32));
    p6[13] = 0xff & (psid >> 8);
    p6[14] = 0xff & (psid);
    p6[15] = 0;
    /* old EID */
  }

  /* copy the necessary part of domain IPv6 prefix into place, w/o overwriting the existing data */
  bitarray_copy(&rule->v6_pref, 0, rule->v6_pref_len, p6, 0);

  if (v4_lsb_bits_len) {
    /* insert the lower 32-v4_pref_len bits of IPv4 address at rule->v6_pref_len */
    bitarray_copy(pipv4, rule->v4_pref_len, v4_lsb_bits_len, p6, rule->v6_pref_len);
  }

  if (psid_bits_len) {
    /* insert the psid bits at rule->v6_pref_len + v4_lsb_bits */
    bitarray_copy(&l4id, rule->psid_offset, psid_bits_len, p6, rule->v6_pref_len + v4_lsb_bits_len);
  }

  ret = 1;

  return ret;
}

int xlate_map_v6_to_v4(nat46_instance_t *nat46, nat46_xlate_rule_t *rule, void *pipv6, void *pipv4, int version) {
  uint8_t v4_lsb_bits_len = 32 - rule->v4_pref_len;

  if (memcmp(pipv6, &rule->v6_pref, rule->v6_pref_len/8)) {
    /* address not within the MAP IPv6 prefix */
    nat46debug(5, "xlate_map_v6_to_v4: IPv6 address %pI6 outside of MAP domain %pI6/%d", pipv6, &rule->v6_pref, rule->v6_pref_len);
    return 0;
  }
  if (rule->v6_pref_len % 8) {
    uint8_t mask = 0xff << (8 - (rule->v6_pref_len % 8));
    uint8_t *pa1 = (uint8_t *)pipv6 + (rule->v6_pref_len/8);
    uint8_t *pa2 = (uint8_t *)&rule->v6_pref + (rule->v6_pref_len/8);

    if ( (*pa1 & mask) != (*pa2 & mask) ) {
      nat46debug(5, "xlate_map_v6_to_v4: IPv6 address %pI6 outside of MAP domain %pI6/%d (LSB)", pipv6, &rule->v6_pref, rule->v6_pref_len);
      return 0;
    }
  }

  if (rule->ea_len < (32 - rule->v4_pref_len) ) {
    nat46debug(0, "xlate_map_v6_to_v4: rule->ea_len < (32 - rule->v4_pref_len)");
    return 0;
  }

  memcpy(pipv4, &rule->v4_pref, 4);
  if (v4_lsb_bits_len) {
    bitarray_copy(pipv6, rule->v6_pref_len, v4_lsb_bits_len, pipv4, rule->v4_pref_len);
  }
  /*
   * I do not verify the PSID here. The idea is that if the destination port is incorrect, this
   * will be caught in the NAT44 module.
   */
  return 1;
}

int xlate_v4_to_v6(nat46_instance_t *nat46, nat46_xlate_rule_t *rule, void *pipv4, void *pipv6, uint16_t *pl4id) {
  int ret = 0;
  switch(rule->style) {
    case NAT46_XLATE_NONE: /* always fail unless it is a host 1:1 translation */
      if ( (rule->v6_pref_len == 128) && (rule->v4_pref_len == 32) &&
           (0 == memcmp(pipv4, &rule->v4_pref, sizeof(rule->v4_pref))) ) {
         memcpy(pipv6, &rule->v6_pref, sizeof(rule->v6_pref));
         ret = 1;
      }
      break;
    case NAT46_XLATE_MAP0:
      ret = xlate_map_v4_to_v6(nat46, rule, pipv4, pipv6, pl4id, 0);
      break;
    case NAT46_XLATE_MAP:
      ret = xlate_map_v4_to_v6(nat46, rule, pipv4, pipv6, pl4id, 1);
      break;
    case NAT46_XLATE_RFC6052:
      xlate_v4_to_nat64(nat46, rule, pipv4, pipv6);
      /* NAT46 rules using RFC6052 always succeed since they can map any IPv4 address */
      ret = 1;
      break;
  }
  return ret;
}

int xlate_v6_to_v4(nat46_instance_t *nat46, nat46_xlate_rule_t *rule, void *pipv6, void *pipv4) {
  int ret = 0;
  switch(rule->style) {
    case NAT46_XLATE_NONE: /* always fail unless it is a host 1:1 translation */
      if ( (rule->v6_pref_len == 128) && (rule->v4_pref_len == 32) &&
           (0 == memcmp(pipv6, &rule->v6_pref, sizeof(rule->v6_pref))) ) {
         memcpy(pipv4, &rule->v4_pref, sizeof(rule->v4_pref));
         ret = 1;
      }
      break;
    case NAT46_XLATE_MAP0:
      ret = xlate_map_v6_to_v4(nat46, rule, pipv6, pipv4, 0);
      break;
    case NAT46_XLATE_MAP:
      ret = xlate_map_v6_to_v4(nat46, rule, pipv6, pipv4, 1);
      break;
    case NAT46_XLATE_RFC6052:
      ret = xlate_nat64_to_v4(nat46, rule, pipv6, pipv4);
      break;
  }
  return ret;
}

__sum16 csum16_upd(__sum16 csum, u16 old, u16 new) {
  u32 s;
  csum = ntohs(~csum);
  s = (u32)csum + ntohs(~old) + ntohs(new);
  s = ((s >> 16) & 0xffff) + (s & 0xffff);
  s += ((s >> 16) & 0xffff);
  return htons((u16)(~s));
}

/* Add the TCP/UDP pseudoheader, basing on the existing checksum */

__sum16 csum_tcpudp_remagic(__be32 saddr, __be32 daddr, unsigned short len,
                  unsigned char proto, u16 csum) {
  u16 *pdata;
  u16 len0, len1;

  pdata = (u16 *)&saddr;
  csum = csum16_upd(csum, 0, *pdata++);
  csum = csum16_upd(csum, 0, *pdata++);
  pdata = (u16 *)&daddr;
  csum = csum16_upd(csum, 0, *pdata++);
  csum = csum16_upd(csum, 0, *pdata++);

  csum = csum16_upd(csum, 0, htons(proto));
  len1 = htons( (len >> 16) & 0xffff );
  len0 = htons(len & 0xffff);
  csum = csum16_upd(csum, 0, len1);
  csum = csum16_upd(csum, 0, len0);
  return csum;
}

/* Undo the IPv6 pseudoheader inclusion into the checksum */
__sum16 csum_ipv6_unmagic(nat46_instance_t *nat46, const struct in6_addr *saddr,
                        const struct in6_addr *daddr,
                        __u32 len, unsigned short proto,
                        __sum16 csum) {
  u16 *pdata;
  int i;
  u16 len0, len1;

  pdata = (u16 *)saddr;
  for(i=0;i<8;i++) {
    csum = csum16_upd(csum, *pdata, 0);
    pdata++;
  }
  pdata = (u16 *)daddr;
  for(i=0;i<8;i++) {
    csum = csum16_upd(csum, *pdata, 0);
    pdata++;
  }
  csum = csum16_upd(csum, htons(proto), 0);
  len1 = htons( (len >> 16) & 0xffff );
  len0 = htons(len & 0xffff);
  csum = csum16_upd(csum, len1, 0);
  csum = csum16_upd(csum, len0, 0);
  return csum;
}

/* Update ICMPv6 type/code with incremental checksum adjustment */
void update_icmp6_type_code(nat46_instance_t *nat46, struct icmp6hdr *icmp6h, u8 type, u8 code) {
  u16 old_tc = *((u16 *)icmp6h);
  u16 new_tc;
  u16 old_csum = icmp6h->icmp6_cksum;
  u16 new_csum;
  icmp6h->icmp6_type = type;
  icmp6h->icmp6_code = code;
  new_tc = *((u16 *)icmp6h);
  /* https://tools.ietf.org/html/rfc1624 */
  new_csum = csum16_upd(old_csum, old_tc, new_tc);
  nat46debug(1, "Updating the ICMPv6 type to ICMP type %d and code to %d. Old T/C: %04X, New T/C: %04X, Old CS: %04X, New CS: %04X", type, code, old_tc, new_tc, old_csum, new_csum);
  icmp6h->icmp6_cksum = new_csum;
}


u16 get_next_ip_id(void) {
  static u16 id = 0;
  return id++;
}

u16 fold_ipv6_frag_id(u32 v6id) {
  return ((0xffff & (v6id >> 16)) ^ (v6id & 0xffff));
}

void *add_offset(void *ptr, u16 offset) {
  return (((char *)ptr)+offset);
}


/* FIXME: traverse the headers properly */
void *get_next_header_ptr6(void *pv6, int v6_len) {
  struct ipv6hdr *ip6h = pv6;
  void *ret = (ip6h+1);

  if (ip6h->nexthdr == NEXTHDR_FRAGMENT) {
    struct frag_hdr *fh = (struct frag_hdr*)(ip6h + 1);
    if(fh->frag_off == 0) {
      /* Atomic fragment */
      ret = add_offset(ret, 8);
    }
  }
  return ret;
}

void fill_v4hdr_from_v6hdr(struct iphdr * iph, struct ipv6hdr *ip6h, __u32 v4saddr, __u32 v4daddr, __u16 id, __u16 frag_off, __u16 proto, int l3_payload_len) {
  iph->ttl = ip6h->hop_limit;
  iph->saddr = v4saddr;
  iph->daddr = v4daddr;
  iph->protocol = proto;
  *((__be16 *)iph) = htons((4 << 12) | (5 << 8) | (0x00/*tos*/ & 0xff));
  iph->frag_off = frag_off;
  iph->id = id;
  iph->tot_len = htons( l3_payload_len + IPV4HDRSIZE );
  iph->check = 0;
  iph->check = ip_fast_csum((unsigned char *)iph, iph->ihl);
}

u16 unchecksum16(void *p, int count, u16 csum) {
  u16 *pu16 = p;
  int i = count;
  while(i--) {
    csum = csum16_upd(csum, *pu16++, 0);
  }
  return csum;
}

u16 rechecksum16(void *p, int count, u16 csum) {
  u16 *pu16 = p;
  int i = count;
  while(i--) {
    csum = csum16_upd(csum, 0, *pu16++);
  }
  return csum;
}

/* Last rule in group must not have "none" as either source or destination */
int is_last_pair_in_group(nat46_xlate_rulepair_t *apair) {
  return ( (apair->local.style != NAT46_XLATE_NONE) && (apair->remote.style != NAT46_XLATE_NONE) );
}

void pairs_xlate_v6_to_v4_inner(nat46_instance_t *nat46, struct ipv6hdr *ip6h, __u32 *pv4saddr, __u32 *pv4daddr) {
  int ipair = 0;
  nat46_xlate_rulepair_t *apair = NULL;
  int xlate_src = -1;
  int xlate_dst = -1;

  for(ipair = 0; ipair < nat46->npairs; ipair++) {
    apair = &nat46->pairs[ipair];

    if(-1 == xlate_dst) {
      if(xlate_v6_to_v4(nat46, &apair->remote, &ip6h->daddr, pv4daddr)) {
        xlate_dst = ipair;
      }
    }
    if(-1 == xlate_src) {
      if(xlate_v6_to_v4(nat46, &apair->local, &ip6h->saddr, pv4saddr)) {
        xlate_src = ipair;
      }
    }
    if((xlate_src >= 0) && (xlate_dst >= 0)) {
      /* we did manage to translate it */
      break;
    } else {
      /* We did not match fully and there are more rules */
      if((ipair+1 < nat46->npairs) && is_last_pair_in_group(apair)) {
        xlate_src = -1;
        xlate_dst = -1;
      }
    }
  }
  nat46debug(5, "[nat46payload] xlate results: src %d dst %d", xlate_src, xlate_dst);
}

/*
 * pv6 is pointing to the ipv6 header inside the payload.
 * Translate this header and attempt to extract the sport/dport
 * so the callers can use them for translation as well.
 */
int xlate_payload6_to4(nat46_instance_t *nat46, void *pv6, void *ptrans_hdr, int v6_len, u16 *ul_sum, int *ptailTruncSize) {
  struct ipv6hdr *ip6h = pv6;
  __u32 v4saddr, v4daddr;
  struct iphdr new_ipv4;
  struct iphdr *iph = &new_ipv4;
  u16 proto = ip6h->nexthdr;
  u16 ipid = 0;
  u16 ipflags = htons(IP_DF);
  int infrag_payload_len = ntohs(ip6h->payload_len);

  /*
   * The packet is supposedly our own packet after translation - so the rules
   * will be swapped compared to translation of the outer packet
   */
  pairs_xlate_v6_to_v4_inner(nat46, pv6, &v4saddr, &v4daddr);

  if (proto == NEXTHDR_FRAGMENT) {
    struct frag_hdr *fh = (struct frag_hdr*)(ip6h + 1);
    if(fh->frag_off == 0) {
      /* Atomic fragment */
      proto = fh->nexthdr;
      ipid = fold_ipv6_frag_id(fh->identification);
      v6_len -= 8;
      infrag_payload_len -= 8;
      *ptailTruncSize += 8;
      ipflags = 0;
    }
  }


  switch(proto) {
    case NEXTHDR_TCP: {
      struct tcphdr *th = ptrans_hdr;
      u16 sum1 = csum_ipv6_unmagic(nat46, &ip6h->saddr, &ip6h->daddr, infrag_payload_len, NEXTHDR_TCP, th->check);
      u16 sum2 = csum_tcpudp_remagic(v4saddr, v4daddr, infrag_payload_len, NEXTHDR_TCP, sum1); /* add pseudoheader */
      if(ul_sum) {
        *ul_sum = csum16_upd(*ul_sum, th->check, sum2);
        }
      th->check = sum2;
      break;
      }
    case NEXTHDR_UDP: {
      struct udphdr *udp = ptrans_hdr;
      u16 sum1 = csum_ipv6_unmagic(nat46, &ip6h->saddr, &ip6h->daddr, infrag_payload_len, NEXTHDR_UDP, udp->check);
      u16 sum2 = csum_tcpudp_remagic(v4saddr, v4daddr, infrag_payload_len, NEXTHDR_UDP, sum1); /* add pseudoheader */
      if(ul_sum) {
        *ul_sum = csum16_upd(*ul_sum, udp->check, sum2);
        }
      udp->check = sum2;
      break;
      }
    case NEXTHDR_ICMP: {
      struct icmp6hdr *icmp6h = ptrans_hdr;
      u16 sum0 = icmp6h->icmp6_cksum;
      u16 sum1 = csum_ipv6_unmagic(nat46, &ip6h->saddr, &ip6h->daddr, infrag_payload_len, NEXTHDR_ICMP, icmp6h->icmp6_cksum);
      if(ul_sum) {
        *ul_sum = csum16_upd(*ul_sum, sum0, sum1);
        }
      icmp6h->icmp6_cksum = sum1;
      proto = IPPROTO_ICMP;
      switch(icmp6h->icmp6_type) {
        case ICMPV6_ECHO_REQUEST:
          update_icmp6_type_code(nat46, icmp6h, ICMP_ECHO, icmp6h->icmp6_code);
          break;
        case ICMPV6_ECHO_REPLY:
          update_icmp6_type_code(nat46, icmp6h, ICMP_ECHOREPLY, icmp6h->icmp6_code);
          break;
        default:
          break;
      }
    }
  }

  fill_v4hdr_from_v6hdr(iph, ip6h, v4saddr, v4daddr, ipid, ipflags, proto, infrag_payload_len);
  if(ul_sum) {
    *ul_sum = unchecksum16(pv6, (((u8 *)ptrans_hdr)-((u8 *)pv6))/2, *ul_sum);
    *ul_sum = rechecksum16(iph, 10, *ul_sum);
  }

  memmove(((char *)pv6) + IPV4HDRSIZE, get_next_header_ptr6(ip6h, v6_len), v6_len - IPV4HDRSIZE);
  memcpy(pv6, iph, IPV4HDRSIZE);
  *ptailTruncSize += IPV6V4HDRDELTA;
  return (v6_len - IPV6V4HDRDELTA);
}

u8 *icmp_parameter_ptr(struct icmphdr *icmph) {
  u8 *icmp_pptr = ((u8 *)(icmph))+4;
  return icmp_pptr;
}

u32 *icmp6_parameter_ptr(struct icmp6hdr *icmp6h) {
  u32 *icmp6_pptr = ((u32 *)(icmp6h))+1;
  return icmp6_pptr;
}

static void nat46_fixup_icmp6_dest_unreach(nat46_instance_t *nat46, struct ipv6hdr *ip6h, struct icmp6hdr *icmp6h, struct sk_buff *old_skb, int *ptailTruncSize) {
  /*
   * Destination Unreachable (Type 1)  Set the Type to 3, and adjust
   * the ICMP checksum both to take the type/code change into
   * account and to exclude the ICMPv6 pseudo-header.
   *
   * Translate the Code as follows:
   *
   * Code 0 (No route to destination):  Set the Code to 1 (Host
   *            unreachable).
   *
   * Code 1 (Communication with destination administratively
   *        prohibited):  Set the Code to 10 (Communication with
   *        destination host administratively prohibited).
   *
   * Code 2 (Beyond scope of source address):  Set the Code to 1
   *        (Host unreachable).  Note that this error is very unlikely
   *        since an IPv4-translatable source address is typically
   *        considered to have global scope.
   *
   * Code 3 (Address unreachable):  Set the Code to 1 (Host
   *        unreachable).
   *
   * Code 4 (Port unreachable):  Set the Code to 3 (Port
   *        unreachable).
   *
   * Other Code values:  Silently drop.
   */

  int len;

  switch(icmp6h->icmp6_code) {
    case 0:
    case 2:
    case 3:
      update_icmp6_type_code(nat46, icmp6h, 3, 1);
      break;
    case 1:
      update_icmp6_type_code(nat46, icmp6h, 3, 10);
      break;
    case 4:
      update_icmp6_type_code(nat46, icmp6h, 3, 3);
      break;
    default:
      ip6h->nexthdr = NEXTHDR_NONE;
  }
  len = ntohs(ip6h->payload_len)-sizeof(*icmp6h);
  len = xlate_payload6_to4(nat46, (icmp6h + 1), get_next_header_ptr6((icmp6h + 1), len), len, &icmp6h->icmp6_cksum, ptailTruncSize);
}

static void nat46_fixup_icmp6_pkt_toobig(nat46_instance_t *nat46, struct ipv6hdr *ip6h, struct icmp6hdr *icmp6h, struct sk_buff *old_skb, int *ptailTruncSize) {
  /*
   * Packet Too Big (Type 2):  Translate to an ICMPv4 Destination
   * Unreachable (Type 3) with Code 4, and adjust the ICMPv4
   * checksum both to take the type change into account and to
   * exclude the ICMPv6 pseudo-header.  The MTU field MUST be
   * adjusted for the difference between the IPv4 and IPv6 header
   * sizes, taking into account whether or not the packet in error
   * includes a Fragment Header, i.e., minimum(advertised MTU-20,
   * MTU_of_IPv4_nexthop, (MTU_of_IPv6_nexthop)-20).
   *
   * See also the requirements in Section 6.
   *
   * Section 6 says this for v6->v4 side translation:
   *
   * 2.  In the IPv6-to-IPv4 direction:
   *
   *        A.  If there is a Fragment Header in the IPv6 packet, the last 16
   *            bits of its value MUST be used for the IPv4 identification
   *            value.
   *
   *        B.  If there is no Fragment Header in the IPv6 packet:
   *
   *            a.  If the packet is less than or equal to 1280 bytes:
   *
   *                -  The translator SHOULD set DF to 0 and generate an IPv4
   *                   identification value.
   *
   *                -  To avoid the problems described in [RFC4963], it is
   *                   RECOMMENDED that the translator maintain 3-tuple state
   *                   for generating the IPv4 identification value.
   *
   *            b.  If the packet is greater than 1280 bytes, the translator
   *                SHOULD set the IPv4 DF bit to 1.
   */
  int len = ntohs(ip6h->payload_len)-sizeof(*icmp6h);
  u16 *pmtu = ((u16 *)icmp6h) + 3; /* IPv4-compatible MTU value is 16 bit */
  u16 old_csum = icmp6h->icmp6_cksum;

  if (ntohs(*pmtu) > IPV6V4HDRDELTA) {
    icmp6h->icmp6_cksum = csum16_upd(old_csum, *pmtu, htons(ntohs(*pmtu) - IPV6V4HDRDELTA));
    *pmtu = htons(ntohs(*pmtu) - IPV6V4HDRDELTA);
  }

  len = xlate_payload6_to4(nat46, (icmp6h + 1), get_next_header_ptr6((icmp6h + 1), len), len, &icmp6h->icmp6_cksum, ptailTruncSize);

  update_icmp6_type_code(nat46, icmp6h, 3, 4);

}

static void nat46_fixup_icmp6_time_exceed(nat46_instance_t *nat46, struct ipv6hdr *ip6h, struct icmp6hdr *icmp6h, struct sk_buff *old_skb, int *ptailTruncSize) {
  /*
   * Time Exceeded (Type 3):  Set the Type to 11, and adjust the ICMPv4
   * checksum both to take the type change into account and to
   * exclude the ICMPv6 pseudo-header.  The Code is unchanged.
   */
  int len = ntohs(ip6h->payload_len)-sizeof(*icmp6h);
  len = xlate_payload6_to4(nat46, (icmp6h + 1), get_next_header_ptr6((icmp6h + 1), len), len, &icmp6h->icmp6_cksum, ptailTruncSize);

  update_icmp6_type_code(nat46, icmp6h, 11, icmp6h->icmp6_code);
}

static void nat46_fixup_icmp6_paramprob(nat46_instance_t *nat46, struct ipv6hdr *ip6h, struct icmp6hdr *icmp6h, struct sk_buff *old_skb, int *ptailTruncSize) {
  /*
   *         Parameter Problem (Type 4):  Translate the Type and Code as
   *         follows, and adjust the ICMPv4 checksum both to take the type/
   *         code change into account and to exclude the ICMPv6 pseudo-
   *         header.
   *
   *         Translate the Code as follows:
   *
   *         Code 0 (Erroneous header field encountered):  Set to Type 12,
   *            Code 0, and update the pointer as defined in Figure 6.  (If
   *            the Original IPv6 Pointer Value is not listed or the
   *            Translated IPv4 Pointer Value is listed as "n/a", silently
   *            drop the packet.)
   *
   *         Code 1 (Unrecognized Next Header type encountered):  Translate
   *            this to an ICMPv4 protocol unreachable (Type 3, Code 2).
   *
   *         Code 2 (Unrecognized IPv6 option encountered):  Silently drop.
   *
   *      Unknown error messages:  Silently drop.
   *
   *     +--------------------------------+--------------------------------+
   *     |   Original IPv6 Pointer Value  | Translated IPv4 Pointer Value  |
   *     +--------------------------------+--------------------------------+
   *     |  0  | Version/Traffic Class    |  0  | Version/IHL, Type Of Ser |
   *     |  1  | Traffic Class/Flow Label |  1  | Type Of Service          |
   *     | 2,3 | Flow Label               | n/a |                          |
   *     | 4,5 | Payload Length           |  2  | Total Length             |
   *     |  6  | Next Header              |  9  | Protocol                 |
   *     |  7  | Hop Limit                |  8  | Time to Live             |
   *     | 8-23| Source Address           | 12  | Source Address           |
   *     |24-39| Destination Address      | 16  | Destination Address      |
   *     +--------------------------------+--------------------------------+
   */
  static int ptr6_4[] = { 0, 1, -1, -1, 2, 2, 9, 8,
                          12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
                          16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, -1 };
  u32 *pptr6 = icmp6_parameter_ptr(icmp6h);
  u8 *pptr4 = icmp_parameter_ptr((struct icmphdr *)icmp6h);
  int new_pptr = -1;
  int len = ntohs(ip6h->payload_len)-sizeof(*icmp6h);

  switch(icmp6h->icmp6_code) {
    case 0:
      if(*pptr6 < sizeof(ptr6_4)/sizeof(ptr6_4[0])) {
        new_pptr = ptr6_4[*pptr6];
        if (new_pptr >= 0) {
          icmp6h->icmp6_cksum = csum16_upd(icmp6h->icmp6_cksum, (*pptr6 & 0xffff), (new_pptr << 8));
          *pptr4 = 0xff & new_pptr;
        } else {
          ip6h->nexthdr = NEXTHDR_NONE;
        }
      } else {
        ip6h->nexthdr = NEXTHDR_NONE;
      }
      break;
    case 1:
      *pptr6 = 0;
      update_icmp6_type_code(nat46, icmp6h, 3, 2);
      len = xlate_payload6_to4(nat46, (icmp6h + 1), get_next_header_ptr6((icmp6h + 1), len), len, &icmp6h->icmp6_cksum, ptailTruncSize);
      break;
    case 2: /* fallthrough to default */
    default:
      ip6h->nexthdr = NEXTHDR_NONE;
  }
}


/* Fixup ICMP6->ICMP before IP header translation, according to http://tools.ietf.org/html/rfc6145 */

static void nat46_fixup_icmp6(nat46_instance_t *nat46, struct ipv6hdr *ip6h, struct icmp6hdr *icmp6h, struct sk_buff *old_skb, int *ptailTruncSize) {

  if(icmp6h->icmp6_type & 128) {
    /* Informational ICMP */
    switch(icmp6h->icmp6_type) {
      case ICMPV6_ECHO_REQUEST:
        update_icmp6_type_code(nat46, icmp6h, ICMP_ECHO, icmp6h->icmp6_code);
        break;
      case ICMPV6_ECHO_REPLY:
        update_icmp6_type_code(nat46, icmp6h, ICMP_ECHOREPLY, icmp6h->icmp6_code);
        break;
      default:
        ip6h->nexthdr = NEXTHDR_NONE;
    }
  } else {
    /* ICMPv6 errors */
    switch(icmp6h->icmp6_type) {
      case ICMPV6_DEST_UNREACH:
        nat46_fixup_icmp6_dest_unreach(nat46, ip6h, icmp6h, old_skb, ptailTruncSize);
        break;
      case ICMPV6_PKT_TOOBIG:
        nat46_fixup_icmp6_pkt_toobig(nat46, ip6h, icmp6h, old_skb, ptailTruncSize);
        break;
      case ICMPV6_TIME_EXCEED:
        nat46_fixup_icmp6_time_exceed(nat46, ip6h, icmp6h, old_skb, ptailTruncSize);
        break;
      case ICMPV6_PARAMPROB:
        nat46_fixup_icmp6_paramprob(nat46, ip6h, icmp6h, old_skb, ptailTruncSize);
        break;
      default:
        ip6h->nexthdr = NEXTHDR_NONE;
    }
  }
}


int ip6_input_not_interested(nat46_instance_t *nat46, struct ipv6hdr *ip6h, struct sk_buff *old_skb) {
  if (old_skb->protocol != htons(ETH_P_IPV6)) {
    nat46debug(3, "Not an IPv6 packet");
    return 1;
  }
  if(old_skb->len < sizeof(struct ipv6hdr) || ip6h->version != 6) {
    nat46debug(3, "Len short or not correct version: %d", ip6h->version);
    return 1;
  }
  if (!(ipv6_addr_type(&ip6h->saddr) & IPV6_ADDR_UNICAST)) {
    nat46debug(3, "Source address not unicast");
    return 1;
  }
  return 0;
}

static uint16_t nat46_fixup_icmp_time_exceeded(nat46_instance_t *nat46, struct iphdr *iph, struct icmphdr *icmph, struct sk_buff *old_skb) {
  /*
   * Set the Type to 3, and adjust the
   * ICMP checksum both to take the type change into account and
   * to include the ICMPv6 pseudo-header.  The Code is unchanged.
   */
  icmph->type = 3;
  return 0;
}

static uint16_t nat46_fixup_icmp_parameterprob(nat46_instance_t *nat46, struct iphdr *iph, struct icmphdr *icmph, struct sk_buff *old_skb) {
  /*
   * Set the Type to 4, and adjust the
   * ICMP checksum both to take the type/code change into account
   * and to include the ICMPv6 pseudo-header.
   *
   * Translate the Code as follows:
   *
   * Code 0 (Pointer indicates the error):  Set the Code to 0
   * (Erroneous header field encountered) and update the
   * pointer as defined in Figure 3.  (If the Original IPv4
   * Pointer Value is not listed or the Translated IPv6
   * Pointer Value is listed as "n/a", silently drop the
   * packet.)
   *
   * Code 1 (Missing a required option):  Silently drop.
   *
   * Code 2 (Bad length):  Set the Code to 0 (Erroneous header
   * field encountered) and update the pointer as defined in
   * Figure 3.  (If the Original IPv4 Pointer Value is not
   * listed or the Translated IPv6 Pointer Value is listed as
   * "n/a", silently drop the packet.)
   *
   *            Other Code values:  Silently drop.
   *
   *     +--------------------------------+--------------------------------+
   *     |   Original IPv4 Pointer Value  | Translated IPv6 Pointer Value  |
   *     +--------------------------------+--------------------------------+
   *     |  0  | Version/IHL              |  0  | Version/Traffic Class    |
   *     |  1  | Type Of Service          |  1  | Traffic Class/Flow Label |
   *     | 2,3 | Total Length             |  4  | Payload Length           |
   *     | 4,5 | Identification           | n/a |                          |
   *     |  6  | Flags/Fragment Offset    | n/a |                          |
   *     |  7  | Fragment Offset          | n/a |                          |
   *     |  8  | Time to Live             |  7  | Hop Limit                |
   *     |  9  | Protocol                 |  6  | Next Header              |
   *     |10,11| Header Checksum          | n/a |                          |
   *     |12-15| Source Address           |  8  | Source Address           |
   *     |16-19| Destination Address      | 24  | Destination Address      |
   *     +--------------------------------+--------------------------------+
   */
  static int ptr4_6[] = { 0, 1, 4, 4, -1, -1, -1, -1, 7, 6, -1, -1, 8, 8, 8, 8, 24, 24, 24, 24, -1 };
  u8 *icmp_pptr = icmp_parameter_ptr(icmph);
  int new_pptr = -1;
  switch (icmph->code) {
    case 0:
    case 2:
      if (*icmp_pptr < (sizeof(ptr4_6)/sizeof(ptr4_6[0]))) {
        icmph->code = 0;
        new_pptr = ptr4_6[*icmp_pptr];
        if(new_pptr >= 0) {
          /* FIXME: update the parameter pointer in ICMPv6 with new_pptr value */
        }
      } else {
        iph->protocol = NEXTHDR_NONE;
      }
      break;
    default:
      iph->protocol = NEXTHDR_NONE;
  }
  return 0;
}

static uint16_t nat46_fixup_icmp_dest_unreach(nat46_instance_t *nat46, struct iphdr *iph, struct icmphdr *icmph, struct sk_buff *old_skb) {
  /*
   *    Translate the Code as
   *    described below, set the Type to 1, and adjust the ICMP
   *    checksum both to take the type/code change into account and
   *    to include the ICMPv6 pseudo-header.
   *
   *    Translate the Code as follows:
   *
   *    Code 0, 1 (Net Unreachable, Host Unreachable):  Set the Code
   *       to 0 (No route to destination).
   *
   *    Code 2 (Protocol Unreachable):  Translate to an ICMPv6
   *       Parameter Problem (Type 4, Code 1) and make the Pointer
   *       point to the IPv6 Next Header field.
   *
   *    Code 3 (Port Unreachable):  Set the Code to 4 (Port
   *       unreachable).
   *
   *    Code 4 (Fragmentation Needed and DF was Set):  Translate to
   *       an ICMPv6 Packet Too Big message (Type 2) with Code set
   *       to 0.  The MTU field MUST be adjusted for the difference
   *       between the IPv4 and IPv6 header sizes, i.e.,
   *       minimum(advertised MTU+20, MTU_of_IPv6_nexthop,
   *       (MTU_of_IPv4_nexthop)+20).  Note that if the IPv4 router
   *       set the MTU field to zero, i.e., the router does not
   *       implement [RFC1191], then the translator MUST use the
   *       plateau values specified in [RFC1191] to determine a
   *       likely path MTU and include that path MTU in the ICMPv6
   *       packet.  (Use the greatest plateau value that is less
   *       than the returned Total Length field.)
   *
   *       See also the requirements in Section 6.
   *
   *    Code 5 (Source Route Failed):  Set the Code to 0 (No route
   *       to destination).  Note that this error is unlikely since
   *       source routes are not translated.
   *
   *    Code 6, 7, 8:  Set the Code to 0 (No route to destination).
   *
   *    Code 9, 10 (Communication with Destination Host
   *       Administratively Prohibited):  Set the Code to 1
   *       (Communication with destination administratively
   *       prohibited).
   *
   *    Code 11, 12:  Set the Code to 0 (No route to destination).
   *
   *    Code 13 (Communication Administratively Prohibited):  Set
   *       the Code to 1 (Communication with destination
   *       administratively prohibited).
   *
   *    Code 14 (Host Precedence Violation):  Silently drop.
   *
   *    Code 15 (Precedence cutoff in effect):  Set the Code to 1
   *       (Communication with destination administratively
   *       prohibited).
   *
   *    Other Code values:  Silently drop.
   *
   */

  u16 *pmtu = ((u16 *)icmph) + 3; /* IPv4-compatible MTU value is 16 bit */

  switch (icmph->code) {
    case 0:
    case 1:
      icmph->code = 0;
      break;
    case 2:
      /* FIXME: set ICMPv6 parameter pointer to 6 */
      icmph->type = 4;
      icmph->code = 1;
      break;
    case 3:
      icmph->code = 4;
      break;
    case 4:
      /*
       * On adjusting the signaled MTU within packet:
       *
       * IPv4 has 20 bytes smaller header size, so, standard says
       * we can advertise a higher MTU here. However, then we will
       * need to ensure it does not overshoot our egress link MTU,
       * which implies knowing the egress interface, which is
       * not trivial in the current model.
       *
       * So, we'd want to leave the MTU as aside. But, the Section 6
       * has something more to say:
       *
       *   1.  In the IPv4-to-IPv6 direction: if the MTU value of ICMPv4 Packet
       *     Too Big (PTB) messages is less than 1280, change it to 1280.
       *     This is intended to cause the IPv6 host and IPv6 firewall to
       *     process the ICMP PTB message and generate subsequent packets to
       *     this destination with an IPv6 Fragment Header.
       *
       */
      icmph->type = 2;
      icmph->code = 0;
      if (ntohs(*pmtu) < 1280) {
        *pmtu = htons(1280);
      }
      break;
    case 5:
    case 6:
    case 7:
    case 8:
      icmph->code = 0;
      break;
    case 9:
    case 10:
      icmph->code = 1;
      break;
    case 11:
    case 12:
      icmph->code = 0;
      break;
    case 13:
    case 15:
      icmph->code = 1;
      break;
    default:
      iph->protocol = NEXTHDR_NONE;
  }
  return 0;
}


/* Fixup ICMP->ICMP6 before IP header translation, according to http://tools.ietf.org/html/rfc6145 */

static uint16_t nat46_fixup_icmp(nat46_instance_t *nat46, struct iphdr *iph, struct sk_buff *old_skb) {
  struct icmphdr *icmph = (struct icmphdr *)(iph+1);
  uint16_t ret = 0;

  iph->protocol = NEXTHDR_ICMP;

  switch(icmph->type) {
    case ICMP_ECHO:
      icmph->type = ICMPV6_ECHO_REQUEST;
      ret = icmph->un.echo.id;
      nat46debug(3, "ICMP echo request translated into IPv6, id: %d", ntohs(ret));
      break;
    case ICMP_ECHOREPLY:
      icmph->type = ICMPV6_ECHO_REPLY;
      ret = icmph->un.echo.id;
      nat46debug(3, "ICMP echo reply translated into IPv6, id: %d", ntohs(ret));
      break;
    case ICMP_TIME_EXCEEDED:
      ret = nat46_fixup_icmp_time_exceeded(nat46, iph, icmph, old_skb);
      break;
    case ICMP_PARAMETERPROB:
      ret = nat46_fixup_icmp_parameterprob(nat46, iph, icmph, old_skb);
      break;
    case ICMP_DEST_UNREACH:
      ret = nat46_fixup_icmp_dest_unreach(nat46, iph, icmph, old_skb);
      break;
    default:
      /* Silently drop. */
      iph->protocol = NEXTHDR_NONE;
  }
  return ret;
}

int pairs_xlate_v6_to_v4_outer(nat46_instance_t *nat46, struct ipv6hdr *ip6h, uint16_t proto, __u32 *pv4saddr, __u32 *pv4daddr) {
  int ipair = 0;
  nat46_xlate_rulepair_t *apair = NULL;
  int xlate_src = -1;
  int xlate_dst = -1;

  for(ipair = 0; ipair < nat46->npairs; ipair++) {
    apair = &nat46->pairs[ipair];

    if(-1 == xlate_dst) {
      if (xlate_v6_to_v4(nat46, &apair->local, &ip6h->daddr, pv4daddr)) {
        xlate_dst = ipair;
      }
    }
    if(-1 == xlate_src) {
      if (xlate_v6_to_v4(nat46, &apair->remote, &ip6h->saddr, pv4saddr)) {
        xlate_src = ipair;
      }
    }
    if( (xlate_src >= 0) && (xlate_dst >= 0) ) {
      break;
    } else {
      /* We did not match fully and there are more rules */
      if((ipair+1 < nat46->npairs) && is_last_pair_in_group(apair)) {
        xlate_src = -1;
        xlate_dst = -1;
      }
    }
  }
  if (xlate_dst >= 0) {
    if (xlate_src < 0) {
      if(proto == NEXTHDR_ICMP) {
        nat46debug(1, "[nat46] Could not translate remote address v6->v4, ipair %d, for ICMP6 use dest addr", ipair);
        *pv4saddr = *pv4daddr;
        xlate_src = xlate_dst;
      } else {
        nat46debug(5, "[nat46] Could not translate remote address v6->v4, ipair %d", ipair);
      }
    }
  } else {
    nat46debug(1, "[nat46] Could not find a translation pair v6->v4 src %pI6c dst %pI6c", &ip6h->saddr, &ip6h->daddr);
  }
  nat46debug(5, "[nat46] pairs_xlate_v6_to_v4_outer result src %d dst %d", xlate_src, xlate_dst);
  return ( (xlate_src >= 0) && (xlate_dst >= 0) );
}


void nat46_ipv6_input(struct sk_buff *old_skb) {
  struct ipv6hdr *ip6h = ipv6_hdr(old_skb);
  nat46_instance_t *nat46 = get_nat46_instance(old_skb);
  uint16_t proto;
  uint16_t frag_off;
  uint16_t frag_id;

  struct iphdr * iph;
  __u32 v4saddr, v4daddr;
  struct sk_buff * new_skb = 0;
  int truncSize = 0;
  int tailTruncSize = 0;
  int v6packet_l3size = sizeof(*ip6h);
  int l3_infrag_payload_len = ntohs(ip6h->payload_len);
  int check_for_l4 = 0;

  nat46debug(4, "nat46_ipv6_input packet");

  if(ip6_input_not_interested(nat46, ip6h, old_skb)) {
    nat46debug(1, "nat46_ipv6_input not interested");
    goto done;
  }
  nat46debug(5, "nat46_ipv6_input next hdr: %d, len: %d, is_fragment: %d",
                ip6h->nexthdr, old_skb->len, ip6h->nexthdr == NEXTHDR_FRAGMENT);
  proto = ip6h->nexthdr;
  if (proto == NEXTHDR_FRAGMENT) {
    struct frag_hdr *fh = (struct frag_hdr*)(ip6h + 1);
    v6packet_l3size += sizeof(struct frag_hdr);
    l3_infrag_payload_len -= sizeof(struct frag_hdr);
    nat46debug(2, "Fragment ID: %08X", fh->identification);
    nat46debug_dump(nat46, 6, fh, ntohs(ip6h->payload_len));

    if(fh->frag_off == 0) {
      /* Atomic fragment */
      proto = fh->nexthdr;
      frag_off = 0; /* no DF bit */
      frag_id = fold_ipv6_frag_id(fh->identification);
      nat46debug(2, "Atomic fragment");
      check_for_l4 = 1;
    } else {
      if (0 == (ntohs(fh->frag_off) & IP6_OFFSET)) {
        /* First fragment. Pretend business as usual, but when creating IP, set the "MF" bit. */
        frag_off = htons(((ntohs(fh->frag_off) & 7) << 13) + (((ntohs(fh->frag_off) >> 3) & 0x1FFF)));
        frag_id = fold_ipv6_frag_id(fh->identification);
	/* ntohs(fh->frag_off) & IP6_MF */
        proto = fh->nexthdr;
        check_for_l4 = 1;
        nat46debug(2, "First fragment, frag_off: %04X, frag id: %04X orig frag_off: %04X", ntohs(frag_off), frag_id, ntohs(fh->frag_off));
      } else {
        /* Not the first fragment - leave as is, allow to translate IPv6->IPv4 */
        proto = fh->nexthdr;
        frag_off = htons(((ntohs(fh->frag_off) & 7) << 13) + (((ntohs(fh->frag_off) >> 3) & 0x1FFF)));
        frag_id = fold_ipv6_frag_id(fh->identification);
        nat46debug(2, "Not first fragment, frag_off: %04X, frag id: %04X orig frag_off: %04X", ntohs(frag_off), frag_id, ntohs(fh->frag_off));
      }
    }
  } else {
    frag_off = htons(IP_DF);
    frag_id = get_next_ip_id();
    check_for_l4 = 1;
  }

  if(!pairs_xlate_v6_to_v4_outer(nat46, ip6h, proto, &v4saddr, &v4daddr)) {
    goto done;
  }

  if (check_for_l4) {
    switch(proto) {
      /* CHECKSUMS UPDATE */
      case NEXTHDR_TCP: {
        struct tcphdr *th = add_offset(ip6h, v6packet_l3size);
        u16 sum1 = csum_ipv6_unmagic(nat46, &ip6h->saddr, &ip6h->daddr, l3_infrag_payload_len, NEXTHDR_TCP, th->check);
        u16 sum2 = csum_tcpudp_remagic(v4saddr, v4daddr, l3_infrag_payload_len, NEXTHDR_TCP, sum1);
        th->check = sum2;
        break;
        }
      case NEXTHDR_UDP: {
        struct udphdr *udp = add_offset(ip6h, v6packet_l3size);
        if (udp->check || !nullcheck) {
          u16 sum1 = csum_ipv6_unmagic(nat46, &ip6h->saddr, &ip6h->daddr, l3_infrag_payload_len, NEXTHDR_UDP, udp->check);
          u16 sum2 = csum_tcpudp_remagic(v4saddr, v4daddr, l3_infrag_payload_len, NEXTHDR_UDP, sum1);
          udp->check = sum2;
        }
        break;
        }
      case NEXTHDR_ICMP: {
        struct icmp6hdr *icmp6h = add_offset(ip6h, v6packet_l3size);
        u16 sum1 = csum_ipv6_unmagic(nat46, &ip6h->saddr, &ip6h->daddr, l3_infrag_payload_len, NEXTHDR_ICMP, icmp6h->icmp6_cksum);
        icmp6h->icmp6_cksum = sum1;
        nat46debug_dump(nat46, 10, icmp6h, l3_infrag_payload_len);
        nat46_fixup_icmp6(nat46, ip6h, icmp6h, old_skb, &tailTruncSize);
        proto = IPPROTO_ICMP;
        break;
        }
      default:
        break;
    }
  } else {
    if(NEXTHDR_ICMP == proto) {
      proto = IPPROTO_ICMP;
    }
  }

  new_skb = skb_copy(old_skb, GFP_ATOMIC); // other possible option: GFP_ATOMIC
  if (!new_skb) {
    nat46debug(0, "[nat46] Could not copy v6 skb");
    goto done;
  }

  /* Remove any debris in the socket control block */
  memset(IPCB(new_skb), 0, sizeof(struct inet_skb_parm));
  /* Remove netfilter references to IPv6 packet, new netfilter references will be created based on IPv4 packet */
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,4,0)
  nf_reset(new_skb);
#else
  nf_reset_ct(new_skb);
#endif

  /* modify packet: actual IPv6->IPv4 transformation */
  truncSize = v6packet_l3size - sizeof(struct iphdr); /* chop first 20 bytes */
  skb_pull(new_skb, truncSize);
  skb_put(new_skb, -tailTruncSize);
  l3_infrag_payload_len -= tailTruncSize;
  skb_reset_network_header(new_skb);
  skb_set_transport_header(new_skb,IPV4HDRSIZE); /* transport (TCP/UDP/ICMP/...) header starts after 20 bytes */

  /* build IPv4 header */
  iph = ip_hdr(new_skb);
  fill_v4hdr_from_v6hdr(iph, ip6h, v4saddr, v4daddr, frag_id, frag_off, proto, l3_infrag_payload_len);
  new_skb->protocol = htons(ETH_P_IP);

  if (ntohs(iph->tot_len) >= 2000) {
    nat46debug(0, "Too big IP len: %d", ntohs(iph->tot_len));
  }

  nat46debug(5, "about to send v4 packet, flags: %02x",  IPCB(new_skb)->flags);
  nat46_netdev_count_xmit(new_skb, old_skb->dev);
  netif_rx(new_skb);

  /* TBD: should copy be released here? */

done:
  release_nat46_instance(nat46);
}



void ip6_update_csum(struct sk_buff * skb, struct ipv6hdr * ip6hdr, int do_atomic_frag)
{
  u32 sum1=0;
  u16 sum2=0;
  __sum16 oldsum = 0;

  switch (ip6hdr->nexthdr) {
    case IPPROTO_TCP: {
      struct tcphdr *th = tcp_hdr(skb);
      unsigned tcplen = 0;

      oldsum = th->check;
      tcplen = ntohs(ip6hdr->payload_len) - (do_atomic_frag?8:0); /* TCP header + payload */
      th->check = 0;
      sum1 = csum_partial((char*)th, tcplen, 0); /* calculate checksum for TCP hdr+payload */
      sum2 = csum_ipv6_magic(&ip6hdr->saddr, &ip6hdr->daddr, tcplen, ip6hdr->nexthdr, sum1); /* add pseudoheader */
      th->check = sum2;
      break;
      }
    case IPPROTO_UDP: {
      struct udphdr *udp = udp_hdr(skb);
      unsigned udplen = ntohs(ip6hdr->payload_len) - (do_atomic_frag?8:0); /* UDP hdr + payload */

      oldsum = udp->check;
      udp->check = 0;

      sum1 = csum_partial((char*)udp, udplen, 0); /* calculate checksum for UDP hdr+payload */
      sum2 = csum_ipv6_magic(&ip6hdr->saddr, &ip6hdr->daddr, udplen, ip6hdr->nexthdr, sum1); /* add pseudoheader */

      udp->check = sum2;

      break;
      }
    case NEXTHDR_ICMP: {
      struct icmp6hdr *icmp6h = icmp6_hdr(skb);
      unsigned icmp6len = 0;
      icmp6len = ntohs(ip6hdr->payload_len) - (do_atomic_frag?8:0); /* ICMP header + payload */
      icmp6h->icmp6_cksum = 0;
      sum1 = csum_partial((char*)icmp6h, icmp6len, 0); /* calculate checksum for TCP hdr+payload */
      sum2 = csum_ipv6_magic(&ip6hdr->saddr, &ip6hdr->daddr, icmp6len, ip6hdr->nexthdr, sum1); /* add pseudoheader */
      icmp6h->icmp6_cksum = sum2;
      break;
      }
    }
}

int ip4_input_not_interested(nat46_instance_t *nat46, struct iphdr *iph, struct sk_buff *old_skb) {
  if (old_skb->protocol != htons(ETH_P_IP)) {
    nat46debug(3, "Not an IPv4 packet");
    return 1;
  }
  // FIXME: check source to be within our prefix
  return 0;
}

int pairs_xlate_v4_to_v6_outer(nat46_instance_t *nat46, struct iphdr *hdr4, uint16_t *sport, uint16_t *dport, void *v6saddr, void *v6daddr) {
  int ipair = 0;
  nat46_xlate_rulepair_t *apair = NULL;
  int xlate_src = -1;
  int xlate_dst = -1;

  for(ipair = 0; ipair < nat46->npairs; ipair++) {
    apair = &nat46->pairs[ipair];

    if(-1 == xlate_src) {
      if(xlate_v4_to_v6(nat46, &apair->local, &hdr4->saddr, v6saddr, sport)) {
        xlate_src = ipair;
      }
    }
    if(-1 == xlate_dst) {
      if(xlate_v4_to_v6(nat46, &apair->remote, &hdr4->daddr, v6daddr, dport)) {
        xlate_dst = ipair;
      }
    }
    if( (xlate_src >= 0) && (xlate_dst >= 0) ) {
      break;
    } else {
      /* We did not match fully and there are more rules */
      if((ipair+1 < nat46->npairs) && is_last_pair_in_group(apair)) {
        xlate_src = -1;
        xlate_dst = -1;
      }
    }
  }
  nat46debug(5, "[nat46] pairs_xlate_v4_to_v6_outer result: src %d dst %d", xlate_src, xlate_dst);
  if ( (xlate_src >= 0) && (xlate_dst >= 0) ) {
    return 1;
  }

  nat46debug(1, "[nat46] Could not find a translation pair v4->v6");

  return 0;
}


void nat46_ipv4_input(struct sk_buff *old_skb) {
  nat46_instance_t *nat46 = get_nat46_instance(old_skb);
  struct sk_buff *new_skb;
  uint16_t sport = 0, dport = 0;

  int check_for_l4 = 0;
  int having_l4 = 0;
  int add_frag_header = 0;

  struct ipv6hdr * hdr6;
  struct iphdr * hdr4 = ip_hdr(old_skb);

  char v6saddr[16], v6daddr[16];

  memset(v6saddr, 1, 16);
  memset(v6daddr, 2, 16);

  if (ip4_input_not_interested(nat46, hdr4, old_skb)) {
    goto done;
  }
  nat46debug(1, "nat46_ipv4_input packet");
  nat46debug(5, "nat46_ipv4_input protocol: %d, len: %d, flags: %02x", hdr4->protocol, old_skb->len, IPCB(old_skb)->flags);
  if(0 == (ntohs(hdr4->frag_off) & 0x3FFF) ) {
    check_for_l4 = 1;
  } else {
    add_frag_header = 1;
    if (0 == (ntohs(hdr4->frag_off) & 0x1FFF)) {
      check_for_l4 = 1;
    }
  }

  if (check_for_l4) {
    switch(hdr4->protocol) {
      case IPPROTO_TCP: {
	struct tcphdr *th = tcp_hdr(old_skb);
	sport = th->source;
	dport = th->dest;
	having_l4 = 1;
	break;
	}
      case IPPROTO_UDP: {
	struct udphdr *udp = udp_hdr(old_skb);
	sport = udp->source;
	dport = udp->dest;
	having_l4 = 1;
	break;
	}
      case IPPROTO_ICMP:
	sport = dport = nat46_fixup_icmp(nat46, hdr4, old_skb);
	having_l4 = 1;
	break;
      default:
	break;
    }
  } else {
    if (IPPROTO_ICMP == hdr4->protocol) {
      hdr4->protocol = NEXTHDR_ICMP;
    }
    dport = 0;
    sport = 0;
    having_l4 = 1;
  }

  if(!pairs_xlate_v4_to_v6_outer(nat46, hdr4, having_l4 ? &sport : NULL, having_l4 ? &dport : NULL, v6saddr, v6daddr)) {
    nat46debug(0, "[nat46] Could not translate v4->v6");
    goto done;
  }

  new_skb = skb_copy(old_skb, GFP_ATOMIC);
  if (!new_skb) {
    nat46debug(0, "[nat46] Could not copy v4 skb");
    goto done;
  }

  /* Remove any debris in the socket control block */
  memset(IPCB(new_skb), 0, sizeof(struct inet_skb_parm));
  /* Remove netfilter references to IPv4 packet, new netfilter references will be created based on IPv6 packet */
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,4,0)
  nf_reset(new_skb);
#else
  nf_reset_ct(new_skb);
#endif

  /* expand header (add 20 extra bytes at the beginning of sk_buff) */
  pskb_expand_head(new_skb, IPV6HDRSIZE - (hdr4->ihl << 2) + (add_frag_header?8:0), 0, GFP_ATOMIC);

  skb_push(new_skb, IPV6HDRSIZE - (hdr4->ihl << 2) + (add_frag_header?8:0)); /* push boundary by extra 20 bytes */

  skb_reset_network_header(new_skb);
  skb_set_transport_header(new_skb, IPV6HDRSIZE + (add_frag_header?8:0) ); /* transport (TCP/UDP/ICMP/...) header starts after 40 bytes */

  hdr6 = ipv6_hdr(new_skb);
  memset(hdr6, 0, sizeof(*hdr6) + (add_frag_header?8:0));

  /* build IPv6 header */
  ip6_flow_hdr(hdr6, hdr4->tos, 0); /* version, priority, flowlabel */

  /* IPv6 length is a payload length, IPv4 is hdr+payload */
  hdr6->payload_len = htons(ntohs(hdr4->tot_len) - (hdr4->ihl << 2) + (add_frag_header?8:0));
  hdr6->nexthdr = hdr4->protocol;
  hdr6->hop_limit = hdr4->ttl;
  memcpy(&hdr6->saddr, v6saddr, 16);
  memcpy(&hdr6->daddr, v6daddr, 16);

  new_skb->protocol = htons(ETH_P_IPV6);

  if (add_frag_header) {
    struct frag_hdr *fh = (struct frag_hdr*)(hdr6 + 1);
    fh->frag_off = htons(((ntohs(hdr4->frag_off) >> 13) & 7) + ((ntohs(hdr4->frag_off) & 0x1FFF) << 3));
    fh->nexthdr = hdr4->protocol;
    fh->identification = htonl(ntohs(hdr4->id));
  }
  ip6_update_csum(new_skb, hdr6, add_frag_header);

  hdr6->nexthdr = add_frag_header ? NEXTHDR_FRAGMENT : hdr4->protocol;


  // FIXME: check if you can not fit the packet into the cached MTU
  // if (dst_mtu(skb_dst(new_skb))==0) { }
  skbuff_bcm_ext_map_get(new_skb, map_forward) = (hdr4->frag_off & htons(IP_DF)) ? MAP_FORWARD_MODE1 : MAP_FORWARD_MODE2;

  nat46debug(5, "about to send v6 packet, flags: %02x",  IPCB(new_skb)->flags);
  nat46_netdev_count_xmit(new_skb, old_skb->dev);
  netif_rx(new_skb);

done:
  release_nat46_instance(nat46);
}


