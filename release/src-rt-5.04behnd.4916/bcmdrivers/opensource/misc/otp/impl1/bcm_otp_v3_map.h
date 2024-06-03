/*
<:copyright-BRCM:2021:DUAL/GPL:standard 

   Copyright (c) 2021 Broadcom 
   All Rights Reserved

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

#ifndef BCM_OTP_V3_MAP_H
#define BCM_OTP_V3_MAP_H

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

#define OTP_CPU_LOCK_OFF                  0x54
#define OTP_CPU_LOCK_MASK	              (0x01 << 0)   

#define OTP_TYPE_V3

#define OTP_JTAG_SER_NUM_ROW_1          26          // Row26 = CSEC_JTAGID
#define OTP_JTAG_PWD_ROW_1              27          // Row27 = CSEC_JTAGPWD[31:0]
#define OTP_JTAG_PWD_ROW_2              28          // ROW28 = CSEC_JTAGPWD[63:32]
#define OTP_JTAG_PWD_MASK_1             0xFFFFFFFF
#define OTP_JTAG_PWD_MASK_2             0xFFFFFFFF
#define OTP_JTAG_PWD_SHIFT_1            0
#define OTP_JTAG_PWD_SHIFT_2            32
#define OTP_JTAG_PWD_RDLOCK_ROW         14          // ROW14[25]
#define OTP_JTAG_PWD_RDLOCK_REG_SHIFT   25          
#define OTP_JTAG_MODE_NUM_BITS          2
#define OTP_JTAG_MODE_ROW               14
#define OTP_JTAG_MODE_REG_SHIFT         16         
#define OTP_JTAG_MODE_LOCK              0x02 
#define OTP_JTAG_MODE_PERMALOCK         0x03
#define OTP_JTAG_MODE_MASK              0x03
#define OTP_JTAG_CUST_LOCK_ROW          11	    // ROW11 = CFG_LOCK
#define OTP_JTAG_CUST_LOCK_VAL          0x1F 
#define OTP_JTAG_CUST_LOCK_REG_SHIFT    15

#endif
