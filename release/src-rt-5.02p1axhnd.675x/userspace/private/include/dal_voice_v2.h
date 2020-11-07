/*
*
*  Filename: dal_voice.h
*
****************************************************************************
*  Description:
*
*
****************************************************************************/

#ifndef __DAL_VOICE_V2_H__
#define __DAL_VOICE_V2_H__

#define LINE_EXT_BUF_SIZE_MAX 16

typedef struct {
    int    Idx;
    char  *name;
} DAL_VOICE_CODEC_LIST;

typedef struct {
    int    cpIdx;  /* codec profile index */
    int    codecIdx; /* codec index */
} DAL_VOICE_CP_LIST;

/* Map MDM stats object structure to DAL, and map this to a VOICE structure for use outside of DAL */
typedef struct
{
    UBOOL8    resetStatistics;  /**< ResetStatistics */ 
    UINT32    packetsSent;  /**< PacketsSent */ 
    UINT32    packetsReceived;  /**< PacketsReceived */ 
    UINT32    bytesSent;    /**< BytesSent */ 
    UINT32    bytesReceived;    /**< BytesReceived */ 
    UINT32    packetsLost;  /**< PacketsLost */ 
    UINT32    incomingCallsReceived;    /**< IncomingCallsReceived */ 
    UINT32    incomingCallsAnswered;    /**< IncomingCallsAnswered */ 
    UINT32    incomingCallsConnected;   /**< IncomingCallsConnected */ 
    UINT32    incomingCallsFailed;  /**< IncomingCallsFailed */
    UINT32    incomingCallsDropped;
    UINT32    incomingTotalCallTime;  /* In seconds */
    UINT32    outgoingCallsAttempted;   /**< OutgoingCallsAttempted */ 
    UINT32    outgoingCallsAnswered;    /**< OutgoingCallsAnswered */ 
    UINT32    outgoingCallsConnected;   /**< OutgoingCallsConnected */ 
    UINT32    outgoingCallsFailed;  /**< OutgoingCallsFailed */
    UINT32    outgoingCallsDropped;
    UINT32    outgoingTotalCallTime;  /* In seconds */
    UINT32    receivePacketLossRate;    /**< ReceivePacketLossRate */ 
    UINT32    farEndPacketLossRate; /**< FarEndPacketLossRate */ 
    UINT32    receiveInterarrivalJitter;    /**< ReceiveInterarrivalJitter */ 
    UINT32    farEndInterarrivalJitter; /**< FarEndInterarrivalJitter */ 
    UINT32    roundTripDelay;   /**< RoundTripDelay */ 
    UINT32    averageReceiveInterarrivalJitter; /**< AverageReceiveInterarrivalJitter */ 
    UINT32    averageFarEndInterarrivalJitter;  /**< AverageFarEndInterarrivalJitter */ 
    UINT32    averageRoundTripDelay;    /**< AverageRoundTripDelay */ 
} VoiceStatsObject;

typedef VoiceStatsObject DAL_VOICE_CALL_STATS_BLK;
typedef DAL_VOICE_CALL_STATS_BLK VOICE_CALL_STATS_BLK;

/*================================= Public Function Prototypes ================================*/

/***************************************************************************
** Function Name: dalVoice_Save
** Description  : Saves mdm to flash
**
** Parameters   :
** Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_Save(void);

#ifdef DMP_X_BROADCOM_COM_PSTNENDPOINT_1

CmsRet dalVoice_mapFxoInterfaceIDToPstnInst(int id, int *inst);

/*****************************************************************
**  FUNCTION:       dalVoice_GetVoiceFxoPhyInterfaceList
**
**  PURPOSE:
**
**  INPUT PARMS:    none
**
**  OUTPUT PARMS:   list  - list of physical endpoint ID
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVoiceFxoPhyInterfaceList(DAL_VOICE_PARMS *parms, char *list, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_mapCmPstnLineToPstnInst
* Description  : This returns the Voice Profile instance number corresponding
*                to a certain service provider index.
*
* Parameters   : cmPstnLineNum (IN)     - pstnLineNumber
*                pstnInst (OUT)         - pointer to mdm pstnInst
* Returns      : CMSRET_SUCCESS when successfule.
****************************************************************************/
CmsRet dalVoice_mapCmPstnLineToPstnInst ( int cmPstnLineNum, int * pstnInst );

/*****************************************************************
**  FUNCTION:       dalVoice_GetVlFxoPhyReferenceList
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0];
**                  lineInst  - parms->op[1]
**
**  OUTPUT PARMS:   physEptId - Fxo Physical endpoint ID
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVlFxoPhyReferenceList(DAL_VOICE_PARMS *parms, char *physEptId, unsigned int length );

#endif

/***************************************************************************
* Function Name: dalVoice_SetDefaults
* Description  : Sets up default values to setup IAD in peer-peer mode.
*
* Parameters   : None ( parameters are ignored );
* Returns      : CMSRET_SUCCESS when successfule.
****************************************************************************/
CmsRet dalVoice_SetDefaults( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_mapSpNumToVpInst
* Description  : This returns the Voice Profile instance number corresponding
*                to a certain service provider index.
*
* Parameters   : spNum (IN)     - service provider index
*                vpInst (OUT)   - pointer to VoiceProfile instance number
* Returns      : CMSRET_SUCCESS when successfule.
****************************************************************************/
CmsRet dalVoice_mapSpNumToVpInst ( int spNum, int * vpInst );

#ifdef  DMP_VOICE_SERVICE_2
/***************************************************************************
* Function Name: dalVoice_mapSpNumToSvcInst
* Description  : This returns the Voice Service instance number corresponding
*                to a certain service provider index.
*
* Parameters   : spNum (IN)     - service provider index
*                vpInst (OUT)   - pointer to VoiceProfile instance number
* Returns      : CMSRET_SUCCESS when successfule.
****************************************************************************/
CmsRet dalVoice_mapSpNumToSvcInst ( int spNum, int * vpInst );

/***************************************************************************
* Function Name: dalVoice_MapNumVpToInst
* Description  : Map number of voip profile to instance number
*
* Parameters   : parms->op[0] = voice service instance, value = value to be set
*              : parms->op[1] (IN) = number of voip profiles (zero based)
* Returns      : CMSRET_SUCCESS when successful.
*              : inst (OUT)= voip profile instance ( 1 based )
****************************************************************************/
CmsRet dalVoice_mapVoipProfNumToInst( DAL_VOICE_PARMS *parms, int *vpInst );

/*****************************************************************
**  FUNCTION:       dalVoice_AddVoipProfile
**
**  PURPOSE:        Adds a VoIP profile object in MDM
**
**  INPUT PARAMS:   svc inst - parms->op[0]
**
**  OUTPUT PARMS:   instance of the added object
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_AddVoipProfile( DAL_VOICE_PARMS *parms, int *inst );
CmsRet dalVoice_AddCodecProfile( DAL_VOICE_PARMS *parms, int *inst );
/*****************************************************************
**  FUNCTION:       dalVoice_AddSipNetwork
**
**  PURPOSE:        Adds SIP network object
**
**  INPUT PARMS:    voice service instance    - parms->op[0]
**
**  OUTPUT PARMS:   added object instance
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_AddSipNetwork( DAL_VOICE_PARMS *parms, int *inst );
CmsRet dalVoice_AddSipClient( DAL_VOICE_PARMS *parms, int *inst );
/*****************************************************************
**  FUNCTION:       dalVoice_AddCallCtrlLine
**
**  PURPOSE:        Adds a call control line object to the given voice service instance
**
**  INPUT PARMS:    op[0] - voice service instance
**
**  OUTPUT PARMS:   inst - instance number of a created object
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_AddCallCtrlLine( DAL_VOICE_PARMS *parms, int *inst );

/*****************************************************************
**  FUNCTION:       dalVoice_AddCallCtrlExt
**
**  PURPOSE:        Adds a call control extension object to the given voice service instance
**
**  INPUT PARMS:    op[0] - voice service instance
**
**  OUTPUT PARMS:   inst - instance number of a created object
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_AddCallCtrlExt( DAL_VOICE_PARMS *parms, int *inst );

/*****************************************************************
**  FUNCTION:       dalVoice_AddCallFeatureSet
**
**  PURPOSE:        creates a calling features set object in MDM
**
**  INPUT PARMS:    parms->op[0] - voice service instance
**
**  OUTPUT PARMS:   inst         - pointer to integer where instance ID 
**                                 of the created object will be stored
**
**  RETURNS:        CMSRET_SUCCESS - object creation success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_AddCallFeatureSet( DAL_VOICE_PARMS *parms, int *inst );

/*****************************************************************
**  FUNCTION:       dalVoice_DeleteVoipProfile
**
**  PURPOSE:        Deletes a VoIP profile object in MDM
**
**  INPUT PARAMS:   svc inst - parms->op[0]
**                  VoIP profile instance to be deleted - parms->op[1]
**
**  OUTPUT PARMS:   none
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_DeleteVoipProfile(DAL_VOICE_PARMS *parms);

/*****************************************************************
**  FUNCTION:       dalVoice_DeleteSipNetwork
**
**  PURPOSE:        Deletes a SIP network object
**
**  INPUT PARMS:    op[0] - voice service instance
**                  op[1] - instance of the SIP network object to delete 
**
**  OUTPUT PARMS:   none
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_DeleteSipNetwork( DAL_VOICE_PARMS *parms );

/*****************************************************************
**  FUNCTION:       dalVoice_DeleteSipClient
**
**  PURPOSE:        Deletes a SIP client object
**
**  INPUT PARMS:    op[0] - voice service instance
**                  op[1] - instance of the SIP client object to delete 
**
**  OUTPUT PARMS:   none
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_DeleteSipClient( DAL_VOICE_PARMS *parms );

/*****************************************************************
**  FUNCTION:       dalVoice_DeleteCodecProfile
**
**  PURPOSE:        deletes a codec profile 
**
**  INPUT PARMS:    vpInst - parms->op[0],
**                  codec profile instance - parms->op[1]
**
**  OUTPUT PARMS:   none 
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_DeleteCodecProfile(DAL_VOICE_PARMS *parms);

/*****************************************************************
**  FUNCTION:       dalVoice_DeleteCallCtrlLine
**
**  PURPOSE:        Deletes a call control line object
**
**  INPUT PARMS:    op[0] - voice service instance
**                  op[1] - instance of the object to delete 
**
**  OUTPUT PARMS:   none
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_DeleteCallCtrlLine(DAL_VOICE_PARMS *parms);

/*****************************************************************
**  FUNCTION:       dalVoice_DeleteCallCtrlExt
**
**  PURPOSE:        Deletes a call control ext object
**
**  INPUT PARMS:    op[0] - voice service instance
**                  op[1] - instance of the object to delete 
**
**  OUTPUT PARMS:   none
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_DeleteCallCtrlExt(DAL_VOICE_PARMS *parms);

/*****************************************************************
**  FUNCTION:       dalVoice_DeleteCallFeatureSet
**
**  PURPOSE:        deletes a calling features set object in MDM
**
**  INPUT PARMS:    parms->op[0] - voice service instance
**
**  OUTPUT PARMS:   none
**
**  RETURNS:        CMSRET_SUCCESS - object creation success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_DeleteCallFeatureSet(DAL_VOICE_PARMS *parms);

/*****************************************************************
**  FUNCTION:       dalVoice_GetNumSipNetwork
**
**  PURPOSE:        Returns no. of network in this voice service 
**
**  INPUT PARMS:    length - size of string buffer
**
**  OUTPUT PARMS:   value - no. of lines that is configured
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNumSipNetwork( DAL_VOICE_PARMS *parms, int *value );

/***************************************************************************
* Function Name: dalVoice_mapNetworkNumToInst
* Description  : This returns the Line instance number corresponding
*                to a Voice Profile number and account index.
*
* Parameters   : vpInst (IN)    - VoiceProfile instance number
*                acntNum (IN)   - Account index
*                lineInst (OUT) - pointer to line instance number
* Returns      : CMSRET_SUCCESS when successfule.
****************************************************************************/
CmsRet dalVoice_mapNetworkNumToInst( DAL_VOICE_PARMS *parms, int *netwkInst );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNumSipClient
**
**  PURPOSE:        returns total accounts per specific serviceprovider
**                  ( i.e. corresponds to number of clients per specific voice network instance )
**
**  INPUT PARMS:    op[0] - SIP service instnace;
**                  op[1] - SIP network instnace;
**
**  OUTPUT PARMS:   Number of accounts per this service provider (num vplines per vp)
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNumSipClient( DAL_VOICE_PARMS *parms, int *value );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNumSipClientStr
**
**  PURPOSE:        returns total accounts per specific service provider, in string form
**                  ( i.e. corresponds to number of clients per specific voice network instance )
**
**  INPUT PARMS:    op[0] - SIP service instnace;
**                  op[1] - SIP network instnace;
**
**  OUTPUT PARMS:   Number of accounts per this service provider (num vplines per vp)
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNumSipClientStr( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_AddSipClient
**
**  PURPOSE:        Adds a SIP client object
**
**  INPUT PARMS:    op[0] - voice service instance
**
**  OUTPUT PARMS:   inst - instance of the added object
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_AddSipClient( DAL_VOICE_PARMS *parms, int *inst );

/***************************************************************************
* Function Name: dalVoice_mapAcntNumToClientInst
* Description  : This returns the Sip Client instance number corresponding
*                to a account index.
*
* Parameters   : vpInst (IN)    - VoiceProfile instance number
*                acntNum (IN)   - Account index
*                lineInst (OUT) - pointer to line instance number
* Returns      : CMSRET_SUCCESS when successfule.
****************************************************************************/
CmsRet dalVoice_mapAcntNumToClientInst( DAL_VOICE_PARMS *parms, int *lineInst );

/*****************************************************************
**  FUNCTION:       dalVoice_AddCodecProfile
**
**  PURPOSE:        adds a codec profile 
**
**  INPUT PARMS:    vpInst - parms->op[0]
**                  
**  OUTPUT PARMS:   inst - instance of the added codec profile 
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_AddCodecProfile( DAL_VOICE_PARMS *parms, int *inst);

/*****************************************************************
**  FUNCTION:       dalVoice_mapCpNumToInst
**
**  PURPOSE:        maps codec profile number to instance
**
**  INPUT PARMS:    vpInst - parms->op[0],
**                  codec profile number - parms->op[1]
**
**  OUTPUT PARMS:   cpInst - instance ID of the mapped codec profile 
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_mapCpNumToInst(DAL_VOICE_PARMS *parms, int *cpInst);

/*****************************************************************
**  FUNCTION:       dalVoice_GetVlCLCodecList
**
**  PURPOSE:        Obtains the codec list for entire voice service
**
**  INPUT PARMS:    vpInst - parms->op[0];
**                  lineInst  - parms->op[1]
**
**  OUTPUT PARMS:   codec - Priority sorted list of encoders
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVoiceSvcCodecList(DAL_VOICE_PARMS *parms, char *codec, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_MapCodecNameToCodecInst
**
**  PURPOSE:        maps a codec name to codec capability instance 
**
**  INPUT PARMS:    vpInst - parms->op[0],
**                  name   - name of the codec to be mapped
**                  
**  OUTPUT PARMS:   inst - instance of the mapped codec capability 
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_MapCodecNameToCodecInst( DAL_VOICE_PARMS *parms, const char *codecName, int *codecInst );

/*****************************************************************
**  FUNCTION:       dalVoice_SetCodecProfileAssoc
**
**  PURPOSE:        set codec profile point to the codec instance
**
**  INPUT PARMS:    op[0] - voice service instance
**                  op[1] - codec profile instance
**                  op[2] - codec instance 
**
**  OUTPUT PARMS:   None
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetCodecProfileAssoc( DAL_VOICE_PARMS *parms );
CmsRet dalVoice_SetSipNetworkEnabled( DAL_VOICE_PARMS *parms, char *value );
/*****************************************************************
**  FUNCTION:       dalVoice_SetSipNetworkVoipProfileAssoc
**
**  PURPOSE:        set network voip profile point to the voip instance
**
**  INPUT PARMS:    op[0] - voice service instance
**                  op[1] - network instance
**                  op[2] - voip instance 
**
**  OUTPUT PARMS:   None
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetSipNetworkVoipProfileAssoc( DAL_VOICE_PARMS *parms );
CmsRet dalVoice_GetSipNetworkVoipProfileAssoc( DAL_VOICE_PARMS *parms );
/*****************************************************************
**  FUNCTION:       dalVoice_SetSipClientNetworkAssoc
**
**  PURPOSE:        Set sip client point to the network instance
**
**  INPUT PARMS:    op[0] - voice service instance
**                  op[1] - SIP client instance
**                  op[2] - network instance 
**
**  OUTPUT PARMS:   None
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetSipClientNetworkAssoc( DAL_VOICE_PARMS *parms );
CmsRet dalVoice_GetSipClientNetworkAssoc( DAL_VOICE_PARMS *parms );
CmsRet dalVoice_mapSipClientInstToNum ( int vpInst, int clientInst, int *num );
/*****************************************************************
**  FUNCTION:       dalVoice_SetCcLineSipClientAssoc
**
**  PURPOSE:        Set association between call control line ans SIP client
**
**  INPUT PARMS:    op[0] - voice service instance
**                  op[1] - call ctrl line instance
**                  op[2] - sip client instance 
**
**  OUTPUT PARMS:   None
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetCcLineSipClientAssoc( DAL_VOICE_PARMS *parms );

/*****************************************************************
**  FUNCTION:       dalVoice_SetCcLinePotsFxoAssoc
**
**  PURPOSE:        Set association between call control line and FXO port
**
**  INPUT PARMS:    op[0] - voice service instance
**                  op[1] - call ctrl line instance
**                  op[2] - fxo instance 
**
**  OUTPUT PARMS:   None
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetCcLinePotsFxoAssoc( DAL_VOICE_PARMS *parms );

/*****************************************************************
**  FUNCTION:       dalVoice_GetCcLinePotsFxoAssoc
**
**  PURPOSE:        Get association between call control line and FXO port
**
**  INPUT PARMS:    op[0] - voice service instance
**                  op[1] - call ctrl line instance
**                   
**
**  OUTPUT PARMS:   op[2] - fxo instance
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetCcLinePotsFxoAssoc( DAL_VOICE_PARMS *parms );

/*****************************************************************
**  FUNCTION:       dalVoice_SetCcLineFeatureSetAssoc
**
**  PURPOSE:        set sip client point to the network instance
**
**  INPUT PARMS:    op[0] - voice service instance
**                  op[1] - call ctrl line instance
**                  op[2] - sip client instance 
**
**  OUTPUT PARMS:   None
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetCcLineFeatureSetAssoc( DAL_VOICE_PARMS *parms );

/*****************************************************************
**  FUNCTION:       dalVoice_SetCodecProfPacketPeriod
**
**  PURPOSE:        Sets the packetization period of the codec in codec profile
**
**  INPUT PARMS:    length - size of string buffer
**
**  OUTPUT PARMS:   value - codec packetization period(s), comma separated
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetCodecProfPacketPeriod( DAL_VOICE_PARMS *parms, char* value );

/*****************************************************************
**  FUNCTION:       dalVoice_mapPotsFxsNumToInst
**
**  PURPOSE:        Maps FXS object number to object instance number
**
**  INPUT PARMS:    parms->op[0] - voice service instance number
**                  parms->op[1] - FXS object number
**
**  OUTPUT PARMS:   fxsInst - FXS object instance
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_mapPotsFxsNumToInst(DAL_VOICE_PARMS *parms, int *fxsInst);
CmsRet dalVoice_mapPotsFxsInstToNum(DAL_VOICE_PARMS *parms, int *num);
CmsRet dalVoice_mapPotsFxoNumToInst(DAL_VOICE_PARMS *parms, int *fxoInst);
/*****************************************************************
**  FUNCTION:       dalVoice_GetNumOfCodecs
**
**  PURPOSE:        gets the number of codecs for a given voice service
**
**  INPUT PARMS:    svcIdx - voice service instance number
**
**  OUTPUT PARMS:   numCodec - number of codecs
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNumOfCodecs(int svcIdx,int  *numCodec);

/*****************************************************************
**  FUNCTION:       dalVoice_GetNumOfExtension
**
**  PURPOSE:        returns total number of extension per specific voice service
**
**  INPUT PARMS:    op[0] - SIP service instnace;
**
**  OUTPUT PARMS:   Number of extension in this service provider
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNumOfExtension( DAL_VOICE_PARMS *parms, int *numAcc );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNumOfExtensionStr
**
**  PURPOSE:        returns total number of extension per specific voice service, in string form
**
**  INPUT PARMS:    op[0] - SIP service instnace;
**
**  OUTPUT PARMS:   Number of extension in this service provider
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNumOfExtensionStr( DAL_VOICE_PARMS *parms, char *value, unsigned int length);

/***************************************************************************
* Function Name: dalVoice_mapExtNumToExtInst
* Description  : This returns the Callctrl Extension instance number corresponding
*                to a Extension index.
*
* Parameters   : vpInst (IN)    - VoiceProfile instance number
*                acntNum (IN)   - Account index
*                lineInst (OUT) - pointer to line instance number
* Returns      : CMSRET_SUCCESS when successfule.
****************************************************************************/
CmsRet dalVoice_mapExtNumToExtInst ( int vpInst, int num, int *extInst );

/*****************************************************************
**  FUNCTION:       dalVoice_MapCodecNameToCodecProfileInst
**
**  PURPOSE:        maps a codec name to codec profile instance 
**
**  INPUT PARMS:    vpInst - parms->op[0],
**                  name   - name of the codec to be mapped
**                  
**  OUTPUT PARMS:   inst - instance of the mapped codec profile 
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_MapCodecNameToCodecProfileInst( DAL_VOICE_PARMS *parms, const char *name, int *codecInst );

/*****************************************************************
**  FUNCTION:       dalVoice_mapCodecProfNumToInst
**
**  PURPOSE:        maps Codec profile object number to instance
**
**  INPUT PARAMS:   svc inst - parms->op[0]
**                  Codec profile object number - parms->op[1]
**
**  OUTPUT PARMS:   vpInst - object instance that input parameters map to
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_mapCodecProfNumToInst( DAL_VOICE_PARMS *parms, int *vpInst );

/*****************************************************************
**  FUNCTION:       dalVoice_SetSipNetworkCodecList
**
**  PURPOSE:        Set codec list for a given network
**
**  INPUT PARMS:    op[0] - voice service instance
**                  op[1] - network instance 
**
**  OUTPUT PARMS:   None
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetSipNetworkCodecList( DAL_VOICE_PARMS *parms, char *codecList );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipClientEnable
**
**  PURPOSE:        Obtains SIP client enable flag
**
**  INPUT PARMS:    voice service instance    - parms->op[0]
**                  client instance  - parms->op[1]
**
**  OUTPUT PARMS:   Line Enable status ("true" if enabled)
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipClientEnable(DAL_VOICE_PARMS *parms, char *lineEnabled, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipClientStatus
**
**  PURPOSE:        Obtains SIP client status
**
**  INPUT PARMS:    voice service instance    - parms->op[0]
**                  client instance  - parms->op[1]
**
**  OUTPUT PARMS:   Line status ("true" if enabled)
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipClientStatus(DAL_VOICE_PARMS *parms, char *lineStatus, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetSipClientStatus
**
**  PURPOSE:        Set SIP client status
**
**  INPUT PARMS:    voice service instance    - parms->op[0]
**                  client instance  - parms->op[1]
**                  Line status "up/registering/deregistering..."
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetSipClientStatus(DAL_VOICE_PARMS *parms, char *status );
/*****************************************************************
**  FUNCTION:       dalVoice_GetSipClientAttachedNetworkInst
**
**  PURPOSE:        Obtains the network instance attached to a particular SIP client
**
**  INPUT PARMS:    voice service instance    - parms->op[0]
**                  client instance  - parms->op[1]
**
**  OUTPUT PARMS:   Sip Network Instance number which this client attached to.
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipClientAttachedNetworkInst(DAL_VOICE_PARMS *parms, char *network, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipClientAttachedNetworkIdx
**
**  PURPOSE:        Obtains the network ID attached to a particular SIP client
**
**  INPUT PARMS:    voice service instance    - parms->op[0]
**                  client instance  - parms->op[1]
**
**  OUTPUT PARMS:   Sip Network index which this client attached to.
**                  the index is from 0 to n
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipClientAttachedNetworkIdx(DAL_VOICE_PARMS *parms, char *network, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipNetworkCodecList
**
**  PURPOSE:        Gets codec list for a given SIP network
**
**  INPUT PARMS:    vpInst - parms->op[0],
**                  network instance - parms->op[1]
**
**  OUTPUT PARMS:   codec - list of codecs for the SIP network of interest
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipNetworkCodecList(DAL_VOICE_PARMS *parms, char *codec, unsigned int length );
CmsRet dalVoice_GetSipNetworkEnabled( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_MapVoIPProfileNameToVoIPProfileInst
**
**  PURPOSE:        maps a VoIP profile name to VoIP profile instance 
**
**  INPUT PARMS:    vpInst - parms->op[0],
**                  name   - name of the codec to be mapped
**                  
**  OUTPUT PARMS:   inst - instance of the mapped VOIP profile 
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_MapVoIPProfileNameToVoIPProfileInst( DAL_VOICE_PARMS *parms, const char *name, int *voipProfInst );

/*****************************************************************
**  FUNCTION:       dalVoice_GetRtcpEnabled
**
**  PURPOSE:        Obtains the RTCP enable flag
**
**  INPUT PARMS:    op[0] - voice service instance;
**                  op[1] - voip profile  instance;
**
**  OUTPUT PARMS:   enable - RTCP enable flag
**                           MDMVS_YES = enabled
**                           MDMVS_NO  = disabled
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetRtcpEnabled(DAL_VOICE_PARMS *parms, char *enable, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetRtcpInterval
**
**  PURPOSE:        Obtains RTCP TX interval in ms
**
**  INPUT PARMS:    op[0] - voice service instance;
**                  op[1] - voip profile  instance;
**
**  OUTPUT PARMS:   interval - RTCP TX interval in ms
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetRtcpInterval(DAL_VOICE_PARMS *parms, char *interval, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetRtcpXrEnabled
**
**  PURPOSE:        Obtains the RTCP-XR enable flag
**
**  INPUT PARMS:    op[0] - voice service instance;
**                  op[1] - voip profile  instance;
**
**  OUTPUT PARMS:   enable - RTCP enable flag
**                           MDMVS_YES = enabled
**                           MDMVS_NO  = disabled
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetRtcpXrEnabled(DAL_VOICE_PARMS *parms, char *enable, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetRtcpXrPubRepAddr
**
**  PURPOSE:        Obtains RTCP-XR SIP PUBLISH report address
**
**  INPUT PARMS:    op[0] - voice service instance;
**                  op[1] - voip profile  instance;
**
**  OUTPUT PARMS:   addr - RTCP-XR SIP PUBLISH report address
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetRtcpXrPubRepAddr(DAL_VOICE_PARMS *parms, char *addr, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetVoipProfileEnable
**
**  PURPOSE:        Obtains the enable status of a particular VOIP profile
**
**  INPUT PARMS:    op[0] - voice service instance;
**                  op[1] - voip profile  instance;
**
**  OUTPUT PARMS:   enable - enable flag
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVoipProfileEnable(DAL_VOICE_PARMS *parms, char *enable, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetVoipProfileName
**
**  PURPOSE:        Obtains the name of a particular VOIP profile
**
**  INPUT PARMS:    op[0] - voice service instance;
**                  op[1] - voip profile instance;
**
**  OUTPUT PARMS:   enable - RTCP enable flag
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVoipProfileName(DAL_VOICE_PARMS *parms, char *name, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetCodecProfileName
**
**  PURPOSE:        Returns the name of the codec in codec profile
**
**  INPUT PARMS:    length - size of string buffer
**
**  OUTPUT PARMS:   value - codec name
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetCodecProfileName( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetCodecProfileEnable
**
**  PURPOSE:        Returns the packetization period of the codec in codec profile
**
**  INPUT PARMS:    length - size of string buffer
**
**  OUTPUT PARMS:   value - codec packetization period(s), comma separated
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetCodecProfileEnable( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetCallCtrlExtEnable
**
**  PURPOSE:        Returns the extension enable status
**
**  INPUT PARMS:    op[0] - voice service instance
**                  op[1] - Extension instance
**
**  OUTPUT PARMS:   value - true/false
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetCallCtrlExtEnable( DAL_VOICE_PARMS *parms, char* value, unsigned int length );
CmsRet dalVoice_SetCallCtrlExtEnabled( DAL_VOICE_PARMS *parms, char* value );

/*****************************************************************
**  FUNCTION:       dalVoice_GetCallCtrlExtStatus
**
**  PURPOSE:        Returns the extension status
**
**  INPUT PARMS:    op[0] - voice service instance
**                  op[1] - Extension instance
**
**  OUTPUT PARMS:   value - Up/Disabled/Initializing ...
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetCallCtrlExtStatus( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetCallCtrlExtCallStatus
**
**  PURPOSE:        Returns the extension call status
**
**  INPUT PARMS:    op[0] - voice service instance
**                  op[1] - Extension instance
**
**  OUTPUT PARMS:   value - Idle/Dialing/Connected ....
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetCallCtrlExtCallStatus( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetCallCtrlExtNumber
**
**  PURPOSE:        Returns the extension number
**
**  INPUT PARMS:    op[0] - voice service instance
**                  op[1] - Extension instance
**
**  OUTPUT PARMS:   value - extension number
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetCallCtrlExtNumber( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetCallCtrlExtName
**
**  PURPOSE:        Returns the extension name
**
**  INPUT PARMS:    op[0] - voice service instance
**                  op[1] - Extension instance
**
**  OUTPUT PARMS:   value - extension name string
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetCallCtrlExtName( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetCallCtrlExtProvider
**
**  PURPOSE:        Returns the extension provider
**
**  INPUT PARMS:    op[0] - voice service instance
**                  op[1] - Extension instance
**
**  OUTPUT PARMS:   value - extension provide name: FXS/FXO/DECT..
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetCallCtrlExtProvider( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetCallCtrlExtConfStatus
**
**  PURPOSE:        Returns the conferencing call status
**
**  INPUT PARMS:    op[0] - voice service instance
**                  op[1] - Extension instance
**
**  OUTPUT PARMS:   value - conf call status "Disabled/Idle/InConferenceCall..."
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetCallCtrlExtConfStatus( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetCallCtrlExtCWStatus
**
**  PURPOSE:        Returns call waiting status
**
**  INPUT PARMS:    op[0] - voice service instance
**                  op[1] - Extension instance
**
**  OUTPUT PARMS:   value - call waiting status "Disabled/Idle/SecondaryRinging...."
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetCallCtrlExtCWStatus( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetCallCtrlExtCallFeatureSet
**
**  PURPOSE:        Returns call feature set name
**
**  INPUT PARMS:    op[0] - voice service instance
**                  op[1] - Extension instance
**
**  OUTPUT PARMS:   value - call feature set name
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetCallCtrlExtCallFeatureSet( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetCallCtrlExtNumberPlan
**
**  PURPOSE:        Returns numbering plan name
**
**  INPUT PARMS:    op[0] - voice service instance
**                  op[1] - Extension instance
**
**  OUTPUT PARMS:   value - numbering plan name "numberPlan_xx" which
**                  xx is numbering plan index
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetCallCtrlExtNumberPlan( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetCallCtrlExtNumber
**
**  PURPOSE:        Sets the extension number
**
**  INPUT PARMS:    op[0] - voice service instance
**                  op[1] - Extension instance
**
**  OUTPUT PARMS:   value - extension number
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetCallCtrlExtNumber(DAL_VOICE_PARMS *parms, char *extNumber );

/*****************************************************************
**  FUNCTION:       dalVoice_SetCallCtrlExtConfStatus
**
**  PURPOSE:        Sets the conferencing call status
**
**  INPUT PARMS:    op[0] - voice service instance
**                  op[1] - Extension instance
**
**  OUTPUT PARMS:   value - conf call status "Disabled/Idle/InConferenceCall..."
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetCallCtrlExtConfStatus( DAL_VOICE_PARMS *parms, char* value );

/*****************************************************************
**  FUNCTION:       dalVoice_SetCallCtrlExtFxsAssoc
**
**  PURPOSE:        Associates an extension with FXS port
**
**  INPUT PARMS:    op[0] - voice service instance
**                  op[1] - Extension instance
**                  op[2] - FXS object instance to associate the extension to
**
**  OUTPUT PARMS:   None
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetCallCtrlExtFxsAssoc( DAL_VOICE_PARMS *parms );

/*****************************************************************
**  FUNCTION:       dalVoice_GetCallCtrlExtFxsAssoc
**
**  PURPOSE:        Get extension associated FXS instance
**
**  INPUT PARMS:    op[0] - voice service instance
**                  op[1] - Extension instance
**                  
**
**  OUTPUT PARMS:   op[2] - FXS object instance the extension associates to
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetCallCtrlExtFxsAssoc( DAL_VOICE_PARMS *parms );
CmsRet dalVoice_GetCallCtrlExtAssocType( DAL_VOICE_PARMS *parms, int *type );
CmsRet dalVoice_GetCallCtrlLineAssocType( DAL_VOICE_PARMS *parms, int *type );

/*****************************************************************
**  FUNCTION:       dalVoice_SetCallCtrlExtName
**
**  PURPOSE:        Sets the extension name
**
**  INPUT PARMS:    op[0] - voice service instance
**                  op[1] - Extension instance
**
**  OUTPUT PARMS:   value - extension name string
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetCallCtrlExtName( DAL_VOICE_PARMS *parms, char* value );

/*****************************************************************
**  FUNCTION:       dalVoice_SetCallCtrlExtCallStatus
**
**  PURPOSE:        Sets the extension call status
**
**  INPUT PARMS:    op[0] - voice service instance
**                  op[1] - Extension instance
**
**  OUTPUT PARMS:   value - Idle/Dialing/Connected ....
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetCallCtrlExtCallStatus( DAL_VOICE_PARMS *parms, char* value );

/*****************************************************************
**  FUNCTION:       dalVoice_GetCallCtrlLineEnable
**
**  PURPOSE:        Gets the line enable status
**
**  INPUT PARMS:    op[0] - voice service instance
**                  op[1] - call control line instance
**
**  OUTPUT PARMS:   value - true/false
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetCallCtrlLineEnable( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetCallCtrlLineEnabled
**
**  PURPOSE:        Sets the line enable status
**
**  INPUT PARMS:    op[0] - voice service instance
**                  op[1] - Extension instance
**
**  OUTPUT PARMS:   value - true/false
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetCallCtrlLineEnabled( DAL_VOICE_PARMS *parms, char* value );

/*****************************************************************
**  FUNCTION:       dalVoice_GetCallCtrlLineStatus
**
**  PURPOSE:        get call status of line
**
**  INPUT PARMS:    op[0] - voice service instance
**                  op[1] - line instance
**                  value - idle, dialing, connected, delivered, alerting, disconnected
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetCallCtrlLineCallStatus( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetCallCtrlLineCallStatus
**
**  PURPOSE:        set call status of line
**
**  INPUT PARMS:    op[0] - voice service instance
**                  op[1] - line instance
**                  value - idle, dialing, connected, delivered, alerting, disconnected
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetCallCtrlLineCallStatus( DAL_VOICE_PARMS *parms, char* value );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNumOfLine
**
**  PURPOSE:        returns total number of extension per specific voice service
**
**  INPUT PARMS:    op[0] - SIP service instnace;
**
**  OUTPUT PARMS:   Number of extension in this service provider
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNumOfLine( DAL_VOICE_PARMS *parms, int *numAcc );
CmsRet dalVoice_GetNumOfActiveLine( DAL_VOICE_PARMS *parms, int *numAcc );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNumOfLineStr
**
**  PURPOSE:        returns total number of extension per specific voice service, in string form
**
**  INPUT PARMS:    op[0] - SIP service instnace;
**
**  OUTPUT PARMS:   Number of extension in this service provider
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNumOfLineStr( DAL_VOICE_PARMS *parms, char *value, unsigned int length);

/*****************************************************************
**  FUNCTION:       dalVoice_GetNumIncomingMap
**
**  PURPOSE:        returns total number of incoming map entries per specific voice service
**
**  INPUT PARMS:    op[0] - SIP service instnace;
**
**  OUTPUT PARMS:   Number of incoming map entries in this service provider
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNumIncomingMap( DAL_VOICE_PARMS *parms, int *numOfMap );

/*****************************************************************
**  FUNCTION:       dalVoice_mapIncomingMapNumToInst
**
**  PURPOSE:        maps incoming map to map instance
**
**  INPUT PARMS:    parms->op[0] - voice service profile instance
**                  parms->op[1] - instance of the object to be mapped
**
**  OUTPUT PARMS:   mapInst - instance of the map object
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_mapIncomingMapNumToInst( DAL_VOICE_PARMS *parms,  int *mapInst );

/*****************************************************************
**  FUNCTION:       dalVoice_mapLineExtToIncomingMapInst
**
**  PURPOSE:        maps line+ext number to incoming map instance
**
**  INPUT PARMS:    parms->op[0] - voice service profile instance
**                  parms->op[1] - line number to be mapped
**                  parms->op[2] - ext number to be mapped
**
**  OUTPUT PARMS:   mapInst - instance of the map object
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_mapLineExtToIncomingMapInst( DAL_VOICE_PARMS *parms,  int *mapInst );

/*****************************************************************
**  FUNCTION:       dalVoice_GetIncomingMapEnable
**
**  PURPOSE:        gets the enable flag of the given map object 
**
**  INPUT PARMS:    parms->op[0] - voice service profile instance
**                  parms->op[1] - instance of the map object of interest
**
**  OUTPUT PARMS:   value - value of the enable flag
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetIncomingMapEnable( DAL_VOICE_PARMS *parms, char *value, unsigned int length );
CmsRet dalVoice_SetIncomingMapEnabled( DAL_VOICE_PARMS *parms, char *value );
/*****************************************************************
**  FUNCTION:       dalVoice_GetIncomingMapLineNum
**
**  PURPOSE:        gets the line number of the given map object 
**
**  INPUT PARMS:    parms->op[0] - voice service profile instance
**                  parms->op[1] - instance of the map object of interest
**
**  OUTPUT PARMS:   value - value of the line number
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetIncomingMapLineNum( DAL_VOICE_PARMS *parms, char *value, unsigned int length );
CmsRet dalVoice_SetIncomingMapLineExt( DAL_VOICE_PARMS *parms );

/*****************************************************************
**  FUNCTION:       dalVoice_GetIncomingMapExtNum
**
**  PURPOSE:        gets the extension number of the given map object 
**
**  INPUT PARMS:    parms->op[0] - voice service profile instance
**                  parms->op[1] - instance of the map object of interest
**
**  OUTPUT PARMS:   value - value of the ext number
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetIncomingMapExtNum( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetIncomingMapOrder
**
**  PURPOSE:        gets the order parameter of the given map object 
**
**  INPUT PARMS:    parms->op[0] - voice service profile instance
**                  parms->op[1] - instance of the map object of interest
**
**  OUTPUT PARMS:   value - value of the order parameter 
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetIncomingMapOrder( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetIncomingMapTimeout
**
**  PURPOSE:        gets the timeout parameter of the given map object 
**
**  INPUT PARMS:    parms->op[0] - voice service profile instance
**                  parms->op[1] - instance of the map object of interest
**
**  OUTPUT PARMS:   value - value of the timeout parameter 
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetIncomingMapTimeout( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNumOutgoingMap
**
**  PURPOSE:        returns total number of outgoing map entries per specific voice service
**
**  INPUT PARMS:    op[0] - SIP service instnace;
**
**  OUTPUT PARMS:   Number of outgoing map entries in this service provider
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNumOutgoingMap( DAL_VOICE_PARMS *parms, int *numOfMap );

/*****************************************************************
**  FUNCTION:       dalVoice_mapOutgoingMapNumToInst
**
**  PURPOSE:        maps outgoing map to map instance
**
**  INPUT PARMS:    parms->op[0] - voice service profile instance
**                  parms->op[1] - instance of the object to be mapped
**
**  OUTPUT PARMS:   mapInst - instance of the map object
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_mapOutgoingMapNumToInst( DAL_VOICE_PARMS *parms, int *mapInst );

/*****************************************************************
**  FUNCTION:       dalVoice_mapLineExtToOutgoingMapInst
**
**  PURPOSE:        maps line+ext number to outgoing map instance
**
**  INPUT PARMS:    parms->op[0] - voice service profile instance
**                  parms->op[1] - line number to be mapped
**                  parms->op[2] - ext number to be mapped
**
**  OUTPUT PARMS:   mapInst - instance of the map object
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_mapLineExtToOutgoingMapInst( DAL_VOICE_PARMS *parms,  int *mapInst );

/*****************************************************************
**  FUNCTION:       dalVoice_GetOutgoingMapEnable
**
**  PURPOSE:        gets the enable flag of the given map object 
**
**  INPUT PARMS:    parms->op[0] - voice service profile instance
**                  parms->op[1] - instance of the map object of interest
**
**  OUTPUT PARMS:   value - value of the enable flag
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetOutgoingMapEnable( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetOutgoingMapLineNum
**
**  PURPOSE:        gets the line number of the given map object 
**
**  INPUT PARMS:    parms->op[0] - voice service profile instance
**                  parms->op[1] - instance of the map object of interest
**
**  OUTPUT PARMS:   value - value of the line number
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetOutgoingMapLineNum( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetOutgoingMapExtNum
**
**  PURPOSE:        gets the extension number of the given map object 
**
**  INPUT PARMS:    parms->op[0] - voice service profile instance
**                  parms->op[1] - instance of the map object of interest
**
**  OUTPUT PARMS:   value - value of the ext number
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetOutgoingMapExtNum( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetOutgoingMapOrder
**
**  PURPOSE:        gets the order parameter of the given map object 
**
**  INPUT PARMS:    parms->op[0] - voice service profile instance
**                  parms->op[1] - instance of the map object of interest
**
**  OUTPUT PARMS:   value - value of the order parameter 
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetOutgoingMapOrder( DAL_VOICE_PARMS *parms, char *value, unsigned int length );
CmsRet dalVoice_SetOutgoingMapEnabled( DAL_VOICE_PARMS *parms, char *value );
CmsRet dalVoice_SetOutgoingMapLineExt( DAL_VOICE_PARMS *parms );

/*****************************************************************
**  FUNCTION:       dalVoice_AddCodecProfileByName
**
**  PURPOSE:        adds a codec profile 
**
**  INPUT PARMS:    vpInst - parms->op[0],
**                  name   - name of the codec profile to be added
**                  
**  OUTPUT PARMS:   inst - instance of the added codec profile 
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_AddCodecProfileByName( DAL_VOICE_PARMS *parms, const char *name, int *inst );

/*****************************************************************
**  FUNCTION:       dalVoice_DeleteCallCtrlOutgoingMap
**
**  PURPOSE:        deletes a call control outgoing call map object
**
**  INPUT PARMS:    parms->op[0] - voice service profile instance
**                  parms->op[1] - instance of the object to be deleted
**
**  OUTPUT PARMS:   none
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_DeleteCallCtrlOutgoingMap( DAL_VOICE_PARMS *parms );

/*****************************************************************
**  FUNCTION:       dalVoice_AddCallCtrlOutgoingMap
**
**  PURPOSE:        Adds a call control outgoing call map object
**
**  INPUT PARMS:    parms->op[0] - voice service profile instance
**
**  OUTPUT PARMS:   instance of added object
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_AddCallCtrlOutgoingMap( DAL_VOICE_PARMS *parms, int *inst );

/*****************************************************************
**  FUNCTION:       dalVoice_AddCallCtrlIncomingMap
**
**  PURPOSE:        Adds a call control incoming call map object
**
**  INPUT PARMS:    parms->op[0] - voice service profile instance
**
**  OUTPUT PARMS:   instance of added object
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_AddCallCtrlIncomingMap( DAL_VOICE_PARMS *parms, int *inst );

/*****************************************************************
**  FUNCTION:       dalVoice_DeleteCallCtrlIncomingMap
**
**  PURPOSE:        deletes a call control incoming call map object
**
**  INPUT PARMS:    parms->op[0] - voice service profile instance
**                  parms->op[1] - instance of the object to be deleted
**
**  OUTPUT PARMS:   none
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_DeleteCallCtrlIncomingMap( DAL_VOICE_PARMS *parms );

/*****************************************************************
**  FUNCTION:       dalVoice_mapCallFeatureSetNumToInst
**
**  PURPOSE:        maps a calling features set object number to instance ID
**
**  INPUT PARMS:    parms->op[0] - voice service instance
**                  parms->op[1] - object number to map
**
**  OUTPUT PARMS:   setInst - the object instance ID which the object number maps to
**
**  RETURNS:        CMSRET_SUCCESS - object creation success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_mapCallFeatureSetNumToInst( DAL_VOICE_PARMS *parms,  int *setInst );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNumCallFeatureSet
**
**  PURPOSE:        obtains the number of calling feature set objects from MDM
**
**  INPUT PARMS:    parms->op[0] - voice service instance
**
**  OUTPUT PARMS:   numOfSet - number of calling feature set objects
**
**  RETURNS:        CMSRET_SUCCESS - object creation success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNumCallFeatureSet( DAL_VOICE_PARMS *parms, int *numOfSet );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNumCallFeatureSetStr
**
**  PURPOSE:        obtains the number of calling feature set objects from MDM, in string format
**
**  INPUT PARMS:    parms->op[0] - voice service instance
**                  length - max length of the output
**
**  OUTPUT PARMS:   value - number of calling feature set objects
**
**  RETURNS:        CMSRET_SUCCESS - object creation success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNumCallFeatureSetStr( DAL_VOICE_PARMS *parms, char *value , unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetVlCFCallTransfer
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0]
**
**  OUTPUT PARMS:   'call transfer' enable flag (Yes/No)
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVlCFCallTransfer( DAL_VOICE_PARMS *parms, char *getVal, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetVlCFCallIdName
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0]
**
**  OUTPUT PARMS:   'call ID name' enable flag (Yes/No)
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVlCFCallIdName( DAL_VOICE_PARMS *parms, char *getVal, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetVlCFCallId
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0]
**
**  OUTPUT PARMS:   'call ID' enable flag (Yes/No)
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVlCFCallId( DAL_VOICE_PARMS *parms, char *getVal, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipNetworkVoipProfileIdx
**
**  PURPOSE:        Gets VoIP profile ID for the given SIP network
**
**  INPUT PARMS:    vpInst - parms->op[0],
**                  network instance - parms->op[1]
**
**  OUTPUT PARMS:   idx - VoIP profile ID for the SIPnetwork of interest 
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipNetworkVoipProfileIdx(DAL_VOICE_PARMS *parms, char *idx, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetRtpLocalPortMin
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0];
**
**  OUTPUT PARMS:   port - Value of RTP min port
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetRtpLocalPortMin(DAL_VOICE_PARMS *parms, char *port );

/*****************************************************************
**  FUNCTION:       dalVoice_SetRtpLocalPortMax
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0];
**
**  OUTPUT PARMS:   port - Value of RTP max port
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetRtpLocalPortMax(DAL_VOICE_PARMS *parms, char *port );
#endif

/***************************************************************************
* Function Name: dalVoice_mapVpInstLineInstToCMAcnt
* Description  : This returns the call manager account number corresponding
*                to a certain Voice Profile instance number and line instance
*                number.
*
* Parameters   : vpInst (IN)    - voice profile instance
*                lineInst (IN)  - line instance
*                cmAcnt (OUT)   - pointer to call manager account number
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_mapVpInstLineInstToCMAcnt ( int vpInst, int lineInst, int * cmAcnt );

/***************************************************************************
* Function Name: dalVoice_mapAcntNumToLineInst
* Description  : This returns the Line instance number corresponding
*                to a Voice Profile number and account index.
*
* Parameters   : vpInst (IN)    - VoiceProfile instance number
*                acntNum (IN)   - Service provider account index
*                lineInst (OUT) - pointer to line instance number
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_mapAcntNumToLineInst ( int vpInst, int acntNum, int * lineInst );

/***************************************************************************
* Function Name: dalVoice_mapAcntNumToLineInst2
* Description  : This returns the Line instance number corresponding
*                to a Voice Profile number and account index.
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = acntNum
*                lineInst (OUT) - pointer to line instance number
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_mapAcntNumToLineInst2 ( DAL_VOICE_PARMS *parms, int * lineInst );

/***************************************************************************
* Function Name: dalVoice_mapLineInstToAcntNum
* Description  : This returns the account number corresponding
*                to a Voice Profile number and line instance.
*
* Parameters   : vpInst (IN)    - VoiceProfile instance number
*                lineInst (IN)  - Service provider account index
*                acntNum (OUT)  - pointer to line instance number
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_mapLineInstToAcntNum ( int vpInst, int lineInst, int * acntNum );

/***************************************************************************
* Function Name: dalVoice_mapCmLineInstToVpLineInst
* Description  : This returns the Voice Profile instance number and line instance
*                number corresponding to a callmanager line index
*
* Parameters   : cmLine (IN)    - callmanger line index
*                vpInst (OUT)   - pointer to VoiceProfile instance number
*                lineInst (OUT) - pointer to line instance number
* Returns      : CMSRET_SUCCESS when successfule.
****************************************************************************/
CmsRet dalVoice_mapCmLineToVpInstLineInst ( int cmLine, int * vpInst, int * lineInst );

/***************************************************************************
* Function Name: dalVoice_mapCountryCode3To2
* Description  : Maps Alpha-3 locale to Alpha-2. Also checks if locale is valid
*
* Parameters   : alpha3 (IN) - locale(Alpha3)
*                alpha2 (OUT)   - locale alpha 2 format 
* Returns      : CMSRET_SUCCESS when successfule.
****************************************************************************/
CmsRet dalVoice_mapCountryCode3To2 ( char *alpha3, char *alpha2, unsigned int length );

/***************************************************************************
** Function Name: dalVoice_mapAlpha2toVrg
** Description  : Given an alpha2 country string returns a VRG country code
**
** Parameters   : locale (IN) - callmanger line index
**                id (OUT)    - VRG country code
**                found (OUT) - Flag that indicates if code is found
** Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_mapAlpha2toVrg( char *locale, int *id, UBOOL8 *found, unsigned int length );

#ifdef SIPLOAD
/***************************************************************************
* Function Name: dalVoice_isValidVP
* Description  : Checks if Voice Profile is valid/supported
*
* Parameters   : vpInst (IN)  - voice profile number
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_isValidVP ( int vpInst );

/***************************************************************************
* Function Name: dalVoice_isValidVPVL
* Description  : Checks if Voice Profile & Voice line is valid
*
* Parameters   : vpInst (IN)    - voice profile number
*                lineInst (IN)  - voice line number
* Returns      : CMSRET_SUCCESS when valid.
****************************************************************************/
CmsRet dalVoice_isValidVPVL ( int vpInst, int lineInst );
#endif /* SIPLOAD */

/***************************************************************************
* Function Name: dalVoice_SetT38Enable
* Description  : CLI wrapper for setT38Enable
*                Enable Fax
*                VoiceProfile.{i}.FaxT38.Enable == newVal
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetT38Enable ( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetV18Enable
* Description  : Enable V.18 detection
*                VoiceProfile.{i}.V18.Enable == new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetV18Enable( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetRegion
* Description  : CLI wrapper for setRegion()
*                Set the region identically for all Voice Profiles
*                VoiceProfile.{i}.region = newVal
*                                       s
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetRegion ( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetVoipProfEnable
* Description  : Set the "Enable" value in VOIP profile
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = profile inst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVoipProfEnable( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetVoipProfName
* Description  : Set the name in VOIP profile
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = profile inst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVoipProfName( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetDTMFMethod
* Description  : CLI wrapper for setDTMFMethod()
*                Set the method by which DTMF digits must be passed
*                Set VoiceProfile.{i}.DTMFMethod
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetDTMFMethod ( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetRtcpInterval
* Description  : Sets RTCP interval in VOIP profile RTCP object
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetRtcpInterval( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetRtcpEnable
* Description  : Sets RTCP enable value in VOIP profile RTCP object
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetRtcpEnable( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetRtcpXrEnable
* Description  : Sets RTCP-XR enable value in VOIP profile RTCP object
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetRtcpXrEnable( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetRtcpXrPubRepAddr
* Description  : Sets RTCP-XR SIP PUBLISH report address
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetRtcpXrPubRepAddr( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetDigitMap
* Description  : Set the method by which DTMF digits must be passed
*                Set VoiceProfile.{i}.digitMap
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetDigitMap ( DAL_VOICE_PARMS *parms, char *value );

#ifdef SIPLOAD
/***************************************************************************
* Function Name: dalVoice_SetCCTKDigitMap
* Description  : Set the method by which DTMF digits must be passed (custom)
*                Set VoiceProfile.{i}.X_BROADCOM_COM_CCTK_DigitMap
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetCCTKDigitMap( DAL_VOICE_PARMS *parms, char *value );
#endif

/***************************************************************************
* Function Name: dalVoice_SetCriticalDigitTimer
* Description  : Set VoiceProfile.{i}.X_BROADCOM_COM_CriticalDigitTimer
*
* Parameters   : parms->op[0] = Voice Profile Instance
*                value        = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetCriticalDigitTimer ( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetPartialDigitTimer
* Description  : Set VoiceProfile.{i}.X_BROADCOM_COM_PartialDigitTimer
*
* Parameters   : parms->op[0] = Voice Profile Instance
*                value        = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetPartialDigitTimer ( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetSTUNServer
* Description  : Set Domain name or IP address of the STUN server
*                Set VoiceProfile.{i}.STUNServer = newVal
*                Set VoiceProfile.{i}.STUNEnable = 1
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSTUNServer ( DAL_VOICE_PARMS *parms, char *value );

#ifdef SIPLOAD
/***************************************************************************
* Function Name: dalVoice_SetSipRealm
* Description  : Set the SIP authentication realm for a network
*                VoiceService.{i}.SIP.Network.{i}.Realm
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = netInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSipRealm( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetVlSipAuthUserName
* Description  : Set the SIP Authentication Username for a specified line
*                VoiceProfile.{i}.Line{i}.Sip.AuthUserName = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlSipAuthUserName( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetVlSipAuthPassword
* Description  : Set the SIP Authentication Username for a specified line
*                VoiceProfile.{i}.Line{i}.Sip.AuthUserName = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlSipAuthPassword( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetVlSipURI
* Description  : URI by which the user agent will identify itself for this line
*                VoiceProfile.{i}.Line{i}.Sip.URI = new value
*                VoiceProfile.{i}.Line.{i}.DirectoryNumber = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlSipURI ( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetVlCFCallerIDName
* Description  : String used to identify the caller also SIP display name
*                VoiceProfile.{i}.Line{i}.CallingFeatures.CallerIDName = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlCFCallerIDName( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetVlCFMWIEnable
* Description  : Enable or disable Message Waiting Indication by the endpoint
*                VoiceProfile.{i}.Line{i}.CallingFeatures.MWIEnable = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlCFMWIEnable( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetVlCLCodecList
* Description  : CLI wrapper for SetVlCLCodecList()
*                Comma-separate list of codecs (no whitespaces or other delimiters).
*                First in list is highest priority (priority == 1).
*                Last in list is lowest priority (priority == max(all_codec_priorities)).
*                VoiceProfile.{i}.Line{i}.Codec.List.{i}. = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlCLCodecList( DAL_VOICE_PARMS *parms, char *value );
#endif /* SIPLOAD */

/***************************************************************************
** Function Name: dalVoice_SetVlCFCallFwdNum
** Description  : Set a call forwarding number for a given line
**
** Parameters   : vpInst              - parms->op[0]
**                lineInst            - parms->op[1]
**                call forward number - value
** Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlCFCallFwdNum ( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
** Function Name: dalVoice_SetVlCFWarmLineNum
** Description  : Set a warm line number for a given line
**
** Parameters   : vpInst              - parms->op[0]
**                lineInst            - parms->op[1]
**                warm line number - value
** Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlCFWarmLineNum ( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetVlCFCallFwdAll
* Description  : Enable or disable callforward unconditional by the endpoint
*                VoiceProfile.{i}.Line{i}.CallingFeatures.CallForwardUnconditionalEnable = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlCFCallFwdAll( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetVlCFCallFwdNoAns
* Description  : Enable or disable call forward on no answer by the endpoint
*                VoiceProfile.{i}.Line{i}.CallingFeatures.CallForwardOnNoAnswerEnable = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlCFCallFwdNoAns ( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetVlCFCallFwdBusy
* Description  : Enable or disable call forward on busy by the endpoint
*                VoiceProfile.{i}.Line{i}.CallingFeatures.CallForwardOnBusyEnable = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlCFCallFwdBusy ( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetVlCFCallWaiting
* Description  : Enable or disable call waiting by the endpoint
*                VoiceProfile.{i}.Line{i}.CallingFeatures.CallWaitingEnable = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlCFCallWaiting ( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetVlCFCallId
* Description  : Enable or disable call ID by the endpoint
*                
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = feature set, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlCFCallId( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetVlCFCallIdName
* Description  : Enable or disable call ID name by the endpoint
*          
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = feature set, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlCFCallIdName( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetVlCFCallTransfer
* Description  : Enable or disable call transfer by the endpoint
*                
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = feature set, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlCFCallTransfer( DAL_VOICE_PARMS *parms, char *value );

#ifdef SIPLOAD
/***************************************************************************
* Function Name: dalVoice_SetVlCFAnonCallBlck
* Description  : Enable or disable Anonymous call blocking by the endpoint
*                VoiceProfile.{i}.Line{i}.CallingFeatures.MWIEnable = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlCFAnonCallBlck ( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetVlCFAnonymousCalling
* Description  : Enable or disable anonymous calling by the endpoint
*                VoiceProfile.{i}.Line{i}.CallingFeatures.AnonymousCallEnable = !(new value)
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlCFAnonymousCalling ( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetVlCFDoNotDisturb
* Description  : Enable or disable do not distrub feature
*                VoiceProfile.{i}.Line{i}.CallingFeatures.DoNotDisturbEnable = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlCFDoNotDisturb ( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetVlCFCallCompletionOnBusy
* Description  : Enable or disable call completion on busy
*                VoiceProfile.{i}.Line{i}.CallingFeatures.X_BROADCOM_COM_CAllCompletionOnBusyEnable = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlCFCallCompletionOnBusy ( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetVlCFSpeedDial
* Description  : Enable or disable call completion on busy
*                VoiceProfile.{i}.Line{i}.CallingFeatures.X_BROADCOM_COM_SpeedDialEnable = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlCFSpeedDial ( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetVlCFWarmLine
* Description  : Enable or disable warm line
*                VoiceProfile.{i}.Line{i}.CallingFeatures.X_BROADCOM_COM_WarmLineEnable = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlCFWarmLine ( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetVlCFCallBarring
* Description  : Enable or disable call barring feature
*                VoiceProfile.{i}.Line{i}.CallingFeatures.X_BROADCOM_COM_CallBarringEnable = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlCFCallBarring ( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetVlCFCallBarringMode
* Description  : Set the Barring mode to none (0), all (1), or per digit map (2)
*                VoiceProfile.{i}.Line{i}.CallingFeatures.X_BROADCOM_COM_CallBarringMode = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlCFCallBarringMode( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetVlCFCallBarringPin
* Description  : Call barring pin number
*                VoiceProfile.{i}.Line{i}.CallingFeatures.X_BROADCOM_COM_CallBarringUserPin = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlCFCallBarringPin ( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetVlCFCallBarringDigitMap
* Description  : Set the method by which DTMF digits must be passed for call barring
*                Set VoiceProfile.{i}.digitMap
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlCFCallBarringDigitMap ( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetVlCFNetworkPrivacy
* Description  : Enable or disable network privacy feature
*                VoiceProfile.{i}.Line{i}.CallingFeatures.X_BROADCOM_COM_NetworkPrivacyEnable = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlCFNetworkPrivacy ( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetVlCFVisualMWI
* Description  : Enable or disable visual message waiting indication feature
*                VoiceProfile.{i}.Line{i}.CallingFeatures.X_BROADCOM_COM_VMWIEnable = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlCFVisualMWI ( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetSilenceSuppression
* Description  : Set all codec profiles silence suppression flag
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSilenceSuppression( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetVlCLPacketizationPeriod
* Description  : CLI wrapper for SetVlCLPacketizationPeriod()
*                Comma-separate list of supported packetization periods, in milliseconds,
*                or continuous ranges of packetization periods
*                VoiceProfile.{i}.Line{i}.Codec.List.{i}.PacketizationPeriod = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlCLPacketizationPeriod( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetVlEnable
* Description  : CLI wrapper for setVlEnable
*                Enable voice line, will also enable vprofile if disabled
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlEnable ( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetVlPhyReferenceList
* Description  : A comma separated list of Physical Interface
*                Identifiers that this Line is associated with
*                VoiceProfile.{i}.Line{i}.PhyReferenceList = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlPhyReferenceList ( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetSipTimerT1
* Description  : set SIP protocol T1 timer value
*                RTT estimate
*                VoiceService.{i}.SIP.Network.{i}.TimerT1 = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = netInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSipTimerT1( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetSipTimerT2
* Description  : set SIP protocol T2 timer value
*                Maximum retransmit interval for non-INVITE requests
*                and INVITE responses
*                VoiceService.{i}.SIP.Network.{i}.TimerT2 = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = netInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSipTimerT2( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetSipTimerT4
* Description  : set SIP protocol T4 timer value
*                Maximum duration a message will remain in the network
*                VoiceService.{i}.SIP.Network.{i}.TimerT4 = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = netInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSipTimerT4( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetSipTimerB
* Description  : set SIP protocol B timer value
*                INVITE transaction timeout timer
*                VoiceService.{i}.SIP.Network.{i}.TimerB = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = netInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSipTimerB( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetSipTimerD
* Description  : set SIP protocol D timer value
*                wait time for response retransmits
*                VoiceService.{i}.SIP.Network.{i}.TimerD = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = netInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSipTimerD( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetSipTimerF
* Description  : set SIP protocol Timer F value
*                Non-INVITE transaction timeout timer
*                VoiceService.{i}.SIP.Network.{i}.TimerF = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = netInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSipTimerF( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetSipTimerH
* Description  : set SIP protocol Timer H value
*                Non-INVITE transaction timeout timer
*                VoiceService.{i}.SIP.Network.{i}.TimerH = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = netInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSipTimerH( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetSipTimerJ
* Description  : set SIP protocol Timer J value
*                Non-INVITE transaction timeout timer
*                VoiceService.{i}.SIP.Network.{i}.TimerJ = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = netInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSipTimerJ( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetSipInviteExpires
* Description  : set SIP protocol Invite Expires time
*                INVITE SIP header "Expires:" value in seconds
*                VoiceService.{i}.SIP.Network.{i}.InviteExpires = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = netInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSipInviteExpires( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetSipRegistrarServer
* Description  : set Host name or IP address of the SIP registrar server
*                VoiceProfile.{i}.Sip.RegistrarServer = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSipRegistrarServer( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetSipRegistrarServerPort
* Description  : set Host name or IP address of the SIP registrar server port
*                VoiceProfile.{i}.Sip.RegistrarServerPort = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSipRegistrarServerPort( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetSipProxyServer
* Description  : set Host name or IP address of the SIP Proxy server
*                VoiceProfile.{i}.Sip.ProxyServer = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSipProxyServer ( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetSipProxyServerPort
* Description  : set Host name or IP address of the SIP Proxy server port
*                VoiceProfile.{i}.Sip.ProxyServerPort = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSipProxyServerPort ( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetSipUserAgentDomain
* Description  : CPE domain string
*                VoiceProfile.{i}.Sip.UserAgentDomain = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSipUserAgentDomain( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetSipUserAgentPort
* Description  : Port used for incoming call control signaling
*                VoiceProfile.{i}.Sip.ProxyServerPort = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSipUserAgentPort( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetSipOutboundProxy
* Description  : Host name or IP address of the outbound proxy
*                VoiceProfile.{i}.Sip.OutboundProxy = new value
*                Current Implementation ignores 'lineInst' param because
*                the variable being set is global in Call Manager
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSipOutboundProxy( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetSipOutboundProxyPort
* Description  : Destination port to be used in connecting to the outbound proxy
*                VoiceProfile.{i}.Sip.OutboundProxyPort = new value
*                Current Implementation ignores 'lineInst' param because
*                the variable being set is global in Call Manager
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSipOutboundProxyPort( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetSipRegistrationPeriod
* Description  : Period over which the user agent must periodicallyregister, in seconds
*                VoiceProfile.{i}.Sip.RegistrationPeriod = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSipRegistrationPeriod( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetSipRegisterRetryInterval
* Description  : Register retry interval, in seconds
*                VoiceProfile.{i}.Sip.RegisterRetryInterval = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSipRegisterRetryInterval ( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetSipRegisterExpires
* Description  : Register request Expires header value, in seconds
*                VoiceProfile.{i}.Sip.RegisterExpires = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSipRegisterExpires ( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetSipDSCPMark
* Description  : Diffserv code point to be used for outgoing SIP signaling packets.
*                VoiceProfile.{i}.Sip.DSCPMark = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSipDSCPMark ( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetSipTransport
* Description  : Transport protocol to be used in connecting to the SIP server
*                VoiceProfile.{i}.Sip.ProxyServerTransport = new value
*                VoiceProfile.{i}.Sip.RegistrarServerTransport = new value
*                VoiceProfile.{i}.Sip.UserAgentTransport = new value
*                We only support one protocol at a time, so we write to all
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSipTransport ( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
** Function Name: dalVoice_SetVlCFCallFwdNum
** Description  : Set a call forwarding number for a given line
**
** Parameters   : vpInst              - parms->op[0]
**                lineInst            - parms->op[1]
**                call forward number - value
** Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlCFCallFwdNum( DAL_VOICE_PARMS *parms, char * value );

/***************************************************************************
** Function Name: dalVoice_SetVlCFFeatureStarted
** Description  : Activate/Deactivate a call feature
**
** Parameters   : vpInst              - parms->op[0]
**                lineInst            - parms->op[1]
**                call feature ID     - parms->op[2]
**                call feature value  - value
** Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlCFFeatureStarted( DAL_VOICE_PARMS *parms, char * value );

/***************************************************************************
** Function Name: dalVoice_SetVlCFFeatureEnabled
** Description  : Enable/Disable a call feature
**
** Parameters   : vpInst              - parms->op[0]
**                lineInst            - parms->op[1]
**                call feature ID     - parms->op[2]
**                call feature value  - value
** Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlCFFeatureEnabled( DAL_VOICE_PARMS *parms, char * value );

#endif /* SIPLOAD */

/***************************************************************************
* Function Name: dalVoice_SetSrtpOption
* Description  : SRTP Protocol Usage Option (mandatory, optional or disabled)
*                VoiceProfile.{i}.SrtpOption = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSrtpOption( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetRedOption
* Description  : RFC2198 Usage Option (Disabled, 1, 2, 3)
*                VoiceProfile.{i}.RTP.Redundancy = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetRedOption( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetRedEnable
* Description  : VoiceService.{i}.VoIPProfile.{i}.RTP.Redundancy - enable/disable
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetRedEnable( DAL_VOICE_PARMS *parms, char *value);

/***************************************************************************
* Function Name: dalVoice_SetVlCLCodecList
* Description  : CLI wrapper for SetVlCLCodecList()
*                Comma-separate list of codecs (no whitespaces or other delimiters).
*                First in list is highest priority (priority == 1).
*                Last in list is lowest priority (priority == max(all_codec_priorities)).
*                VoiceProfile.{i}.Line{i}.Codec.List.{i}. = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlCLCodecList( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
** Function Name: dalVoice_SetVlCFCallFwdNum
** Description  : Set a call forwarding number for a given line
**
** Parameters   : vpInst              - parms->op[0]
**                lineInst            - parms->op[1]
**                call forward number - value
** Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlCFCallFwdNum( DAL_VOICE_PARMS *parms, char * value );

/***************************************************************************
* Function Name: dalVoice_SetVlVPReceiveGain
* Description  : Gain in units of 0.1 dB to apply to the received
*                voice signal after decoding.
*                VoiceProfile.{i}.Line{i}.VoiceProcessing.ReceiveGain = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlVPReceiveGain( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetVlVPTransmitGain
* Description  : Gain in units of 0.1 dB to apply to the transmitted
*                voice signal prior to encoding.
*                VoiceProfile.{i}.Line{i}.VoiceProcessing.TransmitGain = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlVPTransmitGain( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetVlEnable
* Description  : CLI wrapper for setVlEnable
*                Enable voice line, will also enable vprofile if disabled
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlEnable ( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetVlPhyReferenceList
* Description  : A comma separated list of Physical Interface
*                Identifiers that this Line is associated with
*                VoiceProfile.{i}.Line{i}.PhyReferenceList = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlPhyReferenceList ( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetRtpDSCPMark
* Description  : Diffserv code point to be used for outgoing RTP
*                packets for this profile
*                VoiceProfile.{i}.Sip.DSCPMark = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetRtpDSCPMark ( DAL_VOICE_PARMS *parms, char *value );

#ifdef DMP_X_BROADCOM_COM_PSTNENDPOINT_1
/***************************************************************************
* Function Name: dalVoice_SetPstnDialPlan
* Description  : Set the PSTN outgoing dial plan
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetPstnDialPlan ( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetPstnRouteRule
* Description  : Set the PSTN outgoing routing rule
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetPstnRouteRule ( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetPstnRouteData
* Description  : Set the PSTN outgoing routing data
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetPstnRouteData ( DAL_VOICE_PARMS *parms, char *value );
#endif

/***************************************************************************
* Function Name: dalVoice_performFilterOperation
* Description  : Add/delete a firewall filter for voice
*
* Parameters   : parms = Not used, fwCtlBlk = firewall control block
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_performFilterOperation( DAL_VOICE_PARMS *parms, DAL_VOICE_FIREWALL_CTL_BLK *fwCtlBlk );
CmsRet dalVoice_performFilterOperation_igd( DAL_VOICE_PARMS *parms, DAL_VOICE_FIREWALL_CTL_BLK *fwCtlBlk );
CmsRet dalVoice_performFilterOperation_dev2( DAL_VOICE_PARMS *parms, DAL_VOICE_FIREWALL_CTL_BLK *fwCtlBlk );

#if defined(SUPPORT_DM_LEGACY98)
#define dalVoice_performFilterOperation(p, f)  dalVoice_performFilterOperation_igd((p), (f))
#elif defined(SUPPORT_DM_HYBRID)
#define dalVoice_performFilterOperation(p, f)  dalVoice_performFilterOperation_igd((p), (f))
#elif defined(SUPPORT_DM_PURE181)
#define dalVoice_performFilterOperation(p, f)  dalVoice_performFilterOperation_dev2((p), (f))
#elif defined(SUPPORT_DM_DETECT)
#define dalVoice_performFilterOperation(p, f)  (cmsMdm_isDataModelDevice2() ? \
                                     dalVoice_performFilterOperation_dev2((p), (f)) : \
                                     dalVoice_performFilterOperation_igd((p), (f)))
#endif

#ifdef SIPLOAD
/*****************************************************************
**  FUNCTION:       dalVoice_GetSipRegistrarServer
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst  - parms->op[0]
**                  lineInst  - parms->op[1]
**
**  OUTPUT PARMS:   regSvrAddr - RegistrarServer URL
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipRegistrarServer(DAL_VOICE_PARMS *parms, char *regSvrAddr, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipProxyServer
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst  - parms->op[0]
**                  lineInst  - parms->op[1]
**
**  OUTPUT PARMS:   proxyAddr - SIP Proxy Server address
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipProxyServer(DAL_VOICE_PARMS *parms, char *proxyAddr, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipProxyServerPort
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst  - parms->op[0]
**                  lineInst  - parms->op[1]
**
**  OUTPUT PARMS:   proxyPort - Proxy server port
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipProxyServerPort(DAL_VOICE_PARMS *parms, char *proxyPort, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetVlCFCallerIDName
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0]
**                  lineInst  - parms->op[1]
**
**  OUTPUT PARMS:   userName - Username/DisplayName
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVlCFCallerIDName(DAL_VOICE_PARMS *parms, char *userName, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetVlSipURI
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst    - parms->op[0]
**                  lineInst  - parms->op[1]
**
**
**  OUTPUT PARMS:   Entire SIP URI
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVlSipURI(DAL_VOICE_PARMS *parms, char *userId, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetVlSipURIUser
**
**  PURPOSE:        Wrap dalVoice_GetVlSipURI() and only return
**                  user or extension part of SIP URI.
**
**  INPUT PARMS:    vpInst    - parms->op[0]
**                  lineInst  - parms->op[1]
**
**
**  OUTPUT PARMS:   User/extension
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVlSipURIUser(DAL_VOICE_PARMS *parms, char *userId, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipRegistrarServerPort
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst  - parms->op[0]
**                  lineInst  - parms->op[1]
**
**  OUTPUT PARMS:   port - Registrar Server Port
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipRegistrarServerPort(DAL_VOICE_PARMS *parms, char *port, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipOutboundProxy
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst  - parms->op[0]
**                  lineInst  - parms->op[1]
**
**  OUTPUT PARMS:   outgoingProxy - Outgoing Proxy Address
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipOutboundProxy(DAL_VOICE_PARMS *parms, char *outgoingProxy, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipOutboundProxyPort
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst  - parms->op[0]
**                  lineInst  - parms->op[1]
**
**  OUTPUT PARMS:   port - Outgoing Proxy Address
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipOutboundProxyPort(DAL_VOICE_PARMS *parms, char *port, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipRegisterExpires
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst  - parms->op[0]
**                  lineInst  - parms->op[1]
**
**  OUTPUT PARMS:   regExpire - Registration Expire Timeout
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipRegisterExpires(DAL_VOICE_PARMS *parms, char *regExpire, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipTimerT1
**
**  PURPOSE:        Get TimerT1 value
**                  RTT estimate
**
**  INPUT PARMS:    vpInst  - parms->op[0]
**                  netInst - parms->op[1]
**
**  OUTPUT PARMS:   tmrT1 - timer T1 value
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipTimerT1(DAL_VOICE_PARMS *parms, char *tmrT1, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipTimerT2
**
**  PURPOSE:        Get TimerT2 value
**                  The maximum retransmit interval for non-INVITE requests
**                  and INVITE responses
**
**  INPUT PARMS:    vpInst  - parms->op[0]
**                  netInst - parms->op[1]
**
**  OUTPUT PARMS:   tmrT2 - timer T2 value
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipTimerT2(DAL_VOICE_PARMS *parms, char *tmrT2, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipTimerT4
**
**  PURPOSE:        Get TimerT4 value
**                  Maximum duration a message will remain in the network
**
**  INPUT PARMS:    vpInst  - parms->op[0]
**                  netInst - parms->op[1]
**
**  OUTPUT PARMS:   tmrT4 - timer T4 value
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipTimerT4(DAL_VOICE_PARMS *parms, char *tmrT4, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipTimerB
**
**  PURPOSE:        Get TimerB value
**                  INVITE transaction timeout timer
**
**  INPUT PARMS:    vpInst  - parms->op[0]
**                  netInst - parms->op[1]
**
**  OUTPUT PARMS:   tmrB - timer B value
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipTimerB(DAL_VOICE_PARMS *parms, char *tmrB, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipTimerD
**
**  PURPOSE:        Get TimerD value
**                  Wait time for response retransmits
**
**  INPUT PARMS:    vpInst  - parms->op[0]
**                  netInst - parms->op[1]
**
**  OUTPUT PARMS:   tmrD - timer D value
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipTimerD(DAL_VOICE_PARMS *parms, char *tmrD, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipTimerF
**
**  PURPOSE:        Get TimerF value
**                  non-INVITE transaction timeout timer
**
**  INPUT PARMS:    vpInst  - parms->op[0]
**                  netInst - parms->op[1]
**
**  OUTPUT PARMS:   tmrF - timer F value
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipTimerF(DAL_VOICE_PARMS *parms, char *tmrF, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipTimerH
**
**  PURPOSE:        Get TimerH value
**                  Wait time for ACK receipt
**
**  INPUT PARMS:    vpInst  - parms->op[0]
**                  netInst - parms->op[1]
**
**  OUTPUT PARMS:   tmrH - timer H value
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipTimerH(DAL_VOICE_PARMS *parms, char *tmrH, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipTimerJ
**
**  PURPOSE:        Get TimerJ value
**                  Wait time for non-INVITE request retransmits
**
**  INPUT PARMS:    vpInst  - parms->op[0]
**                  netInst - parms->op[1]
**
**  OUTPUT PARMS:   tmrJ - timer J value
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipTimerJ(DAL_VOICE_PARMS *parms, char *tmrJ, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipInviteExpires
**
**  PURPOSE:        Get SIP Invite expires value
**                  value of SIP header "Expires:" in INVITE
**
**  INPUT PARMS:    vpInst  - parms->op[0]
**                  netInst - parms->op[1]
**
**  OUTPUT PARMS:   inviteExpires - Invite expires value in seconds
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipInviteExpires(DAL_VOICE_PARMS *parms, char *inviteExpires, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipRegisterRetryInterval
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst  - parms->op[0]
**                  lineInst  - parms->op[1]
**
**  OUTPUT PARMS:   regRetry - Registration retry interval
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipRegisterRetryInterval(DAL_VOICE_PARMS *parms, char *regRetry, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipUserAgentDomain
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst  - parms->op[0]
**                  lineInst  - parms->op[1]
**
**  OUTPUT PARMS:   User Agent Domain ( FQDN )
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet  dalVoice_GetSipUserAgentDomain(DAL_VOICE_PARMS *parms, char *fqdn, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipUserAgentPort
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst  - parms->op[0]
**                  lineInst  - parms->op[1]
**
**  OUTPUT PARMS:   port - User Agent port
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet  dalVoice_GetSipUserAgentPort(DAL_VOICE_PARMS *parms, char *port, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipTransport
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0]
**
**  OUTPUT PARMS:   transport - 1 - UDP
**                              4 - TLS
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet  dalVoice_GetSipTransport(DAL_VOICE_PARMS *parms, char *transport, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipTransportString
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0];
**
**  OUTPUT PARMS:   String Value of SIP transport
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipTransportString(DAL_VOICE_PARMS *parms, char *transport, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipDSCPMark
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0];
**
**  OUTPUT PARMS:   dscpMark - Value of SIP DSCP mark
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet  dalVoice_GetSipDSCPMark(DAL_VOICE_PARMS *parms, char *dscpMark, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipRealm
**
**  PURPOSE:        Get realm for this network
**                  (to be used with user name and password)
**                  Empty realm will use realm in 401/407 response.
**
**  INPUT PARMS:    vpInst  - parms->op[0]
**                  netInst  - parms->op[1]
**
**  OUTPUT PARMS:   realm - SIP network realm
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipRealm(DAL_VOICE_PARMS *parms, char *realm, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetVlSipAuthUserName
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst  - parms->op[0]
**                  lineInst  - parms->op[1]
**
**  OUTPUT PARMS:   authName - SIP Auth username
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVlSipAuthUserName(DAL_VOICE_PARMS *parms, char *authName, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetVlSipAuthPassword
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst  - parms->op[0]
**                  lineInst  - parms->op[1]
**
**  OUTPUT PARMS:   passwd - Password
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVlSipAuthPassword(DAL_VOICE_PARMS *parms, char *passwd, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetVlCFFeatureEnabled
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0]
**                  lineInst - parms->op[1]
**                  call feature - parms->op[2]
**
**  OUTPUT PARMS:   Call feature enabled flag
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVlCFFeatureEnabled(DAL_VOICE_PARMS *parms, char *getVal , unsigned int length);

/*****************************************************************
**  FUNCTION:       dalVoice_GetVlCFFeatureStarted
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0]
**                  lineInst - parms->op[1]
**                  call feature - parms->op[2]
**
**  OUTPUT PARMS:   Call feature action flag
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVlCFFeatureStarted(DAL_VOICE_PARMS *parms, char *getVal, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetVlCFCallWaiting
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0]
**
**  OUTPUT PARMS:   'call waiting' enable flag (Yes/No)
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVlCFCallWaiting( DAL_VOICE_PARMS *parms, char *getVal, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetVlCFCallBarring
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0]
**
**  OUTPUT PARMS:   'call barring' enable flag (Yes/No)
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVlCFCallBarring( DAL_VOICE_PARMS *parms, char *getVal, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetVlCFAnonCallBlck
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0]
**
**  OUTPUT PARMS:   'anonymous call rejection' enable flag (Yes/No)
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVlCFAnonCallBlck( DAL_VOICE_PARMS *parms, char* getVal, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetVlCFAnonymousCalling
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0]
**
**  OUTPUT PARMS:   'anonymous outgoing call' enable flag (Yes/No)
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVlCFAnonymousCalling( DAL_VOICE_PARMS *parms, char *getVal, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetVlCFDoNotDisturb
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0]
**
**  OUTPUT PARMS:   'anonymous outgoing call' enable flag (Yes/No)
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVlCFDoNotDisturb( DAL_VOICE_PARMS *parms, char *getVal, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetVlCFWarmLine
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0]
**
**  OUTPUT PARMS:   'warm line' enable flag (Yes/No)
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVlCFWarmLine( DAL_VOICE_PARMS *parms, char *getVal, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetVlCFCallReturn
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0]
**
**  OUTPUT PARMS:   'call return' enable flag (Yes/No)
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVlCFCallReturn( DAL_VOICE_PARMS *parms, char *getVal, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetVlCFCallRedial
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0]
**
**  OUTPUT PARMS:   'call redial' enable flag (Yes/No)
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVlCFCallRedial( DAL_VOICE_PARMS *parms, char *getVal, unsigned int length );

/***************************************************************************
** Function Name: dalVoice_GetFeatureString
** Description  : Provis wrapper to get the value of the call feature string
** Parameters   : vpInst  - parms->op[0]
**                lineInst - parms->op[1]
**                call feature ID - parms->op[2]
** Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetFeatureString(DAL_VOICE_PARMS *parms, char *getVal, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetLogServer
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0]
**
**  OUTPUT PARMS:   logServer - SIP log server address
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetLogServer(DAL_VOICE_PARMS *parms, char *logServer, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetLogServerPort
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0]
**
**  OUTPUT PARMS:   port - SIP log server port
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetLogServerPort(DAL_VOICE_PARMS *parms, char *port, unsigned int length );


/*****************************************************************
**  FUNCTION:       dalVoice_GetSipConferencingURI
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0]
**
**  OUTPUT PARMS:   conferencingURI - Conferencing server URI
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipConferencingURI(DAL_VOICE_PARMS *parms, char *conferencingURI, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipConferencingOption
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0]
**
**  OUTPUT PARMS:   conferencingOption - Conferencing option
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipConferencingOption(DAL_VOICE_PARMS *parms, char *conferencingOption, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipFailoverEnable
**
**  PURPOSE:        obtains the "Enable" value of the SIP failover feature
**
**  INPUT PARMS:    vpInst - parms->op[0]
**
**  OUTPUT PARMS:   enable - true or false
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipFailoverEnable(DAL_VOICE_PARMS *parms, char *enable, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipOptionsEnable
**
**  PURPOSE:        obtains the "Enable" value of the SIP OPTIONS ping feature
**
**  INPUT PARMS:    vpInst - parms->op[0]
**
**  OUTPUT PARMS:   enable - true or false
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipOptionsEnable(DAL_VOICE_PARMS *parms, char *enable, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipSecDomainName
**
**  PURPOSE:        Obtains SIP secondary domain name
**
**  INPUT PARMS:    vpInst - parms->op[0]
**
**  OUTPUT PARMS:   secDomainName - Secondary domain name
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipSecDomainName(DAL_VOICE_PARMS *parms, char *secDomainName, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipSecProxyAddr
**
**  PURPOSE:        Obtains SIP secondary proxy address
**
**  INPUT PARMS:    vpInst - parms->op[0]
**
**  OUTPUT PARMS:   secProxyAddr - Secondary proxy address
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipSecProxyAddr(DAL_VOICE_PARMS *parms, char *secProxyAddr, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipSecProxyPort
**
**  PURPOSE:        Obtains SIP secondary proxy port
**
**  INPUT PARMS:    vpInst - parms->op[0]
**
**  OUTPUT PARMS:   port - Secondary proxy port
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipSecProxyPort(DAL_VOICE_PARMS *parms, char *port, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipSecOutboundProxyAddr
**
**  PURPOSE:        Obtains SIP secondary outbound proxy address
**
**  INPUT PARMS:    vpInst - parms->op[0]
**
**  OUTPUT PARMS:   secObProxyAddr - Secondary outbound proxy address
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipSecOutboundProxyAddr(DAL_VOICE_PARMS *parms, char *secObProxyAddr, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipSecOutboundProxyPort
**
**  PURPOSE:        Obtains SIP secondary outbound proxy port
**
**  INPUT PARMS:    vpInst - parms->op[0]
**
**  OUTPUT PARMS:   port - Secondary outbound proxy port
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipSecOutboundProxyPort(DAL_VOICE_PARMS *parms, char *port, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipSecRegistrarAddr
**
**  PURPOSE:        Obtains SIP secondary registrar address
**
**  INPUT PARMS:    vpInst - parms->op[0]
**
**  OUTPUT PARMS:   secRegistrarAddr - Secondary registrar address
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipSecRegistrarAddr(DAL_VOICE_PARMS *parms, char *secRegistrarAddr, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipSecRegistrarPort
**
**  PURPOSE:        Obtains SIP secondary registrar port
**
**  INPUT PARMS:    vpInst - parms->op[0]
**
**  OUTPUT PARMS:   port - Secondary registrar port
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipSecRegistrarPort(DAL_VOICE_PARMS *parms, char *port, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipBackToPrimOption
**
**  PURPOSE:        obtains the back-to-primary value of the SIP failover feature
**
**  INPUT PARMS:    vpInst - parms->op[0]
**
**  OUTPUT PARMS:   option - from the list of available back-to-primary options
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipBackToPrimOption(DAL_VOICE_PARMS *parms, char *option, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipToTagMatching
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0]
**
**  OUTPUT PARMS:   tagMatching - SIP to tag matching
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipToTagMatching(DAL_VOICE_PARMS *parms, char *tagMatching, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipUriForTls
**
**  PURPOSE:        Gets the value of SIP URI usage parameter for TLS. 
**                  If true, SIP URI is used for TLS calls
**
**  INPUT PARMS:    vpInst - parms->op[0]
**
**  OUTPUT PARMS:   tagMatching - SIP to tag matching
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipUriForTls(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetVlCFCallBarringMode
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0]
**                  lineInst - parms->op[1]
**
**  OUTPUT PARMS:   mode - Call barring mode
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVlCFCallBarringMode(DAL_VOICE_PARMS *parms, char *mode, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetVlCFCallBarringPin
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0]
**
**  OUTPUT PARMS:   pin - Call barring user PIN
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVlCFCallBarringPin(DAL_VOICE_PARMS *parms, char *pin, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetVlCFCallBarringDigitMap
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0]
**
**  OUTPUT PARMS:   digitMap - Call barring digit map
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVlCFCallBarringDigitMap(DAL_VOICE_PARMS *parms, char *digitMap, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetVlCFVisualMWI
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0]
**
**  OUTPUT PARMS:   vwmi - Visual Message waiting indication
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVlCFVisualMWI(DAL_VOICE_PARMS *parms, char *vmwi, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_SetLogServer
* Description  : set Host name or IP address of the log server
*                VoiceProfile.{i}.X_BROADCOM_COM_LogServer = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetLogServer ( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetLogServerPort
* Description  : set Host name or IP address of the log server port
*                VoiceProfile.{i}.X_BROADCOM_COM_LogServerPort = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetLogServerPort ( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetSipMusicServer
* Description  : set Host name or IP address of the music server
*                VoiceProfile.{i}.X_BROADCOM_COM_MusicServer = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSipMusicServer ( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetSipMusicServerPort
* Description  : set Host name or IP address of the music server port
*                VoiceProfile.{i}.X_BROADCOM_COM_MusicServerPort = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSipMusicServerPort ( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetSipConferencingURI
* Description  : set Host name or IP address of the conferencing URI
*                VoiceProfile.{i}.SIP.X_BROADCOM_COM_ConferencingURI = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSipConferencingURI( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetSipConferencingOption
* Description  : set SIP conferencing option
*                VoiceProfile.{i}.SIP.X_BROADCOM_COM_ConferencingOption = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSipConferencingOption( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetSipFailoverEnable
* Description  : Enables SIP failover feature
*                VoiceProfile.{i}.X_BROADCOM_COM_SipFailoverEnable = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSipFailoverEnable( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetSipOptionsEnable
* Description  : set value of SIP Options ping enable flag
*                VoiceProfile.{i}.X_BROADCOM_COM_SipOptionsEnable = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSipOptionsEnable( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetSipSecDomainName
* Description  : set value of the secondary domain name
*                VoiceProfile.{i}.X_BROADCOM_COM_SecondaryDomainName = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSipSecDomainName( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetSipSecProxyAddr
* Description  : set IP address of the secondary proxy
*                VoiceProfile.{i}.X_BROADCOM_COM_SecondaryProxyAddress = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSipSecProxyAddr( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetSipSecProxyPort
* Description  : set port of the secondary proxy
*                VoiceProfile.{i}.X_BROADCOM_COM_SecondaryProxyPort = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSipSecProxyPort( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetSipSecOutboundProxyAddr
* Description  : set IP address of the secondary outbound proxy
*                VoiceProfile.{i}.X_BROADCOM_COM_SecondaryOutboundProxyAddress = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSipSecOutboundProxyAddr( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetSipSecOutboundProxyPort
* Description  : set port of the secondary outbound proxy
*                VoiceProfile.{i}.X_BROADCOM_COM_SecondaryOutboundProxyPort = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSipSecOutboundProxyPort( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetSipSecRegistrarAddr
* Description  : set IP address of the secondary registrar
*                VoiceProfile.{i}.X_BROADCOM_COM_SecondaryRegistrarAddress = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSipSecRegistrarAddr( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetSipSecRegistrarPort
* Description  : set port of the secondary registrar
*                VoiceProfile.{i}.X_BROADCOM_COM_SecondaryRegistrarPort = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSipSecRegistrarPort( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetSipBackToPrimOption
* Description  : set back-to-primary option for SIP failover
*                VoiceProfile.{i}.X_BROADCOM_COM_BackToPrimMode = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSipBackToPrimOption( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetSipToTagMatching
* Description  : set value of SIP to tag matching
*                VoiceProfile.{i}.X_BROADCOM_COM_ToTagMatching = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSipToTagMatching( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetSipUriForTls
* Description  : Sets whether SIP URI (true) or SIPS URI (false) is to be used for TLS calls 
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSipUriForTls( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetSTUNServerPort
* Description  : set the STUNServer port
*                VoiceProfile.{i}.X_BROADCOM_COM_STUNServerPort = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSTUNServerPort ( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetHookFlashMethod
* Description  : set hook flash method
*                VoiceProfile.{i}.X_BROADCOM_COM_HookFlashMethod = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = profInst,
*                value = value to be set, None/SIPInfo
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetHookFlashMethod ( DAL_VOICE_PARMS *parms, char *value );

/*****************************************************************
**  FUNCTION:       dalVoice_SetMaxLinesPerVoiceProfile
**
**  PURPOSE:        Sets max no. of lines that can be configured, as int
**
**  INPUT PARMS:    
**
**  OUTPUT PARMS:   
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetMaxLinesPerVoiceProfile( DAL_VOICE_PARMS *parms, unsigned int value );

/*****************************************************************
**  FUNCTION:       dalVoice_SetMaxExtPerVoiceProfile
**
**  PURPOSE:        Sets max no. of extensions that can be configured, as int
**
**  INPUT PARMS:    
**
**  OUTPUT PARMS:   
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetMaxExtPerVoiceProfile( DAL_VOICE_PARMS *parms, unsigned int value );

/*****************************************************************
**  FUNCTION:       dalVoice_SetMaxSessionsPerLine
**
**  PURPOSE:        Sets max no. of sessions per line, as int
**
**  INPUT PARMS:    
**
**  OUTPUT PARMS:   
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetMaxSessionsPerLine( DAL_VOICE_PARMS *parms, unsigned int value );

/*****************************************************************
**  FUNCTION:       dalVoice_SetMaxSessionsPerExt
**
**  PURPOSE:        Sets max no. of sessions per extension, as int
**
**  INPUT PARMS:    
**
**  OUTPUT PARMS:   
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetMaxSessionsPerExt( DAL_VOICE_PARMS *parms, unsigned int value );

/*****************************************************************
**  FUNCTION:       dalVoice_SetMaxSessionCount
**
**  PURPOSE:        Sets max no. of sessions supported across all lines, as int
**
**  INPUT PARMS:    
**
**  OUTPUT PARMS:   
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetMaxSessionCount( DAL_VOICE_PARMS *parms, unsigned int value );

/****************************************************************************
* FUNCTION:    dalVoice_SetVoiceFxsLineEnable
*
* PURPOSE:     Set enable value for the given FXS line
*
* PARAMETERS:  Voice profile and FXS line #
*
* RETURNS:     true if enabled, false otherwise
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_SetVoiceFxsLineEnable( DAL_VOICE_PARMS *parms, char* value );

/****************************************************************************
* FUNCTION:    dalVoice_SetVoiceFxsLineFaxPass
*
* PURPOSE:     Set fax passthrough config for the given FXS line
*
* PARAMETERS:  Voice profile and FXS line #
*
* RETURNS:     string to indicate fax config, as per TR-104v2 spec
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_SetVoiceFxsLineFaxPass( DAL_VOICE_PARMS *parms, char* value );

/****************************************************************************
* FUNCTION:    dalVoice_SetVoiceFxsLineModemPass
*
* PURPOSE:     Set modem passthrough config for the given FXS line
*
* PARAMETERS:  Voice profile and FXS line #
*
* RETURNS:     string to indicate modem config, as per TR-104v2 spec
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_SetVoiceFxsLineModemPass( DAL_VOICE_PARMS *parms, char* value );

/****************************************************************************
* FUNCTION:    dalVoice_SetVoiceFxsLineDialType
*
* PURPOSE:     Set dial type config for the given FXS line
*
* PARAMETERS:  Voice profile and FXS line #
*
* RETURNS:     string to indicate dial type config, as per TR-104v2 spec
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_SetVoiceFxsLineDialType( DAL_VOICE_PARMS *parms, char* value );

/****************************************************************************
* FUNCTION:    dalVoice_SetVoiceFxsLineClipGen
*
* PURPOSE:     Set dial type config for the given FXS line
*
* PARAMETERS:  Voice profile and FXS line #
*
* RETURNS:     string to indicate whether CLIP is generated by the board
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_SetVoiceFxsLineClipGen( DAL_VOICE_PARMS *parms, char* value );

/****************************************************************************
* FUNCTION:    dalVoice_SetVoiceFxsLineTermType
*
* PURPOSE:     Set terminal type config for the given FXS line
*
* PARAMETERS:  None
*
* RETURNS:     string to indicate the terminal type
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_SetVoiceFxsLineTermType( DAL_VOICE_PARMS *parms, char* value );

/****************************************************************************
* FUNCTION:    dalVoice_SetVoiceFxsLineHookState
*
* PURPOSE:     Set hook state for the given FXS line
*
* PARAMETERS:  Voice service # and FXS line # and hook state string.  Must be
*              MDMVS_ONHOOK, MDMVS_ONHOOKWITHACTIVITY, or MDMVS_OFFHOOK
*
* RETURNS:     string to indicate whether CLIP is generated by the board
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_SetVoiceFxsLineHookState( DAL_VOICE_PARMS *parms, char* value );

/****************************************************************************
* FUNCTION:    dalVoice_SetVoiceFxsLineTxGain
*
* PURPOSE:     Set TX gain for the given FXS line
*
* PARAMETERS:  Voice profile and FXS line #
*
* RETURNS:     
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_SetVoiceFxsLineTxGain( DAL_VOICE_PARMS *parms, int txGain);

/****************************************************************************
* FUNCTION:    dalVoice_SetVoiceFxsLineRxGain
*
* PURPOSE:     Set RX gain for the given FXS line
*
* PARAMETERS:  Voice profile and FXS line #
*
* RETURNS:     
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_SetVoiceFxsLineRxGain( DAL_VOICE_PARMS *parms, int rxGain);

/****************************************************************************
* FUNCTION:    dalVoice_SetVoiceFxsLineTxGainStr
*
* PURPOSE:     Set TX gain for the given FXS line, in string form
*
* PARAMETERS:  Voice profile and FXS line #
*
* RETURNS:     
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_SetVoiceFxsLineTxGainStr( DAL_VOICE_PARMS *parms, char *txGain);

/****************************************************************************
* FUNCTION:    dalVoice_SetVoiceFxsLineRxGainStr
*
* PURPOSE:     Set RX gain for the given FXS line, in string form
*
* PARAMETERS:  Voice profile and FXS line #
*
* RETURNS:     
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_SetVoiceFxsLineRxGainStr( DAL_VOICE_PARMS *parms, char *rxGain);

/*****************************************************************
**  FUNCTION:       dalVoice_SetSrtpEnable
**
**  PURPOSE:        Set SRTP enable flag
**
**  INPUT PARMS:    vpInst - parms->op[0];
**
**  OUTPUT PARMS:   None
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetSrtpEnable(DAL_VOICE_PARMS *parms, char *enableValue );

/****************************************************************************
* FUNCTION:    dalVoice_SetWanPortRange
*
* PURPOSE:     Set the range of ports used by WAN interface
*
* PARAMETERS:  None
*
* RETURNS:     Comma-separated list WAN port ranges
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_SetWanPortRange( DAL_VOICE_PARMS *parms, char* value);

/****************************************************************************
* FUNCTION:    dalVoice_SetLanPortRange
*
* PURPOSE:     Set the range of ports used by LAN interface
*
* PARAMETERS:  None
*
* RETURNS:     Comma-separated list LAN port ranges
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_SetLanPortRange( DAL_VOICE_PARMS *parms, char* value);

/*****************************************************************
**  FUNCTION:       dalVoice_GetDTMFMethodIntValue
**
**  PURPOSE:
**
**  INPUT PARAMS:   vpInst  - parms->op[0]
**
**  OUTPUT PARMS:   0 - InBand
**                  1 - RFC2833
**                  2 - SIPINFO
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetDTMFMethodIntValue(DAL_VOICE_PARMS *parms, char *dtmfRelay, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetDTMFMethod
**
**  PURPOSE:
**
**  INPUT PARAMS:   vpInst  - parms->op[0]
**
**  OUTPUT PARMS:   dtmfRelay - InBand
**                            - SIPINFO
**                            - IFC2833
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetDTMFMethod(DAL_VOICE_PARMS *parms, char *dtmfRelay, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetHookFlashMethod
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst   - parms->op[0]
**                  profInst - parms->op[1]
**
**  OUTPUT PARMS:   method - Hook flash method
**                         - None/SIPInfo
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetHookFlashMethod(DAL_VOICE_PARMS *parms, char *method, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetMaxPrefCodecs
**
**  PURPOSE:
**
**  INPUT PARAMS:   None
**
**  OUTPUT PARMS:   value - maximum number of preferred codecs
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetMaxPrefCodecs( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetVlCLCodecList
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0];
**                  lineInst  - parms->op[1]
**
**  OUTPUT PARMS:   codec - Priority sorted list of encoders
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVlCLCodecList(DAL_VOICE_PARMS *parms, char *codec, unsigned int length );

#ifdef BRCM_SIP_TLS_SUPPORT
/*****************************************************************
**  FUNCTION:       dalVoice_GetLocalSipCertPrivKey
**
**  PURPOSE:
**
**  INPUT PARMS:    None
**
**  OUTPUT PARMS:   Private key of the "sipcert" local certificate,
**                   provided it is configured in MDM
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetLocalSipCertPrivKey(DAL_VOICE_PARMS *parms, char *privKey, unsigned int length);

/*****************************************************************
**  FUNCTION:       dalVoice_GetLocalSipCertContents
**
**  PURPOSE:
**
**  INPUT PARMS:    None
**
**  OUTPUT PARMS:   Contents of the "sipcert" local certificate,
**                   provided it is configured in MDM
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetLocalSipCertContents(DAL_VOICE_PARMS *parms, char *contents, unsigned int length);

/*****************************************************************
**  FUNCTION:       dalVoice_GetTrustedCaSipCertContents
**
**  PURPOSE:
**
**  INPUT PARMS:    None
**
**  OUTPUT PARMS:   Contents of the "sipcert" trusted certificate,
**                   provided it is configured in MDM
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetTrustedCaSipCertContents(DAL_VOICE_PARMS *parms, char *contents, unsigned int length);

#endif /* BRCM_SIP_TLS_SUPPORT */

/*****************************************************************
**  FUNCTION:       dalVoice_GetVlCFWarmLineNum
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0];
**                  lineInst - parms->op[1]
**
**  OUTPUT PARMS:   warmLineNumber - Warm Line Number
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVlCFWarmLineNum(DAL_VOICE_PARMS *parms, char *warmLineNumber, unsigned int length );

#endif /* SIPLOAD */

/*****************************************************************
**  FUNCTION:       dalVoice_GetSrtpOptionString
**
**  PURPOSE:        Get SRTP usage option )mandatory, optional or disabled)
**
**  INPUT PARMS:    vpInst - parms->op[0];
**
**  OUTPUT PARMS:   String Value of SRTP option
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSrtpOptionString(DAL_VOICE_PARMS *parms, char *option, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetRedOptionString
**
**  PURPOSE:        Get RED option option (Disabled,1,2,3)
**
**  INPUT PARMS:    vpInst - parms->op[0];
**
**  OUTPUT PARMS:   String Value of RED option
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetRedOptionString(DAL_VOICE_PARMS *parms, char *option, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSrtpOption
**
**  PURPOSE:        Get SRTP usage option (mandatory, optional or disabled)
**                  in enum form
**
**  INPUT PARMS:    vpInst - parms->op[0];
**
**  OUTPUT PARMS:   String Value of SRTP option
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSrtpOption(DAL_VOICE_PARMS *parms, char *option, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetLoggingLevel
**
**  PURPOSE:
**
**  INPUT PARMS:    mdm log level for voice
**
**  OUTPUT PARMS:
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetLoggingLevel(  DAL_VOICE_PARMS *parms, char * setVal );

/*****************************************************************
**  FUNCTION:       dalVoice_SetModuleLoggingLevel
**
**  PURPOSE:        Set a specific module's logging level
**
**  INPUT PARMS:    module name, mdm log level for voice
**
**  OUTPUT PARMS:
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetModuleLoggingLevel( DAL_VOICE_PARMS *parms, char* modName, char* setVal );

/*****************************************************************
**  FUNCTION:       dalVoice_SetModuleLoggingLevels
**
**  PURPOSE:        Set the entire log levels string
**
**  INPUT PARMS:    mdm log level string for all voice modules
**
**  OUTPUT PARMS:
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetModuleLoggingLevels( DAL_VOICE_PARMS *parms, char* setVal );

/*****************************************************************
**  FUNCTION:       dalVoice_SetManagementProtocol
**
**  PURPOSE:        Track Protocol used to Manage Voice
**
**  INPUT PARMS:    Protocol Identifier
**
**  OUTPUT PARMS:   None
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetManagementProtocol(  DAL_VOICE_PARMS *parms, char * setVal );

#ifdef SIPLOAD
/*****************************************************************
**  FUNCTION:       dalVoice_SetCCTKTraceLevel
**
**  PURPOSE:
**
**  INPUT PARMS:    cctk trace level
**
**  OUTPUT PARMS:
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetCCTKTraceLevel(  DAL_VOICE_PARMS *parms, char * setVal );

/*****************************************************************
**  FUNCTION:       dalVoice_SetCCTKTraceGroup
**
**  PURPOSE:
**
**  INPUT PARMS:    cctk trace group
**
**  OUTPUT PARMS:
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetCCTKTraceGroup(  DAL_VOICE_PARMS *parms, char * setVal );
#endif /* SIPLOAD */


/*****************************************************************
**  FUNCTION:       dalVoice_SetBoundIfName
**
**  PURPOSE:
**
**  INPUT PARMS:    bound ifname for  voice
**
**  OUTPUT PARMS:
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetBoundIfName( DAL_VOICE_PARMS *parms, char * setVal );

/*****************************************************************
**  FUNCTION:       dalVoice_SetIpFamily
**
**  PURPOSE:
**
**  INPUT PARMS:    set IP address family for  voice
**
**  OUTPUT PARMS:
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetIpFamily( DAL_VOICE_PARMS *parms, char * setVal );

/*****************************************************************
**  FUNCTION:       dalVoice_SetBoundIPAddr
**
**  PURPOSE:        Stores the specified bound IP address in MDM.
**
**  INPUT PARMS:    bound ipaddr for  voice
**
**  OUTPUT PARMS:   None
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetBoundIPAddr( DAL_VOICE_PARMS *parms, char *setVal );

/*****************************************************************
**  FUNCTION:       dalVoice_SetVoiceDnsEnable
**
**  PURPOSE:        Set the enable or disable flag for voice DNS
**                  (X_BROADCOM_COM_VoiceDnsEnable)
**
**  INPUT PARMS:    Enable flag (MDMVS_ON, or MDMVS_OFF)
**
**  OUTPUT PARMS:   None
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetVoiceDnsEnable ( DAL_VOICE_PARMS *parms, char *setVal );

/*****************************************************************
**  FUNCTION:       dalVoice_SetDnsServerAddrPri
**
**  PURPOSE:        Set IP address of the primary voice DNS server
**                  (X_BROADCOM_COM_VoiceDnsServerPri)
**
**  INPUT PARMS:    Primary DNS IP addr for voice
**
**  OUTPUT PARMS:   None
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetDnsServerAddrPri ( DAL_VOICE_PARMS *parms, char *setVal );

/*****************************************************************
**  FUNCTION:       dalVoice_SetDnsServerAddrSec
**
**  PURPOSE:        Set IP address of the secondary voice DNS server
**                  (X_BROADCOM_COM_VoiceDnsServerSec)
**
**  INPUT PARMS:    Secondary DNS IP addr for voice
**
**  OUTPUT PARMS:   None
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetDnsServerAddrSec ( DAL_VOICE_PARMS *parms, char *setVal );

/*************************************************************
**  FUNCTION:       dalVoice_GetSTUNServerPort
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0]
**
**  OUTPUT PARMS:   port - STUN server port
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSTUNServerPort(DAL_VOICE_PARMS *parms, char *port, unsigned int length );

#ifdef DMP_X_BROADCOM_COM_PSTNENDPOINT_1
/*****************************************************************
**  FUNCTION:       dalVoice_GetPstnDialPlan
**
**  PURPOSE:
**
**  INPUT PARMS:    PSTN instance  - parms->op[0]
**
**  OUTPUT PARMS:   dialPlan - PSTN outgoing dial plan
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetPstnDialPlan(DAL_VOICE_PARMS *parms, char *dialPlan, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetPstnRouteRule
**
**  PURPOSE:
**
**  INPUT PARMS:    PSTN instance  - parms->op[0]
**
**  OUTPUT PARMS:   mode - PSTN call routing mode
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetPstnRouteRule(DAL_VOICE_PARMS *parms, char *mode, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetPstnRouteData
**
**  PURPOSE:
**
**  INPUT PARMS:    PSTN instance  - parms->op[0]
**
**  OUTPUT PARMS:   dest - PSTN call routing destination
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetPstnRouteData(DAL_VOICE_PARMS *parms, char *dest, unsigned int length );
#endif /* DMP_X_BROADCOM_COM_PSTNENDPOINT_1 */

/*****************************************************************
**  FUNCTION:       dalVoice_GetVlEnable
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst    - parms->op[0]
**                  lineInst - parms->op[1]
**
**  OUTPUT PARMS:   Line Enabled status
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVlEnable(DAL_VOICE_PARMS *parms, char *lineEnabled, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetVlDisable
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst    - parms->op[0]
**                  lineInst - parms->op[1]
**
**  OUTPUT PARMS:   Line Disable status
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
** NOTE: There is no line disable object in TR104 specifying
**       whether the line is enabled or disabled. This function simply
**       returns the inverted value of dalVoice_GetVlEnable
**
*******************************************************************/
CmsRet dalVoice_GetVlDisable(DAL_VOICE_PARMS *parms, char *lineDisabled, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetVlPhyReferenceList
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst  - parms->op[0]
**                  lineInst  - parms->op[1]
**
**  OUTPUT PARMS:   physEptId - Physical endpoint ID
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVlPhyReferenceList(DAL_VOICE_PARMS *parms, char *physEptId, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSilenceSuppression
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0];
**
**  OUTPUT PARMS:   vad - Silence suppression
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSilenceSuppression(DAL_VOICE_PARMS *parms, char *vad, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetDigitMap
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst  - parms->op[0]
**                  lineInst  - parms->op[1]
**
**  OUTPUT PARMS:   map - Dialing Digits Mapping
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet  dalVoice_GetDigitMap(DAL_VOICE_PARMS *parms, char *map, unsigned int length );

#ifdef SIPLOAD
/*****************************************************************
**  FUNCTION:       dalVoice_GetCCTKDigitMap
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst  - parms->op[0]
**                  lineInst  - parms->op[1]
**
**  OUTPUT PARMS:   map - Dialing Digits Mapping (custom)
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet  dalVoice_GetCCTKDigitMap(DAL_VOICE_PARMS *parms, char *map, unsigned int length );
#endif

/*****************************************************************
**  FUNCTION:       dalVoice_GetCriticalDigitTimer
**
**  PURPOSE:        Return value of
**                  VoiceProfile.{i}.X_BROADCOM_COM_CriticalDigitTimer
**
**  INPUT PARMS:    parms->op[0] = Voice Profile Instance
**                  length       = Maximum length of output string
**
**  OUTPUT PARMS:   timer - Critical Digit Timing
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetCriticalDigitTimer ( DAL_VOICE_PARMS *parms, char *timer, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetPartialDigitTimer
**
**  PURPOSE:        Return value of
**                  VoiceProfile.{i}.X_BROADCOM_COM_PartialDigitTimer
**
**  INPUT PARMS:    parms->op[0] = Voice Profile Instance
**                  length       = Maximum length of output string
**
**  OUTPUT PARMS:   timer - Partial Digit Timing
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetPartialDigitTimer ( DAL_VOICE_PARMS *parms, char *timer, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetVlCLEncoder
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0];
**                  lineInst  - parms->op[1]
**                  codec priority - parms->op[2]
**
**  OUTPUT PARMS:   codec - Encoder of specific priority
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVlCLEncoder(DAL_VOICE_PARMS *parms, char *codec, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetIpAddr
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst  - parms->op[0]
**
**  OUTPUT PARMS:   Interface IP Address
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet  dalVoice_GetIpAddr(DAL_VOICE_PARMS *parms, char *ipAddr, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetFlexTermSupport
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst    - parms->op[0]
**                  lineInst  - parms->op[1]
**
**  OUTPUT PARMS:   True if CMGR type is CCTK,
**                  False if type is CALLCTL.
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetFlexTermSupport( DAL_VOICE_PARMS *parms, char* type, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetT38Enable
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst    - parms->op[0]
**
**  OUTPUT PARMS:   enabled - T38 Enabled
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetT38Enable(DAL_VOICE_PARMS *parms, char *enabled, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetV18Enable
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst    - parms->op[0]
**
**  OUTPUT PARMS:   enabled - V18 Enabled
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetV18Enable(DAL_VOICE_PARMS *parms, char *enabled, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetVBDEnable
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst    - parms->op[0]
**
**  OUTPUT PARMS:   enabled - VBD Enabled
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
** NOTE: There is no VBD object in TR104 specifying whether voice-band
**       data mode is enabled or disabled. This function simply
**       returns the inverted value of dalVoice_GetT38Enable
**
*******************************************************************/
CmsRet dalVoice_GetVBDEnable(DAL_VOICE_PARMS *parms, char *enabled, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSTUNServer
**
**  PURPOSE:
**
**  INPUT PARAMS:   vpInst  - parms->op[0]
**
**  OUTPUT PARMS:   stunServer - STUN server IP address
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet  dalVoice_GetSTUNServer(DAL_VOICE_PARMS *parms, char *stunServer, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetRegion
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst  - parms->op[0]
**
**  OUTPUT PARMS:   country Alpha3 string
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetRegion(DAL_VOICE_PARMS *parms, char *country, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetRegionVrgCode
**
**  PURPOSE:
**
**  INPUT PARAMS:   vpInst  - parms->op[0]
**
**  OUTPUT PARMS:   vrg country code
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet  dalVoice_GetRegionVrgCode(DAL_VOICE_PARMS *parms, char *country, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetRegionSuppString
**
**  PURPOSE:
**
**  INPUT PARMS:
**
**  OUTPUT PARMS:   value - list of supported locales in Alpha3
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetRegionSuppString( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetVlCFMWIEnable
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst    - parms->op[0]
**                  lineInst  - parms->op[1]
**
**  OUTPUT PARMS:   enable - MWI enable
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVlCFMWIEnable(DAL_VOICE_PARMS *parms, char *enable, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetRtpDSCPMark
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0];
**
**  OUTPUT PARMS:   dscpMark - Value of RTP DSCP mark
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet  dalVoice_GetRtpDSCPMark(DAL_VOICE_PARMS *parms, char *dscpMark, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSrtpEnabled
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0];
**
**  OUTPUT PARMS:   getVal - Value of Srtp enable, 0 ( false ) or 1 ( true )
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSrtpEnabled(DAL_VOICE_PARMS *parms, char *getVal, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetRtpLocalPortMin
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0];
**
**  OUTPUT PARMS:   port - Value of RTP min port
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetRtpLocalPortMin(DAL_VOICE_PARMS *parms, char *port, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetRtpLocalPortMax
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0];
**
**  OUTPUT PARMS:   port - Value of RTP max port
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetRtpLocalPortMax(DAL_VOICE_PARMS *parms, char *port, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetIpv6Enabled
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0];
**
**  OUTPUT PARMS:   enabled - Enabled flag
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetIpv6Enabled(DAL_VOICE_PARMS *parms, char *enabled, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetVlCFCallFwdNum
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0];
**                  lineInst - parms->op[1]
**
**  OUTPUT PARMS:   cfNumber - Call Forward Number
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVlCFCallFwdNum(DAL_VOICE_PARMS *parms, char *cfNumber, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetEptAppStatus
**
**  PURPOSE:        Get EptApp Enable flag
**
**  INPUT PARMS:    None 
**
**  OUTPUT PARMS:   enabled - EptApp Enabled
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
#ifdef EPTAPPLOAD
CmsRet dalVoice_GetEptAppStatus(DAL_VOICE_PARMS *parms, char *enabled, unsigned int length );
#endif

/***************************************************************************
* Function Name: dalVoice_cliDumpParams
* Description  : Dumps voice parameters on console
*
* Parameters   : none
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_cliDumpParams(void);
#ifdef SIPLOAD
/***************************************************************************
* Function Name: dalVoice_cliDumpStats
* Description  : Dumps voice call statistics on console
*
* Parameters   : none
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_cliDumpStats(void);
#endif /* SIPLOAD */
/***************************************************************************
* Function Name: dalVoice_GetLoggingLevel
* Description  : Gets the voice logging level
*
* Parameters   : none
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetLoggingLevel( DAL_VOICE_PARMS *parms, char * getVal, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetModuleLoggingLevels
* Description  : Gets the logging levels for all modules
*
* Parameters   : none
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetModuleLoggingLevels( DAL_VOICE_PARMS *parms, char * getVal, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetModuleLoggingLevel
* Description  : Gets the specific voice module's logging level
*
* Parameters   : none
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetModuleLoggingLevel( DAL_VOICE_PARMS *parms, char* modName, char * getVal, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetManagementProtocol
* Description  : Gets the Protocol used to manage the Voice Service
*
* Parameters   : none
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetManagementProtocol( DAL_VOICE_PARMS *parms, char * getVal, unsigned int length );

#ifdef SIPLOAD
/*****************************************************************
**  FUNCTION:       dalVoice_GetCCTKTraceLevel
**
**  PURPOSE:
**
**  INPUT PARMS:    None
**
**  OUTPUT PARMS:   cctk trace level
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetCCTKTraceLevel( DAL_VOICE_PARMS *parms, char * getVal, unsigned int length);

/*****************************************************************
**  FUNCTION:       dalVoice_GetCCTKTraceGroup
**
**  PURPOSE:
**
**  INPUT PARMS:    None
**
**  OUTPUT PARMS:   cctk trace group
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetCCTKTraceGroup( DAL_VOICE_PARMS *parms, char * getVal, unsigned int length);
#endif /* SIPLOAD */

/*****************************************************************
**  FUNCTION:       dalVoice_GetMgtProtList
**
**  PURPOSE:
**
**  INPUT PARMS:    None
**
**  OUTPUT PARMS:   List of management protocols for  voice
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetMgtProtList( DAL_VOICE_PARMS *parms, char * getVal, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSupportedDtmfMethods
**
**  PURPOSE:
**
**  INPUT PARMS:    None
**
**  OUTPUT PARMS:   Set of supported DTMF methods
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSupportedDtmfMethods( DAL_VOICE_PARMS *parms, char * getVal, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSupportedHookFlashMethods
**
**  PURPOSE:
**
**  INPUT PARMS:    None
**
**  OUTPUT PARMS:   Set of supported hook flash methods
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSupportedHookFlashMethods( DAL_VOICE_PARMS *parms, char * getVal, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_GetSignalingProtocol
*
* PURPOSE:     Get signaling protocol capability
*
* PARAMETERS:  None
*
* RETURNS:     Supported signalling protocols
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetSignalingProtocol( DAL_VOICE_PARMS *parms, char* sigProt, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_GetSupportedTransports
*
* PURPOSE:     Get list of available Transport layer protocols
*
* PARAMETERS:  None
*
* RETURNS:     Supported Transport layer protocols
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetSupportedTransports( DAL_VOICE_PARMS *parms, char* transports, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_GetSupportedSrtpOptions
*
* PURPOSE:     Get list of available SRTP options
*
* PARAMETERS:  None
*
* RETURNS:     Supported SRTP options
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetSupportedSrtpOptions( DAL_VOICE_PARMS *parms, char* options, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_GetSupportedBackToPrimOptions
*
* PURPOSE:     Get list of available back-to-primary failover options
*
* PARAMETERS:  None
*
* RETURNS:     Supported back-to-primary options for failover
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetSupportedBackToPrimOptions( DAL_VOICE_PARMS *parms, char* options, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_GetSupportedRedOptions
*
* PURPOSE:     Get list of available RED options
*
* PARAMETERS:  None
*
* RETURNS:     Supported Red options
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetSupportedRedOptions( DAL_VOICE_PARMS *parms, char* options, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_GetSupportedConfOptions
*
* PURPOSE:     Get list of available conferencing options
*
* PARAMETERS:  None
*
* RETURNS:     Supported conferencing options
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetSupportedConfOptions( DAL_VOICE_PARMS *parms, char* options, unsigned int length );

#ifdef DMP_VOICE_SERVICE_2
/****************************************************************************
* FUNCTION:    dalVoice_GetMaxExtensionPerVoiceProfile
*
* PURPOSE:     Get maximum number of extensions per voice profile
*
* PARAMETERS:  None
*
* RETURNS:     maximum call log instance number
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetMaxExtensionPerVoiceProfile( DAL_VOICE_PARMS *parms, int* value );

/****************************************************************************
* FUNCTION:    dalVoice_GetMaxCallsPerLine
*
* PURPOSE:     Get maximum number of calls per line
*
* PARAMETERS:  None
*
* RETURNS:     maximum number of calls per line
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetMaxCallsPerLine( DAL_VOICE_PARMS *parms, int* value );

/****************************************************************************
* FUNCTION:    dalVoice_GetMaxCallsPerExtension
*
* PURPOSE:     Get maximum number of calls per extension
*
* PARAMETERS:  None
*
* RETURNS:     maximum number of calls per extension
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetMaxCallsPerExtension( DAL_VOICE_PARMS *parms, int* value );

/****************************************************************************
* FUNCTION:    dalVoice_GetMaxCalls
*
* PURPOSE:     Get maximum number of calls per extension
*
* PARAMETERS:  None
*
* RETURNS:     maximum number of calls across system
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetMaxCalls( DAL_VOICE_PARMS *parms, int* value );

/****************************************************************************
* FUNCTION:    dalVoice_GetMaxCallLogCount
*
* PURPOSE:     Get maximum call log instance number
*
* PARAMETERS:  parms->op[0] = vpInst
*
* RETURNS:     maximum call log instance number
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetMaxCallLogCount( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_GetUserSignalingProtocol
*
* PURPOSE:     Get user signaling protocol capability
*
* PARAMETERS:  None
*
* RETURNS:     User-supported signalling protocols
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetUserSignalingProtocol( DAL_VOICE_PARMS *parms, char* sigProt, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_GetToneFileFormats
*
* PURPOSE:     Get supported tone file formats
*
* PARAMETERS:  None
*
* RETURNS:     Comma-separated tone file formats
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetToneFileFormats( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_GetRingFileFormats
*
* PURPOSE:     Get supported ring file formats
*
* PARAMETERS:  None
*
* RETURNS:     Comma-separated ring file formats
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetRingFileFormats( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_GetFacilityActions
*
* PURPOSE:     Get facility actions	supported by this voice service instance
*
* PARAMETERS:  None
*
* RETURNS:     maximum call log instance number
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetFacilityActions( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_GetExtensions
*
* PURPOSE:     Get supported SIP extensions
*
* PARAMETERS:  None
*
* RETURNS:     Comma-separated list of SIP extensions
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetExtensions( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_GetURISchemes
*
* PURPOSE:     Get supported URI schemes
*
* PARAMETERS:  None
*
* RETURNS:     Comma-separated list of URI schemes
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetURISchemes( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_GetEventTypes
*
* PURPOSE:     Get supported SIP event types
*
* PARAMETERS:  None
*
* RETURNS:     Comma-separated list of SIP event types
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetEventTypes( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_GetTLSAuthenticationProtocols
*
* PURPOSE:     Get supported TLS Authentication Protocols
*
* PARAMETERS:  None
*
* RETURNS:     Comma-separated list of TLS Authentication Protocols
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetTLSAuthenticationProtocols( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_GetTLSAuthenticationKeySizes
*
* PURPOSE:     Get supported TLS Authentication key sizes
*
* PARAMETERS:  None
*
* RETURNS:     Comma-separated list of TLS Authentication key sizes
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetTLSAuthenticationKeySizes( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_GetTLSEncryptionProtocols
*
* PURPOSE:     Get supported TLS Encryption Protocols
*
* PARAMETERS:  None
*
* RETURNS:     Comma-separated list of TLS Encryption Protocols
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetTLSEncryptionProtocols( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_GetTLSEncryptionKeySizes
*
* PURPOSE:     Get supported TLS Encryption key sizes
*
* PARAMETERS:  None
*
* RETURNS:     Comma-separated list of TLS Encryption key sizes
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetTLSEncryptionKeySizes( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_GetTLSKeyExchangeProtocols
*
* PURPOSE:     Get supported TLS key exchange protocols
*
* PARAMETERS:  None
*
* RETURNS:     Comma-separated list of TLS key exchange protocols
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetTLSKeyExchangeProtocols( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_GetPotsDialType
*
* PURPOSE:     Get dial type from POTS capabilities
*
* PARAMETERS:  None
*
* RETURNS:     Supported dial type
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetPotsDialType( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_GetPotsClipGeneration
*
* PURPOSE:     Get CLIP generation from POTS capabilities
*
* PARAMETERS:  None
*
* RETURNS:     true if CLIP generation is supported, false otherwise 
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetPotsClipGeneration( DAL_VOICE_PARMS *parms, char* value );

/****************************************************************************
* FUNCTION:    dalVoice_GetPotsChargingPulse
*
* PURPOSE:     Get charging pulse support value from POTS capabilities
*
* PARAMETERS:  None
*
* RETURNS:     true if pulse charging is supported, false otherwise
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetPotsChargingPulse( DAL_VOICE_PARMS *parms, char* value );

/****************************************************************************
* FUNCTION:    dalVoice_GetMaxQIValues
*
* PURPOSE:     Get the maximum number of QI values which can be reported for a session
*
* PARAMETERS:  None
*
* RETURNS:     maximum number of QI values which can be reported for a session 
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetMaxQIValues( DAL_VOICE_PARMS *parms, unsigned int* value );

/****************************************************************************
* FUNCTION:    dalVoice_GetMaxWorstQIValues
*
* PURPOSE:     Get the maximum number of worst QI values which can be reported 
*
* PARAMETERS:  None
*
* RETURNS:     maximum number of worst QI values that CPE can store and report 
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetMaxWorstQIValues( DAL_VOICE_PARMS *parms, unsigned int* value );

/****************************************************************************
* FUNCTION:    dalVoice_GetWanPortRange
*
* PURPOSE:     Get the range of ports used by WAN interface
*
* PARAMETERS:  None
*
* RETURNS:     Comma-separated list WAN port ranges
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetWanPortRange( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_GetLanPortRange
*
* PURPOSE:     Get the range of ports used by LAN interface
*
* PARAMETERS:  None
*
* RETURNS:     Comma-separated list LAN port ranges
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetLanPortRange( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_GetVoiceFxsLineEnable
*
* PURPOSE:     Get enable value for the given FXS line
*
* PARAMETERS:  None
*
* RETURNS:     true if enabled, false otherwise
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetVoiceFxsLineEnable( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_GetVoiceFxsLineStatus
*
* PURPOSE:     Get status for the given FXS line
*
* PARAMETERS:  None
*
* RETURNS:     string to indicate line status, as per TR-104v2 spec
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetVoiceFxsLineStatus( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_GetVoiceFxsLineFaxPass
*
* PURPOSE:     Get fax passthrough config for the given FXS line
*
* PARAMETERS:  None
*
* RETURNS:     string to indicate fax config, as per TR-104v2 spec
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetVoiceFxsLineFaxPass( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_GetVoiceFxsLineModemPass
*
* PURPOSE:     Get modem passthrough config for the given FXS line
*
* PARAMETERS:  None
*
* RETURNS:     string to indicate modem config, as per TR-104v2 spec
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetVoiceFxsLineModemPass( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_GetVoiceFxsLineDialType
*
* PURPOSE:     Get dial type config for the given FXS line
*
* PARAMETERS:  None
*
* RETURNS:     string to indicate dial type config, as per TR-104v2 spec
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetVoiceFxsLineDialType( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_GetVoiceFxsLineClipGen
*
* PURPOSE:     Get dial type config for the given FXS line
*
* PARAMETERS:  None
*
* RETURNS:     string to indicate whether CLIP is generated by the board
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetVoiceFxsLineClipGen( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_GetVoiceFxsLineTermType
*
* PURPOSE:     Get terminal type config for the given FXS line
*
* PARAMETERS:  None
*
* RETURNS:     string to indicate the terminal type
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetVoiceFxsLineTermType( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_GetVoiceFxsLineHookState
*
* PURPOSE:     Get hook state for the given FXS line
*
* PARAMETERS:  Voice profile and FXS line #
*
* RETURNS:     string to indicate hook state
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetVoiceFxsLineHookState( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_GetVoiceFxsLineTxGain
*
* PURPOSE:     Get TX gain for the given FXS line
*
* PARAMETERS:  Voice profile and FXS line #
*
* RETURNS:     TX gain for the given FXS line
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetVoiceFxsLineTxGain( DAL_VOICE_PARMS *parms, int* transmitGain);

/****************************************************************************
* FUNCTION:    dalVoice_GetVoiceFxsLineRxGain
*
* PURPOSE:     Get RX gain for the given FXS line
*
* PARAMETERS:  Voice profile and FXS line #
*
* RETURNS:     RX gain for the given FXS line
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetVoiceFxsLineRxGain( DAL_VOICE_PARMS *parms, int* receiveGain);

/****************************************************************************
* FUNCTION:    dalVoice_GetVoiceFxsLineTxGainStr
*
* PURPOSE:     Get TX gain for the given FXS line, in string format
*
* PARAMETERS:  Voice profile and FXS line #
*
* RETURNS:     TX gain for the given FXS line
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetVoiceFxsLineTxGainStr( DAL_VOICE_PARMS *parms, char* transmitGain, unsigned int length);

/****************************************************************************
* FUNCTION:    dalVoice_GetVoiceFxsLineRxGainStr
*
* PURPOSE:     Get RX gain for the given FXS line, in string format
*
* PARAMETERS:  Voice profile and FXS line #
*
* RETURNS:     RX gain for the given FXS line
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetVoiceFxsLineRxGainStr( DAL_VOICE_PARMS *parms, char* receiveGain, unsigned int length);

#endif

/****************************************************************************
* FUNCTION:    dalVoice_GetNetworkIntfList
*
* PURPOSE:     Get list of available network interfaces for voice
*
* PARAMETERS:  None
*
* RETURNS:     list of available network interfaces for voice
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetNetworkIntfList( DAL_VOICE_PARMS *parms, char* intfList, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_GetNetworkIntfList
*
* PURPOSE:     Get list of available network instances
*
* PARAMETERS:  None
*
* RETURNS:     Supported signalling protocols
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetNetworkIntfList_igd( DAL_VOICE_PARMS *parms, char* intfList, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNetworkIntfList_dev2
**
**  PURPOSE:        Obtains the list of available network interfaces
**
**  INPUT PARMS:    None
**
**  OUTPUT PARMS:   intfList - obtained list of network interfaces
**
**  RETURNS:        CMSRET_SUCCESS - Success
**
**  NOTE:           
*******************************************************************/
CmsRet dalVoice_GetNetworkIntfList_dev2(DAL_VOICE_PARMS *parms, char* intfList, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetBoundIfName
**
**  PURPOSE:
**
**  INPUT PARMS:    None
**
**  OUTPUT PARMS:   bound ifname for  voice
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetBoundIfName( DAL_VOICE_PARMS *parms, char * getVal, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_GetIpFamilyList
*
* PURPOSE:     Get list of available IP address families for voice
*
* PARAMETERS:  None
*
* RETURNS:     list of available network interfaces for voice
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetIpFamilyList( DAL_VOICE_PARMS *parms, char* intfList, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetIpFamily
**
**  PURPOSE:
**
**  INPUT PARMS:    None
**
**  OUTPUT PARMS:   IP address family for voice
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetIpFamily( DAL_VOICE_PARMS *parms, char * getVal, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetBoundIPAddr
**
**  PURPOSE:
**
**  INPUT PARMS:    None
**
**  OUTPUT PARMS:   bound ifname for  voice
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetBoundIPAddr( DAL_VOICE_PARMS *parms, char * getVal, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetVoiceDnsEnable
**
**  PURPOSE:        Check if voice DNS is enabled
**                  (X_BROADCOM_COM_VoiceDnsEnable)
**
**  INPUT PARMS:    None
**
**  OUTPUT PARMS:   getVal - Voice DNS enable flag ("0" or "1")
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVoiceDnsEnable(DAL_VOICE_PARMS *parms, char *getVal, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetDnsServerAddrPri
**
**  PURPOSE:        Get IP address of the primary voice DNS server
*                   (X_BROADCOM_COM_VoiceDnsServerPri)
**
**  INPUT PARMS:    None
**
**  OUTPUT PARMS:   getVal - Primary voice DNS server address
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetDnsServerAddrPri(DAL_VOICE_PARMS *parms, char *getVal, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetDnsServerAddrSec
**
**  PURPOSE:        Get IP address of the secondary voice DNS server
*                   (X_BROADCOM_COM_VoiceDnsServerSec)
**
**  INPUT PARMS:    None
**
**  OUTPUT PARMS:   getVal - Secondary voice DNS server address
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetDnsServerAddrSec(DAL_VOICE_PARMS *parms, char *getVal, unsigned int length );

#ifdef MGCPLOAD
/***************************************************************************
** Function Name: dalVoice_GetMgcpCallAgentIpAddress
** Description  : Provis wrapper for dalVoice_GetMgcpCallAgentIpAddress()
**                InternetGatewayDevice.Services.VoiceService.{i}.VoiceProfile.{i}.MGCP.callAgent1
** Parameters   : vpInst  - parms->op[0]
**              : callAgentIpAddress
** Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetMgcpCallAgentIpAddress(DAL_VOICE_PARMS *parms, char *callAgentIpAddress, unsigned int length );

/***************************************************************************
** Function Name: dalVoice_GetMgcpGatewayName
** Description  : Provis wrapper for dalVoice_GetMgcpGatewayName()
**                InternetGatewayDevice.Services.VoiceService.{i}.VoiceProfile.{i}.MGCP.domain
** Parameters   : vpInst  - parms->op[0]
** Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetMgcpGatewayName(DAL_VOICE_PARMS *parms, char *gatewayName, unsigned int length );

/***************************************************************************
** Function Name: dalVoice_SetMgcpCallAgentIpAddress
** Description  : Provis wrapper for dalVoice_SetMgcpCallAgentIpAddress()
**                InternetGatewayDevice.Services.VoiceService.{i}.VoiceProfile.{i}.MGCP.callAgent1
** Parameters   : vpInst  - parms->op[0]
**              : callAgentIpAddress
** Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetMgcpCallAgentIpAddress(DAL_VOICE_PARMS *parms, char *callAgentIpAddress );

/***************************************************************************
** Function Name: dalVoice_SetMgcpGatewayName
** Description  : Provis wrapper for dalVoice_SetMgcpGatewayName()
**                InternetGatewayDevice.Services.VoiceService.{i}.VoiceProfile.{i}.MGCP.domain
** Parameters   : vpInst  - parms->op[0]
**              : gatewayName
** Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetMgcpGatewayName(DAL_VOICE_PARMS *parms, char *gatewayName );
#endif /* MGCPLOAD */

#if defined( DMP_X_BROADCOM_COM_NTR_1 )
/***************************************************************************
* Function Name: dalVoice_SetNtrEnable
* Description  : Enable NTR mode to apply a feedback offset to the PCM feedback control registers
*                X_BROADCOM_COM_Ntr.Enable = new value
*
* Parameters   : parms->op[0] = vpInst, setVal = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetNtrEnable(DAL_VOICE_PARMS *parms, char *setVal );

/***************************************************************************
* Function Name: dalVoice_SetNtrAutoModeEnable
* Description  : set NTR mode to automatically calculate the most appropriate
*                feedback offset
*                X_BROADCOM_COM_Ntr.AutoModeEnable = new value
*
* Parameters   : parms->op[0] = vpInst, setVal = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetNtrAutoModeEnable(DAL_VOICE_PARMS *parms, char *setVal );

/***************************************************************************
* Function Name: dalVoice_SetNtrManualOffset
* Description  : set the user-specified manual offset for NTR to use with the
*                PCM highway feedback control registers
*                X_BROADCOM_COM_Ntr.ManualOffset = new value
*
* Parameters   : parms->op[0] = vpInst, setVal = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetNtrManualOffset(DAL_VOICE_PARMS *parms, char *setVal );

/***************************************************************************
* Function Name: dalVoice_SetNtrManualPpm
* Description  : set the automatically-calculated offset in PPM for NTR to use with the
*                PCM highway feedback control registers
*                X_BROADCOM_COM_Ntr.ManualPpm = new value
*
* Parameters   : parms->op[0] = vpInst, setVal = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetNtrManualPpm(DAL_VOICE_PARMS *parms, char *setVal );

/***************************************************************************
* Function Name: dalVoice_SetNtrManualSteps
* Description  : set the manual offset in PLL steps for NTR to use with the
*                PCM feedback control registers
*                X_BROADCOM_COM_Ntr.ManualSteps = new value
*
* Parameters   : parms->op[0] = vpInst, setVal = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetNtrManualSteps(DAL_VOICE_PARMS *parms, char *setVal );

/***************************************************************************
* Function Name: dalVoice_SetNtrDebugEnable
* Description  : set the debug information display settings
*                for the NTR task in automatic mode
*                X_BROADCOM_COM_Ntr.DebugEnable = new value
*
* Parameters   : parms->op[0] = vpInst, setVal = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetNtrDebugEnable(DAL_VOICE_PARMS *parms, char *setVal );

/***************************************************************************
* Function Name: dalVoice_SetNtrPcmMipsTallyCurrent
* Description  : set the current PCM-MIPS Tally
*                for the NTR task in automatic mode
*                X_BROADCOM_COM_Ntr.PcmMipsTallyCurrent = new value
*
* Parameters   : parms->op[0] = vpInst, setVal = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetNtrPcmMipsTallyCurrent(DAL_VOICE_PARMS *parms, char *setVal );

/***************************************************************************
* Function Name: dalVoice_SetNtrPcmMipsTallyPrevious
* Description  : set the previous PCM-MIPS Tally
*                for the NTR task in automatic mode
*                X_BROADCOM_COM_Ntr.PcmMipsTallyPrevious = new value
*
* Parameters   : parms->op[0] = vpInst, setVal = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetNtrPcmMipsTallyPrevious(DAL_VOICE_PARMS *parms, char *setVal );

/***************************************************************************
* Function Name: dalVoice_SetNtrPcmNtrTallyCurrent
* Description  : set the current PCM-NTR Tally
*                for the NTR task in automatic mode
*                X_BROADCOM_COM_Ntr.PcmNtrTallyCurrent = new value
*
* Parameters   : parms->op[0] = vpInst, setVal = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetNtrPcmNtrTallyCurrent(DAL_VOICE_PARMS *parms, char *setVal );

/***************************************************************************
* Function Name: dalVoice_SetNtrPcmNtrTallyPrevious
* Description  : set the previous PCM-NTR Tally
*                for the NTR task in automatic mode
*                X_BROADCOM_COM_Ntr.PcmNtrTallyPrevious = new value
*
* Parameters   : parms->op[0] = vpInst, setVal = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetNtrPcmNtrTallyPrevious(DAL_VOICE_PARMS *parms, char *setVal );

/***************************************************************************
* Function Name: dalVoice_SetNtrDslMipsTallyCurrent
* Description  : set the current DSL-MIPS Tally
*                for the NTR task in automatic mode
*                X_BROADCOM_COM_Ntr.DslMipsTallyCurrent = new value
*
* Parameters   : parms->op[0] = vpInst, setVal = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetNtrDslMipsTallyCurrent(DAL_VOICE_PARMS *parms, char *setVal );

/***************************************************************************
* Function Name: dalVoice_SetNtrDslMipsTallyPrevious
* Description  : set the previous DSL-MIPS Tally
*                for the NTR task in automatic mode
*                X_BROADCOM_COM_Ntr.DslMipsTallyPrevious = new value
*
* Parameters   : parms->op[0] = vpInst, setVal = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetNtrDslMipsTallyPrevious(DAL_VOICE_PARMS *parms, char *setVal );

/***************************************************************************
* Function Name: dalVoice_SetNtrDslNtrTallyCurrent
* Description  : set the current DSL-NTR Tally
*                for the NTR task in automatic mode
*                X_BROADCOM_COM_Ntr.DslNtrTallyCurrent = new value
*
* Parameters   : parms->op[0] = vpInst, setVal = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetNtrDslNtrTallyCurrent(DAL_VOICE_PARMS *parms, char *setVal );

/***************************************************************************
* Function Name: dalVoice_SetNtrDslNtrTallyPrevious
* Description  : set the previous DSL-NTR Tally
*                for the NTR task in automatic mode
*                X_BROADCOM_COM_Ntr.DslNtrTallyPrevious = new value
*
* Parameters   : parms->op[0] = vpInst, setVal = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetNtrDslNtrTallyPrevious(DAL_VOICE_PARMS *parms, char *setVal );

/***************************************************************************
* Function Name: dalVoice_SetNtrSampleRate
* Description  : set the NTR sampling rate in automatic mode
*                X_BROADCOM_COM_Ntr.SampleRate = new value
*
* Parameters   : parms->op[0] = vpInst, setVal = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetNtrSampleRate(DAL_VOICE_PARMS *parms, char *setVal );

/***************************************************************************
* Function Name: dalVoice_SetNtrPllBandwidth
* Description  : set the NTR PLL bandwidth in automatic mode
*                X_BROADCOM_COM_Ntr.PllBandwidth = new value
*
* Parameters   : parms->op[0] = vpInst, setVal = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetNtrPllBandwidth(DAL_VOICE_PARMS *parms, char *setVal );

/***************************************************************************
* Function Name: dalVoice_SetNtrDampingFactor
* Description  : set the NTR damping factor in automatic mode
*                X_BROADCOM_COM_Ntr.DampingFactor = new value
*
* Parameters   : parms->op[0] = vpInst, setVal = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetNtrDampingFactor(DAL_VOICE_PARMS *parms, char *setVal );

/***************************************************************************
* Function Name: dalVoice_SetNtrDefaults
* Description  : set NTR default values for sampling, bandwidth, damping.
*
* Parameters   : None ( parameters are ignored );
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetNtrDefaults( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetNtrAutoOffset
* Description  : set the most recent automatically-calculated offset in Hz for NTR to use with the
*                PCM highway feedback control registers without shifting history
*                X_BROADCOM_COM_Ntr.History.{i}.AutoOffset = new value
*
* Parameters   : parms->op[0] = vpInst, setVal = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetNtrAutoOffset(DAL_VOICE_PARMS *parms, char *setVal );

/***************************************************************************
* Function Name: dalVoice_SetNtrAutoPpmHistory
* Description  : set the most recent automatically-calculated offset in PPM for NTR to use with the
*                PCM highway feedback control registers without shifting history
*                X_BROADCOM_COM_Ntr.History.{i}.AutoPpm = new value
*
* Parameters   : parms->op[0] = vpInst, setVal = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetNtrAutoPpm(DAL_VOICE_PARMS *parms, char *setVal );

/***************************************************************************
* Function Name: dalVoice_SetNtrAutoStepsHistory
* Description  : set the most recent feedback offset applied to the PLL
*                in steps for the NTR task in automatic mode
*                X_BROADCOM_COM_Ntr.History.{i}.AutoSteps = new value
*
* Parameters   : parms->op[0] = vpInst, setVal = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetNtrAutoStepsHistory(DAL_VOICE_PARMS *parms, char *setVal );

/***************************************************************************
* Function Name: dalVoice_SetNtrAutoOffsetHistory
* Description  : set the most recent automatically-calculated offset in Hz for NTR to use with the
*                PCM highway feedback control registers
*                X_BROADCOM_COM_Ntr.History.{i}.AutoOffset = new value
*
* Parameters   : parms->op[0] = vpInst, setVal = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetNtrAutoOffsetHistory(DAL_VOICE_PARMS *parms, char *setVal );

/***************************************************************************
* Function Name: dalVoice_SetNtrAutoPpmHistory
* Description  : set the most recent automatically-calculated offset in PPM for NTR to use with the
*                PCM highway feedback control registers
*                X_BROADCOM_COM_Ntr.History.{i}.AutoPpm = new value
*
* Parameters   : parms->op[0] = vpInst, setVal = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetNtrAutoPpmHistory(DAL_VOICE_PARMS *parms, char *setVal );

/***************************************************************************
* Function Name: dalVoice_SetNtrAutoPhaseErrorHistory
* Description  : set the most recent automatically-calculated phase error from calculations
*                X_BROADCOM_COM_Ntr.History.{i}.AutoPhaseError = new value
*
* Parameters   : parms->op[0] = vpInst, setVal = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetNtrAutoPhaseErrorHistory(DAL_VOICE_PARMS *parms, char *setVal );

/***************************************************************************
* Function Name: dalVoice_SetNtrResetHistory
* Description  : Resets history parameters to 0
*                X_BROADCOM_COM_Ntr.History.{i}.AutoSteps = 0
*                X_BROADCOM_COM_Ntr.History.{i}.AutoOffset = 0
*                X_BROADCOM_COM_Ntr.History.{i}.AutoPpm = 0
*                X_BROADCOM_COM_Ntr.History.{i}.AutoPhaseError = 0
*
* Parameters   : parms->op[0] = vpInst
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetNtrResetHistory(DAL_VOICE_PARMS *parms, char *setVal );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNtrAutoModeEnable
**
**  PURPOSE:        Get NTR Auto Mode on (1) or off (0)
**
**  INPUT PARMS:    NTR instance  - parms->op[0]
**
**  OUTPUT PARMS:   enabled - NTR Automatic Offset Calculate Mode Enabled
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNtrAutoModeEnable(DAL_VOICE_PARMS *parms, char *enabled, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNtrManualOffset
**
**  PURPOSE:        Get Offset for NTR Manual Mode
**
**  INPUT PARMS:    NTR instance  - parms->op[0]
**
**  OUTPUT PARMS:   manualOffset - User-specified feedback offset in miliHertz [mHz]
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNtrManualOffset(DAL_VOICE_PARMS *parms, char *manualOffsetv, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNtrManualPpm
**
**  PURPOSE:        Get Offset in PPM for NTR Manual Mode
**
**  INPUT PARMS:    NTR instance  - parms->op[0]
**
**  OUTPUT PARMS:   manualPpm - Manually specified feedback offset in PPM
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNtrManualPpm(DAL_VOICE_PARMS *parms, char *manualPpm, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNtrManualSteps
**
**  PURPOSE:        Get feedback offset in PLL steps for NTR manual mode
**
**  INPUT PARMS:    NTR instance  - parms->op[0]
**
**  OUTPUT PARMS:   manualSteps - Manually specified feedback offset in PLL steps
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNtrManualSteps(DAL_VOICE_PARMS *parms, char *manualSteps, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNtrDebugEnable
**
**  PURPOSE:        Get NTR Debug flag
**
**  INPUT PARMS:    NTR instance  - parms->op[0]
**
**  OUTPUT PARMS:   enabled - NTR Debug Enabled
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNtrDebugEnable(DAL_VOICE_PARMS *parms, char *enabled, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNtrPcmMipsTallyCurrent
**
**  PURPOSE:        Get current PCM-MIPS tally for NTR task
**
**  INPUT PARMS:    NTR instance  - parms->op[0]
**
**  OUTPUT PARMS:   pcmMipsTallyCurrent - Tally
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNtrPcmMipsTallyCurrent(DAL_VOICE_PARMS *parms, char *pcmMipsTallyCurrent, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNtrPcmMipsTallyPrevious
**
**  PURPOSE:        Get previous PCM-MIPS tally for NTR task
**
**  INPUT PARMS:    NTR instance  - parms->op[0]
**
**  OUTPUT PARMS:   pcmMipsTallyPrevious - Tally
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNtrPcmMipsTallyPrevious(DAL_VOICE_PARMS *parms, char *pcmMipsTallyPrevious, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNtrPcmNtrTallyCurrent
**
**  PURPOSE:        Get current PCM-NTR tally for NTR task
**
**  INPUT PARMS:    NTR instance  - parms->op[0]
**
**  OUTPUT PARMS:   pcmNtrTallyCurrent - Tally
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNtrPcmNtrTallyCurrent(DAL_VOICE_PARMS *parms, char *pcmNtrTallyCurrent, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNtrPcmNtrTallyPrevious
**
**  PURPOSE:        Get previous PCM-NTR tally for NTR task
**
**  INPUT PARMS:    NTR instance  - parms->op[0]
**
**  OUTPUT PARMS:   pcmNtrTallyPrevious - Tally
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNtrPcmNtrTallyPrevious(DAL_VOICE_PARMS *parms, char *pcmNtrTallyPrevious, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNtrDslMipsTallyCurrent
**
**  PURPOSE:        Get current DSL-MIPS tally for NTR task
**
**  INPUT PARMS:    NTR instance  - parms->op[0]
**
**  OUTPUT PARMS:   dslMipsTallyCurrent - Tally
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNtrDslMipsTallyCurrent(DAL_VOICE_PARMS *parms, char *dslMipsTallyCurrent, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNtrDslMipsTallyPrevious
**
**  PURPOSE:        Get previous DSL-MIPS tally for NTR task
**
**  INPUT PARMS:    NTR instance  - parms->op[0]
**
**  OUTPUT PARMS:   dslMipsTallyPrevious - Tally
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNtrDslMipsTallyPrevious(DAL_VOICE_PARMS *parms, char *dslMipsTallyPrevious, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNtrDslNtrTallyCurrent
**
**  PURPOSE:        Get current DSL-NTR tally for NTR task
**
**  INPUT PARMS:    NTR instance  - parms->op[0]
**
**  OUTPUT PARMS:   dslNtrTallyCurrent - Tally
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNtrDslNtrTallyCurrent(DAL_VOICE_PARMS *parms, char *dslNtrTallyCurrent, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNtrDslNtrTallyPrevious
**
**  PURPOSE:        Get previous DSL-NTR tally for NTR task
**
**  INPUT PARMS:    NTR instance  - parms->op[0]
**
**  OUTPUT PARMS:   dslNtrTallyPrevious - Tally
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNtrDslNtrTallyPrevious(DAL_VOICE_PARMS *parms, char *dslNtrTallyPrevious, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNtrSampleRate
**
**  PURPOSE:        Get previous SampleRate for NTR task
**
**  INPUT PARMS:    NTR instance  - parms->op[0]
**
**  OUTPUT PARMS:   sampleRate
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNtrSampleRate(DAL_VOICE_PARMS *parms, char *sampleRate, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNtrPllBandwidth
**
**  PURPOSE:        Get previous PllBandwidth for NTR task
**
**  INPUT PARMS:    NTR instance  - parms->op[0]
**
**  OUTPUT PARMS:   pllBandwidth
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNtrPllBandwidth(DAL_VOICE_PARMS *parms, char *pllBandwidth, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNtrDampingFactor
**
**  PURPOSE:        Get previous DampingFactor for NTR task
**
**  INPUT PARMS:    NTR instance  - parms->op[0]
**
**  OUTPUT PARMS:   dampingFactor
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNtrDampingFactor(DAL_VOICE_PARMS *parms, char *dampingFactor, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNtrAutoOffset
**
**  PURPOSE:        Get the most recent feedback offsets applied in Hz in NTR automatic mode separated by ','
**
**  INPUT PARMS:    NTR instance  - parms->op[0]
**
**  OUTPUT PARMS:   autoOffset - Most recent automatically-calculated feedback offset in Hz
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNtrAutoOffset(DAL_VOICE_PARMS *parms, char *autoOffset, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNtrAutoPpm
**
**  PURPOSE:        Get the most recent feedback offsets applied in PPM in NTR automatic mode separated by ','
**
**  INPUT PARMS:    NTR instance  - parms->op[0]
**
**  OUTPUT PARMS:   autoPpm - Most recent automatically-calculated feedback offset in PPM
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNtrAutoPpm(DAL_VOICE_PARMS *parms, char *autoPpm, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNtrAutoStepsHistory
**
**  PURPOSE:        Get the history of feedback offsets applied in PLL steps in NTR automatic mode separated by ','
**
**  INPUT PARMS:    NTR instance  - parms->op[0]
**
**  OUTPUT PARMS:   autoSteps - History of feedback offset applied in PLL steps
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNtrAutoStepsHistory(DAL_VOICE_PARMS *parms, char *autoSteps, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNtrAutoOffsetHistory
**
**  PURPOSE:        Get the history of feedback offsets applied in Hz in NTR automatic mode separated by ','
**
**  INPUT PARMS:    NTR instance  - parms->op[0]
**
**  OUTPUT PARMS:   autoOffset - History of automatically-calculated feedback offset in Hz
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNtrAutoOffsetHistory(DAL_VOICE_PARMS *parms, char *autoOffset, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNtrAutoPpmHistory
**
**  PURPOSE:        Get the history of feedback offsets applied in PPM in NTR automatic mode separated by ','
**
**  INPUT PARMS:    NTR instance  - parms->op[0]
**
**  OUTPUT PARMS:   autoPpm - History of automatically-calculated feedback offset in PPM
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNtrAutoPpmHistory(DAL_VOICE_PARMS *parms, char *autoPpm, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNtrAutoPhaseErrorHistory
**
**  PURPOSE:        Get the history of automatically calculated phase error separated by ','
**
**  INPUT PARMS:    NTR instance  - parms->op[0]
**
**  OUTPUT PARMS:   autoPpm - History of automatically-calculated phase error
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNtrAutoPhaseErrorHistory(DAL_VOICE_PARMS *parms, char *autoPhaseError, unsigned int length );
#endif /* DMP_X_BROADCOM_COM_NTR_1 */

/* GetNtrEnable is used by the GUI even if NTR support is off */
/*****************************************************************
**  FUNCTION:       dalVoice_GetNtrEnable
**
**  PURPOSE:        Get NTR Enable flag
**
**  INPUT PARMS:    NTR instance  - parms->op[0]
**
**  OUTPUT PARMS:   enabled - NTR Enabled
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNtrEnable(DAL_VOICE_PARMS *parms, char *enabled, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNumSrvProv
**
**  PURPOSE:        Returns total no. of service providers configured
**                  ( i.e corresponds to total no. of Voice profiles)
**
**  INPUT PARMS:    None;
**
**  OUTPUT PARMS:   Number of service providers ( voice profiles ) configured
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNumSrvProv( int * numSp );

CmsRet dalVoice_GetNumVoiceProfiles( DAL_VOICE_PARMS *parms, char *value, unsigned int length );
/*****************************************************************
**  FUNCTION:       dalVoice_GetNumAccPerSrvProv
**
**  PURPOSE:        returns total accounts per specific serviceprovider
**                  ( i.e. corresponds to number of lines per specific voice profile )
**
**  INPUT PARMS:    srvProvNum - Index of service provider;
**
**  OUTPUT PARMS:   Number of accounts per this service provider (num vplines per vp)
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNumAccPerSrvProv( int srvProvNum, int * numAcc );
/*****************************************************************
**  FUNCTION:       dalVoice_GetNumPhysFxsEndpt
**
**  PURPOSE:        returns total number of physical fxs endpoints in system
**
**  INPUT PARMS:    none;
**
**  OUTPUT PARMS:
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNumPhysFxsEndpt( int * numPhysFxs );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNumPhysFxsEndptStr
**
**  PURPOSE:        returns total number of physical fxs endpoints in system
**
**  INPUT PARMS:    none;
**
**  OUTPUT PARMS:
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNumPhysFxsEndptStr( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNumPhysFxoEndpt
**
**  PURPOSE:        returns total number of physical fxo endpoints in system
**
**  INPUT PARMS:    none;
**
**  OUTPUT PARMS:
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNumPhysFxoEndpt( int * numPhysFxo );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNumPhysFxoEndptStr
**
**  PURPOSE:        returns total number of physical fxo endpoints in system
**
**  INPUT PARMS:    none;
**
**  OUTPUT PARMS:
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNumPhysFxoEndptStr( DAL_VOICE_PARMS *parms, char* value, unsigned int length );



CmsRet dalVoice_GetLineSettingId( DAL_VOICE_PARMS *parms, UINT32 *lineId, unsigned int length );
CmsRet dalVoice_GetLineSettingName( DAL_VOICE_PARMS *parms, char *lineName, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetVoicePhyInterfaceList
**
**  PURPOSE:
**
**  INPUT PARMS:    parms->op[0] - voice service instance
**
**  OUTPUT PARMS:   list  - list of physical endpoint ID, seperated by ','
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVoicePhyInterfaceList(DAL_VOICE_PARMS *parms, char *list, unsigned int length );

CmsRet dalVoice_GetVlAssociatedNonFxsPhyType(DAL_VOICE_PARMS *parms, char *type, unsigned int length );
CmsRet dalVoice_GetTotalNumLines( int * numTotLines );
CmsRet dalVoice_GetNumPhysEndpts( int * numPhysEndpts );
CmsRet dalVoice_GetMaxLinesPerVoiceProfile( DAL_VOICE_PARMS *parms, char* value, unsigned int length );
CmsRet dalVoice_GetNumLinesPerVoiceProfile( DAL_VOICE_PARMS *parms, char* value, unsigned int length );
CmsRet dalVoice_GetMaxVoiceProfiles( DAL_VOICE_PARMS *parms, char* value, unsigned int length );
CmsRet dalVoice_GetSuppCodecsString( DAL_VOICE_PARMS *parms, char* value, unsigned int length );
CmsRet dalVoice_GetSuppCodecBitRate( DAL_VOICE_PARMS *parms, int* rate );
CmsRet dalVoice_GetSuppCodecPacketizationPeriod( DAL_VOICE_PARMS *parms, char* value, unsigned int length );
CmsRet dalVoice_GetSuppCodecSilSupp( DAL_VOICE_PARMS *parms, char* value, unsigned int length );
CmsRet dalVoice_GetMaxSuppCodecs( DAL_VOICE_PARMS *parms, char* value, unsigned int length );


/*****************************************************************
**  FUNCTION:       dalVoice_GetCodecProfCodec
**
**  PURPOSE:        Returns the name of the codec in codec profile
**
**  INPUT PARMS:    length - size of string buffer
**
**  OUTPUT PARMS:   value - codec name
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetCodecProfCodec( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetCodecProfPacketPeriod
**
**  PURPOSE:        Returns the packetization period of the codec in codec profile
**
**  INPUT PARMS:    length - size of string buffer
**
**  OUTPUT PARMS:   value - codec packetization period(s), comma separated
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetCodecProfPacketPeriod( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetCodecProfSilSupp
**
**  PURPOSE:        Returns the packetization period of the codec in codec profile
**
**  INPUT PARMS:    length - size of string buffer
**
**  OUTPUT PARMS:   value - silence suppression, true/false
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetCodecProfSilSupp( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

CmsRet dalVoice_SetVlFxoPhyReferenceList( DAL_VOICE_PARMS *parms, char *value );
CmsRet dalVoice_GetNumFxoEndpt( DAL_VOICE_PARMS *parms, char* value, unsigned int length );
CmsRet dalVoice_voiceStart(void* msgHandleArg);
CmsRet dalVoice_voiceStop(void* msgHandleArg);
CmsRet dalVoice_voiceReboot(void* msgHandleArg);
CmsRet dalVoice_GetStatus(DAL_VOICE_PARMS * parms, char* value, unsigned int length);


/***************************************************************************
* Function Name: dalVoice_GetNumVoipProfile
* Description  : Get total number of VoIP profile in the system
*
* Parameters   : parms->op[0] = voice service instance, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetNumVoipProfile( DAL_VOICE_PARMS *parms, int *value );

/****************************************************************************
* FUNCTION:    dalVoice_GetVoipProfileList
*
* PURPOSE:     Get list of available VOIP profiles
*
* PARAMETERS:  parms - voice service parameters to use for queries
*              profList - placeholder for the VOIP profile list
*
* RETURNS:     CMSRET_SUCCESS when successful
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetVoipProfileList( DAL_VOICE_PARMS *parms, char* profList, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetNumCodecProfile
* Description  : Get total number of Codec profile in the system
*
* Parameters   : parms->op[0] = voice service instance, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetNumCodecProfile( DAL_VOICE_PARMS *parms, int *value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCodecProfileList
*
* PURPOSE:     Get list of available codec profiles
*
* PARAMETERS:  parms - voice service parameters to use for queries
*              profList - placeholder for the codec profile list
*
* RETURNS:     CMSRET_SUCCESS when successful
*
* NOTE:
*
****************************************************************************/
CmsRet dalVoice_GetCodecProfileList( DAL_VOICE_PARMS *parms, char* profList, unsigned int length );

#ifdef DMP_X_BROADCOM_COM_DECTENDPOINT_1
/***************************************************************************
* Function Name: dalVoice_dectCtlRegWnd
* Description  : Controls (Open or closes) the registration window
*                for DECT support.
*
* Parameters   : open - 1 to open the DECT registration window, 0 to close
*                       the DECT registration window.
*                msgHandleArg - the calling application's message handle
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_dectCtlRegWnd( unsigned int open, void* msgHandleArg );

/***************************************************************************
* Function Name: dalVoice_dectCtlDelHset
* Description  : Deletes a registered handset from the base station.
*
* Parameters   : hset - the handset index to delete.
*                msgHandleArg - the calling application's message handle
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_dectCtlDelHset( unsigned int hset, void* msgHandleArg );

/***************************************************************************
* Function Name: dalVoice_dectCtlPingHset
* Description  : Pings a registered handset (i.e. apply short ring on the
*                handset to locate it).
*
* Parameters   : hset - the handset index to ping.
*                msgHandleArg - the calling application's message handle
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_dectCtlPingHset( unsigned int hset, void* msgHandleArg );

CmsRet dalVoice_SetDectEepromData( DAL_VOICE_PARMS *parms, char * setVal );
CmsRet dalVoice_GetDectEepromData( DAL_VOICE_PARMS *parms, char * getVal, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetDectRegWnd
* Description  : Gets information from the DECT module about registration
*                window setup.
*
* Parameters   : parms
*                value - returned value information
*                length - size of the returned information
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectRegWnd(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_SetDectRegWnd
* Description  : Sets information from the DECT module about registration
*                window setup.
*
* Parameters   : parms
*                value - value to set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetDectRegWnd(DAL_VOICE_PARMS *parms, unsigned int value);

/***************************************************************************
* Function Name: dalVoice_GetDectStatus
* Description  : Gets information from the DECT module about whether the
*                module was initialized properly or not.
*
* Parameters   : parms
*                value - returned value information
*                length - size of the returned information
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectStatus(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_SetDectStatus
* Description  : Sets information from the DECT module about whether the
*                module was initialized properly or not.
*
* Parameters   : parms
*                value - value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetDectStatus(DAL_VOICE_PARMS *parms, unsigned value);

/***************************************************************************
* Function Name: dalVoice_GetDectAc
* Description  : Gets information from the DECT module the access code used
*                for handset registration.
*
* Parameters   : parms
*                value - returned value information
*                length - size of the returned information
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectAc(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_SetDectAc
* Description  : Sets information to the DECT module the access code used
*                for handset registration.
*
*                This function is used to send a command to the DECT
*                module in order to set (update) the access code that is
*                known there.  Once we get confirmation from the DECT
*                module that the code has been updated successfully, the
*                DECT support framework will update the information in
*                MDM flash as will if appropriate.
*
* Parameters   : parms
*                value - value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetDectAc(DAL_VOICE_PARMS *parms, char *value);

/***************************************************************************
* Function Name: dalVoice_SetDectAcPersist
* Description  : Sets the DECT access code information into persistent
*                storage (the MDM).
*
*                This function is used by the DECT support framework to
*                save information about the access code to flash once we
*                received confirmation from the DECT module that attempt to
*                update the access code has been successful.
*
* Parameters   : parms
*                value - value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetDectAcPersist(DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_GetDectFwVer
* Description  : Gets information from the DECT module about the firmware
*                version being used.
*
* Parameters   : parms
*                value - returned value information
*                length - size of the returned information
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectFwVer(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetDectInfo
* Description  : Gets information from the DECT module 
*
* Parameters   : parms
*                value - returned value information
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectInfo(DAL_VOICE_PARMS *parms, DAL_VOICE_DECT_INFO *value );

/***************************************************************************
* Function Name: dalVoice_GetDectLnk
* Description  : Gets information from the DECT module about the firmware
*                link date being used.
*
* Parameters   : parms
*                value - returned value information
*                length - size of the returned information
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectLnk(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_SetDectLnk
* Description  : Sets information from the DECT module about the firmware
*                link date being used.
*
* Parameters   : parms
*                value - value to set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetDectLnk(DAL_VOICE_PARMS *parms, char *value);

/***************************************************************************
* Function Name: dalVoice_GetDectType
* Description  : Gets information from the DECT module about the firmware
*                type being used.
*
* Parameters   : parms
*                value - returned value information
*                length - size of the returned information
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectType(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_SetDectType
* Description  : Sets information from the DECT module about the firmware
*                type being used.
*
* Parameters   : parms
*                value - value to set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetDectType(DAL_VOICE_PARMS *parms, unsigned int value);

/***************************************************************************
* Function Name: dalVoice_GetDectId
* Description  : Gets information from the DECT module about the unique
*                identifier.
*
* Parameters   : parms
*                value - returned value information
*                length - size of the returned information
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectId(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_SetDectId
* Description  : Sets information from the DECT module about the unique
*                identifier.
*
* Parameters   : parms
*                value - value to set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetDectId(DAL_VOICE_PARMS *parms, char *value);

/***************************************************************************
* Function Name: dalVoice_GetDectManic
* Description  : Gets information from the DECT module about the
*                manufacturer identifier of the DECT device.
*
* Parameters   : parms
*                value - returned value information
*                length - size of the returned information
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectManic(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_SetDectManic
* Description  : Sets information from the DECT module about the
*                manufacturer identifier of the DECT device.
*
* Parameters   : parms
*                value - returned value information
*                length - size of the returned information
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetDectManic(DAL_VOICE_PARMS *parms, unsigned int value);

/***************************************************************************
* Function Name: dalVoice_GetDectModic
* Description  : Gets information from the DECT module about the modele
*                identifier of the DECT module.
*
* Parameters   : parms
*                value - returned value information
*                length - size of the returned information
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectModic(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_SetDectModic
* Description  : Sets information from the DECT module about the
*                model identifier of the DECT device.
*
* Parameters   : parms
*                value - returned value information
*                length - size of the returned information
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetDectModic(DAL_VOICE_PARMS *parms, unsigned int value);

/***************************************************************************
* Function Name: dalVoice_GetDectCurHset
* Description  : Gets the current number of handset registered with the
*                DECT module.
*
* Parameters   : parms
*                value - returned value information
*                length - size of the returned information
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectCurHset(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetDectHsetInstanceList
* Description  : Gets the registered handset instances list in the MDM
*
* Parameters   : parms
*                total - returned total number of instance in MDM
*                list  - list of registered handset instance
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectHsetInstanceList(DAL_VOICE_PARMS *parms, unsigned int *total, unsigned int *list );

/***************************************************************************
* Function Name: dalVoice_GetDectCurHsetList
* Description  : Gets the current number of handset registered with the
*                DECT module.
*
* Parameters   : parms
*                value - returned value information
*                list  - list of registered handset id
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectCurHsetList(DAL_VOICE_PARMS *parms, unsigned int *total, unsigned int *list );

/***************************************************************************
* Function Name: dalVoice_GetDectMaxHset
* Description  : Gets the maximum number of handset to be registered with the
*                DECT module.
*
* Parameters   : parms
*                value - returned value information
*                length - size of the returned information
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectMaxHset(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetDectHsStatus
* Description  : Gets handset status for the handset associated with the
*                DECT module.
*
* Parameters   : parms
*                value - returned value information
*                length - size of the returned information
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectHsStatus(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetDectHsManic
* Description  : Gets information from the DECT module about the
*                manufacturer identifier of the handset registered.
*
* Parameters   : parms
*                value - returned value information
*                length - size of the returned information
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectHsManic(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetDectHsModic
* Description  : Gets information from the DECT module about the modele
*                identifier of the handset registered.
*
* Parameters   : parms
*                value - returned value information
*                length - size of the returned information
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectHsModic(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetDectHsIpei
* Description  : Gets information from the DECT module about the IPEI
*                of the handset registered.
*
* Parameters   : parms
*                value - returned value information
*                length - size of the returned information
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectHsIpei(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_SetDectHsIpuiPersist
* Description  : Sets the DECT handset IPUI information into persistent
*                storage (the MDM).
*
*                This function is used by the DECT support framework to
*                save information about the handset IPUI to flash once we
*                received confirmation from the DECT module about the IPUI
*                of the registered handset(s).
*
* Parameters   : parms
*                value - value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetDectHsIpuiPersist(DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_GetDectHsSubTime
* Description  : Gets information from the DECT module about the subscription
*                time of the handset registered.
*
* Parameters   : parms
*                value - returned value information
*                length - size of the returned information
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectHsSubTime(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetDectHsId
* Description  : Gets information from the DECT module about the
*                internal identifier of the handset registered.
*
* Parameters   : parms
*                value - returned value information
*                length - size of the returned information
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectHsId(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetDectHsetName
* Description  : Gets handset name ( internal name )
*
* Parameters   : 
*                (INPUT) parms->op[0] - voice service instance
*                parms.op[1] -  handset Id
*                value - name string
*                length - size of buffer
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectHsetName(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetCallInterception
* Description  : Gets information from the DECT module about call interception
*                support on this handset.
*
* Parameters   : parms
*                value - returned value information
*                length - size of the returned information
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectHsCallInterception(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetMaxHset
* Description  : Gets the maximum number of handset that the MDM software
*                allow the DECT module to support, note this is different
*                from the dalVoice_GetDectMaxHset API which is used to
*                query from the DECT module directly how many handset can
*                be supported at once.  Ideally those two values should be
*                in synch.
*
* Parameters   : parms
*                value - returned value information
*                length - size of the returned information
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetMaxHset( DAL_VOICE_PARMS *parms, int *value );

/*****************************************************************
**  FUNCTION:       dalVoice_GetDectRegisteredHsList
**
**  PURPOSE:
**
**  INPUT PARMS:    parms->op[0] - voice service instance
**
**  OUTPUT PARMS:   list - list of registered handset id, seperated by ','
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet  dalVoice_GetDectRegisteredHsList(DAL_VOICE_PARMS *parms, char *list, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetDectLoggingLevel
**
**  PURPOSE:
**
**  INPUT PARMS:    None
**
**  OUTPUT PARMS:   getVal - mdm log level for voice
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetDectLoggingLevel( DAL_VOICE_PARMS *parms, void * getVal, unsigned int length);

/*****************************************************************
**  FUNCTION:       dalVoice_GetLineIntrusionCallMode
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst  - parms->op[0]
**                  lineInst  - parms->op[1]
**
**  OUTPUT PARMS:   mode - Dect Line intrustion call mode, "enable" or "disable"
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet  dalVoice_GetLineIntrusionCallMode(DAL_VOICE_PARMS *parms, char *mode, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetLineMultiCallMode
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst  - parms->op[0]
**                  lineInst  - parms->op[1]
**
**  OUTPUT PARMS:   mode - Dect Line call mode, "single" or "mutliple"
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet  dalVoice_GetLineMultiCallMode(DAL_VOICE_PARMS *parms, char *mode, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetLineMelody
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst  - parms->op[0]
**                  lineInst  - parms->op[1]
**
**  OUTPUT PARMS:   melody - Dect Handset Melody
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet  dalVoice_GetLineMelody(DAL_VOICE_PARMS *parms, char *melody, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetDectResetBase
*
* Description  : Gets Dect Reset base value from MDM
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (OUTPUT) value - returned value information
*                (INPUT)  length - size of the returned information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectResetBase(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetDectPinCode
*
* Description  : Gets Pin code from MDM
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (OUTPUT) value - returned value information
*                (INPUT)  length - size of the returned information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectPinCode(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetDectHwVersion
*
* Description  : Gets Dect Hardware version from MDM
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (OUTPUT) value - returned value information
*                (INPUT)  length - size of the returned information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectHwVersion(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetDectFwVersion
*
* Description  : Gets Dect Firmware version from MDM
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (OUTPUT) value - returned value information
*                (INPUT)  length - size of the returned information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectFwVersion(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetDectRomVersion
*
* Description  : Gets Dect EEPROM version from MDM
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (OUTPUT) value - returned value information
*                (INPUT)  length - size of the returned information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectRomVersion(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetDectClockMaster
*
* Description  : Gets Dect clock master value from MDM
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (OUTPUT) value - returned value information
*                (INPUT)  length - size of the returned information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectClockMaster(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetDectIpAddrType
*
* Description  : Dummy function
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (OUTPUT) value - returned value information
*                (INPUT)  length - size of the returned information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectIpAddrType(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetDectIpAddress
*
* Description  : Dummy function
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (OUTPUT) value - returned value information
*                (INPUT)  length - size of the returned information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectIpAddress(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetDectIpSubnetMask
*
* Description  : Dummy function
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (OUTPUT) value - returned value information
*                (INPUT)  length - size of the returned information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectIpSubnetMask(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetDectIpGateway
*
* Description  : Dummy function
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (OUTPUT) value - returned value information
*                (INPUT)  length - size of the returned information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectIpGateway(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetDectIpDns
*
* Description  : Dummy function
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (OUTPUT) value - returned value information
*                (INPUT)  length - size of the returned information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectIpDns(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetDectEmissionMode
*
* Description  : Dummy function
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (OUTPUT) value - returned value information
*                (INPUT)  length - size of the returned information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectEmissionMode(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetDectNewPinCode
*
* Description  : Dummy function
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (OUTPUT) value - returned value information
*                (INPUT)  length - size of the returned information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectNewPinCode(DAL_VOICE_PARMS *parms, char *value, unsigned int length );


/*****************************************************************
**  FUNCTION:       dalVoice_GetLineAttachedHandsetList
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst  - parms->op[0]
**                  lineInst  - parms->op[1]
**
**  OUTPUT PARMS:   list - list of Dect Handset Id, seperate by ','
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet  dalVoice_GetLineAttachedHandsetList(DAL_VOICE_PARMS *parms, char *list, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetTotalNumberOfAttachedHs
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst  - parms->op[0]
**                  lineInst  - parms->op[1]
**
**  OUTPUT PARMS:   num - total number of attached handset
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet  dalVoice_GetTotalNumberOfAttachedHs(DAL_VOICE_PARMS *parms, char *num, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetDectRegisteredHsList
**
**  PURPOSE:
**
**  INPUT PARMS:    parms->op[0] - voice service instance
**
**  OUTPUT PARMS:   list - list of registered handset id, seperated by ','
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet  dalVoice_GetDectRegisteredHsList(DAL_VOICE_PARMS *parms, char *list, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetDectLineId
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst  - parms->op[0]
**                  lineInst  - parms->op[1]
**
**  OUTPUT PARMS:   line - Dect Line Id
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet  dalVoice_GetDectLineId(DAL_VOICE_PARMS *parms, char *line, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetLineName
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst  - parms->op[0]
**                  lineInst  - parms->op[1]
**
**  OUTPUT PARMS:   name - Dect Line name
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet  dalVoice_GetLineName(DAL_VOICE_PARMS *parms, char *Name, unsigned int length );



/*****************************************************************
**  FUNCTION:       dalVoice_SetDectLoggingLevel
**
**  PURPOSE:
**
**  INPUT PARMS:    setVal - log mdm log level for dectd
**
**  OUTPUT PARMS:   none
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetDectLoggingLevel(  DAL_VOICE_PARMS *parms, char * setVal );

/*****************************************************************
**  FUNCTION:       dalVoice_SetLineAttachedHandsetList
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst  - parms->op[0]
**                  lineInst  - parms->op[1]
**                  list - Dect handset list, like "1,3,6,7"
**  OUTPUT PARMS:   none
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet  dalVoice_SetLineAttachedHandsetList(DAL_VOICE_PARMS *parms, char *list);

/*****************************************************************
**  FUNCTION:       dalVoice_SetLineIntrusionCall
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst   - parms->op[0]
**                  lineInst - parms->op[1]
**                  mode     - Dect Line intrustion call mode, "enable" or "disable"
**
**  OUTPUT PARMS:   none
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet  dalVoice_SetLineIntrusionCall(DAL_VOICE_PARMS *parms, char *mode );

/*****************************************************************
**  FUNCTION:       dalVoice_SetLineMultiCallMode
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst   - parms->op[0]
**                  lineInst - parms->op[1]
**                  mode     - Dect Line call mode, "single" or "mutliple"
**
**  OUTPUT PARMS:   none
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet  dalVoice_SetLineMultiCallMode(DAL_VOICE_PARMS *parms, char *mode );

/*****************************************************************
**  FUNCTION:       dalVoice_SetDectLineMelody
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst   - parms->op[0]
**                  lineInst - parms->op[1]
**                  melody   - Dect Handset Melody
**
**  OUTPUT PARMS:   none
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet  dalVoice_SetLineMelody(DAL_VOICE_PARMS *parms, char *melody );

/*****************************************************************
**  FUNCTION:       dalVoice_SetDectLineId
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst  - parms->op[0]
**                  lineInst  - parms->op[1]
**                  line     - line ID
**
**  OUTPUT PARMS:   none
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet  dalVoice_SetDectLineId(DAL_VOICE_PARMS *parms, char *line );

/*****************************************************************
**  FUNCTION:       dalVoice_SetDectLineName
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst  - parms->op[0]
**                  lineInst  - parms->op[1]
**                  name - Dect Line name
**
**  OUTPUT PARMS:   none
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet  dalVoice_SetDectLineName(DAL_VOICE_PARMS *parms, char *Name );


CmsRet dalVoice_mapHsetId2Instance(int handsetId, int *inst);

/***************************************************************************
* Function Name: dalVoice_DelAllHandset
* Description  : delete all registered handset instance in MDM
*
* Parameters   : none
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_DelAllHandset( void );

/***************************************************************************
* Function Name: dalVoice_DelHandset
* Description  : delete one registered handset instance in MDM
*
* Parameters   : (INPUT) handsetId - handset Id to be deleted
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_DelHandset(int handsetId);

/***************************************************************************
* Function Name: dalVoice_AddHandset
* Description  : add one registered handset instance in MDM
*
* Parameters   : (INPUT) handsetId - handset Id to be added
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_AddHandset(int handsetId);

/***************************************************************************
* Function Name: dalVoice_GetDectHandsetName
*
* Description  : Gets internal name from MDM
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (INPUT)  parms->op[1] - handset instance
*                (OUTPUT) value - returned value information
*                (INPUT)  length - size of the returned information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectHandsetName(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetDectHandsetNumber
*
* Description  : Gets handset id from MDM
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (INPUT)  parms->op[1] - handset instance
*                (OUTPUT) value - returned value information
*                (INPUT)  length - size of the buffer
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectHandsetNumber(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetTotalVoiceCalls
*
* Description  : Gets total number of voice calls from MDM
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (OUTPUT) value - total number of instances
*                (OUTPUT) value - returned value information
*                (INPUT)  length - size of the returned information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetTotalVoiceCalls(DAL_VOICE_PARMS *parms, unsigned int *value, unsigned int *list);

/***************************************************************************
* Function Name: dalVoice_GetTotalVoiceCallsWithType
*
* Description  : Gets total number of missed voice calls from MDM
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (OUTPUT) value - total number of instances
*                (INPUT)  type -  voice call type
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetTotalVoiceCallsWithType(DAL_VOICE_PARMS *parms, unsigned int *value, char *type, unsigned int *list);

/***************************************************************************
* Function Name: dalVoice_GetTotalMissedVoiceCalls
*
* Description  : Gets total number of missed voice calls from MDM
*
* Parameters   : (OUTPUT) value - total number of instances
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetTotalMissedVoiceCalls(DAL_VOICE_PARMS *parms, unsigned int *value, unsigned int *list);

/***************************************************************************
* Function Name: dalVoice_GetTotalIncomingVoiceCalls
*
* Description  : Gets total number of incoming voice calls from MDM
*
* Parameters   : (OUTPUT) value - total number of instances
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetTotalIncomingVoiceCalls(DAL_VOICE_PARMS *parms, unsigned int *value, unsigned int *list);

/***************************************************************************
* Function Name: dalVoice_GetTotalVoiceCallsWithType
*
* Description  : Gets total number of outgoing voice calls from MDM
*
* Parameters   : (OUTPUT) value - total number of instances
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetTotalOutgoingVoiceCalls(DAL_VOICE_PARMS *parms, unsigned int *value, unsigned int *list);

/***************************************************************************
* Function Name: dalVoice_GetTotalAllIncomingVoiceCalls
*
* Description  : Gets total number of all incoming voice calls from MDM
*
* Parameters   : (OUTPUT) value - total number of instances
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetTotalAllIncomingVoiceCalls(DAL_VOICE_PARMS *parms, unsigned int *value, unsigned int *list);

/***************************************************************************
* Function Name: dalVoice_GetVoiceCallListStats
*
* Description  : Gets total numbers of each voice call type from MDM
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (OUTPUT) stats - stats block to fill in
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetVoiceCallListStats(DAL_VOICE_PARMS *parms, DAL_VOICE_DECT_CALL_LIST_STATS *stats);

/***************************************************************************
* Function Name: dalVoice_GetDectContactListNumber
*
* Description  : Gets Dect contact number from MDM
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (INPUT)  parms->op[1] - contact list instance
*                (OUTPUT) value - returned value information
*                (INPUT)  length - size of the returned information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectContactListNumber(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetDectContactListFirstName
*
* Description  : Gets Dect Reset base value from MDM
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (INPUT)  parms->op[1] - contact list instance
*                (OUTPUT) value - returned value information
*                (INPUT)  length - size of the returned information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectContactListFirstName(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetDectContactListMelody
*
* Description  : Gets Dect Reset base value from MDM
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (INPUT)  parms->op[1] - contact list instance
*                (OUTPUT) value - returned value information
*                (INPUT)  length - size of the returned information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectContactListMelody(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetDectContactListLineId
*
* Description  : Gets Dect Reset base value from MDM
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (INPUT)  parms->op[1] - contact list instance
*                (OUTPUT) value - returned value information
*                (INPUT)  length - size of the returned information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectContactListLineId(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetVoiceCallNumber
*
* Description  : Gets voice call number from call list
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (INPUT)  parms->op[1] - call list instance
*                (OUTPUT) value - returned value information
*                (INPUT)  length - size of the returned information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetVoiceCallNumber(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetVoiceCallName
*
* Description  : Gets calling party name from call list
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (INPUT)  parms->op[1] - call list instance
*                (OUTPUT) value - returned value information
*                (INPUT)  length - size of the returned information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetVoiceCallName(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetVoiceCallLineName
*
* Description  : Gets line name from call list
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (INPUT)  parms->op[1] - call list instance
*                (OUTPUT) value - returned value information
*                (INPUT)  length - size of the returned information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetVoiceCallLineName(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetVoiceCallNewFlag
*
* Description  : Gets line Id from call list
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (INPUT)  parms->op[1] - call list instance
*                (OUTPUT) value - returned value information
*                (INPUT)  length - size of the returned information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetVoiceCallNewFlag(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetVoiceCallNewFlag
*
* Description  : Gets line Id from call list
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (INPUT)  parms->op[1] - call list instance
*                (OUTPUT) value - returned value information
*                (INPUT)  length - size of the returned information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetVoiceCallNewFlag(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetVoiceCallDateTime
*
* Description  : Gets call date and time from call list
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (INPUT)  parms->op[1] - call list instance
*                (OUTPUT) value - returned value information
*                (INPUT)  length - size of the returned information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetVoiceCallDateTime(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetVoiceCallType
*
* Description  : Gets call type from call list
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (INPUT)  parms->op[1] - call list instance
*                (OUTPUT) value - returned value information
*                (INPUT)  length - size of the returned information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetVoiceCallType(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetVoiceMissedCalls
*
* Description  : Gets call type from call list
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (INPUT)  parms->op[1] - call list instance
*                (OUTPUT) value - returned value information
*                (INPUT)  length - size of the returned information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetVoiceMissedCalls(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetVoiceNumberOfMissedCalls
*
* Description  : Gets call type from call list
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (INPUT)  parms->op[1] - call list instance
*                (OUTPUT) value - returned value information
*                (INPUT)  length - size of the returned information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetVoiceNumberOfMissedCalls(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetDectTotalInternalName
*
* Description  : Gets total number of internal name from MDM
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (OUTPUT) value - total number of instances
*                (OUTPUT) value - returned value information
*                (INPUT)  length - size of the returned information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectTotalInternalName(DAL_VOICE_PARMS *parms, unsigned int *value, unsigned int *list);

/***************************************************************************
* Function Name: dalVoice_GetDectTotalSystemSettings
*
* Description  : Gets total number of system settings from MDM
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (OUTPUT) value - total number of instances
*                (OUTPUT) value - returned value information
*                (INPUT)  length - size of the returned information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectTotalSystemSettings(DAL_VOICE_PARMS *parms, unsigned int *value, unsigned int *list);

/***************************************************************************
* Function Name: dalVoice_GetDectTotalLineSettings
*
* Description  : Gets total number of system settings from MDM
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (OUTPUT) value - total number of instances
*                (OUTPUT) value - returned value information
*                (INPUT)  length - size of the returned information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectTotalLineSettings(DAL_VOICE_PARMS *parms, unsigned int *value, unsigned int *list);

/***************************************************************************
* Function Name: dalVoice_GetDectTotalContacts
*
* Description  : Gets total number of contacts from MDM
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (OUTPUT) value - total number of instances
*                (OUTPUT) value - returned value information
*                (INPUT)  length - size of the returned information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectTotalContacts(DAL_VOICE_PARMS *parms, unsigned int *value, unsigned int *list);

/***************************************************************************
* Function Name: dalVoice_GetDectContactListName
*
* Description  : Gets Dect Reset base value from MDM
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (INPUT)  parms->op[1] - contact list instance
*                (OUTPUT) value - returned value information
*                (INPUT)  length - size of the returned information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectContactListName(DAL_VOICE_PARMS *parms, char *value, unsigned int length );


/***************************************************************************
* Function Name: dalVoice_GetVoiceCallLineId
*
* Description  : Gets line Id from call list
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (INPUT)  parms->op[1] - call list instance
*                (OUTPUT) value - returned value information
*                (INPUT)  length - size of the returned information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetVoiceCallLineId(DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_SetDectHandsetName
*
* Description  : Set dect internal name
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (INPUT)  parms->op[1] - handsetId
*                (INPUT)  value - returned value information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetDectHandsetName(DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetDectHsCallInterception
* Description  : sets dect HS call interception status
*
* Parameters   : parms
*                value - returned value information
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetDectHsCallInterception(DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetDectContactListName
*
* Description  : Gets Dect Reset base value from MDM
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (INPUT)  parms->op[1] - list object instance
*                (INPUT)  value - returned value information
*                (INPUT)  length - size of the returned information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetDectContactListName(DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetDectContactListFirstName
*
* Description  : Gets Dect Reset base value from MDM
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (INPUT)  parms->op[1] - contact list instance
*                (INPUT) value - returned value information
*                (INPUT)  length - size of the returned information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetDectContactListFirstName(DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetDectContactListMelody
*
* Description  : Gets Dect Reset base value from MDM
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (INPUT)  parms->op[1] - contact list instance
*                (INPUT) value - returned value information
*                (INPUT)  length - size of the returned information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetDectContactListMelody(DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetDectContactListLineId
*
* Description  : Gets Dect Reset base value from MDM
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (INPUT)  parms->op[1] - contact list instance
*                (INPUT) value - returned value information
*                (INPUT)  length - size of the returned information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetDectContactListLineId(DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetDectContactListNumber
*
* Description  : Gets Dect contact number from MDM
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (INPUT)  parms->op[1] - contact list instance
*                (INPUT)  value - returned value information
*                (INPUT)  length - size of the returned information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetDectContactListNumber(DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetVoiceCallNewFlag
*
* Description  : Set calling party number in call list
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (INPUT)  parms->op[1] - list object instance
*                (INPUT)  value - returned value information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVoiceCallNewFlag(DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetVoiceCallName
*
* Description  : Set calling party name in call list
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (INPUT)  parms->op[1] - list object instance
*                (INPUT)  value - returned value information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVoiceCallName(DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetVoiceCallNumber
*
* Description  : Set calling party number in call list
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (INPUT)  parms->op[1] - list object instance
*                (INPUT)  value - returned value information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVoiceCallNumber(DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetVoiceCallDateTime
*
* Description  : Set calling party number in call list
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (INPUT)  parms->op[1] - list object instance
*                (INPUT)  value - returned value information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVoiceCallDateTime(DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetDectCallLineName
*
* Description  : Set calling party number in call list
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (INPUT)  parms->op[1] - list object instance
*                (INPUT)  value - returned value information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVoiceCallLineName(DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetDectCallLineId
*
* Description  : Set calling party number in call list
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (INPUT)  parms->op[1] - list object instance
*                (INPUT)  value - returned value information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVoiceCallLineId(DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetVoiceCallCallType
*
* Description  : Set calling party number in call list
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (INPUT)  parms->op[1] - list object instance
*                (INPUT)  value - returned value information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVoiceCallType(DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_AddVoiceCallListEntry
*
* Description  :
*
* Parameters   :
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_AddVoiceCallListEntry(DAL_VOICE_PARMS *parms, char *name, char *number, char *dateTime, char *lineName, int lineId, char *callType);

/***************************************************************************
* Function Name: dalVoice_SetDectHsetName
* Description  : Set registered dect handset name
*
* Parameters   : (INPUT) parms->op[0] - voice service instance
*                (INPUT) parms->op[1] - handset Id
*                (INPUT) value - name information
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetDectHsetName(DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_AddVoiceCallInstance
*
* Description  : Add instance into call list in MDM
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (OUTPUT) value - last added instance id
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_AddVoiceCallInstance(DAL_VOICE_PARMS *parms, unsigned int *value);

/***************************************************************************
* Function Name: dalVoice_AddVoiceContactInstance
*
* Description  : Gets total number of contacts from MDM
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (OUTPUT) value - last added instance id
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_AddVoiceContactInstance(DAL_VOICE_PARMS *parms, unsigned int *value);

/***************************************************************************
* Function Name: dalVoice_DeleteVoiceCallInstance
*
* Description  : Delete one instance from call list
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (INPUT)  parms->op[1] - list object instance
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_DeleteVoiceCallInstance(DAL_VOICE_PARMS *parms);

/***************************************************************************
* Function Name: dalVoice_DeleteVoiceContactInstance
*
* Description  : Gets total number of contacts from MDM
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (INPUT)  parms->op[1] - contact list instance
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_DeleteVoiceContactInstance(DAL_VOICE_PARMS *parms);

/***************************************************************************
* Function Name: dalVoice_DeleteDectHandsetNameInstance
*
* Description  : delete handset instance from MDM
*                by sending de-registered message to dectd 
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (INPUT)  parms->op[1] - handset instance
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_DeleteDectHandsetNameInstance(DAL_VOICE_PARMS *parms);

/***************************************************************************
* Function Name: dalVoice_SetDectPinCode
*
* Description  : Sets Pin code in MDM
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (OUTPUT) value - returned value information
*                (INPUT)  length - size of the returned information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetDectPinCode(DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetDectFwVersion
* Description  : Gets information from the DECT module about the modele
*                identifier of the handset registered.
*
* Parameters   : parms
*                value - returned value information
*                length - size of the returned information
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetDectFwVersion(DAL_VOICE_PARMS *parms, char *value);

/***************************************************************************
* Function Name: dalVoice_SetDectClockMaster
*
* Description  : Sets Dect clock master value in MDM
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (OUTPUT) value - returned value information
*                (INPUT)  length - size of the returned information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetDectClockMaster(DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetDectResetBase
*
* Description  : Sets Dect Reset base value in MDM
*
* Parameters   : (INPUT)  parms->op[0] - voice service instance
*                (OUTPUT) value - returned value information
*                (INPUT)  length - size of the returned information
*
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetDectResetBase(DAL_VOICE_PARMS *parms, char *value );


#endif

/***************************************************************************
* Function Name: dalVoice_GetDectSupport
* Description  : Checks whether DECT is supported as part of this voice
*                application or not.
*
* Parameters   : parms
*                enabled - returned value information, will be '1' is DECT
*                          is supported on this voice application, or will
*                          be '0' otherwise.
*                length - size of the returned information
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetDectSupport(DAL_VOICE_PARMS *parms, char *enabled, unsigned int length );

CmsRet dalVoice_GetVlCFCallFwdAll( DAL_VOICE_PARMS *parms, char *getVal, unsigned int length );
CmsRet dalVoice_GetVlCFCallFwdBusy( DAL_VOICE_PARMS *parms, char *getVal, unsigned int length );
CmsRet dalVoice_GetVlCFCallFwdNoAns( DAL_VOICE_PARMS *parms, char *getVal, unsigned int length );


CmsRet dalVoice_MapDectLineToCmLine( int dectline, int *cmline );
CmsRet dalVoice_MapCmLineToDectLine( int cmline, int *dectline );
CmsRet dalVoice_GetLineAttachedHandsetMask(DAL_VOICE_PARMS *parms, UINT32 *mask);
CmsRet  dalVoice_GetCcLineFeatureSetAssoc( DAL_VOICE_PARMS *parms );


/***************************************************************************
* Function Name: dalVoice_SetVlCFCallReturn
* Description  : Enable or disable auto call return..
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = feature inst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlCFCallReturn( DAL_VOICE_PARMS *parms, char *value );
/***************************************************************************
* Function Name: dalVoice_SetVlCFCallRedial
* Description  : Enable or disable call redial feature
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = feature inst, value = yes/no
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlCFCallRedial( DAL_VOICE_PARMS *parms, char *value );
/*****************************************************************
**  FUNCTION:       dalVoice_GetVlCFCallRedial
**
**  PURPOSE:        get callredial feature enable status
**
**  INPUT PARMS:    vpInst - parms->op[0]
**
**  OUTPUT PARMS:   'call redial' enable flag (Yes/No)
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVlCFCallRedial( DAL_VOICE_PARMS *parms, char *getVal, unsigned int length );
/*****************************************************************
**  FUNCTION:       dalVoice_GetVlCFCallReturn
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0]
**
**  OUTPUT PARMS:   'call waiting' enable flag (Yes/No)
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVlCFCallReturn( DAL_VOICE_PARMS *parms, char *getVal, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetCallCtrlExtFxsList
**
**  PURPOSE:        Returns list of extensions which associate with FXS
**
**  INPUT PARMS:    op[0] - voice service instance
**
**  OUTPUT PARMS:   value - true/false
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetCallCtrlExtFxsList( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetCallCtrlLineFxoList
**
**  PURPOSE:        Returns list of lines which associate with FXO
**
**  INPUT PARMS:    op[0] - voice service instance
**
**  OUTPUT PARMS:   value - true/false
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetCallCtrlLineFxoList( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetCcLineStatsRtpPacketSent
**
**  PURPOSE:        set call status of line
**
**  INPUT PARMS:    op[0] - voice service instance
**                  op[1] - line instance
**                  value - total rtp packets sent 
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetCcLineStatsRtpPacketSent( DAL_VOICE_PARMS *parms, UINT32 value );

/*****************************************************************
**  FUNCTION:       dalVoice_SetCcLineStatsRtpPacketRecv
**
**  PURPOSE:        set RTP received value of line
**
**  INPUT PARMS:    op[0] - voice service instance
**                  op[1] - line instance
**                  value - total rtp packets received
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetCcLineStatsRtpPacketRecv( DAL_VOICE_PARMS *parms, UINT32 value );

/*****************************************************************
**  FUNCTION:       dalVoice_SetCcLineStatsRtpPacketLost
**
**  PURPOSE:        set RTP received value of line
**
**  INPUT PARMS:    op[0] - voice service instance
**                  op[1] - line instance
**                  value - total rtp packets lost
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetCcLineStatsRtpPacketLost( DAL_VOICE_PARMS *parms, UINT32 value );

/*****************************************************************
**  FUNCTION:       dalVoice_SetCcLineStatsRtpBytesSent
**
**  PURPOSE:        set RTP bytes sent value of line
**
**  INPUT PARMS:    op[0] - voice service instance
**                  op[1] - line instance
**                  value - total bytes sent
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetCcLineStatsRtpBytesSent( DAL_VOICE_PARMS *parms, UINT32 value );

/*****************************************************************
**  FUNCTION:       dalVoice_SetCcLineStatsRtpBytesRecv
**
**  PURPOSE:        set RTP received bytes of line
**
**  INPUT PARMS:    op[0] - voice service instance
**                  op[1] - line instance
**                  value - total bytes received
**
**  RETURNS:        CMSRET_SUCCESS if success
**                  otherwise failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetCcLineStatsRtpBytesRecv( DAL_VOICE_PARMS *parms, UINT32 value );

CmsRet dalVoice_GetCcLineStatsRtpPacketSentString( DAL_VOICE_PARMS *parms, char* value, unsigned int len );
CmsRet dalVoice_GetCcLineStatsRtpPacketRecvString( DAL_VOICE_PARMS *parms, char* value, unsigned int len );
CmsRet dalVoice_GetCcLineStatsRtpPacketLostString( DAL_VOICE_PARMS *parms, char* value, unsigned int len );
CmsRet dalVoice_GetCcLineStatsRtpBytesSentString( DAL_VOICE_PARMS *parms, char* value, unsigned int len );
CmsRet dalVoice_GetCcLineStatsRtpBytesRecvString( DAL_VOICE_PARMS *parms, char* value, unsigned int len );
CmsRet dalVoice_SetCcLineStatsInCallRecv( DAL_VOICE_PARMS *parms, UINT32 value );
CmsRet dalVoice_SetCcLineStatsInCallConn( DAL_VOICE_PARMS *parms, UINT32 value );
CmsRet dalVoice_SetCcLineStatsInCallFailed( DAL_VOICE_PARMS *parms, UINT32 value );
CmsRet dalVoice_SetCcLineStatsInCallDrop( DAL_VOICE_PARMS *parms, UINT32 value );
CmsRet dalVoice_SetCcLineStatsInTotalCallTime( DAL_VOICE_PARMS *parms, UINT32 value );
CmsRet dalVoice_SetCcLineStatsOutCallAttempt( DAL_VOICE_PARMS *parms, UINT32 value );
CmsRet dalVoice_SetCcLineStatsOutCallConn( DAL_VOICE_PARMS *parms, UINT32 value );
CmsRet dalVoice_SetCcLineStatsOutCallFailed( DAL_VOICE_PARMS *parms, UINT32 value );
CmsRet dalVoice_SetCcLineStatsOutCallDrop( DAL_VOICE_PARMS *parms, UINT32 value );
CmsRet dalVoice_SetCcLineStatsOutTotalCallTime( DAL_VOICE_PARMS *parms, UINT32 value );
CmsRet dalVoice_GetCcLineStatsInCallRecvString( DAL_VOICE_PARMS *parms, char *value, unsigned int len );
CmsRet dalVoice_GetCcLineStatsInCallConnString( DAL_VOICE_PARMS *parms, char *value, unsigned int len );
CmsRet dalVoice_GetCcLineStatsInCallFailedString( DAL_VOICE_PARMS *parms, char *value, unsigned int len );
CmsRet dalVoice_GetCcLineStatsInCallDropString( DAL_VOICE_PARMS *parms, char *value, unsigned int len );
CmsRet dalVoice_GetCcLineStatsInTotalCallTimeString( DAL_VOICE_PARMS *parms, char *value, unsigned int len );
CmsRet dalVoice_GetCcLineStatsOutCallAttemptString( DAL_VOICE_PARMS *parms, char *value, unsigned int len );
CmsRet dalVoice_GetCcLineStatsOutCallConnString( DAL_VOICE_PARMS *parms, char *value, unsigned int len );
CmsRet dalVoice_GetCcLineStatsOutCallFailedString( DAL_VOICE_PARMS *parms, char *value, unsigned int len );
CmsRet dalVoice_GetCcLineStatsOutCallDropString( DAL_VOICE_PARMS *parms, char *value, unsigned int len );
CmsRet dalVoice_GetCcLineStatsOutTotalCallTimeString( DAL_VOICE_PARMS *parms, char *value, unsigned int len );
CmsRet dalVoice_ResetCcLineStats( DAL_VOICE_PARMS *parms );
CmsRet dalVoice_SetCcLineResetStats( DAL_VOICE_PARMS *parms, char* value );

CmsRet dalVoice_SetEuroFlashEnable( DAL_VOICE_PARMS *parms, char *value );
CmsRet dalVoice_GetEuroFlashEnable( DAL_VOICE_PARMS *parms, char *pEuroFlashEn, unsigned int length );
CmsRet dalVoice_SetCCBSEnable( DAL_VOICE_PARMS *parms, char *value );
CmsRet dalVoice_GetCCBSEnable( DAL_VOICE_PARMS *parms, char *pEuroFlashEn, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_mapCallLogNumToInst
*
* PURPOSE:     Map calllog number to MDM instance number
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = call log number to map (0-based)
*
* OUTPUT:      inst = calllog MDM instance number
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_mapCallLogNumToInst( DAL_VOICE_PARMS *parms, int *inst );

/****************************************************************************
* FUNCTION:    dalVoice_GetNumCallLog
*
* PURPOSE:     Get number of calllog instances
*
* PARAMETERS:  parms->op[0] = vpInst
*              value = string buffer to receive data
*              length = length of value buffer
*
* OUTPUT:      value = string containing number of callog instances in MDM
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetNumCallLog( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_AddCallLogInstance
*
* PURPOSE:     Add a calllog instance to MDM
*
* PARAMETERS:  parms->op[0] = vpInst
*
* OUTPUT:      value = new calllog instance number
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_AddCallLogInstance( DAL_VOICE_PARMS *parms, int *value );

/****************************************************************************
* FUNCTION:    dalVoice_DeleteCallLogInstance
*
* PURPOSE:     Delete a calllog instance from MDM
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_DeleteCallLogInstance( DAL_VOICE_PARMS *parms );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogCallingParty
*
* PURPOSE:     Set calllog calling party
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              value = string buffer containing new value
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogCallingParty( DAL_VOICE_PARMS *parms, char *value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogCallingParty
*
* PURPOSE:     Get calllog calling party
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing calling party
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogCallingParty( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogCalledParty
*
* PURPOSE:     Set calllog called party
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              value = string buffer containing new value
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogCalledParty( DAL_VOICE_PARMS *parms, char *value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogCalledParty
*
* PURPOSE:     Get calllog called party
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing called party
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogCalledParty( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogStartDateTime
*
* PURPOSE:     Set calllog start of call timestamp
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              value = string buffer containing new value
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogStartDateTime( DAL_VOICE_PARMS *parms, char *value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogStartDateTime
*
* PURPOSE:     Get calllog start of call timestamp
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing timestamp
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogStartDateTime( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogStopDateTime
*
* PURPOSE:     Set calllog end of call timestamp
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              value = string buffer containing new value
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogStopDateTime( DAL_VOICE_PARMS *parms, char *value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogStopDateTime
*
* PURPOSE:     Get calllog end of call timestamp
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing timestamp
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogStopDateTime( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogDuration
*
* PURPOSE:     Set calllog call duration in seconds
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              value = call duration in seconds
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogDuration( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogDuration
*
* PURPOSE:     Get calllog call duration in seconds
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing call duration in seconds
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogDuration( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogDuration
*
* PURPOSE:     Set calllog call direction
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              value = string buffer containing new value
*                      can be either "Incoming" MDMVS_INCOMING
*                      or "Outgoing" MDMVS_OUTGOING
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogDirection( DAL_VOICE_PARMS *parms, char *value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogDirection
*
* PURPOSE:     Get calllog call direction
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing call direction
*                      can be either "Incoming" MDMVS_INCOMING
*                      or "Outgoing" MDMVS_OUTGOING
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogDirection( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogDuration
*
* PURPOSE:     Set calllog call termination reason
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              value = string buffer containing new value
*                      currently support:
*                      MDMVS_LOCALREJECT, MDMVS_LOCALDISCONNECT,
*                      MDMVS_REMOTEDISCONNECT
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogReason( DAL_VOICE_PARMS *parms, char *value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogReason
*
* PURPOSE:     Get calllog call termination reason
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing call termination reason
*                      currently support:
*                      MDMVS_LOCALREJECT, MDMVS_LOCALDISCONNECT,
*                      MDMVS_REMOTEDISCONNECT
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogReason( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogUsedLineAssoc
*
* PURPOSE:     Set calllog call UsedLine.
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              cmLine = call manager line
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogUsedLineAssoc( DAL_VOICE_PARMS *parms, unsigned int cmLine );

/****************************************************************************
* FUNCTION:    dalVoice_AddCallLogSessionInstance
*
* PURPOSE:     Add session instance to calllog instance
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*
* OUTPUT:      value = new session instance number
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_AddCallLogSessionInstance( DAL_VOICE_PARMS *parms, int *value );

/****************************************************************************
* FUNCTION:    dalVoice_DeleteCallLogSessionInstance
*
* PURPOSE:     Delete session instance from calllog instance
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_DeleteCallLogSessionInstance( DAL_VOICE_PARMS *parms );

/****************************************************************************
* FUNCTION:    dalVoice_GetNumCallLogSession
*
* PURPOSE:     Get number of session instances in a calllog instance.
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing number of sessions
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetNumCallLogSession( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_mapCallLogSessionNumToInst
*
* PURPOSE:     Map session number (0-based) to session instance number.
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*
* OUTPUT:      inst = session instance number in MDM
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_mapCallLogSessionNumToInst( DAL_VOICE_PARMS *parms, int *inst );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStartDateTime
*
* PURPOSE:     Set calllog session start of call timestamp
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer containing new value
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStartDateTime( DAL_VOICE_PARMS *parms, char *value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStartDateTime
*
* PURPOSE:     Get calllog session start of call timestamp
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing start of call timestamp
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStartDateTime( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStopDateTime
*
* PURPOSE:     Set calllog session end of call timestamp
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer containing new value
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStopDateTime( DAL_VOICE_PARMS *parms, char *value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStopDateTime
*
* PURPOSE:     Get calllog session end of call timestamp
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing end of call timestamp
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStopDateTime( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionDuration
*
* PURPOSE:     Set calllog session duration in seconds
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = session duration in seconds
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionDuration( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionDuration
*
* PURPOSE:     Get calllog session duration in seconds
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session duration in seconds
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionDuration( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionSessionId
*
* PURPOSE:     Set calllog session ID
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer containing new value
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionSessionId( DAL_VOICE_PARMS *parms, char *value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionSessionId
*
* PURPOSE:     Get calllog session ID
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session ID
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionSessionId( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsLocalValid
*
* PURPOSE:     Set calllog session stats local valid flag
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = 1 (true) if the local stats are valid
*                    = 0 (false) if local stats are not valid
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsLocalValid( DAL_VOICE_PARMS *parms, UBOOL8 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsLocalValid
*
* PURPOSE:     Get calllog session local stats valid flag
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing local stats valid flag
*                      MDMVS_YES = "Yes", or MDMVS_NO = "No"
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsLocalValid( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsSsrcOfSource
*
* PURPOSE:     Set calllog session SSRC of source
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsSsrcOfSource( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsSsrcOfSource
*
* PURPOSE:     Get calllog session SSRC of source
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session SSRC of source
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsSsrcOfSource( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsLossRate
*
* PURPOSE:     Set calllog session loss rate
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsLossRate( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsLossRate
*
* PURPOSE:     Get calllog session loss rate
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session loss rate
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsLossRate( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsDiscardRate
*
* PURPOSE:     Set calllog session discard date
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsDiscardRate( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsDiscardRate
*
* PURPOSE:     Get calllog session discard rate
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session discard rate
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsDiscardRate( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsBurstDensity
*
* PURPOSE:     Set calllog session burst density
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsBurstDensity( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsBurstDensity
*
* PURPOSE:     Get calllog session burst density
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session burst density
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsBurstDensity( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsGapDensity
*
* PURPOSE:     Set calllog session gap density
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsGapDensity( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsGapDensity
*
* PURPOSE:     Get calllog session gap density
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session gap density
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsGapDensity( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsBurstDuration
*
* PURPOSE:     Set calllog session burst duration
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsBurstDuration( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsBurstDuration
*
* PURPOSE:     Get calllog session burst duration
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session burst duration
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsBurstDuration( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsGapDuration
*
* PURPOSE:     Set calllog session gap duration
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsGapDuration( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsGapDuration
*
* PURPOSE:     Get calllog session gap duration
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session gap duration
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsGapDuration( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsRoundTripDelay
*
* PURPOSE:     Set calllog session round trip delay
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsRoundTripDelay( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsRoundTripDelay
*
* PURPOSE:     Get calllog session round trip delay
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session round trip delay
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsRoundTripDelay( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsEndSystemDelay
*
* PURPOSE:     Set calllog session end system delay
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsEndSystemDelay( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsEndSystemDelay
*
* PURPOSE:     Get calllog session end system delay
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session end system delay
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsEndSystemDelay( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsSignalLevel
*
* PURPOSE:     Set calllog session signal level
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsSignalLevel( DAL_VOICE_PARMS *parms, SINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsSignalLevel
*
* PURPOSE:     Get calllog session signal level
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session signal level
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsSignalLevel( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsNoiseLevel
*
* PURPOSE:     Set calllog session noise level
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsNoiseLevel( DAL_VOICE_PARMS *parms, SINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsNoiseLevel
*
* PURPOSE:     Get calllog session noise level
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session noise level
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsNoiseLevel( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsRERL
*
* PURPOSE:     Set calllog session RERL
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsRERL( DAL_VOICE_PARMS *parms, SINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsRERL
*
* PURPOSE:     Get calllog session RERL
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session RERL
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsRERL( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsGMin
*
* PURPOSE:     Set calllog session GMin
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsGMin( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsGMin
*
* PURPOSE:     Get calllog session GMin
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session GMin
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsGMin( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsRFactor
*
* PURPOSE:     Set calllog session RFactor
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsRFactor( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsRFactor
*
* PURPOSE:     Get calllog session RFactor
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session RFactor
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsRFactor( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsExtRFactor
*
* PURPOSE:     Set calllog session Ext RFactor
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsExtRFactor( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsExtRFactor
*
* PURPOSE:     Get calllog session Ext RFactor
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session Ext RFactor
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsExtRFactor( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsMOSCQ
*
* PURPOSE:     Set calllog session MOSCQ
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsMOSCQ( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsMOSCQ
*
* PURPOSE:     Get calllog session MOSCQ
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session MOSCQ
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsMOSCQ( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsMOSLQ
*
* PURPOSE:     Set calllog session MOSLQ
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsMOSLQ( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsMOSLQ
*
* PURPOSE:     Get calllog session MOSLQ
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session MOSLQ
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsMOSLQ( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsPLC
*
* PURPOSE:     Set calllog session Packet Loss Concealment Type
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsPLC( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsPLC
*
* PURPOSE:     Get calllog session Packet Loss Concealment Type
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session Packet Loss Concealment Type
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsPLC( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsJBAdaptive
*
* PURPOSE:     Set calllog session Jitter Buffer Adaptive
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsJBAdaptive( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsJBAdaptive
*
* PURPOSE:     Get calllog session jitter buffer adaptive
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session jitter buffer adaptive
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsJBAdaptive( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsJBAdaptive
*
* PURPOSE:     Set calllog session Jitter Buffer Rate
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsJBRate( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsJBRate
*
* PURPOSE:     Get calllog session jitter buffer rate
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session jitter buffer rate
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsJBRate( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsJBNominal
*
* PURPOSE:     Set calllog session Jitter Buffer Nominal rate
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsJBNominal( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsJBNominal
*
* PURPOSE:     Get calllog session jitter buffer nominal rate
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session jitter buffer nominal rate
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsJBNominal( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsJBMaximum
*
* PURPOSE:     Set calllog session Jitter Buffer Maximum delay
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsJBMaximum( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsJBMaximum
*
* PURPOSE:     Get calllog session jitter buffer maximum rate
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session jitter buffer maximum rate
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsJBMaximum( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsJBAbsMax
*
* PURPOSE:     Set calllog session Jitter Buffer Absolute Maximum delay
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsJBAbsMax( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsJBAbsMax
*
* PURPOSE:     Get calllog session jitter buffer absolute maximum rate
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session jitter buffer absolute maximum rate
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsJBAbsMax( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsRemoteValid
*
* PURPOSE:     Set calllog session stats remote valid flag
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = 1 (true) if the remote stats are valid
*                    = 0 (false) if remote stats are not valid
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsRemoteValid( DAL_VOICE_PARMS *parms, UBOOL8 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsRemoteValid
*
* PURPOSE:     Get calllog session remote stats valid flag
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing remote stats valid flag
*                      MDMVS_YES = "Yes", or MDMVS_NO = "No"
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsRemoteValid( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsRemSsrcOfSource
*
* PURPOSE:     Set calllog session remote SSRC of source
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsRemSsrcOfSource( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsRemSsrcOfSource
*
* PURPOSE:     Get calllog session remote SSRC of source
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session remote SSRC of source
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsRemSsrcOfSource( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsRemLossRate
*
* PURPOSE:     Set calllog session remote loss rate
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsRemLossRate( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsRemLossRate
*
* PURPOSE:     Get calllog session remote loss rate
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session remote loss rate
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsRemLossRate( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsRemDiscardRate
*
* PURPOSE:     Set calllog session remote discard date
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsRemDiscardRate( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsRemDiscardRate
*
* PURPOSE:     Get calllog session remote discard rate
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session remote discard rate
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsRemDiscardRate( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsRemBurstDensity
*
* PURPOSE:     Set calllog session remote burst density
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsRemBurstDensity( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsRemBurstDensity
*
* PURPOSE:     Get calllog session remote burst density
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session remote burst density
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsRemBurstDensity( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsRemGapDensity
*
* PURPOSE:     Set calllog session remote gap density
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsRemGapDensity( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsRemGapDensity
*
* PURPOSE:     Get calllog session remote gap density
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session remote gap density
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsRemGapDensity( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsRemBurstDuration
*
* PURPOSE:     Set calllog session remote burst duration
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsRemBurstDuration( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsRemBurstDuration
*
* PURPOSE:     Get calllog session remote burst duration
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session remote burst duration
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsRemBurstDuration( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsRemGapDuration
*
* PURPOSE:     Set calllog session remote gap duration
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsRemGapDuration( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsRemGapDuration
*
* PURPOSE:     Get calllog session remote gap duration
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session remote gap duration
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsRemGapDuration( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsRemRoundTripDelay
*
* PURPOSE:     Set calllog session remote round trip delay
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsRemRoundTripDelay( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsRemRoundTripDelay
*
* PURPOSE:     Get calllog session remote round trip delay
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session remote round trip delay
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsRemRoundTripDelay( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsRemEndSystemDelay
*
* PURPOSE:     Set calllog session remote end system delay
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsRemEndSystemDelay( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsRemEndSystemDelay
*
* PURPOSE:     Get calllog session remote end system delay
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session remote end system delay
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsRemEndSystemDelay( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsRemSignalLevel
*
* PURPOSE:     Set calllog session remote signal level
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsRemSignalLevel( DAL_VOICE_PARMS *parms, SINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsRemSignalLevel
*
* PURPOSE:     Get calllog session remote signal level
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session remote signal level
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsRemSignalLevel( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsRemNoiseLevel
*
* PURPOSE:     Set calllog session remote noise level
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsRemNoiseLevel( DAL_VOICE_PARMS *parms, SINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsRemNoiseLevel
*
* PURPOSE:     Get calllog session remote noise level
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session remote noise level
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsRemNoiseLevel( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsRemRERL
*
* PURPOSE:     Set calllog session remote RERL
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsRemRERL( DAL_VOICE_PARMS *parms, SINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsRemRERL
*
* PURPOSE:     Get calllog session remote RERL
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session remote RERL
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsRemRERL( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsRemGMin
*
* PURPOSE:     Set calllog session remote GMin
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsRemGMin( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsRemGMin
*
* PURPOSE:     Get calllog session remote GMin
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session remote GMin
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsRemGMin( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsRemRFactor
*
* PURPOSE:     Set calllog session remote RFactor
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsRemRFactor( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsRemRFactor
*
* PURPOSE:     Get calllog session remote RFactor
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session remote RFactor
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsRemRFactor( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsRemExtRFactor
*
* PURPOSE:     Set calllog session remote Ext RFactor
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsRemExtRFactor( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsRemExtRFactor
*
* PURPOSE:     Get calllog session remote Ext RFactor
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session remote Ext RFactor
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsRemExtRFactor( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsRemMOSCQ
*
* PURPOSE:     Set calllog session remote MOSCQ
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsRemMOSCQ( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsRemMOSCQ
*
* PURPOSE:     Get calllog session remote MOSCQ
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session remote MOSCQ
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsRemMOSCQ( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsMOSLQ
*
* PURPOSE:     Set calllog session remote MOSLQ
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsRemMOSLQ( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsRemMOSLQ
*
* PURPOSE:     Get calllog session remote MOSLQ
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session remote MOSLQ
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsRemMOSLQ( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsRemPLC
*
* PURPOSE:     Set calllog session remote Packet Loss Concealment Type
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsRemPLC( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsRemPLC
*
* PURPOSE:     Get calllog session remote Packet Loss Concealment Type
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session remote Packet Loss Concealment Type
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsRemPLC( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsRemJBAdaptive
*
* PURPOSE:     Set calllog session remote Jitter Buffer Adaptive
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsRemJBAdaptive( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsRemJBAdaptive
*
* PURPOSE:     Get calllog session remote jitter buffer adaptive
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session remote jitter buffer adaptive
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsRemJBAdaptive( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsRemJBAdaptive
*
* PURPOSE:     Set calllog session remote Jitter Buffer Rate
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsRemJBRate( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsRemJBRate
*
* PURPOSE:     Get calllog session remote jitter buffer rate
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session remote jitter buffer rate
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsRemJBRate( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsRemJBNominal
*
* PURPOSE:     Set calllog session remote Jitter Buffer Nominal rate
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsRemJBNominal( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsRemJBNominal
*
* PURPOSE:     Get calllog session remote jitter buffer nominal rate
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session remote jitter buffer nominal rate
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsRemJBNominal( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsRemJBMaximum
*
* PURPOSE:     Set calllog session remote Jitter Buffer Maximum delay
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsRemJBMaximum( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsRemJBMaximum
*
* PURPOSE:     Get calllog session remote jitter buffer maximum rate
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session remote jitter buffer maximum rate
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsRemJBMaximum( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsRemJBAbsMax
*
* PURPOSE:     Set calllog session remote Jitter Buffer Absolute Maximum delay
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsRemJBAbsMax( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsRemJBAbsMax
*
* PURPOSE:     Get calllog session remote jitter buffer absolute maximum rate
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session remote jitter buffer
*                      absolute maximum rate
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsRemJBAbsMax( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionStatsCallTrace
*
* PURPOSE:     Set calllog session call trace
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionStatsCallTrace( DAL_VOICE_PARMS *parms, char *value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionStatsCallTrace
*
* PURPOSE:     Get calllog session call trace
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing session call trace
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionStatsCallTrace( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionSrcRtpFarEndIpAddress
*
* PURPOSE:     Set calllog session source rtp far end IP address
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionSrcRtpFarEndIpAddress( DAL_VOICE_PARMS *parms, char *value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionSrcRtpFarEndIpAddress
*
* PURPOSE:     Get calllog session source rtp far end IP address
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing far end IP address
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionSrcRtpFarEndIpAddress( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionSrcRtpFarEndUDPPort
*
* PURPOSE:     Set calllog session source rtp far end UDP port
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionSrcRtpFarEndUDPPort( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionSrcRtpFarEndUDPPort
*
* PURPOSE:     Get calllog session source rtp far end UDP port
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing far end UDP port
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionSrcRtpFarEndUDPPort( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionSrcRtpLocalUDPPort
*
* PURPOSE:     Set calllog session source rtp local UDP port
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionSrcRtpLocalUDPPort( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionSrcRtpLocalUDPPort
*
* PURPOSE:     Get calllog session source rtp local UDP port
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing local UDP port
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionSrcRtpLocalUDPPort( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionSrcRtpPacketsReceived
*
* PURPOSE:     Set calllog session source rtp packets received
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionSrcRtpPacketsReceived( DAL_VOICE_PARMS *parms, UINT64 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionSrcRtpPacketsReceived
*
* PURPOSE:     Get calllog session source rtp packets received
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing rtp packets received
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionSrcRtpPacketsReceived( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionSrcRtpPacketsSent
*
* PURPOSE:     Set calllog session source rtp packets sent
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionSrcRtpPacketsSent( DAL_VOICE_PARMS *parms, UINT64 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionSrcRtpPacketsSent
*
* PURPOSE:     Get calllog session source rtp packets sent
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing rtp packets sent
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionSrcRtpPacketsSent( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionSrcRtpPacketsLost
*
* PURPOSE:     Set calllog session source rtp packets lost
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionSrcRtpPacketsLost( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionSrcRtpPacketsLost
*
* PURPOSE:     Get calllog session source rtp packets lost
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing rtp packets lost
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionSrcRtpPacketsLost( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionSrcRtpPacketsDiscarded
*
* PURPOSE:     Set calllog session source rtp packets discarded
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionSrcRtpPacketsDiscarded( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionSrcRtpPacketsDiscarded
*
* PURPOSE:     Get calllog session source rtp packets discarded
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing rtp packets discarded
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionSrcRtpPacketsDiscarded( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionSrcRtpBytesReceived
*
* PURPOSE:     Set calllog session source rtp packets received
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionSrcRtpBytesReceived( DAL_VOICE_PARMS *parms, UINT64 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionSrcRtpBytesReceived
*
* PURPOSE:     Get calllog session source rtp bytes received
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing rtp bytes received
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionSrcRtpBytesReceived( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionSrcRtpBytesSent
*
* PURPOSE:     Set calllog session source rtp bytes sent
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionSrcRtpBytesSent( DAL_VOICE_PARMS *parms, UINT64 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionSrcRtpBytesSent
*
* PURPOSE:     Get calllog session source rtp bytes sent
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing rtp bytes sent
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionSrcRtpBytesSent( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionSrcRtpReceivePacketLossRate
*
* PURPOSE:     Set calllog session source rtp receive packet loss rate
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionSrcRtpReceivePacketLossRate( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionSrcRtpReceivePacketLossRate
*
* PURPOSE:     Get calllog session source rtp packet loss rate
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing rtp packet loss rate
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionSrcRtpReceivePacketLossRate( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionSrcRtpFarEndPacketLossRate
*
* PURPOSE:     Set calllog session source rtp far end packet loss rate
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionSrcRtpFarEndPacketLossRate( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionSrcRtpFarEndPacketLossRate
*
* PURPOSE:     Get calllog session source rtp far end packet loss rate
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing rtp far end packet loss rate
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionSrcRtpFarEndPacketLossRate( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionSrcRtpReceiveInterarrivalJitter
*
* PURPOSE:     Set calllog session source rtp receive interarrival jitter
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionSrcRtpReceiveInterarrivalJitter( DAL_VOICE_PARMS *parms, SINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionSrcRtpReceiveInterarrivalJitter
*
* PURPOSE:     Get calllog session source rtp receive interarrival jitter
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing rtp receive interarrival jitter
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionSrcRtpReceiveInterarrivalJitter( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionSrcRtpFarEndInterarrivalJitter
*
* PURPOSE:     Set calllog session source rtp far end interarrival jitter
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionSrcRtpFarEndInterarrivalJitter( DAL_VOICE_PARMS *parms, SINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionSrcRtpFarEndInterarrivalJitter
*
* PURPOSE:     Get calllog session source rtp far end interarrival jitter
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing rtp far end interarrival jitter
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionSrcRtpFarEndInterarrivalJitter( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionSrcRtpRoundTripDelay
*
* PURPOSE:     Set calllog session source rtp round trip delay
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionSrcRtpRoundTripDelay( DAL_VOICE_PARMS *parms, SINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionSrcRtpRoundTripDelay
*
* PURPOSE:     Get calllog session source rtp round trip delay
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing rtp far round trip delay
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionSrcRtpRoundTripDelay( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionSrcRtpSamplingFrequency
*
* PURPOSE:     Set calllog session source rtp sampling frequency
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionSrcRtpSamplingFrequency( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionSrcRtpSamplingFrequency
*
* PURPOSE:     Get calllog session source rtp sampling frequency
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing rtp sampling frequency
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionSrcRtpSamplingFrequency( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionSrcRtpAverageTxDelay
*
* PURPOSE:     Set calllog session source rtp average tx delay
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionSrcRtpAverageTxDelay( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionSrcRtpAverageTxDelay
*
* PURPOSE:     Get calllog session source rtp average Tx delay
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing rtp average Tx delay
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionSrcRtpAverageTxDelay( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionSrcDspReceiveCodecCodec
*
* PURPOSE:     Set calllog session source DSP receive codec
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionSrcDspReceiveCodecCodec( DAL_VOICE_PARMS *parms, char *value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionSrcDspReceiveCodecCodec
*
* PURPOSE:     Get calllog session source DSP receive codec
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing DSP receive codec
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionSrcDspReceiveCodecCodec( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionSrcDspTransmitCodecCodec
*
* PURPOSE:     Set calllog session source DSP transmit codec
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionSrcDspTransmitCodecCodec( DAL_VOICE_PARMS *parms, char *value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionSrcDspTransmitCodecCodec
*
* PURPOSE:     Get calllog session source DSP transmit codec
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing DSP transmit codec
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionSrcDspTransmitCodecCodec( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionDstRtpFarEndIpAddress
*
* PURPOSE:     Set calllog session destination rtp far end IP address
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionDstRtpFarEndIpAddress( DAL_VOICE_PARMS *parms, char *value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionDstRtpFarEndIpAddress
*
* PURPOSE:     Get calllog session destination rtp far end IP address
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing far end IP address
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionDstRtpFarEndIpAddress( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionDstRtpFarEndUDPPort
*
* PURPOSE:     Set calllog session destination rtp far end UDP port
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionDstRtpFarEndUDPPort( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionDstRtpFarEndUDPPort
*
* PURPOSE:     Get calllog session destination rtp far end UDP port
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing far end UDP port
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionDstRtpFarEndUDPPort( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionDstRtpLocalUDPPort
*
* PURPOSE:     Set calllog session destination rtp local UDP port
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionDstRtpLocalUDPPort( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionDstRtpLocalUDPPort
*
* PURPOSE:     Get calllog session destination rtp local UDP port
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing local UDP port
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionDstRtpLocalUDPPort( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionDstRtpPacketsReceived
*
* PURPOSE:     Set calllog session destination rtp packets received
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionDstRtpPacketsReceived( DAL_VOICE_PARMS *parms, UINT64 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionDstRtpPacketsReceived
*
* PURPOSE:     Get calllog session destination rtp packets received
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing rtp packets received
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionDstRtpPacketsReceived( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionDstRtpPacketsSent
*
* PURPOSE:     Set calllog session destination rtp packets sent
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionDstRtpPacketsSent( DAL_VOICE_PARMS *parms, UINT64 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionDstRtpPacketsSent
*
* PURPOSE:     Get calllog session destination rtp packets sent
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing rtp packets sent
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionDstRtpPacketsSent( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionDstRtpPacketsLost
*
* PURPOSE:     Set calllog session destination rtp packets lost
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionDstRtpPacketsLost( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionDstRtpPacketsLost
*
* PURPOSE:     Get calllog session destination rtp packets lost
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing rtp packets lost
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionDstRtpPacketsLost( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionDstRtpPacketsDiscarded
*
* PURPOSE:     Set calllog session destination rtp packets discarded
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionDstRtpPacketsDiscarded( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionDstRtpPacketsDiscarded
*
* PURPOSE:     Get calllog session destination rtp packets discarded
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing rtp packets discarded
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionDstRtpPacketsDiscarded( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionDstRtpBytesReceived
*
* PURPOSE:     Set calllog session destination rtp packets received
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionDstRtpBytesReceived( DAL_VOICE_PARMS *parms, UINT64 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionDstRtpBytesReceived
*
* PURPOSE:     Get calllog session destination rtp bytes received
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing rtp bytes received
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionDstRtpBytesReceived( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionDstRtpBytesSent
*
* PURPOSE:     Set calllog session destination rtp bytes sent
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionDstRtpBytesSent( DAL_VOICE_PARMS *parms, UINT64 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionDstRtpBytesSent
*
* PURPOSE:     Get calllog session destination rtp bytes sent
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing rtp bytes sent
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionDstRtpBytesSent( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionDstRtpReceivePacketLossRate
*
* PURPOSE:     Set calllog session destination rtp receive packet loss rate
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionDstRtpReceivePacketLossRate( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionDstRtpReceivePacketLossRate
*
* PURPOSE:     Get calllog session destination rtp packet loss rate
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing rtp packet loss rate
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionDstRtpReceivePacketLossRate( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionDstRtpFarEndPacketLossRate
*
* PURPOSE:     Set calllog session destination rtp far end packet loss rate
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionDstRtpFarEndPacketLossRate( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionDstRtpFarEndPacketLossRate
*
* PURPOSE:     Get calllog session destination rtp far end packet loss rate
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing rtp far end packet loss rate
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionDstRtpFarEndPacketLossRate( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionDstRtpReceiveInterarrivalJitter
*
* PURPOSE:     Set calllog session destination rtp receive interarrival jitter
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionDstRtpReceiveInterarrivalJitter( DAL_VOICE_PARMS *parms, SINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionDstRtpReceiveInterarrivalJitter
*
* PURPOSE:     Get calllog session destination rtp receive interarrival jitter
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing rtp receive interarrival jitter
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionDstRtpReceiveInterarrivalJitter( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionDstRtpFarEndInterarrivalJitter
*
* PURPOSE:     Set calllog session destination rtp far end interarrival jitter
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionDstRtpFarEndInterarrivalJitter( DAL_VOICE_PARMS *parms, SINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionDstRtpFarEndInterarrivalJitter
*
* PURPOSE:     Get calllog session destination rtp far end interarrival jitter
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing rtp far end interarrival jitter
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionDstRtpFarEndInterarrivalJitter( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionDstRtpRoundTripDelay
*
* PURPOSE:     Set calllog session destination rtp round trip delay
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionDstRtpRoundTripDelay( DAL_VOICE_PARMS *parms, SINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionDstRtpRoundTripDelay
*
* PURPOSE:     Get calllog session destination rtp round trip delay
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing rtp far round trip delay
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionDstRtpRoundTripDelay( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionDstRtpSamplingFrequency
*
* PURPOSE:     Set calllog session destination rtp sampling frequency
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionDstRtpSamplingFrequency( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionDstRtpSamplingFrequency
*
* PURPOSE:     Get calllog session destination rtp sampling frequency
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing rtp sampling frequency
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionDstRtpSamplingFrequency( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionDstRtpAverageTxDelay
*
* PURPOSE:     Set calllog session destination rtp average tx delay
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionDstRtpAverageTxDelay( DAL_VOICE_PARMS *parms, UINT32 value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionDstRtpAverageTxDelay
*
* PURPOSE:     Get calllog session destination rtp average Tx delay
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing rtp average Tx delay
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionDstRtpAverageTxDelay( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionDstDspReceiveCodecCodec
*
* PURPOSE:     Set calllog session destination DSP receive codec
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionDstDspReceiveCodecCodec( DAL_VOICE_PARMS *parms, char *value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionDstDspReceiveCodecCodec
*
* PURPOSE:     Get calllog session destination DSP receive codec
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing DSP receive codec
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionDstDspReceiveCodecCodec( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/****************************************************************************
* FUNCTION:    dalVoice_SetCallLogSessionDstDspTransmitCodecCodec
*
* PURPOSE:     Set calllog session destination DSP transmit codec
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = new value to set
*
* OUTPUT:
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_SetCallLogSessionDstDspTransmitCodecCodec( DAL_VOICE_PARMS *parms, char *value );

/****************************************************************************
* FUNCTION:    dalVoice_GetCallLogSessionDstDspTransmitCodecCodec
*
* PURPOSE:     Get calllog session destination DSP transmit codec
*
* PARAMETERS:  parms->op[0] = vpInst
*              parms->op[1] = calllog instance number
*              parms->op[2] = session instance number
*              value = string buffer to hold value
*              length = length of value buffer
*
* OUTPUT:      value = string containing DSP transmit codec
*
* RETURNS:     CMSRET_SUCCESS - Operation success
*              other failed, check with reason code
****************************************************************************/
CmsRet dalVoice_GetCallLogSessionDstDspTransmitCodecCodec( DAL_VOICE_PARMS *parms, char *value, unsigned int length );


int dalVoice_InOutMapConsistencyCheck( DAL_VOICE_PARMS *parms );

CmsRet dalVoice_SetFxsLineTest( DAL_VOICE_PARMS *parms, char* value );
CmsRet dalVoice_SetFxsDiagTestSelector( DAL_VOICE_PARMS *parms, char* value );
CmsRet dalVoice_SetFxsDiagTestState( DAL_VOICE_PARMS *parms, char* value );
CmsRet dalVoice_SetFxsDiagTestResult( DAL_VOICE_PARMS *parms, char* value );
CmsRet dalVoice_GetFxsDiagTestSelector( DAL_VOICE_PARMS *parms, char* value, unsigned int length );
CmsRet dalVoice_GetFxsDiagTestState( DAL_VOICE_PARMS *parms, char* value, unsigned int length );
CmsRet dalVoice_GetFxsDiagTestResult( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

CmsRet dalVoice_AddSipContactUri( DAL_VOICE_PARMS *parms, int  *inst );
CmsRet dalVoice_DeleteSipContactUri( DAL_VOICE_PARMS *parms );
CmsRet dalVoice_SetSipContactUri( DAL_VOICE_PARMS *parms , char *value);
CmsRet dalVoice_GetSipContactUri( DAL_VOICE_PARMS *parms , char *value, unsigned int length);
CmsRet dalVoice_mapSipContactNumToInst ( DAL_VOICE_PARMS *parms, int *Inst );

CmsRet dalVoice_GetEServiceNwHoldTime(DAL_VOICE_PARMS *parms, char *val, unsigned int length );
CmsRet dalVoice_GetEServiceDSCPMark(DAL_VOICE_PARMS *parms, char *val, unsigned int length );
CmsRet dalVoice_GetEServiceNwHoldDisable(DAL_VOICE_PARMS *parms, char *val, unsigned int length );
CmsRet dalVoice_GetEServiceNwHoldBypass(DAL_VOICE_PARMS *parms, char *val, unsigned int length );
CmsRet dalVoice_GetEServiceAllow3WayCall(DAL_VOICE_PARMS *parms, char *val, unsigned int length );
CmsRet dalVoice_GetEServiceNoLocInfo(DAL_VOICE_PARMS *parms, char *val, unsigned int length );
CmsRet dalVoice_GetEServiceEndAllCall(DAL_VOICE_PARMS *parms, char *val, unsigned int length );
CmsRet dalVoice_GetEServiceHowlerDuration(DAL_VOICE_PARMS *parms, char *duration, unsigned int length );

CmsRet dalVoice_SetEServiceNwHoldTime(DAL_VOICE_PARMS *parms, char *val );
CmsRet dalVoice_SetEServiceDSCPMark(DAL_VOICE_PARMS *parms, char *val);
CmsRet dalVoice_SetEServiceNwHoldDisable(DAL_VOICE_PARMS *parms, char *val);
CmsRet dalVoice_SetEServiceNwHoldBypass(DAL_VOICE_PARMS *parms, char *val);
CmsRet dalVoice_SetEServiceAllow3WayCall(DAL_VOICE_PARMS *parms, char *val);
CmsRet dalVoice_SetEServiceNoLocInfo(DAL_VOICE_PARMS *parms, char *val);
CmsRet dalVoice_SetEServiceEndAllCall(DAL_VOICE_PARMS *parms, char *val);
CmsRet dalVoice_SetEServiceHowlerDuration(DAL_VOICE_PARMS *parms, char *val );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipMode
**
**  PURPOSE:        Get the SIP mode.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              netInst: parms->op[1]
**                  value   - SIP mode, "RFC3261" or "IMS"
**                  length  - value buffer length
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipMode( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetSipMode
**
**  PURPOSE:        Set the SIP mode.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              netInst: parms->op[1]
**                  value   - SIP mode, "RFC3261" or "IMS"
**
**  RETURNS:        CMSRET_SUCCESS - Write Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetSipMode( DAL_VOICE_PARMS *parms, char *value );

/*****************************************************************
**  FUNCTION:       dalVoice_GetPCSCFMaxTime
**
**  PURPOSE:        Get the max time.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              netInst: parms->op[1]
**                  length  - Buffer length
**
**  OUTPUT PARMS:   value - max time in string form
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetPCSCFMaxTime( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetPCSCFMaxTime
**
**  PURPOSE:        Set the max time.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              netInst: parms->op[1]
**                  value   - max time in string form
**
**  RETURNS:        CMSRET_SUCCESS - Write Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetPCSCFMaxTime( DAL_VOICE_PARMS *parms, char *value );

/*****************************************************************
**  FUNCTION:       dalVoice_GetPCSCFBaseTimeAllFailed
**
**  PURPOSE:        Get the base time (all failed).
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              netInst: parms->op[1]
**                  length  - Buffer length
**
**  OUTPUT PARMS:   value - base time in string form
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetPCSCFBaseTimeAllFailed( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetPCSCFBaseTimeAllFailed
**
**  PURPOSE:        Set the base time (all failed).
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              netInst: parms->op[1]
**                  value   - base time in string form
**
**  RETURNS:        CMSRET_SUCCESS - Write Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetPCSCFBaseTimeAllFailed( DAL_VOICE_PARMS *parms, char *value );

/*****************************************************************
**  FUNCTION:       dalVoice_GetPCSCFBaseTimeAllNotFailed
**
**  PURPOSE:        Get the base time (all not failed).
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              netInst: parms->op[1]
**                  length  - Buffer length
**
**  OUTPUT PARMS:   value - base time in string form
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetPCSCFBaseTimeAllNotFailed( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetPCSCFBaseTimeAllNotFailed
**
**  PURPOSE:        Set the base time (all not failed).
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              netInst: parms->op[1]
**                  value   - base time in string form
**
**  RETURNS:        CMSRET_SUCCESS - Write Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetPCSCFBaseTimeAllNotFailed( DAL_VOICE_PARMS *parms, char *value );

/*****************************************************************
**  FUNCTION:       dalVoice_GetPCSCFBaseTimeAllNotFailed
**
**  PURPOSE:        Get the failed subscription timer.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              netInst: parms->op[1]
**                  length  - Buffer length
**
**  OUTPUT PARMS:   value - base time in string form
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetTimerSubscriptionFailed( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetTimerSubscriptionFailed
**
**  PURPOSE:        Set the failed subscription timer.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              netInst: parms->op[1]
**                  value   - failed subscription timer in string form
**
**  RETURNS:        CMSRET_SUCCESS - Write Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetTimerSubscriptionFailed( DAL_VOICE_PARMS *parms, char *value );

/*****************************************************************
**  FUNCTION:       dalVoice_SetSipClientNetworkAssocIdx
**
**  PURPOSE:        Set sip client point to the network instance
**
**  INPUT PARMS:    op[0] - voice service instance
**                  op[1] - SIP client number
**                  op[2] - SIP network number
**
**  OUTPUT PARMS:   None
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetSipClientNetworkAssocIdx( DAL_VOICE_PARMS *parms,  char *network);

/*****************************************************************
**  FUNCTION:       dalVoice_SetSipNetworkVoipProfileAssocIdx
**
**  PURPOSE:        set network profile assocation to the voip profile number
**
**  INPUT PARMS:    op[0] - voice service instance
**                  op[1] - network instance
**                  profile - voip profile number ( 0 based )
**
**  OUTPUT PARMS:   None
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetSipNetworkVoipProfileAssocIdx( DAL_VOICE_PARMS *parms, char *profile );

/*****************************************************************
**  FUNCTION:       dalVoice_GetCallFwdRingReminder
**
**  PURPOSE:        Get the CallFwdRingReminder.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  length  - Buffer length
**
**  OUTPUT PARMS:   value - CallFwdRingReminder
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetCallFwdRingReminder( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetCallFwdRingReminder
**
**  PURPOSE:        Set the CallFwdRingReminder.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  value   - CallFwdRingReminder
**
**  RETURNS:        CMSRET_SUCCESS - Write Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetCallFwdRingReminder( DAL_VOICE_PARMS *parms, char *value );

/*****************************************************************
**  FUNCTION:       dalVoice_GetCallFwdSubDuration
**
**  PURPOSE:        Get the CallFwdSubDuration.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  length  - Buffer length
**
**  OUTPUT PARMS:   value - CallFwdSubDuration
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetCallFwdSubDuration( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetCallFwdSubDuration
**
**  PURPOSE:        Set the CallFwdSubDuration.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  value   - CallFwdSubDuration
**
**  RETURNS:        CMSRET_SUCCESS - Write Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetCallFwdSubDuration( DAL_VOICE_PARMS *parms, char *value );

/*****************************************************************
**  FUNCTION:       dalVoice_GetCallFwdAUID
**
**  PURPOSE:        Get the CallFwdAUID.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  length  - Buffer length
**
**  OUTPUT PARMS:   value - CallFwdAUID
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetCallFwdAUID( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetCallFwdAUID
**
**  PURPOSE:        Set the CallFwdAUID.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  value   - CallFwdAUID
**
**  RETURNS:        CMSRET_SUCCESS - Write Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetCallFwdAUID( DAL_VOICE_PARMS *parms, char *value );

/*****************************************************************
**  FUNCTION:       dalVoice_GetCallFwdSpDialTone
**
**  PURPOSE:        Get the CallFwdSpDialTone.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  length  - Buffer length
**
**  OUTPUT PARMS:   value - CallFwdSpDialTone
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetCallFwdSpDialTone( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetCallFwdSpDialTone
**
**  PURPOSE:        Set the CallFwdSpDialTone.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  value   - CallFwdSpDialTone
**
**  RETURNS:        CMSRET_SUCCESS - Write Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetCallFwdSpDialTone( DAL_VOICE_PARMS *parms, char *value );

/*****************************************************************
**  FUNCTION:       dalVoice_GetCXNtfyTimeout
**
**  PURPOSE:        Get the CXNtfyTimeout.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  length  - Buffer length
**
**  OUTPUT PARMS:   value - CXNtfyTimeout
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetCXNtfyTimeout( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetCXNtfyTimeout
**
**  PURPOSE:        Set the CXNtfyTimeout.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  value   - CXNtfyTimeout
**
**  RETURNS:        CMSRET_SUCCESS - Write Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetCXNtfyTimeout( DAL_VOICE_PARMS *parms, char *value );

/*****************************************************************
**  FUNCTION:       dalVoice_CXInDialogRefer
**
**  PURPOSE:        Get the CXInDialogRefer.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  length  - Buffer length
**
**  OUTPUT PARMS:   value - CXInDialogRefer
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetCXInDialogRefer( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetCXInDialogRefer
**
**  PURPOSE:        Set the CXInDialogRefer.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  value   - CXInDialogRefer
**
**  RETURNS:        CMSRET_SUCCESS - Write Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetCXInDialogRefer( DAL_VOICE_PARMS *parms, char *value );

/*****************************************************************
**  FUNCTION:       dalVoice_GetCXIncomingOnly
**
**  PURPOSE:        Get the CXIncomingOnly.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  length  - Buffer length
**
**  OUTPUT PARMS:   value - CXIncomingOnly
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetCXIncomingOnly( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetCXIncomingOnly
**
**  PURPOSE:        Set the CXIncomingOnly.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  value   - CXIncomingOnly
**
**  RETURNS:        CMSRET_SUCCESS - Write Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetCXIncomingOnly( DAL_VOICE_PARMS *parms, char *value );

/*****************************************************************
**  FUNCTION:       dalVoice_GetCIDDisDefCountry
**
**  PURPOSE:        Get the CIDDisDefCountry.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  length  - Buffer length
**
**  OUTPUT PARMS:   value - CIDDisDefCountry
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetCIDDisDefCountry( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetCIDDisDefCountry
**
**  PURPOSE:        Set the CIDDisDefCountry.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  value   - CIDDisDefCountry
**
**  RETURNS:        CMSRET_SUCCESS - Write Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetCIDDisDefCountry( DAL_VOICE_PARMS *parms, char *value );

/*****************************************************************
**  FUNCTION:       dalVoice_GetCIDDisCIDCWActStat
**
**  PURPOSE:        Get the CIDDisCIDCWActStat.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  length  - Buffer length
**
**  OUTPUT PARMS:   value - CIDDisCIDCWActStat
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetCIDDisCIDCWActStat( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetCIDDisCIDCWActStat
**
**  PURPOSE:        Set the CIDDisCIDCWActStat.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  value   - CIDDisCIDCWActStat
**
**  RETURNS:        CMSRET_SUCCESS - Write Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetCIDDisCIDCWActStat( DAL_VOICE_PARMS *parms, char *value );

/*****************************************************************
**  FUNCTION:       dalVoice_GetCIDDisDSTInfo
**
**  PURPOSE:        Get the CIDDisDSTInfo.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  length  - Buffer length
**
**  OUTPUT PARMS:   value - CIDDisDSTInfo
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetCIDDisDSTInfo( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetCIDDisDSTInfo
**
**  PURPOSE:        Set the CIDDisDSTInfo.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  value   - CIDDisDSTInfo
**
**  RETURNS:        CMSRET_SUCCESS - Write Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetCIDDisDSTInfo( DAL_VOICE_PARMS *parms, char *value );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNfBCallByeDelay
**
**  PURPOSE:        Get the NfBCallByeDelay.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  length  - Buffer length
**
**  OUTPUT PARMS:   value - NfBCallByeDelay
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNfBCallByeDelay( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetNfBCallByeDelay
**
**  PURPOSE:        Set the NfBCallByeDelay.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  value   - NfBCallByeDelay
**
**  RETURNS:        CMSRET_SUCCESS - Write Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetNfBCallByeDelay( DAL_VOICE_PARMS *parms, char *value );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNfBCallOrigDTTimer
**
**  PURPOSE:        Get the NfBCallOrigDTTimer.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  length  - Buffer length
**
**  OUTPUT PARMS:   value - NfBCallOrigDTTimer
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNfBCallOrigDTTimer( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetNfBCallOrigDTTimer
**
**  PURPOSE:        Set the NfBCallOrigDTTimer.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  value   - NfBCallOrigDTTimer
**
**  RETURNS:        CMSRET_SUCCESS - Write Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetNfBCallOrigDTTimer( DAL_VOICE_PARMS *parms, char *value );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNfBCallTermOHErrSig
**
**  PURPOSE:        Get the NfBCallTermOHErrSig.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  length  - Buffer length
**
**  OUTPUT PARMS:   value - NfBCallTermOHErrSig
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNfBCallTermOHErrSig( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetNfBCallTermOHErrSig
**
**  PURPOSE:        Set the NfBCallTermOHErrSig.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  value   - NfBCallTermOHErrSig
**
**  RETURNS:        CMSRET_SUCCESS - Write Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetNfBCallTermOHErrSig( DAL_VOICE_PARMS *parms, char *value );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNfBCallTermErrSigTimer
**
**  PURPOSE:        Get the NfBCallTermErrSigTimer.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  length  - Buffer length
**
**  OUTPUT PARMS:   value - NfBCallTermErrSigTimer
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNfBCallTermErrSigTimer( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetNfBCallTermErrSigTimer
**
**  PURPOSE:        Set the NfBCallTermErrSigTimer.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  value   - NfBCallTermErrSigTimer
**
**  RETURNS:        CMSRET_SUCCESS - Write Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetNfBCallTermErrSigTimer( DAL_VOICE_PARMS *parms, char *value );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNfBCallPermSeqTone1
**
**  PURPOSE:        Get the NfBCallPermSeqTone1.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  length  - Buffer length
**
**  OUTPUT PARMS:   value - NfBCallPermSeqTone1
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNfBCallPermSeqTone1( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetNfBCallPermSeqTone1
**
**  PURPOSE:        Set the NfBCallPermSeqTone1.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  value   - NfBCallPermSeqTone1
**
**  RETURNS:        CMSRET_SUCCESS - Write Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetNfBCallPermSeqTone1( DAL_VOICE_PARMS *parms, char *value );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNfBCallPermSeqTimer1
**
**  PURPOSE:        Get the NfBCallPermSeqTimer1.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  length  - Buffer length
**
**  OUTPUT PARMS:   value - NfBCallPermSeqTimer1
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNfBCallPermSeqTimer1( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetNfBCallPermSeqTimer1
**
**  PURPOSE:        Set the NfBCallPermSeqTimer1.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  value   - NfBCallPermSeqTimer1
**
**  RETURNS:        CMSRET_SUCCESS - Write Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetNfBCallPermSeqTimer1( DAL_VOICE_PARMS *parms, char *value );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNfBCallPermSeqTone2
**
**  PURPOSE:        Get the NfBCallPermSeqTone2.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  length  - Buffer length
**
**  OUTPUT PARMS:   value - NfBCallPermSeqTone2
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNfBCallPermSeqTone2( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetNfBCallPermSeqTone2
**
**  PURPOSE:        Set the NfBCallPermSeqTone2.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  value   - NfBCallPermSeqTone2
**
**  RETURNS:        CMSRET_SUCCESS - Write Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetNfBCallPermSeqTone2( DAL_VOICE_PARMS *parms, char *value );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNfBCallPermSeqTimer2
**
**  PURPOSE:        Get the NfBCallPermSeqTimer2.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  length  - Buffer length
**
**  OUTPUT PARMS:   value - NfBCallPermSeqTimer2
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNfBCallPermSeqTimer2( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetNfBCallPermSeqTimer2
**
**  PURPOSE:        Set the NfBCallPermSeqTimer2.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  value   - NfBCallPermSeqTimer2
**
**  RETURNS:        CMSRET_SUCCESS - Write Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetNfBCallPermSeqTimer2( DAL_VOICE_PARMS *parms, char *value );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNfBCallPermSeqTone3
**
**  PURPOSE:        Get the NfBCallPermSeqTone3.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  length  - Buffer length
**
**  OUTPUT PARMS:   value - NfBCallPermSeqTone3
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNfBCallPermSeqTone3( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetNfBCallPermSeqTone3
**
**  PURPOSE:        Set the NfBCallPermSeqTone3.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  value   - NfBCallPermSeqTone3
**
**  RETURNS:        CMSRET_SUCCESS - Write Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetNfBCallPermSeqTone3( DAL_VOICE_PARMS *parms, char *value );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNfBCallPermSeqTimer3
**
**  PURPOSE:        Get the NfBCallPermSeqTimer3.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  length  - Buffer length
**
**  OUTPUT PARMS:   value - NfBCallPermSeqTimer3
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNfBCallPermSeqTimer3( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetNfBCallPermSeqTimer3
**
**  PURPOSE:        Set the NfBCallPermSeqTimer3.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  value   - NfBCallPermSeqTimer3
**
**  RETURNS:        CMSRET_SUCCESS - Write Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetNfBCallPermSeqTimer3( DAL_VOICE_PARMS *parms, char *value );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNfBCallPermSeqTone4
**
**  PURPOSE:        Get the NfBCallPermSeqTone4.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  length  - Buffer length
**
**  OUTPUT PARMS:   value - NfBCallPermSeqTone4
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNfBCallPermSeqTone4( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetNfBCallPermSeqTone4
**
**  PURPOSE:        Set the NfBCallPermSeqTone4.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  value   - NfBCallPermSeqTone4
**
**  RETURNS:        CMSRET_SUCCESS - Write Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetNfBCallPermSeqTone4( DAL_VOICE_PARMS *parms, char *value );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNfBCallPermSeqTimer4
**
**  PURPOSE:        Get the NfBCallPermSeqTimer4.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  length  - Buffer length
**
**  OUTPUT PARMS:   value - NfBCallPermSeqTimer4
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNfBCallPermSeqTimer4( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetNfBCallPermSeqTimer4
**
**  PURPOSE:        Set the NfBCallPermSeqTimer4.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  value   - NfBCallPermSeqTimer4
**
**  RETURNS:        CMSRET_SUCCESS - Write Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetNfBCallPermSeqTimer4( DAL_VOICE_PARMS *parms, char *value );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNfBCallLORTimer
**
**  PURPOSE:        Get the NfBCallLORTimer.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  length  - Buffer length
**
**  OUTPUT PARMS:   value - NfBCallLORTimer
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNfBCallLORTimer( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetNfBCallLORTimer
**
**  PURPOSE:        Set the NfBCallLORTimer.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  value   - NfBCallLORTimer
**
**  RETURNS:        CMSRET_SUCCESS - Write Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetNfBCallLORTimer( DAL_VOICE_PARMS *parms, char *value );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNfBCallOrigModLongIntDig
**
**  PURPOSE:        Get the NfBCallOrigModLongIntDig.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  length  - Buffer length
**
**  OUTPUT PARMS:   value - NfBCallOrigModLongIntDig
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNfBCallOrigModLongIntDig( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetNfBCallOrigModLongIntDig
**
**  PURPOSE:        Set the NfBCallOrigModLongIntDig.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  value   - NfBCallOrigModLongIntDig
**
**  RETURNS:        CMSRET_SUCCESS - Write Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetNfBCallOrigModLongIntDig( DAL_VOICE_PARMS *parms, char *value );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNfBCallOverrideNotifyRejected
**
**  PURPOSE:        Get the NfBCallOverrideNotifyRejected.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  length  - Buffer length
**
**  OUTPUT PARMS:   value - NfBCallOverrideNotifyRejected
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNfBCallOverrideNotifyRejected( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetNfBCallOverrideNotifyRejected
**
**  PURPOSE:        Set the NfBCallOverrideNotifyRejected.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  value   - NfBCallOverrideNotifyRejected
**
**  RETURNS:        CMSRET_SUCCESS - Write Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetNfBCallOverrideNotifyRejected( DAL_VOICE_PARMS *parms, char *value );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNfMWISubDuration
**
**  PURPOSE:        Get the NfMWISubDuration.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  length  - Buffer length
**
**  OUTPUT PARMS:   value - NfMWISubDuration
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNfMWISubDuration( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetNfMWISubDuration
**
**  PURPOSE:        Set the NfMWISubDuration.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  value   - NfMWISubDuration
**
**  RETURNS:        CMSRET_SUCCESS - Write Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetNfMWISubDuration( DAL_VOICE_PARMS *parms, char *value );

/*****************************************************************
**  FUNCTION:       dalVoice_GetNoAnsTODuration
**
**  PURPOSE:        Get the NoAnsTODuration.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  length  - Buffer length
**
**  OUTPUT PARMS:   value - NoAnsTODuration
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetNoAnsTODuration( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetNoAnsTODuration
**
**  PURPOSE:        Set the NoAnsTODuration.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  value   - NoAnsTODuration
**
**  RETURNS:        CMSRET_SUCCESS - Write Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetNoAnsTODuration( DAL_VOICE_PARMS *parms, char *value );

/*****************************************************************
**  FUNCTION:       dalVoice_GetWarmLineOffhookTimer
**
**  PURPOSE:        Get the WarmLineOffhookTimer.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  length  - Buffer length
**
**  OUTPUT PARMS:   value - WarmLineOffhookTimer
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetWarmLineOffhookTimer( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetWarmLineOffhookTimer
**
**  PURPOSE:        Set the WarmLineOffhookTimer.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  value   - WarmLineOffhookTimer
**
**  RETURNS:        CMSRET_SUCCESS - Write Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetWarmLineOffhookTimer( DAL_VOICE_PARMS *parms, char *value );

/*****************************************************************
**  FUNCTION:       dalVoice_GetCIDDelStatus
**
**  PURPOSE:        Get the CIDDelStatus.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  length  - Buffer length
**
**  OUTPUT PARMS:   value - CIDDelStatus
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetCIDDelStatus( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetCIDDelStatus
**
**  PURPOSE:        Set the CIDDelStatus.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  value   - CIDDelStatus
**
**  RETURNS:        CMSRET_SUCCESS - Write Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetCIDDelStatus( DAL_VOICE_PARMS *parms, char *value );

/*****************************************************************
**  FUNCTION:       dalVoice_GetCIDCBlkStatus
**
**  PURPOSE:        Get the CIDCBlkStatus.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  length  - Buffer length
**
**  OUTPUT PARMS:   value - CIDCBlkStatus
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetCIDCBlkStatus( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetCIDCBlkStatus
**
**  PURPOSE:        Set the CIDCBlkStatus.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              featInst: parms->op[1]
**                  value   - CIDCBlkStatus
**
**  RETURNS:        CMSRET_SUCCESS - Write Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetCIDCBlkStatus( DAL_VOICE_PARMS *parms, char *value );

/*****************************************************************
**  FUNCTION:       dalVoice_GetKeepAliveSetting
**
**  PURPOSE:        Get the KeepAliveSetting.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              profInst: parms->op[1]
**                  length  - Buffer length
**
**  OUTPUT PARMS:   value - KeepAliveSetting
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetKeepAliveSetting( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetKeepAliveSetting
**
**  PURPOSE:        Set the KeepAliveSetting.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              profInst: parms->op[1]
**                  value   - KeepAliveSetting
**
**  RETURNS:        CMSRET_SUCCESS - Write Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetKeepAliveSetting( DAL_VOICE_PARMS *parms, char *value );

/*****************************************************************
**  FUNCTION:       dalVoice_GetHeldMediaEnabled
**
**  PURPOSE:        Get the HeldMediaEnabled.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              profInst: parms->op[1]
**                  length  - Buffer length
**
**  OUTPUT PARMS:   value - HeldMediaEnabled
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetHeldMediaEnabled( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetHeldMediaEnabled
**
**  PURPOSE:        Set the HeldMediaEnabled.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              profInst: parms->op[1]
**                  value   - HeldMediaEnabled
**
**  RETURNS:        CMSRET_SUCCESS - Write Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetHeldMediaEnabled( DAL_VOICE_PARMS *parms, char *value );

/*****************************************************************
**  FUNCTION:       dalVoice_GetEndPntDtmfMinPlayout
**
**  PURPOSE:        Get the EndPntDtmfMinPlayout.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              fxsInst: parms->op[1]
**                  length  - Buffer length
**
**  OUTPUT PARMS:   value - EndPntDtmfMinPlayout
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetEndPntDtmfMinPlayout( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetEndPntDtmfMinPlayout
**
**  PURPOSE:        Set the EndPntDtmfMinPlayout.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              fxsInst: parms->op[1]
**                  value   - EndPntDtmfMinPlayout
**
**  RETURNS:        CMSRET_SUCCESS - Write Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetEndPntDtmfMinPlayout( DAL_VOICE_PARMS *parms, char *value );

/*****************************************************************
**  FUNCTION:       dalVoice_GetEndPntFaxDetection
**
**  PURPOSE:        Get the EndPntFaxDetection.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              fxsInst: parms->op[1]
**                  length  - Buffer length
**
**  OUTPUT PARMS:   value - EndPntFaxDetection
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetEndPntFaxDetection( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetEndPntFaxDetection
**
**  PURPOSE:        Set the EndPntFaxDetection.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              fxsInst: parms->op[1]
**                  value   - EndPntFaxDetection
**
**  RETURNS:        CMSRET_SUCCESS - Write Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetEndPntFaxDetection( DAL_VOICE_PARMS *parms, char *value );

/*****************************************************************
**  FUNCTION:       dalVoice_GetEndPntQosPreconditions
**
**  PURPOSE:        Get the EndPntQosPreconditions.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              fxsInst: parms->op[1]
**                  length  - Buffer length
**
**  OUTPUT PARMS:   value - EndPntQosPreconditions
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetEndPntQosPreconditions( DAL_VOICE_PARMS *parms, char *value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_SetEndPntQosPreconditions
**
**  PURPOSE:        Set the EndPntQosPreconditions.
**
**  PARAMETERS:     parms   - Parameter index - vpInst:  parms->op[0]
**                                              fxsInst: parms->op[1]
**                  value   - EndPntQosPreconditions
**
**  RETURNS:        CMSRET_SUCCESS - Write Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_SetEndPntQosPreconditions( DAL_VOICE_PARMS *parms, char *value );


#endif /* __DAL_VOICE_V2_H__ */

