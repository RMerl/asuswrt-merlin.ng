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
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <bcmnvram.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <shutils.h>
#include <shared.h>
#include <utils.h>
#include <linux/config.h>
#include <ralink.h>
#include "mtkswitch.h"


enum {

	P5_PORT=5,
	CPU_PORT=6,
	P7_PORT=7,
};

int esw_fd;

/**
 * Create string based portmap base on mask parameter.
 * @mask:	port bit mask.
 * 		bit0: P0, bit1: P1, etc
 * @portmap:	pointer to char array. minimal length is 9 bytes.
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

#if defined(RTCONFIG_RALINK_MT7620)
int mt7620_reg_read(int offset, unsigned int *value)
{
	struct ifreq ifr;
	esw_reg reg;

	if (value == NULL)
		return -1;
	reg.off = offset;
	strncpy(ifr.ifr_name, "eth2", 5);
	ifr.ifr_data = (void*) &reg;
	if (-1 == ioctl(esw_fd, RAETH_ESW_REG_READ, &ifr)) {
		_dprintf("%s: read esw register fail. errno %d (%s)\n", __func__, errno, strerror(errno));
		close(esw_fd);
		return -1;
	}
	*value = reg.val;

	//debug
	//printf("mt7620_reg_read()...offset=%4x, value=%8x\n", offset, *value);

	return 0;
}

int mt7620_reg_write(int offset, int value)
{
	struct ifreq ifr;
	esw_reg reg;

	//debug
	//printf("mt7620_reg_write()..offset=%4x, value=%8x\n", offset, value);

	reg.off = offset;
	reg.val = value;
	strncpy(ifr.ifr_name, "eth2", 5);
	ifr.ifr_data = (void*) &reg;
	if (-1 == ioctl(esw_fd, RAETH_ESW_REG_WRITE, &ifr)) {
		perror("ioctl");
		close(esw_fd);
		exit(0);
	}
	return 0;
}

#endif

//vlan set
static void write_VTCR(unsigned int value)
{
	int i;
	unsigned int check;

	value |= 0x80000000;
#if defined(RTCONFIG_RALINK_MT7620)
	mt7620_reg_write(REG_ESW_VLAN_VTCR, value);
#endif	
	for (i = 0; i < 20; i++) {
#if defined(RTCONFIG_RALINK_MT7620)	   
		mt7620_reg_read(REG_ESW_VLAN_VTCR, &check);
#endif		
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
 * 	>0:	Find vlan slot by vid.
 * 	<=0:	Find first available vlan slot.
 * @vawd1:	pointer to a unsigned integer.
 * 		If vlan slot found and vawd1 is not null, save VAWD1 value here.
 * @return:
 * 	0~15:	vlan slot index.
 * 	<0:	not found
 */
static int find_vlan_slot(int vid, unsigned int *vawd1)
{
	int idx;
	unsigned int i, r, v, value;

#if defined(RTCONFIG_RALINK_MT7620)
	i = 0;				/* search vlan index from 0 */
#endif

	for(idx = -1; idx < 0 && i < 16; ++i) {
#if defined(RTCONFIG_RALINK_MT7620)
		if ((r = mt7620_reg_read(REG_ESW_VLAN_ID_BASE + 4*(i>>1), &value))) {
			_dprintf("read VTIM1~8, i %d offset %x fail. (r = %d)\n", i, REG_ESW_VLAN_ID_BASE + 4*(i>>1), r);
			continue;
		}

		if (!(i&1))
			v = value & 0xfff;
		else
			v = (value >> 12) & 0xfff;		

		if ((vid <= 0 && v != (i + 1)) && (vid > 0 && v != vid))
			continue;
#endif		
		value = (0x0 << 12) + i; //read VAWD#
		write_VTCR(value);
#if defined(RTCONFIG_RALINK_MT7620)		
		mt7620_reg_read(REG_ESW_VLAN_VAWD1, &value);
		if ((vid <= 0 ) && v == (i + 1) && !(value & 1))	/* find available vlan slot */
#endif		
			idx = i;
		else if (vid > 0 && v == vid)				/* find vlan slot by vid */
			idx = i;

		if (idx >= 0 && vawd1)
			*vawd1 = value;
	}

	return idx;
}

/**
 * Set a VLAN
 * @idx:	VLAN table index
 *  >= 0:	specify VLAN table index.
 *  <  0:	find available VLAN table.
 * @vid:	VLAN ID
 * @portmap:	Port member. string length must greater than or equal to 8.
 * 		Only '0' and '1' are accepted.
 * 		First digit means port 0, second digit means port1, etc.
 * 		P0, P1, P2, P3, P4, P5, P6, P7
 * @stag:	Service tag
 * @return
 * 	0:	success
 *     -1:	no vlan entry available
 *     -2:	invalid parameter
 */
#if defined(RTCONFIG_RALINK_MT7620)
int mt7620_vlan_set(int idx, int vid, char *portmap, int stag)
#endif
{
	unsigned int i, mbr, value, vawd1;
   
	if (idx < 0) { //auto   
		if ((idx = find_vlan_slot(vid, &vawd1)) < 0 && (idx = find_vlan_slot(-1, &vawd1)) < 0) {
			_dprintf("%s: no empty vlan entry for vid %d portmap %s stag %d!\n",
				__func__, vid, portmap, stag);
			return -1;
		}
	}

	_dprintf("%s: idx=%d, vid=%d, portmap=%s, stag=%d\n", __func__, idx, vid, portmap, stag);

	//set vlan identifier
#if defined(RTCONFIG_RALINK_MT7620)
	mt7620_reg_read(REG_ESW_VLAN_ID_BASE + 4*(idx/2), &value);	
	if (idx % 2 == 0) {
		value &= 0xfff000;
		value |= vid;
	}
	else {
		value &= 0xfff;
		value |= (vid << 12);
	}

	mt7620_reg_write(REG_ESW_VLAN_ID_BASE + 4*(idx/2), value);
#endif	

	//set vlan member
	mbr = 0;
	for (i = 0; i < 8; i++)
		mbr += (*(portmap + i) - '0') * (1 << i);
	value = (mbr << 16);		//PORT_MEM
	value |= (1 << 30);		//IVL
	value |= (1 << 27);		//COPY_PRI
	value |= ((stag & 0xfff) << 4);	//S_TAG
	value |= 1;			//VALID
#if defined(RTCONFIG_RALINK_MT7620)	
	mt7620_reg_write(REG_ESW_VLAN_VAWD1, value);
	value = (0x80001000 + idx); //w_vid_cmd
#endif	
	write_VTCR(value);

	return 0;
}
//end vlan set 


//fix mac clone issue
static void set_user_mode()
{

	char portmap[16];
	unsigned int _mask = 0xe0; // P5/6/7

	if (switch_init() < 0)
		return;

	//set P5/CPU/P7 port as user port
#if defined(RTCONFIG_RALINK_MT7620)
	mt7620_reg_write((REG_ESW_PORT_PVC_P0 + 0x100*P5_PORT), 0x81000000);	//port5, user port, admit all frames
	mt7620_reg_write((REG_ESW_PORT_PVC_P0 + 0x100*CPU_PORT), 0x81000000);	//port6, user port, admit all frames
	mt7620_reg_write((REG_ESW_PORT_PVC_P0 + 0x100*P7_PORT), 0x81000000);	//port7, user port, admit all frames

	//set port base egress vlan tagEgress 
	mt7620_reg_write((REG_ESW_PORT_PCR_P0 + 0x100*P5_PORT), 0x20ff0003);	//port5, VLAN Tag Attribution=tagged
	mt7620_reg_write((REG_ESW_PORT_PCR_P0 + 0x100*CPU_PORT), 0x20ff0003);	//port6, Egress VLAN Tag Attribution=tagged
	mt7620_reg_write((REG_ESW_PORT_PCR_P0 + 0x100*P7_PORT), 0x20ff0003);	//port7, Egress VLAN Tag Attribution=tagged
#endif

	//set vlan id member
	//VLAN member port  P5/6/7
	__create_port_map(_mask, portmap);
#if defined(RTCONFIG_RALINK_MT7620)
	mt7620_vlan_set(1, 2, portmap, 0);
#endif

	switch_fini();

}

/*set vlan id and associated member on port 5/6/7*/
static void set_vlan_mbr(int vid)
{
	char portmap[16];
	unsigned int _mask = 0xe0; // P5/6/7

	if (switch_init() < 0)
		return;
	//set vlan id member
	//VLAN member port  P5/6/7
	__create_port_map(_mask, portmap);
#if defined(RTCONFIG_RALINK_MT7620)
	printf("add to vid id member %d\n", vid);
	mt7620_vlan_set(-1, vid, portmap, vid);
#endif

	switch_fini();
}

/* keep vlan tag for egress packet on port 0~7*/
static void keep_vtag_on(int port)
{

	int offset=0, value=0;

	if (switch_init() < 0)
		return;

	offset = 0x2004 + port * 0x100;
	mt7620_reg_read(offset, &value);
	value |= 0x20000000;
	mt7620_reg_write(offset, value);
	printf("tag vid at port %d\n", port);

	switch_fini();

}

#if defined(RTCONFIG_RALINK_MT7620)
int mt7620_ioctl(int val, int val2)
#endif
{
	printf("mtkswitch!!!=%d\n",val);

	switch (val) {

	case 0:
		set_user_mode();
		break;

	case 1:
		set_vlan_mbr(val2);
		break;

	case 2:
		keep_vtag_on(val2);
		break;

	default:
		printf("wrong ioctl cmd: %d\n", val);
	}

	return 0;
}

int config_mtkswitch(int argc, char *argv[])
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

#if defined(RTCONFIG_RALINK_MT7620)
	return mt7620_ioctl(val, val2);
#endif

}

