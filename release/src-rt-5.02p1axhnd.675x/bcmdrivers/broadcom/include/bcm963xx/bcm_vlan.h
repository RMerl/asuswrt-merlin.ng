/*
<:copyright-BRCM:2007:proprietary:standard

   Copyright (c) 2007 Broadcom 
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

#ifndef _BCM_VLAN_H_
#define _BCM_VLAN_H_

#include "bcm_vlan_defs.h"

/*
 * Macros, type definitions
 */

/* FIXME: IFNAMSIZ is defined in kernel/linux/include/linux/if.h, but this
   file cannot be included in user space */
#define BCM_VLAN_IFNAMSIZ          16

#define BCM_VLAN_IF_SUFFIX_SIZE    5 /* 4 characters + trailing '\0' */
#define BCM_VLAN_IF_SUFFIX_DEFAULT ".v"

#define BCM_VLAN_TABLE_NAME_SIZE   16

#define BCM_VLAN_MAX_TAGS          4

#define BCM_VLAN_MAX_CMD_ARGS      2

#define BCM_VLAN_MAX_RULE_COMMANDS 16

#define BCM_VLAN_DONT_CARE         ~0

#define BCM_VLAN_DEFAULT_DEV_NAME  "DEFAULT"

typedef enum {
    BCM_VLAN_TABLE_DIR_RX=0,
    BCM_VLAN_TABLE_DIR_TX,
    BCM_VLAN_TABLE_DIR_MAX,
} bcmVlan_ruleTableDirection_t;

typedef enum {
    BCM_VLAN_ACTION_ACCEPT=0,
    BCM_VLAN_ACTION_DROP,
    BCM_VLAN_ACTION_MAX,
} bcmVlan_defaultAction_t;

typedef enum {
    BCM_VLAN_POSITION_BEFORE=0,
    BCM_VLAN_POSITION_AFTER,
    BCM_VLAN_POSITION_APPEND, /* always before last if last is occupied. */
    BCM_VLAN_POSITION_LAST,   /* always at the bottom of the table */
    BCM_VLAN_POSITION_MAX,
} bcmVlan_ruleInsertPosition_t;

typedef enum {
    BCM_VLAN_OPCODE_NOP=0,

    BCM_VLAN_OPCODE_POP_TAG,
    BCM_VLAN_OPCODE_PUSH_TAG,
    BCM_VLAN_OPCODE_DEAGGR_TAG, /* deaggregate vlan tag */

    BCM_VLAN_OPCODE_SET_ETHERTYPE,

    BCM_VLAN_OPCODE_SET_PBITS,
    BCM_VLAN_OPCODE_SET_CFI,
    BCM_VLAN_OPCODE_SET_VID,
    BCM_VLAN_OPCODE_SET_TAG_ETHERTYPE,

    BCM_VLAN_OPCODE_COPY_PBITS,
    BCM_VLAN_OPCODE_COPY_CFI,
    BCM_VLAN_OPCODE_COPY_VID,
    BCM_VLAN_OPCODE_COPY_TAG_ETHERTYPE,

    BCM_VLAN_OPCODE_DSCP2PBITS,

    BCM_VLAN_OPCODE_SET_DSCP,

    BCM_VLAN_OPCODE_DROP_FRAME,

    BCM_VLAN_OPCODE_SET_SKB_PRIO,
    BCM_VLAN_OPCODE_SET_SKB_MARK_PORT,
    BCM_VLAN_OPCODE_SET_SKB_MARK_QUEUE,
    BCM_VLAN_OPCODE_SET_SKB_MARK_QUEUE_BYPBITS,
    BCM_VLAN_OPCODE_SET_SKB_MARK_FLOWID,

    BCM_VLAN_OPCODE_OVRD_LEARNING_VID,
    BCM_VLAN_OPCODE_SET_DP,

    BCM_VLAN_OPCODE_CONTINUE,

    BCM_VLAN_OPCODE_MAX,
} bcmVlan_cmdOpCode_t;

typedef struct {
    unsigned int tciMask; /* Should NOT be set by user */
    unsigned int tci; /* Should NOT be set by user */
    unsigned int etherType;
    unsigned int pbits;
    unsigned int cfi;
    unsigned int vid;
} bcmVlan_vlanTag_t;

typedef struct {
    unsigned int skbPrio;
    unsigned int skbMarkFlowId;
    unsigned int skbMarkPort;
    unsigned int etherType;
    unsigned int dscp;
    unsigned int dscp2pbits;
    unsigned int vlanDevMacAddr;
    unsigned int ipProto;
    unsigned int flags;
    bcmVlan_vlanTag_t vlanTag[BCM_VLAN_MAX_TAGS];
    char txVlanDevName[BCM_VLAN_IFNAMSIZ];
    char rxRealDevName[BCM_VLAN_IFNAMSIZ];
} bcmVlan_tagRuleFilter_t;

#define BCM_VLAN_FILTER_FLAGS_ARE_INVALID(_flags)       \
    ( (_flags) & ~BCM_VLAN_FILTER_FLAGS_ALL )

typedef unsigned int bcmVlan_cmdArg_t;

typedef struct {
    bcmVlan_cmdOpCode_t opCode;
    bcmVlan_cmdArg_t arg[BCM_VLAN_MAX_CMD_ARGS];
} bcmVlan_tagRuleCommand_t;

typedef unsigned int bcmVlan_tagRuleIndex_t;

typedef enum {
    BCM_VLAN_MODE_ONT = 0,
    BCM_VLAN_MODE_RG,
    BCM_VLAN_MODE_MAX
} bcmVlan_realDevMode_t;

#define BCM_VLAN_FLOW_MAX_QOS_RULES 16

typedef enum {
    BCM_VLAN_RULE_TYPE_FLOW = 0,
    BCM_VLAN_RULE_TYPE_QOS,
    BCM_VLAN_RULE_TYPE_INVALID,
} bcmVlan_ruleType_t;

typedef struct {
    bcmVlan_tagRuleFilter_t filter;
    bcmVlan_tagRuleCommand_t cmd[BCM_VLAN_MAX_RULE_COMMANDS];
    bcmVlan_tagRuleIndex_t id;
    char rxVlanDevName[BCM_VLAN_IFNAMSIZ];
    bcmVlan_ruleType_t type;
    char isIptvOnly;
} bcmVlan_tagRule_t;

typedef struct {
    char realDevName[BCM_VLAN_IFNAMSIZ];
    bcmVlan_ruleTableDirection_t tableDir;
    unsigned int nbrOfTags;
} bcmVlan_ruleTableId_t;

typedef enum {
    BCM_VLAN_MATCH_NO_VLANDEV_MACADDR = 0,
    BCM_VLAN_MATCH_VLANDEV_MACADDR = 1,
    BCM_VLAN_MATCH_VLANDEV_MACADDR_OR_MULTICAST = 2,
} bcmVlan_vlanDevMacAddrMatch_t;

typedef enum {
    BCM_VLAN_FLOWDIR_US = 0,
    BCM_VLAN_FLOWDIR_DS,
    BCM_VLAN_FLOWDIR_MAX
} bcmVlan_flowDir_t;

/* From ITU-T G.988 "drop precedence colour marking" attribute. */
typedef enum
{
   BCM_VLAN_DP_CODE_NONE = 0,
   BCM_VLAN_DP_CODE_INTERNAL,
   BCM_VLAN_DP_CODE_DEI,
   BCM_VLAN_DP_CODE_PCP8P0D,
   BCM_VLAN_DP_CODE_PCP7P1D,
   BCM_VLAN_DP_CODE_PCP6P2D,
   BCM_VLAN_DP_CODE_PCP5P3D,
   BCM_VLAN_DP_CODE_DSCPAF,
   BCM_VLAN_DP_CODE_MAX
} bcmVlan_dpCode_t;


/*************************************
 ******       User API           *****
 *************************************/

/* Default values */
#define BCM_VLAN_DEFAULT_TAG_TPID  ETH_P_8021Q
#define BCM_VLAN_DEFAULT_TAG_PBITS 0
#define BCM_VLAN_DEFAULT_TAG_CFI   0
#define BCM_VLAN_DEFAULT_TAG_VID   1 /* IEEE 802.1Q, Table 9-2 */

#define BCM_VLAN_MAX_TPID_VALUES   4

/**************************************
 * Macros to get/set command arguments
 * Valid combinations:
 *    TARGET
 *    TARGET + VALUE
 *    TARGET + SOURCE
 *    VALUE  + VALUE2
 **************************************/
#define BCM_VLAN_CMD_GET_TARGET_TAG(_arg)       ( (_arg)[1] )
#define BCM_VLAN_CMD_SET_TARGET_TAG(_arg, _tag) ( (_arg)[1] = (typeof(*(_arg)))(_tag) )

#define BCM_VLAN_CMD_GET_VAL(_arg)              ( (_arg)[0] )
#define BCM_VLAN_CMD_SET_VAL(_arg, _val)        ( (_arg)[0] = (typeof(*(_arg)))(_val) )

#define BCM_VLAN_CMD_GET_SOURCE_TAG(_arg)       ( (_arg)[0] )
#define BCM_VLAN_CMD_SET_SOURCE_TAG(_arg, _tag) ( (_arg)[0] = (typeof(*(_arg)))(_tag) )

#define BCM_VLAN_CMD_GET_VAL2(_arg)             ( (_arg)[1] )
#define BCM_VLAN_CMD_SET_VAL2(_arg, _val)       ( (_arg)[1] = (typeof(*(_arg)))(_val) )

/* Tag Rule Initialization */
static inline void bcmVlan_initTagRule(bcmVlan_tagRule_t *tagRule)
{
    /* initialize filter */
    memset(&tagRule->filter, BCM_VLAN_DONT_CARE, sizeof(bcmVlan_tagRuleFilter_t));

    /* initialize the filter flags */
    tagRule->filter.flags = 0;

    /* initialize the tx filter rx real device name */
    tagRule->filter.rxRealDevName[0] = '\0';

    /* set vlanDevMacAddr flag to BCM_VLAN_MATCH_NO_VLANDEV_MACADDR */
    tagRule->filter.vlanDevMacAddr = BCM_VLAN_MATCH_NO_VLANDEV_MACADDR; 

    /* initialize the IPTV only flag */
    tagRule->isIptvOnly = 0;

    /* initialize transmit VLAN device name filter */
    strcpy(tagRule->filter.txVlanDevName, BCM_VLAN_DEFAULT_DEV_NAME);

    /* initialize commands */
    memset(&tagRule->cmd, BCM_VLAN_OPCODE_NOP, BCM_VLAN_MAX_RULE_COMMANDS * sizeof(bcmVlan_tagRuleCommand_t));

    tagRule->id = BCM_VLAN_DONT_CARE;

    /* initialize receive VLAN device name */
    strcpy(tagRule->rxVlanDevName, BCM_VLAN_DEFAULT_DEV_NAME);

    tagRule->type = BCM_VLAN_RULE_TYPE_FLOW;
}

/* IOCTL commands */
typedef enum {
/* IOCTL cmd values 1 and 2 are mapped to FIBMAP and FIGETBSZ on ARM
   processor. Hence start all IOCTL values from 100 to prevent conflicts */
    BCM_VLAN_IOC_CREATE_VLAN=100, 
    BCM_VLAN_IOC_CREATE_VLAN_BY_NAME,
    BCM_VLAN_IOC_DELETE_VLAN,
    BCM_VLAN_IOC_CREATE_VLAN_FLOWS,
    BCM_VLAN_IOC_DELETE_VLAN_FLOWS,
    BCM_VLAN_IOC_INSERT_TAG_RULE,
    BCM_VLAN_IOC_REMOVE_TAG_RULE,
    BCM_VLAN_IOC_REMOVE_ALL_TAG_RULE,
    BCM_VLAN_IOC_DUMP_RULE_TABLE,
    BCM_VLAN_IOC_GET_NBR_OF_RULES_IN_TABLE,
    BCM_VLAN_IOC_SET_DEFAULT_TAG,
    BCM_VLAN_IOC_SET_DSCP_TO_PBITS,
    BCM_VLAN_IOC_DUMP_DSCP_TO_PBITS,
    BCM_VLAN_IOC_DUMP_LOCAL_STATS,
    BCM_VLAN_IOC_SET_TPID_TABLE,
    BCM_VLAN_IOC_DUMP_TPID_TABLE,
    BCM_VLAN_IOC_SET_IF_SUFFIX,
    BCM_VLAN_IOC_SET_DEFAULT_ACTION,
    BCM_VLAN_IOC_SET_REAL_DEV_MODE,
    BCM_VLAN_IOC_SET_DP,
    BCM_VLAN_IOC_RUN_TEST,
    BCM_VLAN_IOC_DUMP_ALL_RULES,
    BCM_VLAN_IOC_MAX
} bcmVlan_ioctlCmd_t;

typedef struct {
    char realDevName[BCM_VLAN_IFNAMSIZ];
    unsigned int vlanDevId;
    int isRouted;
    int isMulticast;
    int isSwOnly;
} bcmVlan_iocCreateVlan_t;

typedef struct {
    char realDevName[BCM_VLAN_IFNAMSIZ];
    char vlanDevName[BCM_VLAN_IFNAMSIZ];
    int isRouted;
    int isMulticast;
    int isSwOnly;
} bcmVlan_iocCreateVlanByName_t;

typedef struct {
    char rxVlanDevName[BCM_VLAN_IFNAMSIZ];
    char txVlanDevName[BCM_VLAN_IFNAMSIZ];
} bcmVlan_iocVlanFlows_t;

typedef struct {
    char vlanDevName[BCM_VLAN_IFNAMSIZ];
} bcmVlan_iocDeleteVlan_t;

typedef struct {
    bcmVlan_ruleTableId_t ruleTableId;
    bcmVlan_tagRule_t tagRule;
    bcmVlan_ruleInsertPosition_t position;
    bcmVlan_tagRuleIndex_t posTagRuleId;
} bcmVlan_iocInsertTagRule_t;

typedef bcmVlan_iocDeleteVlan_t bcmVlan_iocRemoveAllTagRule_t;

typedef struct {
    bcmVlan_ruleTableId_t ruleTableId;
    bcmVlan_tagRuleIndex_t tagRuleId;
    bcmVlan_tagRule_t tagRule;
} bcmVlan_iocRemoveTagRule_t;

typedef struct {
    bcmVlan_ruleTableId_t ruleTableId;
} bcmVlan_iocDumpRuleTable_t;

typedef struct {
    bcmVlan_ruleTableId_t ruleTableId;
    unsigned int nbrOfRules;
} bcmVlan_iocGetNbrOfRulesInTable_t;

typedef struct {
    bcmVlan_ruleTableId_t ruleTableId;
    UINT16 defaultTpid;
    UINT8 defaultPbits;
    UINT8 defaultCfi;
    UINT16 defaultVid;
} bcmVlan_iocSetDefaultVlanTag_t;

typedef struct {
    char realDevName[BCM_VLAN_IFNAMSIZ];
    UINT8 dscp;
    UINT8 pbits;
} bcmVlan_iocDscpToPbits_t;

typedef struct {
    char realDevName[BCM_VLAN_IFNAMSIZ];
} bcmVlan_iocDumpLocalStats_t;

typedef struct {
    char realDevName[BCM_VLAN_IFNAMSIZ];
    unsigned int tpidTable[BCM_VLAN_MAX_TPID_VALUES];
} bcmVlan_iocSetTpidTable_t;

typedef struct {
    char realDevName[BCM_VLAN_IFNAMSIZ];
} bcmVlan_iocDumpTpidTable_t;

typedef struct {
    char suffix[BCM_VLAN_IF_SUFFIX_SIZE];
} bcmVlan_iocSetIfSuffix_t;

typedef struct {
    bcmVlan_ruleTableId_t ruleTableId;
    bcmVlan_defaultAction_t defaultAction;
    char defaultRxVlanDevName[BCM_VLAN_IFNAMSIZ];
} bcmVlan_iocSetDefaultAction_t;

typedef struct {
    char realDevName[BCM_VLAN_IFNAMSIZ];
    bcmVlan_realDevMode_t mode;
} bcmVlan_iocSetRealDevMode_t;

typedef struct {
    unsigned int testNbr;
    char rxVlanDevName[BCM_VLAN_IFNAMSIZ];
    char txVlanDevName[BCM_VLAN_IFNAMSIZ];
} bcmVlan_iocRunTest_t;

typedef struct {
    UINT8 dir;
    UINT8 dpCode;
} bcmVlan_iocSetDropPrecedence_t;

#endif /* _BCM_VLAN_H_ */

