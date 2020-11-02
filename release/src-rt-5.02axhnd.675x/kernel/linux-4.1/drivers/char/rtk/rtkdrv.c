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

#define LAN_PORT_1			0
#define LAN_PORT_2			1
#define LAN_PORT_3			2
#define LAN_PORT_4			3
#define LAN_PORT_EXT			17

#define LAN_PORT_1_MASK			(1U << LAN_PORT_1)
#define LAN_PORT_2_MASK			(1U << LAN_PORT_2)
#define LAN_PORT_3_MASK			(1U << LAN_PORT_3)
#define LAN_PORT_4_MASK			(1U << LAN_PORT_4)
#define LAN_PORT_EXT_MASK		(1U << LAN_PORT_EXT)

#define LAN_ALL_PORTS_MASK		(LAN_PORT_EXT_MASK | LAN_PORT_1_MASK | LAN_PORT_2_MASK | LAN_PORT_3_MASK | LAN_PORT_4_MASK)
#define LAN_PORTS_MASK			(LAN_PORT_1_MASK | LAN_PORT_2_MASK | LAN_PORT_3_MASK | LAN_PORT_4_MASK)

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

#define ENUM_PORT_BEGIN(p, m, port_mask, cond) 	\
	for (p = 0, m = (port_mask);		\
		cond && m > 0;			\
		m >>= 1, p++) {			\
		if (!(m & 1U))			\
			continue;

#define ENUM_PORT_END	}

typedef struct {
	unsigned int link[4];
	unsigned int speed[4];
	unsigned int duplex[4];
} phyState;

typedef struct {
	unsigned int count;
	ether_addr_t ea[256];
} mactable;

static DEFINE_MUTEX(rtkswitch_mutex);

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

	ENUM_PORT_BEGIN(port, mask, LAN_PORTS_MASK, 1)
		ret_t = rtk_vlan_portAcceptFrameType_set(port, ACCEPT_FRAME_TYPE_UNTAG_ONLY);
		if(ret_t != RT_ERR_OK)
			printk("%s() ERROR port:%d errono:%d\n", __func__, port, ret_t);
	ENUM_PORT_END
}

unsigned int is_singtel_mio = 0;

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

/* portInfo:	bit0-bit15 : member mask
		bit16-bit31 :  untag mask */
void createVlan(u32 portinfo)
{
	rtk_int32 ret_t;
	rtk_vlan_cfg_t vlan_t;
	unsigned int port, mask;
	u32 laninfo = 0;

	memset(&vlan_t, 0x00, sizeof(rtk_vlan_cfg_t));
	vlan_t.mbr.bits[0] = (portinfo & 0x0000FFFF) | 0x00020000; //add CPU port to member
	laninfo = (portinfo & 0x0000FFFF) | 0x00020000; //add CPU port to portPvid set
	vlan_t.untag.bits[0] = portinfo >> 16; // CPU port leave tag
	printk("createVlan - vid = %d prio = %d mbrmsk = 0x%X untagmsk = 0x%X\n", vlan_vid, vlan_prio, vlan_t.mbr.bits[0], vlan_t.untag.bits[0]);	
	ret_t = rtk_vlan_set(vlan_vid, &vlan_t);
	if(ret_t != RT_ERR_OK)
		printk("%s() ERROR vid:%d errono:%d\n", __func__, vlan_vid, ret_t);

	ENUM_PORT_BEGIN(port, mask, laninfo, 1)
		ret_t = rtk_vlan_portPvid_set(port, vlan_vid, vlan_prio);
		if(ret_t != RT_ERR_OK)
			printk("%s() ERROR port:%d errono:%d\n", __func__, port, ret_t);
	ENUM_PORT_END
}

static int rtkswitch_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	rtk_api_ret_t retVal;
	rtk_port_t port;
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

	switch(cmd) {
	case 0:	// Get LAN phy status of all LAN ports
		copy_from_user(&pS, (phyState __user *)arg, sizeof(pS));

		ENUM_PORT_BEGIN(port, mask, LAN_PORTS_MASK, 1)
			retVal = rtk_port_phyStatus_get(port, &LinkStatus, &Speed, &Duplex);
			pS.link[port] = LinkStatus;
			pS.speed[port] = Speed;
			pS.duplex[port] = Duplex;
		ENUM_PORT_END

		copy_to_user((phyState __user *)arg, &pS, sizeof(pS));

		break;
	case 1:	// Dump all counters of the specified LAN port
		copy_from_user(&port_user, (int __user *)arg, sizeof(int));

		printk("rtk_stat_port_getAll(%d)\n", port_user);
		retVal = rtk_stat_port_getAll(port_user, &Port_cntrs);

		if (retVal == RT_ERR_OK)
			copy_to_user((rtk_stat_port_cntr_t __user *)arg, &Port_cntrs, sizeof(rtk_stat_port_cntr_t));
		else {
			printk("rtk_stat_port_getAll(%d) return %d\n", port_user, retVal);
			return retVal;
		}

		break;
	case 3:	// Get link status of the specified LAN port
		copy_from_user(&port_user, (int __user *)arg, sizeof(int));

		retVal = rtk_port_phyStatus_get(port_user, &LinkStatus, &Speed, &Duplex);
		if (retVal == RT_ERR_OK)
			port_user = LinkStatus;
		else
			port_user = 0;

		copy_to_user((int __user *)arg, &port_user, sizeof(int));

		break;
	case 4:	// Get link status of LAN ports
		memset(&pS, 0, sizeof(pS));
		for (port = 0; port < 4; port++)
		{
			retVal = rtk_port_phyStatus_get(port, &LinkStatus, &Speed, &Duplex);
			pS.link[port] = LinkStatus;
			pS.speed[port] = Speed;
			pS.duplex[port] = Duplex;
		}

		port_user = 0;
		if (pS.link[0] || pS.link[1] || pS.link[2] || pS.link[3])
			port_user = 1;

		copy_to_user((int __user *)arg, &port_user, sizeof(int));

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
		copy_from_user(&port_user, (int __user *)arg, sizeof(int));

		memset(&Port_mactable, 0, sizeof(mactable));

		while (1) {
			if ((retVal = rtk_l2_addr_next_get(READMETHOD_NEXT_L2UC, port_user, &address, &l2_data)) != RT_ERR_OK)
				break;

			if (count > 255)
				break;

			if (port_user == l2_data.port) {
#if 0
				printk("%02X:%02X:%02X:%02X:%02X:%02X\n", l2_data.mac.octet[0], l2_data.mac.octet[1], l2_data.mac.octet[2], l2_data.mac.octet[3], l2_data.mac.octet[4], l2_data.mac.octet[5]);
#endif
				memcpy(Port_mactable.ea[count++].octet, l2_data.mac.octet, ETHER_ADDR_LEN);
			}

			address++;
		}

		Port_mactable.count = count;
		copy_to_user((mactable __user *)arg, &Port_mactable, sizeof(mactable));

		break;
	case 8: // reset per port MIB counter
		printk("reset per port MIB counter\n");
		ENUM_PORT_BEGIN(port, mask, LAN_PORTS_MASK, 1)
			rtk_stat_port_reset(port);
		ENUM_PORT_END

		break;
	case 9: // LAN/WAN partition
		copy_from_user(&wan_stb_x, (int __user *)arg, sizeof(int));

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
	case 36:/* Set Vlan VID. */
		copy_from_user(&vlan_vid, (int __user *)arg, sizeof(int));		

		break;
	case 37:/* Set Vlan PRIO. */
		copy_from_user(&vlan_prio, (int __user *)arg, sizeof(int));

		break;
	case 38:/* Initialize VLAN */
		copy_from_user(&portInfo, (int __user *)arg, sizeof(int));
		initialVlan((u32) portInfo);
		vlan_accept_adv(wan_stb_x);

		break;
	case 39:/* Create VLAN. */
		copy_from_user(&portInfo, (int __user *)arg, sizeof(int));
		createVlan((u32) portInfo);

		break;
	case 40:
		copy_from_user(&is_singtel_mio, (unsigned int __user *)arg, sizeof(unsigned int));

		break;

	default:
		return -EINVAL;
	}

	return 0;
}

static long rtkswitch_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret;

	mutex_lock(&rtkswitch_mutex);
	ret = rtkswitch_ioctl(file, cmd, arg);
	mutex_unlock(&rtkswitch_mutex);

	return ret;
}

int rtl8367s_open(struct inode *inode, struct file *file)
{
	return 0;
}

int rtl8367s_release(struct inode *inode, struct file *file)
{
	return 0;
}

const struct file_operations rtl8367s_fops = {
	.owner		= THIS_MODULE,
	.unlocked_ioctl	= rtkswitch_unlocked_ioctl,
	.open		= rtl8367s_open,
	.release	= rtl8367s_release,
};

int rtk_ext_swctl_init(void);

int __init rtl8367s_init(void)
{
	rtk_api_ret_t retVal;
	rtk_data_t pLen;
	rtk_data_t pEnable;

#ifdef CONFIG_DEVFS_FS
	if (devfs_register_chrdev(rtl8367s_major, RTL8367S_DEVNAME,
				&rtl8367s_fops)) {
		printk(KERN_ERR NAME ": unable to register character device\n");
		return -EIO;
	}
	devfs_handle = devfs_register(NULL, RTL8367S_DEVNAME,
			DEVFS_FL_DEFAULT, rtl8367s_major, 0,
			S_IFCHR | S_IRUGO | S_IWUGO, &rtl8367s_fops, NULL);
#else
	int r = 0;
	r = register_chrdev(rtl8367s_major, RTL8367S_DEVNAME,
			&rtl8367s_fops);
	if (r < 0) {
		printk(KERN_ERR NAME ": unable to register character device\n");
		return r;
	}
	if (rtl8367s_major == 0) {
		rtl8367s_major = r;
		printk(KERN_DEBUG NAME ": got dynamic major %d\n", r);
	}
#endif

	printk("init rtk switch %s:%d\n", __FUNCTION__, __LINE__);
	rtk_ext_swctl_init();

	retVal = rtk_switch_maxPktLenCfg_get(0, &pLen);
	printk("rtk_switch_maxPktLenCfg_get(): return %d\n", retVal);
	printk("current rtk_switch_maxPktLen: %d\n", pLen);
	retVal = rtk_switch_maxPktLenCfg_set(0, RTK_SWITCH_MAX_PKTLEN);
	printk("rtk_switch_maxPktLenCfg_set(): return %d\n", retVal);

	retVal = rtk_switch_greenEthernet_get(&pEnable);
	printk("rtk_switch_greenEthernet_get(): return %d\n", retVal);
	printk("current rtk_switch_greenEthernet state: %d\n", pEnable);
	retVal = rtk_switch_greenEthernet_set(ENABLED);
	printk("rtk_switch_greenEthernet_set(): return %d\n", retVal);

	LANWANPartition();
	vlan_accept_none();

	return 0;
}

void __exit rtl8367s_exit(void)
{
#ifdef CONFIG_DEVFS_FS
	devfs_unregister_chrdev(rtl8367s_major, RTL8367S_DEVNAME);
	devfs_unregister(devfs_handle);
#else
	unregister_chrdev(rtl8367s_major, RTL8367S_DEVNAME);
#endif

	printk("RTL8367S driver exited\n");
}

module_init(rtl8367s_init);
module_exit(rtl8367s_exit);

MODULE_LICENSE("GPL");
