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

#ifndef __GPONCLKOUT_CFG_H_INCLUDED
#define __GPONCLKOUT_CFG_H_INCLUDED

#include "access_macros.h"
#include "packing.h"

/*****************************************************************************************/
/* This module is the HW implementation for GPONCLKOUT_CFG.                              */
/*****************************************************************************************/

/*****************************************************************************************/
/* Blocks offsets                                                                        */
/*****************************************************************************************/
/*****************************************************************************************/
/* Functions offsets and addresses                                                       */
/*****************************************************************************************/
#define GPONCLKOUT_CFG_OFFSET 	( 0x00000000 )
#define GPONCLKOUT_CFG_ADDRESS	( GPONCLKOUT_CFG_REGS_OFFSET + GPONCLKOUT_CFG_OFFSET )

/*   'd' is module index   */
/*   'i' is block index    */
/*   'j' is function index */
/*   'e' is function entry */
/*   'k' is register index */


/*****************************************************************************************/
/* Pbi Reset Register                                                                    */
/* This register provides reset control over the various clock submodules.               */
/*****************************************************************************************/

#define GPONCLKOUT_CFG_PBI_RESET_R1_VALUE                   ( 0x0 )
#define GPONCLKOUT_CFG_PBI_RESET_CLKDIVRSTN_HOLD_VALUE      ( 0x0 )
#define GPONCLKOUT_CFG_PBI_RESET_CLKDIVRSTN_ENABLE_VALUE    ( 0x1 )
#define GPONCLKOUT_CFG_PBI_RESET_NCORSTN_HOLD_VALUE         ( 0x0 )
#define GPONCLKOUT_CFG_PBI_RESET_NCORSTN_ENABLE_VALUE       ( 0x1 )

#define GPONCLKOUT_CFG_PBI_RESET_OFFSET ( 0x00000000 )

#define GPONCLKOUT_CFG_PBI_RESET_ADDRESS   	( GPONCLKOUT_CFG_ADDRESS + GPONCLKOUT_CFG_PBI_RESET_OFFSET )
#define GPONCLKOUT_CFG_PBI_RESET_READ( r ) 	READ_32( ( GPONCLKOUT_CFG_PBI_RESET_ADDRESS ), ( r ) )
#define GPONCLKOUT_CFG_PBI_RESET_WRITE( v ) WRITE_32( ( GPONCLKOUT_CFG_PBI_RESET_ADDRESS ), ( v ) )

typedef struct
{
	/* Reserved */
	uint32_t r1       	: 30 __PACKING_ATTRIBUTE_FIELD_LEVEL__ ;

	/* clkDivRstN */
	uint32_t clkdivrstn : 1 __PACKING_ATTRIBUTE_FIELD_LEVEL__ ;

	/* ncoRstN */
	uint32_t ncorstn	: 1 __PACKING_ATTRIBUTE_FIELD_LEVEL__ ;
}
__PACKING_ATTRIBUTE_STRUCT_END__
GPONCLKOUT_CFG_PBI_RESET ;


/********************************************************************************************/
/* Nco Config Register                                                                      */
/* This register is used to provision the Numerically Controlled Oscillator (NCO) function. */
/********************************************************************************************/

#define GPONCLKOUT_CFG_NCO_CONFIG_R2_VALUE                  ( 0x0 )
#define GPONCLKOUT_CFG_NCO_CONFIG_CFGBYPASS_DIRPASS_VALUE   ( 0x0 )
#define GPONCLKOUT_CFG_NCO_CONFIG_CFGBYPASS_BYPASS_VALUE    ( 0x1 )
#define GPONCLKOUT_CFG_NCO_CONFIG_R1_VALUE                  ( 0x0 )
#define GPONCLKOUT_CFG_NCO_CONFIG_CFGSRCOUT_NCO_VALUE       ( 0x0 )
#define GPONCLKOUT_CFG_NCO_CONFIG_CFGSRCOUT_GPON_VALUE      ( 0x1 )
#define GPONCLKOUT_CFG_NCO_CONFIG_CFGSRCOUT_XGPON_VALUE     ( 0x2 )
#define GPONCLKOUT_CFG_NCO_CONFIG_CFGSRCOUT_ZERO_VALUE      ( 0x3 )
#define GPONCLKOUT_CFG_NCO_CONFIG_CFGSRCIN_FREE_VALUE       ( 0x0 )
#define GPONCLKOUT_CFG_NCO_CONFIG_CFGSRCIN_GPON_VALUE       ( 0x1 )
#define GPONCLKOUT_CFG_NCO_CONFIG_CFGSRCIN_XGPON_VALUE      ( 0x2 )
#define GPONCLKOUT_CFG_NCO_CONFIG_CFGSRCIN_RESERVED_VALUE   ( 0x3 )
#define GPONCLKOUT_CFG_NCO_CONFIG_CFGNCOCLR_NORMAL_VALUE    ( 0x0 )
#define GPONCLKOUT_CFG_NCO_CONFIG_CFGNCOCLR_HOLD_VALUE      ( 0x1 )

#define GPONCLKOUT_CFG_NCO_CONFIG_OFFSET ( 0x00000300 )

#define GPONCLKOUT_CFG_NCO_CONFIG_ADDRESS   	( GPONCLKOUT_CFG_ADDRESS + GPONCLKOUT_CFG_NCO_CONFIG_OFFSET )
#define GPONCLKOUT_CFG_NCO_CONFIG_READ( r ) 	READ_32( ( GPONCLKOUT_CFG_NCO_CONFIG_ADDRESS ), ( r ) )
#define GPONCLKOUT_CFG_NCO_CONFIG_WRITE( v )    WRITE_32( ( GPONCLKOUT_CFG_NCO_CONFIG_ADDRESS ), ( v ) )

typedef struct
{
	/* Reserved */
	uint32_t r2        : 24 __PACKING_ATTRIBUTE_FIELD_LEVEL__ ;

	/* cfgBypass */
	uint32_t cfgbypass : 1 __PACKING_ATTRIBUTE_FIELD_LEVEL__ ;

	/* Reserved */
	uint32_t r1        : 2 __PACKING_ATTRIBUTE_FIELD_LEVEL__ ;

	/* cfgSrcOut */
	uint32_t cfgsrcout : 2 __PACKING_ATTRIBUTE_FIELD_LEVEL__ ;

	/* cfgSrcIn */
	uint32_t cfgsrcin  : 2 __PACKING_ATTRIBUTE_FIELD_LEVEL__ ;

	/* cfgNcoClr */
	uint32_t cfgncoclr : 1 __PACKING_ATTRIBUTE_FIELD_LEVEL__ ;
}
__PACKING_ATTRIBUTE_STRUCT_END__
GPONCLKOUT_CFG_NCO_CONFIG ;

#endif /* __GPONCLKOUT_CFG_H_INCLUDED */


