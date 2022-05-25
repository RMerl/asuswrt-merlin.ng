/*
   Copyright (c) 2017 Broadcom Corporation
   All Rights Reserved

   <:label-BRCM:2017:DUAL/GPL:standard

   Unless you and Broadcom execute a separate written software license
   agreement governing use of this software, this software is licensed
   to you under the terms of the GNU General Public License version 2
   (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
   with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

   Not withstanding the above, under no circumstances may you combine
   this software in any way with any other Broadcom software provided
   under a license other than the GPL, without Broadcom's express prior
   written consent.

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
