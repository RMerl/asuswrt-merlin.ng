/******************************************************************************

    Copyright 2022 Maxlinear

    SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0-only)

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.
******************************************************************************/
/**
   \file smdio_access.h
    GPHY mode flashless load FW example. 

*/

#ifndef __SMDIO_ACCESS_H__
#define __SMDIO_ACCESS_H__

#include <stdint.h>

int smdio_write(uint8_t lid, uint8_t phy, uint16_t phy_reg, uint16_t phy_reg_data);
int smdio_cont_write(uint8_t lid, uint8_t phy, uint16_t phy_reg, uint16_t phy_reg_data[8], uint8_t num);
int smdio_read(uint8_t lid, uint8_t phy, uint16_t phy_reg);
int mdio_read(uint8_t lid, uint8_t phy, uint16_t phy_reg);
int smdio_init(void);

#endif /* __SMDIO_ACCESS_H__ */
