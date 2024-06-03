/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2020 Broadcom Ltd.
 */

#ifndef _OTP_HW_MAP_H
#define _OTP_HW_MAP_H

/*
 * includes common OTP map rows for JTAG password
 * */
#if defined (CONFIG_OTP_V1)

#define OTP_JTAG_SER_NUM_ROW_1          	19          // Row19[25:20] = CSEC_CHIPID[5:0]
#define OTP_JTAG_SER_NUM_MASK_1         	0x0000003F
#define OTP_JTAG_SER_NUM_REG_SHIFT_1    	20
#define OTP_JTAG_SER_NUM_ROW_2          	20          // Row20[25:0]  = CSEC_CHIPID[31:6]
#define OTP_JTAG_SER_NUM_MASK_2         	0x03FFFFFF
#define OTP_JTAG_SER_NUM_SHIFT_2        	6
#define OTP_JTAG_PWD_ROW_1              	21          // Row21[25:0]  = CSEC_PWD[25:0]
#define OTP_JTAG_PWD_ROW_2              	22          // Row22[25:0]  = CSEC_PWD[51:26]
#define OTP_JTAG_PWD_ROW_3              	23          // Row23[11:0]  = CSEC_PWD[63:52]
#define OTP_JTAG_PWD_MASK_1             	0x03FFFFFF
#define OTP_JTAG_PWD_MASK_2             	0x03FFFFFF
#define OTP_JTAG_PWD_MASK_3             	0x00000FFF
#define OTP_JTAG_PWD_SHIFT_1            	0
#define OTP_JTAG_PWD_SHIFT_2            	26
#define OTP_JTAG_PWD_SHIFT_3            	52
#define OTP_JTAG_PWD_RDLOCK_ROW         	23
#define OTP_JTAG_PWD_RDLOCK_REG_SHIFT   	25          // Row23[25]

#define OTP_JTAG_MODE_ROW               	18
#define OTP_JTAG_MODE_REG_SHIFT         	9
#define OTP_JTAG_MODE_LOCK              	0x38
#define OTP_JTAG_MODE_PERMALOCK         	0x3F
#define OTP_JTAG_MODE_MASK			0x3F

#define OTP_JTAG_CUST_LOCK_ROW          	6
#define OTP_JTAG_CUST_LOCK_VAL          	0x1F
#define OTP_JTAG_CUST_LOCK_REG_SHIFT    	25



#elif defined (CONFIG_OTP_V2)


#define OTP_JTAG_MODE_ROW               18
#define OTP_JTAG_MODE_REG_SHIFT         9
#define OTP_JTAG_MODE_LOCK              0x38
#define OTP_JTAG_MODE_PERMALOCK         0x3F
#define OTP_JTAG_MODE_MASK              0x3F

#define OTP_JTAG_SER_NUM_ROW            20          // Row20 = CSEC_CHIPID
#define OTP_JTAG_SER_NUM_SHIFT		0x0
#define OTP_JTAG_SER_NUM_MASK		0xFFFFFFFF

#define OTP_JTAG_PWD_ROW_1              21          // Row21 = CSEC_JTAGPWD[31:0]
#define OTP_JTAG_PWD_SHIFT_1            0
#define OTP_JTAG_PWD_MASK_1             0xFFFFFFFF

#define OTP_JTAG_PWD_ROW_2              22          // ROW22 = CSEC_JTAGPWD[63:32]
#define OTP_JTAG_PWD_MASK_2             0xFFFFFFFF
#define OTP_JTAG_PWD_SHIFT_2		0x0  //was 31 why?

/* row 19 */
#define OTP_JTAG_PWD_RDLOCK_ROW		19          // ROW19[31]
#define OTP_JTAG_PWD_RDLOCK_SHIFT	31
#define OTP_JTAG_PWD_RDLOCK_MASK	0x1	

/* row 6 */
/* This is a section LOCKING row - whole section getting locked
 * either CUSTomer( USER2)
 * rows 18-25 are locked 
 *
 * */
#define OTP_JTAG_CUST_LOCK_ROW          6
#define OTP_JTAG_CUST_LOCK_SHIFT	25
#define OTP_JTAG_CUST_LOCK_MASK		0x1F
#define OTP_JTAG_CUST_LOCK_VAL          0x1F


#elif defined (CONFIG_OTP_V3)


/* row 14 */
#define OTP_JTAG_MODE_ROW               	14 /* 0 -fully open; 2 - pwd protected; 3 fully closed*/
#define OTP_JTAG_MODE_SHIFT         		16
#define OTP_JTAG_MODE_MASK         		0x3
#define OTP_JTAG_MODE_LOCK              	0x2
#define OTP_JTAG_MODE_PERMALOCK         	0x3

/* row 26 */
#define OTP_JTAG_SER_NUM_ROW          		26          // Row26 = CSEC_JTAGID[31:0] (formerly CSEC_CHIPID) 
#define OTP_JTAG_SER_NUM_SHIFT			0x0
#define OTP_JTAG_SER_NUM_MASK			0xFFFFFFFF

/* row 27 */
#define OTP_JTAG_PWD_ROW_1              	27          // Row27 = CSEC_JTAGPWD[31:0]
#define OTP_JTAG_PWD_SHIFT_1            	0
#define OTP_JTAG_PWD_MASK_1             	0xFFFFFFFF

/* row 28 */
#define OTP_JTAG_PWD_ROW_2              	28          // ROW28 = CSEC_JTAGPWD[63:32]
#define OTP_JTAG_PWD_SHIFT_1            	0
#define OTP_JTAG_PWD_MASK_2             	0xFFFFFFFF

/* row 14 */
#define OTP_JTAG_PWD_RDLOCK_ROW         	14          // ROW14[25] (formerly CSEC_READLOCK) 
#define OTP_JTAG_PWD_RDLOCK_SHIFT       	25	
#define OTP_JTAG_PWD_RDLOCK_MASK         	0x1 


#else

#error OTP MAP is not defined  

#endif

#endif
