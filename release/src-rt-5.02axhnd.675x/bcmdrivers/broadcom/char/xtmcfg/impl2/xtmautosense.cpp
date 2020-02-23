/*
    <:copyright-BRCM:2011:proprietary:standard
    
       Copyright (c) 2011 Broadcom 
       All Rights Reserved
    
     This program is the proprietary software of Broadcom and/or its
     licensors, and may only be used, duplicated, modified or distributed pursuant
     to the terms and conditions of a separate, written license agreement executed
     between you and Broadcom (an "Authorized License").  Except as set forth in
     an Authorized License, Broadcom grants no license (express or implied), right
     to use, or waiver of any kind with respect to the Software, and Broadcom
     expressly reserves all rights in and to the Software and all intellectual
     property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
     NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
     BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
    
     Except as expressly set forth in the Authorized License,
    
     1. This program, including its structure, sequence and organization,
        constitutes the valuable trade secrets of Broadcom, and you shall use
        all reasonable efforts to protect the confidentiality thereof, and to
        use this information only in connection with your use of Broadcom
        integrated circuit products.
    
     2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
        AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
        WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
        RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
        ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
        FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
        COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
        TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
        PERFORMANCE OF THE SOFTWARE.
    
     3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
        ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
        INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
        WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
        IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
        OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
        SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
        SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
        LIMITED REMEDY.
    :>    


*/
/***************************************************************************
 * File Name  : xtmautosense.cpp
 *
 * Description: This file contains the implementation for the XTM autosense
 *  class.
 ***************************************************************************/

#include "xtmcfgimpl.h"
#include "board.h"



////////////////////////////////////////////////////////////////////////////////
// Tunable operational parameters
////////////////////////////////////////////////////////////////////////////////
#define XTMAUTOSENSE_ONELINETIMEOUTMS      30000         // Timeout to determine if only one line is configured
#define XTMAUTOSENSE_BONDTRAFFICTIMEOUTMS 130000         // Timeout for bonding indication traffic


////////////////////////////////////////////////////////////////////////////////
// Public methods for this class
////////////////////////////////////////////////////////////////////////////////


/***************************************************************************
 * Function Name: XTM_AUTOSENSE
 * Description  : Constructor for the XTM autosense class.
 * Returns      : None.
 ***************************************************************************/
XTM_AUTOSENSE::XTM_AUTOSENSE() {
    // Do nothing - this is a stub that is not invoked because we do not use
    //  the normal C++ constuction technique ('new') but instead our own
    //  allocation and casting of type.
}
	
/***************************************************************************
 * Function Name: ~XTM_AUTOSENSE
 * Description  : Destructor for the XTM autosense class.
 * Returns      : None.
 ***************************************************************************/
XTM_AUTOSENSE::~XTM_AUTOSENSE( void )
{
    Uninitialize();
} /* ~XTM_AUTOSENSE */

/***************************************************************************
 * Function Name: initializeVars()
 * Description  : Initialize internal variables based on caller.
 * Returns      : None.  
 * Note         : Call after initialize() 
 ***************************************************************************/
void XTM_AUTOSENSE::initializeVars( bool bEnableAutosense) {
    // Set stuff per parameters
    m_bAutoSenseEnabled = bEnableAutosense;     // Should autosense be enabled?
                                                // (Default false in initialize())
}

/***************************************************************************
 * Function Name: Initialize()
 * Description  : INitialize object members as a constructor would normally do.
 * Returns      : XTMSTS_SUCCESS on success. 
 * Parameters   : pInitParms - Indicates default parameters are to be used.
 * Note         : Call before initializeVars() 
 ***************************************************************************/
BCMXTM_STATUS XTM_AUTOSENSE::Initialize( PXTM_INITIALIZATION_PARMS pInitParms ) {

   // Do we need to initialize parameters to defaults because this is a first pass?
   if(pInitParms) {
       // Set operational parameter defaults
       m_ulOneLineTimeoutMs = XTMAUTOSENSE_ONELINETIMEOUTMS;
       m_ulBondingTrafficTimeoutMs = XTMAUTOSENSE_BONDTRAFFICTIMEOUTMS;
   }

   // Set internal variables
   m_bAutoSenseEnabled = false;                 // By default, do not enable autosense
                                                // (May be overriden by parameter in initializeVars())
   m_eCurrentState = BOTH_LINES_IDLE;           // Current state of state machine
   //XtmOsPrintf ("[XTM]m_eCurrentState = %d\n", m_eCurrentState) ;
   for (int i=0; i<MAX_BONDED_LINES; i++) 
      m_pbLineActivityDetected[i] = false;
   m_bPhyChangePending = false;                 // One shot flag indicates PHY needs to change
   //timerStrike = 0;
   return XTMSTS_SUCCESS;
}

/***************************************************************************
 * Function Name: Uninitialize()
 * Description  : Unused stub for future expansion.
 * Returns      : XTMSTS_SUCCESS on success.
 ***************************************************************************/
BCMXTM_STATUS XTM_AUTOSENSE::Uninitialize( void ) {
    // Do nothing - this is a stub for future expansion
    return XTMSTS_SUCCESS;
}


/***************************************************************************
 * Function Name: TimerUpdate
 * Description  : Perform periodic tasks.  Called by XTM_PROCESSOR timer.
 * Returns      : None.
 ***************************************************************************/
void XTM_AUTOSENSE::TimerUpdate() {
    // Check for transitions based on timer
    //timerStrike++ ;
    //if (!(timerStrike % 1000))
       //XtmOsPrintf ("TimerUpdate %d \n", timerStrike) ;
    Transition(TRANSITION_TIMERSTRIKE);
}



/***************************************************************************
 * Function Name: SetOneLineTimeoutMs()
 * Description  : Set timeout to determine if only one line is configured.
 * Returns      : None
 ***************************************************************************/
void XTM_AUTOSENSE::SetOneLineTimeoutMs(unsigned long ulTimeoutMs) {
   m_ulOneLineTimeoutMs = ulTimeoutMs;
}


/***************************************************************************
 * Function Name: SetBondingTrafficTimeoutMs()
 * Description  : Set timeout for bonding indication traffic.
 * Returns      : None
 ***************************************************************************/
void XTM_AUTOSENSE::SetBondingTrafficTimeoutMs(unsigned long ulTimeoutMs) {
   m_ulBondingTrafficTimeoutMs = ulTimeoutMs;
}


/***************************************************************************
 * Function Name: GetOneLineTimeoutMs()
 * Description  : Get timeout to determine if only one line is configured.
 * Returns      : Timeout in Ms
 ***************************************************************************/
unsigned long XTM_AUTOSENSE::GetOneLineTimeoutMs(void) {
   return(m_ulOneLineTimeoutMs);
}


/***************************************************************************
 * Function Name: GetOneLineTimeoutMs()
 * Description  : Get timeout for bonding indication traffic.
 * Returns      : Timeout in Ms
 ***************************************************************************/
unsigned long XTM_AUTOSENSE::GetBondingTrafficTimeoutMs(void) {
   return(m_ulBondingTrafficTimeoutMs);
}



/***************************************************************************
 * Function Name: LineActive()
 * Description  : Record indication of activity on a line.
 * Returns      : Result of transition (XTMSTS_SUCCESS on success)
 ***************************************************************************/
int XTM_AUTOSENSE::LineActive(UINT32 ulPhysPort) {
    //XtmOsPrintf("DEBUG(%s): ulPhysPort=%lu, lineActivity[] = [%d,%d]\n", __func__, ulPhysPort, m_pbLineActivityDetected[0], m_pbLineActivityDetected[1]);

    // Check input parameters
    if(ulPhysPort < MAX_BONDED_LINES) {
        // Record this line as active
        m_pbLineActivityDetected[ulPhysPort] = true;

        // Now, send appropriate transition to state machine.
        if(m_pbLineActivityDetected[0] && m_pbLineActivityDetected[1]) 
            return Transition(TRANSITION_BOTH_LINES_NONIDLE);
        else
            return Transition(TRANSITION_ONE_LINE_NONIDLE);
    }
    else {
        // Flag and return error
        XtmOsPrintf("ERROR(%s): Invalid port ID passed - ulPhysPort=%lu\n", __func__, ulPhysPort);
        return(-1);
    }
}


/***************************************************************************
 * Function Name: LineIdle()
 * Description  : Record indication of line going idle.
 * Returns      : Result of transition (XTMSTS_SUCCESS on success)
 ***************************************************************************/
int XTM_AUTOSENSE::LineIdle(UINT32 ulPhysPort) {
    //XtmOsPrintf("DEBUG(%s): ulPhysPort=%lu\n", __func__, ulPhysPort);

    // Check input parameters
    if(ulPhysPort < MAX_BONDED_LINES) {

        // Record this line as idle
        m_pbLineActivityDetected[ulPhysPort] = false;

        // Now, send appropriate transition to state machine.
        if(!m_pbLineActivityDetected[0] && !m_pbLineActivityDetected[1]) 
            return Transition(TRANSITION_BOTH_LINES_IDLE);
        else
            return Transition(TRANSITION_ONE_LINE_NONIDLE);

        // Return good value
        return(0);
    }
    else {
        // Flag and return error
        XtmOsPrintf("ERROR(%s): Invalid port ID passed - ulPhysPort=%lu\n", __func__, ulPhysPort);
        return(-1);
    }
}


/***************************************************************************
 * Function Name: BondingIndicationRx()
 * Description  : Record bonding indication (either an ASM or a G.HS with 
 *                bonding bit set) recieved.
 * Returns      : Result of transition (XTMSTS_SUCCESS on success)
 ***************************************************************************/
int XTM_AUTOSENSE::BondingIndicationRx() {
    return(Transition(TRANSITION_BONDING_INDICATION_RX));
}


/***************************************************************************
 * Function Name: ForceBondingPhy()
 * Description  : Override all other states and force the Bonding PHY.
 * Returns      : Result of transition (XTMSTS_SUCCESS on success)
 ***************************************************************************/
int XTM_AUTOSENSE::ForceBondingPhy() {
    return(Transition(TRANSITION_FORCEBONDING));
}


/***************************************************************************
 * Function Name: ForceSingleLinePhy()
 * Description  : Override all other states and force Single Line PHY.
 * Returns      : Result of transition (XTMSTS_SUCCESS on success)
 ***************************************************************************/
int XTM_AUTOSENSE::ForceSingleLinePhy() {
    return(Transition(TRANSITION_FORCESINGLELINE));
}


/***************************************************************************
 * Function Name: NonAtmMode()
 * Description  : Override all other states and indicate not in an ATM 
 *                mode (i.e. PTM).  Reset state machine.
 * Returns      : Result of transition (XTMSTS_SUCCESS on success)
 ***************************************************************************/
int XTM_AUTOSENSE::NonAtmMode() {
    return(Transition(TRANSITION_NON_ATM_TRAFFIC));
}



//////////////////
// Status methods
//////////////////

/***************************************************************************
 * Function Name: PhyStatus
 * Description  : Read state machine and determine which PHY to use.
 * Returns      : Enum XtmAutoSensePhyStatus indicating to use 
 *                Single Line PHY, Bonding PHY, that the PHY has not 
 *                yet been calculated, or that Autosense is not enabled.
 ***************************************************************************/
XtmAutoSensePhyStatus XTM_AUTOSENSE::PhyStatus(void) {

    // Is this feature enabled?
    if(m_bAutoSenseEnabled == false) {
        // No - return disabled value
        return(PHYSTATUS_AUTOSENSE_DISABLED);
    }

    // Return value based on current state
    switch(m_eCurrentState) {
    case SINGLE_LINE_DATA_OPERATION:
        return PHYSTATUS_ATM_SINGLELINE;
        break;

    case BONDING_DATA_OPERATION:
        return PHYSTATUS_ATM_BONDING;
        break;

    default:
        return PHYSTATUS_UNKNOWN;
        break;
    }
}

/***************************************************************************
 * Function Name: PhyChangePending
 * Description  : Return and clear flag indicating PHY needs to change.
 * Returns      : Flag indicating PHY change needed.
 ***************************************************************************/
bool XTM_AUTOSENSE::PhyChangePending(void) {
    bool bTemp = m_bPhyChangePending;   // Save flag value so we can clear it`

    // Clear flag
    m_bPhyChangePending = false;

    // Return flag
    return(bTemp);
}
	
    	
////////////////////////////////////////////////////////////////////////////////
// Private methods for this class
////////////////////////////////////////////////////////////////////////////////

/***************************************************************************
 * Function Name: Transition
 * Description  : Core of the state machine.  Handle all state machine 
 *                transitions.
 * Returns      : Result of transition (XTMSTS_SUCCESS on success).
 ***************************************************************************/
int XTM_AUTOSENSE::Transition(XtmAutoSenseTransition eTransition) {
   int iRetvalue = 0;           // Return value
   UINT32 ulCurrTimestampMs;  // Current time for timeouts
   XtmAutoSenseState eOriginalState = m_eCurrentState;           // Save current state

   // Is this feature enabled?
   if(m_bAutoSenseEnabled == false) {
       // No - just quit
       return(XTMSTS_ERROR);
   }

   // Capture the current time
   ulCurrTimestampMs = XtmOsGetTimeStampMs();

   // Act depending on the type of transition
   switch(eTransition) {
   case TRANSITION_NON_ATM_TRAFFIC:
       // Not using ATM (most likely PTM).  Force us back to start state.
       m_eCurrentState = BOTH_LINES_IDLE;
       //XtmOsPrintf ("[XTM]m_eCurrentState = %d\n", m_eCurrentState) ;
       break;

   case TRANSITION_BOTH_LINES_IDLE:
       // Force us back to start state
       m_eCurrentState = BOTH_LINES_IDLE;
       //XtmOsPrintf ("[XTM]m_eCurrentState = %d\n", m_eCurrentState) ;
       break;

   case TRANSITION_ONE_LINE_NONIDLE:
       // Are we in a state where we care about lines going idle?
       if(m_eCurrentState == BOTH_LINES_IDLE) {
           // Set up timeout for Single Line PHY
           m_ulOneLineStartedTimestampMs = ulCurrTimestampMs;
           XtmOsPrintf ("m_ulOneLineStartedTimestampMs = %d\n", m_ulOneLineStartedTimestampMs) ;

           // Set state to wait for timeout
           m_eCurrentState = ONE_LINE_NONIDLE;
           //XtmOsPrintf ("[XTM]m_eCurrentState = %d\n", m_eCurrentState) ;
       }
       break;

   case TRANSITION_BOTH_LINES_NONIDLE:
       // Are we already in bonding or single line data operations?
       if((m_eCurrentState != BONDING_DATA_OPERATION) && (m_eCurrentState != SINGLE_LINE_DATA_OPERATION)) {
           // Not yet.  Set state to wait for timeout for single line or bonding.
           m_eCurrentState = BOTH_LINES_NONIDLE;
          // XtmOsPrintf ("[XTM]m_eCurrentState = %d\n", m_eCurrentState) ;
       }
       break;

   case TRANSITION_BONDING_INDICATION_RX:
       // Set state to reflect Bonding PHY operation
       m_eCurrentState = BONDING_DATA_OPERATION;
       //XtmOsPrintf ("[XTM]m_eCurrentState = %d\n", m_eCurrentState) ;
       break;

   case TRANSITION_TIMERSTRIKE:
       // Are we waiting on timeout for nonidle activity on both lines?
       if((m_eCurrentState == ONE_LINE_NONIDLE) 
                    && 
          ((((signed long) (ulCurrTimestampMs - m_ulOneLineStartedTimestampMs)) & 0x7fffffff) > m_ulOneLineTimeoutMs)
          )
         {
           // Set state to reflect Single Line PHY operation
           m_eCurrentState = SINGLE_LINE_DATA_OPERATION;
           XtmOsPrintf ("[XTM]m_eCurrentState = %u, m_ulOneLineTimeoutMs = %u, m_ulOneLineStartedTimestampMs = %u, ulCurrTimestampMs = %u \n",
                          m_eCurrentState, m_ulOneLineTimeoutMs, m_ulOneLineStartedTimestampMs, ulCurrTimestampMs) ;
       }

       // Are we waiting on bonding traffic timeout?
       if((m_eCurrentState == BOTH_LINES_NONIDLE) 
                    && 
          ((((signed long) (ulCurrTimestampMs - m_ulOneLineStartedTimestampMs)) & 0x7fffffff) > m_ulBondingTrafficTimeoutMs)
          )
         {
           // Set state to reflect Single Line PHY operation
           XtmOsPrintf ("[XTM]m_eCurrentState = %u, m_ulBondingTrafficTimeoutMs = %u, m_ulOneLineStartedTimestampMs = %u, ulCurrTimestampMs = %u \n", 
                          m_eCurrentState, m_ulBondingTrafficTimeoutMs, m_ulOneLineStartedTimestampMs, ulCurrTimestampMs) ;
           m_eCurrentState = SINGLE_LINE_DATA_OPERATION;
       }
       break;

   case TRANSITION_FORCEBONDING:
       // Override all other states and force Bonding PHY
       m_eCurrentState = BONDING_DATA_OPERATION;
       //XtmOsPrintf ("[XTM]m_eCurrentState = %d\n", m_eCurrentState) ;
       break;
       
   case TRANSITION_FORCESINGLELINE:
       // Override all other states and force Single Line PHY
       m_eCurrentState = SINGLE_LINE_DATA_OPERATION;
       //XtmOsPrintf ("[XTM]m_eCurrentState = %d\n", m_eCurrentState) ;
       break;
   }

   // Now, determine if we changed state
   if(eOriginalState != m_eCurrentState) {

       //XtmOsPrintf("DEBUG(%s): Transition %d causes state change from %d to %d.\n", __func__, eTransition, eOriginalState, m_eCurrentState);

       // Yes.  Perform actions for certain states.
       switch(m_eCurrentState) {
       case SINGLE_LINE_DATA_OPERATION:
           // Announce PHY
           XtmOsPrintf("bcmxtmcfg: Autosense state machine commands Single Line PHY\n");

           // Set PHY penging and SAR reconfig flags
           m_bPhyChangePending = true;
           break;
       case BONDING_DATA_OPERATION:
           // Announce PHY
           XtmOsPrintf("bcmxtmcfg: Autosense state machine commands Bonding PHY\n");

           // Set PHY penging and SAR reconfig flags
           m_bPhyChangePending = true;
           break;

       default:
           // Insert any new actions on states here
           break;
       }
   }

   return(iRetvalue);
}
