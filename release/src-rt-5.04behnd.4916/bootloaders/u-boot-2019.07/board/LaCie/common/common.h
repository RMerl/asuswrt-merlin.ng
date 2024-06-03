/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011 Simon Guinot <sguinot@lacie.com>
 */

#ifndef _LACIE_COMMON_H
#define _LACIE_COMMON_H

#if defined(CONFIG_CMD_NET) && defined(CONFIG_RESET_PHY_R)
void mv_phy_88e1116_init(const char *name, u16 phyaddr);
void mv_phy_88e1318_init(const char *name, u16 phyaddr);
#endif
#if defined(CONFIG_CMD_I2C) && defined(CONFIG_SYS_I2C_EEPROM_ADDR)
int lacie_read_mac_address(uchar *mac);
#endif

#endif /* _LACIE_COMMON_H */
