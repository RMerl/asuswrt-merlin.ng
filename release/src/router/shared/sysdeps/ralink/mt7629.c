/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <bcmnvram.h>

#include <utils.h>
#include <shutils.h>
#include <shared.h>
#ifdef RTACRH18
#include <linux/autoconf.h>
#else
#include <linux/config.h>
#endif
#include <ralink_gpio.h>
#include <ralink.h>
#include "ra_ioctl.h"
#include "mt7629.h"

//#define RTKSWITCH_DEV	"/dev/rtkswitch"
#define ETH_DEVNAME     "eth0"
#define MII_IFNAME      "switch0"
#define NR_WANLAN_PORT  5

int esw_fd;

enum {
    WAN_PORT=0,
    LAN1_PORT=1,
    LAN2_PORT=2,
    LAN3_PORT=3,
    LAN4_PORT=4,
    P5_PORT=5,  
    CPU_PORT=6,
    P7_PORT=7,
};

#define BIT(n)  (1 << (n))
/* 0: LAN, 1:WAN or STB */
static const int lan_wan_partition[] = {
    BIT( WAN_PORT ),                    // WAN
    BIT( WAN_PORT ) | BIT( LAN1_PORT ),         // WAN + LAN1
    BIT( WAN_PORT ) | BIT( LAN2_PORT ),         // WAN + LAN2
    BIT( WAN_PORT ) | BIT( LAN3_PORT ),         // WAN + LAN3
    BIT( WAN_PORT ) | BIT( LAN4_PORT ),         // WAN + LAN4   
    BIT( WAN_PORT ) | BIT( LAN1_PORT ) | BIT( LAN2_PORT ),  // WAN + LAN1+2
    BIT( WAN_PORT ) | BIT( LAN3_PORT ) | BIT( LAN4_PORT ),  // WAN + LAN3+4
    0,                          // ALL LAN
};

/* Final model-specific LAN/WAN/WANS_LAN partition definitions.
 * bit0: P0, bit1: P1, bit2: P2, bit3: P3, bit4: P4
 */
static unsigned int lan_mask = 0;   /* LAN only. Exclude WAN, WANS_LAN, and generic IPTV port. */
static unsigned int wan_mask = 0;   /* wan_type = WANS_DUALWAN_IF_WAN. Include generic IPTV port. */
static unsigned int wans_lan_mask = 0;  /* wan_type = WANS_DUALWAN_IF_LAN. */

/* RT-N56U's P0, P1, P2, P3, P4 = LAN4, LAN3, LAN2, LAN1, WAN
 * ==> Model-specific port number.
 */
static int switch_port_mapping[] = {    
    LAN4_PORT,  //0000 0000 0001 LAN4
    LAN3_PORT,  //0000 0000 0010 LAN3   
    LAN2_PORT,  //0000 0000 0100 LAN2
    LAN1_PORT,  //0000 0000 1000 LAN1
    WAN_PORT,   //0000 0001 0000 WAN
    P5_PORT,    //0000 0010 0000 -
    P5_PORT,    //0000 0100 0000 -
    P5_PORT,    //0000 1000 0000 -
    CPU_PORT,   //0001 0000 0000 RGMII LAN
    P7_PORT,    //0010 0000 0000 RGMII WAN
};

/* Model-specific LANx ==> Model-specific PortX mapping */
const int lan_id_to_port_mapping[NR_WANLAN_PORT] = {
    WAN_PORT,   /* not used */
    LAN1_PORT,
    LAN2_PORT,
    LAN3_PORT,
    LAN4_PORT,
};

/* Model-specific LANx ==> Model-specific PortX */
static inline int lan_id_to_port_nr(int id)
{
    return lan_id_to_port_mapping[id];
}

/**
 * Get WAN port mask
 * @wan_unit:   wan_unit, if negative, select WANS_DUALWAN_IF_WAN
 * @return: port bitmask
 */
static unsigned int get_wan_port_mask(int wan_unit)
{
    char nv[] = "wanXXXports_maskXXXXXX";
    if (sw_mode() == SW_MODE_REPEATER)
        return 0;

    if (wan_unit <= 0 || wan_unit >= WAN_UNIT_MAX)
        strlcpy(nv, "wanports_mask", sizeof(nv));
    else
        snprintf(nv, sizeof(nv), "wan%dports_mask", wan_unit);
    
    return nvram_get_int(nv);    
}


/**
 * Get LAN port mask
 * @return: port bitmask
 */
static unsigned int get_lan_port_mask(void)
{
    int sw_mode = sw_mode();
    unsigned int m = nvram_get_int("lanports_mask");

    if (sw_mode == SW_MODE_AP)
        m = 0x1F;

    return m;
}

/**
 * Create string based portmap base on mask parameter.
 * @mask:   port bit mask.
 *      bit0: P0, bit1: P1, etc
 * @portmap:    pointer to char array. minimal length is 9 bytes.
 */
static void __create_port_map(unsigned int mask, char *portmap)
{
    int i;
    char *p;
    unsigned int m;

    for (i = 0, m = mask, p = portmap; i < 8; ++i, m >>= 1, ++p)
        *p = '0' + (m & 1);
    *p = '\0';
}

int switch_init(void)
{
    esw_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (esw_fd < 0) {
        perror("socket");
        return -1;
    }
    return 0;    
}

void switch_fini(void)
{
    close(esw_fd);
}

int mt7531_reg_read(int offset, int *value)
{
    struct ifreq ifr;
    esw_reg reg;

    ra_mii_ioctl_data mii;

    if (value == NULL)
        return -1;

    strncpy(ifr.ifr_name, ETH_DEVNAME, IFNAMSIZ);
    ifr.ifr_data = &mii;

    mii.phy_id = 0x1f;
    mii.reg_num = offset;

    if (-1 == ioctl(esw_fd, RAETH_MII_READ, &ifr)) {
        perror("ioctl");
        _dprintf("%s: read esw register fail. errno %d (%s)\n", __func__, errno, strerror(errno));
        close(esw_fd);
        return -1;
    }

    *value = mii.val_out;   
    //_dprintf("%s(%d): offset:0x%08X, value:0x%08X\n", __func__, __LINE__, offset, *value);     
    return 0;
}

int mt7531_reg_write(int offset, int value)
{
    struct ifreq ifr;
    esw_reg reg;
    ra_mii_ioctl_data mii;
    
    strncpy(ifr.ifr_name, ETH_DEVNAME, 5);
    ifr.ifr_data = &mii;
    
    mii.phy_id = 0x1f;
    mii.reg_num = offset;
    mii.val_in = value;
    //_dprintf("%s(%d): offset:0x%08X, value:0x%08X\n", __func__, __LINE__, offset, value);
    if (-1 == ioctl(esw_fd, RAETH_MII_WRITE, &ifr)) {
        perror("ioctl");
        _dprintf("ioctl error.\n");
        close(esw_fd);
        exit(0);
    }
    
    return 0;    
}

static void write_VTCR(unsigned int value)
{
    int i;
    unsigned int check;

    value |= 0x80000000;
    mt7531_reg_write(REG_ESW_VLAN_VTCR, value);

    for (i = 0; i < 20; i++) {
        mt7531_reg_read(REG_ESW_VLAN_VTCR, &check);
        if ((check & 0x80000000) == 0 ) //table busy
            break;
        usleep(1000);
    }

    if (i == 20)
    {
        cprintf("VTCR timeout.\n");
    }
    else if(check & (1<<16))
    {
        cprintf("%s(%08x) invalid\n", __func__, value);
    }    
}

/**
 * Find first available vlan slot or find vlan slot by vid.
 * VLAN slot 0~2 are reserved.
 * If VLAN ID is not equal to index + 1, it is assumed unavailable.
 * @vid:
 *  >0: Find vlan slot by vid.
 *  <=0:    Find first available vlan slot.
 * @vawd1:  pointer to a unsigned integer.
 *      If vlan slot found and vawd1 is not null, save VAWD1 value here.
 * @return:
 *  0~15:   vlan slot index.
 *  <0: not found
 */
static int find_vlan_slot(int vid, unsigned int *vmsc)
{
    int idx; 
    unsigned int i, r, v, vmsc_value, value;

    for (i = 2, idx = -1; idx < 0 && i < 16; ++i) {
        if ((r = mt7531_reg_read(REG_ESW_VLANI0 + 4*(i>>1), &value))) {
            _dprintf("read VLANI0-7, i %d offset %x fail. (r = %d)\n", i, REG_ESW_VLANI0 + 4*(i>>1), r);
            continue;
        }
                        
        if (!(i&1))
            v = value & 0xfff;
        else
            v = (value  >> 12) & 0xfff;
        if ((vid <= 0 && v != (i + 1)) || (vid > 0 && v != vid))
            continue;
        /* Read VMSC */
        mt7531_reg_read(REG_ESW_VMSC0 + 4*(i>>2), &vmsc_value);
                                                        
        value = (vmsc_value >> (8*(i%4))) & 0xff;
        if ((vid <= 0) && v == (i + 1) && value == 0xff)
            idx = i; 
        else if (vid > 0 && v == vid)
            idx = i;
                                                                                
        if (idx >= 0 && vmsc)
            *vmsc = vmsc_value;
    }
        
    return idx;
}

/**
 * Set a VLAN
 * @idx:    VLAN table index
 *  >= 0:   specify VLAN table index.
 *  <  0:   find available VLAN table.
 * @vid:    VLAN ID
 * @portmap:    Port member. string length must greater than or equal to 8.
 *      Only '0' and '1' are accepted.
 *      First digit means port 0, second digit means port1, etc.
 *      P0, P1, P2, P3, P4, P5, P6, P7
 * @stag:   Service tag
 * @return
 *      0:  success
 *     -1:  no vlan entry available
 *     -2:  invalid parameter
 */
int mt7531_vlan_set(int idx, int vid, char *portmap, int stag)
{
    unsigned int i, value, mbr, vmsc;
    
    if (idx < 0) {
        if ((idx = find_vlan_slot(vid, &vmsc)) < 0 && (idx = find_vlan_slot(-1, &vmsc)) < 0) {
            _dprintf("%s: no empty vlan entry for vid %d portmap %s stag %d!\n",
                __func__, vid, portmap, stag);
            return -1;
        }
    }
    else {
        mt7531_reg_read(REG_ESW_VMSC0 + 4*(idx>>2), &vmsc);
    }

    _dprintf("%s: idx=%d, vid=%d, portmap=%s, stag=%d\n", __func__, idx, vid, portmap, stag);
    
    //set vlan identifie
    mt7531_reg_read(REG_ESW_VLANI0 + 4*(idx>>1), &value);
    if (idx % 2 == 0) {
        value &= 0xfff000;
        value |= vid;
    }
    else {
        value &= 0xfff;
        value |= (vid << 12);
    }
    mt7531_reg_write(REG_ESW_VLANI0 + 4*(idx>>1), value);
    mt7531_reg_read(REG_ESW_VLANI0 + 4*(idx>>1), &value);   
    _dprintf("%s: IDX value = 0x%x\n", __func__, value);

    //set vlan member
    mbr = 0;
    for (i = 0; i < 8; i++)
        mbr += (*(portmap + i) - '0') * (1 << i);
    if (idx % 4 == 0) {
        vmsc &= 0xffffff00;
    }
    else if (idx % 4 == 1) {
        vmsc &= 0xffff00ff;
    }
    else if (idx % 4 == 2) {
        vmsc &= 0xff00ffff;
    }
    else {
        vmsc &= 0x00ffffff;
    }

    vmsc |= (mbr << 8*(idx%4));
    mt7531_reg_write(REG_ESW_VMSC0 + 4*(idx>>2), vmsc); 
    return 0;
}

struct trafficCount_t
{
	long long rxByteCount;
	long long txByteCount;
};

int get_ralink_lan_status(unsigned int port, unsigned int *linkup, unsigned int *linkspeed)
{
    FILE *fp = NULL;
    int len = 0;
    char s[128], b[1024];

    if (!linkup || !linkspeed) {
        _dprintf("Wrong argument value.\n");
        goto lan_status_error;
    }

    *linkup = 0;
    *linkspeed = 0;

    memset(s, 0, sizeof(s));
    snprintf(s, sizeof(s), "swconfig dev switch0 port %d get link", port);
    if (!(fp = popen(s, "r"))) {
        _dprintf("popen error.\n");
        goto lan_status_error;
    }
    
    memset(b, 0, sizeof(b));
    len = fread(b, 1, sizeof(b), fp);
    if (len > 1) {
        b[len - 1] = '\0';
        if (strstr(b, "link:up")) {
            *linkup = 1;
            if (strstr(b, "speed:1000baseT"))
                *linkspeed = 2;
            else if (strstr(b, "speed:100baseT"))
                *linkspeed = 1;
            else 
                *linkspeed = 0;
        }
    }

    pclose(fp);
    return 0;

lan_status_error:
    if (fp)
        pclose(fp);
    return -1;
}

int get_ralink_wan_status(unsigned int *linkup, unsigned int *linkspeed)
{
    unsigned long reg1_val = 0, reg10_val = 0;  
    struct ifreq ifr;
    ra_mii_ioctl_data mii;
    int sk = -1;
    
    if (!linkup || !linkspeed) {
        _dprintf("Wrong argument value.\n");
        goto wan_status_error;
    }

    *linkup = 0;
    *linkspeed = 0;

    sk = socket(AF_INET, SOCK_DGRAM, 0);
    if (sk < 0) {
        _dprintf("Open socket failed.\n");
        goto wan_status_error;
    }

    strncpy(ifr.ifr_name, ETH_DEVNAME, 5);
    ifr.ifr_data = (char *)&mii;

    mii.phy_id = 0;
    mii.reg_num = 1;

    if (ioctl(sk, RAETH_MII_READ, &ifr) < 0) {
        _dprintf("ioctl error.\n");
        goto wan_status_error;
    }

    reg1_val = mii.val_out;

    mii.reg_num = 10;
    if (ioctl(sk, RAETH_MII_READ, &ifr) < 0) {
        _dprintf("ioctl error.\n");
        goto wan_status_error;
    }

    reg10_val = mii.val_out;

    if ((reg1_val & (1<<2))) {
        *linkup = 1;
        if ((reg10_val & (1<<10)) || (reg10_val & (1<<11)))
            *linkspeed = 2; // 1000M
        else {
            if ((reg1_val & (1<<14)) || (reg1_val & (1<<13)))
                *linkspeed = 1; // 100M
            else
                *linkspeed = 0; // 10M
        }
    }

    close(sk);
    return 0;

wan_status_error:
    if (sk > -1)
        close(sk);
    return -1;
}

void reset_mt7531_esw(void)
{
    unsigned int value = 0;

    if (switch_init() < 0)
        return;

    /*Software Register Reset  and Software System Reset */
    mt7531_reg_write(0x7000, 0x3);
    mt7531_reg_read(0x7000, &value);
    _dprintf("SYS_CTRL(0x7000) register value =0x%x  \n", value);
    //if (chip_name == 0x7531) {
        mt7531_reg_write(0x7c0c, 0x11111111);
        mt7531_reg_read(0x7c0c, &value);
        _dprintf("GPIO Mode (0x7c0c) select value =0x%x  \n", value);
    //} 
    _dprintf("Switch Software Reset !!! \n");

    switch_fini();
    return;
}
static void set_Port_PVID(int vid, int bitmap)
{
    unsigned int value = 0;
    unsigned short i;
            
    if (switch_init() < 0)
        return;
                
    for (i = 0; i < 7; i++) {
        if (bitmap & (1<<i)) {
            /* Set PVID */
            mt7531_reg_read(REG_ESW_PVIDC0 + 4*(i/2), &value);
            if (i % 2) { /* 23:12 bits */
                value &= 0xfff;
                value |= (vid << 12);
            }
            else { /* 11:0 bits */
                value &= 0xfff000;
                value |= vid;
            }
                                                                                
            mt7531_reg_write(REG_ESW_PVIDC0 + 4*(i/2), value);
        }
    }
                    
    switch_fini();
}


static void set_Port_PRIO(int prio, int bitmap)
{
    if (prio == 0) // Prio 0 is default. So skip.
        return;
        
    if (switch_init() < 0)
        return;
    unsigned int value = 0, value2 = 0;
    unsigned short i = 0;
    unsigned short high_prio = nvram_get_int("switch_wan1prio");
    unsigned short low_prio = nvram_get_int("switch_wan2prio");
    unsigned short tmp = 0, q_number = 1;

    /* Get switch_wan1prio switch_wan2prio */
    if (low_prio > high_prio) {
        tmp = low_prio;
        low_prio = high_prio;
        high_prio = tmp;    
    }

    if (high_prio > 0) { // Q1 default is 0
        if (low_prio <= 0 || (high_prio == low_prio)) { // Just set Q2
            if (high_prio != 7 && high_prio != 5 && high_prio != 2) { // Q3 default is 7. Q2 default is 5. Q0 default is 2.
                /* Set Q2 */
                mt7531_reg_read(REG_ESW_PVIDC3, &value);
                value &= 0xf0ffffff;
                value += (high_prio << 24);
                mt7531_reg_write(REG_ESW_PVIDC3, value);
            }
        }
        else { // Set Q3 Q2 
            if (low_prio != 5 && low_prio != 2 ) { // Q2 default is 5. Q0 default is 2.
                /* Set Q2 */
                mt7531_reg_read(REG_ESW_PVIDC3, &value);
                value &= 0xf0ffffff;
                value += (low_prio << 24);
                mt7531_reg_write(REG_ESW_PVIDC3, value);
            }
            if (high_prio != 7 && high_prio != 2) {
                /* Set Q3 */
                mt7531_reg_read(REG_ESW_PVIDC3, &value);
                value &= 0x0fffffff;
                value += (high_prio << 28);
                mt7531_reg_write(REG_ESW_PVIDC3, value);
            }
        }
    }

    /* Find prio queue number */
    mt7531_reg_read(REG_ESW_PVIDC3, &value);
    value = value >> 16;
    
    for (i = 0; i < 4; i++) {
        value2 = value >> (i*4);
        if ((value2 & 0xf) == prio) {
            q_number = i;
            break;
        }   
    } 

    /* Set default queue per port */
    mt7531_reg_read(REG_ESW_PFC1, &value);
    for (i = 0; i < 7; i++) {
            
            if (bitmap & (1<<i)) {
                value &= (0xffffffff - (0x3 << (i*2)));
                value += (q_number << (i*2));
            }   
    }   
    mt7531_reg_write(REG_ESW_PFC1, value);
    
    switch_fini();
}

static void set_Vlan_VID(int vid)
{
    char tmp[8];

    snprintf(tmp, sizeof(tmp), "%d", vid);
    nvram_set("vlan_vid", tmp);
}

static void set_Vlan_PRIO(int prio)
{
    char tmp[2];

    snprintf(tmp, sizeof(tmp), "%d", prio);
    nvram_set("vlan_prio", tmp);
}

//convert port mapping from  RT-N56U   to   RT-N14U / RT-AC52U / RT-AC51U (MT7620) /RT-N54U
static int convert_port_bitmask(int orig)
{
    int i, mask, result;
    result = 0;
    for(i = 0; i < NR_WANLAN_PORT; i++) {
        mask = (1 << i);
        if (orig & mask)
            result |= (1 << switch_port_mapping[i]);
    }
    return result;
}

static void build_wan_lan_mask(int stb)
{
    int unit;
    int wanscap_lan = get_wans_dualwan() & WANSCAP_LAN;
    int wans_lanport = nvram_get_int("wans_lanport");
    int sw_mode = sw_mode();
    char prefix[8], nvram_ports[20];

    if (sw_mode == SW_MODE_AP || sw_mode == SW_MODE_REPEATER)
        wanscap_lan = 0;

    if (stb == 100 && (sw_mode == SW_MODE_AP || __mediabridge_mode(sw_mode)))
        stb = 7;    /* Don't create WAN port. */

    if (wanscap_lan && (wans_lanport < 0 || wans_lanport > 4)) {
        _dprintf("%s: invalid wans_lanport %d!\n", __func__, wans_lanport);
        wanscap_lan = 0;
    }

    lan_mask = wan_mask = wans_lan_mask = 0;
    if(stb < 0 || stb >= ARRAY_SIZE(lan_wan_partition)) 
    {
        _dprintf("%s: invalid partition index: %d\n", __func__, stb);
        stb = 0;
    }

    wan_mask = lan_wan_partition[stb];
    lan_mask = ((1<<NR_WANLAN_PORT) -1) & ~lan_wan_partition[stb];

    //DUALWAN
    if (wanscap_lan) {
        wans_lan_mask = 1U << lan_id_to_port_nr(wans_lanport);
        lan_mask &= ~wans_lan_mask;
    }

    for (unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
        snprintf(prefix, sizeof(prefix), "%d", unit);
        snprintf(nvram_ports, sizeof(nvram_ports), "wan%sports_mask", (unit == WAN_UNIT_FIRST)?"":prefix);

        if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
            nvram_set_int(nvram_ports, wan_mask);
        }
        else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
            nvram_set_int(nvram_ports, wans_lan_mask);
        }
        else
            nvram_unset(nvram_ports);
    }
    
    nvram_set_int("lanports_mask", lan_mask);
}


/**
 * @stb_bitmask:    bitmask of STB port(s)
 *          e.g. bit0 = P0, bit1 = P1, etc.
 */
static void initialize_Vlan(int stb_bitmask)
{
    char portmap[16];
    int wans_lan_vid = 3, wanscap_wanlan = get_wans_dualwan() & (WANSCAP_WAN | WANSCAP_LAN);
    int wanscap_lan = get_wans_dualwan() & WANSCAP_LAN;
    int wans_lanport = nvram_get_int("wans_lanport");
    int sw_mode = sw_mode();
                        
    if (switch_init() < 0)
        return;

    unsigned int value;
    /* Enable VLAN check per port. */
    mt7531_reg_read(REG_ESW_PFC1, &value);
    value |= (0x7f << 16);
    mt7531_reg_write(REG_ESW_PFC1, value);

    if (sw_mode == SW_MODE_AP || sw_mode == SW_MODE_REPEATER)
        wanscap_lan = 0;
        
    if (wanscap_lan && (wans_lanport < 0 || wans_lanport > 4)) {
        _dprintf("%s: invalid wans_lanport %d!\n", __func__, wans_lanport);
        wanscap_lan = 0;
        wanscap_wanlan &= ~WANSCAP_LAN;
    }
            
    if (wanscap_lan && (!(get_wans_dualwan() & WANSCAP_WAN)) && !(!nvram_match("switch_wantag", "none") && !nvram_match("switch_wantag", ""))) 
        wans_lan_vid = 2;

    build_wan_lan_mask(0);
    stb_bitmask = convert_port_bitmask(stb_bitmask);
    lan_mask &= ~stb_bitmask;
    wan_mask |= stb_bitmask;
    _dprintf("%s: LAN/WAN/WANS_LAN portmask %08x/%08x/%08x\n", __func__, lan_mask, wan_mask, wans_lan_mask);

    //VLAN member port: LAN, WANS_LAN
    //LAN: P7, P6, P5, lan_mask    
    __create_port_map(0x40 | lan_mask, portmap);
    mt7531_vlan_set(0, 1, portmap, 1);
    if (wanscap_lan) {
        switch (wanscap_wanlan) {
        case WANSCAP_WAN | WANSCAP_LAN:
            //WANSLAN: P6, wans_lan_mask
            __create_port_map(0x40 | wans_lan_mask, portmap);
            mt7531_vlan_set(3, wans_lan_vid, portmap, wans_lan_vid);
            break;
        case WANSCAP_LAN:
            if (!nvram_match("switch_wantag", "none") && !nvram_match("switch_wantag", "")) {
                //WANSLAN: P6, wans_lan_mask
                __create_port_map(0x40 | wans_lan_mask, portmap);
            } else {
                //WANSLAN: P7, P6, wans_lan_mask
                __create_port_map(0xC0 | wans_lan_mask, portmap);
            }
            mt7531_vlan_set(3, wans_lan_vid, portmap, wans_lan_vid);
            break;
        case WANSCAP_WAN:
            /* Do nothing. */
            break;
        default:
            _dprintf("%s: Unknown WANSCAP %x\n", __func__, wanscap_wanlan); 
        }
    }

    switch_fini();
}

#if defined(RTN14U) || defined(RTAC52U) || defined(RTAC51U) || defined(RTN11P) || defined(RTN54U)
static void fix_up_hwnat_for_wifi(void)
{
    int i, j, m, r, v, isp_profile_hwnat_not_safe = 0;
    char portmap[10];
    char bss[] = "wl0.1_bss_enabledXXXXXX";
    char mode_x[] = "wl0_mode_xXXXXXX";
    struct wifi_if_vid_s w = {
#if defined(RTAC52U) || defined(RTAC51U) || defined(RTN54U)
        .wl_vid = { 21, 43 },       /* DP_RA0  ~ DP_RA3:  21, 22, 23, 24;   DP_RAI0  ~ DP_RAI3:  43, 44, 45, 46 */
        .wl_wds_vid = { 37, 59 },   /* DP_WDS0 ~ DP_WDS3: 37, 38, 39, 40;   DP_WDSI0 ~ DP_WDSI3: 59, 60, 61, 62 */
#elif defined(RTN14U) || defined(RTN11P)
        .wl_vid = { 21, -1 },       /* DP_RA0  ~ DP_RA3:  21, 22, 23, 24;   DP_RAI0  ~ DP_RAI3:  43, 44, 45, 46 */
        .wl_wds_vid = { 37, -1 },   /* DP_WDS0 ~ DP_WDS3: N/A;              DP_WDSI0 ~ DP_WDSI3: N/A */
#else
#error Define VLAN ID of WiFi interface!
#endif
    };

    if (nvram_match("switch_wantag", "none") || nvram_match("switch_wantag", "")) {
        nvram_unset("isp_profile_hwnat_not_safe");
        return;
    }

    if (switch_init() < 0)
        return;

    /* Create VLANs on P6 and P7 for WiFi interfaces to make sure VLAN ID of skbs
     * that come from WiFi interface and are injected to PPE is not swapped as zero.
     * This is used to workaround VirIfIdx=0 problem.
     */
    strlcpy(portmap, "00000011", sizeof(portmap));  /* P6, P7 */
    /* Initial state, 2G/5G interface only. */
    for (i = 0; i <= 1; ++i) {
        if ((v = w.wl_vid[i]) <= 0)
            continue;

        if ((r = mt7531_vlan_set(-1, v, portmap, v)) != 0)
            isp_profile_hwnat_not_safe = 1;
        for (j = 1; j <= 3; ++j)
            mt7531_vlan_unset(v + j);

        if ((v = w.wl_wds_vid[i]) <= 0)
            continue;
        for (j = 0; j <= 3; ++j, ++v)
            mt7531_vlan_unset(v);
    }

    /* 2G/5G guest network i/f */
    for (i = 0; !isp_profile_hwnat_not_safe && i <= 1; ++i) {
        if ((v = w.wl_vid[i]) <= 0)
            continue;
        
        snprintf(mode_x, sizeof(mode_x), "wl%d_mode_x", i);
        m = nvram_get_int(mode_x);  /* 1: WDS only */
        for (j = 1; !isp_profile_hwnat_not_safe && j <= 3; ++j) {
            snprintf(bss, sizeof(bss), "wl%d.%d_bss_enabled", i, j);
            if (m == 1 || !nvram_get_int(bss))
                continue;

            v++;
            if ((r = mt7531_vlan_set(-1, v, portmap, v)) != 0)
                isp_profile_hwnat_not_safe = 1;
        }       
    }

    /* 2G/5G WDS i/f */
    for (i = 0; !isp_profile_hwnat_not_safe && i <= 1; ++i) {
        if ((v = w.wl_wds_vid[i]) <= 0)
            continue;

        snprintf(mode_x, sizeof(mode_x), "wl%d_mode_x", i);
        m = nvram_get_int(mode_x);
        if (m != 1 && m != 2)
            continue;
        for (j = 0; !isp_profile_hwnat_not_safe && j <= 3; ++j, ++v) {
            if ((r = mt7531_vlan_set(-1, v, portmap, v)) != 0)
                isp_profile_hwnat_not_safe = 1;
        }
    }

    nvram_set_int("isp_profile_hwnat_not_safe", isp_profile_hwnat_not_safe);

    switch_fini();
}
#else
static inline void fix_up_hwnat_for_wifi(void) { }
#endif

static void set_Vlan_untag(int idx, int untag_mask);
/**
 * Create VLAN for LAN and/or WAN in accordance with bitmask parameter.
 * @bitmask:
 *  bit15~bit0:     member port bitmask.
 *  bit0:       RT-N56U port0, LAN4
 *  bit1:       RT-N56U port1, LAN3
 *  bit2:       RT-N56U port2, LAN2
 *  bit3:       RT-N56U port3, LAN1
 *  bit4:       RT-N56U port4, WAN
 *  bit8:       RT-N56U port8, LAN_CPU port
 *  bit9:       RT-N56U port9, WAN_CPU port
 *  bit31~bit16:    untag port bitmask.
 *  bit16:      RT-N56U port0, LAN4
 *  bit17:      RT-N56U port1, LAN3
 *  bit18:      RT-N56U port2, LAN2
 *  bit19:      RT-N56U port3, LAN1
 *  bit20:      RT-N56U port4, WAN
 *  bit24:      RT-N56U port8, LAN_CPU port
 *  bit25:      RT-N56U port9, WAN_CPU port
 * First Ralink-based model is RT-N56U.
 * Convert RT-N56U-specific bitmask to physical port of your model,
 * base on relationship between physical port and visual WAN/LAN1~4 of that model first.
 */
static void create_Vlan(int bitmask)
{
    char untag_portmap[16];
    unsigned int value, untag_mask = 0, pvid_mask = 0;
    int idx;    
    char portmap[16];
    int vid = nvram_get_int("vlan_vid") & 0xFFF;
    int prio = nvram_get_int("vlan_prio") & 0x7;
    int mbr = bitmask & 0xffff;
    int untag = (bitmask >> 16) & 0xffff;
    int i, mask;

    if (switch_init() < 0)
        return;
    /* Set PVID & VLAN member port */
    strlcpy(portmap, "00000000", sizeof(portmap));
    strlcpy(untag_portmap, "00000000", sizeof(untag_portmap));
    /* Convert port mapping */
    for (i = 0; i < NR_WANLAN_PORT; i++) {
        mask = (1 << i);
        if (mbr & mask)
            portmap[switch_port_mapping[i]] = '1';
    }
    for (i = 0; i < NR_WANLAN_PORT; i++) {
        mask = (1 << i);
        if (untag & mask)
            untag_portmap[switch_port_mapping[i]] = '1';
    }

    if (mbr & 0x200) {
        portmap[CPU_PORT]='1';
        /* Set WAN VID */
        mt7531_vlan_set(1, vid, portmap, vid);
        /* Tag or untag */
        for (i = 0; i < 7 ; i++) {
            if (i != CPU_PORT && i != WAN_PORT && i != LAN4_PORT && i != 7) // Skip CPU/WAN
                untag_mask += (1 << i);
        }
        set_Vlan_untag(1, untag_mask);
    }
    else {

        if (mt7531_vlan_set(-1, vid, portmap, vid) < 0)
            return;
        idx = find_vlan_slot(vid, &value); // Just get idx.

        if (idx < 0)
            return; // It was not supposed to happen.

        /* Rewrite valid member bits */
        mbr = 0;
        for (i = 0; i < 8; i++)
            mbr += (*(portmap + i) - '0') * (1 << i);
        untag = 0;
        for (i = 0; i < 8; i++)
            untag += (*(untag_portmap + i) - '0') * (1 << i);
        for(i = 0; i < NR_WANLAN_PORT; i++) //LAN port only
        {
            if (i == WAN_PORT)
                continue;

            mask = (1 << i);
            if (mbr & mask) {
                if (untag & mask) { // set untag mask
                    untag_mask += (1 << i);
                    pvid_mask += (1 << i); // set pvid mask
                }
            }
        }
        if (pvid_mask != 0) {
            set_Port_PVID(vid, pvid_mask);
            set_Port_PRIO(prio, pvid_mask);
        }
        if (untag_mask != 0)
            set_Vlan_untag(idx, untag_mask);

        fix_up_hwnat_for_wifi();
    }
    switch_fini();
}

static void is_singtel_mio(int is)
{
    int i;
    unsigned int value;

    if (!is)
        return;

    if (switch_init() < 0)
        return;

    for (i = 0; i < NR_WANLAN_PORT; i++) { //WAN/LAN, admit all frames
        mt7531_reg_read((REG_ESW_PORT_PVC_P0 + 0x100*i), &value);
        value &= 0xfffffffc;
        mt7531_reg_write((REG_ESW_PORT_PVC_P0 + 0x100*i), value);
    }

    switch_fini();
}

static void get_mt7531_esw_WAN_Speed(unsigned int *speed)
{
    unsigned int v = 0;

    if (!speed)
        return;

    *speed = 0;
    v = rtkswitch_WanPort_phySpeed();
    switch (v) {
        case 0:
            *speed = 10;
            break;
        case 1:
            *speed = 100;
            break;
        case 2:
            *speed = 1000;
            break;  
    }

    return;
}

static void link_down_up_mt7531_PHY(unsigned int mask, int status, int inverse)
{
    int i;
    char idx[2];
    char value[5] = "3300"; //power up PHY
    unsigned int m;

    if (!status)        //power down PHY
        value[1] = '9';

    for (i = 0, m = mask; m && i < NR_WANLAN_PORT; ++i, m >>= 1) {
        if (!(m & 1))
            continue;
        snprintf(idx, sizeof(idx), "%d", i);
        eval("mii_mgr", "-s", "-p", idx, "-r", "0", "-v", value);
        _dprintf("mii_mgr -s -p %s -r 0 -v %s \n", idx, value);
    }

    return;
}

void set_mt7531_esw_broadcast_rate(int bsr)
{
#if 0
    int i;
    unsigned int value;
#endif

    if ((bsr < 0) || (bsr > 255))
        return;
    
    if (switch_init() < 0)
        return;

    printf("set broadcast strom control rate as: %d\n", bsr);
#if 0
    for (i = 0; i < NR_WANLAN_PORT; i++) {
        mt7620_reg_read((REG_ESW_PORT_BSR_P0 + 0x100*i), &value);
        value |= 0x1 << 31; //STRM_MODE=Rate-based
        value |= (bsr << 16) | (bsr << 8) | bsr;
        mt7620_reg_write((REG_ESW_PORT_BSR_P0 + 0x100*i), value);
    }
#endif
    switch_fini();
}

static void set_Vlan_untag(int idx, int untag_mask)
{
    _dprintf("[%s][%d] idx = %d, untag_mask = %d\n", __func__, __LINE__, idx, untag_mask);
    unsigned int value, mask = 0;

    if (switch_init() < 0)
        return;

    /* Set POC2 use untag enable bitmap in Vlan table. */
    mt7531_reg_read(REG_ESW_POC2, &value);
    value &= 0xffff7fff;
    value += 0x8000;

    /* Set untag bitmap in VLAN UNTAG Block 0-3 */
    mt7531_reg_read(REG_ESW_VUB0 + 4*(idx / 4), &value);

    if (idx % 4 == 0) {
        value &= 0xffffff80;
        value |= untag_mask;
    }
    else if (idx % 4 == 1) {
        value &= 0xfffc07f;
        untag_mask <<= 7;
        value += untag_mask;
    }
    else if (idx % 4 == 2) {
        value &= 0xffe03fff;
        untag_mask <<= 14;
        value += untag_mask;    
    }
    else {
        value &= 0xf013ffff;
        untag_mask <<= 2;    
        value += untag_mask;
    }

    mt7531_reg_write(REG_ESW_VUB0 + 4*(idx / 4), value);

    switch_fini();
}

/**
 * Configure LAN/WAN partition base on generic IPTV type.
 * @type:
 * 0:  Default.
 * 1:  LAN1
 * 2:  LAN2
 * 3:  LAN3
 * 4:  LAN4
 * 5:  LAN1+LAN2
 * 6:  LAN3+LAN4
 */
static void config_mt7531_esw_LANWANPartition(int type)
{
    char portmap[16];
    int i, v, wans_lan_vid = 3, wanscap_wanlan = get_wans_dualwan() & (WANSCAP_WAN | WANSCAP_LAN);
    int wanscap_lan = get_wans_dualwan() & WANSCAP_LAN;
    int wans_lanport = nvram_get_int("wans_lanport");
    int sw_mode = sw_mode();
    unsigned int m;
    unsigned int pvid = 0, vmsc = 0;
    unsigned int untag_portmap = 0;
    
    if (switch_init() < 0)
        return;   

    if (sw_mode == SW_MODE_AP || sw_mode == SW_MODE_REPEATER)
        wanscap_lan = 0;

    if (wanscap_lan && (wans_lanport < 0 || wans_lanport > 4)) {
        _dprintf("%s: invalid wans_lanport %d!\n", __func__, wans_lanport);
        wanscap_lan = 0;
        wanscap_wanlan &= ~WANSCAP_LAN;
    }

    if (wanscap_lan && !(get_wans_dualwan() & WANSCAP_WAN))
        wans_lan_vid = 2;

    /* set priority flow control */
    if (lan_mask != 0x1f)
        mt7531_reg_write(REG_ESW_PFC1, 0x405555);
    else
        mt7531_reg_write(REG_ESW_PFC1, 0x005555);

    for (i = 0, m = 1; i < NR_WANLAN_PORT + 1; ++i, m <<= 1) {
        if (lan_mask & m)
            v = 1;  //LAN
        else if (wanscap_lan && (wans_lan_mask & m))
            v = wans_lan_vid;   //WANSLAN
        else
            v = 2;  //WAN
        //_dprintf("%s: P%d PVID %d\n", __func__, i, v);

        if (i % 2) {
            if (i != 5)
                pvid += (v << 12);
            else
                pvid += (0x1 << 12);
            _dprintf("%s: PVIDC%d - %08x\n", __func__, i/2,  pvid);
            mt7531_reg_write(REG_ESW_PVIDC0 + ((i / 2) * 0x4), pvid);
            pvid = 0;   /* reset for next */
        }
        else
            pvid += v; 
        
    }

    /* set VLAN Identifier */
    mt7531_reg_write(REG_ESW_VLANI0, 0x2001);
    if (wans_lan_mask)
        mt7531_reg_write(REG_ESW_VLANI1, 0x4003);  

    /* VLAN Member Port Configuration */
    //mt7628_reg_write(REG_ESW_VMSC0, 0xffff506f);
    vmsc = 0;
    for (i = 0, m = 1; i < NR_WANLAN_PORT; ++i, m <<= 1) {
        if (lan_mask & m)
            vmsc += (0x1 << i);
    }
    if (lan_mask)
        vmsc += (0x1 << CPU_PORT);  /* port 6 */

    for (i = 0, m = 1; i < NR_WANLAN_PORT; ++i, m <<= 1) {
        if (wan_mask & m) 
            vmsc += (0x1 << (0x8 + i));
    }
    if (wan_mask)
        vmsc += (0x1 << (0x8 + CPU_PORT));  /* port 6 */

    for (i = 0, m = 1; i < NR_WANLAN_PORT; ++i, m <<= 1) {
        if (wans_lan_mask & m)
            vmsc += (0x1 << (0x10 + i));
    }
    if (wans_lan_mask)
            vmsc += (0x1 << (0x10 + CPU_PORT)); /* port 6 */

    vmsc += 0xff000000; // Should not modify VLAN 3 Member Port value. Set VLAN 3 Member Port to default 0xFF.
    _dprintf("%s: VMSC - %08x\n", __func__, vmsc);
    mt7531_reg_write(REG_ESW_VMSC0, vmsc);

    /* set Port Control */
    if (lan_mask != 0x1f) {
        mt7531_reg_write(REG_ESW_POC2, 0xff3f);
        set_Vlan_untag(0, lan_mask);
        set_Vlan_untag(1, wan_mask);
#if defined(RTCONFIG_DUALWAN)
        set_Vlan_untag(2, wans_lan_mask);
#endif
    }
    else {
        mt7531_reg_write(REG_ESW_POC2, 0xff7f);
        set_Vlan_untag(0, lan_mask);
        set_Vlan_untag(1, wan_mask);
#if defined(RTCONFIG_DUALWAN)
        set_Vlan_untag(2, wans_lan_mask);
#endif
    }

#if 0
    //LAN/WAN ports as security mode
    for (i = 0; i < 6; i++)
        mt7531_reg_write((REG_ESW_PORT_PCR_P0 + 0x100*i), 0xff0003);

    //LAN/WAN ports as transparent port
    for (i = 0; i < 6; i++)
        mt7531_reg_write((REG_ESW_PORT_PVC_P0 + 0x100*i), 0x810000c2);  //transparent port, admit untagged frames

    
    //set CPU/P7 port as user port
    mt7531_reg_write((REG_ESW_PORT_PVC_P0 + 0x100*CPU_PORT), 0x81000000);   //port6, user port, admit all frames
    mt7531_reg_write((REG_ESW_PORT_PVC_P0 + 0x100*P7_PORT), 0x81000000);    //port7, user port, admit all frames

    mt7531_reg_write((REG_ESW_PORT_PCR_P0 + 0x100*CPU_PORT), 0x20ff0003);   //port6, Egress VLAN Tag Attribution=tagged
    mt7531_reg_write((REG_ESW_PORT_PCR_P0 + 0x100*P7_PORT), 0x20ff0003);    //port7, Egress VLAN Tag Attribution=tagged
    
    build_wan_lan_mask(type);
    _dprintf("%s: LAN/WAN/WANS_LAN portmask %08x/%08x/%08x\n", __func__, lan_mask, wan_mask, wans_lan_mask);

    //set PVID
    for (i = 0, m = 1; i < NR_WANLAN_PORT; ++i, m <<= 1) {
        if (lan_mask & m)
            v = 1;  //LAN
        else if (wanscap_lan && (wans_lan_mask & m))
            v = wans_lan_vid;   //WANSLAN
        else
            v = 2;  //WAN
        _dprintf("%s: P%d PVID %d\n", __func__, i, v);
        mt7531_reg_write((REG_ESW_PORT_PPBV1_P0 + 0x100*i), 0x10000 | v);
    }

    mt7531_reg_write((REG_ESW_PORT_PPBV1_P0 + 0x100*P5_PORT), 0x10001);
    //VLAN member port: WAN, LAN, WANS_LAN
    //LAN: P7, P6, P5, lan_mask
    __create_port_map(0xE0 | lan_mask, portmap);
    mt7531_vlan_set(0, 1, portmap, 0);
    if (sw_mode == SW_MODE_ROUTER) {
        switch (wanscap_wanlan) {
        case WANSCAP_WAN | WANSCAP_LAN:
            //WAN: P7, P6, wan_mask
            __create_port_map(0xC0 | wan_mask, portmap);
            mt7531_vlan_set(1, 2, portmap, 0);
            //WANSLAN: P6, wans_lan_mask
            __create_port_map(0x40 | wans_lan_mask, portmap);
            mt7531_vlan_set(2, wans_lan_vid, portmap, 0);
            break;
        case WANSCAP_LAN:
            //WANSLAN: P7, P6, wans_lan_mask
            __create_port_map(0xC0 | wans_lan_mask, portmap);
            mt7531_vlan_set(1, 2, portmap, 0);
            break;
        case WANSCAP_WAN:
            //WAN: P7, P6, wan_mask
            __create_port_map(0xC0 | wan_mask, portmap);
            mt7531_vlan_set(1, 2, portmap, 0);
            break;
        default:
            _dprintf("%s: Unknown WANSCAP %x\n", __func__, wanscap_wanlan);
        }
    }
#endif
    switch_fini(); 
    eval("swconfig", "dev", MII_IFNAME, "set", "enable_vlan", "1"); // enable vlan
    eval("swconfig", "dev", MII_IFNAME, "set", "apply");    // apply changes
}

int rtkswitch_ioctl(int val, int val2)
{
    unsigned int value2 = 0;
    int i, max_wan_unit = 0;

	switch (val) {
	case 0:		/* check WAN port phy status */
	    value2 = rtkswitch_wanPort_phyStatus(-1);
        printf("WAN link status : %u\n", value2);
        break;
    case 3:		/* check LAN ports phy status */
        value2 = rtkswitch_lanPorts_phyStatus();
        printf("LAN link status : %u\n", value2);
        break;
	case 8:
        config_mt7531_esw_LANWANPartition(val2);
        break;
    case 13:
        get_mt7531_esw_WAN_Speed(&value2);
        printf("WAN speed : %u Mbps\n", value2);
        break;
    case 14:    /* power up LAN port(s) */
        rtkswitch_LanPort_linkUp();
        break;
    case 15:    /* power down LAN port(s) */
        rtkswitch_LanPort_linkDown();
        break;
    case 16:    /* power up all ports */
        rtkswitch_AllPort_linkUp();
        break;
    case 17:    /* power down all ports */
        rtkswitch_AllPort_linkDown();
        break;        
    case 21:
        break;
	case 22:
        break;
	case 23:
        break;
	case 24:
        break;
	case 25:
        set_mt7531_esw_broadcast_rate(val2);
        break;
    case 27:
        reset_mt7531_esw();
        break;       
	case 36:	/* Set Vlan VID. */
        set_Vlan_VID(val2);
        break;
    case 37:	/* Set Vlan PRIO. */
        set_Vlan_PRIO(val2);
        break;
    case 38:
        initialize_Vlan(val2);
        break;
    case 39:
        create_Vlan(val2);
        break;
    case 40:
        is_singtel_mio(val2);
        break;
    case 50:
        fix_up_hwnat_for_wifi();
        break;
	case 114:	/* power up WAN port(s) */
        rtkswitch_WanPort_linkUp();
        break;
	case 115:	/* power down WAN port(s) */
		rtkswitch_WanPort_linkDown();
        break;
    case 200:   /* set LAN port number that is used as WAN port */
        /* Nothing to do, nvram_get_int("wans_lanport ") is enough. */
        break;        
	default:
		printf("wrong ioctl cmd: %d\n", val);
		break;
	}

	return 0;
}

int config_rtkswitch(int argc, char *argv[])
{
	int val;
	int val2 = 0;
	char *cmd = NULL;
	char *cmd2 = NULL;

	if (argc >= 2)
		cmd = argv[1];
	else
		return -1;
	if (argc >= 3)
		cmd2 = argv[2];

	val = (int) strtol(cmd, NULL, 0);
	if (cmd2)
		val2 = (int) strtol(cmd2, NULL, 0);

	return rtkswitch_ioctl(val, val2);
}

typedef struct {
	unsigned int idx;
	unsigned int value;
} asus_gpio_info;

#define RALINK_MT7629_GPIO_NUM_BASE 	433
#define RALINK_MT7629_GPIO_DEV			"/sys/class/gpio"
#define RALINK_MT7629_GPIO_EXPORT		"/sys/class/gpio/export"

int ralink_gpio_write_bit(int idx, int value)
{
	int fd = 0;
	int pin = idx;
	char gpio_fname[128], b[128];

	memset(gpio_fname, 0, sizeof(gpio_fname));
	snprintf(gpio_fname, sizeof(gpio_fname), "%s/gpio%d/value", RALINK_MT7629_GPIO_DEV, (pin + RALINK_MT7629_GPIO_NUM_BASE));

	fd = open(gpio_fname, O_WRONLY);
	if (fd < 0)
		goto ralink_gpio_write_bit_error;
	close(fd);

	memset(b, 0, sizeof(b));
	snprintf(b, sizeof(b), "echo %d >%s", value, gpio_fname);
	system(b);
	return 0;

ralink_gpio_write_bit_error:
	if (fd) close(fd);
	return -1;
}

int ralink_gpio_read_bit(int idx)
{
	int fd = 0;
	int pin = idx;
	int value = 0;
	char gpio_fname[128], b[32];

	memset(gpio_fname, 0, sizeof(gpio_fname));
	snprintf(gpio_fname, sizeof(gpio_fname), "%s/gpio%d/value", RALINK_MT7629_GPIO_DEV, (pin + RALINK_MT7629_GPIO_NUM_BASE));

	fd = open(gpio_fname, O_RDONLY);
	if (fd < 0)
	{
		goto ralink_gpio_read_bit_error;
	}

	if (read(fd, b, sizeof(b)) <= 0)
	{
		goto ralink_gpio_read_bit_error;
	}

	close(fd);
	value = atoi(b); 	// value == 0: pressed
	return value;

ralink_gpio_read_bit_error:
	if (fd) close(fd);
	return -1;
}

int ralink_gpio_init(unsigned int idx, int dir)
{
	int fd = 0;
	int pin = idx;
	char gpio_fname[128], s[128];

	memset(s, 0, sizeof(s));
	snprintf(s, sizeof(s), "echo %d >%s", pin + RALINK_MT7629_GPIO_NUM_BASE, RALINK_MT7629_GPIO_EXPORT);
	system(s);

	memset(gpio_fname, 0, sizeof(gpio_fname));
	snprintf(gpio_fname, sizeof(gpio_fname), "%s/gpio%d/direction", RALINK_MT7629_GPIO_DEV, (pin + RALINK_MT7629_GPIO_NUM_BASE));

	fd = open(gpio_fname, O_RDONLY);
	if (fd < 0)
	{
		goto ralink_gpio_init_error;
	}

	close(fd);

	if (dir == GPIO_DIR_OUT)
	{		
		memset(s, 0, sizeof(s));
		snprintf(s, sizeof(s), "echo out >%s", gpio_fname);
		system(s);
	}

	return 0;

ralink_gpio_init_error:
	if (fd) close(fd);
	return -1;	
}

// a wrapper to broadcom world
// bit 1: LED_POWER
// bit 6: BTN_RESET
// bit 8: BTN_WPS

uint32_t ralink_gpio_read(void)
{
	uint32_t r = 0;

	if (ralink_gpio_read_bit(RA_BTN_RESET))
		r |= (1 << 6);
	if (ralink_gpio_read_bit(RA_BTN_WPS))
		r |= (1 << 8);

	return r;
}

int ralink_gpio_write(uint32_t bit, int en)
{
	if (bit & (1 << 1)) {
		if (!en)
			ralink_gpio_write_bit(RA_LED_POWER, RA_LED_OFF);
		else
			ralink_gpio_write_bit(RA_LED_POWER, RA_LED_ON);
	}

	return 0;
}

unsigned int rtkswitch_wanPort_phyStatus(int wan_unit)
{
    unsigned int link = 0, speed = 0;
    get_ralink_wan_status(&link, &speed);
    return link;
}


unsigned int rtkswitch_lanPorts_phyStatus(void)
{
    unsigned int value = 0;
    unsigned int i, link = 0, speed = 0;
    
    for (i=0; i<4; i++) {
        get_ralink_lan_status(i, &link, &speed);
        if (link) {
            value = 1;
            break;
        }
    }

    return value;
}

unsigned int rtkswitch_WanPort_phySpeed(void)
{
    unsigned int link = 0, speed = 0;
    get_ralink_wan_status(&link, &speed);
    return speed;
}

int rtkswitch_WanPort_linkUp(void)
{
	//eval("rtkswitch", "114");
    system("mii_mgr -s -p 0 -r 0 -v 3300");
	return 0;
}

int rtkswitch_WanPort_linkDown(void)
{
	//eval("rtkswitch", "115");
    system("mii_mgr -s -p 0 -r 0 -v 3900");
	return 0;
}

int rtkswitch_LanPort_linkUp(void)
{
	//eval("rtkswitch", "14");
    // swconfig dev switch0 port 2 set link "speed 1000 duplex full autoneg on"
    system("swconfig dev switch0 port 0 set link \x22speed 1000 duplex full autoneg on\x22");
    system("swconfig dev switch0 port 1 set link \x22speed 1000 duplex full autoneg on\x22");
    system("swconfig dev switch0 port 2 set link \x22speed 1000 duplex full autoneg on\x22");
    system("swconfig dev switch0 port 3 set link \x22speed 1000 duplex full autoneg on\x22");
    return 0;
}

int rtkswitch_LanPort_linkDown(void)
{
	//eval("rtkswitch", "15");
    //swconfig dev switch0 port 2 set link "speed 1000 duplex full autoneg off"
    system("swconfig dev switch0 port 0 set link \x22speed 1000 duplex full autoneg off\x22");
    system("swconfig dev switch0 port 1 set link \x22speed 1000 duplex full autoneg off\x22");
    system("swconfig dev switch0 port 2 set link \x22speed 1000 duplex full autoneg off\x22");
    system("swconfig dev switch0 port 3 set link \x22speed 1000 duplex full autoneg off\x22");
    return 0;
}

int rtkswitch_AllPort_linkUp(void)
{
	//eval("rtkswitch", "16");
    system("mii_mgr -s -p 0 -r 0 -v 3300");
    system("swconfig dev switch0 port 0 set link \x22speed 1000 duplex full autoneg on\x22");
    system("swconfig dev switch0 port 1 set link \x22speed 1000 duplex full autoneg on\x22");
    system("swconfig dev switch0 port 2 set link \x22speed 1000 duplex full autoneg on\x22");
    system("swconfig dev switch0 port 3 set link \x22speed 1000 duplex full autoneg on\x22");      
    return 0;
}

int rtkswitch_AllPort_linkDown(void)
{
	//eval("rtkswitch", "17");
    system("mii_mgr -s -p 0 -r 0 -v 3900");
    system("swconfig dev switch0 port 0 set link \x22speed 1000 duplex full autoneg off\x22");
    system("swconfig dev switch0 port 1 set link \x22speed 1000 duplex full autoneg off\x22");
    system("swconfig dev switch0 port 2 set link \x22speed 1000 duplex full autoneg off\x22");
    system("swconfig dev switch0 port 3 set link \x22speed 1000 duplex full autoneg off\x22");     
    return 0;
}

int rtkswitch_Reset_Storm_Control(void)
{
	eval("rtkswitch", "21");
	return 0;
}

int rtkswitch_AllPort_phyState(void)
{
    char buf[32];
    const char *portMark = "W0=%C;L1=%C;L2=%C;L3=%C;L4=%C;";
    phyState pS;
    unsigned int port, link = 0, speed = 0;

    pS.link[0] = pS.link[1] = pS.link[2] = pS.link[3] = pS.link[4] = 0;
    pS.speed[0] = pS.speed[1] = pS.speed[2] = pS.speed[3] = pS.speed[4] = 0;

    // lan status 
    for (port = 0; port < 4; port++) {
        if (get_ralink_lan_status(port, &link, &speed) != 0) {
            _dprintf("get_ralink_lan_status() error .\n");
            return -1;
        }

        pS.link[port] = link;
        pS.speed[port] = speed;   
    }

    // wan status 
    if (get_ralink_wan_status(&link, &speed) != 0) {
        _dprintf("get_ralink_wan_status() error .\n");
        return -1;
    }

    pS.link[4] = link;
    pS.speed[4] = speed;

    sprintf(buf, portMark,
            (pS.link[4] == 1) ? (pS.speed[4] == 2) ? 'G' : 'M': 'X',
            (pS.link[0] == 1) ? (pS.speed[0] == 2) ? 'G' : 'M': 'X',
            (pS.link[1] == 1) ? (pS.speed[1] == 2) ? 'G' : 'M': 'X',
            (pS.link[2] == 1) ? (pS.speed[2] == 2) ? 'G' : 'M': 'X',
            (pS.link[3] == 1) ? (pS.speed[3] == 2) ? 'G' : 'M': 'X');

    puts(buf);
    return 0;
}

int get_realtek_wan_bytecount(unsigned long *tx, unsigned long *rx)
{
    char tx_str[64];
    char rx_str[64];

    memset(tx_str, 0, sizeof(tx_str));
    if (f_read("/sys/class/net/eth1/statistics/tx_bytes", tx_str, sizeof(tx_str)) <= 0)
        return -1;
    if (tx) 
        *tx = strtoul(tx_str, NULL, 10);

    memset(rx_str, 0, sizeof(rx_str));
    if (f_read("/sys/class/net/eth1/statistics/rx_bytes", rx_str, sizeof(rx_str)) <= 0)
        return -1;
    if (rx) 
        *rx = strtoul(rx_str, NULL, 10);

    return 0;
}

int get_realtek_lans_bytecount(unsigned long *tx, unsigned long *rx)
{
    char tx_str[64];
    char rx_str[64];

    memset(tx_str, 0, sizeof(tx_str));
    if (f_read("/sys/class/net/eth0/statistics/tx_bytes", tx_str, sizeof(tx_str)) <= 0)
        return -1;
    if (tx)
        *tx = strtoul(tx_str, NULL, 10);

    memset(rx_str, 0, sizeof(rx_str));
    if (f_read("/sys/class/net/eth0/statistics/rx_bytes", rx_str, sizeof(rx_str)) <= 0)
        return -1;
    if (rx)
        *rx = strtoul(rx_str, NULL, 10);

    return 0;
}


#if 0
void usage(char *cmd)
{
	printf("Usage: %s 1 - set (SCK, SD) as (0, 1)\n", cmd);
	printf("       %s 2 - set (SCK, SD) as (1, 0)\n", cmd);
	printf("       %s 3 - set (SCK, SD) as (1, 1)\n", cmd);
	printf("       %s 0 - set (SCK, SD) as (0, 0)\n", cmd);
	printf("       %s 4 - init vlan\n", cmd);
	printf("       %s 5 - set cpu port 0 0\n", cmd);
	printf("       %s 6 - set cpu port 0 1\n", cmd);
	printf("       %s 7 - set cpu port 1 0\n", cmd);
	printf("       %s 8 - set cpu port 1 1\n", cmd);
	printf("       %s 10 - set vlan entry, no cpu port, but mbr\n", cmd);
	printf("       %s 11 - set vlan entry, no cpu port, no mbr\n", cmd);
	printf("       %s 15 - set vlan PVID, no cpu port\n", cmd);
	printf("       %s 20 - set vlan entry, with cpu port\n", cmd);
	printf("       %s 21 - set vlan entry, with cpu port and no cpu port in untag sets\n", cmd);
	printf("       %s 25 - set vlan PVID, with cpu port\n", cmd);
	printf("       %s 26 - set vlan PVID, not set cpu port\n", cmd);
	printf("       %s 90 - accept all frmaes\n", cmd);
	printf("       %s 66 - setup default vlan\n", cmd);
	printf("       %s 61 - setup vlan type1\n", cmd);
	printf("       %s 62 - setup vlan type2\n", cmd);
	printf("       %s 63 - setup vlan type3\n", cmd);
	printf("       %s 64 - setup vlan type4\n", cmd);
	printf("       %s 65 - setup vlan type34\n", cmd);
	printf("       %s 70 - disable multicast snooping\n", cmd);
	printf("       %s 81 - setRtctTesting on port x\n", cmd);
	printf("       %s 82 - getRtctResult on port x\n", cmd);
	printf("       %s 83 - setGreenEthernet x(green, powsav)\n", cmd);
	printf("       %s 84 - setAsicGreenFeature x(txGreen, rxGreen)\n", cmd);
	printf("       %s 85 - getAsicGreenFeature\n", cmd);
	printf("       %s 86 - enable GreenEthernet on port x\n", cmd);
	printf("       %s 87 - disable GreenEthernet on port x\n", cmd);
	printf("       %s 88 - getAsicPowerSaving x\n", cmd);
	printf("       %s 50 - getPortLinkStatus x\n", cmd);
	exit(0);
}
#endif
