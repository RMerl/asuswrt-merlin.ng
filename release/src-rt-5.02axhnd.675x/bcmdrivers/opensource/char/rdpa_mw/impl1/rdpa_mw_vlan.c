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

#if defined(CONFIG_BLOG)

#include <rdpa_api.h>

#include "bcm_OS_Deps.h"
#include <linux/bcm_log.h>
#include <linux/blog_rule.h>
#include "rdpa_mw_vlan.h"

#define SINGLE_TPID_DEFAULT 0x8100
#define DOUBLE_TPID_DEFAULT 0x88a8

#define protoLogDebug bcmLog_logIsEnabled(BCM_LOG_ID_RDPA, BCM_LOG_LEVEL_DEBUG)
#define protoDebug(fmt, arg...) BCM_LOG_DEBUG(BCM_LOG_ID_RDPA, fmt, ##arg)
#define protoInfo(fmt, arg...) BCM_LOG_INFO(BCM_LOG_ID_RDPA, fmt, ##arg)

#define TAGS_MAX 4
typedef struct {
    rdpa_vtag_cmd_parm_t tags[TAGS_MAX];
    int tags_first_offset; /* Can be negative value in case we add vlans */
} rdpa_mw_vlan_tags_t;

static inline int rdpa_vtag_number_of_tags(rdpa_mw_vlan_tags_t *tags)
{
    return 0 - tags->tags_first_offset;
}

static inline rdpa_vtag_cmd_parm_t *rdpa_mw_vlan_tag(rdpa_mw_vlan_tags_t *tags, int i)
{
    int t;
    if (i < 0 || i > TAGS_MAX)
        return NULL;

    t = (tags->tags_first_offset + i) % TAGS_MAX;
    if (t < 0)
        t = TAGS_MAX + t;
    return &tags->tags[t];
}

static void rdpa_mw_vlan_tags_pop(rdpa_mw_vlan_tags_t *tags)
{
    tags->tags_first_offset++;
    rdpa_mw_vlan_tag(tags, TAGS_MAX-1)->vid = rdpa_mw_vlan_tag(tags, TAGS_MAX-1)->pbit = -1;
}

static void rdpa_mw_vlan_tags_push(rdpa_mw_vlan_tags_t *tags, uint16_t vid, rdpa_pbit pbit, uint16_t tpid)
{
    tags->tags_first_offset--;
    rdpa_mw_vlan_tag(tags, 0)->vid = vid;
    rdpa_mw_vlan_tag(tags, 0)->pbit = pbit;
    rdpa_mw_vlan_tag(tags, 0)->tpid = tpid;
}

static void rdpa_mw_vlan_tags_init(rdpa_mw_vlan_tags_t *tags)
{
    int i;
    rdpa_vtag_cmd_parm_t *tag;

    memset(tags, 0, sizeof(*tags));
    tags->tags_first_offset = 0;

    /* -2 is init, -1 is dont care */
    for (i = 0; i < TAGS_MAX; i++)
    {
        tag = rdpa_mw_vlan_tag(tags, i);
        tag->pbit = tag->vid = tag->tpid = -2;
    }
}

static int blog_rule_vlan_action_translate(int num_tags_in_filter, rdpa_mw_vlan_tags_t *tags, rdpa_vlan_action_cfg_t *action)
{
#define HAS(x) ({ typeof(x) t = -1; x != t && x != (t - 1); }) /* test if x has been initialized */
#define COPY_TAG(x) do { action->parm[x] = *tag##x; \
  if (!HAS(tag##x->pbit)) action->parm[x].pbit = 0; \
  if (!HAS(tag##x->vid)) action->parm[x].vid = 0; \
  } while(0)
#define COPY_TAG_TO_TAG(x,y) do { action->parm[y] = *tag##x; if (!HAS(tag##x->pbit)) action->parm[y].pbit = 0; } while(0)
#define REMARK_VID0 (1 << 0)
#define REMARK_PBIT0 (1 << 1)
#define REMARK_VID1 (1 << 2)
#define REMARK_PBIT1 (1 << 3)
#define TAGS_REMARK(tags, r) ((uint8_t)(tags + TAGS_MAX) | (r) << 8)
    rdpa_vtag_cmd_parm_t *tag0 = rdpa_mw_vlan_tag(tags, 0), *tag1 = rdpa_mw_vlan_tag(tags, 1);
    uint8_t remark, num_tags_diff = 0 - tags->tags_first_offset - num_tags_in_filter;

    remark = (HAS(tag0->vid) ? REMARK_VID0 : 0) | (HAS(tag0->pbit) ? REMARK_PBIT0 : 0) |
        (HAS(tag1->vid) ? REMARK_VID1 : 0) | (HAS(tag1->pbit) ? REMARK_PBIT1 : 0);

    protoDebug("num_tags_diff: %d tags_first_offset: %d num_tags_in_filter: %d, remark %x", num_tags_diff, tags->tags_first_offset, num_tags_in_filter, remark);

    switch (TAGS_REMARK(num_tags_diff, remark))
    {
        case TAGS_REMARK(-2, 0):
            action->cmd = RDPA_VLAN_CMD_POP2;
            break; /* BL_LILAC_RDD_VLAN_COMMAND_REMOVE_TWO_TAGS */
        case TAGS_REMARK(-1, 0):
            action->cmd = RDPA_VLAN_CMD_POP;
            break; /* BL_LILAC_RDD_VLAN_COMMAND_REMOVE_TAG */
        case TAGS_REMARK(-1, REMARK_VID0):
            COPY_TAG(0);
            action->cmd = RDPA_VLAN_CMD_POP | RDPA_VLAN_CMD_REPLACE;
            break; /* BL_LILAC_RDD_VLAN_COMMAND_REMOVE_OUTER_TAG_REPLACE_INNER_TAG */
        case TAGS_REMARK(-1, REMARK_VID0 | REMARK_PBIT0):
            COPY_TAG(0);
            action->cmd = RDPA_VLAN_CMD_POP | RDPA_VLAN_CMD_REPLACE | RDPA_VLAN_CMD_REMARK;
            break; /* BL_LILAC_RDD_VLAN_COMMAND_REMOVE_OUTER_TAG_REPLACE_INNER_TAG + REMARK */
        case TAGS_REMARK(0, 0):
            action->cmd = 0;
            break; /* BL_LILAC_RDD_VLAN_COMMAND_TRANSPARENT */
        case TAGS_REMARK(0, REMARK_VID0):
            COPY_TAG(0);
            action->cmd = RDPA_VLAN_CMD_REPLACE;
            break; /* BL_LILAC_RDD_VLAN_COMMAND_REPLACE_TAG */
        case TAGS_REMARK(0, REMARK_PBIT0):
            COPY_TAG(0);
            action->cmd = RDPA_VLAN_CMD_REMARK;
            break;
        case TAGS_REMARK(0, REMARK_VID0 | REMARK_PBIT0):
            COPY_TAG(0);
            action->cmd = RDPA_VLAN_CMD_REPLACE | RDPA_VLAN_CMD_REMARK;
            break; /* BL_LILAC_RDD_VLAN_COMMAND_REPLACE_TAG + REMARK */
        case TAGS_REMARK(0, REMARK_VID0 | REMARK_VID1):
            COPY_TAG(0);
            COPY_TAG(1);
            action->cmd = RDPA_VLAN_CMD_REPLACE2;
            break; /* BL_LILAC_RDD_VLAN_COMMAND_REPLACE_OUTER_TAG_REPLACE_INNER_TAG */
        case TAGS_REMARK(0, REMARK_VID0 | REMARK_PBIT0 | REMARK_VID1 | REMARK_PBIT1):
            COPY_TAG(0);
            COPY_TAG(1);
            action->cmd = RDPA_VLAN_CMD_REPLACE2 | RDPA_VLAN_CMD_REMARK;
            break; /* BL_LILAC_RDD_VLAN_COMMAND_REPLACE_OUTER_TAG_REPLACE_INNER_TAG + REMARK */
        case TAGS_REMARK(1, REMARK_VID0 | REMARK_PBIT0):
#ifdef XRDP
            if (!HAS(tag0->tpid))
                tag0->tpid = SINGLE_TPID_DEFAULT;
#endif

            COPY_TAG(0);
            action->cmd = RDPA_VLAN_CMD_PUSH_ALWAYS | RDPA_VLAN_CMD_REMARK;
            break; /* BL_LILAC_RDD_VLAN_COMMAND_ADD_TAG_ALWAYS + REMARK */
        case TAGS_REMARK(1, REMARK_VID0):
#ifdef XRDP
            if (!HAS(tag0->tpid))
                tag0->tpid = SINGLE_TPID_DEFAULT;
#endif

            COPY_TAG(0);
            action->cmd = RDPA_VLAN_CMD_PUSH_ALWAYS;
            break;
        case TAGS_REMARK(1, REMARK_VID0 | REMARK_PBIT0 | REMARK_VID1 | REMARK_PBIT1):
#ifdef XRDP
            if (!HAS(tag0->tpid))
                tag0->tpid = SINGLE_TPID_DEFAULT;
            if (!HAS(tag1->tpid))
                tag1->tpid = SINGLE_TPID_DEFAULT;
#endif

            COPY_TAG(0);
            COPY_TAG(1);
            action->cmd = RDPA_VLAN_CMD_PUSH | RDPA_VLAN_CMD_REPLACE | RDPA_VLAN_CMD_REMARK;
            break;
        case TAGS_REMARK(1, REMARK_VID0 | REMARK_VID1 | REMARK_PBIT0):
#ifdef XRDP
            if (!HAS(tag0->tpid))
                tag0->tpid = DOUBLE_TPID_DEFAULT;
            if (!HAS(tag1->tpid))
                tag1->tpid = SINGLE_TPID_DEFAULT;
#endif
            COPY_TAG(0);
            COPY_TAG(1);
            action->cmd = RDPA_VLAN_CMD_PUSH | RDPA_VLAN_CMD_REPLACE;
            break;
        case TAGS_REMARK(2, 0):
#ifdef XRDP
            if (!HAS(tag0->tpid))
                tag0->tpid = DOUBLE_TPID_DEFAULT;
            if (!HAS(tag1->tpid))
                tag1->tpid = SINGLE_TPID_DEFAULT;
#endif

            COPY_TAG(0);
            COPY_TAG(1);
            action->cmd = RDPA_VLAN_CMD_PUSH2 | RDPA_VLAN_CMD_REMARK;
            break; /* BL_LILAC_RDD_VLAN_COMMAND_ADD_TWO_TAGS + REMARK */
        case TAGS_REMARK(2, REMARK_VID0 | REMARK_PBIT0 | REMARK_VID1 | REMARK_PBIT1):
#ifdef XRDP
            if (!HAS(tag0->tpid))
                tag0->tpid = DOUBLE_TPID_DEFAULT;
#endif

            COPY_TAG(0);
            COPY_TAG(1);
            action->cmd = RDPA_VLAN_CMD_PUSH2| RDPA_VLAN_CMD_REPLACE | RDPA_VLAN_CMD_REMARK;
            break; /* BL_LILAC_RDD_VLAN_COMMAND_ADD_OUTER_TAG_REPLACE_INNER_TAG + REMARK */

        default:
            protoInfo("vlan_action not supported remark<0x%x>", remark);
            if (protoLogDebug)
            {
                int i;

                for (i = 0; i < TAGS_MAX; i++)
                    protoDebug("%d) vid<%2d> pbit<%2d>", i, rdpa_mw_vlan_tag(tags, i)->vid, rdpa_mw_vlan_tag(tags, i)->pbit);
            }
            return -1;
    }

    if (HAS(tag0->tpid) || HAS(tag1->tpid))
    {
        action->cmd |= RDPA_VLAN_CMD_TPID_REMARK;

        if (HAS(tag0->tpid))
            action->parm[0].tpid = tag0->tpid;
        else
            action->parm[0].tpid = RDPA_VLAN_ACTION_TPID_DONT_CARE;

        if (HAS(tag1->tpid))
            action->parm[1].tpid = tag1->tpid;
        else
            action->parm[1].tpid = RDPA_VLAN_ACTION_TPID_DONT_CARE;
    }

    protoDebug("result vlan cmd: %x", action->cmd);

    return 0;
}

static int blog_rule_vlan_action_process_actions(blogRule_t *blog_rule, rdpa_mw_vlan_tags_t *tags)
{
    int i;

    for (i = 0; i < blog_rule->filter.nbrOfVlanTags; i++)
        rdpa_mw_vlan_tags_push(tags, -1, -1, -1);

    for (i = 0; i < blog_rule->actionCount; i++)
    {
        blogRuleCommand_t cmd = blog_rule->action[i].cmd;

        switch (cmd)
        {
            case BLOG_RULE_CMD_PUSH_VLAN_HDR:
                /* Defaults are based on bcmdrivers/broadcom/include/bcm963xx/bcm_vlan.h */
		rdpa_mw_vlan_tags_push(tags, 1, 0, ETH_P_8021Q);
                break;
            case BLOG_RULE_CMD_POP_VLAN_HDR:
                rdpa_mw_vlan_tags_pop(tags);
                break;
            case BLOG_RULE_CMD_SET_VID:
            case BLOG_RULE_CMD_SET_PBITS:
            case BLOG_RULE_CMD_SET_ETHERTYPE:
            case BLOG_RULE_CMD_SET_VLAN_PROTO:
                if (rdpa_vtag_number_of_tags(tags) <= blog_rule->action[i].toTag)
                {
                    BDMF_TRACE_ERR("Setting non-exist VLAN tag %d. Number of VLAN tags = %d!",
                        blog_rule->action[i].toTag + 1, rdpa_vtag_number_of_tags(tags));
                    return -1;
                }

                if (cmd == BLOG_RULE_CMD_SET_VID)
                    rdpa_mw_vlan_tag(tags, blog_rule->action[i].toTag)->vid = blog_rule->action[i].vid;
                else if (cmd == BLOG_RULE_CMD_SET_PBITS)
                    rdpa_mw_vlan_tag(tags, blog_rule->action[i].toTag)->pbit = blog_rule->action[i].pbits;
                else /* TPID */
                {
                    if (!blog_rule->action[i].toTag && !rdpa_vtag_number_of_tags(tags))
                    {
                        BDMF_TRACE_ERR("Cannot set ethertype on non-tagged flow\n");
                        return -1;
                    }
                    else
                        rdpa_mw_vlan_tag(tags, blog_rule->action[i].toTag)->tpid = blog_rule->action[i].tpid;
                }
                break;
            case BLOG_RULE_CMD_COPY_VID:
                protoInfo("Copy VID action is not supported");
                break;
            case BLOG_RULE_CMD_COPY_PBITS:
                if ((blog_rule->action[i].toTag != 0) || (!rdpa_vtag_number_of_tags(tags)))
                {
                    protoInfo("Copy pbit action not supported: number of VLAN tags = %d,"
                      "copy to VLAN tag %d", 
                      rdpa_vtag_number_of_tags(tags), blog_rule->action[i].toTag + 1);
                }
                else
                {
                    /* Clear the pbit set by BLOG_RULE_CMD_PUSH_VLAN_HDR. */
                    rdpa_mw_vlan_tag(tags, 0)->pbit = -1;
                }
                break;
            default:
                /* Other actions (non VLAN-related) may be handled by other action
                 * handlers. */
                break;
        }	
    }

    return 0;
}

static int vlan_action_add(rdpa_vlan_action_cfg_t *action, rdpa_traffic_dir dir, bdmf_object_handle *vlan_action_obj)
{
    int rc;
    BDMF_MATTR(vlan_action_attr, rdpa_vlan_action_drv());

    rc = rdpa_vlan_action_dir_set(vlan_action_attr, dir);
    rc = rc ? : rdpa_vlan_action_action_set(vlan_action_attr, action);
    /* index will be picked automatically by rdpa */
    rc = rc ? : bdmf_new_and_set(rdpa_vlan_action_drv(), NULL, vlan_action_attr, vlan_action_obj);

    return rc;
}

static void vlan_action_find(rdpa_vlan_action_cfg_t *action, rdpa_traffic_dir dir, bdmf_object_handle *vlan_action_obj)
{
    bdmf_object_handle obj = NULL;
    rdpa_vlan_action_cfg_t action_tmp;
    rdpa_traffic_dir dir_tmp;

    *vlan_action_obj = NULL;

    while ((obj = bdmf_get_next(rdpa_vlan_action_drv(), obj, NULL)))
    {
        rdpa_vlan_action_dir_get(obj, &dir_tmp);
        if (dir != dir_tmp)
            continue;

        rdpa_vlan_action_action_get(obj, &action_tmp);
        if (!memcmp(action, &action_tmp, sizeof(action_tmp)))
        {
            *vlan_action_obj = obj;
            return;
        }
    }
}

int blog_rule_to_vlan_action(blogRule_t *blog_rule, rdpa_traffic_dir dir, bdmf_object_handle *vlan_action_obj)
{
    rdpa_vlan_action_cfg_t action = {};
    rdpa_mw_vlan_tags_t tags;
    int rc, reuse = 1;
    bdmf_number idx;

    *vlan_action_obj = NULL;
    rdpa_mw_vlan_tags_init(&tags);
    rc = blog_rule_vlan_action_process_actions(blog_rule, &tags);
    rc = rc ? : blog_rule_vlan_action_translate(blog_rule->filter.nbrOfVlanTags, &tags, &action);
    if (rc < 0)
        return rc;

    vlan_action_find(&action, dir, vlan_action_obj);
    if (*vlan_action_obj)
        goto Exit;

    reuse = 0;
    rc = vlan_action_add(&action, dir, vlan_action_obj);
    if (rc < 0)
        BDMF_TRACE_RET(rc, "Failed to create vlan action object\n");

Exit:
    rdpa_vlan_action_index_get(*vlan_action_obj, &idx);
    protoDebug("%s vlan_object %d", reuse ? "Reusing" : "Created", (int)idx);
    return 0;
}
#endif /* CONFIG_BLOG */

