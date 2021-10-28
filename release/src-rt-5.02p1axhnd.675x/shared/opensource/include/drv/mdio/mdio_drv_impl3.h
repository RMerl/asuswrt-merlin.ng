/*
   Copyright (c) 2016 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
    :>
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
