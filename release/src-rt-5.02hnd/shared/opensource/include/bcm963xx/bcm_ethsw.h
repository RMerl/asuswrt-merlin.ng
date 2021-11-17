/*
 * <:copyright-BRCM:2013:DUAL/GPL:standard
 * 
 *    Copyright (c) 2013 Broadcom 
 *    All Rights Reserved
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 * :>
 */

void bcm_ethsw_init(void);
void bcm_ethsw_open(void);
void bcm_ethsw_close(void);
uint16_t bcm_ethsw_phy_read_reg(int phy_id, int reg);
void bcm_ethsw_phy_write_reg(int phy_id, int reg, uint16 data);


#if defined(_CFE_) 
#define mii_write bcm_ethsw_phy_write_reg
#define mii_read  bcm_ethsw_phy_read_reg
#endif
