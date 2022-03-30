/*
   Copyright (c) 2017 Broadcom Corporation
   All Rights Reserved

   <:label-BRCM:2017:DUAL/GPL:standard
   
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

#ifndef MERLIN_MDIO_H
#define MERLIN_MDIO_H

#define  MDIO_CMD                0x3FFD00
#define  MDIO_CFG                0x3FFD04
#define  MDIO_IRQ_CLEAR          0x3FFD08

#define  MDIO_CTRL_CL45_ADDRESS  0x0
#define  MDIO_CTRL_WRITE         0x1
#define  MDIO_CTRL_READ          0x2
#define  MDIO_CTRL_CL45_READ     0x3
#define  MDIO_CTRL_CMD_START     26
#define  MDIO_CTRL_START         (1<<29)
#define  MDIO_CTRL_MDIO_FAIL_MASK (1<<28)
#define  MDIO_CTRL_ID_START      21
#define  MDIO_CTRL_ID_MASK       (0x1f<<MDIO_CTRL_ID_START)
#define  MDIO_CTRL_ADDR_START    16
#define  MDIO_CTRL_ADDR_MASK     (0x1f<<MDIO_CTRL_ADDR_START)
#define  MDIO_CTRL_WR_DATA_MASK  0xFFFF

#endif //MERLIN_MDIO_H
