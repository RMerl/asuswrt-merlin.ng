/*
 <:copyright-BRCM:2014:DUAL/GPL:standard
 
    Copyright (c) 2014 Broadcom 
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

/**
	@file
	@brief	This include file contains the common constanst and enum declarration  
			for user kernel communication. 

	@defgroup SIMDeviceDrivers SIM Device Drivers
	@brief This group is the device driver interface to the SIM.
	@ingroup DeviceDriverGroup
*/
#ifndef _SIMIO_DEF_COMMON_H_
#define _SIMIO_DEF_COMMON_H_

/* Supported SIMIO IDs */
typedef enum
{
    SIMIO_ID_0 = 0,    ///< first SIMIO controller 
    SIMIO_ID_1 = 1     ///< second SIMIO controller
} SIMIO_ID_t;

typedef enum
{
    SIM_PROTOCOL_T0,        //  Protocol T=0, Asynchronous, half duplex char transmission protocol
    SIM_PROTOCOL_T1         //  Protocol T=1, Asynchronous, half duplex block transmission protocol
} PROTOCOL_t;

typedef enum
{
    SIMIO_CLK_4P16MHZ,   // SIM interface clock 25MHz, SIM clock 4.16MHz
    SIMIO_CLK_3P12HZ,   // SIM interface clock 25MHz, SIM clock 
    SIMIO_CLK_2P5MHZ,   // SIM interface clock 25MHz, SIM clock 
    SIMIO_CLK_2P08MHZ,   // SIM interface clock 25MHz, SIM clock 
    SIMIO_CLK_1P78MHZ,   // SIM interface clock 25MHz, SIM clock 
    SIMIO_CLK_1P5MHZ,   // SIM interface clock 25MHz, SIM clock 
    SIMIO_CLK_1P3MHZ,   // SIM interface clock 25MHz, SIM clock 
    SIMIO_CLK_1P25MHZ,   // SIM interface clock 25MHz, SIM clock 
    SIMIO_CLK_1P13MHZ,   // SIM interface clock 25MHz, SIM clock 
    SIMIO_CLK_1P04MHZ,   // SIM interface clock 25MHz, SIM clock 
    SIMIO_CLK_LAST,
} SIMIO_DIVISOR_t;

/* Current voltage level of VCC */
typedef enum
{
    SIM_5V,			// 5V
    SIM_3V,			// 3V
    SIM_1P8V,       // 1.8V 
    SIM_0V			// 0V
} SimVoltageLevel_t;

typedef enum 
{
    SIMIO_SIGNAL_IDLE,					// Idle state
    SIMIO_SIGNAL_SIMRESET,				// Sim reset
    SIMIO_SIGNAL_TIMEOUT,				// Sim access timeout
    SIMIO_SIGNAL_SIMINSERT, 			// Sim insert state
    SIMIO_SIGNAL_RSPDATA,				// response data
    SIMIO_SIGNAL_SIMREMOVED, 			// SIM Removed
    SIMIO_SIGNAL_ATRCORRUPTED,			// Corrupted ATR
    SIMIO_SIGNAL_T1_PARITY,				// T1 Parity
    SIMIO_SIGNAL_T1_INVALID_LENGTH,		// T1 block length invalid
    SIMIO_SIGNAL_T1_BWT_TIME_OUT,		// T1 BWT time out
    SIMIO_SIGNAL_ATR_WRONG_VOLTAGE		// Wrong Voltage
} SIMIO_SIGNAL_t;	

#define SIM_CARD_MAX_BUFFER_SIZE        300
#define SIM_CARD_MAX_CHUNK_SIZE         0xFF

#endif

