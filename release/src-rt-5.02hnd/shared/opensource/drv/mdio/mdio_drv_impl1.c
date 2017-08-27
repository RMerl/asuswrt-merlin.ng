/*
   Copyright (c) 2013 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2013:DUAL/GPL:standard

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
/******************************************************************************/
/*                                                                            */
/* File Description:                                                          */
/*                                                                            */
/* This file contains the implementation for Broadcom's MDIO block	          */
/*                                                                            */
/******************************************************************************/

/*****************************************************************************/
/*                                                                           */
/* Include files                                                             */
/*                                                                           */
/*****************************************************************************/
#include "mdio_drv_impl1.h"

#ifdef _CFE_
#define spin_lock_bh(x)
#define spin_unlock_bh(x)
#else
#include <linux/spinlock.h>
static DEFINE_SPINLOCK(mdio_access);
#endif

typedef enum
{
	MDIO_CMD = MDIO_CMD_REGISTER_OFFSET,
	MDIO_CFG = MDIO_CFG_REGISTER_OFFSET
}MDIO_REG;
/*****************************************************************************
 * type of MDIO Clause 														 *
 * MDIO clause 45 define different register access rather than Clause 22	 *
 ****************************************************************************/
typedef enum
{
	MDIO_CLAUSE_45,
	MDIO_CLAUSE_22,
}MDIO_CLAUSE;
/****************************************************************************
 * MDIO Clause 45 operation type											*
 ***************************************************************************/
typedef enum
{
	MDIO45_ADDRESS,
	MDIO45_WRITE,
	MDIO45_READINC,
	MDIO45_READ
}MDIO45_OP;
/****************************************************************************
 * MDIO Clause 22 operation type											*
 ***************************************************************************/
typedef enum
{
	MDIO22_WRITE = 1,
	MDIO22_READ = 2,
}MDIO22_OP;


/****************************************************************************
 * structure describing the COMMAND register of the MDIO block				*
 ***************************************************************************/
#pragma pack(push, 1)
typedef struct
{
	 uint32_t	reserved:2;// for ECO Reserved bits must be written with 0.  A read returns an unknown value.
	 uint32_t	mdio_busy:1;// CPU writes this bit to 1 in order to initiate MCIO transaction. When transaction completes hardware will clear this bit. 	Reset value is 0x0.
	 uint32_t	fail:1;// This bit is set when PHY does not reply to READ command (PHY does not drive 0 on bus turnaround).  Reset value is 0x0.
	 uint32_t	op_code:2;// MDIO command that is OP[1:0]:  00 - Address for clause 45  01 - Write  10 - Read increment for clause 45  11 - Read for clause 45  10 - Read for clause 22  Reset value is 0x0.
	 uint32_t	phy_prt_addr:5;// PHY address[4:0] for clause 22, Port address[4:0] for Clause 45.  Reset value is 0x0.
	 uint32_t	reg_dec_addr:5;// Register address[4:0] for clause 22, Device address[4:0] for Clause 45. Reset value is 0x0.
	 uint32_t	data_addr:16;// " MDIO Read/Write data[15:0], clause 22 and 45 or MDIO address[15:0] for clause 45".	Reset value is 0x0.

}MDIO_CMD_REG;
#pragma pack(pop)
/****************************************************************************
 * structure describing the CONFIG register of the MDIO block				*
 ***************************************************************************/
#pragma pack(push, 1)
typedef struct
{
	uint32_t	reserved2:18;// Reserved bits must be written with 0.  A read returns an unknown value.
	uint32_t	supress_preamble:1;// When this bit set, preamble (32 consectutive 1's) is suppressed for MDIO transaction that is MDIO transaction starts with ST.Reset value is 0x0.
	uint32_t	reserved1:1;// Reserved bit must be written with 0.  A read returns an unknown value.
	uint32_t	mdio_clk_divider:7;// MDIO clock divider[6:0], system clock is divided by 2x(MDIO_CLK_DIVIDER+1) to generate MDIO clock(MDC); system clock = 200Mhz.Minimum supported value: 1	Reset value is 0x7.
	uint32_t	reserved0:4;// bits must be written with 0.  A read returns an unknown value.
	uint32_t	mdio_clause:1;// 0: Clause 45  1: Clause 22  Reset value is 0x1.

}MDIO_CFG_REG;
#pragma pack(pop)


#define MDIO_READ32_REG(b,r,o)	READ_32( (b + r), (o) )
#define MDIO_WRITE32_REG(b,r,i)	WRITE_32( (b + r), (i) )
#define MDIO_READ_FIELD(b,r,f,o) ( (*(volatile uint32_t*)&(o)) = ( VAL32(b + r) & MDIO_##r##_##f##_MASK ) >> MDIO_##r##_##f##_SHIFT  )
#define MDIO_WRITE_FIELD(b,r,f,i) ( VAL32(b+MDIO_##r##_REGISTER_OFFSET) = ( VAL32(b+ MDIO_##r##_REGISTER_OFFSET) & ~MDIO_##r##_##f##_MASK ) | ((i << MDIO_##r##_##f##_SHIFT) & MDIO_##r##_##f##_MASK) )

#define MDIO_BUSY_RETRY		2000
#define CHECK_MDIO_READY(type) if (!mdioIsReady(type)) {spin_unlock_bh(&mdio_access); return MDIO_ERROR;}

static inline int32_t mdioIsReady(MDIO_TYPE type)
{
	volatile MDIO_CMD_REG	mdioReg ;
	uint32_t		retry = MDIO_BUSY_RETRY;

	/*check for business of MDIO*/
	MDIO_READ32_REG(type,MDIO_CMD,mdioReg);

	while( mdioReg.mdio_busy && ( retry-- > 0))
	{
		udelay(10);
		MDIO_READ32_REG(type,MDIO_CMD,mdioReg);
	}
	/*check we didn't reach the delay*/
	if(!retry )
	{
		printk("MDIO Error:mdioIsReady reached maximum retries of %d\n ",retry);
		return 0;
	}
	if ( mdioReg.mdio_busy == 1 && mdioReg.fail != 1)
	{
		printk("MDIO Error:busy bit not cleared or MDIO failed\n ");
		return 0;
	}
	return 1;
}
/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mdio_read_c22_register						                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MDIO Driver - read Clause 22            				                  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function implement the Clause 22 read register as it described	  */
/* 	 in IEEE P802.3ae MDC/MDIO Clause 22							          */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   MDIO_TYPE - type of the mdio interface ( EGPY,EPON,AE,EXT)               */
/*	 phyAddr   - the address of the phy accessed in the mdio bus ( up to 32)  */
/*	 regAddr   - the offset of the register accessed in the phy				  */
/*                                                                            */
/* Output:                                                                    */
/*   dataRead - value of read register.										  */
/*                                                                            */
/******************************************************************************/
int32_t mdio_read_c22_register(MDIO_TYPE type,uint32_t phyAddr,uint32_t regAddr,uint16_t *dataRead)
{
	volatile  MDIO_CMD_REG	mdioReg ;

    spin_lock_bh(&mdio_access);

	/*configure the MDIO to work in clause 22 mode*/
	MDIO_WRITE_FIELD(type,CFG,mdio_clause,MDIO_CLAUSE_22);

	CHECK_MDIO_READY(type);

	mdioReg.phy_prt_addr = phyAddr;
	mdioReg.reg_dec_addr = regAddr;
	mdioReg.op_code		 = MDIO22_READ;
	mdioReg.mdio_busy	 = 1;
	/*write the command*/
	MDIO_WRITE32_REG(type,MDIO_CMD,mdioReg);


	CHECK_MDIO_READY(type);

	/*if reached here we shall have proper read value*/
	MDIO_READ32_REG(type,MDIO_CMD,mdioReg);

	*dataRead = mdioReg.data_addr;

    spin_unlock_bh(&mdio_access);

	return MDIO_OK;
}
EXPORT_SYMBOL(mdio_read_c22_register);
/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mdio_write_c22_register					                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MDIO Driver - write Clause 22            				                  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function implement the Clause 22 write register as it described	  */
/* 	 in IEEE P802.3ae MDC/MDIO Clause 22							          */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   MDIO_TYPE - type of the mdio interface ( EGPY,EPON,AE,EXT)               */
/*	 phyAddr   - the address of the phy accessed in the mdio bus ( up to 32)  */
/*	 regAddr   - the offset of the register accessed in the phy				  */
/*	 dataWrite - new value for accessed register							  */
/*                                                                            */
/* Output:                                                                    */
/*   									  									  */
/*                                                                            */
/******************************************************************************/
int32_t mdio_write_c22_register(MDIO_TYPE type,uint32_t phyAddr,uint32_t regAddr,uint16_t dataWrite)
{
	volatile MDIO_CMD_REG	mdioReg ;

    spin_lock_bh(&mdio_access);

	/*configure the MDIO to work in clause 22 mode*/
	MDIO_WRITE_FIELD(type,CFG,mdio_clause,MDIO_CLAUSE_22);

	CHECK_MDIO_READY(type);

	mdioReg.phy_prt_addr = phyAddr;
	mdioReg.reg_dec_addr = regAddr;
	mdioReg.op_code		 = MDIO22_WRITE;
	mdioReg.data_addr	 = dataWrite;
	mdioReg.mdio_busy	 = 1;

	/*write the command*/
	MDIO_WRITE32_REG(type,MDIO_CMD,mdioReg);

	CHECK_MDIO_READY(type);

    spin_unlock_bh(&mdio_access);

	return MDIO_OK;
}
EXPORT_SYMBOL(mdio_write_c22_register);
/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mdio_read_c45_register						                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MDIO Driver - write Clause 45            				                  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function implement the Clause 45 write register as it described	  */
/* 	 in IEEE P802.3ae MDC/MDIO Clause 45							          */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   MDIO_TYPE  - type of the mdio interface ( EGPY,EPON,AE,EXT)              */
/*	 portAddr   - the address of the phy accessed in the mdio bus ( up to 32) */
/*	 DevAddr    - the inner device in phy (0-32)							  */
/*	 RegOffset  - the offset of the register accessed in the phy(64K)		  */
/*                                                                            */
/* Output:                                                                    */
/*  dataRead  - read value of accessed register								  */
/*                                                                            */
/******************************************************************************/
int32_t mdio_read_c45_register(MDIO_TYPE type,uint32_t PortAddr,uint32_t DevAddr,uint16_t DevOffset ,uint16_t *dataRead)
{
	MDIO_CMD_REG	mdioReg ;

    spin_lock_bh(&mdio_access);

	/*configure the MDIO to work in clause 45 mode*/
	MDIO_WRITE_FIELD(type,CFG,mdio_clause,MDIO_CLAUSE_45);


	CHECK_MDIO_READY(type);

	/*write the address*/
	mdioReg.op_code		 = MDIO45_ADDRESS;
	mdioReg.phy_prt_addr = PortAddr;
	mdioReg.reg_dec_addr = DevAddr;
	mdioReg.data_addr	 = DevOffset;
	mdioReg.mdio_busy	 = 1;
	MDIO_WRITE32_REG(type,MDIO_CMD,mdioReg);


	CHECK_MDIO_READY(type);

	/*read the Data */
	mdioReg.op_code		 = MDIO45_READ;
	mdioReg.phy_prt_addr = PortAddr;
	mdioReg.reg_dec_addr = DevAddr;
	mdioReg.mdio_busy	 = 1;
	/*write the read command*/
	MDIO_WRITE32_REG(type,MDIO_CMD,mdioReg);


	CHECK_MDIO_READY(type);
	
    /*if reached here we shall have proper read value*/
	MDIO_READ32_REG(type,MDIO_CMD,mdioReg);

	*dataRead = mdioReg.data_addr;

    spin_unlock_bh(&mdio_access);

    return MDIO_OK;

}
EXPORT_SYMBOL(mdio_read_c45_register);

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   mdio_write_c45_register					                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   MDIO Driver - write Clause 45            				                  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function implement the Clause 45 write register as it described	  */
/* 	 in IEEE P802.3ae MDC/MDIO Clause 45							          */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   MDIO_TYPE  - type of the mdio interface ( EGPY,EPON,AE,EXT)              */
/*	 portAddr   - the address of the phy accessed in the mdio bus ( up to 32) */
/*	 DevAddr    - the inner device in phy (0-32)							  */
/*	 RegOffset  - the offset of the register accessed in the phy(64K)		  */
/*	 dataWrite  - new value of accessed register							  */
/*                                                                            */
/* Output:                                                                    */
/*   									  									  */
/*                                                                            */
/******************************************************************************/
int32_t mdio_write_c45_register(MDIO_TYPE type,uint32_t PortAddr,uint32_t DevAddr,uint16_t DevOffset,uint16_t dataWrite)
{
	MDIO_CMD_REG	mdioReg ;

    spin_lock_bh(&mdio_access);

	/*configure the MDIO to work in clause 45 mode*/
	MDIO_WRITE_FIELD(type,CFG,mdio_clause,MDIO_CLAUSE_45);


	CHECK_MDIO_READY(type);
	/*write the address*/
	mdioReg.op_code		 = MDIO45_ADDRESS;
	mdioReg.phy_prt_addr = PortAddr;
	mdioReg.reg_dec_addr = DevAddr;
	mdioReg.data_addr	 = DevOffset;
	mdioReg.mdio_busy	 = 1;
	MDIO_WRITE32_REG(type,MDIO_CMD,mdioReg);

	CHECK_MDIO_READY(type);

	mdioReg.op_code 	= MDIO45_WRITE;
	mdioReg.data_addr	= dataWrite;
	mdioReg.mdio_busy	 = 1;
	MDIO_WRITE32_REG(type,MDIO_CMD,mdioReg);


	CHECK_MDIO_READY(type);

    spin_unlock_bh(&mdio_access);

	return MDIO_OK;
}
EXPORT_SYMBOL(mdio_write_c45_register);

