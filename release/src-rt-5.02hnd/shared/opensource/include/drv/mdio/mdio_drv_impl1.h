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
/* This file contains the definition for Broadcom's MDIO block		          */
/*                                                                            */
/******************************************************************************/
#ifndef __MDIO_DRV_IMPL1_H
#define __MDIO_DRV_IMPL1_H


/*****************************************************************************/
/*                                                                           */
/* Include files                                                             */
/*                                                                           */
/*****************************************************************************/
#include "os_dep.h"
#include "access_macros.h"
#include "bcm_map_part.h"
/******************************************************************************/
/*                                                                            */
/* Types and values definitions                                               */
/*                                                                            */
/******************************************************************************/


/******************************************************************************************************/
typedef enum
{
	MDIO_EXT = MDIO_EXT_BASE ,
	MDIO_EGPHY = MDIO_EGPHY_BASE,
	MDIO_SATA = MDIO_SATA_BASE,
	MDIO_AE = MDIO_AE_BASE

}MDIO_TYPE;


#define MDIO_ERROR -1
#define MDIO_OK		0


/******************************************************************************/
/*                                                                            */
/* Functions prototypes                                                       */
/*                                                                            */
/******************************************************************************/

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
int32_t mdio_read_c22_register(MDIO_TYPE type,uint32_t phyAddr,uint32_t regAddr,uint16_t *dataRead);

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
int32_t mdio_write_c22_register(MDIO_TYPE type,uint32_t phyAddr,uint32_t regAddr,uint16_t dataWrite);

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
int32_t mdio_read_c45_register(MDIO_TYPE type,uint32_t PortAddr,uint32_t DevAddr,uint16_t RegOffset ,uint16_t *dataRead);

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
int32_t mdio_write_c45_register(MDIO_TYPE type,uint32_t PortAddr,uint32_t DevAddr,uint16_t DevOffset,uint16_t dataWrite);

#endif
