// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2017 Broadcom Corporation
   All Rights Reserved

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
