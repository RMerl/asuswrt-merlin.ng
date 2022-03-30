// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2016 Broadcom Corporation
   All Rights Reserved

    
*/

#ifndef __MDIO_DRV_IMPL3_H
#define __MDIO_DRV_IMPL3_H

#include "os_dep.h"

#define MDIO_ERROR     -1
#define MDIO_OK        0

extern int32_t mdio_read_c22_register(uint8_t core_num, uint32_t phy_addr, uint32_t reg_addr, uint16_t *data_read);
extern int32_t mdio_write_c22_register(uint8_t core_num, uint32_t phy_addr, uint32_t reg_addr, uint16_t data_write);

extern int32_t mdio_read_c45_register(uint8_t core_num, uint32_t port_addr, uint32_t dev_addr, uint16_t dev_offset, uint16_t *data_read);
extern int32_t mdio_write_c45_register(uint8_t core_num, uint32_t port_addr, uint32_t dev_addr,uint16_t dev_offset, uint16_t data_write);


#endif
