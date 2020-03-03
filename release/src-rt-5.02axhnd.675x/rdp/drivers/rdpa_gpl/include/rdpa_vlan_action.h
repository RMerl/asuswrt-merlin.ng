/*
* <:copyright-BRCM:2013:DUAL/GPL:standard
* 
*    Copyright (c) 2013 Broadcom 
*    All Rights Reserved
* 
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
* 
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
* 
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
* 
* :> 
*/


#ifndef _RDPA_VLAN_ACTION_H_
#define _RDPA_VLAN_ACTION_H_

/** \defgroup vlan_action VLAN Actions
 * RDPA supports many different kinds of L2 header manipulations (VLAN actions).
 * Once VLAN action is configured, it can be applied for specific Ethernet flows,
 * ports, connections.
 * @{
 */

/* ToDo: check constants */
#ifdef XRDP
#if !defined(BCM6878)
#define RDPA_MAX_VLAN_ACTION        129  /**< Max number of VLAN actions per direction */
#else
#define RDPA_MAX_VLAN_ACTION        33  /**< Max number of VLAN actions per direction */
#endif
#define RDPA_DROP_ACTION RDPA_MAX_VLAN_ACTION
#else
#define RDPA_MAX_VLAN_ACTION        128  /**< Max number of VLAN actions per direction */
#define RDPA_DROP_ACTION RDPA_MAX_VLAN_ACTION
#endif

/** Max number of tags supported by vlan_action */
#define RDPA_VLAN_MAX_TAGS          2

/** Outer tag index */
#define RDPA_VLAN_TAG_OUT   0 
/** Inner tag index */
#define RDPA_VLAN_TAG_IN    1 

#define RDPA_VLAN_ACTION_TPID_DONT_CARE (0xffff)

/* Per VLAN tag Command bits */
typedef enum
{
    RDPA_VLAN_CMD_BIT_PUSH,         /* Add VLAN tag */
    RDPA_VLAN_CMD_BIT_PUSH_ALWAYS,  /* Add VLAN tag always (even if packet is double-tagged) */
    RDPA_VLAN_CMD_BIT_POP,          /* Remove VLAN tag */
    RDPA_VLAN_CMD_BIT_REPLACE,      /* Replace VLAN tag */
    RDPA_VLAN_CMD_BIT_PUSH2,        /* Add 2 VLAN tags */
    RDPA_VLAN_CMD_BIT_PUSH2_ALWAYS, /* Add 2 VLAN tag always (even if packet is double-tagged) */
    RDPA_VLAN_CMD_BIT_POP2,         /* Remove 2 VLAN tags */
    RDPA_VLAN_CMD_BIT_REPLACE2,     /* Replace 2 VLAN tags */
    RDPA_VLAN_CMD_BIT_REMARK,       /* P-Bit re-marking in VLAN tag*/
    RDPA_VLAN_CMD_BIT_REMAP,        /* Re-mapping according to P-bit in VLAN tag */
    RDPA_VLAN_CMD_BIT_DSCP,         /* Remap DSCP -> P-bit in VLAN tag mapping */
    RDPA_VLAN_CMD_BIT_TPID,         /* Replace TPID in VLAN tag */
    RDPA_VLAN_CMD_BIT_TPID_REMARK   /* Remark TPID in VLAN tag */
} rdpa_vlan_cmd_bit;

/** VLAN command bits
 * VLAN command is a combination of RDPA_VLAN_CMD_XX bits
 */
typedef enum {
    /** Transparent action */
    RDPA_VLAN_CMD_TRANSPARENT = 0,
    /** Add VLAN tag */
    RDPA_VLAN_CMD_PUSH = (1 << RDPA_VLAN_CMD_BIT_PUSH),
    /** Add VLAN tag always (even if packet is double-tagged) */
    RDPA_VLAN_CMD_PUSH_ALWAYS = (1 << RDPA_VLAN_CMD_BIT_PUSH_ALWAYS),
    /** Remove VLAN tag */
    RDPA_VLAN_CMD_POP = (1 << RDPA_VLAN_CMD_BIT_POP),
    /** Replace VLAN tag */
    RDPA_VLAN_CMD_REPLACE = (1 << RDPA_VLAN_CMD_BIT_REPLACE),
    /** Add 2 VLAN tags */
    RDPA_VLAN_CMD_PUSH2 = (1 << RDPA_VLAN_CMD_BIT_PUSH2),
    /** Add 2 VLAN tag always (even if packet is double-tagged) */
    RDPA_VLAN_CMD_PUSH2_ALWAYS = (1 << RDPA_VLAN_CMD_BIT_PUSH2_ALWAYS),
    /** Remove 2 VLAN tags */
    RDPA_VLAN_CMD_POP2 = (1 << RDPA_VLAN_CMD_BIT_POP2),
    /** Replace 2 VLAN tags */
    RDPA_VLAN_CMD_REPLACE2 = (1 << RDPA_VLAN_CMD_BIT_REPLACE2),
    /** P-Bit re-marking in VLAN tag*/
    RDPA_VLAN_CMD_REMARK = (1 << RDPA_VLAN_CMD_BIT_REMARK),
    /** Re-mapping according to P-bit in VLAN tag */
    RDPA_VLAN_CMD_REMAP = (1 << RDPA_VLAN_CMD_BIT_REMAP),
    /** Remap DSCP -> P-bit in VLAN tag mapping */
    RDPA_VLAN_CMD_DSCP = (1 << RDPA_VLAN_CMD_BIT_DSCP),
    /** Replace TPID in VLAN tag */
    RDPA_VLAN_CMD_TPID = (1 << RDPA_VLAN_CMD_BIT_TPID),
    /** Remark TPID in VLAN tag */
    RDPA_VLAN_CMD_TPID_REMARK = (1 << RDPA_VLAN_CMD_BIT_TPID_REMARK),
} rdpa_vlan_command;

/** VLAN tag + pbit */
typedef struct {
    uint16_t vid;       /**< VID */
    rdpa_pbit pbit;     /**< PBIT */
    uint16_t tpid;      /**< TPID */
} rdpa_vtag_cmd_parm_t;

/** VLAN action parameters */
typedef struct {
    uint32_t cmd;       /**< Action command - combination of ::rdpa_vlan_command bits */
    rdpa_vtag_cmd_parm_t parm[RDPA_VLAN_MAX_TAGS];    /**< Command parameters */
} rdpa_vlan_action_cfg_t;

/** @} end of vlan_action Doxygen group */

/* VID command groups */
#define RDPA_VLAN_CMD_GROUP_1VID \
    (RDPA_VLAN_CMD_PUSH  | RDPA_VLAN_CMD_PUSH_ALWAYS  | RDPA_VLAN_CMD_POP  | RDPA_VLAN_CMD_REPLACE)
#define RDPA_VLAN_CMD_GROUP_2VID \
    (RDPA_VLAN_CMD_PUSH2 | RDPA_VLAN_CMD_PUSH2_ALWAYS | RDPA_VLAN_CMD_POP2 | RDPA_VLAN_CMD_REPLACE2)

#define RDPA_VLAN_CMD_GROUP_VID \
    (RDPA_VLAN_CMD_GROUP_1VID | RDPA_VLAN_CMD_GROUP_2VID)

#define RDPA_VLAN_CMD_GROUP_PUSH \
    (RDPA_VLAN_CMD_PUSH | RDPA_VLAN_CMD_PUSH_ALWAYS | RDPA_VLAN_CMD_PUSH2 | RDPA_VLAN_CMD_PUSH2_ALWAYS)

/* VID command groups */
#define RDPA_VLAN_CMD_GROUP_PUSH_REPLACE        (RDPA_VLAN_CMD_PUSH | RDPA_VLAN_CMD_REPLACE)
#define RDPA_VLAN_CMD_GROUP_PUSH_ALWAYS_REPLACE (RDPA_VLAN_CMD_PUSH_ALWAYS | RDPA_VLAN_CMD_REPLACE)
#define RDPA_VLAN_CMD_GROUP_POP_REPLACE         (RDPA_VLAN_CMD_POP | RDPA_VLAN_CMD_REPLACE)

/* PBIT command groups */
#define RDPA_VLAN_CMD_GROUP_PBIT (RDPA_VLAN_CMD_REMARK | RDPA_VLAN_CMD_REMAP | RDPA_VLAN_CMD_DSCP)

/* #define __RDPA_VLAN_ACTION_DBG__ */

static inline int vlan_action_is_1tag(uint32_t cmd)
{
    uint32_t vid_cmd = (cmd & RDPA_VLAN_CMD_GROUP_VID);

    /* Verify: Single-tag bits are set */
    if ((vid_cmd & RDPA_VLAN_CMD_GROUP_1VID) != vid_cmd)
        return 0;

    /* Verify: At most one single-tag bit is set */
    if ((vid_cmd & (vid_cmd - 1)) != 0)
        return 0;

    return 1;
}

/* PBIT, TPID replace, remark ? */
static inline int vlan_action_is_0tag_only(uint32_t cmd)
{
    uint32_t vid_cmd = (cmd & RDPA_VLAN_CMD_GROUP_VID);
    return (vid_cmd == 0);
}

#endif /* _RDPA_VLAN_ACTION_H_ */
