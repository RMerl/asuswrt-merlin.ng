/*
<:copyright-BRCM:2021:DUAL/GPL:standard 

   Copyright (c) 2021 Broadcom 
   All Rights Reserved

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

#ifndef BCM_OTP_V2_MAP_H
#define BCM_OTP_V2_MAP_H

#define OTP_CTRL0_OFF                     0x0
#define JTAG_OTP_CTRL_ACCESS_MODE         (0x2 << 22)
#define JTAG_OTP_CTRL_PROG_EN             (0x1 << 21)
#define JTAG_OTP_CTRL_START               (0x1 << 0)
#define JTAG_OTP_CTRL_CMD_OTP_PROG_EN	  (0x2 << 1)
#define JTAG_OTP_CTRL_CMD_PROG		      (0xa << 1)
#define JTAG_OTP_CTRL_CMD_PROG_LOCK	      (0x19 << 1)

#define OTP_CTRL1_OFF                     0x4
#define JTAG_OTP_CTRL_CPU_MODE            (0x1 << 0)

#define OTP_CTRL2_OFF                     0x8
#define OTP_CTRL2_HI_OFF                  0xc
#define OTP_CTRL3_OFF                     0x10
#define OTP_CTRL4_OFF                     0x14

#define OTP_STATUS0_OFF                   0x18
#define OTP_STATUS0_HI_OFF                0x1c
#define OTP_STATUS1_OFF                   0x20
#define JTAG_OTP_STATUS_1_PROG_OK         (0x1 << 2) 
#define JTAG_OTP_STATUS_1_CMD_DONE        (0x1 << 1)

#define OTP_TYPE_V2

#define OTP_JTAG_SER_NUM_ROW_1            20          // Row20 = CSEC_CHIPID
#define OTP_JTAG_PWD_ROW_1                21          // Row21 = CSEC_JTAGPWD[31:0]
#define OTP_JTAG_PWD_ROW_2                22          // ROW22 = CSEC_JTAGPWD[63:32]
#define OTP_JTAG_PWD_MASK_1               0xFFFFFFFF
#define OTP_JTAG_PWD_MASK_2               0xFFFFFFFF
#define OTP_JTAG_PWD_SHIFT_1              0
#define OTP_JTAG_PWD_SHIFT_2              32
#define OTP_JTAG_PWD_RDLOCK_ROW           19          // ROW19[31]
#define OTP_JTAG_PWD_RDLOCK_REG_SHIFT     31          
#define OTP_JTAG_MODE_NUM_BITS            6
#define OTP_JTAG_MODE_MAJORITY_BIT_CNT    2
#define OTP_JTAG_MODE_ROW                 18
#define OTP_JTAG_MODE_REG_SHIFT           9         
#define OTP_JTAG_MODE_LOCK                0x38
#define OTP_JTAG_MODE_PERMALOCK           0x3F
#define OTP_JTAG_MODE_MASK                0x3F
#define OTP_JTAG_CUST_LOCK_ROW            6
#define OTP_JTAG_CUST_LOCK_VAL            0x1F
#define OTP_JTAG_CUST_LOCK_REG_SHIFT      25

#endif
