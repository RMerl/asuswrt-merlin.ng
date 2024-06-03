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
#ifndef _RTKSWITCH_H_
#define _RTKSWITCH_H_

#define REG_ESW_PORT_PCR_P0     0x2004
#define REG_ESW_PORT_PVC_P0     0x2010
#define REG_ESW_PORT_PPBV1_P0       0x2014
#define REG_ESW_PORT_BSR_P0     0x201c
#define REG_ESW_MAC_PMSR_P0     0x3008
#define REG_ESW_MAC_PMCR_P6     0x3600
#define REG_ESW_PORT_TGOCN_P0       0x4018
#define REG_ESW_PORT_RGOCN_P0       0x4028

#define REG_ESW_PFC1        0x14    /* Priority Flow Control 1 */
#define REG_ESW_PVIDC0      0x40    /* PVID Configuration 0 */
#define REG_ESW_PVIDC1      0x44    /* PVID Configuration 1 */
#define REG_ESW_PVIDC2      0x48    /* PVID Configuration 2 */
#define REG_ESW_PVIDC3      0x4C    /* PVID Configuration 3 */
#define REG_ESW_VLANI0      0x50    /* VLAN Identifier 0 */
#define REG_ESW_VLANI1      0x54    /* VLAN Identifier 1 */
#define REG_ESW_VMSC0       0x70    /* VLAN Member Port Configuration 0 */
#define REG_ESW_POC0        0x90    /* Port Control 0 */
#define REG_ESW_POC2        0x98    /* Port Control 2 */
#define REG_ESW_POA     0x80
#define REG_ESW_FPA     0x84
#define REG_ESW_STRT        0xA0    /* Switch Reset */
#define REG_ESW_VUB0        0x100   /* VLAN Untag Block 0 */
#define REG_ESW_POA_LINK_STATUS_SHIFT       25
#define REG_ESW_VLAN_ID_BASE        0x50

#if 0
extern int config_rtkswitch(int argc, char *argv[]);

extern int ralink_gpio_write_bit(int idx, int value);
extern int ralink_gpio_read_bit(int idx);
extern int ralink_gpio_init(unsigned int idx, int dir);
extern uint32_t ralink_gpio_read(void);
extern int ralink_gpio_write(uint32_t bit, int en);

extern int rtkswitch_LanPort_linkUp();
extern int rtkswitch_LanPort_linkDown();
extern int rtkswitch_AllPort_linkUp();
extern int rtkswitch_AllPort_linkDown();
extern void rtkswitch_AllPort_phyState();
//sherry fix rt-ac53 bug#5 traffic incorrect when hwnat enable{
extern int get_realtek_wan_bytecount(unsigned long *tx, unsigned long *rx);
extern int get_realtek_lans_bytecount(unsigned long *tx, unsigned long *rx);
//end sherry}

extern int get_ralink_wan_status(unsigned int *linkup, unsigned int *linkspeed);
#endif
#endif
