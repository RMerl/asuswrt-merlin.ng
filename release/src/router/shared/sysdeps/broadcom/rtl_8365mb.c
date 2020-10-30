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
#include <errno.h>

#include <linux/major.h>
#include <rtk_switch.h>
#include <rtk_types.h>

#define RTKSWITCH_DEV	"/dev/rtkswitch"

#define CASEID(a)	#a
char *rtk_switch_cmds[] = RTK_SWITCH_CMDS;
#undef CASEID
unsigned int rtk_cmds_pa[MAX_REQ];

void usage(char *cmd);

int rtkswitch_ioctl(int val, int val2, int val3)
{
	int fd;
	int value = 0;
	rtk_asic_t asics;
	void *p = NULL;

	fd = open(RTKSWITCH_DEV, O_RDONLY);
	if (fd < 0) {
		perror(RTKSWITCH_DEV);
		return -1;
	}

	switch (val) {
	/* w/ no options */
	case INIT_SWITCH:
	case INIT_SWITCH_UP:
	case GET_EXT_TXRXDELAY:
	case GET_LANPORTS_LINK_STATUS:
	case BAD_ADDR_X:
	case POWERUP_LANPORTS:
	case POWERDOWN_LANPORTS:
	case GET_RTK_PHYSTATES:
	case SOFT_RESET:
	case GET_CPU:
	case GET_AWARE:
	case GET_GREEN_ETHERNET:
		p = NULL;
		break;

	/* w/ 1 option */
	case RESET_PORT:
	case GET_PORT_STAT:
	case GET_PORT_SPEED:
	case SET_EXT_TXDELAY:
	case SET_EXT_RXDELAY:
	case GET_REG:
	case SET_EXT_MODE:
	case SET_CPU:
	case GET_TMODE:
	case GET_PHY_REG9:
	case SET_GREEN_ETHERNET:
		p = (void*)&value;
		value = (unsigned int)val2;
		break;

	/* w/ 2 options */
	case TEST_REG:
	case SET_REG:
	case SET_TMODE:
	case SET_PHY_REG9:
		p = (void*)&asics;
		asics.rtk_reg = (rtk_uint32)val2;
		asics.rtk_val = (rtk_uint32)val3;
		break;

	default:
		usage("rtkswitch");
		close(fd);
		return 0;
	}

	if (ioctl(fd, val, p) < 0) {
		perror("rtkswitch ioctl");
		close(fd);
		return -1;
	}

	close(fd);
	return 0;
}

int config_rtkswitch(int argc, char *argv[])
{
	int val;
	int val2 = 0;
	int val3 = 0;
	char *cmd = NULL;
	char *cmd2 = NULL;
	char *cmd3 = NULL;

	if (argc >= 2)
		cmd = argv[1];
	else {
		usage("rtkswitch");
		return -1;
	}

	if (argc >= 3) {
		cmd2 = argv[2];
		if(argc > 3)
			cmd3 = argv[3];
	}

	val = (int) strtol(cmd, NULL, 0);
	if (cmd2)
		val2 = (int) strtol(cmd2, NULL, 0);
	if (cmd3)
		val3 = (int) strtol(cmd3, NULL, 0);

	return rtkswitch_ioctl(val, val2, val3);
}

int ext_rtk_phy_mib(int port, rtk_stat_port_cntr_t *pPort_cntrs) {
	int fd;
	int *p = NULL;
	rtk_stat_port_cntr_t Port_cntrs;

	if ((port < 0) || !pPort_cntrs)
		return -1;

	fd = open("/dev/rtkswitch", O_RDONLY);
	if (fd < 0) {
		perror("/dev/rtkswitch");
	} else {
		memset(&Port_cntrs, 0, sizeof(Port_cntrs));
		p = (int *) &Port_cntrs;
		*p = port;
		if (ioctl(fd, GET_PORT_STAT, &Port_cntrs) < 0) {
			perror("rtkswitch ioctl");
			close(fd);
			return -1;
		} else {
			/*fprintf(stderr, "pPort_cntrs=%p, tx_bytes=%llu, rx_bytes=%llu, tx_packets=%u rx_packets=%u, crc_errors=%u, "
				"sizeof(Port_cntrs)=%d, sizeof(rtk_stat_port_cntr_t)=%d\n", 
				pPort_cntrs, Port_cntrs.ifOutOctets, Port_cntrs.ifInOctets,
				Port_cntrs.ifOutUcastPkts + Port_cntrs.ifOutMulticastPkts + Port_cntrs.ifOutBrocastPkts,
				Port_cntrs.ifInUcastPkts + Port_cntrs.ifInMulticastPkts + Port_cntrs.ifInBroadcastPkts,
				Port_cntrs.dot3StatsFCSErrors, sizeof(Port_cntrs), sizeof(rtk_stat_port_cntr_t));*/
			memcpy(pPort_cntrs, &Port_cntrs, sizeof(Port_cntrs));
		}

		close(fd);
	}

	return 0;
}

#if defined(RTCONFIG_EXT_RTL8370MB)
#define MAX_RTL_PORTS 8
#else
#define MAX_RTL_PORTS 4
#endif
typedef struct {
	unsigned int link[MAX_RTL_PORTS];
	unsigned int speed[MAX_RTL_PORTS];
	unsigned int duplex[MAX_RTL_PORTS];
} phyState;

int  ext_rtk_phyState(int verbose, char* BCMPorts, phy_info_list *list)
{
	int model;
	char buf[64];
#ifdef RTCONFIG_NEW_PHYMAP
	char cap_buf[64] = {0};
#endif
#if defined(RTCONFIG_EXT_RTL8370MB)
	const char *portMark = "P0=%C;P1=%C;P2=%C;P3=%C;P4=%C;P5=%C;P6=%C;P7=%C;";
#else
	const char *portMark = "L5=%C;L6=%C;L7=%C;L8=%C;";
#endif
	int i, ret;
	int fd = open(RTKSWITCH_DEV, O_RDONLY);

	if (fd < 0) {
		perror(RTKSWITCH_DEV);
		return -1;
	}

	phyState pS;

#ifdef RTCONFIG_NEW_PHYMAP
	phy_port_mapping port_mapping = get_phy_port_mapping();
	int o[port_mapping.extsw_count];
	for (i = 0; i < port_mapping.extsw_count; i++) {
		pS.link[i] = 0;
		pS.speed[i] = 0;
		pS.duplex[i] = 0;
		o[i] = port_mapping.port[1+i+port_mapping.extsw_count].phy_port_id - S_RTL8365MB;
	}
#else
	int *o;
	for(i = 0 ; i < MAX_RTL_PORTS ; i++){
		pS.link[i] = 0;
		pS.speed[i] = 0;
		pS.duplex[i] = 0;
	}

        switch(model = get_model()) {
        case MODEL_RTAC5300:
		{
		/* RTK_LAN  BRCM_LAN  WAN  POWER */
		/* R0 R1 R2 R3 B4 B0 B1 B2 B3 */
		/* L8 L7 L6 L5 L4 L3 L2 L1 W0 */
		
#if defined(RTCONFIG_EXT_RTL8370MB)
		static const int porder[MAX_RTL_PORTS] = {2,3,4,0,1,5,6,7};
		o = (int *)porder;
#else
		static const int porder[MAX_RTL_PORTS] = {3,2,1,0};
		o = (int *)porder;
#endif

		break;
		}
        case MODEL_RTAC88U:
		{
		/* RTK_LAN  BRCM_LAN  WAN  POWER */
		/* R3 R2 R1 R0 B3 B2 B1 B0 B4 */
		/* L8 L7 L6 L5 L4 L3 L2 L1 W0 */
		
		static const int porder[4] = {0,1,2,3};
		o = (int *)porder;

		break;
		}
	default:
		{	
		static const int porder[4] = {0,1,2,3};
		o = (int *)porder;

		break;
		}
	}
#endif

	if (ioctl(fd, GET_RTK_PHYSTATES, &pS) < 0) {
		perror("rtkswitch ioctl");
		close(fd);
		return -1;
	}

	close(fd);

#if defined(RTCONFIG_EXT_RTL8370MB)
	sprintf(buf, portMark,
		(pS.link[o[0]] == 1) ? (pS.speed[o[0]] == 2) ? 'G' : 'M': 'X',
		(pS.link[o[1]] == 1) ? (pS.speed[o[1]] == 2) ? 'G' : 'M': 'X',
		(pS.link[o[2]] == 1) ? (pS.speed[o[2]] == 2) ? 'G' : 'M': 'X',
		(pS.link[o[3]] == 1) ? (pS.speed[o[3]] == 2) ? 'G' : 'M': 'X',
		(pS.link[o[4]] == 1) ? (pS.speed[o[4]] == 2) ? 'G' : 'M': 'X',
		(pS.link[o[5]] == 1) ? (pS.speed[o[5]] == 2) ? 'G' : 'M': 'X',
		(pS.link[o[6]] == 1) ? (pS.speed[o[6]] == 2) ? 'G' : 'M': 'X',
		(pS.link[o[7]] == 1) ? (pS.speed[o[7]] == 2) ? 'G' : 'M': 'X'
		);
#else
	sprintf(buf, portMark,
		(pS.link[o[0]] == 1) ? (pS.speed[o[0]] == 2) ? 'G' : 'M': 'X',
		(pS.link[o[1]] == 1) ? (pS.speed[o[1]] == 2) ? 'G' : 'M': 'X',
		(pS.link[o[2]] == 1) ? (pS.speed[o[2]] == 2) ? 'G' : 'M': 'X',
		(pS.link[o[3]] == 1) ? (pS.speed[o[3]] == 2) ? 'G' : 'M': 'X');
#endif

	if(verbose)
		printf("%s\n", buf);

	// prepare the phy_info list.
	if (list) {
		rtk_stat_port_cntr_t Port_cntrs;
#ifdef RTCONFIG_NEW_PHYMAP
		for(i = 0; i < port_mapping.extsw_count; i++) {
			list->phy_info[list->count].phy_port_id = port_mapping.port[list->count].phy_port_id;
			snprintf(list->phy_info[list->count].label_name, sizeof(list->phy_info[list->count].label_name), "%s", 
				port_mapping.port[1+i+port_mapping.extsw_count].label_name);
			snprintf(list->phy_info[list->count].cap_name, sizeof(list->phy_info[list->count].cap_name), "%s",
				get_phy_port_cap_name(port_mapping.port[1+i+port_mapping.extsw_count].cap, cap_buf, sizeof(cap_buf)));
#else
		for(i = 0; i < MAX_RTL_PORTS; i++) {
			list->phy_info[list->count].phy_port_id = o[i];
			snprintf(list->phy_info[list->count].label_name, sizeof(list->phy_info[list->count].label_name), "L%d", 
				list->count);
			snprintf(list->phy_info[list->count].cap_name, sizeof(list->phy_info[list->count].cap_name), "lan");
#endif

			snprintf(list->phy_info[list->count].state, sizeof(list->phy_info[list->count].state), "%s", 
				(pS.link[o[i]] == 1) ? "up" : "down");

			/*fprintf(stderr, "%p, o[%d]=%d, count=%d, phy_port=%d, cap_name=%s, state=%s\n", 
				&Port_cntrs, i, o[i], list->count, list->phy_info[list->count].phy_port, list->phy_info[list->count].cap_name, list->phy_info[list->count].state);*/
			if (pS.link[o[i]] == 1) {
				list->phy_info[list->count].link_rate = ((pS.speed[o[i]] == 0) ? 10 : ((pS.speed[o[i]] == 1) ? 100 : 1000));
				snprintf(list->phy_info[list->count].duplex, sizeof(list->phy_info[list->count].duplex), "%s", 
					pS.duplex ? "full" : "half");
				if(!ext_rtk_phy_mib(o[i], &Port_cntrs)) { 
					list->phy_info[list->count].tx_bytes = Port_cntrs.ifOutOctets;
					list->phy_info[list->count].rx_bytes = Port_cntrs.ifInOctets;
					list->phy_info[list->count].tx_packets = Port_cntrs.ifOutUcastPkts + Port_cntrs.ifOutMulticastPkts + Port_cntrs.ifOutBrocastPkts;
					list->phy_info[list->count].rx_packets = Port_cntrs.ifInUcastPkts + Port_cntrs.ifInMulticastPkts + Port_cntrs.ifInBroadcastPkts;
					list->phy_info[list->count].crc_errors = Port_cntrs.dot3StatsFCSErrors;
				}
			} else {
				snprintf(list->phy_info[list->count].duplex, sizeof(list->phy_info[list->count].duplex), "none");
			}
			list->count++;
		}
	}

	ret = 0;
	for(i = 0; i < MAX_RTL_PORTS; i++){
		if(pS.link[o[i]] == 1){
			ret = 1;
			break;
		}
	}
	return ret;
}

void usage(char *cmd)
{
	int ci, pa;

	/* set pa */
	for(ci = 0; ci < MAX_REQ; ++ci) {
        	switch (ci) {
        	case GET_PORT_STAT:
		case GET_PORT_SPEED:
        	case RESET_PORT:
        	case SET_EXT_TXDELAY:
        	case SET_EXT_RXDELAY:
        	case GET_REG:
		case SET_EXT_MODE:
		case SET_CPU:
		case GET_PHY_REG9:
		case GET_TMODE:
		case SET_GREEN_ETHERNET:
			rtk_cmds_pa[ci] = 1;
                	break;

        	case TEST_REG:
        	case SET_REG:
		case SET_PHY_REG9:
		case SET_TMODE:
			rtk_cmds_pa[ci] = 2;
                	break;

		default:
			rtk_cmds_pa[ci] = 0;
		}
	}

	printf("Usage:\n");
	for(ci = 0; ci < MAX_REQ; ++ci)
		printf("  %s %d %s##( %s )##\n", cmd, ci, (pa=rtk_cmds_pa[ci])?pa==1?"\t[arg1] \t":"\t[arg1] [arg2] ":ci > 9?"\t\t":" \t\t", rtk_switch_cmds[ci]);
}
