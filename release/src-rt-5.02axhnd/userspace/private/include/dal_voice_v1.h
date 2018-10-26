/*
*
*  Filename: dal_voice.h
*
****************************************************************************
*  Description:
*
*
****************************************************************************/

#ifndef __DAL_VOICE_V1_H__
#define __DAL_VOICE_V1_H__

/* Map MDM stats object structure to DAL, and map this to a VOICE structure for use outside of DAL */
typedef VoiceLineStatsObject DAL_VOICE_CALL_STATS_BLK;
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
**  PUROPOSE:
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
CmsRet dalVoice_GetVoiceFxsPhyInterfaceList(DAL_VOICE_PARMS *parms, char *list, unsigned int length );
CmsRet dalVoice_GetVoiceNoSigPhyInterfaceList(DAL_VOICE_PARMS *parms, char *list, unsigned int length );

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
**  PUROPOSE:
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
* Returns      : CMSRET_SUCCESS when successfule.
****************************************************************************/
CmsRet dalVoice_mapAcntNumToLineInst ( int vpInst, int acntNum, int * lineInst );

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
* Parameters   : country (INOUT) - locale(Alpha3), on success exec contains Alpha2 locale
*                found (OUT)   - true indicates locale is supported
* Returns      : CMSRET_SUCCESS when successfule.
****************************************************************************/
CmsRet dalVoice_mapCountryCode3To2 ( char * country, UBOOL8 * found, unsigned int length );

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
* Function Name: dalVoice_SetVlCFCallId
* Description  : Enable or disable incoming caller ID number by the endpoint
*                VoiceProfile.{i}.Line{i}.CallingFeatures.CallerIDEnable = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlCFCallId ( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetVlCFCallIdName
* Description  : Enable or disable incoming call ID name by the endpoint
*                VoiceProfile.{i}.Line{i}.CallingFeatures.CallerIDNameEnable = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlCFCallIdName ( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetVlCFCallWaiting
* Description  : Enable or disable call waiting by the endpoint
*                VoiceProfile.{i}.Line{i}.CallingFeatures.CallWaitingEnable = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlCFCallWaiting ( DAL_VOICE_PARMS *parms, char *value );

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
* Function Name: dalVoice_SetVlCLSilenceSuppression
* Description  : CLI wrapper for SetVlCodecListSilenceSuppression()
*                Indicates support for silence suppression for this codec.
*                VoiceProfile.{i}.Line{i}.Codec.List.{i}.SilenceSuppression = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlCLSilenceSuppression( DAL_VOICE_PARMS *parms, char *value );

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
* Function Name: dalVoice_SetSipTimerB
* Description  : set SIP protocol B timer value
*                VoiceProfile.{i}.Sip.TimerB = new value
*
* Parameters   : parms->op[0] = vpInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSipTimerB( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetSipTimerF
* Description  : set SIP protocol Timer F value
*                VoiceProfile.{i}.Sip.TimerF = new value
*
* Parameters   : parms->op[0] = vpInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSipTimerF( DAL_VOICE_PARMS *parms, char *value );


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
* Function Name: dalVoice_SetVlCLSilenceSuppression
* Description  : CLI wrapper for SetVlCLSilenceSuppression()
*                Indicates support for silence suppression for this codec.
*                VoiceProfile.{i}.Line{i}.Codec.List.{i}.SilenceSuppression = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetVlCLSilenceSuppression( DAL_VOICE_PARMS *parms, char *value );

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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  FUNCTION:       dalVoice_GetVlSipAuthPassword
**
**  PUROPOSE:
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
**  FUNCTION:       dalVoice_GetVlSipURI
**
**  PUROPOSE:
**
**  INPUT PARMS:    vpInst    - parms->op[0]
**                  lineInst  - parms->op[1]
**
**
**  OUTPUT PARMS:   Account Id/extension
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVlSipURI(DAL_VOICE_PARMS *parms, char *userId, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipRegistrarServerPort
**
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  FUNCTION:       dalVoice_GetVlSipAuthUserName
**
**  PUROPOSE:
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
**  FUNCTION:       dalVoice_GetSipRegisterExpires
**
**  PUROPOSE:
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
**  FUNCTION:       dalVoice_GetSipTimerB
**
**  PUROPOSE:
**
**  INPUT PARMS:    vpInst  - parms->op[0]
**
**  OUTPUT PARMS:   tmrB - timer B value
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipTimerB(DAL_VOICE_PARMS *parms, char *tmrB, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipRegisterExpires
**
**  PUROPOSE:
**
**  INPUT PARMS:    vpInst  - parms->op[0]
**
**  OUTPUT PARMS:   tmrF - timer F value
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipTimerF(DAL_VOICE_PARMS *parms, char *tmrF, unsigned int length );


/*****************************************************************
**  FUNCTION:       dalVoice_GetSipRegisterRetryInterval
**
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  FUNCTION:       dalVoice_GetSipBackToPrimOption
**
**  PUROPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0]
**
**  OUTPUT PARMS:   Enum value of SIP failover back-to-primary option
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet  dalVoice_GetSipBackToPrimOption(DAL_VOICE_PARMS *parms, char *failover, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipBackToPrimOptionString
**
**  PUROPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0];
**
**  OUTPUT PARMS:   String value of SIP failover back-to-primary option
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipBackToPrimOptionString(DAL_VOICE_PARMS *parms, char *failover, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipDSCPMark
**
**  PUROPOSE:
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
**  FUNCTION:       dalVoice_GetVlCFFeatureEnabled
**
**  PUROPOSE:
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
**  FUNCTION:       dalVoice_GetVlCFFeatureAction
**
**  PUROPOSE:
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
CmsRet dalVoice_GetVlCFFeatureAction(DAL_VOICE_PARMS *parms, char *getVal, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetVlCFFeatureStarted
**
**  PUROPOSE:
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

CmsRet dalVoice_GetVlCFCallId( DAL_VOICE_PARMS *parms, char *getVal, unsigned int length );
CmsRet dalVoice_GetVlCFCallIdName( DAL_VOICE_PARMS *parms, char *getVal, unsigned int length );
CmsRet dalVoice_GetVlCFCallWaiting( DAL_VOICE_PARMS *parms, char *getVal, unsigned int length );
CmsRet dalVoice_GetVlCFCallBarring( DAL_VOICE_PARMS *parms, char *getVal, unsigned int length );
CmsRet dalVoice_GetVlCFAnonCallBlck( DAL_VOICE_PARMS *parms, char* getVal, unsigned int length );
CmsRet dalVoice_GetVlCFAnonymousCalling( DAL_VOICE_PARMS *parms, char *getVal, unsigned int length );
CmsRet dalVoice_GetVlCFDoNotDisturb( DAL_VOICE_PARMS *parms, char *getVal, unsigned int length );
CmsRet dalVoice_GetVlCFWarmLine( DAL_VOICE_PARMS *parms, char *getVal, unsigned int length );


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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  FUNCTION:       dalVoice_GetSipMusicServer
**
**  PUROPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0]
**
**  OUTPUT PARMS:   musicServer - Music server address
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipMusicServer(DAL_VOICE_PARMS *parms, char *musicServer, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipMusicServerPort
**
**  PUROPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0]
**
**  OUTPUT PARMS:   port - Music server port
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipMusicServerPort(DAL_VOICE_PARMS *parms, char *port, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipConferencingURI
**
**  PUROPOSE:
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
**  PUROPOSE:
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
**  FUNCTION:       dalVoice_GetSipSecOutboundProxyAddr
**
**  PUROPOSE:
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
**  PUROPOSE:
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
**  FUNCTION:       dalVoice_GetSipSecDomainName
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0]
**
**  OUTPUT PARMS:   secDomainName - Secondary SIP domain name
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipSecDomainName(DAL_VOICE_PARMS *parms, char *secDomainName, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipSecProxyAddr
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0]
**
**  OUTPUT PARMS:   secProxyAddr - Secondary SIP proxy address
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipSecProxyAddr(DAL_VOICE_PARMS *parms, char *secProxyAddr, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipSecProxyPort
**
**  PURPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0]
**
**  OUTPUT PARMS:   port - Secondary SIP proxy port
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipSecProxyPort(DAL_VOICE_PARMS *parms, char *port, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipSecRegistrarAddr
**
**  PURPOSE:
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
**  PURPOSE:
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
**  FUNCTION:       dalVoice_GetSipFailoverEnable
**
**  PUROPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0]
**
**  OUTPUT PARMS:   failover - SIP failover enable
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipFailoverEnable(DAL_VOICE_PARMS *parms, char *failover, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipOptionsEnable
**
**  PUROPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0]
**
**  OUTPUT PARMS:   sipOptions - SIP OPTIONS ping enable
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetSipOptionsEnable(DAL_VOICE_PARMS *parms, char *sipOptions, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSipToTagMatching
**
**  PUROPOSE:
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
**  FUNCTION:       dalVoice_GetVlCFCallBarringMode
**
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:
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
* Function Name: dalVoice_SetSipSecDomainName
* Description  : set SIP secondary domain name
*                VoiceProfile.{i}.SIP.X_BROADCOM_COM_SecondaryDomainName = new value
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
* Function Name: dalVoice_SetSipFailoverEnable
* Description  : set value of SIP failover enable flag
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
* Function Name: dalVoice_SetSipToTagMatching
* Description  : set value of SIP to tag matching
*                VoiceProfile.{i}.X_BROADCOM_COM_ToTagMatching = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSipToTagMatching( DAL_VOICE_PARMS *parms, char *value );

/***************************************************************************
* Function Name: dalVoice_SetSipBackToPrimOption
* Description  : set value of SIP to tag matching
*                VoiceProfile.{i}.SIP.X_BROADCOM_COM_BackToPrimMode = new value
*
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetSipBackToPrimOption( DAL_VOICE_PARMS *parms, char *value );

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
* Parameters   : parms->op[0] = vpInst, parms->op[1] = lineInst, value = value to be set
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_SetHookFlashMethod ( DAL_VOICE_PARMS *parms, char *value );

/*****************************************************************
**  FUNCTION:       dalVoice_GetDTMFMethodIntValue
**
**  PUROPOSE:
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
**  PUROPOSE:
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

CmsRet dalVoice_GetMaxPrefCodecs( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetVlCLCodecList
**
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PURPOSE:		Get SRTP usage option )mandatory, optional or disabled)
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
**  PURPOSE:		Get SRTP usage option (mandatory, optional or disabled)
**                    in enum form
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
**  PUROPOSE:       Track Protocol used to Manage Voice
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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:       Stores the specified bound IP address in MDM.
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
**  PUROPOSE:       Set the enable or disable flag for voice DNS
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
**  PUROPOSE:       Set IP address of the primary voice DNS server
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
**  PUROPOSE:       Set IP address of the secondary voice DNS server
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

/*****************************************************************
**  FUNCTION:       dalVoice_GetHookFlashMethod
**
**  PUROPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0]
**
**  OUTPUT PARMS:   method - Hook flash method
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetHookFlashMethod(DAL_VOICE_PARMS *parms, char *method, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetHookFlashMethodString
**
**  PUROPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0]
**
**  OUTPUT PARMS:   method - Hook flash method
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetHookFlashMethodString(DAL_VOICE_PARMS *parms, char *method, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetSTUNServerPort
**
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  FUNCTION:       dalVoice_GetVlStats
**
**  PUROPOSE:
**
**  INPUT PARMS:    vpInst    - parms->op[0]
**                  lineInst - parms->op[1]
**
**  OUTPUT PARMS:   DAL_VOICE_CALL_STATS_BLK stats block
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVlStats(DAL_VOICE_PARMS *parms, DAL_VOICE_CALL_STATS_BLK *statsBlk );

/*****************************************************************
**  FUNCTION:       dalVoice_GetVlEnable
**
**  PUROPOSE:
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
**  FUNCTION:       dalVoice_GetVlVoipStatus
**
**  PUROPOSE:       Get VOIP service status.
**
**  INPUT PARMS:    vpInst    - parms->op[0]
**                  lineInst  - parms->op[1]
**
**  OUTPUT PARMS:   lineStatus - String of voip service status
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVlVoipStatus(DAL_VOICE_PARMS *parms, char *lineStatus, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetVlDisable
**
**  PUROPOSE:
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
**  PUROPOSE:
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
**  FUNCTION:       dalVoice_GetVlCLPacketizationPeriod
**
**  PUROPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0];
**                  lineInst  - parms->op[1]
**
**  OUTPUT PARMS:   ptime - Packetization period
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVlCLPacketizationPeriod(DAL_VOICE_PARMS *parms, char *ptime, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetVlCLSilenceSuppression
**
**  PUROPOSE:
**
**  INPUT PARMS:    vpInst - parms->op[0];
**                  lineInst  - parms->op[1]
**
**  OUTPUT PARMS:   vad - Silence suppression
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVlCLSilenceSuppression(DAL_VOICE_PARMS *parms, char *vad, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetDigitMap
**
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:       Return value of
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
**  PUROPOSE:       Return value of
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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  FUNCTION:       dalVoice_GetVlVPTransmitGain
**
**  PUROPOSE:
**
**  INPUT PARMS:    vpInst    - parms->op[0]
**                  lineInst  - parms->op[1]
**
**  OUTPUT PARMS:   tx - Tx gain
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVlVPTransmitGain(DAL_VOICE_PARMS *parms, char *tx, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetVlVPRecieveGain
**
**  PUROPOSE:
**
**  INPUT PARMS:    vpInst    - parms->op[0]
**                  lineInst  - parms->op[1]
**
**  OUTPUT PARMS:   rx - Rx gain
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetVlVPRecieveGain(DAL_VOICE_PARMS *parms, char *rx, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetVlCFMWIEnable
**
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:       Get EptApp Enable flag
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
* Function Name: dalVoice_GetModuleLoggingLevel
* Description  : Gets the specific voice module's logging level
*
* Parameters   : none
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetModuleLoggingLevel( DAL_VOICE_PARMS *parms, char* modName, char * getVal, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetModuleLoggingLevels
* Description  : Gets all the voice module logging levels
*
* Parameters   : none
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetModuleLoggingLevels( DAL_VOICE_PARMS *parms, char * getVal, unsigned int length );

/***************************************************************************
* Function Name: dalVoice_GetManagementProtocol
* Description  : Gets the Protocol used to manage the Voice Service
*
* Parameters   : none
* Returns      : CMSRET_SUCCESS when successful.
****************************************************************************/
CmsRet dalVoice_GetManagementProtocol( DAL_VOICE_PARMS *parms, void * getVal, unsigned int length );

#ifdef SIPLOAD
/*****************************************************************
**  FUNCTION:       dalVoice_GetCCTKTraceLevel
**
**  PUROPOSE:
**
**  INPUT PARMS:    None
**
**  OUTPUT PARMS:   cctk trace level
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetCCTKTraceLevel( DAL_VOICE_PARMS *parms, void * getVal, unsigned int length);

/*****************************************************************
**  FUNCTION:       dalVoice_GetCCTKTraceGroup
**
**  PUROPOSE:
**
**  INPUT PARMS:    None
**
**  OUTPUT PARMS:   cctk trace group
**
**  RETURNS:        CMSRET_SUCCESS - Read Success
**                  other failed, check with reason code
**
*******************************************************************/
CmsRet dalVoice_GetCCTKTraceGroup( DAL_VOICE_PARMS *parms, void * getVal, unsigned int length);
#endif /* SIPLOAD */

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
* RETURNS:     Supported SRTP options
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
CmsRet dalVoice_GetNetworkIntfList_igd( DAL_VOICE_PARMS *parms, char* intfList, unsigned int length );
CmsRet dalVoice_GetNetworkIntfList_dev2(DAL_VOICE_PARMS *parms, char* intfList, unsigned int length );

/*****************************************************************
**  FUNCTION:       dalVoice_GetBoundIfName
**
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:       Check if voice DNS is enabled
*                   (X_BROADCOM_COM_VoiceDnsEnable)
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
**  PUROPOSE:       Get IP address of the primary voice DNS server
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
**  PUROPOSE:       Get IP address of the secondary voice DNS server
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
**  PUROPOSE:       Get NTR Auto Mode on (1) or off (0)
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
**  PUROPOSE:       Get Offset for NTR Manual Mode
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
**  PUROPOSE:       Get Offset in PPM for NTR Manual Mode
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
**  PUROPOSE:       Get feedback offset in PLL steps for NTR manual mode
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
**  PUROPOSE:       Get NTR Debug flag
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
**  PUROPOSE:       Get current PCM-MIPS tally for NTR task
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
**  PUROPOSE:       Get previous PCM-MIPS tally for NTR task
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
**  PUROPOSE:       Get current PCM-NTR tally for NTR task
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
**  PUROPOSE:       Get previous PCM-NTR tally for NTR task
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
**  PUROPOSE:       Get current DSL-MIPS tally for NTR task
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
**  PUROPOSE:       Get previous DSL-MIPS tally for NTR task
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
**  PUROPOSE:       Get current DSL-NTR tally for NTR task
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
**  PUROPOSE:       Get previous DSL-NTR tally for NTR task
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
**  PUROPOSE:       Get previous SampleRate for NTR task
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
**  PUROPOSE:       Get previous PllBandwidth for NTR task
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
**  PUROPOSE:       Get previous DampingFactor for NTR task
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
**  PUROPOSE:       Get the most recent feedback offsets applied in Hz in NTR automatic mode separated by ','
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
**  PUROPOSE:       Get the most recent feedback offsets applied in PPM in NTR automatic mode separated by ','
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
**  PUROPOSE:       Get the history of feedback offsets applied in PLL steps in NTR automatic mode separated by ','
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
**  PUROPOSE:       Get the history of feedback offsets applied in Hz in NTR automatic mode separated by ','
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
**  PUROPOSE:       Get the history of feedback offsets applied in PPM in NTR automatic mode separated by ','
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
**  PUROPOSE:       Get the history of automatically calculated phase error separated by ','
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
**  PUROPOSE:       Get NTR Enable flag
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
**  PUROPOSE:       Returns total no. of service providers configured
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

/*****************************************************************
**  FUNCTION:       dalVoice_GetNumAccPerSrvProv
**
**  PUROPOSE:       returns total accounts per specific serviceprovider
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
**  PUROPOSE:       returns total number of physical fxs endpoints in system
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
**  PUROPOSE:       returns total number of physical fxs endpoints in system
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
**  PUROPOSE:       returns total number of physical fxo endpoints in system
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
**  PUROPOSE:       returns total number of physical fxo endpoints in system
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
**  PUROPOSE:
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
CmsRet dalVoice_GetNumVoiceProfiles( DAL_VOICE_PARMS *parms, char* value, unsigned int length );
CmsRet dalVoice_GetSuppCodecsString( DAL_VOICE_PARMS *parms, char* value, unsigned int length );
CmsRet dalVoice_GetMaxSuppCodecs( DAL_VOICE_PARMS *parms, char* value, unsigned int length );

CmsRet dalVoice_SetVlFxoPhyReferenceList( DAL_VOICE_PARMS *parms, char *value );
CmsRet dalVoice_GetNumFxoEndpt( DAL_VOICE_PARMS *parms, char* value, unsigned int length );
CmsRet dalVoice_voiceStart(void* msgHandleArg);
CmsRet dalVoice_voiceStop(void* msgHandleArg);
CmsRet dalVoice_voiceReboot(void* msgHandleArg);
CmsRet dalVoice_GetStatus(DAL_VOICE_PARMS * parms, char* value, unsigned int length);


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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:
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
**  PUROPOSE:
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

CmsRet dalVoice_SetEuroFlashEnable( DAL_VOICE_PARMS *parms, char *value );
CmsRet dalVoice_GetEuroFlashEnable( DAL_VOICE_PARMS *parms, char *pEuroFlashEn, unsigned int length );

CmsRet dalVoice_mapFxsInterfaceIdToPhyIntfId(int id, int *pid);
#endif /* __DAL_VOICE_V1_H__ */

