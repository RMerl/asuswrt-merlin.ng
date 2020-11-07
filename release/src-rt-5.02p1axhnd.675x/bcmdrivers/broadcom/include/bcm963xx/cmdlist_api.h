#ifndef __CMDLIST_API_H_INCLUDED__
#define __CMDLIST_API_H_INCLUDED__

/*
<:copyright-BRCM:2017:proprietary:standard

   Copyright (c) 2017 Broadcom 
   All Rights Reserved

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:> 
*/

#include <linux/if_ether.h>
#include <linux/if_vlan.h>
#if !defined(RDP_SIM)
#include <linux/blog_rule.h>
#endif
#include "cmdlist_defines.h"

#if defined(CONFIG_BCM_ARCHER) || defined(CONFIG_BCM_ARCHER_MODULE) || defined(CONFIG_BCM_ARCHER_SIM)
#define CMDLIST_CMD_LIST_SIZE_MAX        128
#define CMDLIST_CMD_LIST_SIZE_MAX_16     (CMDLIST_CMD_LIST_SIZE_MAX / 2)
#define CMDLIST_CMD_LIST_SIZE_MAX_32     (CMDLIST_CMD_LIST_SIZE_MAX / 4)
#else
#endif

#define CMDLIST_PREPEND_SIZE_MAX  32

typedef struct {
    int (* ipv6_addresses_table_add)(Blog_t *blog_p, uint32_t *table_sram_address_p);
    int (* ipv4_addresses_table_add)(Blog_t *blog_p, uint32_t *table_sram_address_p);
    int (* brcm_tag_info)(Blog_t *blog_p, int *switch_port_p, int *switch_queue_p);
} cmdlist_hooks_t;

/* Command Targets */
typedef enum {
    CMDLIST_CMD_TARGET_SRAM,
    CMDLIST_CMD_TARGET_DDR,
    CMDLIST_CMD_TARGET_CL,
    CMDLIST_CMD_TARGET_MAX
} cmdlist_cmd_target_t;

/* Multicast Header Parsing Information */
typedef struct {
    int txAdjust;
    int vlanTagsAdjust;
    int rxL2HeaderLength;
    union {
        struct {
            uint32_t drop          : 1;
            uint32_t insertEth     : 1;
            uint32_t rxBrcmTag     : 1;
            uint32_t txBrcmTag     : 1;
            uint32_t ptBrcmTag     : 1;
            uint32_t popPPPoA      : 1;
            uint32_t popPPPoE      : 1;
            uint32_t unused        : 25;
        };
        uint32_t flags;
    };
} cmdlist_mcast_parse_t;

typedef enum {
    CMDLIST_BRCM_TAG_NONE,
    CMDLIST_BRCM_TAG_RX_TX,
    CMDLIST_BRCM_TAG_TX,
    CMDLIST_BRCM_TAG_PT, // Passthrough
    CMDLIST_BRCM_TAG_MAX
} cmdlist_brcm_tag_t;

int cmdlist_bind(cmdlist_hooks_t *hooks_p);
void cmdlist_unbind(void);
void cmdlist_init(uint32_t *cmd_list_p, uint32_t cmd_list_length_max, uint32_t cmd_list_start_offset);
int cmdlist_ucast_create(Blog_t *blog_p, cmdlist_cmd_target_t target,
                         uint8_t *prependData_p, int prependSize, void **buffer_pp,
                         cmdlist_brcm_tag_t brcm_tag);
int cmdlist_l2_ucast_create(Blog_t *blog_p, cmdlist_cmd_target_t target,
                            uint8_t *prependData_p, int prependSize, void **buffer_pp,
                            cmdlist_brcm_tag_t brcm_tag);
void cmdlist_mcast_parse(Blog_t *blog_p, blogRule_t *blogRule_p, cmdlist_mcast_parse_t *parse_p,
                         cmdlist_brcm_tag_t brcm_tag);
int cmdlist_l2_mcast_create(Blog_t *blog_p, blogRule_t *blogRule_p, cmdlist_mcast_parse_t *parse_p);
int cmdlist_l3_mcast_create(Blog_t *blog_p, blogRule_t *blogRule_p, cmdlist_mcast_parse_t *parse_p, uint8_t *isRouted_p);
int cmdlist_l2_l3_mcast_create(Blog_t *blog_p, blogRule_t *blogRule_p, cmdlist_mcast_parse_t *parse_p, uint8_t *isRouted_p);
uint32_t cmdlist_get_length(void);
void cmdlist_dump(uint32_t *cmd_list_p, int length32);
void cmdlist_dump_partial(void);
void cmdlist_buffer_free(void *buffer_p);
uint32_t cmdlist_print_stats(void *sf_p);
#endif  /* defined(__CMDLIST_API_H_INCLUDED__) */
