/*
    Copyright 2000-2010 Broadcom Corporation

    <:label-BRCM:2011:DUAL/GPL:standard

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

/**************************************************************************
 * File Name  : boardparms_voice.h
 *
 * Description: This file contains definitions and function prototypes for
 *              the BCM63xx voice board parameter access functions.
 *
 ***************************************************************************/

#if !defined(_BOARDPARMS_VOICE_H)
#define _BOARDPARMS_VOICE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <boardparms.h>

#define VOICE_BOARD_ID_DEFAULT         "ID_NOT_SET"

/* Daughtercard defines */
#define VOICECFG_NOSLIC_STR            "NOSLIC"

#define VOICECFG_LE9530_STR            "LE9530"
#define VOICECFG_LE9530_WB_STR         "LE9530_WB"
#define VOICECFG_LE9540_STR            "LE9540"
#define VOICECFG_LE9540_WB_STR         "LE9540_WB"
#define VOICECFG_LE9541_STR            "LE9541"
#define VOICECFG_LE9541_WB_STR         "LE9541_WB"
#define VOICECFG_LE9530_LE88506_STR    "LE9530_LE88506"

#define VOICECFG_SI3239_STR            "SI3239"
#define VOICECFG_SI32392_STR           "SI32392"

#define VOICECFG_LE88506_STR           "LE88506"
#define VOICECFG_LE88536_ZSI_STR       "LE88536_ZSI"

#define VOICECFG_VE890_INVBOOST_STR    "VE890_INVBOOST"
#define VOICECFG_LE89116_STR           "LE89116"
#define VOICECFG_LE89316_STR           "LE89316"

#define VOICECFG_VE890HV_STR           "VE890HV"
#define VOICECFG_LE89136_STR           "LE89136"
#define VOICECFG_LE89336_STR           "LE89336"

#define VOICECFG_LE88266x2_STR         "LE88266x2"
#define VOICECFG_LE88266_STR           "LE88266"
#define VOICECFG_ZL88601x2_STR         "ZL88601x2"
#define VOICECFG_ZL88601_STR           "ZL88601"
#define VOICECFG_ZL88601x4_8FXS_STR    "ZL88601x4_8FXS"
#define VOICECFG_ZL88701_STR           "ZL88701"
#define VOICECFG_ZL88702_ZSI_STR       "ZL88702_ZSI"
#define VOICECFG_LE9672_ZSI_STR        "LE9672_ZSI"
#define VOICECFG_LE9662_ZSI_STR        "LE9662_ZSI"
#define VOICECFG_LE9662_ZSI_BB_STR     "LE9662_ZSI_BB"
#define VOICECFG_LE9642_ZSI_BB_STR     "LE9642_ZSI_BB"
#define VOICECFG_LE9642_ZSI_IB_STR     "LE9642_ZSI_IB"
#define VOICECFG_LE9643_ZSI_IB_STR     "LE9643_ZSI_IB"
#define VOICECFG_LE9641_ZSI_BB_STR     "LE9641_ZSI_BB"
#define VOICECFG_LE9652_ZSI_IB_STR     "LE9652_ZSI_IB"
#define VOICECFG_LE9653_IB_STR         "LE9653_IB"
#define VOICECFG_LE9661_ZSI_STR        "LE9661_ZSI"
#define VOICECFG_LE9661_ZSI_BB_STR     "LE9661_ZSI_BB"
#define VOICECFG_ZL88801_89010_BB_STR  "ZL88801_89010BB"
#define VOICECFG_LE9622_IB_STR         "LE9622_IB"

#define VOICECFG_SI3217X_STR           "SI3217X"
#define VOICECFG_SI32176_STR           "SI32176"
#define VOICECFG_SI32176_LCQC_STR      "SI32176_LCQC"
#define VOICECFG_SI32172_ISI_LCQC_STR  "SI32172_ISI_LQC"
#define VOICECFG_SI32173_ISI_LCQC_STR  "SI32173_ISI_LQC"
#define VOICECFG_SI32178_STR           "SI32178"
#define VOICECFG_SI3217X_NOFXO_STR     "SI3217X_NOFXO"

#define VOICECFG_SI32267_STR           "SI32267"
#define VOICECFG_SI32267_LCQC_STR      "SI32267_LCQC"
#define VOICECFG_SI32267_NTR_STR       "SI32267_NTR"

#define VOICECFG_SI32260x2_SI3050_STR  "SI32260x2_3050"
#define VOICECFG_SI32260x2_STR         "SI32260x2"
#define VOICECFG_SI32260_STR           "SI32260"
#define VOICECFG_SI32260_LCQC_STR      "Si32260_LCQC"
#define VOICECFG_SI32260_SI3050_STR    "SI32260_3050"
#define VOICECFG_SI32260_SI3050_QC_STR "SI32260_3050_QC"
#define VOICECFG_SI32182_ISI_LCCB_STR  "SI32182_ISI_CB"
#define VOICECFG_SI32184_LCCB_STR      "SI32184_CB"
#define VOICECFG_SI32280_ISI_LCCB_STR  "SI32280_ISI_CB"
#define VOICECFG_SI32282_LCCB_STR      "SI32282_CB"
#define VOICECFG_SI32284_ISI_LCCB_STR  "SI32284_ISI_CB"
#define VOICECFG_SI32286_LCCB_STR      "SI32286_CB"
#define VOICECFG_SI32192_ISI_LCCB_STR  "SI32192_ISI_CB"

/* Non-daughtercard defines */
#define BP_DECT_POPULATED              1
#define BP_DECT_NOT_POPULATED          0
#define DECT_SUPPORT_MASK              0x1

/* Max number of voice devices to list */
#define BP_VOICE_FILTERED_MAX_SIZE     50

/* Maximum number of devices in the system (on the board).
** Devices can refer to DECT, SLAC/SLIC, or SLAC/DAA combo. */
#define BP_MAX_VOICE_DEVICES           5

/* Maximum numbers of channels per SLAC. */
#define BP_MAX_CHANNELS_PER_DEVICE     2

/* Maximum number of voice channels in the system.
** This represents the sum of all channels available on the devices in the system */
#define BP_MAX_VOICE_CHAN              (BP_MAX_VOICE_DEVICES * BP_MAX_CHANNELS_PER_DEVICE)

/* Max number of GPIO pins used for controling PSTN failover relays
** Note: the number of PSTN failover relays can be larger if multiple
** relays are controlled by single GPIO */
#define BP_MAX_RELAY_PINS              2

/* Macro that returns the Number of PCM timeslots in use by specificed channel
 * parameters */
#define BP_PCM_TS_COUNT(compression, rate)  \
   ((compression == BP_VC_COMP_LINEAR)      \
      ? (rate == BP_VC_16KHZ ? 4 : 2)       \
      : (rate == BP_VC_16KHZ ? 2 : 1))

/* General-purpose flag definitions (rename as appropriate) */
#define BP_FLAG_DSP_APMHAL_ENABLE            (1 << 0)
#define BP_FLAG_DSP_PCMHAL_ENABLE            (1 << 1)
#define BP_FLAG_ISI_SUPPORT                  (1 << 2)
#define BP_FLAG_ZSI_SUPPORT                  (1 << 3)
#define BP_FLAG_THALASSA_SUPPORT             (1 << 4)
#define BP_FLAG_MODNAME_TESTNAME4            (1 << 5)
#define BP_FLAG_MODNAME_TESTNAME5            (1 << 6)
#define BP_FLAG_MODNAME_TESTNAME6            (1 << 7)
#define BP_FLAG_MODNAME_TESTNAME7            (1 << 8)
#define BP_FLAG_MODNAME_TESTNAME8            (1 << 9)

/* APM channel initializer macro */
#define BP_CHAN_APM(active, vdtype, comp, rate, apmchan)    \
   {                                                        \
     BP_CHAN_INT(active, vdtype, comp, rate),               \
     .override.apm = &(BP_APM_CFG) { .chan = apmchan }      \
   }

/* PCM channel initializer macro with rx and tx timeslot overrides */
#define TS(...)       { __VA_ARGS__ }
#define BP_CHAN_PCM(active, vdtype, comp, rate, ts)                          \
   {                                                                         \
     BP_CHAN_INT(active, vdtype, comp, rate),                                \
     .override.pcm = &(BP_PCM_TS_CFG) { .rxTimeslot = ts, .txTimeslot = ts } \
   }

#define BP_CHAN(active, vdtype, comp, rate)  \
   { BP_CHAN_INT(active, vdtype, comp, rate) }

#define BP_CHAN_INT(active, vdtype, comp, rate)    \
   .status     = active,                           \
   .type       = vdtype,                           \
   .sampleComp = comp,                             \
   .sampleRate = rate


#define BP_NULL_DEVICE_MACRO     \
{                                \
   BP_VD_NONE,                   \
   BP_VDTYPE_NONE,               \
   BP_SPI_SS_NOT_REQUIRED,       \
   BP_RESET_NOT_REQUIRED,        \
   {                             \
      BP_CHAN( BP_VC_INACTIVE, BP_VCTYPE_NONE, BP_VC_COMP_LINEAR, BP_VC_8KHZ ), \
      BP_CHAN( BP_VC_INACTIVE, BP_VCTYPE_NONE, BP_VC_COMP_LINEAR, BP_VC_8KHZ ), \
   }                             \
}

#define BP_MAX_SUPPORTED_DC     20
#define BP_MAX_DECT_DEVICE      2
#define BP_MAX_DC_SPI_DEVICE    3

/* To avoid conflict with GPIO pin defines */
#define BP_NOT_CONNECTED        BP_NOT_DEFINED
#define BP_DEDICATED_PIN        BP_GPIO_NONE

typedef enum
{
   BP_VOICE_NO_DECT,
   BP_VOICE_INT_DECT,
   BP_VOICE_EXT_DECT
} BP_VOICE_DECT_TYPE;

/*
** Device-specific definitions
*/
typedef enum
{
   BP_VD_NONE = -1,
   BP_VD_IDECT1,  /* Do not move this around, otherwise rebuild dect_driver.bin */
   BP_VD_EDECT1,
   BP_VD_SILABS_3050, /* SILAB IDs till BP_VD_SILABS_MAXID. Do NOT mess up Zarlink and Silab IDs. Put SILAB IDs before Zarlink's always*/
   BP_VD_SILABS_3215,
   BP_VD_SILABS_3216,
   BP_VD_SILABS_3217,
   BP_VD_SILABS_32172_ISI,
   BP_VD_SILABS_32173_ISI,
   BP_VD_SILABS_32176,
   BP_VD_SILABS_32178,
   BP_VD_SILABS_3226,
   BP_VD_SILABS_32260,
   BP_VD_SILABS_32261,
   BP_VD_SILABS_32267,
   BP_VD_SILABS_3239,
   BP_VD_SILABS_32392,
   BP_VD_SILABS_32182_ISI,
   BP_VD_SILABS_32184,
   BP_VD_SILABS_32280_ISI,
   BP_VD_SILABS_32282,
   BP_VD_SILABS_32284_ISI,
   BP_VD_SILABS_32286,
   BP_VD_SILABS_32192_ISI,
   BP_VD_SILABS_MAXID,
   
   BP_VD_ZARLINK_88010,/* ZARLINK IDs till BP_VD_MAX */
   BP_VD_ZARLINK_88221,
   BP_VD_ZARLINK_88266,
   BP_VD_ZARLINK_88276,
   BP_VD_ZARLINK_88506,
   BP_VD_ZARLINK_88536,
   BP_VD_ZARLINK_88264,
   BP_VD_ZARLINK_89010,
   BP_VD_ZARLINK_89116,
   BP_VD_ZARLINK_89316,
   BP_VD_ZARLINK_9530,
   BP_VD_ZARLINK_9540,
   BP_VD_ZARLINK_9541,
   BP_VD_ZARLINK_89136,
   BP_VD_ZARLINK_89336,
   BP_VD_ZARLINK_88601,
   BP_VD_ZARLINK_88701,
   BP_VD_ZARLINK_88702_ZSI,
   BP_VD_ZARLINK_9662,
   BP_VD_ZARLINK_9661,
   BP_VD_ZARLINK_88801,
   BP_VD_ZARLINK_9672_ZSI,
   BP_VD_ZARLINK_9642_ZSI,
   BP_VD_ZARLINK_9642_ZSI_IB,
   BP_VD_ZARLINK_9641_ZSI,
   BP_VD_ZARLINK_9652_ZSI_IB,
   BP_VD_ZARLINK_9622_FB,
   BP_VD_ZARLINK_9622_IB,
   BP_VD_ZARLINK_9632_IB,
   BP_VD_ZARLINK_9643_ZSI_IB,
   BP_VD_ZARLINK_9653_IB,
   BP_VD_MAX,
} BP_VOICE_DEVICE_TYPE;

typedef enum
{
	BP_VOICE_ZARLINK,
	BP_VOICE_SILABS,
}BP_VOICE_SS_VENDOR;

typedef enum
{
   BP_VD_FLYBACK,
   BP_VD_FLYBACK_TH,
   BP_VD_BUCKBOOST,
   BP_VD_INVBOOST,
   BP_VD_INVBOOST_TH,
   BP_VD_QCUK,
   BP_VD_FIXEDRAIL,
   BP_VD_MASTERSLAVE_FB,
   BP_VD_MASTERSLAVE_IB,
   BP_VD_FB_TSS,
   BP_VD_FB_TSS_ISO,
   BP_VD_PMOS_BUCK_BOOST,
   BP_VD_LCQCUK,
   BP_VD_LCCB,
} BP_VOICE_DEVICE_PROFILE;

/*
** Channel-specific definitions
*/
typedef enum
{
   BP_VC_NONE = -1,
   BP_VC_ACTIVE,
   BP_VC_INACTIVE,
} BP_VC_STATUS;

typedef enum
{
   BP_VCTYPE_NONE = -1,
   BP_VCTYPE_SLIC,
   BP_VCTYPE_DAA,
   BP_VCTYPE_DECT,
   BP_VCTYPE_AUDIO
} BP_VC_TYPE;

typedef enum
{
   BP_VDTYPE_NONE = -1,
   BP_VDTYPE_PCM,
   BP_VDTYPE_APM,
   BP_VDTYPE_DECT,
   BP_VDTYPE_AUDIO,
} BP_VD_AUDIO_TYPE;

typedef enum
{
   BP_VC_8KHZ,
   BP_VC_16KHZ
} BP_VC_SAMPLE_RATE;

typedef enum
{
   BP_VC_COMP_LINEAR,
   BP_VC_COMP_ALAW,
   BP_VC_COMP_ULAW
} BP_VC_SAMPLE_COMP;

typedef enum
{
   SPI_DEV_0,
   SPI_DEV_1,
   SPI_DEV_2,
   SPI_DEV_3,
   SPI_DEV_4,
   SPI_DEV_5,
   SPI_DEV_6,
   SPI_DEV_7

} BP_SPI_PORT;

typedef enum
{
   BP_SPI_SS_NOT_REQUIRED = -1,
   BP_SPI_SS_B1,
   BP_SPI_SS_B2,
   BP_SPI_SS_B3,
   BP_SPI_SS_B4,

} BP_SPI_SIGNAL;

typedef enum
{
   BP_RESET_NOT_REQUIRED = -1,
   BP_RESET_FXS1,
   BP_RESET_FXS2,
   BP_RESET_FXS3,
   BP_RESET_FXO = BP_RESET_FXS3,
   BP_RESET_FXS4

} BP_RESET_PIN;

typedef enum
{
   BP_APM_CHAN_A,
   BP_APM_CHAN_B,

   BP_APM_CHAN_LAST
} BP_APM_CHAN;

typedef struct
{
   /* Timeslot overrides for receive and transmit direction. Timeslot indices
    * are byte offsets in the PCM frame, given in the order of DMA copy. If no
    * override is set, timeslots will be automatically computed on bootup. By
    * default, it is assumed the device is big endian. Overrides will be needed
    * for little-endian devices. Unconfigured timeslots will be set to -1 */
   int rxTimeslot[4];
   int txTimeslot[4];
} BP_PCM_TS_CFG;

typedef struct
{
   BP_PCM_TS_CFG ts;        /* timeslot configuration */
   unsigned int  pcmChanId; /* PCM channel ID */
} BP_PCM_CFG;

typedef struct
{
   int chan; /* BP_APM_CHAN - APM channel id */
} BP_APM_CFG;

typedef struct
{
   int dectChanId;
} BP_DECT_CFG;

typedef struct
{
   unsigned int id;          /* global voice channel index */
   int          status;      /* BP_VS_STATUS      - active/inactive */
   int          type;        /* BP_VC_TYPE        - type of device */
   int          sampleComp;  /* BP_VC_SAMPLE_COMP - u-law/a-law (applicable for 8-bit samples) */
   int          sampleRate;  /* BP_VC_SAMPLE_RATE - narrowband/wideband */
   union
   {
      BP_PCM_CFG     pcm;
      BP_APM_CFG     apm;
      BP_DECT_CFG    dect;
   } cfg;

} BP_VOICE_CHANNEL;

/* Boardparms internal channel configuration */
typedef struct
{
   BP_VC_STATUS      status;        /* active/inactive */
   BP_VC_TYPE        type;          /* type of device */
   BP_VC_SAMPLE_COMP sampleComp;    /* u-law/a-law (applicable for 8-bit samples) */
   BP_VC_SAMPLE_RATE sampleRate;    /* narrowband/wideband */
   union
   {
      BP_PCM_TS_CFG *pcm;
      BP_APM_CFG    *apm;
   } override;

} BP_VOICE_CHANNEL_INTERNAL;

typedef struct
{
   int                  spiDevId;               /* SPI device id */
   unsigned int         spiGpio;                /* SPI GPIO (if used for SPI control) */
} BP_VOICE_SPI_CONTROL;

typedef struct
{
   unsigned int         relayGpio[BP_MAX_RELAY_PINS];
} BP_PSTN_RELAY_CONTROL;

typedef struct
{
   unsigned short       dectUartGpioTx;
   unsigned short       dectUartGpioRx;
} BP_DECT_UART_CONTROL;

typedef struct
{
   int                  voiceDeviceType;    /* BP_VOICE_DEVICE_TYPE - Specific type of device (Le88276, Si32176, etc.) */
   int                  audioInterfaceType; /* BP_VD_AUDIO_TYPE - Audio interface type */
   BP_VOICE_SPI_CONTROL spiCtrl;            /* SPI control through dedicated SPI pin or GPIO */
   unsigned int         requiresReset;      /* Does the device requires reset (through GPIO) */
   unsigned int         resetGpio;          /* Reset GPIO */
   BP_VOICE_CHANNEL     channel[BP_MAX_CHANNELS_PER_DEVICE]; /* Device channels */

} BP_VOICE_DEVICE;

/* Boardparms internal configuration */
typedef struct
{
   BP_VOICE_DEVICE_TYPE      deviceType;            /* Specific type of device (Le88267, Si32176, etc.) */
   BP_VD_AUDIO_TYPE          audioType;             /* Audio device type */
   BP_SPI_SIGNAL             SPI_SS_Bx;             /* SPI Control */
   BP_RESET_PIN              rstPin;                /* Reset pin */
   BP_VOICE_CHANNEL_INTERNAL channel[BP_MAX_CHANNELS_PER_DEVICE];   /* Device channels */

} BP_VOICE_DEVICE_INTERNAL;

/*
** Main structure for defining the board parameters and used by boardHal
** for proper initialization of the DSP and devices (SLACs, DECT, etc.)
*/
typedef struct VOICE_BOARD_PARMS
{
   char                    szBoardId[BP_BOARD_ID_LEN];      /* daughtercard id string */
   char                    szBaseBoardId[BP_BOARD_ID_LEN];  /* motherboard id string */
   unsigned int            numFxsLines;            /* Number of FXS lines in the system */
   unsigned int            numFxoLines;            /* Number of FXO lines in the system */
   unsigned int            numDectLines;           /* Number of DECT lines in the system */
   unsigned int            numFailoverRelayPins;   /* Number of GPIO pins controling PSTN failover relays */
   BP_VOICE_DEVICE         voiceDevice[BP_MAX_VOICE_DEVICES];  /* Voice devices in the system */
   BP_PSTN_RELAY_CONTROL   pstnRelayCtrl;          /* Control for PSTN failover relays */
   BP_DECT_UART_CONTROL    dectUartControl;        /* Control for external DECT UART */
   int                     deviceProfile;          /* BP_VOICE_DEVICE_PROFILE - Power supply configuration, if required */
   unsigned int            flags;                  /* General-purpose flags */

} VOICE_BOARD_PARMS, *PVOICE_BOARD_PARMS;

typedef struct
{
   char                     szBoardId[BP_BOARD_ID_LEN];         /* daughtercard id string */
   BP_VOICE_DEVICE_INTERNAL voiceDevice[BP_MAX_VOICE_DEVICES];  /* Voice devices in the system */
   int                      deviceProfile;                      /* BP_VOICE_DEVICE_PROFILE - Power supply configuration, if required */
   unsigned int             flags;                              /* General-purpose flags */

} VOICE_DAUGHTER_BOARD_PARMS, *PVOICE_DAUGHTER_BOARD_PARMS;


/* Function prototypes */


#if !defined(_CFE_)
int  BpGetVoiceParms( char *pszVoiceDaughterCardId, VOICE_BOARD_PARMS* voiceParms, char* pszBaseBoardId );
#endif /* !defined(_CFE_) */
int  BpDectPopulated( void );
void BpSetDectPopulatedData( int BpData );
int  BpSetVoiceBoardId(const char *pszVoiceDaughterCardId );
int  BpGetVoiceBoardId( char *pszVoiceDaughterCardId );
int  BpGetNumVoiceBoardIds(const char *pszBaseBoardId);
char *BpGetVoiceBoardIdNameByIndex(int i, const char *pszBaseBoardId);
int  BpGetVoiceBoardIds( char *pszVoiceDaughterCardIds, int nBoardIdsSize, char *pszBaseBoardId );
int  BpGetVoiceDectType( const char *pszBaseBoardId );

#if !defined(_CFE_)
void PrintAllParms(VOICE_BOARD_PARMS *parms);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _BOARDPARMS_VOICE_H */

