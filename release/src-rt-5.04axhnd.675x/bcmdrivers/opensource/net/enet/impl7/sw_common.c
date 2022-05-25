/*
   <:copyright-BRCM:2018:DUAL/GPL:standard

      Copyright (c) 2018 Broadcom
      All Rights Reserved

   Unless you and Broadcom execute a separate written software license
   agreement governing use of this software, this software is licensed
   to you under the terms of the GNU General Public License version 2
   (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
   with the following added to such license:

      As a special exception, the copyright holders of this software give
      you permission to link this software with independent modules, and
      to copy and distribute the resulting executable under terms of your
      choice, provided that you also meet, for each linked independent
      module, the terms and conditions of the license of that module.
      An independent module is a module which is not derived from this
      software.  The special exception does not apply to any modifications
      of the software.

   Not withstanding the above, under no circumstances may you combine
   this software in any way with any other Broadcom software provided
   under a license other than the GPL, without Broadcom's express prior
   written consent.

   :>
 */

/*
 *  Created on: June/2019
 *      Author: fulin.chang@broadcom.com
 */

#include "port.h"
#ifdef SF2_DEVICE
#include "sf2.h"
#include "mac_drv.h"
#include "mac_drv_sf2.h"
#include "phy_drv_dsl_phy.h"
#endif
#include "crossbar_dev.h"
#include "bcmmii.h"
#include "bcmmii_xtn.h"
#include "sw_common.h"
#include "bcmenet_common.h"
#include "bcmenet.h"
#include <linux/spinlock.h>

#ifdef SF2_DEVICE
#define sw_rreg sf2_sw_rreg
#define sw_wreg sf2_sw_wreg
void sf2_sw_rreg(int unit, int page, int reg, void *data_out, int len);
void sf2_sw_wreg(int unit, int page, int reg, void *data_in, int len);
#endif

#define SF2SW_RREG      extsw_rreg_wrap
#define SF2SW_WREG      extsw_wreg_wrap

extern spinlock_t       extsw_reg_config;

enetx_port_t *sw_p = NULL;   /* switch */

void extsw_rreg_wrap(int unit, int page, int reg, void *vptr, int len)
{
    uint8 val[8];
    uint8 *data = (uint8*)vptr;
    int type = len & SWAP_TYPE_MASK;

    len &= ~(SWAP_TYPE_MASK);

    /* Lower level driver always returnes in Little Endian data from history */
    sw_rreg(unit, page, reg, val, len);

    switch (len) {
        case 1:
            data[0] = val[0];
            break;
        case 2:
            *((uint16 *)data) = __le16_to_cpu(*((uint16 *)val));
            break;
        case 4:
            *((uint32 *)data) = __le32_to_cpu(*((uint32 *)val));
            break;
        case 6:
            switch (type) {
                case DATA_TYPE_HOST_ENDIAN:
                default:
                    /*
                        Value type register access
                        Input:  val:Le64 from Lower driver API
                        Output: data:Host64, a pointer to the begining of 64 bit buffer
                    */
                    *(uint64*)data = __le64_to_cpu(*(uint64 *)val);
                    break;
                case DATA_TYPE_BYTE_STRING:
                    /*
                        Byte array for MAC address
                        Input:  val:Mac[5...0] from lower driver
                        Output: data:Mac[0...5]
                    */
                    *(uint64 *)val = __swab64(*(uint64*)val);
                    memcpy(data, val+2, 6);
                    break;
            }
            break;
        case 8:
            switch (type) {
                case DATA_TYPE_HOST_ENDIAN:
                default:
                    /*
                        Input:  val: Le64 for lower driver API
                        Output: data: Host64
                    */
                    *(uint64 *)data = __le64_to_cpu(*(uint64*)val);
                    break;
                case DATA_TYPE_BYTE_STRING:
                    /*
                        Input:  val:byte[7...0] from lower driver API
                        Output: data:byte[0...7]
                    */
                    *(uint64 *)data = __swab64(*(uint64*)val);
                    break;
                case DATA_TYPE_VID_MAC:
                    /*
                        VID-MAC type;
                        Input:  val:Mac[5...0]|VidLeWord from Lower Driver API
                        Output: data:VidHostWord|Mac[0...5] for Caller
                    */
                    /* [Mac[5-0]]|[LEWord]; First always swap all bytes */
                    *((uint64 *)data) = __swab64(*((uint64 *)val));
                    /* Now is [BEWord]|[Mac[0-5]]; Conditional Swap 2 bytes */
                    *((uint16 *)&data[0]) = __be16_to_cpu(*((uint16 *)&data[0]));
                    /* Now is HostEndianWord|Mac[0-5] */
                    break;
            } // switch type
            break;
        default:
            printk("Length %d not supported\n", len);
            break;
    }
}

void extsw_wreg_wrap(int unit, int page, int reg, void *vptr, int len)
{
    uint8 val[8];
    uint8 *data = (uint8*)vptr;
    int type = len & SWAP_TYPE_MASK;

    len  &= ~(SWAP_TYPE_MASK);

    switch (len) {
        case 1:
            val[0] = data[0];
            break;
        case 2:
            *((uint16 *)val) = __cpu_to_le16(*((uint16 *)data));
            break;
        case 4:
            *((uint32 *)val) = __cpu_to_le32(*((uint32 *)data));
            break;
        case 6:
            switch(type) {
                case DATA_TYPE_HOST_ENDIAN:
                default:
                    /*
                        Value type register access
                        Input:  data:Host64, a pointer to the begining of 64 bit buffer
                        Output: val:Le64
                    */
                    *(uint64 *)val = __cpu_to_le64(*(uint64 *)data);
                    break;
                case DATA_TYPE_BYTE_STRING:
                    /*
                        Byte array for MAC address
                        Input:  data:MAC[0...5] from Host
                        Output: val:Mac[5...0] for lower driver API
                    */
                    memcpy(val+2, data, 6);
                    *(uint64 *)val = __swab64(*(uint64*)val);
                    break;
            }
            break;
        case 8:
            switch (type)
            {
                case DATA_TYPE_HOST_ENDIAN:
                default:
                    /*
                        Input: data:Host64
                        Output:  val:Le64 for lower driver API
                    */
                    *(uint64 *)val = __cpu_to_le64(*(uint64*)data);
                    break;
                case DATA_TYPE_BYTE_STRING:
                    /*
                        Input:  data:byte[0...7]
                        Output: val:byte[7...0] for lower driver API
                    */
                    *(uint64 *)val = __swab64(*(uint64*)data);
                    break;
                case DATA_TYPE_VID_MAC:
                    /*
                        VID-MAC type;
                        Input:  VidHostWord|Mac[0...5]
                        Output: Mac[5..0.]|VidLeWord for Lower Driver API
                    */
                    /* Contains HostEndianWord|MAC[0-5] Always swap first*/
                    *((uint64 *)val) = __swab64(*((uint64 *)data));
                    /* Now it is MAC[5-0]|SwappedHostEndianWord */
                    /* Convert the SwappedHostEndianWord to Little Endian; thus BE */
                    *((uint16 *)&val[6]) = __cpu_to_be16(*((uint16 *)&val[6]));
                    /* Now is MAC[5-0]|LEWord as requested by HW */
                    break;
            } // switch type
            break;
        default:
            printk("Length %d not supported\n", len);
            break;
    } // switch len
    sw_wreg(unit, page, reg, val, len);
}


// =========== public ioctl functions =====================

// ----------- SIOCETHSWCTLOPS ETHSWREGACCESS functions ---
static int _alignment_check(struct ethswctl_data *e)
{
    static u16 misalign[][2] = {{0x581, 2}, {0x583, 4}, {0x4001, 4}, {0x4005, 2}};
    int i;

    for(i=0; i < ARRAY_SIZE(misalign); i++)
    {
        if (e->offset == misalign[i][0] && e->length == misalign[i][1]) return 1;
    }

    if (((e->offset % 2) && (e->length == 2 || e->length == 6)) ||
            ((e->offset % 4) && (e->length == 4 || e->length == 8))) return 0;

    return 1;
}

static int _ethsw_phy_access(struct ethswctl_data *e, enetx_port_t *port)
{
    // based on shared\bcmswaccess.c:bcmsw_phy_access()
    uint16 phy_reg_val;
    uint32_t data = 0;
    int reg_offset;
    phy_dev_t *phy = get_active_phy(port->p.phy);

    reg_offset = (e->offset >> PHY_REG_S) & PHY_REG_M;

    if (e->type == TYPE_GET) {
        down(&sw_p->s.conf_sem);
        if (phy->phy_drv && phy->phy_drv->priv_fun)
            phy_priv_fun(phy, PHY_OP_RD_MDIO, reg_offset, &phy_reg_val);
        else
            phy_bus_read(phy, reg_offset, &phy_reg_val);
        data = phy_reg_val;
        up(&sw_p->s.conf_sem);
        data = __le32_to_cpu(data);
        e->length = 4;
        memcpy((void*)(&e->data), (void*)&data, e->length);
    } else {
        memcpy((void *)&phy_reg_val, (void *)e->data, 2);
        phy_reg_val = __cpu_to_le16(phy_reg_val);
        down(&sw_p->s.conf_sem);
        if (phy->phy_drv && phy->phy_drv->priv_fun)
            phy_priv_fun(phy, PHY_OP_WR_MDIO, reg_offset, phy_reg_val);
        else
            phy_bus_write(phy, reg_offset, phy_reg_val);
        up(&sw_p->s.conf_sem);
    }

    return 0;
}

int ioctl_extsw_regaccess(struct ethswctl_data *e, enetx_port_t *port)
{
    // based on shared\bcmswaccess.c:enet_ioctl_ethsw_regaccess()
    unsigned char data[8] = {0};

    if (e->offset & IS_PHY_ADDR_FLAG)
        return _ethsw_phy_access(e, port);

    if ( ((e->length != 1) && (e->length % 2)) || (e->length > 8)) {
        enet_err("Invalid length");
        return -EINVAL;
    }
    if (!IS_UNIT_SF2(e->unit))
                       /* Offset checks are not valid below for SF2 because all offsets
                       * are converted to 4-byte accesses later down the road. */
    {
        /*
            We do have unlignment register access: 0x0583 4 bytes and 0x0581 2bytes for VLAN configuration.
            So unligned access is needed and driver underneath is working correctly.
            Make exaption for 0x0583 and 0x0581
        */
        if (!_alignment_check(e))
        {
            enet_err("len = %d offset = 0x%04x alignment error !! \n", e->length, e->offset);
            return -EINVAL;
        }
    }

    if (e->type == TYPE_GET) {
        if (IS_UNIT_SF2(e->unit)) {
            SF2SW_RREG(e->unit, ((e->offset & 0xFF00)>>8), (e->offset & 0xFF), data, e->length);
        }
        else
        {
            enet_err("internal switch reg read not supported.!! (offset=%x, len=%d)\n", e->offset, e->length);
            return -EINVAL;
            //ethsw_read_reg(e->offset, data, e->length);
        }
        memcpy((void*)(&e->data), (void*)&data, e->length);

    } else {
        if (IS_UNIT_SF2(e->unit)) {
            SF2SW_WREG(e->unit, ((e->offset & 0xFF00)>>8), (e->offset & 0xFF), e->data, e->length);
        }
        else
        {
            enet_err("internal switch reg write not supported.!! (offset=%x, len=%d)\n", e->offset, e->length);
            return -EINVAL;
            //ethsw_write_reg(e->offset, e->data, e->length);
        }
    }

    return BCM_E_NONE;
}

// ----------- SIOCETHSWCTLOPS ETHSWMIRROR functions ---
int ioctl_extsw_port_mirror_ops(struct ethswctl_data *e)
{
    /* based on impl5:bcmsw_port_mirror_get/set() */
    uint16_t v16;

    if (e->type == TYPE_GET) {
        e->port_mirror_cfg.tx_port = -1;
        e->port_mirror_cfg.rx_port = -1;

        SF2SW_RREG(e->unit, PAGE_MANAGEMENT, REG_MIRROR_CAPTURE_CTRL, &v16, sizeof(v16));
        if (v16 & REG_MIRROR_ENABLE)
        {
            e->port_mirror_cfg.enable = 1;
            e->port_mirror_cfg.mirror_port = v16 & REG_CAPTURE_PORT_M;
            e->port_mirror_cfg.blk_no_mrr = v16 & REG_BLK_NOT_MIRROR;
            SF2SW_RREG(e->unit, PAGE_MANAGEMENT, REG_MIRROR_INGRESS_CTRL, &v16, sizeof(v16));
            e->port_mirror_cfg.ing_pmap = v16 & REG_INGRESS_MIRROR_M;
            SF2SW_RREG(e->unit, PAGE_MANAGEMENT, REG_MIRROR_EGRESS_CTRL, &v16, sizeof(v16));
            e->port_mirror_cfg.eg_pmap = v16 & REG_EGRESS_MIRROR_M;
        }
        else
        {
            e->port_mirror_cfg.enable = 0;
        }
    } else {
        if (e->port_mirror_cfg.enable)
        {
            v16 = REG_MIRROR_ENABLE;
            v16 |= (e->port_mirror_cfg.mirror_port & REG_CAPTURE_PORT_M);
            v16 |= e->port_mirror_cfg.blk_no_mrr?REG_BLK_NOT_MIRROR:0;

            SF2SW_WREG(e->unit, PAGE_MANAGEMENT, REG_MIRROR_CAPTURE_CTRL, &v16, sizeof(v16));
            v16 = e->port_mirror_cfg.ing_pmap & REG_INGRESS_MIRROR_M;
            SF2SW_WREG(e->unit, PAGE_MANAGEMENT, REG_MIRROR_INGRESS_CTRL, &v16, sizeof(v16));
            v16 = e->port_mirror_cfg.eg_pmap & REG_INGRESS_MIRROR_M;
            SF2SW_WREG(e->unit, PAGE_MANAGEMENT, REG_MIRROR_EGRESS_CTRL, &v16, sizeof(v16));
        }
        else
        {
            v16  = 0;
            SF2SW_WREG(e->unit, PAGE_MANAGEMENT, REG_MIRROR_CAPTURE_CTRL, &v16, sizeof(v16));
        }
    }

    return 0;
}

// ----------- SIOCETHSWCTLOPS ETHSWPORTTRUNK functions ---
int ioctl_extsw_port_trunk_ops(struct ethswctl_data *e)
{
    /* based on impl5:bcmsw_port_trunk_get/set() */
    uint16_t v16;
    uint8_t v8;

    if (e->type == TYPE_GET) {
        SF2SW_RREG(e->unit, PAGE_MAC_BASED_TRUNK, REG_TRUNK_GRP_CTL , &v16, 2);
        e->port_trunk_cfg.grp0_pmap = (v16 >> TRUNK_EN_GRP_S) & TRUNK_EN_GRP_M ;
        SF2SW_RREG(e->unit, PAGE_MAC_BASED_TRUNK, REG_TRUNK_GRP_CTL+2 , &v16, 2);
        e->port_trunk_cfg.grp1_pmap = (v16 >> TRUNK_EN_GRP_S) & TRUNK_EN_GRP_M ;

        SF2SW_RREG(e->unit, PAGE_MAC_BASED_TRUNK, REG_MAC_TRUNK_CTL, &v8, 1);
        e->port_trunk_cfg.enable = (v8 >> TRUNK_EN_LOCAL_S) & TRUNK_EN_LOCAL_M;
        e->port_trunk_cfg.hash_sel = (v8 >> TRUNK_HASH_SEL_S) & TRUNK_HASH_SEL_M;
    } else {
        SF2SW_RREG(e->unit, PAGE_MAC_BASED_TRUNK, REG_MAC_TRUNK_CTL, &v8, 1);
        v8 &= ~(TRUNK_HASH_SEL_M<<TRUNK_HASH_SEL_S); /* Clear old hash selection first */
        v8 |= ( (e->port_trunk_cfg.hash_sel & TRUNK_HASH_SEL_M) << TRUNK_HASH_SEL_S ); /* Set Hash Selection */
        SF2SW_WREG(e->unit, PAGE_MAC_BASED_TRUNK, REG_MAC_TRUNK_CTL, &v8, 1);
        printk("LAG/Trunking hash selection changed <0x%01x>\n",v8);
    }

    return 0;
}

// ----------- SIOCETHSWCTLOPS ETHSWCONTROL functions ---
/*
 * Get/Set StarFighter flow control options.
 *** Input params
 * e->type  GET/SET
 * e->sub_type Flow control type
 * e->val enable/disable above flow control type
 *** Output params
 * e->val has result for GET
 * Returns 0 for Success, Negative value for failure.
 */
#ifdef SF2_DEVICE
static int _sf2_pause_drop_ctrl(struct ethswctl_data *e)
{
    /* based on impl5:sf2_pause_drop_ctrl() */
    uint16_t val = 0;
    uint16_t val2 = 0;
    if (e->type == TYPE_SET)    { // SET
        SF2SW_RREG(e->unit, PAGE_FLOW_CTRL_XTN, REG_FC_PAUSE_DROP_CTRL, &val, 2);
        switch (e->sub_type) {
            case bcmSwitchFcMode:
                val2 = e->val? FC_CTRL_MODE_PORT: 0;
                SF2SW_WREG(e->unit, PAGE_FLOW_CTRL_XTN, REG_FC_CTRL_MODE, &val2, 2);
                return 0;
            case bcmSwitchQbasedpauseEn:
                val &= ~FC_QUEUE_BASED_PAUSE_EN;
                val |= e->val? FC_QUEUE_BASED_PAUSE_EN: 0;
                break;
            case bcmSwitchTxBasedFc:
                val &= ~FC_TX_BASED_CTRL_EN;
                val |= e->val? FC_TX_BASED_CTRL_EN: 0;
                val &= ~FC_RX_BASED_CTRL_EN;
                val |= e->val? 0: FC_RX_BASED_CTRL_EN;
                val &= ~FC_RX_DROP_EN;
                val |= e->val? 0: FC_RX_DROP_EN;
                break;
            case bcmSwitchTxQdropEn:
                val &= ~FC_TX_TXQ_DROP_EN;
                val |= e->val? FC_TX_TXQ_DROP_EN: 0;
                break;
            case bcmSwitchTxTotdropEn:
                val &= ~FC_TX_TOTAL_DROP_EN;
                val |= e->val? FC_TX_TOTAL_DROP_EN: 0;
                break;
            case bcmSwitchTxQpauseEn:
                val &= ~FC_TX_TXQ_PAUSE_EN;
                val |= e->val? FC_TX_TXQ_PAUSE_EN: 0;
                break;
            case bcmSwitchTxTotPauseEn:
                val &= ~FC_TX_TOTAL_PAUSE_EN;
                val |= e->val? FC_TX_TOTAL_PAUSE_EN: 0;
                break;
            case bcmSwitchTxQpauseEnImp0:
                val &= ~FC_TX_IMP0_TXQ_PAUSE_EN;
                val |= e->val? FC_TX_IMP0_TXQ_PAUSE_EN: 0;
                break;
            case bcmSwitchTxTotPauseEnImp0:
                val &= ~FC_TX_IMP0_TOTAL_PAUSE_EN;
                val |= e->val? FC_TX_IMP0_TOTAL_PAUSE_EN: 0;
                break;
            default:
                printk("%s unknown fc type %u \n", __FUNCTION__, (unsigned int)e->sub_type);
                return -BCM_E_ERROR;
                break;
        }
        SF2SW_WREG(e->unit, PAGE_FLOW_CTRL_XTN, REG_FC_PAUSE_DROP_CTRL, &val, 2);
    } else {
        //   GET
        val2 = 0;
        SF2SW_RREG(e->unit, PAGE_FLOW_CTRL_XTN, REG_FC_PAUSE_DROP_CTRL, &val, 2);
        val2 |= val & FC_QUEUE_BASED_PAUSE_EN? 1 << bcmSwitchQbasedpauseEn: 0;
        val2 |= val & FC_TX_BASED_CTRL_EN? 1 << bcmSwitchTxBasedFc: 0;
        val2 |= val & FC_TX_TXQ_DROP_EN? 1 << bcmSwitchTxQdropEn: 0;
        val2 |= val & FC_TX_TOTAL_DROP_EN? 1 << bcmSwitchTxTotdropEn: 0;
        val2 |= val & FC_TX_TXQ_PAUSE_EN? 1 << bcmSwitchTxQpauseEn: 0;
        val2 |= val & FC_TX_TOTAL_PAUSE_EN? 1 << bcmSwitchTxTotPauseEn: 0;
        val2 |= val & FC_TX_IMP0_TXQ_PAUSE_EN? 1 << bcmSwitchTxQpauseEnImp0: 0;
        val2 |= val & FC_TX_IMP0_TOTAL_PAUSE_EN? 1 << bcmSwitchTxTotPauseEnImp0: 0;

        SF2SW_RREG(e->unit, PAGE_FLOW_CTRL_XTN, REG_FC_CTRL_MODE, &val, 2);
        val2 |= val & FC_CTRL_MODE_PORT? 1 << bcmSwitchFcMode: 0;
        e->val = val2;
        //enet_dbg("%s: val2 = 0x%x \n", __FUNCTION__, val2);
    }
    return 0;
}
#endif

int ioctl_extsw_control(struct ethswctl_data *e)
{
    /* based on impl5:bcmeapi_ioctl_extsw_control() */
    int ret = 0;
    uint8_t val8 = 0;
    unsigned int val;
    switch (e->sw_ctrl_type) {
#ifdef SF2_DEVICE
        case bcmSwitchBufferControl:

            if ((ret = _sf2_pause_drop_ctrl(e)) >= 0) {
                if (e->type == TYPE_GET) {
                    e->ret_val = e->val;
                    //enet_dbg("e->ret_val is = %4x\n", e->ret_val);
                 }
            }
            break;
#endif

        case bcmSwitch8021QControl:
            /* Read the 802.1Q control register */
            SF2SW_RREG(e->unit, PAGE_8021Q_VLAN, REG_VLAN_GLOBAL_8021Q, &val8, 1);
            if (e->type == TYPE_GET) {
                val = (val8 >> VLAN_EN_8021Q_S) & VLAN_EN_8021Q_M;
                if (val && ((val8 >> VLAN_IVL_SVL_S) & VLAN_IVL_SVL_M))
                    val = 2; // IVL mode
                e->val  = val;
                //enet_dbg("e->val is = %4x\n", e->val);
            } else {  // 802.1Q SET
                /* Enable/Disable the 802.1Q */
                if (e->val == 0)
                    val8 &= (~(VLAN_EN_8021Q_M << VLAN_EN_8021Q_S));
                else {
                    val8 |= (VLAN_EN_8021Q_M << VLAN_EN_8021Q_S);
                    if (e->val == 1) // SVL
                        val8 &= (~(VLAN_IVL_SVL_M << VLAN_IVL_SVL_S));
                    else if (e->val == 2) // IVL
                        val8 |= (VLAN_IVL_SVL_M << VLAN_IVL_SVL_S);
                }
                SF2SW_WREG(e->unit, PAGE_8021Q_VLAN, REG_VLAN_GLOBAL_8021Q, &val8, 1);
            }
            break;

        default:
            //up(&bcm_ethlock_switch_config);
            ret = -BCM_E_PARAM;
            break;
    } //switch
    return ret;
}

// ----------- SIOCETHSWCTLOPS ETHSWPBVLAN functions ---
int ioctl_extsw_pbvlan(struct ethswctl_data *e)
{
    /* based on impl5:bcmeapi_ioctl_extsw_pbvlan() */
    uint16_t val16;

    //enet_dbg("Given Port: 0x%02x \n ", e->port);
    if (e->port >= TOTAL_SWITCH_PORTS) {
        printk("Invalid Switch Port \n");
        return BCM_E_ERROR;
    }

    if (e->type == TYPE_GET) {
        down(&sw_p->s.conf_sem);
        SF2SW_RREG(e->unit, PAGE_PORT_BASED_VLAN, REG_VLAN_CTRL_P0 + (e->port*2), &val16, 2);
        up(&sw_p->s.conf_sem);
        e->fwd_map = val16;
        //enet_dbg("get e->fwd_map is = %4x\n", e->fwd_map);
    } else {
        val16 = (uint32_t)e->fwd_map;
        //enet_dbg("set e->fwd_map is = %4x\n", e->fwd_map);
        down(&sw_p->s.conf_sem);
        SF2SW_WREG(e->unit, PAGE_PORT_BASED_VLAN, REG_VLAN_CTRL_P0 + (e->port*2), &val16, 2);
        up(&sw_p->s.conf_sem);
    }

    return 0;
}

// ----------- SIOCETHSWCTLOPS ETHSWVLAN functions ---
static int _read_vlan_table(int unit, bcm_vlan_t vid, uint32_t *val32)
{
    uint8_t val8;
    int i, timeout = 200;

    SF2SW_WREG(unit, PAGE_AVTBL_ACCESS, REG_VLAN_TBL_INDX_531xx, (uint8_t *)&vid, 2);
    val8 = 0x81;
    SF2SW_WREG(unit, PAGE_AVTBL_ACCESS, REG_VLAN_TBL_CTRL_531xx, (uint8_t *)&val8, 1);

    for (i = 0; i < timeout; i++) {
        SF2SW_RREG(unit, PAGE_AVTBL_ACCESS, REG_VLAN_TBL_CTRL_531xx, (uint8_t *)&val8, 1);
        if ((val8 & 0x80) == 0) {
            SF2SW_RREG(unit, PAGE_AVTBL_ACCESS, REG_VLAN_TBL_ENTRY_531xx, (uint8_t *)val32, 4);
            return 0;
        }
        udelay(100);
    }
    return -BCM_E_ERROR;
}

static int _write_vlan_table(int unit, bcm_vlan_t vid, uint32_t val32)
{
    uint8_t val8;
    int i, timeout = 200;

    SF2SW_WREG(unit, PAGE_AVTBL_ACCESS, REG_VLAN_TBL_INDX_531xx, (uint8_t *)&vid, 2);
    SF2SW_WREG(unit, PAGE_AVTBL_ACCESS, REG_VLAN_TBL_ENTRY_531xx, (uint8_t *)&val32, 4);
    val8 = 0x80;
    SF2SW_WREG(unit, PAGE_AVTBL_ACCESS, REG_VLAN_TBL_CTRL_531xx, (uint8_t *)&val8, 1);

    for (i = 0; i < timeout; i++) {
        SF2SW_RREG(unit, PAGE_AVTBL_ACCESS, REG_VLAN_TBL_CTRL_531xx, (uint8_t *)&val8, 1);
        if ((val8 & 0x80) == 0) {
            return 0;
        }
        udelay(100);
    }
    return -BCM_E_ERROR;
}

int ioctl_extsw_vlan(struct ethswctl_data *e)
{
    bcm_vlan_t vid;
    uint32_t val32;

    down(&sw_p->s.conf_sem);
    vid = e->vid & BCM_NET_VLAN_VID_M;
    if (e->type == TYPE_GET) {
        if (_read_vlan_table(e->unit, vid, &val32)) {
            up(&sw_p->s.conf_sem);
            enet_err("SF2 VLAN Table read failed\n");
            return -BCM_E_ERROR;
        }
        e->fwd_map = val32 & VLAN_FWD_MAP_M;
        e->untag_map = (val32 >> VLAN_UNTAG_MAP_S) & VLAN_UNTAG_MAP_M;
    } else {
        val32 = e->fwd_map | (e->untag_map << VLAN_UNTAG_MAP_S);
        if (_write_vlan_table(e->unit, vid, val32)) {
            up(&sw_p->s.conf_sem);
            enet_err("SF2 VLAN Table write failed\n");
            return -BCM_E_ERROR;
        }
    }
    up(&sw_p->s.conf_sem);
    return 0;
}

// ----------- SIOCETHSWCTLOPS ETHSWARLACCESS functions ---
#define _enet_arl_read( mac, vid, val ) FALSE
#define _enet_arl_write(mac, vid, val) {}
#define _enet_arl_dump() {}  /* This should return status actually ?? */
#define _enet_arl_dump_multiport_arl() {}

static inline int enet_arl_remove(char *mac) {return 0;}

// based on bcmsw.h
int _enet_arl_search_ext(int unit, uint8_t *mac, uint32_t *vid, uint32_t *val, int op);
#define _enet_arl_dump_ext(u) _enet_arl_search_ext(u, 0, 0, 0, TYPE_DUMP)
#define _enet_arl_read_ext(u, mc, vd, vl) _enet_arl_search_ext(u, mc, vd, vl, TYPE_GET)
#define _enet_arl_remove_ext(u, mc) _enet_arl_search_ext(u, mc, 0, 0, TYPE_SET)


static int _enet_arl_access_reg_op(int unit, uint8_t v8)
{
    int timeout;

    SF2SW_WREG(unit, PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);
    for ( timeout = 10, SF2SW_RREG(unit, PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);
            (v8 & ARL_TBL_CTRL_START_DONE) && timeout;
            --timeout, SF2SW_RREG(unit, PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1))
    {
        mdelay(1);
    }

    if (timeout <= 0)
    {
        printk("Error: ARL Operation Timeout\n");
        return 0;
    }
    return 1;
}

/* v32: b31 is raw bit,
    If raw: register format; etherwise: b15 is Valid bit */
int extsw_arl_write_ext(int unit, uint8_t *mac, uint16_t vid, uint32_t v32)
{
    uint8_t mac_vid[8];
    uint32_t cur_v32;
    uint16_t ent_vid;
    int bin, empty_bin = -1;

    if (!(v32 & (1<<31))) v32 = ((v32 & 0xfc00) << 1) | (v32 & 0x1ff);  /* If it is raw, shift valid bit left */
    v32 &= ~(1<<31);

    /* Write the MAC Address and VLAN ID */
    SF2SW_WREG(unit, PAGE_AVTBL_ACCESS, REG_ARL_MAC_INDX_LO, mac, 6|DATA_TYPE_BYTE_STRING);
    SF2SW_WREG(unit, PAGE_AVTBL_ACCESS, REG_ARL_VLAN_INDX, &vid, 2);
    if (!_enet_arl_access_reg_op(unit, ARL_TBL_CTRL_START_DONE | ARL_TBL_CTRL_READ)) return 0;

    for (bin = 0; bin < REG_ARL_BINS_PER_HASH; bin++)
    {
        /* Read transaction complete - get the MAC + VID */
        SF2SW_RREG(unit, PAGE_AVTBL_ACCESS, REG_ARL_MAC_LO_ENTRY + bin*0x10, &mac_vid[0], 8|DATA_TYPE_VID_MAC);
        SF2SW_RREG(unit, PAGE_AVTBL_ACCESS, REG_ARL_DATA_ENTRY + bin*0x10,(uint8_t *)&cur_v32, 4);
        ent_vid = *(uint16_t*)mac_vid;

        if (!(v32 & ARL_DATA_ENTRY_VALID_531xx))
        {
            /* If it is del op, find the matched bin */
            if (memcmp(&mac[0], &mac_vid[2], 6) != 0 || ent_vid != vid) continue;
        }
        else
        {
            /* If it is a modification or addition,
               find a matching entry, empty slot or last slot */
            if (memcmp(&mac[0], &mac_vid[2], 6) == 0 && vid == ent_vid) goto found_slot;
            if (!(cur_v32 & ARL_DATA_ENTRY_VALID_531xx) && empty_bin == -1) empty_bin = bin;  /* save empty bin for non matching case */
            if (bin < REG_ARL_BINS_PER_HASH-1) continue;  /* Continue to find next bin for matching if it not the last */
            /* No matching found here, if there is empty bin, use empty_bin or use last bin */
            if (empty_bin != -1) bin = empty_bin;
        }

        found_slot:

        /* Modify the data entry for this ARL */
        *(uint16 *)(&mac_vid[0]) = (vid & 0xFFF);
        memcpy(&mac_vid[2], &mac[0], 6);
        SF2SW_WREG(unit, PAGE_AVTBL_ACCESS, REG_ARL_MAC_LO_ENTRY + bin*0x10, mac_vid, 8|DATA_TYPE_VID_MAC);
        SF2SW_WREG(unit, PAGE_AVTBL_ACCESS, REG_ARL_DATA_ENTRY + bin*0x10,(uint8_t *)&v32, 4);

        /* Initiate a write transaction */
        if (!_enet_arl_access_reg_op(unit, ARL_TBL_CTRL_START_DONE)) return 0;
        return 1;
    }
    enet_err("Error - can't find the requested ARL entry\n");
    return 0;
}

int _enet_arl_entry_op(int unit, uint8_t *mac, uint32_t *vid, uint32_t *val, int op, int *count, u8 *mac_vid, u32 data)
{
    switch(op)
    {
        case TYPE_DUMP:
            if (*count == 0) printk("\nExternal Switch ARL Dump:\n");
            if ((((*count)++) % 10)==0)
            {
                printk("  No: VLAN  MAC          DATA" "(15:Valid,14:Static,13:Age,12-10:Pri,8-0:Port/Pmap)\n");
            }

            printk("%4d: %04d  %02x%02x%02x%02x%02x%02x 0x%04x\n",
                    *count, *(uint16 *)&mac_vid[0],
                    mac_vid[2], mac_vid[3], mac_vid[4], mac_vid[5], mac_vid[6], mac_vid[7],
                    ((data & 0x1f800)>>1)|(data&0x1ff));
            break;
        case TYPE_SET:
            if (memcmp(&mac[0], &mac_vid[2], 6) == 0)
            {
                extsw_arl_write_ext(unit, mac, *(u16*)mac_vid, 0);
                (*count)++;
            }
            break;
        case TYPE_GET:
            if (memcmp(&mac[0], &mac_vid[2], 6) == 0 &&
                    (*vid == -1 || *vid == *(u16*)mac_vid))
            {
                /* entry found */
                *vid = *(uint16_t*)mac_vid;
                if (*val & (1<<31)) /* Raw flag passed down from users space */
                {
                    *val = data;
                }
                else
                {
                    *val = ((data & 0x1f800)>>1)|(data & 0x1ff);
                }
                /* Return FALSE to terminate loop */
                return TRUE;
            }
            break;
    }
    return FALSE;
}

int _enet_arl_search_ext(int unit, uint8_t *mac, uint32_t *vid, uint32_t *val, int op)
{
    int timeout = 1000, count = 0, hash_ent;
    uint32_t cur_data;
    uint8_t v8, mac_vid[8];

    v8 = ARL_SRCH_CTRL_START_DONE;
    SF2SW_WREG(unit, PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL_531xx, (uint8_t *)&v8, 1);

    for( SF2SW_RREG(unit, PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL_531xx, (uint8_t *)&v8, 1);
            (v8 & ARL_SRCH_CTRL_START_DONE);
            SF2SW_RREG(unit, PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL_531xx, (uint8_t *)&v8, 1))
    {
        /* Now read the Search Ctrl Reg Until :
         * Found Valid ARL Entry --> ARL_SRCH_CTRL_SR_VALID, or
         * ARL Search done --> ARL_SRCH_CTRL_START_DONE */
        for(timeout = 1000;
                (v8 & ARL_SRCH_CTRL_SR_VALID) == 0 && (v8 & ARL_SRCH_CTRL_START_DONE) && timeout-- > 0;
                mdelay(1),
                SF2SW_RREG(unit, PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL_531xx, (uint8_t *)&v8, 1));

        if ((v8 & ARL_SRCH_CTRL_SR_VALID) == 0 || timeout <= 0) break;

        /* Found a valid entry */
        for (hash_ent = 0; hash_ent < REG_ARL_SRCH_HASH_ENTS; hash_ent++)
        {
            SF2SW_RREG(unit, PAGE_AVTBL_ACCESS, REG_ARL_SRCH_MAC_LO_ENTRY0_531xx + hash_ent*0x10,&mac_vid[0], 8|DATA_TYPE_VID_MAC);
            SF2SW_RREG(unit, PAGE_AVTBL_ACCESS, REG_ARL_SRCH_DATA_ENTRY0_531xx + hash_ent*0x10,(uint8_t *)&cur_data, 4);

//            enet_dbg("ARL_SRCH_result (%02x%02x%02x%02x%02x%02x%02x%02x) \n",
//                    mac_vid[0],mac_vid[1],mac_vid[2],mac_vid[3],mac_vid[4],mac_vid[5],mac_vid[6],mac_vid[7]);
//            enet_dbg("ARL_SRCH_DATA = 0x%08x \n", cur_data);

            if ((cur_data & ARL_DATA_ENTRY_VALID_531xx))
            {
                if (_enet_arl_entry_op(unit, mac, vid, val, op, &count, mac_vid, cur_data)) return TRUE;
            }
        }
    }

    if (timeout <= 0)
    {
        printk("ARL Search Timeout for Valid to be 1 \n");
    }

    if (op == TYPE_DUMP) printk("Done: Total %d entries\n", count);
    if (op == TYPE_GET) return FALSE;
    return TRUE;
}

void _enet_arl_dump_ext_multiport_arl(int unit)
{
    uint16 v16;
    uint8 addr[8];
    int i, enabled;
    uint32 vect;
    static char *cmp_type[] = {"Disabled", "Etype", "MAC Addr", "MAC Addr & Etype"};

    SF2SW_RREG(unit, PAGE_ARLCTRL, REG_MULTIPORT_CTRL, &v16, 2);
    enabled = v16 & ((MULTIPORT_CTRL_EN_M << (5*2))| (MULTIPORT_CTRL_EN_M << (4*2))| (MULTIPORT_CTRL_EN_M << (3*2))|
            (MULTIPORT_CTRL_EN_M << (2*2))| (MULTIPORT_CTRL_EN_M << (1*2))| (MULTIPORT_CTRL_EN_M << (0*2)));

    printk("\nExternal Switch Multiport Address Dump: Function %s\n", enabled? "Enabled": "Disabled");
    if (!enabled) return;

    printk("  Mapping to ARL matching: %s\n", v16 & (1<<MULTIPORT_CTRL_DA_HIT_EN)? "Lookup Hit": "Lookup Failed");
    for (i=0; i<6; i++)
    {
        enabled = (v16 >> (i*2)) & MULTIPORT_CTRL_EN_M;
        SF2SW_RREG(unit, PAGE_ARLCTRL, REG_MULTIPORT_ADDR1_LO + i*16, (uint8 *)&addr, sizeof(addr)|DATA_TYPE_VID_MAC);
        SF2SW_RREG(unit, PAGE_ARLCTRL, REG_MULTIPORT_VECTOR1 + i*16, (uint8 *)&vect, sizeof(vect));
        printk("Mport Eth Type: 0x%04x, Mport Addrs: %02x:%02x:%02x:%02x:%02x:%02x, Port Map %04x, Cmp Type: %s\n",
                *(uint16 *)(addr),
                addr[2],
                addr[3],
                addr[4],
                addr[5],
                addr[6],
                addr[7],
                (int)vect, cmp_type[enabled & MULTIPORT_CTRL_EN_M]);
    }
    printk("External Switch Multiport Address Dump Done\n");
}

static void _fast_age_start_done_ext(int unit, uint8_t ctrl);

int ioctl_extsw_arl_access(struct ethswctl_data *e)
{
    // based on impl5:bcmeapi_ioctl_ethsw_arl_access()
    int ret;

    switch(e->type)
    {
        case TYPE_GET:
            enet_dbg("get e->mac: %02x %02x %02x %02x %02x %02x e->vid: %d\n", e->mac[5],
                    e->mac[4], e->mac[3], e->mac[2], e->mac[1], e->mac[0], e->vid);

            if (IS_UNIT_SF2(e->unit))
            {
                ret = _enet_arl_read_ext(e->unit, e->mac, &e->vid, &e->val );
            }
            else
            {
                if ((ret = _enet_arl_read( e->mac, &e->vid, &e->val )) == FALSE)
                {
                    ret = _enet_arl_read_ext(e->unit, e->mac, &e->vid, &e->val );
                }
            }

            if (ret == FALSE)
            {
                return BCM_E_ERROR;
            }
            break;

        case TYPE_SET:
            enet_dbg("set e->mac: %02x %02x %02x %02x %02x %02x e->vid: %d\n", e->mac[5],
                    e->mac[4], e->mac[3], e->mac[2], e->mac[1], e->mac[0], e->vid);

            if (IS_UNIT_SF2(e->unit))
            {
                if(e->vid == 0xffff && (e->val & ARL_DATA_ENTRY_VALID) == 0)
                {
                    _enet_arl_remove_ext(e->unit, e->mac);
                }
                else
                {
                    extsw_arl_write_ext(e->unit, e->mac, e->vid, e->val);
                }
            }
            else
            {
                if(e->vid == 0xffff && (e->val & ARL_DATA_ENTRY_VALID) == 0)
                {
                    enet_arl_remove(e->mac);
                }
                else
                {
                    _enet_arl_write(e->mac, e->vid, e->val);
                }
            }
            break;

        case TYPE_DUMP:
            _enet_arl_dump();
            _enet_arl_dump_multiport_arl();

            _enet_arl_dump_ext(e->unit);
            _enet_arl_dump_ext_multiport_arl(e->unit);
            break;

        case TYPE_FLUSH:
            /* Flush the ARL table */
            _fast_age_start_done_ext(e->unit, FAST_AGE_START_DONE | FAST_AGE_DYNAMIC);
             break;

        default:
            return BCM_E_PARAM;
    }
    return BCM_E_NONE;
}

// add by Andrew
int _enet_arl_entry_op_us(struct ethswctl_data *e, int *count, u8 *mac_vid, u32 data)
{
#if 0
    ethsw_mac_entry *me;

    if (*count > MAX_MAC_ENTRY) 
        return FALSE;

    me = &(e->mac_table.entry[*count]);
    memcpy(&me->mac, &mac_vid[2], 6);

    me->port = data&0x1ff;

    (*count)++;

    // return TRUE to terminate the loop
#endif
    return FALSE;
}

int _enet_arl_dump_ext_us(struct ethswctl_data *e)
{
#if 0
    int timeout = 1000, count = 0, hash_ent;
    uint32_t cur_data;
    uint8_t v8, mac_vid[8];

    v8 = ARL_SRCH_CTRL_START_DONE;
    SF2SW_WREG(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL_531xx, (uint8_t *)&v8, 1);
    for( SF2SW_RREG(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL_531xx, (uint8_t *)&v8, 1);
            (v8 & ARL_SRCH_CTRL_START_DONE);
            SF2SW_RREG(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL_531xx, (uint8_t *)&v8, 1))
    {
        /* Now read the Search Ctrl Reg Until :
         * Found Valid ARL Entry --> ARL_SRCH_CTRL_SR_VALID, or
         * ARL Search done --> ARL_SRCH_CTRL_START_DONE */
        for(timeout = 1000;
                (v8 & ARL_SRCH_CTRL_SR_VALID) == 0 && (v8 & ARL_SRCH_CTRL_START_DONE) && timeout-- > 0;
                mdelay(1),
                SF2SW_RREG(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL_531xx, (uint8_t *)&v8, 1));

        if ((v8 & ARL_SRCH_CTRL_SR_VALID) == 0 || timeout <= 0) break;

        /* Found a valid entry */
        for (hash_ent = 0; hash_ent < REG_ARL_SRCH_HASH_ENTS; hash_ent++)
        {
            SF2SW_RREG(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_MAC_LO_ENTRY0_531xx + hash_ent*0x10,&mac_vid[0], 8|DATA_TYPE_VID_MAC);
            SF2SW_RREG(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_DATA_ENTRY0_531xx + hash_ent*0x10,(uint8_t *)&cur_data, 4);

            if ((cur_data & ARL_DATA_ENTRY_VALID_531xx))
            {
                if (_enet_arl_entry_op_us(e, &count, mac_vid, cur_data)) 
                    return TRUE;
            }
        }
    }
    e->mac_table.count = count;
    e->mac_table.len = count*sizeof(ethsw_mac_entry) + 8;

#endif
    return TRUE;
}

int ioctl_extsw_arl_dump(struct ethswctl_data *e)
{
#if 0
    if (e->type != TYPE_DUMP) 
        return BCM_E_PARAM;

    _enet_arl_dump_ext_us(e); 

#endif
    return BCM_E_NONE;
}
// end of add

int remove_arl_entry_wrapper(void *ptr)
{
    int ret = 0;
    ret = enet_arl_remove(ptr);         /* remove entry from internal switch */
    if (IS_UNIT_SF2(0))
        ret |= _enet_arl_remove_ext(0, ptr);
    if (IS_UNIT_SF2(1))
        ret |= _enet_arl_remove_ext(1, ptr);
    return ret;
}

// ----------- SIOCETHSWCTLOPS ETHSWJUMBO functions ---
static uint32 _ConfigureJumboPort(uint32 regVal, int portVal, unsigned int configVal) // bill
{
    // based on impl5:ConfigureJumboPort()
    UINT32 controlBit;

    // Test for valid port specifier.
    if ((portVal >= ETHSWCTL_JUMBO_PORT_GPHY_0) && (portVal <= ETHSWCTL_JUMBO_PORT_ALL))
    {
        // Switch on port ID.
        switch (portVal)
        {
            case ETHSWCTL_JUMBO_PORT_USB:
                controlBit = ETHSWCTL_JUMBO_PORT_USB_MASK;
                break;
            case ETHSWCTL_JUMBO_PORT_GPON_SERDES:
                controlBit = ETHSWCTL_JUMBO_PORT_GPON_SERDES_MASK;
                break;
            case ETHSWCTL_JUMBO_PORT_GMII_2:
                controlBit = ETHSWCTL_JUMBO_PORT_GMII_2_MASK;
                break;
            case ETHSWCTL_JUMBO_PORT_GMII_1:
                controlBit = ETHSWCTL_JUMBO_PORT_GMII_1_MASK;
                break;
            case ETHSWCTL_JUMBO_PORT_GPHY_1:
                controlBit = ETHSWCTL_JUMBO_PORT_GPHY_1_MASK;
                break;
            case ETHSWCTL_JUMBO_PORT_GPHY_0:
                controlBit = ETHSWCTL_JUMBO_PORT_GPHY_0_MASK;
                break;
            default: // ETHSWCTL_JUMBO_PORT_ALL:
                controlBit = ETHSWCTL_JUMBO_PORT_MASK_VAL;  // ALL bits
                break;
        }

        // Test for accept JUMBO frames.
        if (configVal != 0)
        {
            // Setup register value to accept JUMBO frames.
            regVal |= controlBit;
        }
        else
        {
            // Setup register value to reject JUMBO frames.
            regVal &= ~controlBit;
        }
    }

    // Return new JUMBO configuration control register value.
    return regVal;
}

int ioctl_extsw_port_jumbo_control(struct ethswctl_data *e)
{
    // based on impl5:bcmeapi_ioctl_extsw_port_jumbo_control()
    uint32_t val32;

    if (e->type == TYPE_GET)
    {
        // Read & log current JUMBO configuration control register.
        SF2SW_RREG(e->unit, PAGE_JUMBO, REG_JUMBO_PORT_MASK, (uint8 *)&val32, 4);
        enet_dbg("JUMBO_PORT_MASK = 0x%08X", (unsigned int)val32);
        e->ret_val = val32;
    }
    else
    {
        // Read & log current JUMBO configuration control register.
        SF2SW_RREG(e->unit, PAGE_JUMBO, REG_JUMBO_PORT_MASK, (uint8 *)&val32, 4);
        enet_dbg("Old JUMBO_PORT_MASK = 0x%08X", (unsigned int)val32);

        // Setup JUMBO configuration control register.
        val32 = _ConfigureJumboPort(val32, e->port, e->val);
        SF2SW_WREG(e->unit, PAGE_JUMBO, REG_JUMBO_PORT_MASK, (uint8 *)&val32, 4);

        // Attempt to transfer register write value to user space & test for success.
        e->ret_val = val32;
    }
    return BCM_E_NONE;
}

// ----------- SIOCETHSWCTLOPS ETHSWPORTRXRATE functions ---
/*
 *  Set the burst size and rate limit value of the selected port ingress rate.
 *      limit  :   rate limit value. (Kbits)
 *      burst_size  :   max burst size. (Kbits)
 */
int ioctl_extsw_port_irc_set(struct ethswctl_data *e)
{
    // based on impl5:bcmeapi_ioctl_ethsw_port_irc_set()
    uint32_t  val32, bs = 0, rf = 0;

    down(&sw_p->s.conf_sem);

    if (e->limit == 0) { /* Disable ingress rate control */
        SF2SW_RREG(e->unit, PAGE_PORT_STORM_CONTROL, REG_PN_STORM_CTL_RATE_PORT_0 + e->port * 4, &val32, 4);
        val32 &= ~REG_PN_BUCK1_ENABLE_MASK;
        SF2SW_WREG(e->unit, PAGE_PORT_STORM_CONTROL, REG_PN_STORM_CTL_RATE_PORT_0 + e->port * 4, &val32, 4);
    } else {    /* Enable ingress rate control */
        bs = e->burst_size / 8;
        if (bs <= 4)
        {
            bs = 0;
        }
        else if (bs <= 8)
        {
            bs = 1;
        }
        else if (bs <= 16)
        {
            bs = 2;
        }
        else if (bs <= 32)
        {
            bs = 3;
        }
        else if (bs <= 64)
        {
            bs = 4;
        }
        else
        {
            bs = REG_PN_BUCK1_SIZE_M;
        }

        rf = e->limit;
        if (rf <= 1800)
        {
            rf = rf * 125 / 8 / 1000;
            if (rf == 0) rf = 1;
        }
        else if (rf <= 100000)
        {
            rf = rf / 1000 + 27;
        }
        else
        {
            rf = rf / 1000 / 8 + 115;
            if ( rf > 240) rf = 240;
        }

        SF2SW_RREG(e->unit, PAGE_PORT_STORM_CONTROL, REG_PN_STORM_CTL_RATE_PORT_0 + e->port * 4, &val32, 4);
        val32 &= ~((REG_PN_BUCK1_SIZE_M << REG_PN_BUCK1_SIZE_S)| (REG_PN_BUCK1_REF_CNT_M << REG_PN_BUCK1_REF_CNT_S));
        val32 |= REG_PN_BUCK1_ENABLE_MASK | REG_PN_BUCK1_MODE_MASK; // use bucket 1
        val32 |= (rf & REG_PN_BUCK1_REF_CNT_M) << REG_PN_BUCK1_REF_CNT_S;
        val32 |= bs  << REG_PN_BUCK1_SIZE_S;
        SF2SW_WREG(e->unit, PAGE_PORT_STORM_CONTROL, REG_PN_STORM_CTL_RATE_PORT_0 + e->port * 4, &val32, 4);

        SF2SW_RREG(e->unit, PAGE_PORT_STORM_CONTROL, REG_PN_STORM_CTL_CFG_PORT_0 + e->port * 2, &val32, 4);
	    val32 |= REG_PN_BUCK1_IFG_BYTES_MASK | (REG_PN_BUCK1_PKT_SEL_M << REG_PN_BUCK1_PKT_SEL_S);
        SF2SW_WREG(e->unit, PAGE_PORT_STORM_CONTROL, REG_PN_STORM_CTL_CFG_PORT_0 + e->port * 2, &val32, 4);
    }

    up(&sw_p->s.conf_sem);
    return BCM_E_NONE;
}

/*
 *   Get the burst size and rate limit value of the selected port ingress rate.
 */
int ioctl_extsw_port_irc_get(struct ethswctl_data *e)
{
    // based on impl5:bcmeapi_ioctl_ethsw_port_irc_get()
    uint32_t  val32, rf;

    down(&sw_p->s.conf_sem);

    SF2SW_RREG(e->unit, PAGE_PORT_STORM_CONTROL, REG_PN_STORM_CTL_RATE_PORT_0 + e->port * 4, &val32, 4);
    if ((val32 & REG_PN_BUCK1_ENABLE_MASK) == 0)
    {
        e->limit = 0;
    }
    else
    {
        rf  = (val32 >> REG_PN_BUCK1_REF_CNT_S) & REG_PN_BUCK1_REF_CNT_M;
        if (rf <= 28)
        {
            e->limit = rf * 8000 / 125;
        }
        else if (rf <= 127)
        {
            e->limit = (rf - 27 ) * 1000;
        }
        else
        {
            if (rf > 240) rf = 240;
            e->limit = (rf - 115) * 1000 * 8;
        }

        e->burst_size = 4 << ((val32 >> REG_PN_BUCK1_SIZE_S) & REG_PN_BUCK1_SIZE_M);
        if (e->burst_size > 64) e->burst_size = 488;
        e->burst_size *= 8;
    }

    up(&sw_p->s.conf_sem);
    return BCM_E_NONE;
}

// ----------- SIOCETHSWCTLOPS ETHSWDOSCTRL functions ---
int ioctl_extsw_dos_ctrl(struct ethswctl_data *e)
{
    // based on impl5:enet_ioctl_ethsw_dos_ctrl()
    if (e->type == TYPE_GET)
    {
        uint32_t v32 = 0;
        uint8_t v8 = 0;

        SF2SW_RREG(e->unit, PAGE_DOS_PREVENT_531xx, REG_DOS_CTRL, (uint8 *)&v32, 4);
        /* Taking short-cut : Not following BCM coding guidelines */
        if (v32 & DOS_CTRL_DA_SA_DROP_EN)  e->dosCtrl.da_eq_sa_drop_en = 1;
        if (v32 & IP_LAN_DROP_EN)  e->dosCtrl.ip_lan_drop_en = 1;
        if (v32 & TCP_BLAT_DROP_EN)  e->dosCtrl.tcp_blat_drop_en = 1;
        if (v32 & UDP_BLAT_DROP_EN)  e->dosCtrl.udp_blat_drop_en = 1;
        if (v32 & TCP_NULL_SCAN_DROP_EN)  e->dosCtrl.tcp_null_scan_drop_en = 1;
        if (v32 & TCP_XMAS_SCAN_DROP_EN)  e->dosCtrl.tcp_xmas_scan_drop_en = 1;
        if (v32 & TCP_SYNFIN_SCAN_DROP_EN)  e->dosCtrl.tcp_synfin_scan_drop_en = 1;
        if (v32 & TCP_SYNERR_SCAN_DROP_EN)  e->dosCtrl.tcp_synerr_drop_en = 1;
        if (v32 & TCP_SHORTHDR_SCAN_DROP_EN)  e->dosCtrl.tcp_shorthdr_drop_en = 1;
        if (v32 & TCP_FRAGERR_SCAN_DROP_EN)  e->dosCtrl.tcp_fragerr_drop_en = 1;
        if (v32 & ICMPv4_FRAG_DROP_EN)  e->dosCtrl.icmpv4_frag_drop_en = 1;
        if (v32 & ICMPv6_FRAG_DROP_EN)  e->dosCtrl.icmpv6_frag_drop_en = 1;
        if (v32 & ICMPv4_LONGPING_DROP_EN)  e->dosCtrl.icmpv4_longping_drop_en = 1;
        if (v32 & ICMPv6_LONGPING_DROP_EN)  e->dosCtrl.icmpv6_longping_drop_en = 1;

        SF2SW_RREG(e->unit, PAGE_DOS_PREVENT_531xx, REG_DOS_DISABLE_LRN, (uint8 *)&v8, 1);
        if (v8 & DOS_DISABLE_LRN) e->dosCtrl.dos_disable_lrn = 1;
    }
    else if (e->type == TYPE_SET)
    {
        uint32_t v32 = 0;
        uint8_t v8 = 0;
        /* Taking short-cut : Not following BCM coding guidelines */
        if (e->dosCtrl.da_eq_sa_drop_en) v32 |= DOS_CTRL_DA_SA_DROP_EN;
        if (e->dosCtrl.ip_lan_drop_en) v32 |= IP_LAN_DROP_EN;
        if (e->dosCtrl.tcp_blat_drop_en) v32 |= TCP_BLAT_DROP_EN;
        if (e->dosCtrl.udp_blat_drop_en) v32 |= UDP_BLAT_DROP_EN;
        if (e->dosCtrl.tcp_null_scan_drop_en) v32 |= TCP_NULL_SCAN_DROP_EN;
        if (e->dosCtrl.tcp_xmas_scan_drop_en) v32 |= TCP_XMAS_SCAN_DROP_EN;
        if (e->dosCtrl.tcp_synfin_scan_drop_en) v32 |= TCP_SYNFIN_SCAN_DROP_EN;
        if (e->dosCtrl.tcp_synerr_drop_en) v32 |= TCP_SYNERR_SCAN_DROP_EN;
        if (e->dosCtrl.tcp_shorthdr_drop_en) v32 |= TCP_SHORTHDR_SCAN_DROP_EN;
        if (e->dosCtrl.tcp_fragerr_drop_en) v32 |= TCP_FRAGERR_SCAN_DROP_EN;
        if (e->dosCtrl.icmpv4_frag_drop_en) v32 |= ICMPv4_FRAG_DROP_EN;
        if (e->dosCtrl.icmpv6_frag_drop_en) v32 |= ICMPv6_FRAG_DROP_EN;
        if (e->dosCtrl.icmpv4_longping_drop_en) v32 |= ICMPv4_LONGPING_DROP_EN;
        if (e->dosCtrl.icmpv6_longping_drop_en) v32 |= ICMPv6_LONGPING_DROP_EN;

        /* Enable DOS attack blocking functions) */
        SF2SW_WREG(e->unit, PAGE_DOS_PREVENT_531xx, REG_DOS_CTRL, (uint8 *)&v32, 4);
        if (e->dosCtrl.dos_disable_lrn)
        { /* Enable */
            v8 = DOS_DISABLE_LRN;
        }
        else
        {
            v8 = 0;
        }
        SF2SW_WREG(e->unit, PAGE_DOS_PREVENT_531xx, REG_DOS_DISABLE_LRN, (uint8 *)&v8, 1);
    }
    return BCM_E_NONE;
}

// ----------- SIOCETHSWCTLOPS ETHSWPORTSTORMCTRL functions ---
int ioctl_extsw_port_storm_ctrl(struct ethswctl_data *e)
{
    // based on impl5:bcmeapi_ioctl_extsw_port_storm_ctrl()
    uint32_t val32;

    down(&sw_p->s.conf_sem);
    if (e->type == TYPE_SET) {
        /* configure storm control rate & burst size */
        SF2SW_RREG(e->unit, PAGE_PORT_STORM_CONTROL, REG_PN_STORM_CTL_RATE_PORT_0 + e->port * 4, &val32, 4);
        val32 |= REG_PN_BUCK0_ENABLE_MASK | REG_PN_BUCK0_MODE_MASK; // use bucket 0
        val32 &= ~(REG_PN_BUCK0_REF_CNT_M << REG_PN_BUCK0_REF_CNT_S); 
        val32 &= ~(REG_PN_BUCK0_SIZE_M << REG_PN_BUCK0_SIZE_S); 
        val32 |= (e->limit & REG_PN_BUCK0_REF_CNT_M) << REG_PN_BUCK0_REF_CNT_S;
        val32 |= (e->burst_size & REG_PN_BUCK0_SIZE_M) << REG_PN_BUCK0_SIZE_S;
        SF2SW_WREG(e->unit, PAGE_PORT_STORM_CONTROL, REG_PN_STORM_CTL_RATE_PORT_0 + e->port * 4, &val32, 4);

        /* pkt type */
        SF2SW_RREG(e->unit, PAGE_PORT_STORM_CONTROL, REG_PN_STORM_CTL_CFG_PORT_0 + e->port * 2, &val32, 4);
	    val32 &= ~(REG_PN_BUCK0_PKT_SEL_M << REG_PN_BUCK0_PKT_SEL_S);
	    val32 |= (e->pkt_type_mask & REG_PN_BUCK0_PKT_SEL_M) << REG_PN_BUCK0_PKT_SEL_S;
        SF2SW_WREG(e->unit, PAGE_PORT_STORM_CONTROL, REG_PN_STORM_CTL_CFG_PORT_0 + e->port * 2, &val32, 4);
    } else {
        SF2SW_RREG(e->unit, PAGE_PORT_STORM_CONTROL, REG_PN_STORM_CTL_RATE_PORT_0 + e->port * 4, &val32, 4);
        e->limit = (val32 >> REG_PN_BUCK0_REF_CNT_S) & REG_PN_BUCK0_REF_CNT_M;
        e->burst_size = (val32 >> REG_PN_BUCK0_SIZE_S) & REG_PN_BUCK0_SIZE_M;

        SF2SW_RREG(e->unit, PAGE_PORT_STORM_CONTROL, REG_PN_STORM_CTL_CFG_PORT_0 + e->port * 2, &val32, 4);
        e->pkt_type_mask = (val32 >> REG_PN_BUCK0_PKT_SEL_S) & REG_PN_BUCK0_PKT_SEL_M;
    }

    up(&sw_p->s.conf_sem);
    return 0;
}

// ----------- SIOCETHSWCTLOPS ETHSWHWSTP functions ---
// Set STP state into SF2 register
void _ethsw_set_stp_mode(unsigned int unit, unsigned int port, unsigned char stpState)
{
    // based on impl5:bcmeapi_ethsw_set_stp_mode()
   unsigned char portInfo;

   if(IS_UNIT_SF2(unit)) // SF2
   {
      spin_lock_bh(&extsw_reg_config);
      SF2SW_RREG(unit, PAGE_CONTROL, REG_PORT_CTRL + (port),
                 &portInfo, sizeof(portInfo));
      portInfo &= ~REG_PORT_STP_MASK;
      portInfo |= stpState;
      SF2SW_WREG(unit, PAGE_CONTROL, REG_PORT_CTRL + (port),
                 &portInfo, sizeof(portInfo));
      spin_unlock_bh(&extsw_reg_config);
   }
}

int _ethsw_get_stp_state(unsigned int unit, unsigned int port)
{
   unsigned char portInfo;

   if(IS_UNIT_SF2(unit)) // SF2
   {
      SF2SW_RREG(unit, PAGE_CONTROL, REG_PORT_CTRL + (port),
                 &portInfo, sizeof(portInfo));
      return (portInfo & REG_PORT_STP_MASK);
   }
   return 0;
}

// ----------- static switch functions -----------------------
static void _fast_age_start_done_ext(int unit, uint8_t ctrl)
{
    uint8_t timeout = 100;

    SF2SW_WREG(unit, PAGE_CONTROL, REG_FAST_AGING_CTRL, &ctrl, 1);
    SF2SW_RREG(unit, PAGE_CONTROL, REG_FAST_AGING_CTRL, &ctrl, 1);
    while (ctrl & FAST_AGE_START_DONE) {
        mdelay(1);
        SF2SW_RREG(unit, PAGE_CONTROL, REG_FAST_AGING_CTRL, &ctrl, 1);
        if (!timeout--) {
            printk("Timeout of fast aging \n");
            break;
        }
    }

    /* Restore DYNAMIC bit for normal aging */
    ctrl = FAST_AGE_DYNAMIC;
    SF2SW_WREG(unit, PAGE_CONTROL, REG_FAST_AGING_CTRL, &ctrl, 1);
}


// =========== switch port ops =============================
#include "linux/if_bridge.h"
int port_sw_port_stp_set(enetx_port_t *self, int mode, int state)
{
    // based on impl5:bcm_set_hw_stp()
    int unit = PORT_ON_ROOT_SW(self)?0:1;
    int port = self->p.mac->mac_id;
    int swPort= PHYSICAL_PORT_TO_LOGICAL_PORT(port, unit);
    unsigned char stpVal;

    //enet_dbgv("%s mode=%d state=%d\n", self->obj_name, mode, state);
    switch (mode)
    {
    case STP_MODE_ENABLE:
        root_sw->s.stpDisabledPortMap &= ~(1 << swPort);
        /* set stp state to forward in case bridge STP is off 
           if bridge STP is on, STP state machine will be forced to restart */
        _ethsw_set_stp_mode(unit, port, REG_PORT_STP_STATE_FORWARDING);
        break;
    case STP_MODE_DISABLE:
        root_sw->s.stpDisabledPortMap |= (1 << swPort);
        _ethsw_set_stp_mode(unit, port, REG_PORT_STP_STATE_FORWARDING);
        break;
    default:    // STP_MODE_UNCHANGED
        if (state == STP_STATE_UNCHANGED)
            switch (_ethsw_get_stp_state(unit, port))
            {
            case REG_PORT_STP_STATE_FORWARDING: return STP_FORWARDING;
            case REG_PORT_STP_STATE_LEARNING:   return STP_LEARNING;
            case REG_PORT_STP_STATE_LISTENING:  return STP_LISTENING;
            case REG_PORT_STP_STATE_BLOCKING:   return STP_BLOCKING;
            default:                            return STP_DISABLED;
            }

        if (root_sw->s.stpDisabledPortMap & (1<<swPort))
            break;
        switch (state)
        {
        case STP_BLOCKING:     stpVal= REG_PORT_STP_STATE_BLOCKING; break;
        case STP_FORWARDING:   stpVal= REG_PORT_STP_STATE_FORWARDING; break;
        case STP_LEARNING:     stpVal= REG_PORT_STP_STATE_LEARNING; break;
        case STP_LISTENING:    stpVal= REG_PORT_STP_STATE_LISTENING; break;
        case STP_DISABLED:     //stpVal= REG_PORT_STP_STATE_DISABLED; break;
        default:               stpVal= REG_PORT_NO_SPANNING_TREE; break;
        }
        _ethsw_set_stp_mode(unit, port, stpVal);
    }
    return 0;
}

void port_sw_port_fast_age(enetx_port_t *port)
{
    uint8_t ctrl;
    int unit = PORT_ON_ROOT_SW(port)?0:1;

    //enet_dbgv(" port=%s\n", port->obj_name);
    ctrl = port->p.mac->mac_id;
    SF2SW_WREG(unit, PAGE_CONTROL, REG_FAST_AGING_PORT, &ctrl, 1);
    ctrl = FAST_AGE_START_DONE | FAST_AGE_DYNAMIC | FAST_AGE_PORT;
    _fast_age_start_done_ext(unit, ctrl);
}

extern uint32_t queRemap;
extern int bcmenet_sysport_q_map(uint32_t queRemap);

// REG_WAN_PORT_MAP when bit is set
//      - port has WAN role self->n.port_ndev_role == PORT_NETDEV_ROLE_WAN
//   or - port has LAN role but is hw-switching disabled
int port_sw_port_role_set(enetx_port_t *self, port_netdev_role_t role)
{
    /* based on impl5:bcmsw_config_wan() */
    uint16_t wan_port_map;
    bcmFun_t *enet_port_role_notify = bcmFun_get(BCM_FUN_ID_ENET_PORT_ROLE_NOTIFY);
    int unit = PORT_ON_ROOT_SW(self)?0:1;

    if (role < PORT_NETDEV_ROLE_NONE || role > PORT_NETDEV_ROLE_WAN)
        return 0;

   /* Configure WAN port */
    SF2SW_RREG(unit, PAGE_CONTROL, REG_WAN_PORT_MAP, &wan_port_map, sizeof(wan_port_map));

    if (role == PORT_NETDEV_ROLE_LAN)
    {
        wan_port_map &= ~(1<<self->p.mac->mac_id); /* remove the WAN port in the port map */
    }
    else
    {
        wan_port_map |= (1<<self->p.mac->mac_id); /* Add the WAN port in the port map */
    }
    SF2SW_WREG(unit, PAGE_CONTROL, REG_WAN_PORT_MAP, &wan_port_map, sizeof(wan_port_map));

    enet_dbg(" %s port %s as WAN; wan_pmap <0x%02x>\n", (role!=PORT_NETDEV_ROLE_LAN)?"Add":"Remove", self->obj_name, wan_port_map);
    /* Disable learning */
    SF2SW_WREG(unit, PAGE_CONTROL, REG_DISABLE_LEARNING, &wan_port_map, sizeof(wan_port_map));

    /* NOTE : No need to change the PBVLAN map -- switch logic does not care about pbvlan when the port is WAN */

    /* NOTE: For multiple IMP port products, switch will send all traffic from WAN to P8
       but we should use CFP to send it to correct port based on port grouping
       TBD */

    /* registered modules need to be aware of port role changes */
    if (enet_port_role_notify && (role != PORT_NETDEV_ROLE_NONE) && self->has_interface)
    {
        BCM_EnetPortRole_t port_role;

#if defined(CONFIG_BCM96756)
        port_role.sysport = 0;
        port_role.switch_id = unit;
#else
        port_role.sysport = PORT_ON_ROOT_SW(self) ? 0 : self->p.parent_sw->s.parent_port->p.mac->mac_id;
        port_role.switch_id = 0;
#endif
        port_role.port = self->p.mac->mac_id;
        port_role.is_wan = (self->n.port_netdev_role == PORT_NETDEV_ROLE_WAN);

        enet_port_role_notify(&port_role);
#if defined(ARCHER_DEVICE)
        bcmenet_sysport_q_map(queRemap);
#endif
    }

    port_set_wan_role_link(self, role);

    return 0;
}

// add by Andrew
// ----------- SIOCETHSWCTLOPS ETHSWMIBDUMP functions ---
int port_sf2_mib_dump_us(enetx_port_t *self, void *ethswctl)
{
#if 0
    struct ethswctl_data *e = (struct ethswctl_data *)ethswctl;
    unsigned int v32;
    uint8_t data[8] = {0};
    int port = self->p.mac->mac_id;

    {
        SF2SW_RREG(PAGE_CONTROL, REG_LOW_POWER_EXP1, &v32, 4);
        if (v32 & (1<<port)) {
            enet_err("port=%d is in low power mode - mib counters not accessible!!\n", port);    // SLEEP_SYSCLK_PORT for specified port is set
            return 0;
        }
    }

    /* Calculate Tx statistics */
    e->port_stats.txPackets = 0;
    SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXUPKTS, &v32, 4);  // Get TX unicast packet count
    e->port_stats.txPackets += v32;
    SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXMPKTS, &v32, 4);  // Get TX multicast packet count
    e->port_stats.txPackets += v32;
    SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXBPKTS, &v32, 4);  // Get TX broadcast packet count
    e->port_stats.txPackets += v32;

    SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXDROPS, &v32, 4);
    e->port_stats.txDrops = v32;

    SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXOCTETS, data, DATA_TYPE_HOST_ENDIAN|8);
    e->port_stats.txBytes = *((uint64 *)data);

    /* Calculate Rx statistics */
    e->port_stats.rxPackets = 0;
    SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXUPKTS, &v32, 4);  // Get RX unicast packet count
    e->port_stats.rxPackets += v32;
    SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXMPKTS, &v32, 4);  // Get RX multicast packet count
    e->port_stats.rxPackets += v32;
    SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXBPKTS, &v32, 4);  // Get RX broadcast packet count
    e->port_stats.rxPackets += v32;

    SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXDROPS, &v32, 4);
    e->port_stats.rxDrops = v32;

    SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXDISCARD, &v32, 4);
    e->port_stats.rxDiscards = v32;

    SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXOCTETS, data,  DATA_TYPE_HOST_ENDIAN|8);
    e->port_stats.rxBytes = *((uint64 *)data);

#endif
    return 0;
}
// end of add

// ----------- SIOCETHSWCTLOPS ETHSWDUMPMIB functions ---

/* mib dump for ports on external SF2 switch */
int port_sw_mib_dump(enetx_port_t *self, int all)
{
    /* based on impl5:sf2_bcmsw_dump_mib_ext() */
    unsigned int v32, errcnt;
    uint8_t data[8] = {0};
    int port = self->p.mac->mac_id;
    int unit = PORT_ON_ROOT_SW(self)?0:1;

    {
        SF2SW_RREG(unit, PAGE_CONTROL, REG_LOW_POWER_EXP1, &v32, 4);
        if (v32 & (1<<port)) {
            enet_err("port=%d is in low power mode - mib counters not accessible!!\n", port);    // SLEEP_SYSCLK_PORT for specified port is set
            return 0;
        }
    }

    /* Display Tx statistics */
    printk("\nSF2 Switch Stats: Unit# %d Port# %d\n", unit, port);
    SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXUPKTS, &v32, 4);  // Get TX unicast packet count
    printk("TxUnicastPkts:          %10u \n", v32);
    SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXMPKTS, &v32, 4);  // Get TX multicast packet count
    printk("TxMulticastPkts:        %10u \n",  v32);
    SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXBPKTS, &v32, 4);  // Get TX broadcast packet count
    printk("TxBroadcastPkts:        %10u \n", v32);
    SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXDROPS, &v32, 4);
    printk("TxDropPkts:             %10u \n", v32);

    if (all)
    {
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXOCTETS, data, DATA_TYPE_HOST_ENDIAN|8);
        v32 = *((uint64 *)data);
        printk("TxOctetsLo:             %10u \n", v32);
        v32 = *((uint64 *)data) >> 32;
        printk("TxOctetsHi:             %10u \n", v32);
//
#ifdef SF2_DEVICE
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TX64OCTPKTS, &v32, 4);
        printk("TxPkts64Octets:         %10u \n", v32);
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TX127OCTPKTS, &v32, 4);
        printk("TxPkts65to127Octets:    %10u \n", v32);
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TX255OCTPKTS, &v32, 4);
        printk("TxPkts128to255Octets:   %10u \n", v32);
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TX511OCTPKTS, &v32, 4);
        printk("TxPkts256to511Octets:   %10u \n", v32);
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TX1023OCTPKTS, &v32, 4);
        printk("TxPkts512to1023Octets:  %10u \n", v32);
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXMAXOCTPKTS, &v32, 4);
        printk("TxPkts1024OrMoreOctets: %10u \n", v32);
#endif
//
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXQ0PKT, &v32, 4);
        printk("TxQ0Pkts:               %10u \n", v32);
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXQ1PKT, &v32, 4);
        printk("TxQ1Pkts:               %10u \n", v32);
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXQ2PKT, &v32, 4);
        printk("TxQ2Pkts:               %10u \n", v32);
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXQ3PKT, &v32, 4);
        printk("TxQ3Pkts:               %10u \n", v32);
#ifdef SF2_DEVICE
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXQ4PKT, &v32, 4);
        printk("TxQ4Pkts:               %10u \n", v32);
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXQ5PKT, &v32, 4);
        printk("TxQ5Pkts:               %10u \n", v32);
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXQ6PKT, &v32, 4);
        printk("TxQ6Pkts:               %10u \n", v32);
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXQ7PKT, &v32, 4);
        printk("TxQ7Pkts:               %10u \n", v32);
#endif
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXCOL, &v32, 4);
        printk("TxCol:                  %10u \n", v32);
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXSINGLECOL, &v32, 4);
        printk("TxSingleCol:            %10u \n", v32);
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXMULTICOL, &v32, 4);
        printk("TxMultipleCol:          %10u \n", v32);
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXDEFERREDTX, &v32, 4);
        printk("TxDeferredTx:           %10u \n", v32);
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXLATECOL, &v32, 4);
        printk("TxLateCol:              %10u \n", v32);
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXEXCESSCOL, &v32, 4);
        printk("TxExcessiveCol:         %10u \n", v32);
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXFRAMEINDISC, &v32, 4);
        printk("TxFrameInDisc:          %10u \n", v32);
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXPAUSEPKTS, &v32, 4);
        printk("TxPausePkts:            %10u \n", v32);
    }
    else
    {
        errcnt=0;
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXCOL, &v32, 4);
        errcnt += v32;
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXSINGLECOL, &v32, 4);
        errcnt += v32;
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXMULTICOL, &v32, 4);
        errcnt += v32;
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXDEFERREDTX, &v32, 4);
        errcnt += v32;
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXLATECOL, &v32, 4);
        errcnt += v32;
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXEXCESSCOL, &v32, 4);
        errcnt += v32;
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXFRAMEINDISC, &v32, 4);
        errcnt += v32;
        printk("TxOtherErrors:          %10u \n", errcnt);
    }

    /* Display Rx statistics */
    SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXUPKTS, &v32, 4);  // Get RX unicast packet count
    printk("RxUnicastPkts:          %10u \n", v32);
    SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXMPKTS, &v32, 4);  // Get RX multicast packet count
    printk("RxMulticastPkts:        %10u \n",v32);
    SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXBPKTS, &v32, 4);  // Get RX broadcast packet count
    printk("RxBroadcastPkts:        %10u \n",v32);
    SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXDROPS, &v32, 4);
    printk("RxDropPkts:             %10u \n",v32);
    SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXDISCARD, &v32, 4);
    printk("RxDiscard:              %10u \n", v32);

    if (all)
    {
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXOCTETS, data,  DATA_TYPE_HOST_ENDIAN|8);
        v32 = *((uint64 *)data);
        printk("RxOctetsLo:             %10u \n", v32);
        v32 = *((uint64 *)data) >> 32;
        printk("RxOctetsHi:             %10u \n", v32);
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXGOODOCT, data,  DATA_TYPE_HOST_ENDIAN|8);
        v32 = *((uint64 *)data);
        printk("RxGoodOctetsLo:         %10u \n", v32);
        v32 = *((uint64 *)data) >> 32;
        printk("RxGoodOctetsHi:         %10u \n", v32);
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXJABBERS, &v32, 4);
        printk("RxJabbers:              %10u \n", v32);
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXALIGNERRORS, &v32, 4);
        printk("RxAlignErrs:            %10u \n", v32);
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXFCSERRORS, &v32, 4);
        printk("RxFCSErrs:              %10u \n", v32);
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXFRAGMENTS, &v32, 4);
        printk("RxFragments:            %10u \n", v32);
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXOVERSIZE, &v32, 4);
        printk("RxOversizePkts:         %10u \n", v32);
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXUNDERSIZEPKTS, &v32, 4);
        printk("RxUndersizePkts:        %10u \n", v32);
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXPAUSEPKTS, &v32, 4);
        printk("RxPausePkts:            %10u \n", v32);
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXSACHANGES, &v32, 4);
        printk("RxSAChanges:            %10u \n", v32);
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXSYMBOLERRORS, &v32, 4);
        printk("RxSymbolError:          %10u \n", v32);
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RX64OCTPKTS, &v32, 4);
        printk("RxPkts64Octets:         %10u \n", v32);
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RX127OCTPKTS, &v32, 4);
        printk("RxPkts65to127Octets:    %10u \n", v32);
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RX255OCTPKTS, &v32, 4);
        printk("RxPkts128to255Octets:   %10u \n", v32);
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RX511OCTPKTS, &v32, 4);
        printk("RxPkts256to511Octets:   %10u \n", v32);
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RX1023OCTPKTS, &v32, 4);
        printk("RxPkts512to1023Octets:  %10u \n", v32);
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXMAXOCTPKTS, &v32, 4);
        printk("RxPkts1024OrMoreOctets: %10u \n", v32);
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXJUMBOPKT , &v32, 4);
        printk("RxJumboPkts:            %10u \n", v32);
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXOUTRANGEERR, &v32, 4);
        printk("RxOutOfRange:           %10u \n", v32);
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXINRANGEERR, &v32, 4);
        printk("RxInRangeErr:           %10u \n", v32);
    }
    else
    {
        errcnt=0;
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXJABBERS, &v32, 4);
        errcnt += v32;
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXALIGNERRORS, &v32, 4);
        errcnt += v32;
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXFCSERRORS, &v32, 4);
        errcnt += v32;
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXFRAGMENTS, &v32, 4);
        errcnt += v32;
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXOVERSIZE, &v32, 4);
        errcnt += v32;
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXUNDERSIZEPKTS, &v32, 4);
        errcnt += v32;
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXSYMBOLERRORS, &v32, 4);
        errcnt += v32;
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXOUTRANGEERR, &v32, 4);
        errcnt += v32;
        SF2SW_RREG(unit, PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXINRANGEERR, &v32, 4);
        errcnt += v32;
        printk("RxOtherErrors:          %10u \n", errcnt);

    }

    return 0;
}

// =========== switch ops =============================
void port_sw_fast_age(enetx_port_t *sw)
{
    int unit = IS_ROOT_SW(sw)?0:1;
    //enet_dbgv(" sw=%s\n", sw->obj_name);
    _fast_age_start_done_ext(unit, FAST_AGE_START_DONE | FAST_AGE_DYNAMIC);
}


