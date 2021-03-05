/***********************************************************************
 *
 *  Copyright (c) 2007  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2012:proprietary:standard

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
 *
 ************************************************************************/
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include "iqctl_common.h"
#include "iqctl_api.h"


/*
 *------------------------------------------------------------------------------
 * Function Name: iqctl_open
 * Description  : Opens the ingress QoS device.
 * Returns      : device handle if successsful or -1 if error
 *------------------------------------------------------------------------------
 */
static int iqctl_open(void)
{
    int nFd = open( IQ_DRV_DEVICE_NAME, O_RDWR );
    if ( nFd == -1 )
    {
        fprintf( stderr, "open <%s> error no %d\n",
                 IQ_DRV_DEVICE_NAME, errno );
        return IQCTL_ERROR;
    }
    return nFd;
} /* iqctl_open */

/*
 *------------------------------------------------------------------------------
 * Function Name: iqctl_ioctl
 * Description  : Ioctls into Ingress Qos driver passing the IOCTL command, 
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
static int iqctl_ioctl(iqctl_ioctl_t ioctl_cmd, void *arg)
{
    int devFd, ret = IQCTL_ERROR;

    if ( (devFd = iqctl_open() ) == IQCTL_ERROR )
        return IQCTL_ERROR;

    if ( (ret = ioctl( devFd, ioctl_cmd, (uintptr_t) arg )) == IQCTL_ERROR )
        fprintf( stderr, "iqctl_ioctl <%d> error\n", ioctl_cmd );

    close( devFd );
    return ret;
}

int bcm_iqctl_add_keymask( iqctl_data_t *iq )
{
    int err = 0;

    if (iq == NULL)
        return IQCTL_ERROR;

    iq->subsys = IQCTL_SUBSYS_KEYMASK;
    iq->op = IQCTL_OP_ADD;

    if ((err = iqctl_ioctl(IQCTL_IOCTL_SYS, (void *) iq))) {
        if (err == IQCTL_ERROR)
            fprintf(stderr, "ioctl command return error %d!\n", errno);
    }
    return err;
}

int bcm_iqctl_rem_keymask( iqctl_data_t *iq )
{
    int err = 0;

    if (iq == NULL)
        return IQCTL_ERROR;

    iq->subsys = IQCTL_SUBSYS_KEYMASK;
    iq->op = IQCTL_OP_REM;

    if ((err = iqctl_ioctl(IQCTL_IOCTL_SYS, (void *) iq))) {
        if (err == IQCTL_ERROR)
            fprintf(stderr, "ioctl command return error %d!\n", errno);
    }
    return err;
}

int bcm_iqctl_dump_keymasktbl( void )
{
    int err = 0, idx = 0, f_first = 1, i;
    iqctl_data_t iqdata;
    iqctl_data_t *iq = &iqdata;

    iq->subsys = IQCTL_SUBSYS_KEYMASK;
    iq->op = IQCTL_OP_GETNEXT;
    do {
        iq->nextIx = idx;

        err = iqctl_ioctl(IQCTL_IOCTL_SYS, (void *) iq);

        if (err == IQCTL_ERROR)
        {
            fprintf(stderr, "ioctl command return error %d!\n", errno);
            return err;
        }

        if (iq->nextIx == -1)
            break;

        /* if reach here, means it has got a valid entry at the iq->nextIx */
        if (f_first)
        {
            f_first = 0;
            fprintf(stdout, " keymask table:\n");
        }

        fprintf(stdout, "\tindex#%d: prio = %d, ", iq->nextIx, iq->prio);
        fprintf(stdout, "refcnt = %d\n", iq->refcnt);
        fprintf(stdout, "\t\tmask = ");
        for (i = 0; i < IQCTL_KEY_FIELD_MAX; i++)
        {
            if (iq->key_data.key_field_mask & (0x1 << i))
            {
                switch (i)
		{
                case IQCTL_KEY_FIELD_INGRESS_DEVICE:
                    fprintf(stdout, "--indev ");
                    break;
                case IQCTL_KEY_FIELD_SRC_MAC:
                    fprintf(stdout, "--srcmac ");
                    break;
                case IQCTL_KEY_FIELD_DST_MAC:
                    fprintf(stdout, "--dstmac ");
                    break;
                case IQCTL_KEY_FIELD_ETHER_TYPE:
                    fprintf(stdout, "--ethtype ");
                    break;
                case IQCTL_KEY_FIELD_OUTER_VID:
                    fprintf(stdout, "--outervid ");
                    break;
                case IQCTL_KEY_FIELD_OUTER_PBIT:
                    fprintf(stdout, "--outerpbit ");
                    break;
                case IQCTL_KEY_FIELD_INNER_VID:
                    fprintf(stdout, "--innervid ");
                    break;
                case IQCTL_KEY_FIELD_INNER_PBIT:
                    fprintf(stdout, "--innerpbit ");
                    break;
                case IQCTL_KEY_FIELD_L2_PROTO:
                    fprintf(stdout, "--l2proto ");
                    break;
                case IQCTL_KEY_FIELD_L3_PROTO:
                    fprintf(stdout, "--l3proto ");
                    break;
                case IQCTL_KEY_FIELD_IP_PROTO:
                    fprintf(stdout, "--ipproto ");
                    break;
                case IQCTL_KEY_FIELD_SRC_IP:
                    fprintf(stdout, "--srcip ");
                    break;
                case IQCTL_KEY_FIELD_DST_IP:
                    fprintf(stdout, "--dstip ");
                    break;
                case IQCTL_KEY_FIELD_DSCP:
                    fprintf(stdout, "--dscp ");
                    break;
                case IQCTL_KEY_FIELD_IPV6_FLOW_LABEL:
                    fprintf(stdout, "--ipv6flowlabel ");
                    break;
                case IQCTL_KEY_FIELD_SRC_PORT:
                    fprintf(stdout, "--srcport ");
                    break;
                case IQCTL_KEY_FIELD_DST_PORT:
                    fprintf(stdout, "--dstport ");
                    break;
                case IQCTL_KEY_FIELD_OFFSET_0:
                    fprintf(stdout, "--outset0 ");
                    /* FIXME!! more on the rest of info */
                    break;
                case IQCTL_KEY_FIELD_OFFSET_1:
                    fprintf(stdout, "--outset1 ");
                    /* FIXME!! more on the rest of info */
                    break;
		default:
                    break;
		}
            }
        }
        fprintf(stdout, "\n");
        idx = iq->nextIx + 1;
    } while (err == 0);

    /* means no keymask entry found */
    if (f_first)
        fprintf(stdout, "Keymask table is empty.\n");

    return 0;
}

int bcm_iqctl_add_key( iqctl_data_t *iq )
{
    int err = 0;

    if (iq == NULL)
        return IQCTL_ERROR;

    iq->subsys = IQCTL_SUBSYS_KEY;
    iq->op = IQCTL_OP_ADD;

    if ((err = iqctl_ioctl(IQCTL_IOCTL_SYS, (void *)iq))) {
        if (err == IQCTL_ERROR)
            fprintf(stderr, "ioctl command return error %d!\n", errno);
        return err;
    }

    if (iq->rc)
        return iq->rc;

    return err;
}

int bcm_iqctl_rem_key( iqctl_data_t *iq )
{
    int err = 0;

    if (iq == NULL)
        return IQCTL_ERROR;

    iq->subsys = IQCTL_SUBSYS_KEY;
    iq->op = IQCTL_OP_REM;

    if ((err = iqctl_ioctl(IQCTL_IOCTL_SYS, (void *) iq))) {
        if (err == IQCTL_ERROR)
            fprintf(stderr, "ioctl command return error %d!\n", errno);
        return err;
    }

    if (iq->rc)
        return iq->rc;

    return err;
}

int bcm_iqctl_get_key( iqctl_data_t *iq )
{
    int err = 0;

    if (iq == NULL)
        return IQCTL_ERROR;

    iq->subsys = IQCTL_SUBSYS_KEY;
    iq->op = IQCTL_OP_GET;

    if ((err = iqctl_ioctl(IQCTL_IOCTL_SYS, (void *) iq))) {
        if (err == IQCTL_ERROR)
            fprintf(stderr, "ioctl command return error %d!\n", errno);
        return err;
    }

    if (iq->rc)
        return iq->rc;

    return err;
}

void bcm_iqctl_print_key( iqctl_data_t *iq )
{
    int i;

    fprintf(stdout, "\tindex#%d: ", iq->nextIx, iq->prio);
    fprintf(stdout, "%s, ", (iq->ent == IQCTL_ENT_STAT)? "static" : "dynamic");
    fprintf(stdout, "refcnt = %d, ", iq->refcnt);
    fprintf(stdout, "SW hitcnt = %d\n", iq->hitcnt);
    fprintf(stdout, "\t\tfield+value = ");
    for (i = 0; i < IQCTL_KEY_FIELD_MAX; i++)
    {
        if (iq->key_data.key_field_mask & (0x1 << i))
        {
            switch (i)
            {
            case IQCTL_KEY_FIELD_INGRESS_DEVICE:
                // FIXME!! clean up indev implementation
                fprintf(stdout, "--indev %d ", iq->key_data.ingress_device);
                break;
            case IQCTL_KEY_FIELD_SRC_MAC:
                fprintf(stdout, "--srcmac %02x:%02x:%02x:%02x:%02x:%02x ",
                        iq->key_data.src_mac[0], iq->key_data.src_mac[1],
                        iq->key_data.src_mac[2], iq->key_data.src_mac[3],
                        iq->key_data.src_mac[4], iq->key_data.src_mac[5]);
                break;
            case IQCTL_KEY_FIELD_DST_MAC:
                fprintf(stdout, "--dstmac %02x:%02x:%02x:%02x:%02x:%02x ",
                        iq->key_data.dst_mac[0], iq->key_data.dst_mac[1],
                        iq->key_data.dst_mac[2], iq->key_data.dst_mac[3],
                        iq->key_data.dst_mac[4], iq->key_data.dst_mac[5]);
                break;
            case IQCTL_KEY_FIELD_ETHER_TYPE:
                fprintf(stdout, "--ethtype 0x%04x ", iq->key_data.eth_type);
                break;
            case IQCTL_KEY_FIELD_OUTER_VID:
                fprintf(stdout, "--outervid %d ", iq->key_data.outer_vid);
                break;
            case IQCTL_KEY_FIELD_OUTER_PBIT:
                fprintf(stdout, "--outerpbit 0x%02x ", iq->key_data.outer_pbit);
                break;
            case IQCTL_KEY_FIELD_INNER_VID:
                fprintf(stdout, "--innervid %d ", iq->key_data.inner_vid);
                break;
            case IQCTL_KEY_FIELD_INNER_PBIT:
                fprintf(stdout, "--innerpbit 0x%02x ", iq->key_data.inner_pbit);
                break;
            case IQCTL_KEY_FIELD_L2_PROTO:
                fprintf(stdout, "--l2proto %d ", iq->key_data.l2_proto);
                break;
            case IQCTL_KEY_FIELD_L3_PROTO:
                fprintf(stdout, "--l3proto 0x%04x ", iq->key_data.l3_proto);
                break;
            case IQCTL_KEY_FIELD_IP_PROTO:
                fprintf(stdout, "--ipproto %d ", iq->key_data.ip_proto);
                break;
            case IQCTL_KEY_FIELD_SRC_IP:
                if (iq->key_data.is_ipv6 == 1)
                {
                    fprintf(stdout, "--srcip %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x ",
                            (iq->key_data.src_ip[0] >> 16), (iq->key_data.src_ip[0] & 0xffff),
                            (iq->key_data.src_ip[1] >> 16), (iq->key_data.src_ip[1] & 0xffff),
                            (iq->key_data.src_ip[2] >> 16), (iq->key_data.src_ip[2] & 0xffff),
                            (iq->key_data.src_ip[3] >> 16), (iq->key_data.src_ip[3] & 0xffff));
                }
                else
                {
                    fprintf(stdout, "--srcip %d.%d.%d.%d ",
                            ((iq->key_data.src_ip[0] >> 24) & 0xff),
                            ((iq->key_data.src_ip[0] >> 16) & 0xff),
                            ((iq->key_data.src_ip[0] >> 8) & 0xff),
                            (iq->key_data.src_ip[0] & 0xff));
                }
                break;
            case IQCTL_KEY_FIELD_DST_IP:
                if (iq->key_data.is_ipv6 == 1)
                {
                    fprintf(stdout, "--dstip %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x ",
                            (iq->key_data.dst_ip[0] >> 16), (iq->key_data.dst_ip[0] & 0xffff),
                            (iq->key_data.dst_ip[1] >> 16), (iq->key_data.dst_ip[1] & 0xffff),
                            (iq->key_data.dst_ip[2] >> 16), (iq->key_data.dst_ip[2] & 0xffff),
                            (iq->key_data.dst_ip[3] >> 16), (iq->key_data.dst_ip[3] & 0xffff));
                }
                else
                {
                    fprintf(stdout, "--dstip %d.%d.%d.%d ",
                            ((iq->key_data.dst_ip[0] >> 24) & 0xff),
                            ((iq->key_data.dst_ip[0] >> 16) & 0xff),
                            ((iq->key_data.dst_ip[0] >> 8) & 0xff),
                            (iq->key_data.dst_ip[0] & 0xff));
                }
                break;
            case IQCTL_KEY_FIELD_DSCP:
                fprintf(stdout, "--dscp %d ", iq->key_data.dscp);
                break;
            case IQCTL_KEY_FIELD_IPV6_FLOW_LABEL:
                fprintf(stdout, "--ipv6flowlabel %d ", iq->key_data.flow_label);
                break;
            case IQCTL_KEY_FIELD_SRC_PORT:
                fprintf(stdout, "--srcport %d ", iq->key_data.l4_src_port);
                break;
            case IQCTL_KEY_FIELD_DST_PORT:
                fprintf(stdout, "--dstport %d ", iq->key_data.l4_dst_port);
                break;
            case IQCTL_KEY_FIELD_OFFSET_0:
                fprintf(stdout, "--outset0 ");
                /* FIXME!! more on the rest of info */
                break;
            case IQCTL_KEY_FIELD_OFFSET_1:
                fprintf(stdout, "--outset1 ");
                /* FIXME!! more on the rest of info */
                break;
            default:
                break;

            }
        }
    }

    fprintf(stdout, "\n\t\taction = ");
    switch (iq->action)
    {
    case IQCTL_ACTION_PRIO:
        fprintf(stdout, "prio = %d\n", iq->action_value);
	break;
    case IQCTL_ACTION_DROP:
        fprintf(stdout, "drop\n");
	break;
    case IQCTL_ACTION_DST_Q:
        fprintf(stdout, "dstqueue = %d\n", iq->action_value);
	break;
    case IQCTL_ACTION_TRAP:
        fprintf(stdout, "trap\n");
	break;
    default:
        fprintf(stdout, "NOP or N/A\n");
	break;
    }
}

int bcm_iqctl_dump_keytbl( void )
{
    int err = 0, idx = 0, f_first = 1;
    iqctl_data_t iqdata;
    iqctl_data_t *iq = &iqdata;

    do {
        memset(iq, 0x0, sizeof(iqctl_data_t));
        iq->subsys = IQCTL_SUBSYS_KEY;
        iq->op = IQCTL_OP_GETNEXT;
        iq->nextIx = idx;

        err = iqctl_ioctl(IQCTL_IOCTL_SYS, (void *) iq);

        if (err == IQCTL_ERROR)
        {
            fprintf(stderr, "ioctl command return error %d!\n", errno);
            return err;
        }

        if (iq->nextIx == -1)
            break;

        /* if reach here, means it has got a valid entry at the iq->nextIx */
        if (f_first)
        {
            f_first = 0;
            fprintf(stdout, " key table:\n");
        }

        bcm_iqctl_print_key(iq);
        idx = iq->nextIx + 1;
    } while (err == 0);

    /* means no keymask entry found */
    if (f_first)
        fprintf(stdout, "Keymask table is empty.\n");

    return 0;
}

int bcm_iqctl_dump_all( void )
{
    int err = 0;
    iqctl_data_t iqdata;
    iqctl_data_t *iq = &iqdata;

    iq->subsys = IQCTL_SUBSYS_STATUS;
    iq->op = IQCTL_OP_GET;
    if ((err = iqctl_ioctl(IQCTL_IOCTL_SYS, (void *)iq)))
    {
        if (err == IQCTL_ERROR)
            fprintf(stderr, "ioctl command return error %d!\n", errno);
        return err;
    }
    fprintf(stdout, "BCM Ingress QOS\n");
    fprintf(stdout, "\tStatus: %s\n", (iq->status != 0)? "enabled" : "disabled");
    fprintf(stdout, "\tTotal Static Entry Count: %d\n", iq->stacnt);
    fprintf(stdout, "\tTotal Dynamic Entry Count: %d\n\n", iq->dyncnt);

    err = bcm_iqctl_dump_keymasktbl();
    if (err)
        return err;

    fprintf(stdout, "\n");
    err = bcm_iqctl_dump_keytbl();

    return err;
}

int bcm_iqctl_dump_porttbl_kernel_mode( void )
{
    int err = 0;
    iqctl_data_t iqdata;
    iqctl_data_t *iq = &iqdata;

    iq->subsys = IQCTL_SUBSYS_KEY;
    iq->op = IQCTL_OP_DUMP;

    if ((err = iqctl_ioctl(IQCTL_IOCTL_SYS, (void *) iq))) {
        if (err == IQCTL_ERROR)
            fprintf(stderr, "ioctl command return error %d!\n", errno);
    }

    return err;
}

int bcm_iqctl_flush( void )
{
    int err = 0;
    iqctl_data_t iqdata;
    iqctl_data_t *iq = &iqdata;

    iq->subsys = IQCTL_SUBSYS_KEY;
    iq->op = IQCTL_OP_FLUSH;

    if ((err = iqctl_ioctl(IQCTL_IOCTL_SYS, (void *) iq))) {
        if (err == IQCTL_ERROR)
            fprintf(stderr, "ioctl command return error %d!\n", errno);
    }

    return err;
}


int bcm_iqctl_set_status( iqctl_status_t status )
{
    int err = 0;
    iqctl_data_t iqdata;
    iqctl_data_t *iq = &iqdata;

    if (status >= IQCTL_STATUS_MAX)
    {
        fprintf(stderr, "ioctl command invalid param\n");
        err = IQCTL_ERROR;
        return err;
    }

    iq->subsys = IQCTL_SUBSYS_STATUS;
    iq->op = IQCTL_OP_SET;
    iq->status = status;

    if ((err = iqctl_ioctl(IQCTL_IOCTL_SYS, (void *) iq))) {
        if (err == IQCTL_ERROR)
            fprintf(stderr, "ioctl command return error %d!\n", errno);
    }

    return err;
}

int bcm_iqctl_get_status( iqctl_status_t *status_p )
{
    int err = 0;
    iqctl_data_t iqdata;
    iqctl_data_t *iq = &iqdata;

    if (status_p == NULL)
    {
        fprintf(stderr, "ioctl command invalid param\n");
        err = IQCTL_ERROR;
        return err;
    }

    iq->subsys = IQCTL_SUBSYS_STATUS;
    iq->op = IQCTL_OP_GET;
    if ((err = iqctl_ioctl(IQCTL_IOCTL_SYS, (void *) iq))) {
        if (err == IQCTL_ERROR)
            fprintf(stderr, "ioctl command return error %d!\n", errno);
        goto out;
    }

    *status_p = iq->status;

out:
    return err;
}

int bcm_iqctl_set_hw_accel_cong_ctrl( iqctl_status_t status )
{
    int err = 0;
    iqctl_data_t iqdata;
    iqctl_data_t *iq = &iqdata;

    if (status >= IQCTL_STATUS_MAX)
    {
        fprintf(stderr, "ioctl command invalid param\n");
        err = IQCTL_ERROR;
        return err;
    }

    iq->subsys = IQCTL_SUBSYS_HW_ACCEL_CONG_CTRL;
    iq->op = IQCTL_OP_SET;
    iq->status = status;

    if ((err = iqctl_ioctl(IQCTL_IOCTL_SYS, (void *) iq))) {
        if (err == IQCTL_ERROR)
            fprintf(stderr, "ioctl command return error %d!\n", errno);
    } else if (iq->rc < 0)
        fprintf(stderr, "command failed %d!\n", iq->rc);

    return err;
}

int bcm_iqctl_get_hw_accel_cong_ctrl( iqctl_status_t *status_p )
{
    int err = 0;
    iqctl_data_t iqdata;
    iqctl_data_t *iq = &iqdata;

    if (status_p == NULL)
    {
        fprintf(stderr, "ioctl command invalid param\n");
        err = IQCTL_ERROR;
        return err;
    }

    iq->subsys = IQCTL_SUBSYS_HW_ACCEL_CONG_CTRL;
    iq->op = IQCTL_OP_GET;
    if ((err = iqctl_ioctl(IQCTL_IOCTL_SYS, (void *) iq))) {
        if (err == IQCTL_ERROR)
            fprintf(stderr, "ioctl command return error %d!\n", errno);
        goto out;
    } else if (iq->rc < 0)
        fprintf(stderr, "command failed %d!\n", iq->rc);

    *status_p = iq->status;

out:
    return err;
}

int bcm_iqctl_dump_status_kernel_mode( void )
{
    int err = 0;
    iqctl_data_t iqdata;
    iqctl_data_t *iq = &iqdata;

    iq->subsys = IQCTL_SUBSYS_STATUS;
    iq->op = IQCTL_OP_DUMP;

    if ((err = iqctl_ioctl(IQCTL_IOCTL_SYS, (void *) iq))) {
        if (err == IQCTL_ERROR)
            fprintf(stderr, "ioctl command return error %d!\n", errno);
    }

    return err;
}

/* old API. for backward compatibility */
int bcm_iqctl_add_port( iqctl_proto_t proto, int dport, iqctl_ent_t ent, 
        iqctl_prio_t prio )
{
    iqctl_data_t iqdata;

    if ((proto >= IQCTL_PROTO_MAX) || (ent >= IQCTL_ENT_MAX) ||
        (prio >= IQCTL_PRIO_MAX))
        return -EINVAL;

    memset(&iqdata, 0x0, sizeof(iqctl_data_t));
    iqdata.ent = ent;
    iqdata.action = IQCTL_ACTION_PRIO;
    iqdata.action_value = prio;

    iqdata.key_data.key_field_mask |= 0x1 << IQCTL_KEY_FIELD_IP_PROTO;
    if (proto == IQCTL_PROTO_TCP)
        iqdata.key_data.ip_proto = IPPROTO_TCP; //6;
    else if (proto == IQCTL_PROTO_UDP)
        iqdata.key_data.ip_proto = IPPROTO_UDP; //17;
    else
        return -EINVAL;

    iqdata.key_data.key_field_mask |= 0x1 << IQCTL_KEY_FIELD_DST_PORT;
    iqdata.key_data.l4_dst_port = dport;

    return bcm_iqctl_add_key(&iqdata);
}

/* old API. for backward compatibility */
int bcm_iqctl_rem_port( iqctl_proto_t proto, int dport, iqctl_ent_t ent )
{
    iqctl_data_t iqdata;

    if ((proto >= IQCTL_PROTO_MAX) || (ent >= IQCTL_ENT_MAX))
        return -EINVAL;

    memset(&iqdata, 0x0, sizeof(iqctl_data_t));
    iqdata.ent = ent;

    iqdata.key_data.key_field_mask |= 0x1 << IQCTL_KEY_FIELD_IP_PROTO;
    if (proto == IQCTL_PROTO_TCP)
        iqdata.key_data.ip_proto = IPPROTO_TCP; //6;
    else if (proto == IQCTL_PROTO_UDP)
        iqdata.key_data.ip_proto = IPPROTO_UDP; //17;
    else
        return -EINVAL;

    iqdata.key_data.key_field_mask |= 0x1 << IQCTL_KEY_FIELD_DST_PORT;
    iqdata.key_data.l4_dst_port = dport;

    return bcm_iqctl_rem_key(&iqdata);
}

/* old API. for backward compatibility */
int bcm_iqctl_get_port( iqctl_proto_t proto, int dport, iqctl_ent_t *ent_p,
        iqctl_prio_t *prio_p )
{
    iqctl_data_t iqdata;
    int err = 0;

    if (proto >= IQCTL_PROTO_MAX)
        return -EINVAL;

    if ((ent_p == NULL) || (prio_p == NULL))
        return -EINVAL;

    memset(&iqdata, 0x0, sizeof(iqctl_data_t));

    iqdata.key_data.key_field_mask |= 0x1 << IQCTL_KEY_FIELD_IP_PROTO;
    if (proto == IQCTL_PROTO_TCP)
        iqdata.key_data.ip_proto = IPPROTO_TCP; //6;
    else if (proto == IQCTL_PROTO_UDP)
        iqdata.key_data.ip_proto = IPPROTO_UDP; //17;
    else
        return -EINVAL;

    iqdata.key_data.key_field_mask |= 0x1 << IQCTL_KEY_FIELD_DST_PORT;
    iqdata.key_data.l4_dst_port = dport;

    err = bcm_iqctl_get_key(&iqdata);
    if (err)
        return err;

    if (iqdata.action != IQCTL_ACTION_PRIO)
        return -ENOENT;

    *ent_p = iqdata.ent;
    *prio_p = iqdata.action_value;

    return err;
}

/* old API. for backward compatibility */
int bcm_iqctl_dump_porttbl( iqctl_proto_t proto )
{
    return bcm_iqctl_dump_all();
}

/* old API. for backward compatibility */
int bcm_iqctl_flush_porttbl( iqctl_proto_t proto, iqctl_ent_t ent )
{
    return bcm_iqctl_flush();
}

/* old API. for backward compatibility */
int bcm_iqctl_set_defaultprio( iqctl_prototype_t prototype, int protoval, iqctl_prio_t prio )
{
    iqctl_data_t iqdata;

    if ((prototype >= IQCTL_PROTOTYPE_MAX) || (prio >= IQCTL_PRIO_MAX))
        return -EINVAL;

    memset(&iqdata, 0x0, sizeof(iqctl_data_t));
    iqdata.ent = IQCTL_ENT_STAT;
    iqdata.action = IQCTL_ACTION_PRIO;
    iqdata.action_value = prio;

    if (prototype == IQCTL_PROTOTYPE_IP)
        iqdata.key_data.key_field_mask |= 0x1 << IQCTL_KEY_FIELD_IP_PROTO;
    else
        return -EINVAL;

    iqdata.key_data.ip_proto = protoval;

    return bcm_iqctl_add_key(&iqdata);
}

/* old API. for backward compatibility */
int bcm_iqctl_rem_defaultprio( iqctl_prototype_t prototype, int protoval )
{
    iqctl_data_t iqdata;

    if (prototype >= IQCTL_PROTOTYPE_MAX)
        return -EINVAL;

    memset(&iqdata, 0x0, sizeof(iqctl_data_t));
    iqdata.ent = IQCTL_ENT_STAT;
    iqdata.action = IQCTL_ACTION_PRIO;

    if (prototype == IQCTL_PROTOTYPE_IP)
        iqdata.key_data.key_field_mask |= 0x1 << IQCTL_KEY_FIELD_IP_PROTO;
    else
        return -EINVAL;

    iqdata.key_data.ip_proto = protoval;

    return bcm_iqctl_rem_key(&iqdata);
}

