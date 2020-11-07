/*
 <:copyright-BRCM:2014:DUAL/GPL:standard
 
    Copyright (c) 2014 Broadcom 
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

//
// Description:	This include file contains the functions to handle any board
//              specifics (power source, GPIO, ...)
//
// File:	simio_board.c
//
//******************************************************************************

#include <bcmtypes.h>

#include "bcm_OS_Deps.h"

#include <linux/bcm_log.h>
#include "chal_types.h"
#include "chal_simio.h"

#include <simio_def_common.h>
#include "simio.h"
#include "simio_board.h"

#include "sim_export_defs.h"

//******************************************************************************
//                      Constant and Macro Definition
//******************************************************************************
#define _DBG_(a) a      /* Enable SIM enhancement logging */
//#define _DBG_(a)      /* Disable SIM enhancement logging */

#define SIMIO_WAITTIME          100 // wait not more than 100ms

static SIMIO_LDO_CB_t board_ldo_cb;

/*extern*/ Boolean calMode;

// Wait VCC stable on (on_off=TRUE) or VCC stable off (on_off=FALSE)
// Return TRUE if we get what we wanted.
static Boolean SIMIO_VCC_WAIT(SIMIO_ID_t id, Boolean on_off, UInt32 timeout)
{     
#ifndef NO_PMU
    UInt32 SimioWaitTime;

    if (calMode)
    {
        SimioWaitTime = TIMER_GetValue();
        
        while (PMU_DRV_IsSIMReady((PMU_SIMLDO_t)id) != on_off) {
            if(TIMER_GetValue() > (SimioWaitTime + timeout)) 
            {
                return FALSE;
            }
            OSTASK_Sleep(TICKS_ONE_SECOND / 200);
        }     
    }
    else
    {
        if (board_ldo_cb.isReady)
        {
            SimioWaitTime = TIMER_GetValue();
        
            while (board_ldo_cb.isReady(id) != on_off) {
                if(TIMER_GetValue() > (SimioWaitTime + timeout)) 
                {
                    return FALSE;
                }
                OSTASK_Sleep(TICKS_ONE_SECOND / 200);
            }     
        }
    }
#endif    
    return TRUE;
}    

 
void SIMIO_Board_SetLDO_CB(SIMIO_LDO_CB_t cb)
{
    board_ldo_cb = cb;
}

//******************************************************************************
//
// Function Name:   SIMIO_Board_Init
//
// Description:
//
//******************************************************************************
Boolean SIMIO_Board_Init(SIMIO_ID_t id)
{

    if (SIMIO_Board_Emergency_Shutdown(id)){
    // Need to setup GPIO path for SIM emergency shutdown logic
    // This is already done for VOYAGER in pmu_bcm59036.i: it conditionally calls
    // PMU_DRV_BatRemovalIntInit() to setup GPIO. Need update the code when other
    // boards start supporting this SIM feature.
    
    // We decide not to call PMU_RegisterBattmgrCallbacks() to subscribe battery
    // removal event PMU_DRV_REGISTER_BATREM_CB. No driver owns this event yet.
    // Battery driver might need it and we don't want to steal it.
    }

    return TRUE;
}

//******************************************************************************
//
// Function Name:   SIMIO_Board_Emergency_Shutdown
//
// Description:
//
//******************************************************************************
Boolean SIMIO_Board_Emergency_Shutdown(SIMIO_ID_t id)
{
    // SIM emergency shutdown requires 2 hardware features and 1 board feature
    // 1. Baseband SIM controller must be able to inactivate SIMIO lines per the
    //    7816 specification when the connected GPIO (GPIO19) is asserted LOW
    // 2. PMU must be able to detect battery removal, assert an output line which
    //    is connected to a baseband GPIO pin, and shutdown SIM Vcc
    // 3. Board must connect PMU battery removal pin to baseband GPIO19

    // Only available on certain boards
#ifdef THUNDERBIRD
    // only FFB has GPIO19 connected to PMU
    return TRUE;
#else
    return FALSE;
#endif
}

//******************************************************************************
//
// Function Name:   SIMIO_Board_Cleanup
//
// Description:
//
//******************************************************************************
Boolean SIMIO_Board_Cleanup(SIMIO_ID_t id)
{
    return TRUE;
}

//******************************************************************************
//
// Function Name:   SIMIO_Board_Voltage_On
//
// Description: Set the SIM voltage on VCC
//
//******************************************************************************
Boolean SIMIO_Board_Voltage_On(SIMIO_ID_t id, SimVoltageLevel_t sim_voltage)
{
    Boolean ret = TRUE;

    _DBG_( SIM_LOGV("SIMIO_Board_Voltage_On:",sim_voltage) );
    _DBG_( SIM_LOGV("SIMIO_Board_Voltage_On:",board_ldo_cb.activate) );

    if (calMode)
    {
        switch( sim_voltage )
        {
            case SIM_1P8V:
                _DBG_(SIM_LOG("SIMIO_Board_Voltage_On: SIM_1P8V"));
                _DBG_( SIM_LOGV("SIMIO_Board_Voltage_On: id: ",id) );
                PMU_DRV_ActivateSIM((PMU_SIMLDO_t)id, PMU_SIM1P8Volt);
                break;
            case SIM_3V:
                _DBG_(SIM_LOG("SIMIO_Board_Voltage_On: SIM_3V"));
                _DBG_( SIM_LOGV("SIMIO_Board_Voltage_On: id: ",id) );
                PMU_DRV_ActivateSIM((PMU_SIMLDO_t)id, PMU_SIM3P0Volt);
                break;
            default:
                _DBG_(SIM_LOG("SIMIO_Board_Voltage_On: voltage not supported"));
                break;
        }
    }
    else
    {
        if (board_ldo_cb.activate) 
        {
            ret = board_ldo_cb.activate(id, sim_voltage); 
            _DBG_( SIM_LOGV("SIMIO_Board_Voltage_On: activate ", ret) );        
        }    	
    }

    if (ret)
        if (SIMIO_VCC_WAIT(id, TRUE, SIMIO_WAITTIME) == FALSE)
        {
            _DBG_( SIM_LOG("SIMIO_Board_Voltage_On: timeout waiting vcc on") );
            ret = FALSE;
        }

    if (ret)
        // Wait 10MS for voltage stable
        OSTASK_Sleep(TICKS_ONE_SECOND / 100);

    return ret;
}

//******************************************************************************
//
// Function Name:   SIMIO_Board_Voltage_Off
//
// Description: Set the SIM voltage off
//
//******************************************************************************
Boolean SIMIO_Board_Voltage_Off(SIMIO_ID_t id)
{
    Boolean ret = TRUE;

    _DBG_( SIM_LOG("SIMIO_Board_Voltage_Off") );

    if (calMode)
    {
        PMU_DRV_ActivateSIM((PMU_SIMLDO_t)id, PMU_SIM0P0Volt);
    }
    else
    {
        if (board_ldo_cb.activate)
            ret = board_ldo_cb.activate(id, SIM_0V); 
    }
    
    if (ret)
        if (SIMIO_VCC_WAIT(id, FALSE, SIMIO_WAITTIME) == FALSE)
        {
            _DBG_( SIM_LOG("SIMIO sim_voltage_off: timeout waiting vcc off") );
            ret = FALSE;
        }

    return ret;
}

//******************************************************************************
//
// Function Name:   SIMIO_Board_GetVoltageClass
//
// Description: Return the voltage class the board supports
//
//******************************************************************************
SimVoltageClass_t SIMIO_Board_GetVoltageClass(SIMIO_ID_t id)
{
#ifdef NO_PMU
	return SIM_SUPPORT_3V_ONLY;
#else
	return SIM_SUPPORT_1P8V_3V;
#endif    
}

