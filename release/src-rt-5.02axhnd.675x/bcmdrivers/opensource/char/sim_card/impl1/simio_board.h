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

//
//  Header:  simio_board.h
//

#ifndef __SIMIO_BOARD_H
#define __SIMIO_BOARD_H


void SIMIO_Board_SetLDO_CB(SIMIO_LDO_CB_t cb);

Boolean SIMIO_Board_Init(SIMIO_ID_t id);
Boolean SIMIO_Board_Cleanup(SIMIO_ID_t id);

Boolean SIMIO_Board_Emergency_Shutdown(SIMIO_ID_t id);

Boolean SIMIO_Board_Voltage_On(SIMIO_ID_t id, SimVoltageLevel_t sim_voltage);
Boolean SIMIO_Board_Voltage_Off(SIMIO_ID_t id);

SimVoltageClass_t SIMIO_Board_GetVoltageClass(SIMIO_ID_t id);


#endif

