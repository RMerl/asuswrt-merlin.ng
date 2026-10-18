/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * Copyright 2020, ASUSTeK Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

#include <linux/module.h>

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/init.h>
#include <linux/mutex.h>
#include <asm/uaccess.h>
#include <rtk_types.h>
#include <port.h>
#include <stat.h>
#include <l2.h>
#include <rtk_error.h>
#include <rtk_switch.h>
#include <vlan.h>
#include <svlan.h>
#include <rtkled.h>
#include "bcm_OS_Deps.h"
#include <board.h>
#include "rtl8367c_asicdrv.h"
#include "rtl8367c_reg.h"
#include "rtl8367c_asicdrv_fc.h"
#include "testmode.h"

#ifdef CONFIG_DEVFS_FS
#include <linux/devfs_fs_kernel.h>
static devfs_handle_t devfs_handle;
#endif

#define NAME			"rtl8367s"
#define RTL8367S_DEVNAME	"rtkswitch"
int rtl8367s_major = 206;

MODULE_DESCRIPTION("Realtek RTL8367S support");
MODULE_AUTHOR("ASUS");
MODULE_LICENSE("GPL");

#define CONTROL_REG_PORT_POWER_BIT	0x800
#ifdef BR63
#define LED_GROUP_DEFAULT		LED_GROUP_1
#else
#define LED_GROUP_DEFAULT		LED_GROUP_0
#endif

static const unsigned int s_wan_stb_array[7] = {
	/* 0:LLLL	LAN: P0,P1,P2,P3	STB: N/A (default mode) */
	0,
	/* 1:WLLL	LAN: P1,P2,P3		STB: P0 */
	LAN_PORT_1_MASK,
	/* 2:LWLL	LAN: P0,P2,P3		STB: P1 */
	LAN_PORT_2_MASK,
	/* 3:LLWL	LAN: P0,P1,P3		STB: P2 */
	LAN_PORT_3_MASK,
	/* 4:LLLW	LAN: P0,P1,P2		STB: P3 */
	LAN_PORT_4_MASK,
	/* 5:WWLL	LAN: P2,P3		STB: P0,P1 */
	LAN_PORT_1_MASK | LAN_PORT_2_MASK,
	/* 6:LLWW	LAN: P0,P1		STB: P2,P3 */
	LAN_PORT_3_MASK | LAN_PORT_4_MASK,
};

typedef struct {
	unsigned int link[4];
	unsigned int speed[4];
	unsigned int duplex[4];
} phyState;

typedef struct {
	unsigned int count;
	ether_addr_t ea[256];
} mactable;

static int get_wan_stb_lan_port_mask(int wan_stb_x, unsigned int *wan_pmsk, unsigned int *stb_pmsk, unsigned int *lan_pmsk, int need_mac_port)
{
	int ret = 0;
	unsigned int lan_ports_mask = LAN_PORTS_MASK;
	unsigned int wan_ports_mask = 0;

	if (!wan_pmsk || !stb_pmsk || !lan_pmsk)
		return -EINVAL;

	if (need_mac_port)
		lan_ports_mask |= LAN_ALL_PORTS_MASK;

	if (wan_stb_x >= 0 && wan_stb_x < ARRAY_SIZE(s_wan_stb_array)) {
		*stb_pmsk = s_wan_stb_array[wan_stb_x];
		*lan_pmsk = lan_ports_mask & ~*stb_pmsk;
		*wan_pmsk = wan_ports_mask | *stb_pmsk;
	} else {
		printk(KERN_WARNING "%pF() pass invalid invalid wan_stb_x %d to %s()\n",
			__builtin_return_address(0), wan_stb_x, __func__);
		ret = -EINVAL;
	}

	return 0;
}

void vlan_accept_none(void)
{
	rtk_int32 ret_t;
	unsigned int port, mask;

	if (rtk_vlan_init() != RT_ERR_OK)
		printk("VLAN Initial Failed!!!\n");

	printk("%s\n", __func__);
	ENUM_PORT_BEGIN(port, mask, LAN_PORTS_MASK, 1)
		ret_t = rtk_vlan_portAcceptFrameType_set(port, ACCEPT_FRAME_TYPE_UNTAG_ONLY);
		if(ret_t != RT_ERR_OK)
			printk("%s() ERROR port:%d errono:%d\n", __func__, port, ret_t);
	ENUM_PORT_END
}

void vlan_accept_all(void)
{
	rtk_int32 ret_t;
	unsigned int port, mask;
/*
	if (rtk_vlan_init() != RT_ERR_OK)
		printk("VLAN Initial Failed!!!\n");
*/
	printk("%s\n", __func__);
	ENUM_PORT_BEGIN(port, mask, LAN_PORTS_MASK, 1)
		ret_t = rtk_vlan_portAcceptFrameType_set(port, ACCEPT_FRAME_TYPE_ALL);
		if(ret_t != RT_ERR_OK)
			printk("%s() ERROR port:%d errono:%d\n", __func__, port, ret_t);
	ENUM_PORT_END
}

void vlan_accept_all_byports(unsigned int portMask)
{
	rtk_int32 ret_t;
	unsigned int port, mask;

	printk("%s\n", __func__);
	ENUM_PORT_BEGIN(port, mask, portMask, 1)
		ret_t = rtk_vlan_portAcceptFrameType_set(port, ACCEPT_FRAME_TYPE_ALL);
		if(ret_t != RT_ERR_OK)
			printk("%s() ERROR port:%d errono:%d\n", __func__, port, ret_t);
	ENUM_PORT_END
}

void vlan_accept_tagOnly_byports(unsigned int portMask)
{
	rtk_int32 ret_t;
	unsigned int port, mask;

	printk("%s\n", __func__);
	ENUM_PORT_BEGIN(port, mask, portMask, 1)
		ret_t = rtk_vlan_portAcceptFrameType_set(port, ACCEPT_FRAME_TYPE_TAG_ONLY);
		if(ret_t != RT_ERR_OK)
			printk("%s() ERROR port:%d errono:%d\n", __func__, port, ret_t);
	ENUM_PORT_END
}

void vlan_accept_untagOnly_byports(unsigned int portMask)
{
	rtk_int32 ret_t;
	unsigned int port, mask;

	printk("%s\n", __func__);
	ENUM_PORT_BEGIN(port, mask, portMask, 1)
		ret_t = rtk_vlan_portAcceptFrameType_set(port, ACCEPT_FRAME_TYPE_UNTAG_ONLY);
		if(ret_t != RT_ERR_OK)
			printk("%s() ERROR port:%d errono:%d\n", __func__, port, ret_t);
	ENUM_PORT_END
}

void vlan_dump_frameType_all(void)
{
	rtk_int32 ret_t;
	unsigned int port, mask;
	rtk_vlan_acceptFrameType_t vf_type;

	printk("%s:\n", __func__);
	ENUM_PORT_BEGIN(port, mask, LAN_PORTS_MASK, 1)
		ret_t = rtk_vlan_portAcceptFrameType_get(port, &vf_type);
		if(ret_t != RT_ERR_OK)
			printk("%s() ERROR port:%d errono:%d\n", __func__, port, ret_t);
		printk(" p%d(L%d): vf_type is %d\n", port, rtk_switch_port_P2L_get(port), vf_type);
	ENUM_PORT_END
}

static unsigned int is_singtel_mio = 0;

int vlan_accept_adv(int wan_stb_x)
{
	rtk_int32 ret_t;
	unsigned int port, mask;
	unsigned int lan_port_mask = 0, wan_port_mask = 0, stb_port_mask = 0;

	if (rtk_vlan_init() != RT_ERR_OK)
		printk("VLAN Initial Failed!!!\n");

	if (is_singtel_mio) {
		ENUM_PORT_BEGIN(port, mask, LAN_PORTS_MASK, 1)
			ret_t = rtk_vlan_portAcceptFrameType_set(port, ACCEPT_FRAME_TYPE_ALL);
			if(ret_t != RT_ERR_OK)
				printk("%s() ERROR port:%d errono:%d\n", __func__, port, ret_t);
		ENUM_PORT_END
	} else {
		if (get_wan_stb_lan_port_mask(wan_stb_x, &wan_port_mask, &stb_port_mask, &lan_port_mask, 0))
			return -EINVAL;

		ENUM_PORT_BEGIN(port, mask, wan_port_mask, 1)
			ret_t = rtk_vlan_portAcceptFrameType_set(port, ACCEPT_FRAME_TYPE_ALL);
			if(ret_t != RT_ERR_OK)
				printk("%s() ERROR port:%d errono:%d\n", __func__, port, ret_t);
		ENUM_PORT_END
	}

	return 0;
}

static int get_port_isolation_mask_efid(void)
{
	rtk_portmask_t port_isolation_mask;
	unsigned int port, mask;
	rtk_data_t efid;

	printk("%s\n", __func__);

	/* LAN */
	ENUM_PORT_BEGIN(port, mask, LAN_ALL_PORTS_MASK, 1)
		rtk_port_isolation_get(port, &port_isolation_mask);
		rtk_port_efid_get(port, &efid);
		printk("port[%d]/0x%03x, port_isolation_mask:0x%03x, efid:%d\n", port, LAN_ALL_PORTS_MASK, port_isolation_mask.bits[0], efid);
	ENUM_PORT_END

	return 0;
}

static int __LANWANPartition(int wan_stb_x)
{
	rtk_portmask_t fwd_mask;
	unsigned int port, mask;
	unsigned int lan_port_mask = 0, wan_port_mask = 0, stb_port_mask = 0;

	/* add CPU port fwd */
	if (get_wan_stb_lan_port_mask(wan_stb_x, &wan_port_mask, &stb_port_mask, &lan_port_mask, 1))
		return -EINVAL;

	printk(KERN_INFO "wan_stb_x %d STB,LAN/WAN ports mask 0x%03x,%03x/%03x\n",
		wan_stb_x, stb_port_mask, lan_port_mask, wan_port_mask);

	/* LAN */
	ENUM_PORT_BEGIN(port, mask, lan_port_mask, 1)
		fwd_mask.bits[0] = lan_port_mask;
		rtk_port_isolation_set(port, &fwd_mask);
		rtk_port_efid_set(port, 0);
	ENUM_PORT_END

	/* WAN */
	ENUM_PORT_BEGIN(port, mask, wan_port_mask, 1)
		fwd_mask.bits[0] = wan_port_mask;
		rtk_port_isolation_set(port, &fwd_mask);
		rtk_port_efid_set(port, 1);
	ENUM_PORT_END

	return 0;
}

static void ports_isolation(int enable, unsigned int portInfo)
{
	rtk_portmask_t fwd_mask;
	unsigned int port, mask;
	unsigned int port_mask = 0;
	int efid = 0;

	if(enable) {
		ENUM_PORT_BEGIN(port, mask, portInfo, 1)
			port_mask = ( 1 << port ) | LAN_PORT_EXT_MASK;
			fwd_mask.bits[0] = port_mask;
			rtk_port_isolation_set(port, &fwd_mask);
			//rtk_port_efid_set(port, efid);
			efid++;
		ENUM_PORT_END
	} else {
		ENUM_PORT_BEGIN(port, mask, portInfo, 1)
			fwd_mask.bits[0] = LAN_ALL_PORTS_MASK;
			rtk_port_isolation_set(port, &fwd_mask);
			//rtk_port_efid_set(port, 0);
		ENUM_PORT_END
	}
}

void LANWANPartition(void)
{
	__LANWANPartition(0);
}

static int wan_stb_g = 0;

/* Do not use this function cause there is no WAN on current chipset. */
void LANWANPartition_adv(int wan_stb_x)
{
	__LANWANPartition(wan_stb_x);
}

static rtk_vlan_t vlan_vid = 0;
static rtk_vlan_t vlan_prio = 0;
static rtk_uint32 iso_port = 0;
#if 0
static rtk_led_group_t rtk_led_group = 0;	// LED_GROUP_0
#endif
static rtk_data_t TxDelay=0, RxDelay=0;
static rtk_led_group_t led_port_group = 0;
static rtk_port_t phytest_port;

void initialVlan(u32 portinfo)/* Initalize VLAN. */
{
	rtk_int32 ret_t;
	unsigned int port, mask;

	/* set VLAN filtering for each LAN port and CPU port */
	ENUM_PORT_BEGIN(port, mask, LAN_ALL_PORTS_MASK, 1)
		ret_t = rtk_vlan_portIgrFilterEnable_set(port, ENABLED);
		if(ret_t != RT_ERR_OK)
			printk("%s() ERROR port:%d errono:%d\n", __func__, port, ret_t);
	ENUM_PORT_END
}

void setVlanFilter(u32 portinfo)
{
	rtk_int32 ret_t;
	unsigned int port, mask;
	u32 laninfo = 0;

	laninfo = (portinfo & RTK_MAX_PORT_MASK);
	/* set VLAN filtering for each LAN port and CPU port */
	ENUM_PORT_BEGIN(port, mask, laninfo, 1)
		ret_t = rtk_vlan_portIgrFilterEnable_set(port, ENABLED);
		if(ret_t != RT_ERR_OK)
			printk("%s() ERROR port:%d errono:%d\n", __func__, port, ret_t);
	ENUM_PORT_END
}

void getVlanFilter(void)
{
	rtk_int32 ret_t;
	unsigned int port, mask;
	rtk_enable_t pf;

	/* set VLAN filtering for each LAN port and CPU port */
	ENUM_PORT_BEGIN(port, mask, LAN_ALL_PORTS_MASK, 1)
		ret_t = rtk_vlan_portIgrFilterEnable_get(port, &pf);
		if(ret_t != RT_ERR_OK)
			printk("%s() ERROR port:%d errono:%d\n", __func__, port, ret_t);
		printk("%s, p%d IgrFilterEnabled:%d\n", __func__, port, pf);
	ENUM_PORT_END
}

/* portInfo:	bit0-bit15 : member mask
		bit16-bit31 :  untag mask */
void createVlan(u32 portinfo)
{
	rtk_int32 ret_t;
	rtk_vlan_cfg_t vlan_t;
	unsigned int port, mask;
	u32 laninfo = 0;

	memset(&vlan_t, 0x00, sizeof(rtk_vlan_cfg_t));
	vlan_t.mbr.bits[0] = (portinfo & RTK_MAX_PORT_MASK) | LAN_PORT_EXT_MASK; // add CPU port to member
	laninfo = (portinfo & RTK_MAX_PORT_MASK) | LAN_PORT_EXT_MASK; // add CPU port to portPvid set
	vlan_t.untag.bits[0] = portinfo >> 16; // CPU port leave tag
	if (!vlan_vid) {
		vlan_t.untag.bits[0] |= LAN_PORT_EXT_MASK;
		vlan_t.ivl_en = 1;
	}
	printk("createVlan - vid = %d, laninfo:0x%X, prio = %d, mbrmsk = 0x%X, untagmsk = 0x%X\n", vlan_vid, laninfo, vlan_prio, vlan_t.mbr.bits[0], vlan_t.untag.bits[0]);
	ret_t = rtk_vlan_set(vlan_vid, &vlan_t);
	if(ret_t != RT_ERR_OK)
		printk("%s() ERROR vid:%d errono:%d\n", __func__, vlan_vid, ret_t);

	ENUM_PORT_BEGIN(port, mask, laninfo, 1)
		ret_t = rtk_vlan_portPvid_set(port, vlan_vid, vlan_prio);
		if(ret_t != RT_ERR_OK)
			printk("%s() ERROR port:%d errono:%d\n", __func__, port, ret_t);
	ENUM_PORT_END
}

void set_prio(u32 portinfo)
{
	rtk_int32 ret_t;
	unsigned int port, mask;
	u32 laninfo = 0;
	laninfo = (portinfo & RTK_MAX_PORT_MASK) | LAN_PORT_EXT_MASK; // add CPU port to portPvid set

	ENUM_PORT_BEGIN(port, mask, laninfo, 1)
		ret_t = rtk_vlan_portPvid_set(port, vlan_vid, vlan_prio);
		if(ret_t != RT_ERR_OK)
			printk("%s() ERROR port:%d errono:%d\n", __func__, port, ret_t);
	ENUM_PORT_END
}

/* portInfo:	bit0-bit15 : member mask
		bit16-bit31 :  untag mask */
void createVlan_nopvid(u32 portinfo)
{
	rtk_int32 ret_t;
	rtk_vlan_cfg_t vlan_t;

	memset(&vlan_t, 0x00, sizeof(rtk_vlan_cfg_t));
	vlan_t.mbr.bits[0] = (portinfo & RTK_MAX_PORT_MASK) | LAN_PORT_EXT_MASK; // add CPU port to member
	vlan_t.untag.bits[0] = portinfo >> 16; // CPU port leave tag
	if (!vlan_vid) {
		vlan_t.untag.bits[0] |= LAN_PORT_EXT_MASK;
		vlan_t.ivl_en = 1;
	}
	printk("%s : vid = %d, prio = %d, mbrmsk = 0x%X, untagmsk = 0x%X\n", __func__, vlan_vid, vlan_prio, vlan_t.mbr.bits[0], vlan_t.untag.bits[0]);	
	ret_t = rtk_vlan_set(vlan_vid, &vlan_t);
	if(ret_t != RT_ERR_OK)
		printk("%s() ERROR vid:%d errono:%d\n", __func__, vlan_vid, ret_t);
}

void createVlan_nopvid_mbr_untag_cpu(u32 portinfo)
{
	rtk_int32 ret_t;
	rtk_vlan_cfg_t vlan_t;

	memset(&vlan_t, 0x00, sizeof(rtk_vlan_cfg_t));
	vlan_t.mbr.bits[0] = (portinfo & RTK_MAX_PORT_MASK) | LAN_PORT_EXT_MASK; // add cpu ports
	vlan_t.untag.bits[0] = portinfo >> 16; // CPU port leave tag
	vlan_t.untag.bits[0] |= LAN_PORT_EXT_MASK;
	if (!vlan_vid)
		vlan_t.ivl_en = 1;
	printk("%s : vid = %d, prio = %d, mbrmsk = 0x%X, untagmsk = 0x%X\n", __func__, vlan_vid, vlan_prio, vlan_t.mbr.bits[0], vlan_t.untag.bits[0]);	
	ret_t = rtk_vlan_set(vlan_vid, &vlan_t);
	if(ret_t != RT_ERR_OK)
		printk("%s() ERROR vid:%d errono:%d\n", __func__, vlan_vid, ret_t);
}

void createVlan_nopvid_nocpu(u32 portinfo)
{
	rtk_int32 ret_t;
	rtk_vlan_cfg_t vlan_t;

	memset(&vlan_t, 0x00, sizeof(rtk_vlan_cfg_t));
	vlan_t.mbr.bits[0] = (portinfo & RTK_MAX_PORT_MASK);
	vlan_t.untag.bits[0] = portinfo >> 16; // CPU port leave tag
	if (!vlan_vid)
		vlan_t.ivl_en = 1;
	printk("%s : vid = %d, prio = %d, mbrmsk = 0x%X, untagmsk = 0x%X\n", __func__, vlan_vid, vlan_prio, vlan_t.mbr.bits[0], vlan_t.untag.bits[0]);	
	ret_t = rtk_vlan_set(vlan_vid, &vlan_t);
	if(ret_t != RT_ERR_OK)
		printk("%s() ERROR vid:%d errono:%d\n", __func__, vlan_vid, ret_t);
}

void svlan_init_setup(void)
{
	rtk_api_ret_t retVal = 0;
	rtk_vlan_cfg_t vlancfg;
	rtk_svlan_memberCfg_t svlanCfg;

	printk("%s: start\n", __func__);
	if((retVal = rtk_svlan_init()) != RT_ERR_OK)
	{        
		printk("%s, err %d\n", __func__, retVal);
		return;
	}
			
	//default is 0x88A8
	if ((retVal = rtk_svlan_tpidEntry_set(0x8100)) != RT_ERR_OK)
		return;
       
	if((retVal = rtk_svlan_servicePort_add(EXT_PORT0)) != RT_ERR_OK)
		return;      
       
	memset(&svlanCfg, 0x00, sizeof(rtk_svlan_memberCfg_t));
	svlanCfg.svid = UTP_PORT1_SVLAN; 
	RTK_PORTMASK_PORT_SET(svlanCfg.memberport, EXT_PORT0);
	RTK_PORTMASK_PORT_SET(svlanCfg.memberport, UTP_PORT1);
	RTK_PORTMASK_PORT_SET(svlanCfg.untagport, UTP_PORT1); 
	svlanCfg.fiden = 1;
	svlanCfg.fid = 0;
	svlanCfg.priority = 0;
	if((retVal = rtk_svlan_memberPortEntry_set(UTP_PORT1_SVLAN, &svlanCfg)) != RT_ERR_OK)
		return;

	memset(&svlanCfg, 0x00, sizeof(rtk_svlan_memberCfg_t));
	svlanCfg.svid = UTP_PORT2_SVLAN;
	RTK_PORTMASK_PORT_SET(svlanCfg.memberport, EXT_PORT0);
	RTK_PORTMASK_PORT_SET(svlanCfg.memberport, UTP_PORT2);   
	RTK_PORTMASK_PORT_SET(svlanCfg.untagport, UTP_PORT2);
	svlanCfg.fiden = 1;
	svlanCfg.fid = 1;
	svlanCfg.priority = 0;
	if((retVal = rtk_svlan_memberPortEntry_set(UTP_PORT2_SVLAN, &svlanCfg)) != RT_ERR_OK)
		return;
        
	memset(&svlanCfg, 0x00, sizeof(rtk_svlan_memberCfg_t));
	svlanCfg.svid = UTP_PORT3_SVLAN;  
	RTK_PORTMASK_PORT_SET(svlanCfg.memberport, EXT_PORT0);
	RTK_PORTMASK_PORT_SET(svlanCfg.memberport, UTP_PORT3);
	RTK_PORTMASK_PORT_SET(svlanCfg.untagport, UTP_PORT3); 
	svlanCfg.fiden = 1;
	svlanCfg.fid = 2;
	svlanCfg.priority = 0;
	if((retVal = rtk_svlan_memberPortEntry_set(UTP_PORT3_SVLAN, &svlanCfg)) != RT_ERR_OK)
		return;

	memset(&svlanCfg, 0x00, sizeof(rtk_svlan_memberCfg_t));
	svlanCfg.svid = UTP_PORT4_SVLAN;
	RTK_PORTMASK_PORT_SET(svlanCfg.memberport, EXT_PORT0);
	RTK_PORTMASK_PORT_SET(svlanCfg.memberport, UTP_PORT4);   
	RTK_PORTMASK_PORT_SET(svlanCfg.untagport, UTP_PORT4);
	svlanCfg.fiden = 1;
	svlanCfg.fid = 3;
	svlanCfg.priority = 0;
	if((retVal = rtk_svlan_memberPortEntry_set(UTP_PORT4_SVLAN, &svlanCfg)) != RT_ERR_OK)
		return;
    
	if((retVal = rtk_svlan_untag_action_set(UNTAG_DROP, 0)) != RT_ERR_OK)
		return;
        
	if((retVal = rtk_svlan_defaultSvlan_set(UTP_PORT1, UTP_PORT1_SVLAN)) != RT_ERR_OK)
		return;
	if((retVal = rtk_svlan_defaultSvlan_set(UTP_PORT2, UTP_PORT2_SVLAN)) != RT_ERR_OK)
		return;
	if((retVal = rtk_svlan_defaultSvlan_set(UTP_PORT3, UTP_PORT3_SVLAN)) != RT_ERR_OK)
		return;
	if((retVal = rtk_svlan_defaultSvlan_set(UTP_PORT4, UTP_PORT4_SVLAN)) != RT_ERR_OK)
		return;

	if((retVal = rtk_vlan_portFid_set(UTP_PORT1, ENABLED, 0)) != RT_ERR_OK)
		return;
	if((retVal = rtk_vlan_portFid_set(UTP_PORT2, ENABLED, 1)) != RT_ERR_OK)
		return;
	if((retVal = rtk_vlan_portFid_set(UTP_PORT3, ENABLED, 2)) != RT_ERR_OK)
		return;
	if((retVal = rtk_vlan_portFid_set(UTP_PORT4, ENABLED, 3)) != RT_ERR_OK)
		return;
		
	if((retVal = rtk_vlan_init()) != RT_ERR_OK)
		return;			

	//Set VID 1 for all port default PVID
	memset(&vlancfg, 0x00, sizeof(rtk_vlan_cfg_t));   
	RTK_PORTMASK_ALLPORT_SET(vlancfg.mbr);
	RTK_PORTMASK_ALLPORT_SET(vlancfg.untag);
	vlancfg.ivl_en = 1;
	retVal = rtk_vlan_set(1, &vlancfg);

	if((retVal = rtk_vlan_portPvid_set(UTP_PORT1,1,0)) != RT_ERR_OK)
		return;				   
	if((retVal = rtk_vlan_portPvid_set(UTP_PORT2,1,0)) != RT_ERR_OK)
		return;
	if((retVal = rtk_vlan_portPvid_set(UTP_PORT3,1,0)) != RT_ERR_OK)
		return;	
	if((retVal = rtk_vlan_portPvid_set(UTP_PORT4,1,0)) != RT_ERR_OK)
		return;	

	printk("\n%s: end\n", __func__);
	return;	
}

void createVlan_tagged_byports(u32 portinfo)
{
	rtk_api_ret_t retVal;
	rtk_vlan_cfg_t vlancfg;
        unsigned int port, mask;

	memset(&vlancfg, 0x00, sizeof(rtk_vlan_cfg_t));

	/* Member ports: user-selected ports + CPU/EXT port */
	RTK_PORTMASK_CLEAR(vlancfg.mbr);
	vlancfg.mbr.bits[0] = (portinfo & RTK_MAX_PORT_MASK) | LAN_PORT_EXT_MASK;

	/* Egress tagging: all tagged (no untag ports) */
	RTK_PORTMASK_CLEAR(vlancfg.untag);

	/* Enable IVL (same as sample) */
	vlancfg.ivl_en = 1;

	/* Use global vlan_vid as VLAN ID */
	retVal = rtk_vlan_set(vlan_vid, &vlancfg);
	if (retVal != RT_ERR_OK)
		printk("%s() ERROR vid:%u errono:%d\n", __func__, vlan_vid, retVal);

        ENUM_PORT_BEGIN(port, mask, portinfo, 1)
                retVal = rtk_vlan_portAcceptFrameType_set(port, ACCEPT_FRAME_TYPE_ALL);
                if(retVal != RT_ERR_OK)
                        printk("%s() ERROR port:%d errono:%d\n", __func__, port, retVal);
        ENUM_PORT_END
} 

void svlan_sample_test(void)
{
	svlan_init_setup();

	vlan_vid = 10;
	createVlan_tagged_byports(0x8);
	vlan_vid = 20;
	createVlan_tagged_byports(0x8);
}

void resetVlan(u32 portinfo)
{
	rtk_int32 ret_t;
	rtk_vlan_cfg_t vlan_t;

	memset(&vlan_t, 0x00, sizeof(rtk_vlan_cfg_t));
	vlan_t.mbr.bits[0] = 0;
	vlan_t.untag.bits[0] = 0;

	ret_t = rtk_vlan_set(vlan_vid, &vlan_t);
	if(ret_t != RT_ERR_OK)
		printk("%s() ERROR vid:%d errono:%d\n", __func__, vlan_vid, ret_t);
}

/* portInfo:	bit0-bit3 : port(port_t)
		bit4-bit7 : type(rtk_vlan_acceptFrameType_t)  */
void set_vlan_portType(u32 portinfo)
{
	rtk_int32 ret_t;
	unsigned int port;
	rtk_vlan_acceptFrameType_t vf_type;

	port = portinfo & 0xf;
	vf_type = (portinfo >> 4) & 0xf;

	printk("%s - port-%d: vf_type = %d.\n", __func__, port, vf_type);	

	ret_t = rtk_vlan_portAcceptFrameType_set(port, vf_type);
	if(ret_t != RT_ERR_OK)
		printk("%s() ERROR port:%d errono:%d\n", __func__, port, ret_t);
}

/* portInfo:	bit0-bit3 : port(port_t)
		bit4-bit7 : prio(pri_t)  */
void set_portPvid(u32 portinfo)
{
	rtk_int32 ret_t;
	unsigned int port, priority;

	port = portinfo & 0xf;
	priority = (portinfo >> 4) & 0xf;

	printk("%s - port-%d: vid = %d prio = %d.\n", __func__, port, vlan_vid, priority);	

	ret_t = rtk_vlan_portPvid_set(port, vlan_vid, priority);
	if(ret_t != RT_ERR_OK)
		printk("%s() ERROR port:%d errono:%d\n", __func__, port, ret_t);
}

/* set pvid by ports mask */
void set_ports_pvid(u32 portMask)
{
	rtk_int32 ret_t;
	unsigned int port, priority=0, mask;

	printk("%s:%08x\n", __func__, portMask);
	ENUM_PORT_BEGIN(port, mask, portMask, 1)
		ret_t = rtk_vlan_portPvid_set(port, vlan_vid, priority);
		if(ret_t != RT_ERR_OK)
			printk("%s() ERROR port:%d errono:%d\n", __func__, port, ret_t);
	ENUM_PORT_END
}

static void disable_jumbo_frame(void)
{
	rtk_api_ret_t retVal;

	retVal = rtk_switch_maxPktLenCfg_set(0, 1532);
	printk("rtk_switch_maxPktLenCfg_set(): return %d\n", retVal);
}

static void enable_jumbo_frame(void)
{
	rtk_api_ret_t retVal;

	retVal = rtk_switch_maxPktLenCfg_set(0, RTK_SWITCH_MAX_PKTLEN);
	printk("rtk_switch_maxPktLenCfg_set(): return %d\n", retVal);
}

static void vlan_force_ivl_for_vid(rtk_vlan_t vid)
{
	rtk_api_ret_t ret;
	rtk_vlan_cfg_t vlan_t;

	memset(&vlan_t, 0, sizeof(vlan_t));
	ret = rtk_vlan_get(vid, &vlan_t);
	if (ret != RT_ERR_OK) {
		printk("%s: rtk_vlan_get(vid=%u) err=%d\n", __func__, vid, ret);
		return;
	}

	if (vlan_t.ivl_en == 1) {
		printk("%s: vid=%u already ivl_en=1\n", __func__, vid);
		return;
	}

	vlan_t.ivl_en = 1;
	ret = rtk_vlan_set(vid, &vlan_t);
	printk("%s: set vid=%u ivl_en=1 ret=%d (mbr=0x%X untag=0x%X)\n",
		__func__, vid, ret, vlan_t.mbr.bits[0], vlan_t.untag.bits[0]);
}

static void dump_l2_table_all(void)
{
	rtk_api_ret_t ret;
	rtk_uint32 address = 0;
	rtk_l2_ucastAddr_t l2;
	int printed = 0;
	const int max_print = 4096;

	printk("=== rtkswitch: dump L2 UC table begin ===\n");

	memset(&l2, 0, sizeof(l2));

	while (printed < max_print) {
		ret = rtk_l2_addr_next_get(READMETHOD_NEXT_L2UC, 0 /*dummy*/, &address, &l2);
		if (ret != RT_ERR_OK)
			break;

		printk("L2UC addr=%u mac=%02X:%02X:%02X:%02X:%02X:%02X "
		"port=%u ivl=%u cvid=%u fid=%u efid=%u static=%u "
		"sa_block=%u da_block=%u auth=%u age? n/a(prv)\n",
		address,
		l2.mac.octet[0], l2.mac.octet[1], l2.mac.octet[2],
		l2.mac.octet[3], l2.mac.octet[4], l2.mac.octet[5],
		l2.port,
		l2.ivl, l2.cvid, l2.fid, l2.efid, l2.is_static,
		l2.sa_block, l2.da_block, l2.auth);

		address++;   /* per SDK comment: next call uses address+1 */
		printed++;
	}
	printk("=== rtkswitch: dump L2 UC table end (ret=%d, printed=%d) ===\n",
		ret, printed);
}

static int rtl8367s_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int rtl8367s_release(struct inode *inode, struct file *filp)
{
	return 0;
}

int rtk_ext_swctl_init(void);

#define RTKSWITCH_IOC_MAGIC	'R'

static int rtl8367s_ioctl(struct inode *inode, struct file *flip, unsigned int command, unsigned long arg)
{
	rtk_api_ret_t retVal;
	rtk_port_t port, port_adj;
	rtk_port_linkStatus_t LinkStatus;
	rtk_data_t Speed;
	rtk_data_t Duplex;
	phyState pS;
	int port_user;
	rtk_stat_port_cntr_t Port_cntrs;
	rtk_port_phy_data_t pData;
	unsigned int mask;
	mactable Port_mactable;
	rtk_uint32 address = 0;
	int count = 0;
	rtk_l2_ucastAddr_t l2_data;
	int wan_stb_x = 0;
	u32 portInfo;
	unsigned int pvid=0, priv=0;
	rtk_vlan_cfg_t vlan_t;
	rtk_int32 ret_t;
	rtk_uint32 select, enabled, size;
#if 0
	rtk_uint32 efid;
#endif
	rtk_portmask_t permitPortmask, portmask;
#if 0
	rtk_led_operation_t led_mode;
	rtk_led_blink_rate_t blinkRate;
	rtk_led_congig_t led_config;
	rtk_led_force_mode_t led_force_mode;
	int gpio;
#endif
	rtk_port_phy_test_mode_t test_mode;
	rtk_port_mac_ability_t Portstatus;
	rtk_uint32 regData;
	int i;
	rtk_mac_cnt_t mac_cnt;
	unsigned int cmd_nr;

	if (_IOC_TYPE(command) != RTKSWITCH_IOC_MAGIC)
		return -ENOTTY;

	/* User space encodes the command number's low 8 bits into NR and the high
	 * bits into SIZE; rebuild the full number to support n > 255 (case 771 / 3905 ...). */
	cmd_nr = _IOC_NR(command) | (_IOC_SIZE(command) << 8);
	switch (cmd_nr) {
	case 0:	// Get LAN phy status of all LAN ports
		retVal = raw_copy_from_user(&pS, (phyState __user *)arg, sizeof(pS));

		ENUM_PORT_BEGIN(port, mask, LAN_PORTS_MASK, 1)
			retVal = rtk_port_phyStatus_get(port, &LinkStatus, &Speed, &Duplex);
			if (port == LAN_PORT_1)
				port_adj = 0;
			else if (port == LAN_PORT_2)
				port_adj = 1;
			else if (port == LAN_PORT_3)
				port_adj = 2;
			else if (port == LAN_PORT_4)
				port_adj = 3;
			pS.link[port_adj] = LinkStatus;
			pS.speed[port_adj] = Speed;
			pS.duplex[port_adj] = Duplex;
		ENUM_PORT_END

		retVal = raw_copy_to_user((phyState __user *)arg, &pS, sizeof(pS));

		break;
	case 1:	// Dump all counters of the specified LAN port
		retVal = raw_copy_from_user(&port_user, (int __user *)arg, sizeof(int));
//		printk("rtk_stat_port_getAll(%d)\n", port_user);

		if (port_user == 0)
			port = LAN_PORT_1;
		else if (port_user == 1)
			port = LAN_PORT_2;
		else if (port_user == 2)
			port = LAN_PORT_3;
		else if (port_user == 3)
			port = LAN_PORT_4;
		else if (port_user == 4)
			port = EXT_PORT0;
		else
			return 0;

		retVal = rtk_stat_port_getAll(port, &Port_cntrs);

		if (retVal == RT_ERR_OK)
			retVal = raw_copy_to_user((rtk_stat_port_cntr_t __user *)arg, &Port_cntrs, sizeof(rtk_stat_port_cntr_t));
		else {
			printk("rtk_stat_port_getAll(%d) return %d\n", port, retVal);
			return retVal;
		}

		break;
	case 2: // Dump MAC status of specified LAN port
		retVal = raw_copy_from_user(&port_user, (int __user *)arg, sizeof(int));

		if (port_user == 0)
			port = LAN_PORT_1;
		else if (port_user == 1)
			port = LAN_PORT_2;
		else if (port_user == 2)
			port = LAN_PORT_3;
		else if (port_user == 3)
			port = LAN_PORT_4;
		else if (port_user == 4)
			port = EXT_PORT0;

		if ((port_user >= 0) && (port_user <= 4)) {
			printk("rtk_port_macStatus_get(%d)\n", port);
			retVal = rtk_port_macStatus_get(port, &Portstatus);
			if (retVal == RT_ERR_OK) {
				printk("duplex: %d\n", Portstatus.duplex);
				printk("link: %d\n", Portstatus.link);
				printk("nway: %d\n", Portstatus.nway);
				printk("txpause: %d\n", Portstatus.txpause);
				printk("rxpause: %d\n", Portstatus.rxpause);
			} else {
				printk("rtk_port_macStatus_get(%d) return %d\n", port, retVal);
				return retVal;
			}
		}

		break;
#if 0
	case 100: // set reset pin to 0
		retVal = raw_copy_from_user(&gpio, (int __user *)arg, sizeof(int));
		printk("reset pin[%d]: set to 0\n", gpio);

		kerSysSetGpioState(gpio, 0);

		break;
	case 101: // set reset pin to 1
		retVal = raw_copy_from_user(&gpio, (int __user *)arg, sizeof(int));
		printk("reset pin[%d]: set to 1\n", gpio);

		kerSysSetGpioState(gpio, 1);

		break;
#endif		
	case 3:	// Get link status of the specified LAN port
		retVal = raw_copy_from_user(&port_user, (int __user *)arg, sizeof(int));

		if (port_user == 0)
			port_user = LAN_PORT_1;
		else if (port_user == 1)
			port_user = LAN_PORT_2;
		else if (port_user == 2)
			port_user = LAN_PORT_3;
		else if (port_user == 3)
			port_user = LAN_PORT_4;
		else if (port_user == 4)
			port_user = LAN_PORT_EXT;

		retVal = rtk_port_phyStatus_get(port_user, &LinkStatus, &Speed, &Duplex);
		if (retVal == RT_ERR_OK)
			port_user = LinkStatus;
		else
			port_user = 0;

		retVal = raw_copy_to_user((int __user *)arg, &port_user, sizeof(int));

		break;
	case 31: // Get link status of the specified PHY port directly
		retVal = raw_copy_from_user(&port_user, (int __user *)arg, sizeof(int));

		retVal = rtk_port_phyStatus_get(port_user, &LinkStatus, &Speed, &Duplex);
		
		if (retVal == RT_ERR_OK)
			port_user = LinkStatus;
		else
			port_user = 0;

		retVal = raw_copy_to_user((int __user *)arg, &port_user, sizeof(int));

		break;
	case 4:	// Get link status of LAN ports
		memset(&pS, 0, sizeof(pS));
		for (port = 0; port < 4; port++)
		{
#if defined(RTBE58U) || defined(TUFBE3600) || defined(RTBE58U_V2) || defined(TUFBE3600_V2) || defined(RTBE55)
			retVal = rtk_port_phyStatus_get(4 - port, &LinkStatus, &Speed, &Duplex);
#else
			retVal = rtk_port_phyStatus_get(port, &LinkStatus, &Speed, &Duplex);
#endif
			pS.link[port] = LinkStatus;
			pS.speed[port] = Speed;
			pS.duplex[port] = Duplex;
		}

		port_user = 0;
		if (pS.link[0] || pS.link[1] || pS.link[2] || pS.link[3])
			port_user = 1;

		retVal = raw_copy_to_user((int __user *)arg, &port_user, sizeof(int));

		break;
	case 5:	// power up LAN ports
		printk("power up LAN ports\n");
		ENUM_PORT_BEGIN(port, mask, LAN_PORTS_MASK, 1)
			rtk_port_phyReg_get(port, PHY_CONTROL_REG, &pData);
			pData &= ~CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(port, PHY_CONTROL_REG, pData);
		ENUM_PORT_END

		break;
	case 6:	// power down LAN ports
		printk("power down LAN ports\n");
		ENUM_PORT_BEGIN(port, mask, LAN_PORTS_MASK, 1)
			rtk_port_phyReg_get(port, PHY_CONTROL_REG, &pData);
			pData |= CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(port, PHY_CONTROL_REG, pData);
		ENUM_PORT_END

		break;
	case 7: // dump L2 lookup table of specified LAN port
		retVal = raw_copy_from_user(&port_user, (int __user *)arg, sizeof(int));

		if (port_user == 0)
			port = LAN_PORT_1;
		else if (port_user == 1)
			port = LAN_PORT_2;
		else if (port_user == 2)
			port = LAN_PORT_3;
		else if (port_user == 3)
			port = LAN_PORT_4;

		memset(&Port_mactable, 0, sizeof(mactable));

		if ((port_user >= 0) && (port_user <= 3))
		while (1) {
			if ((retVal = rtk_l2_addr_next_get(READMETHOD_NEXT_L2UC, port, &address, &l2_data)) != RT_ERR_OK)
				break;

			if (count > 255)
				break;

			if (port == l2_data.port) {
#if 0
				printk("%02X:%02X:%02X:%02X:%02X:%02X\n", l2_data.mac.octet[0], l2_data.mac.octet[1], l2_data.mac.octet[2], l2_data.mac.octet[3], l2_data.mac.octet[4], l2_data.mac.octet[5]);
#endif
				memcpy(Port_mactable.ea[count++].octet, l2_data.mac.octet, ETHER_ADDR_LEN);
			}

			address++;
		}

		Port_mactable.count = count;
		retVal = raw_copy_to_user((mactable __user *)arg, &Port_mactable, sizeof(mactable));

		break;
	case 77: /* dump full L2 table for all ports (diagnose SVL/IVL MAC learning) */
		dump_l2_table_all();
		break;

	case 771: /* Find the utp lan port of target mac */
		typedef struct {
			unsigned char mac[6];
			int port;
		} mac_query_t;
		mac_query_t q;
		
		if (raw_copy_from_user(&q, (mac_query_t __user *)arg, sizeof(q)))
			return -EFAULT;

		q.port = -1; 
		address = 0;
		memset(&l2_data, 0, sizeof(l2_data));

		while (1) {
			retVal = rtk_l2_addr_next_get(READMETHOD_NEXT_L2UC, 0 /*dummy*/, &address, &l2_data);
			if (retVal != RT_ERR_OK)
				break;

			if (!memcmp(l2_data.mac.octet, q.mac, ETHER_ADDR_LEN)) {
				
				if (l2_data.port != LAN_PORT_EXT) {
					q.port = l2_data.port;
					
					printk("rtkswitch: target_mac %02X:%02X:%02X:%02X:%02X:%02X : target_port %d\n",
						q.mac[0], q.mac[1], q.mac[2], 
						q.mac[3], q.mac[4], q.mac[5], q.port);
						
					break;
				}
			}
			address++;
		}

		if (raw_copy_to_user((mac_query_t __user *)arg, &q, sizeof(q)))
			return -EFAULT;

		break;
	case 8: // reset per port MIB counter
		printk("reset per port MIB counter\n");
		ENUM_PORT_BEGIN(port, mask, LAN_PORTS_MASK, 1)
			rtk_stat_port_reset(port);
		ENUM_PORT_END

		break;
	case 9: // LAN/WAN partition
		retVal = raw_copy_from_user(&wan_stb_x, (int __user *)arg, sizeof(int));

		if (wan_stb_x == 0)
		{
			printk("LAN: P0,P1,P2,P3 WAN: N/A\n");
		}
		else if (wan_stb_x == 1)
		{
			printk("LAN: P1,P2,P3 WAN: P0\n");
		}
		else if (wan_stb_x == 2)
		{
			printk("LAN: P0,P2,P3 WAN: P1\n");
		}
		else if (wan_stb_x == 3)
		{
			printk("LAN: P0,P1,P3 WAN: P2\n");
		}
		else if (wan_stb_x == 4)
		{
			printk("LAN: P0,P1,P2 WAN: P3\n");
		}
		else if (wan_stb_x == 5)
		{
			printk("LAN: P2,P3 WAN: P0,P1\n");
		}
		else if (wan_stb_x == 6)
		{
			printk("LAN: P0,P1 WAN: P2,P3\n");
		}

		wan_stb_g = wan_stb_x;
		LANWANPartition_adv(wan_stb_x);

		break;
	case 10: // power up LAN1
		printk("power up LAN1\n");
		rtk_port_phyReg_get(LAN_PORT_1, PHY_CONTROL_REG, &pData);
		pData &= ~CONTROL_REG_PORT_POWER_BIT;
		rtk_port_phyReg_set(LAN_PORT_1, PHY_CONTROL_REG, pData);

		break;
	case 11: // power down LAN1
		printk("power down LAN1\n");
		rtk_port_phyReg_get(LAN_PORT_1, PHY_CONTROL_REG, &pData);
		pData |= CONTROL_REG_PORT_POWER_BIT;
		rtk_port_phyReg_set(LAN_PORT_1, PHY_CONTROL_REG, pData);

		break;
	case 12:// power up specified LAN port
		retVal = raw_copy_from_user(&port_user, (int __user *)arg, sizeof(int));
		if (port_user == 0)
			port = LAN_PORT_1;
		else if (port_user == 1)
			port = LAN_PORT_2;
		else if (port_user == 2)
			port = LAN_PORT_3;
		else if (port_user == 3)
			port = LAN_PORT_4;
		if ((port_user >= 0) && (port_user <= 3)) {
			printk("power up LAN%d\n", port_user + 1);
			rtk_port_phyReg_get(port, PHY_CONTROL_REG, &pData);
			pData &= ~CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(port, PHY_CONTROL_REG, pData);
		}

		break;
	case 13:// power down specified LAN port
		retVal = raw_copy_from_user(&port_user, (int __user *)arg, sizeof(int));
		if (port_user == 0)
			port = LAN_PORT_1;
		else if (port_user == 1)
			port = LAN_PORT_2;
		else if (port_user == 2)
			port = LAN_PORT_3;
		else if (port_user == 3)
			port = LAN_PORT_4;

		if ((port_user >= 0) && (port_user <= 3)) {
			printk("power down LAN%d\n", port_user + 1);
			rtk_port_phyReg_get(port, PHY_CONTROL_REG, &pData);
			pData |= CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(port, PHY_CONTROL_REG, pData);
		}

		break;
	case 14:// tweak flow control behavior
		printk("tweak flow control behavior\n");
		rtl8367c_setAsicReg(0x121F, 0x29E);
		rtl8367c_setAsicReg(0x1223, 0x29E);
		rtl8367c_setAsicReg(0x1220, 0x280);
		rtl8367c_setAsicReg(0x1224, 0x280);
		rtl8367c_setAsicReg(0x1221, 0x294);
		rtl8367c_setAsicReg(0x1225, 0x294);
		rtl8367c_setAsicReg(0x1222, 0x276);
		rtl8367c_setAsicReg(0x1226, 0x276);

		break;
	case 15:// reset flow control behavior
		printk("reset flow control behavior\n");
		rtl8367c_setAsicReg(0x121F, 0x1D6);
		rtl8367c_setAsicReg(0x1223, 0x1D6);
		rtl8367c_setAsicReg(0x1220, 0x1B8);
		rtl8367c_setAsicReg(0x1224, 0x1B8);
		rtl8367c_setAsicReg(0x1221, 0x1CC);
		rtl8367c_setAsicReg(0x1225, 0x1CC);
		rtl8367c_setAsicReg(0x1222, 0x1AE);
		rtl8367c_setAsicReg(0x1226, 0x1AE);

		break;
	case 16:// dump buffer status related registers
		if ((retVal = rtl8367c_getAsicReg(0x124C, &regData)) != RT_ERR_OK)
			printk("err rtl8367c_getAsicReg 0x124C\n");
		else
			printk("0x124C: %x\n", regData);
		if ((retVal = rtl8367c_getAsicReg(0x124D, &regData)) != RT_ERR_OK)
			printk("err rtl8367c_getAsicReg 0x124D\n");
		else
			printk("0x124D: %x\n", regData);
		if ((retVal = rtl8367c_getAsicReg(0x124E, &regData)) != RT_ERR_OK)
			printk("err rtl8367c_getAsicReg 0x124E\n");
		else
			printk("0x124E: %x\n", regData);
		if ((retVal = rtl8367c_getAsicReg(0x124F, &regData)) != RT_ERR_OK)
			printk("err rtl8367c_getAsicReg 0x124F\n");
		else
			printk("0x124F: %x\n", regData);
		if ((retVal = rtl8367c_getAsicReg(0x1250, &regData)) != RT_ERR_OK)
			printk("err rtl8367c_getAsicReg 0x1250\n");
		else
			printk("0x1250: %x\n", regData);
		if ((retVal = rtl8367c_getAsicReg(0x1251, &regData)) != RT_ERR_OK)
			printk("err rtl8367c_getAsicReg 0x1251\n");
		else
			printk("0x1251: %x\n", regData);
		if ((retVal = rtl8367c_getAsicReg(0x1252, &regData)) != RT_ERR_OK)
			printk("err rtl8367c_getAsicReg 0x1252\n");
		else
			printk("0x1252: %x\n", regData);
		if ((retVal = rtl8367c_getAsicReg(0x1253, &regData)) != RT_ERR_OK)
			printk("err rtl8367c_getAsicReg 0x1253\n");
		else
			printk("0x1253: %x\n", regData);
		if ((retVal = rtl8367c_getAsicReg(0x1254, &regData)) != RT_ERR_OK)
			printk("err rtl8367c_getAsicReg 0x1254\n");
		else
			printk("0x1254: %x\n", regData);
		if ((retVal = rtl8367c_getAsicReg(0x1255, &regData)) != RT_ERR_OK)
			printk("err rtl8367c_getAsicReg 0x1255\n");
		else
			printk("0x1255: %x\n", regData);
		if ((retVal = rtl8367c_getAsicReg(0x1256, &regData)) != RT_ERR_OK)
			printk("err rtl8367c_getAsicReg 0x1256\n");
		else
			printk("0x1256: %x\n", regData);
		if ((retVal = rtl8367c_getAsicReg(0x1257, &regData)) != RT_ERR_OK)
			printk("err rtl8367c_getAsicReg 0x1257\n");
		else
			printk("0x1257: %x\n", regData);
		if ((retVal = rtl8367c_getAsicReg(0x1260, &regData)) != RT_ERR_OK)
			printk("err rtl8367c_getAsicReg 0x1260\n");
		else
			printk("0x1260: %x\n", regData);
		if ((retVal = rtl8367c_getAsicReg(0x1261, &regData)) != RT_ERR_OK)
			printk("err rtl8367c_getAsicReg 0x1261\n");
		else
			printk("0x1261: %x\n", regData);
		if ((retVal = rtl8367c_getAsicReg(0x1262, &regData)) != RT_ERR_OK)
			printk("err rtl8367c_getAsicReg 0x1262\n");
		else
			printk("0x1262: %x\n", regData);
		if ((retVal = rtl8367c_getAsicReg(0x1263, &regData)) != RT_ERR_OK)
			printk("err rtl8367c_getAsicReg 0x1263\n");
		else
			printk("0x1263: %x\n", regData);
		if ((retVal = rtl8367c_getAsicReg(0x1264, &regData)) != RT_ERR_OK)
			printk("err rtl8367c_getAsicReg 0x1264\n");
		else
			printk("0x1264: %x\n", regData);
		if ((retVal = rtl8367c_getAsicReg(0x1265, &regData)) != RT_ERR_OK)
			printk("err rtl8367c_getAsicReg 0x1265\n");
		else
			printk("0x1265: %x\n", regData);
		if ((retVal = rtl8367c_getAsicReg(0x1266, &regData)) != RT_ERR_OK)
			printk("err rtl8367c_getAsicReg 0x1266\n");
		else
			printk("0x1266: %x\n", regData);
		if ((retVal = rtl8367c_getAsicReg(0x1267, &regData)) != RT_ERR_OK)
			printk("err rtl8367c_getAsicReg 0x1267\n");
		else
			printk("0x1267: %x\n", regData);

		for (port = 0; port < 8; port++) {
			if ((retVal = rtl8367c_setAsicReg(0x22E, port)) != RT_ERR_OK)
				printk("err rtl8367c_setAsicReg 0x22E as %d\n", port);
			else {
				printk("rtl8367c_getAsicReg set 0x22E as %d\n", port);
				if ((retVal = rtl8367c_getAsicReg(0x1230, &regData)) != RT_ERR_OK)
					printk("err rtl8367c_getAsicReg 0x1230\n");
				else
					printk("0x1230: %x\n", regData);
				if ((retVal = rtl8367c_getAsicReg(0x1231, &regData)) != RT_ERR_OK)
					printk("err rtl8367c_getAsicReg 0x1231\n");
				else
					printk("0x1231: %x\n", regData);
				if ((retVal = rtl8367c_getAsicReg(0x1232, &regData)) != RT_ERR_OK)
					printk("err rtl8367c_getAsicReg 0x1232\n");
				else
					printk("0x1232: %x\n", regData);
				if ((retVal = rtl8367c_getAsicReg(0x1233, &regData)) != RT_ERR_OK)
					printk("err rtl8367c_getAsicReg 0x1233\n");
				else
					printk("0x1233: %x\n", regData);
				if ((retVal = rtl8367c_getAsicReg(0x1234, &regData)) != RT_ERR_OK)
					printk("err rtl8367c_getAsicReg 0x1234\n");
				else
					printk("0x1234: %x\n", regData);
				if ((retVal = rtl8367c_getAsicReg(0x1235, &regData)) != RT_ERR_OK)
					printk("err rtl8367c_getAsicReg 0x1235\n");
				else
					printk("0x1235: %x\n", regData);
				if ((retVal = rtl8367c_getAsicReg(0x1236, &regData)) != RT_ERR_OK)
					printk("err rtl8367c_getAsicReg 0x1236\n");
				else
					printk("0x1236: %x\n", regData);
				if ((retVal = rtl8367c_getAsicReg(0x1237, &regData)) != RT_ERR_OK)
					printk("err rtl8367c_getAsicReg 0x1237\n");
				else
					printk("0x1237: %x\n", regData);
				if ((retVal = rtl8367c_getAsicReg(0x1238, &regData)) != RT_ERR_OK)
					printk("err rtl8367c_getAsicReg 0x1238\n");
				else
					printk("0x1238: %x\n", regData);
				if ((retVal = rtl8367c_getAsicReg(0x1239, &regData)) != RT_ERR_OK)
					printk("err rtl8367c_getAsicReg 0x1239\n");
				else
					printk("0x1239: %x\n", regData);
				if ((retVal = rtl8367c_getAsicReg(0x123a, &regData)) != RT_ERR_OK)
					printk("err rtl8367c_getAsicReg 0x123a\n");
				else
					printk("0x123a: %x\n", regData);
				if ((retVal = rtl8367c_getAsicReg(0x123b, &regData)) != RT_ERR_OK)
					printk("err rtl8367c_getAsicReg 0x123b\n");
				else
					printk("0x123b: %x\n", regData);
				if ((retVal = rtl8367c_getAsicReg(0x123c, &regData)) != RT_ERR_OK)
					printk("err rtl8367c_getAsicReg 0x123c\n");
				else
					printk("0x123c: %x\n", regData);
				if ((retVal = rtl8367c_getAsicReg(0x123d, &regData)) != RT_ERR_OK)
					printk("err rtl8367c_getAsicReg 0x123d\n");
				else
					printk("0x123d: %x\n", regData);
				if ((retVal = rtl8367c_getAsicReg(0x123e, &regData)) != RT_ERR_OK)
					printk("err rtl8367c_getAsicReg 0x123e\n");
				else
					printk("0x123e: %x\n", regData);
				if ((retVal = rtl8367c_getAsicReg(0x123f, &regData)) != RT_ERR_OK)
					printk("err rtl8367c_getAsicReg 0x123f\n");
				else
					printk("0x123f: %x\n", regData);
				if ((retVal = rtl8367c_getAsicReg(0x1240, &regData)) != RT_ERR_OK)
					printk("err rtl8367c_getAsicReg 0x1240\n");
				else
					printk("0x1240: %x\n", regData);
				if ((retVal = rtl8367c_getAsicReg(0x1241, &regData)) != RT_ERR_OK)
					printk("err rtl8367c_getAsicReg 0x1241\n");
				else
					printk("0x1241: %x\n", regData);
			}
		}

		break;
	case 17:// disable jumbo frame
		disable_jumbo_frame();

		break;
	case 18:// enable jumbo frame
		enable_jumbo_frame();

		break;
	case 36:/* Set Vlan VID. */
		retVal = raw_copy_from_user(&vlan_vid, (int __user *)arg, sizeof(int));
		printk("chking vlan_vid:%d\n", vlan_vid);

		break;
	case 37:/* Set Vlan PRIO. */
		retVal = raw_copy_from_user(&vlan_prio, (int __user *)arg, sizeof(int));

		break;
	case 371:/* (re)Set vlan port PRIO. */
		retVal = raw_copy_from_user(&portInfo, (int __user *)arg, sizeof(int));
		set_prio((u32) portInfo);

		break;
	case 38:/* Initialize VLAN, set vlanFilter */
		retVal = raw_copy_from_user(&portInfo, (int __user *)arg, sizeof(int));
		initialVlan((u32) portInfo);
		vlan_accept_adv(wan_stb_x);

		break;
	case 380:/* set vlanFilter */
		retVal = raw_copy_from_user(&portInfo, (int __user *)arg, sizeof(int));
		setVlanFilter((u32) portInfo);

		break;
	case 381:/* get vlanFilter */
		retVal = raw_copy_from_user(&portInfo, (int __user *)arg, sizeof(int));
		getVlanFilter();

		break;
	case 382:/* reinit vlan */
		vlan_accept_none();

		break;
	case 39:/* Create VLAN. need to specify vlanid first, by 36 */
		retVal = raw_copy_from_user(&portInfo, (int __user *)arg, sizeof(int));
		createVlan((u32) portInfo);

		break;
	case 390:/* Create VLAN w/o setting pvid, need to specify vlanid first, by 36 */
		retVal = raw_copy_from_user(&portInfo, (int __user *)arg, sizeof(int));
		createVlan_nopvid((u32) portInfo);

		break;
	case 3900:/* Create VLAN w/o pvid, w/o cpu port, need to specify vlanid first, by 36 */
		retVal = raw_copy_from_user(&portInfo, (int __user *)arg, sizeof(int));
		createVlan_nopvid_nocpu((u32) portInfo);

		break;
	case 3901:/* reset VLAN. need to specify vlanid first, by 36 */
		retVal = raw_copy_from_user(&portInfo, (int __user *)arg, sizeof(int));
		resetVlan((u32) portInfo);

		break;
	case 3902:/* Create VLAN w/o setting pvid, w/ cpu ports in mbr/untag , need to specify vlanid first, by 36 */
		retVal = raw_copy_from_user(&portInfo, (int __user *)arg, sizeof(int));
		createVlan_nopvid_mbr_untag_cpu((u32) portInfo);

		break;
        case 3903:/* slvan setup example for bcm_vlan_extswitch w/ eth2 support vlan10,vlan20 and others non-tag */
                svlan_sample_test();

                break;
        case 3904:/* slvan basic setup for bcm_vlan_extswitch */
		svlan_init_setup();

                break;
	case 3905: /* Create vlan by portinfo, need to specify vlanid first, by 36 */
		retVal = raw_copy_from_user(&portInfo, (int __user *)arg, sizeof(int));
		createVlan_tagged_byports((u32) portInfo);

		break;
	case 391: /* Set port PVID. need to specify vlanid first, by 36 */
		retVal = raw_copy_from_user(&portInfo, (int __user *)arg, sizeof(int));
		set_portPvid((u32) portInfo);

		break;
	case 3911: /* Set ports PVID. need to specify vlanid first, by 36 */
		retVal = raw_copy_from_user(&portInfo, (int __user *)arg, sizeof(int));
		set_ports_pvid((u32) portInfo);

		break;
	case 392: /* Get port PVID */
		retVal = raw_copy_from_user(&port_user, (int __user *)arg, sizeof(int));
	
		rtk_vlan_portPvid_get(port_user, &pvid, &priv);
		printk("%s, p%d pvid=%d, priv=%d\n", __func__, port_user, pvid, priv);

		retVal = raw_copy_to_user((int __user *)arg, &port_user, sizeof(int));

		break;
	case 393: /* Get all ports PVID */

		ENUM_PORT_BEGIN(port, mask, LAN_ALL_PORTS_MASK, 1)
		rtk_vlan_portPvid_get(port, &pvid, &priv);
		printk("%s, p%d pvid=%d, priv=%d\n", __func__, port, pvid, priv);
		ENUM_PORT_END

		break;
	case 395:/* reset all ports accept type as all */
		vlan_accept_all();

		break;
	case 3951:/* reset ports accept type as all */
		retVal = raw_copy_from_user(&portInfo, (int __user *)arg, sizeof(int));
		vlan_accept_all_byports(portInfo);

		break;
	case 3952:/* reset ports accept type as tag-only */
		retVal = raw_copy_from_user(&portInfo, (int __user *)arg, sizeof(int));
		vlan_accept_tagOnly_byports(portInfo);

		break;
	case 3953:/* reset ports accept type as untag-only */
		retVal = raw_copy_from_user(&portInfo, (int __user *)arg, sizeof(int));
		vlan_accept_untagOnly_byports(portInfo);

		break;
	case 396:/* dump all ports accept type */
		vlan_dump_frameType_all();

		break;
	case 397:/* set port frame type */
		retVal = raw_copy_from_user(&portInfo, (int __user *)arg, sizeof(int));
		set_vlan_portType((u32) portInfo);

		break;
	//case 398:/* get port_isolation_mask/efid */
	//	get_port_isolation_mask_efid();

	//	break;
	case 399:/* get vlan-x's mbr_untags fields. need to specify vlanid first, by 36 */
        	ret_t = rtk_vlan_get(vlan_vid, &vlan_t);
        	if(ret_t != RT_ERR_OK)
                	printk("%s() ERROR vid:%d errono:%d\n", __func__, vlan_vid, ret_t);

		printk("get vlan mbr/untag - vid = %d, mbrmsk = 0x%X untagmsk = 0x%X\n", vlan_vid, vlan_t.mbr.bits[0], vlan_t.untag.bits[0]);

		break;
	case 40:/* set static is_singtel_mio */
		retVal = raw_copy_from_user(&is_singtel_mio, (unsigned int __user *)arg, sizeof(unsigned int));

		break;
#if 1
	case 41:/* turn off all LED */
		rtk_led_groupConfig_set(LED_GROUP_DEFAULT, LED_CONFIG_LEDOFF);
		rtk_led_modeForce_set(UTP_PORT0, LED_GROUP_DEFAULT, LED_FORCE_NORMAL);
		rtk_led_modeForce_set(UTP_PORT1, LED_GROUP_DEFAULT, LED_FORCE_NORMAL);
		rtk_led_modeForce_set(UTP_PORT2, LED_GROUP_DEFAULT, LED_FORCE_NORMAL);
		rtk_led_modeForce_set(UTP_PORT3, LED_GROUP_DEFAULT, LED_FORCE_NORMAL);
		break;
	case 42:/* turn on all LED by force */
		rtk_led_groupConfig_set(LED_GROUP_DEFAULT, LED_CONFIG_LINK_ACT);
		rtk_led_modeForce_set(UTP_PORT0, LED_GROUP_DEFAULT, LED_FORCE_ON);
		rtk_led_modeForce_set(UTP_PORT1, LED_GROUP_DEFAULT, LED_FORCE_ON);
		rtk_led_modeForce_set(UTP_PORT2, LED_GROUP_DEFAULT, LED_FORCE_ON);
		rtk_led_modeForce_set(UTP_PORT3, LED_GROUP_DEFAULT, LED_FORCE_ON);
		break;
	case 43:/* turn on all LED normally */
		rtk_led_groupConfig_set(LED_GROUP_DEFAULT, LED_CONFIG_LINK_ACT);
		rtk_led_modeForce_set(UTP_PORT0, LED_GROUP_DEFAULT, LED_FORCE_NORMAL);
		rtk_led_modeForce_set(UTP_PORT1, LED_GROUP_DEFAULT, LED_FORCE_NORMAL);
		rtk_led_modeForce_set(UTP_PORT2, LED_GROUP_DEFAULT, LED_FORCE_NORMAL);
		rtk_led_modeForce_set(UTP_PORT3, LED_GROUP_DEFAULT, LED_FORCE_NORMAL);
		break;
#else
	case 401:/* set static rtk_led_group */
		retVal = raw_copy_from_user(&rtk_led_group, (unsigned int __user *)arg, sizeof(unsigned int));

		break;
	case 4020:/* Get Led operation mode */
		rtk_led_operation_get(&led_mode);
		printk("rtk led operation mode is %d\n", led_mode);

		break;
	case 4021:/* Set Led operation mode */
		retVal = raw_copy_from_user(&led_mode, (unsigned int __user *)arg, sizeof(unsigned int));
		rtk_led_operation_set(led_mode);

		break;
	case 4030:/* Get LED blinking rate at mode 0 to mode 3 */
		rtk_led_blinkRate_get(&blinkRate);
		printk("rtk led operation mode is %d\n", blinkRate);

		break;
	case 4031:/* Set LED blinking rate */
		retVal = raw_copy_from_user(&blinkRate, (unsigned int __user *)arg, sizeof(unsigned int));
		rtk_led_blinkRate_set(blinkRate);

		break;
	case 4040:/* Get Led group congiuration mode */
		rtk_led_groupConfig_get(rtk_led_group, &led_config);
		printk("rtk led group%d config is %x\n", rtk_led_group, led_config);

		break;
	case 4041:/* Set per group Led to congiuration mode */
		retVal = raw_copy_from_user(&led_config, (unsigned int __user *)arg, sizeof(unsigned int));
		rtk_led_groupConfig_set(rtk_led_group, led_config);

		break;
	case 4050:/* Get Led group to congiuration force mode */

		ENUM_PORT_BEGIN(port, mask, LAN_ALL_PORTS_MASK, 1)
			rtk_led_modeForce_get(port, rtk_led_group, &led_force_mode);
			printk("p%d w/ griup%d led force mode = %d\n", port, rtk_led_group, led_force_mode);
		ENUM_PORT_END

		break;
	case 4051:/* Set Led group to congiuration force mode */
		retVal = raw_copy_from_user(&led_force_mode, (unsigned int __user *)arg, sizeof(unsigned int));
		ENUM_PORT_BEGIN(port, mask, LAN_ALL_PORTS_MASK, 1)
			rtk_led_modeForce_set(port, rtk_led_group, led_force_mode);
		ENUM_PORT_END

		break;
	case 41:/* turn off all LED */
		rtk_led_groupConfig_set(rtk_led_group, LED_CONFIG_LEDOFF);
		rtk_led_modeForce_set(UTP_PORT0, rtk_led_group, LED_FORCE_NORMAL);
		rtk_led_modeForce_set(UTP_PORT1, rtk_led_group, LED_FORCE_NORMAL);
		rtk_led_modeForce_set(UTP_PORT2, rtk_led_group, LED_FORCE_NORMAL);
		rtk_led_modeForce_set(UTP_PORT3, rtk_led_group, LED_FORCE_NORMAL);

		break;
	case 4119:/* turn off all LED, ebg19 case */
		rtk_led_groupConfig_set(LED_GROUP_0, LED_CONFIG_LEDOFF);
		rtk_led_groupConfig_set(LED_GROUP_1, LED_CONFIG_LEDOFF);

		ENUM_PORT_BEGIN(port, mask, LAN_ALL_PORTS_MASK, 1)
			rtk_led_modeForce_set(port, LED_GROUP_0, LED_FORCE_NORMAL);
			rtk_led_modeForce_set(port, LED_GROUP_1, LED_FORCE_NORMAL);
		ENUM_PORT_END

		break;
	case 42:/* turn on all LED by force */
		rtk_led_groupConfig_set(rtk_led_group, LED_CONFIG_LINK_ACT);
		rtk_led_modeForce_set(UTP_PORT0, rtk_led_group, LED_FORCE_ON);
		rtk_led_modeForce_set(UTP_PORT1, rtk_led_group, LED_FORCE_ON);
		rtk_led_modeForce_set(UTP_PORT2, rtk_led_group, LED_FORCE_ON);
		rtk_led_modeForce_set(UTP_PORT3, rtk_led_group, LED_FORCE_ON);

		break;
	case 4219:/* turn on all LED by force, ebg19 case */
		rtk_led_groupConfig_set(LED_GROUP_0, LED_CONFIG_LINK_ACT);
		rtk_led_groupConfig_set(LED_GROUP_1, LED_CONFIG_LINK_ACT);

		ENUM_PORT_BEGIN(port, mask, LAN_ALL_PORTS_MASK, 1)
			rtk_led_modeForce_set(port, LED_GROUP_0, LED_FORCE_ON);
			rtk_led_modeForce_set(port, LED_GROUP_1, LED_FORCE_ON);
		ENUM_PORT_END

		break;
	case 43:/* turn on LED normally */
		rtk_led_groupConfig_set(rtk_led_group, LED_CONFIG_LINK_ACT);
		rtk_led_modeForce_set(UTP_PORT0, rtk_led_group, LED_FORCE_NORMAL);
		rtk_led_modeForce_set(UTP_PORT1, rtk_led_group, LED_FORCE_NORMAL);
		rtk_led_modeForce_set(UTP_PORT2, rtk_led_group, LED_FORCE_NORMAL);
		rtk_led_modeForce_set(UTP_PORT3, rtk_led_group, LED_FORCE_NORMAL);

		break;
	case 4319:/* turn on LED normally, ebg19 case */
		rtk_led_groupConfig_set(LED_GROUP_0, LED_CONFIG_LINK_ACT);
		rtk_led_groupConfig_set(LED_GROUP_1, LED_CONFIG_SPD1000);

		ENUM_PORT_BEGIN(port, mask, LAN_ALL_PORTS_MASK, 1)
			rtk_led_modeForce_set(port, LED_GROUP_0, LED_FORCE_NORMAL);
			rtk_led_modeForce_set(port, LED_GROUP_1, LED_FORCE_NORMAL);
		ENUM_PORT_END

		break;
#endif
	case 4350:/* set specified led port group */
		retVal = raw_copy_from_user(&led_port_group, (int __user *)arg, sizeof(int));
		printk("chking led_port_group:%d\n", led_port_group);

		break;

	case 4351:/* get port_group-x's enabled mask */
		rtk_led_enable_get(led_port_group, &portmask);
		printk("Get led_enabled:%x [led_group_%d]\n", portmask.bits[0], led_port_group);

		break;

	case 4352:/* set port_group-x's enabled mask */
		retVal = raw_copy_from_user(&portInfo, (int __user *)arg, sizeof(int));
		portmask.bits[0] = portInfo;
		rtk_led_enable_set(led_port_group, &portmask);
		printk("Set led_enabled:%x [led_group_%d]\n", portInfo, led_port_group);

		break;
	case 44:
		printk("hardware reset rtl8367s\n");
#if 0
		kerSysSetGpioState(24, 0);
		mdelay(20);
		kerSysSetGpioState(24, 1);
#endif
		rtk_ext_swctl_init();

		LANWANPartition();
		vlan_accept_none();
		break;
	case 45:
		/* software init */
		printk("software reset rtl8367s\n");
		if ((retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_CHIP_RESET, RTL8367C_DW8051_RST_OFFSET, 1)) != RT_ERR_OK)
			printk("error software reset!\n");
		break;
	case 46:/* power up specified LAN port */
		retVal = raw_copy_from_user(&port_user, (int __user *)arg, sizeof(int));
		if ((port_user >= 0) && (port_user <= 3)) {
			printk("power up specified LAN port: %d\n", port_user);
			rtk_port_phyReg_get(port_user, PHY_CONTROL_REG, &pData);
			pData &= ~CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(port_user, PHY_CONTROL_REG, pData);
		}
		break;
	case 47:/* power down specified LAN port */
		retVal = raw_copy_from_user(&port_user, (int __user *)arg, sizeof(int));
		if ((port_user >= 0) && (port_user <= 3)) {
			printk("power down specified LAN port: %d\n", port_user);
			rtk_port_phyReg_get(port_user, PHY_CONTROL_REG, &pData);
			pData |= CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(port_user, PHY_CONTROL_REG, pData);
		}
		break;
	case 49:
		printk("disable l2 learning for all ports\n");
		for (i = 0; i <= 17; i++)
			rtk_l2_limitLearningCnt_set(i, 0);
		rtk_l2_table_clear();

		break;
	case 490:
		printk("disable l2 learning for lan/cpu ports\n");
		ENUM_PORT_BEGIN(port, mask, LAN_ALL_PORTS_MASK, 1)
			retVal = rtk_l2_limitLearningCnt_set(port, 0);
			if(retVal != RT_ERR_OK)
				printk("ERROR p%d, errno: %d\n", port, retVal);
		ENUM_PORT_END
		rtk_l2_table_clear();

                ENUM_PORT_BEGIN(port, mask, LAN_ALL_PORTS_MASK, 1)
                        retVal = rtk_l2_limitLearningCnt_get(port, &mac_cnt);
                        if(retVal != RT_ERR_OK)
                                printk("ERROR get p%d, errno: %d\n", port, retVal);
			else
				printk("rtk_l2_limitLearningCnt: p%d = %d\n", port, mac_cnt);
                ENUM_PORT_END

		break;
	case 4916:
		retVal = raw_copy_from_user(&port, (int __user *)arg, sizeof(int));

		printk("disable l2 learning for port: %d\n", port);

		rtk_l2_limitLearningCnt_set(port, 0);
		rtk_l2_table_clear();

		break;
	case 494: /* force ivl_en=1 on current vlan_vid (set via case 36) */
		/* use current vlan_vid set by case 36 */
		printk("rtkswitch: force IVL for vid=%u\n", vlan_vid);
		vlan_force_ivl_for_vid(vlan_vid);
		break;

	case 495: /* get current vlan_vid ivl_en / mbr / untag */
		ret_t = rtk_vlan_get(vlan_vid, &vlan_t);
		if (ret_t != RT_ERR_OK) {
			printk("%s() ERROR vid:%d errono:%d\n", __func__, vlan_vid, ret_t);
			break;
		}
		printk("vid=%u ivl_en=%u mbrmsk=0x%X untagmsk=0x%X\n",
		vlan_vid, vlan_t.ivl_en, vlan_t.mbr.bits[0], vlan_t.untag.bits[0]);
		break;
	case 50:/* Get system flow control type */
		rtl8367c_getAsicFlowControlSelect(&select);
		printk("FlowControlSelect:%d\n", select);

		break;
	case 51:/* Set system flow control type */
		retVal = raw_copy_from_user(&select, (int __user *)arg, sizeof(int));
		rtl8367c_setAsicFlowControlSelect(select);

		break;
	case 52:/* Get Jumbo threhsold(enable/disable) for flow control */
		rtl8367c_getAsicFlowControlJumboMode(&enabled);
		printk("FlowControlJumboMode:%d\n", enabled);

		break;
	case 53:/* Set Jumbo threhsold(enable/disable) for flow control */
		retVal = raw_copy_from_user(&enabled, (int __user *)arg, sizeof(int));
		rtl8367c_setAsicFlowControlJumboMode(enabled);

		break;
	case 54:/* Get Jumbo size for Jumbo mode flow control */
		rtl8367c_getAsicFlowControlJumboModeSize(&size);
		printk("getAsicFlowControlJumboModeSize:%d\n", size);

		break;
	case 55:/* Set Jumbo size for Jumbo mode flow control */
		retVal = raw_copy_from_user(&size, (int __user *)arg, sizeof(int));
		rtl8367c_setAsicFlowControlJumboModeSize(size);

		break;
	case 60:/* Get permitted port isolation portmask */
		//retVal = raw_copy_from_user(&port, (int __user *)arg, sizeof(int));
		//rtl8367c_getAsicPortIsolationPermittedPortmask(port, &permitPortmask);
		//printk("PortIsolationPermittedPortmask:%d: %d\n", port, permitPortmask);
		get_port_isolation_mask_efid();

		break;
	case 61:/* Set target iso_port. */
		retVal = raw_copy_from_user(&iso_port, (int __user *)arg, sizeof(int));

		break;
	case 62:/* Set permitted port isolation portmask */
		retVal = raw_copy_from_user(&permitPortmask, (int __user *)arg, sizeof(int));
		//rtl8367c_setAsicPortIsolationPermittedPortmask(iso_port, permitPortmask);
                rtk_port_isolation_set(port, &permitPortmask);
                rtk_port_efid_set(port, 0);

		break;
	case 621:/* Set each port isolation in portmask */
		retVal = raw_copy_from_user(&portInfo, (int __user *)arg, sizeof(int));
		ports_isolation(1, portInfo);

		break;
	case 622:/* Unset each port isolation in portmask */
		retVal = raw_copy_from_user(&portInfo, (int __user *)arg, sizeof(int));
		ports_isolation(0, portInfo);

		break;
	case 70:/* Get TxDelay, RxDelay */
		port = EXT_PORT1;
		rtk_port_rgmiiDelayExt_get(port, &TxDelay, &RxDelay);
		printk("Get port%d txdelay=%d, rxdelay=%d\n", port, TxDelay, RxDelay);

		break;
	case 701:/* Set specified TxDelay */
		port = EXT_PORT1;
		retVal = raw_copy_from_user(&TxDelay, (int __user *)arg, sizeof(int));

		printk("Set port%d txdelay=%d, rxdelay=%d\n", port, TxDelay, RxDelay);
		rtk_port_rgmiiDelayExt_set(port, TxDelay, RxDelay);
	
		break;
	case 702:/* Set specified RxDelay */
		port = EXT_PORT1;
		retVal = raw_copy_from_user(&RxDelay, (int __user *)arg, sizeof(int));

		printk("Set port%d txdelay=%d, rxdelay=%d\n", port, TxDelay, RxDelay);
		rtk_port_rgmiiDelayExt_set(port, TxDelay, RxDelay);
	
		break;
# if 0
	case 63:/* Get all ports isolation EFID */
		ENUM_PORT_BEGIN(port, mask, LAN_PORTS_MASK, 1)
			rtl8367c_getAsicPortIsolationEfid(port, &efid);
			printk("port[%d] efid:%d\n", port, efid);
		ENUM_PORT_END

		break;

	case 64:/* Set port isolation EFID */
		retVal = raw_copy_from_user(&efid, (int __user *)arg, sizeof(int));
		rtl8367c_setAsicPortIsolationEfid(iso_port, efid);

		break;
#endif
	case 900:/* Set phy x in testmode */
		retVal = raw_copy_from_user(&phytest_port, (int __user *)arg, sizeof(int));

		break;

	case 901:/* Set phy testmode x(1, 4) */
		retVal = raw_copy_from_user(&portInfo, (int __user *)arg, sizeof(int));

		test_mode = portInfo;
		printk("%s: phy test mode:%d\n", __func__, test_mode);
		//ENUM_PORT_BEGIN(port, mask, LAN_PORTS_MASK, 1)
		retVal = rtk_port_phyTestMode_set(phytest_port, test_mode);
		printk("p[%d], ret = %d\n", phytest_port, retVal);
		//ENUM_PORT_END
		
		break;
	case 902:/* Get phy testmode */
		rtk_port_phyTestMode_get(phytest_port, &test_mode);
		printk("p[%d] Get phy_test_mode=%x\n", phytest_port, test_mode);

		break;
	default:
		return -EINVAL;
	}

	return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
static DEFINE_MUTEX(rtkswitch_mutex);
static DEFINE_MUTEX(rtk_api_mutex);

static long unlocked_rtl8367s_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
	struct inode *inode;
	long rt;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)
	inode = filep->f_dentry->d_inode;
#else
	inode = file_inode(filep);
#endif
	mutex_lock(&rtkswitch_mutex);
	rt = rtl8367s_ioctl( inode, filep, cmd, arg );
	mutex_unlock(&rtkswitch_mutex);

	return rt;
}

void RTK_API_LOCK(void)
{
	mutex_lock(&rtk_api_mutex);
}

void RTK_API_UNLOCK(void)
{
	mutex_unlock(&rtk_api_mutex);
}
#endif

static struct file_operations rtl8367s_fops =
{
	.owner		= THIS_MODULE,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
	.unlocked_ioctl	= unlocked_rtl8367s_ioctl,
#if defined(CONFIG_COMPAT)
	.compat_ioctl	= unlocked_rtl8367s_ioctl,
#endif
#else
	.ioctl		= rtl8367s_ioctl,
#endif
	.open		= rtl8367s_open,
	.release	= rtl8367s_release,
};

static int __init rtl8367s_init(void)
{
	int ret;

	ret = register_chrdev(rtl8367s_major, RTL8367S_DEVNAME, &rtl8367s_fops );
	if (ret < 0)
		printk("rtl8367s_init(major %d): fail to register device.\n", rtl8367s_major);
	else
		printk("rtl8367s: rtl8367s_init entry\n");

	printk("init rtk switch %s:%d\n", __FUNCTION__, __LINE__);
	rtk_ext_swctl_init();

	LANWANPartition();
	vlan_accept_none();

	return ret;
}

static void __exit rtl8367s_exit(void)
{
	unregister_chrdev(rtl8367s_major, RTL8367S_DEVNAME);

	printk("RTL8367S driver exited\n");
}

module_init(rtl8367s_init);
module_exit(rtl8367s_exit);

MODULE_LICENSE("GPL");
