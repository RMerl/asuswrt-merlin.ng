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
* File Name  : boardparms_voice.c
*
* Description: This file contains the implementation for the BCM63xx board
*              parameter voice access functions.
*
***************************************************************************/

/* ---- Include Files ---------------------------------------------------- */
#include <boardparms_voice.h>
#include "bp_defs.h"
#include <bcm_map_part.h>

#if !defined(_CFE_)
#include <linux/kernel.h>
#include <linux/export.h>
#endif /* !defined(_CFE_) */


/* ---- Public Variables ------------------------------------------------- */
/* ---- Private Constants and Types -------------------------------------- */
#define BP_VOICE_ADD_INTERFACE_PINMUX( pElem, intfEnableFlag )  { pElem->id = bp_ulInterfaceEnable; pElem->u.ul = intfEnableFlag; pElem++; }
#define BP_VOICE_ADD_SIGNAL_PINMUX( pElem, itemId, usVal ) { pElem->id = itemId; pElem->u.us = usVal; pElem++; }


#define SLICSLAC_LIST_ISI_NOFXO         &g_voiceBoard_NOSLIC,             \
                                        &g_voiceBoard_SI32172_ISI_LCQC,   \
                                        &g_voiceBoard_SI32173_ISI_LCQC,   \
                                        &g_voiceBoard_SI32182_ISI_LCBC,   \
                                        &g_voiceBoard_SI32192_ISI_LCBC,   \
                                        &g_voiceBoard_SI32280_ISI_LCCB,   \
                                        &g_voiceBoard_SI32282_LCCB,       \
                                        &g_voiceBoard_SI32284_ISI_LCCB,   \
                                        &g_voiceBoard_SI32267

#define SLICSLAC_LIST_ZSI_NOFXO         &g_voiceBoard_LE88536_ZSI,        \
                                        &g_voiceBoard_LE9641_ZSI_BB,      \
                                        &g_voiceBoard_LE9642_ZSI_BB,      \
                                        &g_voiceBoard_LE9652_ZSI_IB,      \
                                        &g_voiceBoard_LE9643_ZSI_IB,      \
                                        &g_voiceBoard_LE9661_ZSI,         \
                                        &g_voiceBoard_LE9661_ZSI_BB,      \
                                        &g_voiceBoard_LE9662_ZSI,         \
                                        &g_voiceBoard_LE9662_ZSI_BB,      \
                                        &g_voiceBoard_LE9672_ZSI,         \
                                        &g_voiceBoard_ZL88702_ZSI


/* Common Daughter Card lists */
#define SLICSLAC_LIST_COMMON_NOFXO      SLICSLAC_LIST_ISI_NOFXO,          \
                                        SLICSLAC_LIST_ZSI_NOFXO,          \
                                        &g_voiceBoard_LE89116,            \
                                        &g_voiceBoard_LE88506,            \
                                        &g_voiceBoard_LE9653_IB,          \
                                        &g_voiceBoard_ZL88601,            \
                                        &g_voiceBoard_ZL88601x2,          \
                                        &g_voiceBoard_SI32176,            \
                                        &g_voiceBoard_SI32176_LCQC,       \
                                        &g_voiceBoard_SI32184_LCBC,       \
                                        &g_voiceBoard_SI32286_LCCB,       \
                                        &g_voiceBoard_SI32260,            \
                                        &g_voiceBoard_SI32260_LCQC,       \
                                        &g_voiceBoard_SI32267

#define SLICSLAC_LIST_COMMON_FXO        SLICSLAC_LIST_COMMON_NOFXO,       \
                                        &g_voiceBoard_VE890_INVBOOST,     \
                                        &g_voiceBoard_ZL88701,            \
                                        &g_voiceBoard_ZL88801_89010_BB,   \
                                        &g_voiceBoard_SI3217x,            \
                                        &g_voiceBoard_SI32178,            \
                                        &g_voiceBoard_SI32260_SI3050,     \
                                        &g_voiceBoard_SI32260_SI3050_QC

#define SLICSLAC_LIST_4FXS_NOFXO        &g_voiceBoard_SI32260x2

#define SLICSLAC_LIST_4FXS_FXO          SLICSLAC_LIST_4FXS_NOFXO,         \
                                        &g_voiceBoard_SI32260x2_SI3050


#define SLICSLAC_LIST_ZSI_FXO           SLICSLAC_LIST_ZSI_NOFXO



/* ---- Private Variables ------------------------------------------------ */
static char voiceCurrentDgtrCardCfgId[BP_BOARD_ID_LEN] = VOICE_BOARD_ID_DEFAULT;
static int g_BpDectPopulated = 0;
static bp_elem_t g_voice_filteredBp[BP_VOICE_FILTERED_MAX_SIZE];
#if !defined(_CFE_)
static int vpInitialized = 0;
static struct VOICE_BOARD_PARMS vp = {{ 0 }};
#endif

/* ---- Public Functions -------------------------------------------------- */
bp_elem_t * BpGetVoicePmuxBp( bp_elem_t * pCurrentDataBp );

/* ---- External Functions ------------------------------------------------ */
extern bp_elem_t * BpGetElem(enum bp_id id, bp_elem_t **pstartElem, enum bp_id stopAtId);
extern char *BpGetSubCp(enum bp_id id, bp_elem_t *pstartElem, enum bp_id stopAtId );
extern void *BpGetSubPtr(enum bp_id id, bp_elem_t *pstartElem, enum bp_id stopAtId );
extern unsigned char BpGetSubUc(enum bp_id id, bp_elem_t *pstartElem, enum bp_id stopAtId );
extern unsigned short BpGetSubUs(enum bp_id id, bp_elem_t *pstartElem, enum bp_id stopAtId );
extern unsigned int BpGetSubUl(enum bp_id id, bp_elem_t *pstartElem, enum bp_id stopAtId );

/* ---- Private Functions ------------------------------------------------ */
static void bpmemcpy( void* dstptr, const void* srcptr, int size );
static char * bpstrcpy( char* dest, const char* src );
static bp_elem_t * BpGetVoiceBoardStartElemPtr(const char * pszBaseBoardId );
static int BpIsIntfEnabled( unsigned int interfaceFlag, bp_elem_t * pBoardParms );
static int BpElemExists( bp_elem_t * pBoardParms, enum bp_id  id );

#if !defined(_CFE_)
static int bpstrlen( char * src );
static enum bp_id mapDcRstPinToBpType( BP_RESET_PIN rstPin );
static enum bp_id mapDcSpiDevIdToBpType( BP_SPI_SIGNAL spiId );
static unsigned int BpGetZSISpiDevID( void );
static unsigned short BpGetSlaveSelectGpioNum( BP_SPI_PORT ssNum);
static int BpPcmTSCfg( BP_VOICE_CHANNEL *pCh, BP_VOICE_CHANNEL_INTERNAL *pIntCh,
                       unsigned int *pcmRxChanMask, unsigned int *pcmTxChanMask,
                       unsigned int *pcmChanId);
static int BpApmCfg( BP_VOICE_CHANNEL *pCh, BP_VOICE_CHANNEL_INTERNAL *pIntCh,
                     unsigned int *apmChanMask);
#endif /* !defined(_CFE_) */

/*
 * -------------------------- Voice Daughter Board Configs ------------------------------
 */
VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_LE88506 =
{
   VOICECFG_LE88506_STR,     /* szBoardId */
   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88506,
         BP_VDTYPE_PCM,
         BP_SPI_SS_B1,  /* Device uses SPI_SS_B1 pin. Pin on base board depends on base board parameters. */
         BP_RESET_FXS1, /* Device uses FXS1 reset pin. Pin on base board depends on base board parameters. */

         /* Channel description */
         {
            /* Force using timeslots 0,1 for channel 0 and 2,3 for channel 1 */
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(0, 1) ),
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(2, 3) ),
         }
      },
      /* Always end the device list with BP_NULL_DEVICE */
      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_VD_FLYBACK,
   /* General-purpose flags */
   ( BP_FLAG_DSP_PCMHAL_ENABLE )
};


VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_SI32267 =
{
   VOICECFG_SI32267_STR,   /*Daughter board ID */
   {
      {
         /* Device Type */
         BP_VD_SILABS_32267,
         BP_VDTYPE_PCM,
         BP_SPI_SS_NOT_REQUIRED,   /* ISI SPI CS handled internally. It is mapped by the zsiChipMap list. */
         BP_RESET_FXS1,

         /* Channel description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(0, 1) ),
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(2, 3) ),
         }
      },

      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_VD_FB_TSS_ISO,
   /* General-purpose flags */
   ( BP_FLAG_ISI_SUPPORT | BP_FLAG_DSP_PCMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_SI32267_LCQC =
{
   VOICECFG_SI32267_LCQC_STR,   /*Daughter board ID */
   {
      {
         /* Device Type */
         BP_VD_SILABS_32267,
         BP_VDTYPE_PCM,
         BP_SPI_SS_NOT_REQUIRED,   /* ISI SPI CS handled internally. It is mapped by the zsiChipMap list. */
         BP_RESET_FXS1,

         /* Channel description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(0, 1) ),
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(2, 3) ),
         }
      },

      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_VD_LCQCUK,
   /* General-purpose flags */
   ( BP_FLAG_ISI_SUPPORT | BP_FLAG_DSP_PCMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_SI3217x =
{
   VOICECFG_SI3217X_STR,   /*Daughter Card ID */
   {
      {
         /* Device Type */
         BP_VD_SILABS_32176,
         BP_VDTYPE_PCM,
         BP_SPI_SS_B1,  /* Device uses SPI_SS_B1 pin. Pin on base board depends on base board parameters. */
         BP_RESET_FXS1, /* Device uses FXS1 reset pin. Pin on base board depends on base board parameters. */
         /* Channel description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(0, 1) ),
            BP_CHAN( BP_VC_INACTIVE, BP_VCTYPE_NONE, BP_VC_COMP_LINEAR, BP_VC_8KHZ ),
         }
      },
      {
         /* Device Type 2 */
         BP_VD_SILABS_32178,
         BP_VDTYPE_PCM,
         BP_SPI_SS_B2,  /* Device uses SPI_SS_B2 pin. Pin on base board depends on base board parameters. */
         BP_RESET_FXS2, /* Device uses FXS2 reset pin. Pin on base board depends on base board parameters. */
         /* Channel description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(2, 3) ),
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_DAA,  BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(4, 5) ),
         }
      },

      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_VD_FLYBACK,
   /* General-purpose flags */
   ( BP_FLAG_DSP_PCMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_SI32176_LCQC =
{
   VOICECFG_SI32176_LCQC_STR,   /*Daughter Card ID */
   {
      {
         /* Device Type */
         BP_VD_SILABS_32176,
         BP_VDTYPE_PCM,
         BP_SPI_SS_B1,  /* Device uses SPI_SS_B1 pin. Pin on base board depends on base board parameters. */
         BP_RESET_FXS1, /* Device uses FXS1 reset pin. Pin on base board depends on base board parameters. */

         /* Channel description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(0, 1) ),
            BP_CHAN( BP_VC_INACTIVE, BP_VCTYPE_NONE, BP_VC_COMP_LINEAR, BP_VC_8KHZ ),
         }
      },

      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_VD_LCQCUK,
   /* General-purpose flags */
   ( BP_FLAG_DSP_PCMHAL_ENABLE  )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_SI32172_ISI_LCQC =
{
   VOICECFG_SI32172_ISI_LCQC_STR,   /*Daughter Card ID */
   {
      {
         /* Device Type */
         BP_VD_SILABS_32172_ISI,
         BP_VDTYPE_PCM,
         BP_SPI_SS_NOT_REQUIRED,  /* Device uses SPI_SS_B1 pin. Pin on base board depends on base board parameters. */
         BP_RESET_FXS1, /* Device uses FXS1 reset pin. Pin on base board depends on base board parameters. */

         /* Channel description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(0, 1) ),
            BP_CHAN( BP_VC_INACTIVE, BP_VCTYPE_NONE, BP_VC_COMP_LINEAR, BP_VC_8KHZ ),
         }
      },

      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_VD_LCQCUK,
   /* General-purpose flags */
   ( BP_FLAG_ISI_SUPPORT | BP_FLAG_DSP_PCMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_SI32173_ISI_LCQC =
{
   VOICECFG_SI32173_ISI_LCQC_STR,   /*Daughter Card ID */
   {
      {
         /* Device Type */
         BP_VD_SILABS_32172_ISI,
         BP_VDTYPE_PCM,
         BP_SPI_SS_NOT_REQUIRED,  /* Device uses SPI_SS_B1 pin. Pin on base board depends on base board parameters. */
         BP_RESET_FXS1, /* Device uses FXS1 reset pin. Pin on base board depends on base board parameters. */

         /* Channel description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(0, 1) ),
            BP_CHAN( BP_VC_INACTIVE, BP_VCTYPE_NONE, BP_VC_COMP_LINEAR, BP_VC_8KHZ ),
         }
      },

      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_VD_LCQCUK,
   /* General-purpose flags */
   ( BP_FLAG_ISI_SUPPORT | BP_FLAG_DSP_PCMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_SI32182_ISI_LCBC =
{
   VOICECFG_SI32182_ISI_LCCB_STR,   /*Daughter board ID */
   {
      {
         /* Device Type */
         BP_VD_SILABS_32182_ISI,
         BP_VDTYPE_PCM,
         BP_SPI_SS_NOT_REQUIRED,   /* ISI SPI CS handled internally. It is mapped by the zsiChipMap list. */
         BP_RESET_FXS1,

         /* Channel description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(0, 1) ),
            BP_CHAN( BP_VC_INACTIVE, BP_VCTYPE_NONE, BP_VC_COMP_LINEAR, BP_VC_8KHZ ),
         }
      },
      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_VD_LCCB,
   /* General-purpose flags */
   ( BP_FLAG_ISI_SUPPORT | BP_FLAG_DSP_PCMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_SI32184_LCBC =
{
   VOICECFG_SI32184_LCCB_STR,   /*Daughter board ID */
   {
      {
         /* Device Type */
         BP_VD_SILABS_32184,
         BP_VDTYPE_PCM,
         BP_SPI_SS_B1,  /* Device uses SPI_SS_B1 pin. Pin on base board depends on base board parameters. */
         BP_RESET_FXS1, /* Device uses FXS1 reset pin. Pin on base board depends on base board parameters. */

         /* Channel description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(0, 1) ),
            BP_CHAN( BP_VC_INACTIVE, BP_VCTYPE_NONE, BP_VC_COMP_LINEAR, BP_VC_8KHZ ),
         }
      },
      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_VD_LCCB,
   /* General-purpose flags */
   ( BP_FLAG_DSP_PCMHAL_ENABLE )
};


VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_SI32192_ISI_LCBC =
{
   VOICECFG_SI32192_ISI_LCCB_STR,   /*Daughter board ID */
   {
      {
         /* Device Type */
         BP_VD_SILABS_32192_ISI,
         BP_VDTYPE_PCM,
         BP_SPI_SS_NOT_REQUIRED,   /* ISI SPI CS handled internally. It is mapped by the zsiChipMap list. */
         BP_RESET_FXS1,

         /* Channel description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(0, 1) ),
            BP_CHAN( BP_VC_INACTIVE, BP_VCTYPE_NONE, BP_VC_COMP_LINEAR, BP_VC_8KHZ ),
         }
      },
      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_VD_LCCB,
   /* General-purpose flags */
   ( BP_FLAG_ISI_SUPPORT | BP_FLAG_DSP_PCMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_SI32286_LCCB =
{
   VOICECFG_SI32286_LCCB_STR,   /*Daughter board ID */
   {
      {
         /* Device Type */
         BP_VD_SILABS_32286,
         BP_VDTYPE_PCM,
         BP_SPI_SS_B1,   /* ISI SPI CS handled internally. It is mapped by the zsiChipMap list. */
         BP_RESET_FXS1,

         /* Channel description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(0, 1) ),
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(2, 3) ),
         }
      },
      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_VD_LCCB,
   /* General-purpose flags */
   ( BP_FLAG_DSP_PCMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_SI32280_ISI_LCCB =
{
   VOICECFG_SI32280_ISI_LCCB_STR,   /*Daughter board ID */
   {
      {
         /* Device Type */
         BP_VD_SILABS_32280_ISI,
         BP_VDTYPE_PCM,
         BP_SPI_SS_NOT_REQUIRED,   /* ISI SPI CS handled internally. It is mapped by the zsiChipMap list. */
         BP_RESET_FXS1,

         /* Channel description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(0, 1) ),
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(2, 3) ),
         }
      },
      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_VD_LCCB,
   /* General-purpose flags */
   (  BP_FLAG_ISI_SUPPORT | BP_FLAG_DSP_PCMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_SI32282_LCCB =
{
   VOICECFG_SI32282_LCCB_STR,   /*Daughter board ID */
   {
      {
         /* Device Type */
         BP_VD_SILABS_32282,
         BP_VDTYPE_PCM,
         BP_SPI_SS_B1,   /* ISI SPI CS handled internally. It is mapped by the zsiChipMap list. */
         BP_RESET_FXS1,

         /* Channel description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(0, 1) ),
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(2, 3) ),
         }
      },
      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_VD_LCCB,
   /* General-purpose flags */
   ( BP_FLAG_DSP_PCMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_SI32284_ISI_LCCB =
{
   VOICECFG_SI32284_ISI_LCCB_STR,   /*Daughter board ID */
   {
      {
         /* Device Type */
         BP_VD_SILABS_32284_ISI,
         BP_VDTYPE_PCM,
         BP_SPI_SS_NOT_REQUIRED,   /* ISI SPI CS handled internally. It is mapped by the zsiChipMap list. */
         BP_RESET_FXS1,

         /* Channel description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(0, 1) ),
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(2, 3) ),
         }
      },
      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_VD_LCCB,
   /* General-purpose flags */
   (  BP_FLAG_ISI_SUPPORT | BP_FLAG_DSP_PCMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_VE890_INVBOOST =
{
   VOICECFG_VE890_INVBOOST_STR,   /* daughter card ID */
   {   /* voiceDevice0 parameters */
      {
         /* Device Type */
         BP_VD_ZARLINK_89116,
         BP_VDTYPE_PCM,
         BP_SPI_SS_B1,  /* Device uses SPI_SS_B1 pin. Pin on base board depends on base board parameters. */
         BP_RESET_FXS1, /* Device uses FXS1 reset pin. Pin on base board depends on base board parameters. */
         /* Channel description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(0, 1)),
            BP_CHAN( BP_VC_INACTIVE, BP_VCTYPE_NONE, BP_VC_COMP_LINEAR, BP_VC_8KHZ ),
         }
      },
      {
         /* Device Type 2 */
         BP_VD_ZARLINK_89316,
         BP_VDTYPE_PCM,
         BP_SPI_SS_B2,  /* Device uses SPI_SS_B2 pin. Pin on base board depends on base board parameters. */
         BP_RESET_FXS2, /* Device uses FXS2 reset pin. Pin on base board depends on base board parameters. */
         /* Channel description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(2, 3) ),
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_DAA,  BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(4, 5) ),
         }
      },

      /* Always end device list with this macro. */
      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_VD_INVBOOST,
   /* General-purpose flags */
   ( BP_FLAG_DSP_PCMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_SI32260x2 =
{
   VOICECFG_SI32260x2_STR,   /*Daughter Card ID*/
   {   /* voiceDevice0 Parameters */
      {
         /* Device Type */
         BP_VD_SILABS_32261,
         BP_VDTYPE_PCM,
         BP_SPI_SS_B1,  /* Device uses SPI_SS_B1 pin. Pin on base board depends on base board parameters. */
         BP_RESET_FXS1, /* Device uses FXS1 reset pin. Pin on base board depends on base board parameters. */
         /* Channel description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(0, 1) ),
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(2, 3) ),
         }
      },
      {
         /* Device Type 2*/
         BP_VD_SILABS_32261,
         BP_VDTYPE_PCM,
         BP_SPI_SS_B2,  /* Device uses SPI_SS_B2 pin. Pin on base board depends on base board parameters. */
         BP_RESET_FXS2, /* Device uses FXS2 reset pin. Pin on base board depends on base board parameters. */
         /* Channel description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(4, 5) ),
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(6, 7) ),
         }
      },

      /* Always end device list with this macro. */
      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_VD_FB_TSS,
   /* General-purpose flags */
   ( BP_FLAG_DSP_PCMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_SI32260 =
{
   VOICECFG_SI32260_STR,   /*Daughter Card ID*/
   {   /* voiceDevice0 Parameters */
      {
         /* Device Type */
         BP_VD_SILABS_32261,
         BP_VDTYPE_PCM,
         BP_SPI_SS_B1,  /* Device uses SPI_SS_B1 pin. Pin on base board depends on base board parameters. */
         BP_RESET_FXS1, /* Device uses FXS1 reset pin. Pin on base board depends on base board parameters. */
         /* Channel description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(0, 1) ),
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(2, 3) ),
         }
      },

      /* Always end device list with this macro. */
      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_VD_PMOS_BUCK_BOOST,
   /* General-purpose flags */
   ( BP_FLAG_DSP_PCMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_SI32260_SI3050 =
{
   VOICECFG_SI32260_SI3050_STR,   /*Daughter card ID */
   {
      {
         /* Device Type */
         BP_VD_SILABS_32261,
         BP_VDTYPE_PCM,
         BP_SPI_SS_B1,  /* Device uses SPI_SS_B1 pin. Pin on base board depends on base board parameters. */
         BP_RESET_FXS1, /* Device uses FXS1 reset pin. Pin on base board depends on base board parameters. */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(0, 1) ),
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(2, 3) ),
         }
      },
      {
         /* Device Type 2 */
         BP_VD_SILABS_3050,
         BP_VDTYPE_PCM,
         BP_SPI_SS_B2,  /* Device uses SPI_SS_B2 pin. Pin on base board depends on base board parameters. */
         BP_RESET_FXS2, /* Device uses FXS2 reset pin. Pin on base board depends on base board parameters. */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_DAA,  BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(4, 5) ),
            BP_CHAN( BP_VC_INACTIVE, BP_VCTYPE_NONE, BP_VC_COMP_LINEAR, BP_VC_8KHZ ),
         }
      },

      /* Always end device list with this macro. */
      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_VD_PMOS_BUCK_BOOST,
   /* General-purpose flags */
   ( BP_FLAG_DSP_PCMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_SI32260_SI3050_QC =
{
   VOICECFG_SI32260_SI3050_QC_STR,   /*Daughter card ID */
   {
      {
         /* Device Type */
         BP_VD_SILABS_32261,
         BP_VDTYPE_PCM,
         BP_SPI_SS_B1,  /* Device uses SPI_SS_B1 pin. Pin on base board depends on base board parameters. */
         BP_RESET_FXS1, /* Device uses FXS1 reset pin. Pin on base board depends on base board parameters. */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(0, 1) ),
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(2, 3) ),
         }
      },
      {
         /* Device Type 2 */
         BP_VD_SILABS_3050,
         BP_VDTYPE_PCM,
         BP_SPI_SS_B2,  /* Device uses SPI_SS_B2 pin. Pin on base board depends on base board parameters. */
         BP_RESET_FXS2, /* Device uses FXS2 reset pin. Pin on base board depends on base board parameters. */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_DAA,  BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(4, 5) ),
            BP_CHAN( BP_VC_INACTIVE, BP_VCTYPE_NONE, BP_VC_COMP_LINEAR, BP_VC_8KHZ ),
         }
      },

      /* Always end device list with this macro. */
      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_VD_QCUK,
   /* General-purpose flags */
   ( BP_FLAG_DSP_PCMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_SI32260_LCQC =
{
   VOICECFG_SI32260_LCQC_STR,   /*Daughter card ID */
   {
      {
         /* Device Type */
         BP_VD_SILABS_32261,
         BP_VDTYPE_PCM,
         BP_SPI_SS_B1,  /* Device uses SPI_SS_B1 pin. Pin on base board depends on base board parameters. */
         BP_RESET_FXS1, /* Device uses FXS1 reset pin. Pin on base board depends on base board parameters. */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(0, 1) ),
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(2, 3) ),
         }
      },
      /* Always end device list with this macro. */
      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_VD_LCQCUK,
   /* General-purpose flags */
   ( BP_FLAG_DSP_PCMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_LE88536_ZSI =
{
   VOICECFG_LE88536_ZSI_STR,   /* Daughter Board ID */
   {   /* Voice Device 0 Parameters */
      {
         BP_VD_ZARLINK_88536,   /* Device Type */
         BP_VDTYPE_PCM,
         BP_SPI_SS_NOT_REQUIRED,   /* ZSI SPI CS handled internally. It is mapped using the zsiMapList. */
         BP_RESET_FXS1,
         /* Channel description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(2, 3) ),
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(4, 5) ),
         }
      },

      /* Always end device list with this macro. */
      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_VD_INVBOOST,
   /* General-purpose flags */
   ( BP_FLAG_ZSI_SUPPORT | BP_FLAG_DSP_PCMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_ZL88601 =
{
   VOICECFG_ZL88601_STR,   /* szBoardId */
   {
      {
         /* Device Type */
         BP_VD_ZARLINK_88601,
         BP_VDTYPE_PCM,
         BP_SPI_SS_B1,  /* Device uses SPI_SS_B1 pin. Pin on base board depends on base board parameters. */
         BP_RESET_FXS1, /* Device uses FXS1 reset pin. Pin on base board depends on base board parameters. */
         /* Channel Description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(0, 1) ),
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(2, 3) ),
         }
      },

      /* Always end device list with this macro. */
      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_VD_FLYBACK,
   /* General-purpose flags */
   ( BP_FLAG_DSP_PCMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_ZL88601x2 =
{
   VOICECFG_ZL88601x2_STR,   /* szBoardId */
   {
      {
         /* Device Type */
         BP_VD_ZARLINK_88601,
         BP_VDTYPE_PCM,
         BP_SPI_SS_B1,  /* Device uses SPI_SS_B1 pin. Pin on base board depends on base board parameters. */
         BP_RESET_FXS1, /* Device uses FXS1 reset pin. Pin on base board depends on base board parameters. */
         /* Channel Description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(0, 1) ),
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(2, 3) ),
         }
      },
      {
         /* Device Type */
         BP_VD_ZARLINK_88601,
         BP_VDTYPE_PCM,
         BP_SPI_SS_B2,  /* Device uses SPI_SS_B1 pin. Pin on base board depends on base board parameters. */
         BP_RESET_FXS2, /* Device uses FXS1 reset pin. Pin on base board depends on base board parameters. */
         /* Channel Description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(4, 5) ),
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(6, 7) ),
         }
      },

      /* Always end device list with this macro. */
      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
      BP_VD_MASTERSLAVE_IB,
   /* General-purpose flags */
   ( BP_FLAG_DSP_PCMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_ZL88701 =
{
   VOICECFG_ZL88701_STR,   /* szBoardId */
   {
      {
         /* Device Type */
         BP_VD_ZARLINK_88701,
         BP_VDTYPE_PCM,
         BP_SPI_SS_B1,  /* Device uses SPI_SS_B1 pin. Pin on base board depends on base board parameters. */
         BP_RESET_FXS1, /* Device uses FXS1 reset pin. Pin on base board depends on base board parameters. */
         /* Channel Description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(0, 1) ),
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(2, 3) ),
         }
      },
      {
         /* Device Type 2 */
         BP_VD_ZARLINK_89010,
         BP_VDTYPE_PCM,
         BP_SPI_SS_B2,  /* Device uses SPI_SS_B2 pin. Pin on base board depends on base board parameters. */
         BP_RESET_NOT_REQUIRED,  /* FXO reset pin tied with FXS on this board.*/
         /* Channel description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_DAA,  BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(4, 5) ),
            BP_CHAN( BP_VC_INACTIVE, BP_VCTYPE_NONE, BP_VC_COMP_LINEAR, BP_VC_8KHZ ),
         }
      },

      /* Always end device list with this macro. */
      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_VD_INVBOOST,
   /* General-purpose flags */
   ( BP_FLAG_DSP_PCMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_ZL88601x4_8FXS =
{
   VOICECFG_ZL88601x4_8FXS_STR,   /* szBoardId */
   {

      {
         /* Device Type */
         BP_VD_ZARLINK_88601,
         BP_VDTYPE_PCM,
         BP_SPI_SS_B1,  /* Device uses SPI_SS_B1 pin. Pin on base board depends on base board parameters. */
         BP_RESET_FXS1, /* Device uses FXS1 reset pin. Pin on base board depends on base board parameters. */
         /* Channel Description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(0, 1) ),
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(2, 3) ),
         }
      },

      {
         /* Device Type */
         BP_VD_ZARLINK_88601,
         BP_VDTYPE_PCM,
         BP_SPI_SS_B2,  /* Device uses SPI_SS_B1 pin. Pin on base board depends on base board parameters. */
         BP_RESET_FXS2, /* Device uses FXS2 reset pin. Pin on base board depends on base board parameters. */
         /* Channel Description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(4, 5) ),
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(6, 7) ),
         }
      },


      {
         /* Device Type */
         BP_VD_ZARLINK_88601,
         BP_VDTYPE_PCM,
         BP_SPI_SS_B3,  /* Device uses SPI_SS_B1 pin. Pin on base board depends on base board parameters. */
         BP_RESET_FXS3, /* Device uses FXS3 reset pin. Pin on base board depends on base board parameters. */
         /* Channel Description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(8, 9) ),
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(10, 11) ),
         }
      },

      {
         /* Device Type */
         BP_VD_ZARLINK_88601,
         BP_VDTYPE_PCM,
         BP_SPI_SS_B4,  /* Device uses SPI_SS_B1 pin. Pin on base board depends on base board parameters. */
         BP_RESET_FXS4, /* Device uses FXS4 reset pin. Pin on base board depends on base board parameters. */
         /* Channel Description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(12, 13) ),
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(14, 15) ),
         }
      },

      /* Always end device list with this macro. */
      BP_NULL_DEVICE_MACRO,
   },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,
   /* General-purpose flags */
   ( BP_FLAG_DSP_PCMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_ZL88702_ZSI =
{
   VOICECFG_ZL88702_ZSI_STR,   /* szBoardId */
   {
      {
         /* Device Type */
         BP_VD_ZARLINK_88702_ZSI,
         BP_VDTYPE_PCM,
         BP_SPI_SS_NOT_REQUIRED,  /* Device uses SPI_SS_B1 pin. Pin on base board depends on base board parameters. */
         BP_RESET_FXS1, /* Device uses FXS1 reset pin. Pin on base board depends on base board parameters. */
         /* Channel Description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(0, 1) ),
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(2, 3) ),
         }
      },

      /* Always end device list with this macro. */
      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_VD_FLYBACK,
   /* General-purpose flags */
   ( BP_FLAG_ZSI_SUPPORT | BP_FLAG_DSP_PCMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_LE9672_ZSI = /* for ZLRR96741H Rev A0 daughter card */
{
   VOICECFG_LE9672_ZSI_STR,   /* szBoardId */
   {
      {
         /* Device Type */
         BP_VD_ZARLINK_9672_ZSI,
         BP_VDTYPE_PCM,
         BP_SPI_SS_NOT_REQUIRED,  /* Device uses SPI_SS_B1 pin. Pin on base board depends on base board parameters. */
         BP_RESET_FXS1, /* Device uses FXS1 reset pin. Pin on base board depends on base board parameters. */
         /* Channel Description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(0, 1) ),
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(2, 3) ),
         }
      },

      /* Always end device list with this macro. */
      BP_NULL_DEVICE_MACRO
   },
   /* SLIC Device Profile */
   BP_VD_INVBOOST,
   /* General-purpose flags */
   ( BP_FLAG_ZSI_SUPPORT | BP_FLAG_DSP_PCMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_LE9661_ZSI =
{
   VOICECFG_LE9661_ZSI_STR,   /* szBoardId */
   {
      {
         /* Device Type */
         BP_VD_ZARLINK_9661,
         BP_VDTYPE_PCM,
         BP_SPI_SS_NOT_REQUIRED,  /* ZSI SPI CS handled internally. It is mapped using the zsiMapList */
         BP_RESET_FXS1, /* Device uses FXS1 reset pin. Pin on base board depends on base board parameters. */
         /* Channel Description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(0, 1) ),
            BP_CHAN( BP_VC_INACTIVE, BP_VCTYPE_NONE, BP_VC_COMP_LINEAR, BP_VC_8KHZ ),
         }
      },

      /* Always end device list with this macro. */
      BP_NULL_DEVICE_MACRO
   },
   /* SLIC Device Profile */
   BP_VD_FLYBACK,
   /* General-purpose flags */
   ( BP_FLAG_ZSI_SUPPORT | BP_FLAG_DSP_PCMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_LE9661_ZSI_BB =
{
   VOICECFG_LE9661_ZSI_BB_STR,   /* szBoardId */
   {
      {
         /* Device Type */
         BP_VD_ZARLINK_9661,
         BP_VDTYPE_PCM,
         BP_SPI_SS_NOT_REQUIRED,  /* ZSI SPI CS handled internally. It is mapped using the zsiMapList */
         BP_RESET_FXS1, /* Device uses FXS1 reset pin. Pin on base board depends on base board parameters. */

         /* Channel Description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE,   BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(0, 1) ),
            BP_CHAN_PCM( BP_VC_INACTIVE, BP_VCTYPE_NONE, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(2, 3) ),
         }
      },

      /* Always end device list with this macro. */
      BP_NULL_DEVICE_MACRO
   },
   /* SLIC Device Profile */
   BP_VD_BUCKBOOST,
   /* General-purpose flags */
   ( BP_FLAG_ZSI_SUPPORT | BP_FLAG_DSP_PCMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_ZL88801_89010_BB = /* for Microsemi ZLR88842L REV A0 DC */
{
   VOICECFG_ZL88801_89010_BB_STR,   /* szBoardId */
   {
      {
         /* Device Type */
         BP_VD_ZARLINK_88801,
         BP_VDTYPE_PCM,
         BP_SPI_SS_B1,  /* Device uses SPI_SS_B1 pin. Pin on base board depends on base board parameters. */
         BP_RESET_FXS1, /* Device uses FXS1 reset pin. Pin on base board depends on base board parameters. */
         /* Channel Description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(0, 1) ),
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(2, 3) ),
         }
      },
      {
         /* Device Type 2 */
         BP_VD_ZARLINK_89010,
         BP_VDTYPE_PCM,
         BP_SPI_SS_B2,  /* Device uses SPI_SS_B2 pin. Pin on base board depends on base board parameters. */
         BP_RESET_NOT_REQUIRED,  /* FXO reset pin tied with FXS on this board.*/
         /* Channel description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_DAA,  BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(4, 5) ),
            BP_CHAN( BP_VC_INACTIVE, BP_VCTYPE_NONE, BP_VC_COMP_LINEAR, BP_VC_8KHZ ),
         }
      },

      /* Always end device list with this macro. */
      BP_NULL_DEVICE_MACRO
   },
   /* SLIC Device Profile */
   BP_VD_BUCKBOOST,
   /* General-purpose flags */
   ( BP_FLAG_DSP_PCMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_LE9662_ZSI =
{
   VOICECFG_LE9662_ZSI_STR,   /* szBoardId */
   {
      {
         /* Device Type */
         BP_VD_ZARLINK_9662,
         BP_VDTYPE_PCM,
         BP_SPI_SS_NOT_REQUIRED,  /* ZSI SPI CS handled internally. It is mapped using the zsiMapList */
         BP_RESET_FXS1, /* Device uses FXS1 reset pin. Pin on base board depends on base board parameters. */
         /* Channel Description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(0, 1) ),
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(2, 3) ),
         }
      },

      /* Always end device list with this macro. */
      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_VD_FLYBACK,
   /* General-purpose flags */
   ( BP_FLAG_ZSI_SUPPORT | BP_FLAG_DSP_PCMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_LE9662_ZSI_BB =
{
   VOICECFG_LE9662_ZSI_BB_STR,   /* szBoardId */
   {
      {
         /* Device Type */
         BP_VD_ZARLINK_9662,
         BP_VDTYPE_PCM,
         BP_SPI_SS_NOT_REQUIRED,  /* ZSI SPI CS handled internally. It is mapped using the zsiMapList */
         BP_RESET_FXS1, /* Device uses FXS1 reset pin. Pin on base board depends on base board parameters. */
         /* Channel Description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(0, 1) ),
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(2, 3) ),
         }
      },

      /* Always end device list with this macro. */
      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_VD_BUCKBOOST,
   /* General-purpose flags */
   ( BP_FLAG_ZSI_SUPPORT | BP_FLAG_DSP_PCMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_LE9642_ZSI_BB =
{
   VOICECFG_LE9642_ZSI_BB_STR,   /* szBoardId */
   {
      {
         /* Device Type */
         BP_VD_ZARLINK_9642_ZSI,
         BP_VDTYPE_PCM,
         BP_SPI_SS_NOT_REQUIRED,  /* ZSI SPI CS handled internally. It is mapped using the zsiMapList */
         BP_RESET_FXS1, /* Device uses FXS1 reset pin. Pin on base board depends on base board parameters. */
         /* Channel Description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(0, 1) ),
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(2, 3) ),
         }
      },

      /* Always end device list with this macro. */
      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_VD_BUCKBOOST,
   /* General-purpose flags */
   ( BP_FLAG_ZSI_SUPPORT | BP_FLAG_DSP_PCMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_LE9643_ZSI_IB =
{
   VOICECFG_LE9643_ZSI_IB_STR,   /* szBoardId */
   {
      {
         /* Device Type */
         BP_VD_ZARLINK_9643_ZSI_IB,
         BP_VDTYPE_PCM,
         BP_SPI_SS_NOT_REQUIRED,  /* ZSI SPI CS handled internally. It is mapped using the zsiMapList */
         BP_RESET_NOT_REQUIRED, /* Device uses FXS1 reset pin. Pin on base board depends on base board parameters. */
         /* Channel Description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(0, 1) ),
            BP_CHAN( BP_VC_INACTIVE, BP_VCTYPE_NONE, BP_VC_COMP_LINEAR, BP_VC_8KHZ ),
         }
      },

      /* Always end device list with this macro. */
      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_VD_INVBOOST,
   /* General-purpose flags */
   ( BP_FLAG_ZSI_SUPPORT | BP_FLAG_DSP_PCMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_LE9641_ZSI_BB =
{
   VOICECFG_LE9641_ZSI_BB_STR,   /* szBoardId */
   {
      {
         /* Device Type */
         BP_VD_ZARLINK_9641_ZSI,
         BP_VDTYPE_PCM,
         BP_SPI_SS_NOT_REQUIRED,  /* ZSI SPI CS handled internally. It is mapped using the zsiMapList */
         BP_RESET_FXS1, /* Device uses FXS1 reset pin. Pin on base board depends on base board parameters. */

         /* Channel Description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(0, 1) ),
            BP_CHAN( BP_VC_INACTIVE, BP_VCTYPE_NONE, BP_VC_COMP_LINEAR, BP_VC_8KHZ ),
         }
      },

      /* Always end device list with this macro. */
      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_VD_BUCKBOOST,
   /* General-purpose flags */
   ( BP_FLAG_ZSI_SUPPORT | BP_FLAG_DSP_PCMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_LE9652_ZSI_IB =
{
   VOICECFG_LE9652_ZSI_IB_STR,   /* szBoardId */
   {
      {
         /* Device Type */
         BP_VD_ZARLINK_9652_ZSI_IB,
         BP_VDTYPE_PCM,
         BP_SPI_SS_NOT_REQUIRED,  /* ZSI SPI CS handled internally. It is mapped using the zsiMapList */
         BP_RESET_FXS1, /* Device uses FXS1 reset pin. Pin on base board depends on base board parameters. */

         /* Channel Description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(0, 1) ),
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(2, 3) ),
         }
      },

      /* Always end device list with this macro. */
      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_VD_INVBOOST,
   /* General-purpose flags */
   ( BP_FLAG_ZSI_SUPPORT | BP_FLAG_DSP_PCMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_LE9653_IB =
{
   VOICECFG_LE9653_IB_STR,   /* szBoardId */
   {
      {
         /* Device Type */
         BP_VD_ZARLINK_9653_IB,
         BP_VDTYPE_PCM,
         BP_SPI_SS_B2,  /* ZSI SPI CS handled internally. It is mapped using the zsiMapList */
         BP_RESET_NOT_REQUIRED, /* Device don't uses reset pin. Pin on base board depends on base board parameters. */

         /* Channel Description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(0, 1) ),
            BP_CHAN( BP_VC_INACTIVE, BP_VCTYPE_NONE, BP_VC_COMP_LINEAR, BP_VC_8KHZ ),
         }
      },

      /* Always end device list with this macro. */
      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_VD_INVBOOST,
   /* General-purpose flags */
   ( BP_FLAG_DSP_PCMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_LE89116 =
{
   VOICECFG_LE89116_STR,   /* Daughter Card ID */
   {
      {
         /* Device Type */
         BP_VD_ZARLINK_89116,
         BP_VDTYPE_PCM,
         BP_SPI_SS_B1,  /* Device uses SPI_SS_B1 pin. Pin on base board depends on base board parameters. */
         BP_RESET_FXS1, /* Device uses FXS1 reset pin. Pin on base board depends on base board parameters. */
         /* Channel description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(0, 1) ),
            BP_CHAN( BP_VC_INACTIVE, BP_VCTYPE_NONE, BP_VC_COMP_LINEAR, BP_VC_8KHZ ),
         }
      },

      /* Always end device list with this macro. */
      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_VD_INVBOOST,
   /* General-purpose flags */
   ( BP_FLAG_DSP_PCMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_SI32176 =
{
   VOICECFG_SI32176_STR,   /* Daughter Board ID */
   {
      {
         /* Device Type */
         BP_VD_SILABS_32176,
         BP_VDTYPE_PCM,
         BP_SPI_SS_B1,  /* Device uses SPI_SS_B1 pin. Pin on base board depends on base board parameters. */
         BP_RESET_FXS2, /* Device uses FXS2 reset pin. Pin on base board depends on base board parameters. */
         /* Channel description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(0, 1) ),
            BP_CHAN( BP_VC_INACTIVE, BP_VCTYPE_NONE, BP_VC_COMP_LINEAR, BP_VC_8KHZ ),
         }
      },

      /* Always end device list with this macro. */
      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_VD_FLYBACK,
   /* General-purpose flags */
   ( BP_FLAG_DSP_PCMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_VE890HV =
{
   VOICECFG_VE890HV_STR,   /*Daughter Card ID */
   {
      {
         /* Device type */
         BP_VD_ZARLINK_89136,
         BP_VDTYPE_PCM,
         BP_SPI_SS_B1,  /* Device uses SPI_SS_B1 pin. Pin on base board depends on base board parameters. */
         BP_RESET_FXS1, /* Device uses FXS1 reset pin. Pin on base board depends on base board parameters. */
         /* Channel description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(0, 1) ),
            BP_CHAN( BP_VC_INACTIVE, BP_VCTYPE_NONE, BP_VC_COMP_LINEAR, BP_VC_8KHZ ),
         }
      },
      {
         /* Device type 2 */
         BP_VD_ZARLINK_89336,
         BP_VDTYPE_PCM,
         BP_SPI_SS_B2,  /* Device uses SPI_SS_B2 pin. Pin on base board depends on base board parameters. */
         BP_RESET_FXS2, /* Device uses FXS2 reset pin. Pin on base board depends on base board parameters. */
         /* Channel description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(2, 3) ),
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_DAA,  BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(4, 5) ),
         }
      },

      /* Always end the device list with this macro. */
      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_VD_INVBOOST,
   /* General-purpose flags */
   ( BP_FLAG_DSP_PCMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_LE89316 =
{
   VOICECFG_LE89316_STR,   /* Daughter Card ID */
   {
      {
         /* Device Type */
         BP_VD_ZARLINK_89316,
         BP_VDTYPE_PCM,
         BP_SPI_SS_B2,  /* Device uses SPI_SS_B2 pin. Pin on base board depends on base board parameters. */
         BP_RESET_FXS2, /* Device uses FXS2 reset pin. Pin on base board depends on base board parameters. */
         /* Channel description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(0, 1) ),
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_DAA,  BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(2, 3) ),
         }
      },

      /* Always end device list with this macro. */
      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_VD_INVBOOST,
   /* General-purpose flags */
   ( BP_FLAG_DSP_PCMHAL_ENABLE )

};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_SI32178 =
{
   VOICECFG_SI32178_STR,   /* Daughter Board ID */
   {
      {
         /* Device Type */
         BP_VD_SILABS_32178,
         BP_VDTYPE_PCM,
         BP_SPI_SS_B2,  /* Device uses SPI_SS_B2 pin. Pin on base board depends on base board parameters. */
         BP_RESET_FXS2, /* Device uses FXS2 reset pin. Pin on base board depends on base board parameters. */
         /* Channel description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(0, 1) ),
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_DAA,  BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(2, 3) ),
         }
      },

      /* Always end device list with this macro. */
      BP_NULL_DEVICE_MACRO,
   },

   /* SLIC Device Profile */
    BP_VD_FLYBACK,
    /* General-purpose flags */
    ( BP_FLAG_DSP_PCMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_NOSLIC =
{
   VOICECFG_NOSLIC_STR, /*Daughter Board ID */
   {
      BP_NULL_DEVICE_MACRO,
   },

   /* SLIC Device Profile */
   BP_NOT_DEFINED,
   /* General-purpose flags */
   ( 0 )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_SI32267_NTR =
{
   VOICECFG_SI32267_NTR_STR,   /* Daughter Board ID */
   {
      {
         /* Device Type */
         BP_VD_SILABS_32267,
         BP_VDTYPE_PCM,
         BP_SPI_SS_NOT_REQUIRED, /* ISI SPI CS handled internally. It is mapped by the zsiChipMap list. */
         BP_RESET_FXS1,          /* Device uses FXS1 reset pin. Pin on base board depends on base board parameters. */
         /* Channel description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(0, 1) ),
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(2, 3) ),
         }
      },

      /* Always end the device list with this macro. */
      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_VD_FB_TSS_ISO,
   /* General-purpose flags */
   ( BP_FLAG_ISI_SUPPORT | BP_FLAG_DSP_PCMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_SI32260x2_SI3050 =
{
   VOICECFG_SI32260x2_SI3050_STR,   /*Daughter card ID */
   {
      {
         /* Device Type */
         BP_VD_SILABS_32261,
         BP_VDTYPE_PCM,
         BP_SPI_SS_B1,  /* Device uses SPI_SS_B1 pin. Pin on base board depends on base board parameters. */
         BP_RESET_FXS1, /* Device uses FXS1 reset pin. Pin on base board depends on base board parameters. */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(0, 1) ),
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(2, 3) ),
         }
      },
      {
         /* Device Type 2 */
         BP_VD_SILABS_32261,
         BP_VDTYPE_PCM,
         BP_SPI_SS_B2,  /* Device uses SPI_SS_B2 pin. Pin on base board depends on base board parameters. */
         BP_RESET_FXS2, /* Device uses FXS2 reset pin. Pin on base board depends on base board parameters. */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(4, 5) ),
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(6, 7) ),
         }
      },
      {
         /* Device Type 3 */
         BP_VD_SILABS_3050,
         BP_VDTYPE_PCM,
         BP_SPI_SS_B3,  /* Device uses SPI_SS_B3 pin. Pin on base board depends on base board parameters. */
         BP_RESET_FXO,  /* Device uses FXO reset pin. Pin on base board depends on base board parameters. */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_DAA,  BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(8, 9) ),
            BP_CHAN( BP_VC_INACTIVE, BP_VCTYPE_NONE, BP_VC_COMP_LINEAR, BP_VC_8KHZ ),
         }
      },

      /* Always end device list with this macro. */
      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_VD_FB_TSS,
   /* General-purpose flags */
   ( BP_FLAG_DSP_PCMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_LE9530 =
{
   VOICECFG_LE9530_STR,   /* daughter card ID */
   {  /* voiceDevice0 parameters */
      {
         /* Device Type */
         BP_VD_ZARLINK_9530,
         BP_VDTYPE_APM,
         BP_SPI_SS_B1,           /* Device uses SPI_SS_B1 pin. Pin on base board depends on base board parameters. */
         BP_RESET_NOT_REQUIRED,  /* Device does not require a reset pin. */
         {
            BP_CHAN_APM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, BP_APM_CHAN_A ),
            BP_CHAN_APM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, BP_APM_CHAN_B ),
         }
      },

      /* Always end device list with this macro. */
      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_NOT_DEFINED,
   /* General-purpose flags */
   ( BP_FLAG_DSP_APMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_LE9530_WB =
{
   VOICECFG_LE9530_WB_STR, /* daughter card ID */
   {   /* voiceDevice0 parameters */
      {
         /* Device Type */
         BP_VD_ZARLINK_9530,
         BP_VDTYPE_APM,
         BP_SPI_SS_B1,           /* Device uses SPI_SS_B1 pin. Pin on base board depends on base board parameters. */
         BP_RESET_NOT_REQUIRED,  /* Device does not require a reset pin. */
         {
            BP_CHAN_APM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_16KHZ, BP_APM_CHAN_A ),
            BP_CHAN_APM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_16KHZ, BP_APM_CHAN_B ),
         }
      },

      /* Always end device list with this macro. */
      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_NOT_DEFINED,
   /* General-purpose flags */
   ( BP_FLAG_DSP_APMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_LE9540 =
{
   VOICECFG_LE9540_STR,   /* daughter card ID */
   {  /* voiceDevice0 parameters */
      {
         /* Device Type */
         BP_VD_ZARLINK_9540,
         BP_VDTYPE_APM,
         BP_SPI_SS_B1,           /* Device uses SPI_SS_B1 pin. Pin on base board depends on base board parameters. */
         BP_RESET_NOT_REQUIRED,  /* Device does not require a reset pin. */
         {
            BP_CHAN_APM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, BP_APM_CHAN_A ),
            BP_CHAN_APM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, BP_APM_CHAN_B ),
         }
      },

      /* Always end device list with this macro. */
      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_NOT_DEFINED,
   /* General-purpose flags */
   ( BP_FLAG_DSP_APMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_LE9540_WB =
{
   VOICECFG_LE9540_WB_STR, /* daughter card ID */
   {   /* voiceDevice0 parameters */
      {
         /* Device Type */
         BP_VD_ZARLINK_9540,
         BP_VDTYPE_APM,
         BP_SPI_SS_B1,           /* Device uses SPI_SS_B1 pin. Pin on base board depends on base board parameters. */
         BP_RESET_NOT_REQUIRED,  /* Device does not require a reset pin. */
         {
            BP_CHAN_APM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_16KHZ, BP_APM_CHAN_A ),
            BP_CHAN_APM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_16KHZ, BP_APM_CHAN_B ),
         }
      },

      /* Always end device list with this macro. */
      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_NOT_DEFINED,
   /* General-purpose flags */
   ( BP_FLAG_DSP_APMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_LE9541 =
{
   VOICECFG_LE9541_STR,   /* daughter card ID */
   {  /* voiceDevice0 parameters */
      {
         /* Device Type */
         BP_VD_ZARLINK_9541,
         BP_VDTYPE_APM,
         BP_SPI_SS_B1,           /* Device uses SPI_SS_B1 pin. Pin on base board depends on base board parameters. */
         BP_RESET_NOT_REQUIRED,  /* Device does not require a reset pin. */
         {
            BP_CHAN_APM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, BP_APM_CHAN_A ),
            BP_CHAN_APM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, BP_APM_CHAN_B ),
         }
      },

      /* Always end device list with this macro. */
      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_NOT_DEFINED,
   /* General-purpose flags */
   ( BP_FLAG_DSP_APMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_LE9541_WB =
{
   VOICECFG_LE9541_WB_STR, /* daughter card ID */
   {   /* voiceDevice0 parameters */
      {
         /* Device Type */
         BP_VD_ZARLINK_9541,
         BP_VDTYPE_APM,
         BP_SPI_SS_B1,           /* Device uses SPI_SS_B1 pin. Pin on base board depends on base board parameters. */
         BP_RESET_NOT_REQUIRED,  /* Device does not require a reset pin. */
         {
            BP_CHAN_APM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_16KHZ, BP_APM_CHAN_A ),
            BP_CHAN_APM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_16KHZ, BP_APM_CHAN_B ),
         }
      },

      /* Always end device list with this macro. */
      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_NOT_DEFINED,
   /* General-purpose flags */
   ( BP_FLAG_DSP_APMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_SI3239 =
{
   VOICECFG_SI3239_STR,   /* daughter card ID */
   {   /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_SILABS_3239,
         BP_VDTYPE_APM,
         BP_SPI_SS_B1,           /* Device uses SPI_SS_B1 pin. Pin on base board depends on base board parameters. */
         BP_RESET_NOT_REQUIRED,  /* Device does not require a reset pin. */
         {
            BP_CHAN_APM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, BP_APM_CHAN_A ),
            BP_CHAN_APM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, BP_APM_CHAN_B ),
         }
      },

      /* Always end device list with this macro. */
      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_NOT_DEFINED,
   /* General-purpose flags */
   ( BP_FLAG_DSP_APMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_SI32392 =
{
   VOICECFG_SI32392_STR,   /* daughter card ID */
   {   /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32392,
         BP_VDTYPE_APM,
         BP_SPI_SS_NOT_REQUIRED,  /* Device doesn't use SPI_SS_Bx pin. Hard coded in board HAL. */
         BP_RESET_NOT_REQUIRED,  /* Device does not require a reset pin. */
         {
            BP_CHAN_APM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, BP_APM_CHAN_A ),
            BP_CHAN_APM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, BP_APM_CHAN_B ),
         }
      },

      /* Always end device list with this macro. */
      BP_NULL_DEVICE_MACRO,
   },
   /* SLIC Device Profile */
   BP_NOT_DEFINED,
   /* General-purpose flags */
   ( BP_FLAG_DSP_APMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_LE9530_LE88506 =
{
   VOICECFG_LE9530_LE88506_STR,   /* Daughter board ID*/
   {   /*voiceDevice0 Params */
      {
         /* Device Type */
         BP_VD_ZARLINK_88506,
         BP_VDTYPE_PCM,
         BP_SPI_SS_B1,  /* Device uses SPI_SS_B1 pin. Pin on base board depends on base board parameters. */
         BP_RESET_FXS1, /* Device uses FXS1 reset pin. Pin on base board depends on base board parameters. */
         /* Channel description */
         {
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(0, 1) ),
            BP_CHAN_PCM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, TS(2, 3) ),
         }
      },
      {
         /* Device Type 2 */
         BP_VD_ZARLINK_9530,
         BP_VDTYPE_APM,
         BP_SPI_SS_B1,           /*The 9530 chips are actually internal, device ID is always 0. */
         BP_RESET_NOT_REQUIRED,  /* Device does not require a reset pin. */
         /* Channel description */
         {
            BP_CHAN_APM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, BP_APM_CHAN_A ),
            BP_CHAN_APM( BP_VC_ACTIVE, BP_VCTYPE_SLIC, BP_VC_COMP_LINEAR, BP_VC_8KHZ, BP_APM_CHAN_B ),
         }
      },

     /* Always end device list with this macro. */
      BP_NULL_DEVICE_MACRO,
   },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,
   /* General-purpose flags */
   ( BP_FLAG_DSP_APMHAL_ENABLE )
};

BP_VOICE_CHANNEL_INTERNAL g_iDectStdCfg[BP_MAX_DECT_DEVICE * BP_MAX_CHANNELS_PER_DEVICE] =
{
   BP_CHAN( BP_VC_ACTIVE, BP_VCTYPE_DECT, BP_VC_COMP_LINEAR, BP_VC_16KHZ ),
   BP_CHAN( BP_VC_ACTIVE, BP_VCTYPE_DECT, BP_VC_COMP_LINEAR, BP_VC_16KHZ ),
   BP_CHAN( BP_VC_ACTIVE, BP_VCTYPE_DECT, BP_VC_COMP_LINEAR, BP_VC_16KHZ ),
   BP_CHAN( BP_VC_ACTIVE, BP_VCTYPE_DECT, BP_VC_COMP_LINEAR, BP_VC_16KHZ ),
};


/* End of Daughter Card Definitions */


/*
 * -------------------------- Voice Mother Board Configs ------------------------------
 */
#if defined(_BCM963268_) || defined(CONFIG_BCM963268)

static VOICE_DAUGHTER_BOARD_PARMS * g_963168_dCardList_full[] = {
   SLICSLAC_LIST_COMMON_FXO,
   SLICSLAC_LIST_ZSI_FXO,
   SLICSLAC_LIST_4FXS_FXO,
   0
};

static VOICE_DAUGHTER_BOARD_PARMS * g_963168_dCardList_noFxoRst[] = {
   SLICSLAC_LIST_COMMON_NOFXO,
   SLICSLAC_LIST_4FXS_NOFXO,
   0
};

static bp_elem_t g_voice_bcm963168mbv_17a[] = {
   {bp_cpBoardId,               .u.cp = "963168MBV_17A"},
   {bp_usFxsFxo1SpiSSNum,       .u.us = SPI_DEV_1},
   {bp_usFxsFxo2SpiSSNum,       .u.us = SPI_DEV_4},
   {bp_usFxsFxo3SpiSSNum,       .u.us = SPI_DEV_7},
   {bp_usGpioFxsFxoRst1,        .u.us = BP_GPIO_14_AL},
   {bp_usGpioFxsFxoRst2,        .u.us = BP_GPIO_15_AL},
   {bp_usGpioFxsFxoRst3,        .u.us = BP_GPIO_23_AL},
   {bp_usGpioDectRst,           .u.us = BP_GPIO_35_AL},
   {bp_usGpioVoipRelayCtrl1,    .u.us = BP_GPIO_8_AH},
   {bp_iDectCfg,                .u.ptr = g_iDectStdCfg},
   {bp_daughterCardList,        .u.ptr = g_963168_dCardList_full},
   {bp_last}
};

static bp_elem_t g_voice_bcm963168mbv_30a[] = {
   {bp_cpBoardId,               .u.cp = "963168MBV_30A"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm963168mbv_17a},
   {bp_last}
};

static bp_elem_t g_voice_bcm963168mbv17a302[] = {
   {bp_cpBoardId,               .u.cp = "963168MBV17A302"},
   {bp_usGpioFxsFxoRst3,        .u.us = BP_GPIO_10_AL},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm963168mbv_17a},
   {bp_last}
};

static bp_elem_t g_voice_bcm963168mbv30a302[] = {
   {bp_cpBoardId,               .u.cp = "963168MBV30A302"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm963168mbv17a302},
   {bp_last}
};

static bp_elem_t g_voice_bcm963168xh[] = {
   {bp_cpBoardId,               .u.cp = "963168XH"},
   {bp_usGpioFxsFxoRst3,        .u.us = BP_GPIO_21_AL},
   {bp_usGpioVoipRelayCtrl1,    .u.us = BP_GPIO_39_AH},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm963168mbv_17a},
   {bp_last}
};

static bp_elem_t g_voice_bcm963168xh5[] = {
   {bp_cpBoardId,               .u.cp = "963168XH5"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm963168xh},
   {bp_last}
};

static bp_elem_t g_voice_bcm963168mp[] = {
   {bp_cpBoardId,               .u.cp = "963168MP"},
   {bp_usGpioFxsFxoRst3,        .u.us = BP_NOT_DEFINED},
   {bp_usGpioDectRst,           .u.us = BP_GPIO_19_AL},
   {bp_usGpioVoipRelayCtrl1,    .u.us = BP_GPIO_39_AH},
   {bp_daughterCardList,        .u.ptr = g_963168_dCardList_noFxoRst},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm963168mbv_17a},
   {bp_last}
};

static bp_elem_t g_voice_bcm963168mbv3[] = {
   {bp_cpBoardId,               .u.cp = "963168MBV3"},
   {bp_usFxsFxo1SpiSSNum,       .u.us = SPI_DEV_1},
   {bp_usFxsFxo2SpiSSNum,       .u.us = SPI_DEV_4},
   {bp_usFxsFxo3SpiSSNum,       .u.us = SPI_DEV_5},
   {bp_usGpioFxsFxoRst1,        .u.us = BP_GPIO_14_AL},
   {bp_usGpioFxsFxoRst2,        .u.us = BP_GPIO_15_AL},
   {bp_usGpioFxsFxoRst3,        .u.us = BP_GPIO_23_AL},
   {bp_usGpioDectRst,           .u.us = BP_GPIO_35_AL},
   {bp_usGpioVoipRelayCtrl1,    .u.us = BP_GPIO_12_AH},
   {bp_iDectCfg,                .u.ptr = g_iDectStdCfg},
   {bp_daughterCardList,        .u.ptr = g_963168_dCardList_full},
   {bp_last}
};

static bp_elem_t g_voice_bcm963168plc[] = {
   {bp_cpBoardId,               .u.cp = "963168PLC"},
   {bp_usFxsFxo1SpiSSNum,       .u.us = SPI_DEV_1},
   {bp_usFxsFxo2SpiSSNum,       .u.us = SPI_DEV_4},
   {bp_usFxsFxo3SpiSSNum,       .u.us = SPI_DEV_5},
   {bp_usGpioFxsFxoRst1,        .u.us = BP_GPIO_14_AL},
   {bp_usGpioFxsFxoRst2,        .u.us = BP_GPIO_15_AL},
   {bp_usGpioFxsFxoRst3,        .u.us = BP_GPIO_23_AL},
   {bp_usGpioVoipRelayCtrl1,    .u.us = BP_GPIO_12_AH},
   {bp_daughterCardList,        .u.ptr = g_963168_dCardList_full},
   {bp_last}
};

static bp_elem_t g_voice_bcm963168plc_mocawan[] = {
   {bp_cpBoardId,               .u.cp = "963168PLC_MOCAWAN"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm963168plc},
   {bp_last}
};

static bp_elem_t g_voice_bcm963268v30a[] = {
   {bp_cpBoardId,               .u.cp = "963268V30A"},
   {bp_usGpioFxsFxoRst3,        .u.us = BP_GPIO_50_AL},
   {bp_usGpioDectRst,           .u.us = BP_GPIO_51_AL},
   {bp_usGpioVoipRelayCtrl1,    .u.us = BP_GPIO_39_AH},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm963168mbv3},
   {bp_last}
};

static bp_elem_t g_voice_bcm963268bu[] = {
   {bp_cpBoardId,               .u.cp = "963268BU"},
   {bp_usGpioFxsFxoRst3,        .u.us = BP_GPIO_18_AL},
   {bp_usGpioDectRst,           .u.us = BP_GPIO_19_AL},
   {bp_usGpioVoipRelayCtrl1,    .u.us = BP_GPIO_39_AH},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm963168mbv3},
   {bp_last}
};

static bp_elem_t g_voice_bcm963268bu_p300[] = {
   {bp_cpBoardId,               .u.cp = "963268BU_P300"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm963268bu},
   {bp_last}
};

static bp_elem_t g_voice_bcm963168vx[] = {
   {bp_cpBoardId,               .u.cp = "963168VX"},
   {bp_usFxsFxo1SpiSSNum,       .u.us = SPI_DEV_1},
   {bp_usFxsFxo2SpiSSNum,       .u.us = SPI_DEV_4},
   {bp_usFxsFxo3SpiSSNum,       .u.us = SPI_DEV_5},
   {bp_usGpioFxsFxoRst1,        .u.us = BP_GPIO_14_AL},
   {bp_usGpioFxsFxoRst2,        .u.us = BP_GPIO_15_AL},
   {bp_usGpioFxsFxoRst3,        .u.us = BP_GPIO_9_AL},
   {bp_usGpioVoipRelayCtrl1,    .u.us = BP_GPIO_8_AH},
   {bp_daughterCardList,        .u.ptr = g_963168_dCardList_full},
   {bp_last}
};

static bp_elem_t g_voice_bcm963168vx_p400[] = {
   {bp_cpBoardId,               .u.cp = "963168VX_P400"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm963168vx},
   {bp_last}
};

static bp_elem_t g_voice_bcm963168xn5[] = {
   {bp_cpBoardId,               .u.cp = "963168XN5"},
   {bp_usFxsFxo1SpiSSNum,       .u.us = SPI_DEV_1},
   {bp_usFxsFxo2SpiSSNum,       .u.us = SPI_DEV_4},
   {bp_usFxsFxo3SpiSSNum,       .u.us = SPI_DEV_7},
   {bp_usGpioFxsFxoRst1,        .u.us = BP_GPIO_14_AL},
   {bp_usGpioFxsFxoRst2,        .u.us = BP_GPIO_18_AL},
   {bp_usGpioFxsFxoRst3,        .u.us = BP_GPIO_8_AL},
   {bp_usGpioVoipRelayCtrl1,    .u.us = BP_GPIO_39_AH},
   {bp_daughterCardList,        .u.ptr = g_963168_dCardList_full},
   {bp_last}
};

static bp_elem_t g_voice_bcm963168wfar[] = {
   {bp_cpBoardId,               .u.cp = "963168WFAR"},
   {bp_usGpioFxsFxoRst2,        .u.us = BP_GPIO_15_AL},
   {bp_usGpioFxsFxoRst3,        .u.us = BP_GPIO_10_AL},
   {bp_usGpioVoipRelayCtrl1,    .u.us = BP_NOT_DEFINED},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm963168xn5},
   {bp_last}
};

static bp_elem_t g_voice_bcm963168ac5[] = {
   {bp_cpBoardId,               .u.cp = "963168AC5"},
   {bp_usGpioFxsFxoRst2,        .u.us = BP_GPIO_15_AL},
   {bp_usGpioFxsFxoRst3,        .u.us = BP_GPIO_21_AL},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm963168xn5},
   {bp_last}
};

static bp_elem_t g_voice_bcm963268sv1[] = {
   {bp_cpBoardId,               .u.cp = "963268SV1"},
   {bp_usFxsFxo1SpiSSNum,       .u.us = SPI_DEV_1},
   {bp_usFxsFxo2SpiSSNum,       .u.us = SPI_DEV_4},
   {bp_usFxsFxo3SpiSSNum,       .u.us = SPI_DEV_3},
   {bp_usGpioFxsFxoRst1,        .u.us = BP_GPIO_14_AL},
   {bp_usGpioFxsFxoRst2,        .u.us = BP_GPIO_15_AL},
   {bp_usGpioDectRst,           .u.us = BP_GPIO_35_AL},
   {bp_usGpioVoipRelayCtrl1,    .u.us = BP_GPIO_8_AH},
   {bp_iDectCfg,                .u.ptr = g_iDectStdCfg},
   {bp_daughterCardList,        .u.ptr = g_963168_dCardList_noFxoRst},
   {bp_last}
};

static bp_elem_t * g_VoiceBoardParms[]=
{
   g_voice_bcm963168mbv_17a,
   g_voice_bcm963168mbv_30a,
   g_voice_bcm963168mbv17a302,
   g_voice_bcm963168mbv30a302,
   g_voice_bcm963168xh,
   g_voice_bcm963168xh5,
   g_voice_bcm963168mp,
   g_voice_bcm963168mbv3,
   g_voice_bcm963268v30a,
   g_voice_bcm963268bu,
   g_voice_bcm963268bu_p300,
   g_voice_bcm963168vx,
   g_voice_bcm963168vx_p400,
   g_voice_bcm963168xn5,
   g_voice_bcm963168wfar,
   g_voice_bcm963168ac5,
   g_voice_bcm963268sv1,
   g_voice_bcm963168plc,
   g_voice_bcm963168plc_mocawan,
   0
};

#endif


#if defined(_BCM96838_) || defined(CONFIG_BCM96838)

static VOICE_DAUGHTER_BOARD_PARMS * g_968380_dCardList_Le9540[] = {
   &g_voiceBoard_NOSLIC,
   &g_voiceBoard_LE9540,
   0
};

static VOICE_DAUGHTER_BOARD_PARMS * g_968380_dCardList_Si32392[] = {
   &g_voiceBoard_NOSLIC,
   &g_voiceBoard_SI32392,
   0
};

static bp_elem_t g_voice_bcm968380fhgu[] = {
   {bp_cpBoardId,               .u.cp = "968380FHGU"},
   {bp_usZarIfEnable,           .u.us = BP_GPIO_0_AH},
   {bp_usZarIfSdin,             .u.us = BP_GPIO_1_AH},
   {bp_usZarIfSdout,            .u.us = BP_GPIO_2_AH},
   {bp_usZarIfSclk,             .u.us = BP_GPIO_3_AH},
   {bp_daughterCardList,        .u.ptr = g_968380_dCardList_Le9540},
   {bp_last}
};

static bp_elem_t g_voice_bcm968380fehg[] = {
   {bp_cpBoardId,               .u.cp = "968380FEHG"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm968380fhgu},
   {bp_last}
};

static bp_elem_t g_voice_bcm968380fggu[] = {
   {bp_cpBoardId,               .u.cp = "968380FGGU"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm968380fhgu},
   {bp_last}
};

static bp_elem_t g_voice_bcm968380fegu[] = {
   {bp_cpBoardId,               .u.cp = "968380FEGU"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm968380fhgu},
   {bp_last}
};

static bp_elem_t g_voice_bcm968380gerg[] = {
   {bp_cpBoardId,               .u.cp = "968380GERG"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm968380fhgu},
   {bp_last}
};

static bp_elem_t g_voice_bcm968380mgeg[] = {                 
   {bp_cpBoardId,               .u.cp = "968380MGEG"},           
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm968380gerg},
   {bp_last}                                                    
};  

static bp_elem_t g_voice_bcm968380gwan[] = {
   {bp_cpBoardId,               .u.cp = "968380GWAN"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm968380fhgu},
   {bp_last}
};

static bp_elem_t g_voice_bcm968380lte[] = {
   {bp_cpBoardId,               .u.cp = "968380LTE"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm968380fhgu},
   {bp_last}
};

static bp_elem_t g_voice_bcm968380eprg[] = {
   {bp_cpBoardId,               .u.cp = "968380EPRG"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm968380fhgu},
   {bp_last}
};

static bp_elem_t g_voice_bcm968380ffhg[] = {
   {bp_cpBoardId,               .u.cp = "968380FFHG"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm968380fhgu},
   {bp_last}
};

static bp_elem_t g_voice_bcm968380fesfu[] = {
   {bp_cpBoardId,               .u.cp = "968380FESFU"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm968380fhgu},
   {bp_last}
};

static bp_elem_t g_voice_bcm968385sfu[] = {
   {bp_cpBoardId,               .u.cp = "968385SFU"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm968380fhgu},
   {bp_last}
};

static bp_elem_t g_voice_bcm968385esfu[] = {
   {bp_cpBoardId,               .u.cp = "968385ESFU"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm968380fhgu},
   {bp_last}
};

static bp_elem_t g_voice_bcm968380fhgu_si[] = {
   {bp_cpBoardId,               .u.cp = "968380FHGU_SI"},
   {bp_usHvgMaxPwm,             .u.us = BP_GPIO_0_AH},
   {bp_usSi32392SpiSSNum,       .u.us = SPI_DEV_1},
   {bp_daughterCardList,        .u.ptr = g_968380_dCardList_Si32392},
   {bp_last}
};

static VOICE_DAUGHTER_BOARD_PARMS * g_968380fsv_g_dCardList[] = {
   SLICSLAC_LIST_COMMON_NOFXO,
   SLICSLAC_LIST_ZSI_FXO,
   &g_voiceBoard_LE9540,
   0
};

static bp_elem_t g_voice_bcm968380fsv_g[] = {
   {bp_cpBoardId,               .u.cp = "968380FSV_G"},
   {bp_usFxsFxo1SpiSSNum,       .u.us = SPI_DEV_4},
   {bp_usFxsFxo2SpiSSNum,       .u.us = SPI_DEV_6},
   {bp_usFxsFxo3SpiSSNum,       .u.us = SPI_DEV_7},
   {bp_usGpioFxsFxoRst1,        .u.us = BP_GPIO_5_AL},
   {bp_usGpioFxsFxoRst2,        .u.us = BP_GPIO_6_AL},
   {bp_usGpioFxsFxoRst3,        .u.us = BP_GPIO_7_AL},
   {bp_usZarIfEnable,           .u.us = BP_GPIO_40_AH},
   {bp_usZarIfSdin,             .u.us = BP_GPIO_6_AH},
   {bp_usZarIfSdout,            .u.us = BP_GPIO_42_AH},
   {bp_usZarIfSclk,             .u.us = BP_GPIO_41_AH},
   {bp_usGpioLe9540Reset,       .u.us = BP_GPIO_4_AL},
   {bp_usHvgMaxPwm,             .u.us = BP_GPIO_33_AH},
   {bp_usSi32392SpiSSNum,       .u.us = SPI_DEV_7},
   {bp_daughterCardList,        .u.ptr = g_968380fsv_g_dCardList},
   {bp_last}
};

static bp_elem_t g_voice_bcm968380fsv_e[] = {
   {bp_cpBoardId,               .u.cp = "968380FSV_E"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm968380fsv_g},
   {bp_last}
};

static bp_elem_t g_voice_bcm968380fsv_ge[] = {
   {bp_cpBoardId,               .u.cp = "968380FSV_GE"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm968380fsv_g},
   {bp_last}
};

static VOICE_DAUGHTER_BOARD_PARMS * g_968380gerg_si_dCardList[] = {
   SLICSLAC_LIST_COMMON_NOFXO,
   SLICSLAC_LIST_ZSI_FXO,
   &g_voiceBoard_SI32392,
   0
};

static VOICE_DAUGHTER_BOARD_PARMS * g_968381gerg_si_dCardList[] = {
   SLICSLAC_LIST_COMMON_NOFXO,
   SLICSLAC_LIST_ZSI_FXO,
   &g_voiceBoard_SI32392,
   0
};

static bp_elem_t g_voice_bcm968380gerg_si[] = {
   {bp_cpBoardId,               .u.cp = "968380GERG_SI"},
   {bp_usFxsFxo1SpiSSNum,       .u.us = SPI_DEV_0},
   {bp_usFxsFxo2SpiSSNum,       .u.us = SPI_DEV_1},
   {bp_usFxsFxo3SpiSSNum,       .u.us = SPI_DEV_2},
   {bp_usGpioFxsFxoRst1,        .u.us = BP_GPIO_36_AL},
   {bp_usGpioFxsFxoRst2,        .u.us = BP_GPIO_40_AL},
   {bp_usGpioFxsFxoRst3,        .u.us = BP_GPIO_38_AL},
   {bp_usHvgMaxPwm,             .u.us = BP_GPIO_0_AH},
   {bp_usSi32392SpiSSNum,       .u.us = SPI_DEV_1},
   {bp_daughterCardList,        .u.ptr = g_968380gerg_si_dCardList},
   {bp_last}
};

static bp_elem_t g_voice_bcm968380sv_g[] = {
   {bp_cpBoardId,               .u.cp = "968380SV_G"},
   {bp_usFxsFxo1SpiSSNum,       .u.us = SPI_DEV_4},
   {bp_usFxsFxo2SpiSSNum,       .u.us = SPI_DEV_6},
   {bp_usFxsFxo3SpiSSNum,       .u.us = SPI_DEV_7},
   {bp_usGpioFxsFxoRst1,        .u.us = BP_GPIO_5_AL},
   {bp_usGpioFxsFxoRst2,        .u.us = BP_GPIO_38_AL},
   {bp_usGpioFxsFxoRst3,        .u.us = BP_GPIO_7_AL},
   {bp_usZarIfEnable,           .u.us = BP_GPIO_40_AH},
   {bp_usZarIfSdin,             .u.us = BP_GPIO_6_AH},
   {bp_usZarIfSdout,            .u.us = BP_GPIO_42_AH},
   {bp_usZarIfSclk,             .u.us = BP_GPIO_41_AH},
   {bp_daughterCardList,        .u.ptr = g_968380_dCardList_Le9540},
   {bp_last}
};

static bp_elem_t g_voice_bcm968380sv_e[] = {
   {bp_cpBoardId,               .u.cp = "968380SV_E"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm968380sv_g},
   {bp_last}
};

static bp_elem_t g_voice_bcm968381sv_g[] = {
   {bp_cpBoardId,               .u.cp = "968381SV_G"},
   {bp_usFxsFxo1SpiSSNum,       .u.us = SPI_DEV_4},
   {bp_usFxsFxo2SpiSSNum,       .u.us = SPI_DEV_6},
   {bp_usFxsFxo3SpiSSNum,       .u.us = SPI_DEV_6},      // SPI3 and SPI2 are tied with SPI_6
   {bp_usGpioFxsFxoRst1,        .u.us = BP_GPIO_5_AL},
   {bp_usGpioFxsFxoRst2,        .u.us = BP_GPIO_6_AL},
   {bp_usGpioFxsFxoRst3,        .u.us = BP_GPIO_7_AL},
   {bp_daughterCardList,        .u.ptr = g_968381gerg_si_dCardList},
   {bp_last}
};  

static bp_elem_t g_voice_bcm968385sfu_si[] = {
   {bp_cpBoardId,               .u.cp = "968385SFU_SI"},
   {bp_usHvgMaxPwm,             .u.us = BP_GPIO_0_AH},
   {bp_usSi32392SpiSSNum,       .u.us = SPI_DEV_1},
   {bp_daughterCardList,        .u.ptr = g_968380_dCardList_Si32392},
   {bp_last}
};

static VOICE_DAUGHTER_BOARD_PARMS * g_968385sv_g_dCardList[] = {
   SLICSLAC_LIST_COMMON_NOFXO,
   SLICSLAC_LIST_ZSI_FXO,
   &g_voiceBoard_LE9540,
   0
};

static bp_elem_t g_voice_bcm968385sv_g[] = {
   {bp_cpBoardId,               .u.cp = "968385SV_G"},
   {bp_usFxsFxo1SpiSSNum,       .u.us = SPI_DEV_4},
   {bp_usFxsFxo2SpiSSNum,       .u.us = SPI_DEV_6},
   {bp_usFxsFxo3SpiSSNum,       .u.us = SPI_DEV_6},
   {bp_usGpioFxsFxoRst1,        .u.us = BP_GPIO_5_AL},
   {bp_usGpioFxsFxoRst2,        .u.us = BP_GPIO_6_AL},
   {bp_usGpioFxsFxoRst3,        .u.us = BP_GPIO_7_AL},
   {bp_usZarIfEnable,           .u.us = BP_GPIO_40_AH},
   {bp_usZarIfSdin,             .u.us = BP_GPIO_6_AH},
   {bp_usZarIfSdout,            .u.us = BP_GPIO_42_AH},
   {bp_usZarIfSclk,             .u.us = BP_GPIO_41_AH},
   {bp_daughterCardList,        .u.ptr = g_968385sv_g_dCardList},
   {bp_last}
};

static bp_elem_t g_voice_bcm968385sv_e[] = {
   {bp_cpBoardId,               .u.cp = "968385SV_E"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm968385sv_g},
   {bp_last}
};

static bp_elem_t * g_VoiceBoardParms[]=
{
   g_voice_bcm968380fhgu,
   g_voice_bcm968380fhgu_si,
   g_voice_bcm968380fehg,
   g_voice_bcm968380fggu,
   g_voice_bcm968380fegu,
   g_voice_bcm968380fsv_g,
   g_voice_bcm968380fsv_e,
   g_voice_bcm968380fsv_ge,
   g_voice_bcm968380gerg,
   g_voice_bcm968380mgeg,
   g_voice_bcm968380gwan,
   g_voice_bcm968380lte,
   g_voice_bcm968380gerg_si,
   g_voice_bcm968380eprg,
   g_voice_bcm968380ffhg,
   g_voice_bcm968380fesfu,
   g_voice_bcm968380sv_g,
   g_voice_bcm968380sv_e,
   g_voice_bcm968385sfu,
   g_voice_bcm968385sfu_si,
   g_voice_bcm968385esfu,
   g_voice_bcm968385sv_g,
   g_voice_bcm968385sv_e,
   g_voice_bcm968381sv_g,
   0
};

#endif


#if defined(_BCM963138_) || defined(CONFIG_BCM963138)

static VOICE_DAUGHTER_BOARD_PARMS * g_963138_dCardListFull[] = {
   SLICSLAC_LIST_COMMON_FXO,
   0
};

static VOICE_DAUGHTER_BOARD_PARMS * g_963138_dCardListNoFxoRst[] = {
   SLICSLAC_LIST_COMMON_NOFXO,
   0
};


static bp_elem_t g_voice_bcm963138dvt[] = {
   {bp_cpBoardId,               .u.cp = "963138DVT"},
   {bp_usFxsFxo1SpiSSNum,       .u.us = SPI_DEV_1},
   {bp_usFxsFxo2SpiSSNum,       .u.us = SPI_DEV_2},
   {bp_usFxsFxo3SpiSSNum,       .u.us = SPI_DEV_4},
   {bp_usGpioFxsFxoRst1,        .u.us = BP_GPIO_7_AL},
   {bp_usGpioFxsFxoRst2,        .u.us = BP_GPIO_19_AL},
   {bp_usGpioFxsFxoRst3,        .u.us = BP_GPIO_36_AL},
   {bp_usGpioDectRst,           .u.us = BP_GPIO_37_AL},
   {bp_usGpioVoipRelayCtrl1,    .u.us = BP_GPIO_6_AH},
   {bp_iDectCfg,                .u.ptr = g_iDectStdCfg},
   {bp_daughterCardList,        .u.ptr = g_963138_dCardListFull},
   {bp_last}
};

static bp_elem_t g_voice_bcm963138dvt_p300[] = {
   {bp_cpBoardId,               .u.cp = "963138DVT_P300"},
   {bp_usGpioFxsFxoRst3,        .u.us = BP_GPIO_11_AL},
   {bp_usGpioDectRst,           .u.us = BP_GPIO_12_AL},
   {bp_usGpioVoipRelayCtrl1,    .u.us = BP_GPIO_26_AH},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm963138dvt},
   {bp_last}
};

static bp_elem_t g_voice_bcm963138ref_bmu[] = {
   {bp_cpBoardId,               .u.cp = "963138REF_BMU"},
   {bp_usGpioFxsFxoRst1,        .u.us = BP_GPIO_60_AL},
   {bp_usGpioFxsFxoRst2,        .u.us = BP_GPIO_61_AL},
   {bp_usGpioFxsFxoRst3,        .u.us = BP_NOT_DEFINED},
   {bp_usGpioDectRst,           .u.us = BP_GPIO_62_AL},
   {bp_usGpioVoipRelayCtrl1,    .u.us = BP_NOT_DEFINED},
   {bp_daughterCardList,        .u.ptr = g_963138_dCardListNoFxoRst},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm963138dvt},
   {bp_last}
};

static bp_elem_t g_voice_bcm963138ref_bgw[] = {
   {bp_cpBoardId,               .u.cp = "963138REF_BGW"},
   {bp_usGpioFxsFxoRst1,        .u.us = BP_GPIO_34_AL},
   {bp_usGpioFxsFxoRst2,        .u.us = BP_NOT_DEFINED},
   {bp_usGpioDectRst,           .u.us = BP_NOT_DEFINED},
   {bp_daughterCardList,        .u.ptr = g_963138_dCardListNoFxoRst},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm963138ref_bmu},
   {bp_last}
};

static bp_elem_t g_voice_bcm963138bmu_p202[] = {
   {bp_cpBoardId,               .u.cp = "963138BMU_P202"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm963138ref_bmu},
   {bp_last}
};

static bp_elem_t g_voice_bcm963138ref[] = {
   {bp_cpBoardId,               .u.cp = "963138REF"},
   {bp_usFxsFxo1SpiSSNum,       .u.us = SPI_DEV_1},
   {bp_usFxsFxo2SpiSSNum,       .u.us = SPI_DEV_2},
   {bp_usFxsFxo3SpiSSNum,       .u.us = SPI_DEV_4},
   {bp_usGpioFxsFxoRst1,        .u.us = BP_GPIO_7_AL},
   {bp_usGpioFxsFxoRst2,        .u.us = BP_GPIO_117_AL},
   {bp_usGpioFxsFxoRst3,        .u.us = BP_GPIO_116_AL},
   {bp_usGpioVoipRelayCtrl1,    .u.us = BP_GPIO_6_AH},
   {bp_daughterCardList,        .u.ptr = g_963138_dCardListFull},
   {bp_last}
};


static bp_elem_t g_voice_bcm963138ref_p402[] = {
   {bp_cpBoardId,               .u.cp = "963138REF_P402"},
   {bp_usFxsFxo1SpiSSNum,       .u.us = SPI_DEV_1},
   {bp_usFxsFxo2SpiSSNum,       .u.us = SPI_DEV_4},
   {bp_usFxsFxo3SpiSSNum,       .u.us = SPI_DEV_5},
   {bp_usGpioFxsFxoRst1,        .u.us = BP_GPIO_4_AL},
   {bp_usGpioFxsFxoRst2,        .u.us = BP_GPIO_5_AL},
   {bp_usGpioFxsFxoRst3,        .u.us = BP_GPIO_6_AL},
   {bp_usGpioVoipRelayCtrl1,    .u.us = BP_GPIO_3_AH},
   {bp_daughterCardList,        .u.ptr = g_963138_dCardListFull},
   {bp_last}
};

static bp_elem_t g_voice_bcm963138ref_p502[] = {
   {bp_cpBoardId,               .u.cp = "963138REF_P502"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm963138ref_p402},
   {bp_last}
};

static bp_elem_t g_voice_bcm963138ref_p802[] = {
   {bp_cpBoardId,               .u.cp = "963138REF_P802"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm963138ref_p502},
   {bp_last}
};

static bp_elem_t g_voice_bcm963132ref_138_p502[] = {
   {bp_cpBoardId,               .u.cp = "132_138REF_P502"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm963138ref_p402},
   {bp_last}
};

static bp_elem_t g_voice_bcm963138ref_gfast_p40x[] = {
   {bp_cpBoardId,               .u.cp = "963138GFP40X"},
   {bp_usGpioVoipRelayCtrl1,    .u.us = BP_GPIO_NONE},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm963138ref_p402},
   {bp_last}
};

static bp_elem_t g_voice_bcm963138_gfast2[] = {
   {bp_cpBoardId,               .u.cp = "963138GFAST2"},
   {bp_usGpioVoipRelayCtrl1,    .u.us = BP_GPIO_NONE},
   {bp_usGpioFxsFxoRst3,        .u.us = BP_GPIO_NONE},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm963138ref_p402},
   {bp_last}
};

static bp_elem_t g_voice_bcm963132ref_138gfast2[] = {
   {bp_cpBoardId,               .u.cp = "132_138GFAST2"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm963138_gfast2},
   {bp_last}
};


static bp_elem_t g_voice_bcm963138rref_gfast[] = {
   {bp_cpBoardId,               .u.cp = "63138RREF_GFAST"},
   {bp_usGpioFxsFxoRst1,        .u.us = BP_GPIO_29_AL},
   {bp_usGpioFxsFxoRst2,        .u.us = BP_GPIO_30_AL},
   {bp_usGpioDectRst,           .u.us = BP_GPIO_118_AL},
   {bp_usGpioVoipRelayCtrl1,    .u.us = BP_GPIO_NONE},
   {bp_iDectCfg,                .u.ptr = g_iDectStdCfg},
   {bp_daughterCardList,        .u.ptr = g_963138_dCardListFull},
   {bp_last}
};

static bp_elem_t g_voice_bcm963138rref_rnc[] = {
   {bp_cpBoardId,               .u.cp = "63138RREF_RNC"},
   {bp_usGpioFxsFxoRst1,        .u.us = BP_GPIO_29_AL},
   {bp_usGpioFxsFxoRst2,        .u.us = BP_GPIO_30_AL},
   {bp_usGpioVoipRelayCtrl1,    .u.us = BP_GPIO_NONE},
   {bp_usGpioDectRst,           .u.us = BP_GPIO_118_AL},
   {bp_iDectCfg,                .u.ptr = g_iDectStdCfg},
   {bp_daughterCardList,        .u.ptr = g_963138_dCardListFull},
   {bp_last}
};

static VOICE_DAUGHTER_BOARD_PARMS * g_963138ref_lte_dCardList[] = {
   SLICSLAC_LIST_COMMON_FXO,
   SLICSLAC_LIST_ZSI_FXO,
   0
};

static bp_elem_t g_voice_bcm963138ref_lte[] = {
   {bp_cpBoardId,               .u.cp = "963138REF_LTE"},
   {bp_usFxsFxo1SpiSSNum,       .u.us = SPI_DEV_1},
   {bp_usGpioFxsFxoRst1,        .u.us = BP_GPIO_34_AL},
   {bp_daughterCardList,        .u.ptr = g_963138ref_lte_dCardList},
   {bp_last}
};

static bp_elem_t g_voice_bcm963138lte_p302[] = {
   {bp_cpBoardId,               .u.cp = "963138LTE_P302"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm963138ref_lte},
   {bp_last}
};

static VOICE_DAUGHTER_BOARD_PARMS * g_963138ref_rnc_dCardList[] = {
   SLICSLAC_LIST_COMMON_NOFXO,
   0
};

static bp_elem_t g_voice_bcm963138ref_rnc[] = {
   {bp_cpBoardId,               .u.cp = "963138REF_RNC"},
   {bp_usFxsFxo1SpiSSNum,       .u.us = SPI_DEV_1},
   {bp_usGpioFxsFxoRst1,        .u.us = BP_GPIO_7_AL},
   {bp_daughterCardList,        .u.ptr = g_963138ref_rnc_dCardList},
   {bp_last}
};

static bp_elem_t g_voice_bcm963138ref_rncP400[] = {
   {bp_cpBoardId,               .u.cp = "138REFrncP400"},
   {bp_usGpioDectRst,           .u.us = BP_GPIO_115_AL},
   {bp_iDectCfg,                .u.ptr = g_iDectStdCfg},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm963138ref_rnc},
   {bp_last}
};

static bp_elem_t * g_VoiceBoardParms[]=
{
   g_voice_bcm963138dvt,
   g_voice_bcm963138dvt_p300,
   g_voice_bcm963138ref_bmu,
   g_voice_bcm963138bmu_p202,
   g_voice_bcm963138ref,
   g_voice_bcm963138ref_p402,
   g_voice_bcm963138ref_p502,
   g_voice_bcm963138ref_p802,
   g_voice_bcm963138ref_lte,
   g_voice_bcm963138lte_p302,
   g_voice_bcm963138ref_rnc,
   g_voice_bcm963138ref_rncP400,
   g_voice_bcm963138ref_gfast_p40x,
   g_voice_bcm963138_gfast2,
   g_voice_bcm963138ref_bgw,
   g_voice_bcm963138rref_gfast,
   g_voice_bcm963138rref_rnc,
   g_voice_bcm963132ref_138_p502,
   g_voice_bcm963132ref_138gfast2,
   0
};

#endif


#if defined(_BCM963148_) || defined(CONFIG_BCM963148)

static VOICE_DAUGHTER_BOARD_PARMS * g_963148_dCardListFull[] = {
   SLICSLAC_LIST_COMMON_FXO,
   SLICSLAC_LIST_ZSI_FXO,
   0
};

static VOICE_DAUGHTER_BOARD_PARMS * g_963148_dCardListFullNoFxoRst[] = {
   SLICSLAC_LIST_COMMON_NOFXO,
   0
};

static bp_elem_t g_voice_bcm963148dvt[] = {
   {bp_cpBoardId,               .u.cp = "963148DVT"},
   {bp_usFxsFxo1SpiSSNum,       .u.us = SPI_DEV_1},
   {bp_usFxsFxo2SpiSSNum,       .u.us = SPI_DEV_2},
   {bp_usFxsFxo3SpiSSNum,       .u.us = SPI_DEV_4},
   {bp_usGpioFxsFxoRst1,        .u.us = BP_GPIO_7_AL},
   {bp_usGpioFxsFxoRst2,        .u.us = BP_GPIO_19_AL},
   {bp_usGpioFxsFxoRst3,        .u.us = BP_GPIO_36_AL},
   {bp_usGpioDectRst,           .u.us = BP_GPIO_37_AL},
   {bp_usGpioVoipRelayCtrl1,    .u.us = BP_GPIO_6_AH},
   {bp_iDectCfg,                .u.ptr = g_iDectStdCfg},
   {bp_daughterCardList,        .u.ptr = g_963148_dCardListFull},
   {bp_last}
};

static bp_elem_t g_voice_bcm963148dvt_p300[] = {
   {bp_cpBoardId,               .u.cp = "963148DVT_P300"},
   {bp_usGpioFxsFxoRst3,        .u.us = BP_GPIO_11_AL},
   {bp_usGpioDectRst,           .u.us = BP_GPIO_12_AL},
   {bp_usGpioVoipRelayCtrl1,    .u.us = BP_GPIO_26_AH},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm963148dvt},
   {bp_last}
};

static bp_elem_t g_voice_bcm963148sv[] = {
   {bp_cpBoardId,               .u.cp = "963148SV"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm963148dvt},
   {bp_last}
};

static bp_elem_t g_voice_bcm963148ref[] = {
   {bp_cpBoardId,               .u.cp = "963148REF"},
   {bp_usFxsFxo1SpiSSNum,       .u.us = SPI_DEV_1},
   {bp_usFxsFxo2SpiSSNum,       .u.us = SPI_DEV_4},
   {bp_usFxsFxo3SpiSSNum,       .u.us = SPI_DEV_5},
   {bp_usGpioFxsFxoRst1,        .u.us = BP_GPIO_4_AL},
   {bp_usGpioFxsFxoRst2,        .u.us = BP_GPIO_5_AL},
   {bp_usGpioFxsFxoRst3,        .u.us = BP_GPIO_6_AL},
   {bp_usGpioVoipRelayCtrl1,    .u.us = BP_GPIO_3_AH},
   {bp_daughterCardList,        .u.ptr = g_963148_dCardListFull},
   {bp_last}
};

static bp_elem_t g_voice_bcm963148ref_p502[] = { 
   {bp_cpBoardId,               .u.cp = "963148REF_P502"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm963148ref},
   {bp_last}
};

static bp_elem_t g_voice_bcm963148ref_bmu[] = {
   {bp_cpBoardId,               .u.cp = "963148REF_BMU"},
   {bp_usFxsFxo1SpiSSNum,       .u.us = SPI_DEV_1},
   {bp_usFxsFxo2SpiSSNum,       .u.us = SPI_DEV_2},
   {bp_usFxsFxo3SpiSSNum,       .u.us = SPI_DEV_4},
   {bp_usGpioFxsFxoRst1,        .u.us = BP_GPIO_60_AL},
   {bp_usGpioFxsFxoRst2,        .u.us = BP_GPIO_61_AL},
   {bp_usGpioDectRst,           .u.us = BP_GPIO_62_AL},
   {bp_iDectCfg,                .u.ptr = g_iDectStdCfg},
   {bp_daughterCardList,        .u.ptr = g_963148_dCardListFullNoFxoRst},
   {bp_last}
};


static bp_elem_t * g_VoiceBoardParms[]=
{
   g_voice_bcm963148dvt,
   g_voice_bcm963148dvt_p300,
   g_voice_bcm963148sv,
   g_voice_bcm963148ref,
   g_voice_bcm963148ref_p502,
   g_voice_bcm963148ref_bmu,
   0
};

#endif


#if defined(_BCM963381_) || defined(CONFIG_BCM963381)

static VOICE_DAUGHTER_BOARD_PARMS * g_963381ref1_dCardList[] = {
   &g_voiceBoard_NOSLIC,
   SLICSLAC_LIST_ZSI_FXO,
   0
};

static bp_elem_t g_voice_bcm963381ref1[] = {
   {bp_cpBoardId,               .u.cp = "963381REF1"},
   {bp_usGpioFxsFxoRst1,        .u.us = BP_GPIO_22_AL},
   {bp_daughterCardList,        .u.ptr = g_963381ref1_dCardList},
   {bp_last}
};

static bp_elem_t g_voice_bcm963381ref1_a0[] = {
   {bp_cpBoardId,               .u.cp = "963381REF1_A0"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm963381ref1},
   {bp_last}
};

static VOICE_DAUGHTER_BOARD_PARMS * g_963381dvt_dCardList[] = {
   SLICSLAC_LIST_COMMON_FXO,
   SLICSLAC_LIST_ZSI_FXO,
   0
};

static bp_elem_t g_voice_bcm963381dvt[] = {
   {bp_cpBoardId,               .u.cp = "963381DVT"},
   {bp_usFxsFxo1SpiSSNum,       .u.us = SPI_DEV_1},
   {bp_usFxsFxo2SpiSSNum,       .u.us = SPI_DEV_3},    // This is a workaround for the NC SPI_SS2 in REV03 DVT boards. Must
//    {bp_usFxsFxo3SpiSSNum,       .u.us = SPI_DEV_3}, // short SPI_SS2/3 on motherboard to get any 2 chip cards to work.
   {bp_usGpioFxsFxoRst1,        .u.us = BP_GPIO_22_AL},
   {bp_usGpioFxsFxoRst3,        .u.us = BP_GPIO_14_AL},
   {bp_usGpioVoipRelayCtrl1,    .u.us = BP_GPIO_23_AH},
   {bp_daughterCardList,        .u.ptr = g_963381dvt_dCardList},
   {bp_last}
};

static VOICE_DAUGHTER_BOARD_PARMS * g_963381sv_dCardList[] = {
   SLICSLAC_LIST_COMMON_FXO,
   SLICSLAC_LIST_ZSI_FXO,
   0
};

static bp_elem_t g_voice_bcm963381sv[] = {
   {bp_cpBoardId,               .u.cp = "963381SV"},
   {bp_usFxsFxo1SpiSSNum,       .u.us = SPI_DEV_1},
   {bp_usFxsFxo2SpiSSNum,       .u.us = SPI_DEV_2},
   {bp_usGpioFxsFxoRst1,        .u.us = BP_GPIO_22_AL},
   {bp_usGpioFxsFxoRst2,        .u.us = BP_GPIO_23_AL},
   {bp_daughterCardList,        .u.ptr = g_963381sv_dCardList},
   {bp_last}
};


static VOICE_DAUGHTER_BOARD_PARMS * g_963381a_ref1_dCardList[] = {
   &g_voiceBoard_NOSLIC,
   &g_voiceBoard_SI32260_LCQC,
   0
};

static bp_elem_t g_voice_bcm963381a_ref1[] = {
   {bp_cpBoardId,               .u.cp = "963381A_REF1"},
   {bp_usFxsFxo1SpiSSNum,       .u.us = SPI_DEV_1},
   {bp_usGpioFxsFxoRst1,        .u.us = BP_GPIO_22_AL},
   {bp_daughterCardList,        .u.ptr = g_963381a_ref1_dCardList},
   {bp_last}
};

static bp_elem_t g_voice_bcm963381ref2[] = {
   {bp_cpBoardId,               .u.cp = "963381REF2"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm963381a_ref1},
   {bp_last}
};

static VOICE_DAUGHTER_BOARD_PARMS * g_963381ref3_dCardList[] = {
   SLICSLAC_LIST_COMMON_NOFXO,
   0
};

static bp_elem_t g_voice_bcm963381ref3[] = {
   {bp_cpBoardId,               .u.cp = "963381REF3"},
   {bp_usFxsFxo1SpiSSNum,       .u.us = SPI_DEV_1},
   {bp_usFxsFxo2SpiSSNum,       .u.us = SPI_DEV_3},
   {bp_usGpioFxsFxoRst1,        .u.us = BP_GPIO_22_AL},
   {bp_daughterCardList,        .u.ptr = g_963381ref3_dCardList},
   {bp_last}
};

static bp_elem_t * g_VoiceBoardParms[]=
{
   g_voice_bcm963381ref1,
   g_voice_bcm963381ref1_a0,
   g_voice_bcm963381dvt,
   g_voice_bcm963381sv,
   g_voice_bcm963381a_ref1,
   g_voice_bcm963381ref2,
   g_voice_bcm963381ref3,
   0
};

#endif

#if defined(_BCM96848_) || defined(CONFIG_BCM96848)

static VOICE_DAUGHTER_BOARD_PARMS * g_968480fhgu_dCardList[] = {
   &g_voiceBoard_NOSLIC,
   &g_voiceBoard_LE9641_ZSI_BB,
   0
};

static bp_elem_t g_voice_bcm968480fhgu[] = {
   {bp_cpBoardId,               .u.cp = "968480FHGU"},
   {bp_daughterCardList,        .u.ptr = g_968480fhgu_dCardList},
   {bp_last}
};

static bp_elem_t g_voice_bcm968480fhbb[] = {
   {bp_cpBoardId,               .u.cp = "968480FHBB"},
   {bp_daughterCardList,        .u.ptr = g_968480fhgu_dCardList},
   {bp_last}
};

static VOICE_DAUGHTER_BOARD_PARMS * g_968485sfu_dCardList[] = {
   &g_voiceBoard_NOSLIC,
   &g_voiceBoard_LE9641_ZSI_BB,
   0
};

static bp_elem_t g_voice_bcm968485sfu[] = {
   {bp_cpBoardId,               .u.cp = "968485SFU"},
   {bp_daughterCardList,        .u.ptr = g_968485sfu_dCardList},
   {bp_last}
};

static bp_elem_t g_voice_bcm968486sgu[] = {                    
   {bp_cpBoardId,               .u.cp = "968486SGU"},          
   {bp_daughterCardList,        .u.ptr = g_968480fhgu_dCardList},
   {bp_last}                                                    
};

static bp_elem_t g_voice_bcm968488sgw[] = {                    
   {bp_cpBoardId,               .u.cp = "968488SGW"},          
   {bp_daughterCardList,        .u.ptr = g_968480fhgu_dCardList},
   {bp_last}                                                    
};

static VOICE_DAUGHTER_BOARD_PARMS * g_968480sv_dCardList[] = {
   &g_voiceBoard_NOSLIC,
   &g_voiceBoard_LE9641_ZSI_BB,
   0
};

static bp_elem_t g_voice_bcm968480sv[] = {
   {bp_cpBoardId,               .u.cp = "968480SV"},
   {bp_usFxsFxo1SpiSSNum,       .u.us = SPI_DEV_4},
   {bp_usFxsFxo2SpiSSNum,       .u.us = SPI_DEV_6},
   {bp_usGpioFxsFxoRst1,        .u.us = BP_GPIO_7_AL},
   {bp_daughterCardList,        .u.ptr = g_968480sv_dCardList},
   {bp_last}
};

static VOICE_DAUGHTER_BOARD_PARMS * g_968485sv_dCardList[] = {
   SLICSLAC_LIST_COMMON_NOFXO,
   0
};

static bp_elem_t g_voice_bcm968485sv[] = {
   {bp_cpBoardId,               .u.cp = "968485SV"},
   {bp_usFxsFxo1SpiSSNum,       .u.us = SPI_DEV_4},
   {bp_usFxsFxo2SpiSSNum,       .u.us = SPI_DEV_6},
   {bp_usFxsFxo3SpiSSNum,       .u.us = SPI_DEV_7},
   {bp_usGpioFxsFxoRst1,        .u.us = BP_GPIO_63_AL},
   {bp_usGpioFxsFxoRst2,        .u.us = BP_GPIO_64_AL},
   {bp_usGpioFxsFxoRst3,        .u.us = BP_GPIO_7_AL},
   {bp_daughterCardList,        .u.ptr = g_968485sv_dCardList},
   {bp_last}
};

static bp_elem_t * g_VoiceBoardParms[] = {
   g_voice_bcm968480fhgu,
   g_voice_bcm968480fhbb,
   g_voice_bcm968485sfu,
   g_voice_bcm968480sv,
   g_voice_bcm968485sv,
   g_voice_bcm968486sgu,
   g_voice_bcm968488sgw,
   0
};

#endif

#if defined(_BCM94908_) || defined(CONFIG_BCM94908)

static VOICE_DAUGHTER_BOARD_PARMS * g_962118_dCardList[] = {
   SLICSLAC_LIST_COMMON_FXO,
   SLICSLAC_LIST_ZSI_FXO,
   0
};

static bp_elem_t g_voice_bcm962118ref[] = {
   {bp_cpBoardId,               .u.cp = "962118REF"},
   {bp_usFxsFxo1SpiSSNum,       .u.us = SPI_DEV_1},
   {bp_usFxsFxo2SpiSSNum,       .u.us = SPI_DEV_2},
   {bp_usFxsFxo3SpiSSNum,       .u.us = SPI_DEV_3},
   {bp_usGpioFxsFxoRst1,        .u.us = BP_GPIO_11_AL},
   {bp_usGpioFxsFxoRst2,        .u.us = BP_GPIO_12_AL},
   {bp_usGpioFxsFxoRst3,        .u.us = BP_GPIO_13_AL},
   {bp_daughterCardList,        .u.ptr = g_962118_dCardList},
   {bp_last}
};

static bp_elem_t g_voice_bcm962116ref[] = {
   {bp_cpBoardId,               .u.cp = "962116REF"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm962118ref},
   {bp_last}
};

static bp_elem_t * g_VoiceBoardParms[] = {
   g_voice_bcm962118ref,
   g_voice_bcm962116ref,
   0
};

#endif

#if defined(_BCM96858_) || defined(CONFIG_BCM96858)

static VOICE_DAUGHTER_BOARD_PARMS * g_96858_dCardList[] = {
   SLICSLAC_LIST_COMMON_NOFXO,
   &g_voiceBoard_ZL88601x2,
   SLICSLAC_LIST_4FXS_NOFXO,
   &g_voiceBoard_LE9661_ZSI,
   &g_voiceBoard_LE9642_ZSI_BB,
   &g_voiceBoard_ZL88601x4_8FXS,
   0
};

static VOICE_DAUGHTER_BOARD_PARMS * g_96858SV_dCardList[] = {
   SLICSLAC_LIST_COMMON_NOFXO,
   0
};

static bp_elem_t g_voice_bcm96858ref[] = {
   {bp_cpBoardId,               .u.cp = "968580XREF"},
   {bp_usFxsFxo1SpiSSNum,       .u.us = SPI_DEV_3},
   {bp_usFxsFxo2SpiSSNum,       .u.us = SPI_DEV_4},
   {bp_usFxsFxo3SpiSSNum,       .u.us = SPI_DEV_5},
   {bp_usFxsFxo4SpiSSNum,       .u.us = SPI_DEV_1},
   {bp_usGpioFxsFxoRst1,        .u.us = BP_GPIO_21_AL},
   {bp_usGpioFxsFxoRst2,        .u.us = BP_GPIO_22_AL},
   {bp_usGpioFxsFxoRst3,        .u.us = BP_GPIO_20_AL},
   {bp_usGpioFxsFxoRst4,        .u.us = BP_GPIO_4_AL},
   {bp_daughterCardList,        .u.ptr = g_96858_dCardList},
   {bp_last}
};

static bp_elem_t g_voice_bcm96858xref_xfi[] = {
   {bp_cpBoardId,               .u.cp = "968580XREF_XFI"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm96858ref},
   {bp_last}
};

static bp_elem_t g_voice_bcm96858xsv[] = {
   {bp_cpBoardId,               .u.cp = "968580XSV"},
   {bp_daughterCardList,        .u.ptr = g_96858SV_dCardList},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm96858ref},
   {bp_last}
};

static bp_elem_t g_voice_bcm96858xsv_rgmii_phy[] = {
   {bp_cpBoardId,               .u.cp = "968580XSV_RGPHY"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm96858xsv},
   {bp_last}
};

static bp_elem_t g_voice_bcm96858xsv_sgmii_phy[] = {
   {bp_cpBoardId,               .u.cp = "968580XSV_SGPHY"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm96858xsv},
   {bp_last}
};

static bp_elem_t g_voice_bcm96858xsv_hsgmii_phy[] = {
   {bp_cpBoardId,               .u.cp = "968580XSV_HSG"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm96858xsv},
   {bp_last}
};

static bp_elem_t g_voice_bcm96858xsv_sgmii_opt[] = {
   {bp_cpBoardId,               .u.cp = "968580XSV_SGOPT"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm96858xsv},
   {bp_last}
};

static bp_elem_t g_voice_bcm96858xsv_xfi_opt[] = {
   {bp_cpBoardId,               .u.cp = "968580XSV_XFI"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm96858xsv},
   {bp_last}
};

static bp_elem_t g_voice_bcm96858xref_opt[] = {
   {bp_cpBoardId,               .u.cp = "968580XREF_OPT"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm96858ref},
   {bp_last}
};

static bp_elem_t * g_VoiceBoardParms[] = {
   g_voice_bcm96858ref,
   g_voice_bcm96858xref_xfi,
   g_voice_bcm96858xsv,
   g_voice_bcm96858xsv_rgmii_phy,
   g_voice_bcm96858xsv_sgmii_phy,
   g_voice_bcm96858xsv_hsgmii_phy,
   g_voice_bcm96858xsv_sgmii_opt,
   g_voice_bcm96858xsv_xfi_opt,
   g_voice_bcm96858xref_opt,
   0
};

#endif


#if defined(_BCM96846_) || defined(CONFIG_BCM96846)

static VOICE_DAUGHTER_BOARD_PARMS * g_96846_dCardList_XSI[] = {
   SLICSLAC_LIST_ISI_NOFXO,
   SLICSLAC_LIST_ZSI_NOFXO,
   0
};
static VOICE_DAUGHTER_BOARD_PARMS * g_96846_dCardList[] = {
   SLICSLAC_LIST_COMMON_NOFXO,
   0
};
static VOICE_DAUGHTER_BOARD_PARMS * g_968461PRW_dCardList[] = {
   &g_voiceBoard_NOSLIC,
   &g_voiceBoard_LE9641_ZSI_BB,
   0
};
static bp_elem_t g_voice_bcm968460sv[] = {
   {bp_cpBoardId,               .u.cp = "968460SV"},
   {bp_usFxsFxo1SpiSSNum,       .u.us = SPI_DEV_0},
   {bp_usGpioFxsFxoRst1,        .u.us = BP_GPIO_22_AL},
   {bp_daughterCardList,        .u.ptr = g_96846_dCardList},
   {bp_last}
};
static bp_elem_t g_voice_bcm96846refp[] = {
   {bp_cpBoardId,               .u.cp = "968460REFP"},
   {bp_usFxsFxo1SpiSSNum,       .u.us = SPI_DEV_1},
   {bp_usGpioFxsFxoRst1,        .u.us = BP_GPIO_22_AL},
   {bp_daughterCardList,        .u.ptr = g_96846_dCardList_XSI},
   {bp_last}
};
static bp_elem_t g_voice_bcm96846ref[] = {
   {bp_cpBoardId,               .u.cp = "968460REF"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm96846refp},
   {bp_last}
};
static bp_elem_t g_voice_bcm968461prw[] = {
   {bp_cpBoardId,               .u.cp = "968461PRW"},
   {bp_daughterCardList,        .u.ptr = g_968461PRW_dCardList},
   {bp_last}
};
static bp_elem_t * g_VoiceBoardParms[] = {
   g_voice_bcm968460sv,
   g_voice_bcm96846refp,
   g_voice_bcm96846ref,
   g_voice_bcm968461prw,
   0
};

#endif

#if defined(_BCM963158_) || defined(CONFIG_BCM963158)

static VOICE_DAUGHTER_BOARD_PARMS * g_963158_dCardList[] = {
   SLICSLAC_LIST_COMMON_NOFXO,
   0
};

static bp_elem_t g_voice_bcm963158ref1[] = {
   {bp_cpBoardId,               .u.cp = "963158REF1"},
   {bp_usFxsFxo1SpiSSNum,       .u.us = SPI_DEV_1}, 
   {bp_usGpioFxsFxoRst1,        .u.us = BP_GPIO_4_AL},
   {bp_daughterCardList,        .u.ptr = g_963158_dCardList},
   {bp_last}
};

static bp_elem_t g_voice_bcm963158ref2[] = {
   {bp_cpBoardId,               .u.cp = "963158REF2"},
   {bp_usFxsFxo1SpiSSNum,       .u.us = SPI_DEV_1}, 
   {bp_usFxsFxo2SpiSSNum,       .u.us = SPI_DEV_4},
   {bp_usGpioFxsFxoRst1,        .u.us = BP_GPIO_13_AL},   
   {bp_usGpioFxsFxoRst2,        .u.us = BP_GPIO_81_AL},
   {bp_usGpioVoipRelayCtrl1,    .u.us = BP_GPIO_80_AH},    
   {bp_daughterCardList,        .u.ptr = g_963158_dCardList},
   {bp_last}
};

static bp_elem_t g_voice_bcm963158dvt[] = {
   {bp_cpBoardId,               .u.cp = "963158DVT"},
   {bp_usFxsFxo1SpiSSNum,       .u.us = SPI_DEV_1}, 
   {bp_usFxsFxo2SpiSSNum,       .u.us = SPI_DEV_3}, 
   {bp_usGpioFxsFxoRst1,        .u.us = BP_GPIO_4_AL},
   {bp_usGpioFxsFxoRst2,        .u.us = BP_GPIO_5_AL},
   {bp_daughterCardList,        .u.ptr = g_963158_dCardList},
   {bp_last}
};
static bp_elem_t g_voice_bcm963158ref3d[] = {
   {bp_cpBoardId,               .u.cp = "963158REF3D"},
   {bp_usFxsFxo1SpiSSNum,       .u.us = SPI_DEV_1}, 
   {bp_usGpioFxsFxoRst1,        .u.us = BP_GPIO_14_AL},
   {bp_usGpioVoipRelayCtrl1,    .u.us = BP_GPIO_13_AH},
   {bp_daughterCardList,        .u.ptr = g_963158_dCardList},
   {bp_last}
};

static bp_elem_t g_voice_bcm963158ref3[] = {
   {bp_cpBoardId,               .u.cp = "963158REF3"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm963158ref1},
   {bp_last}
};

static bp_elem_t g_voice_bcm963158ref3_p20x[] = {
   {bp_cpBoardId,               .u.cp = "963158REF3_P20X"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm963158ref3},
   {bp_last}
};

static bp_elem_t * g_VoiceBoardParms[] = {
   g_voice_bcm963158ref1,
   g_voice_bcm963158ref2,
   g_voice_bcm963158dvt,
   g_voice_bcm963158ref3d,
   g_voice_bcm963158ref3,
   g_voice_bcm963158ref3_p20x,
   0
};

#endif

#if defined(_BCM96856_) || defined(CONFIG_BCM96856)

static VOICE_DAUGHTER_BOARD_PARMS * g_96856_dCardList[] = {
   SLICSLAC_LIST_COMMON_NOFXO,
   0
};
static bp_elem_t g_voice_bcm968560sv[] = {
   {bp_cpBoardId,               .u.cp = "968560SV"},
   {bp_usFxsFxo1SpiSSNum,       .u.us = SPI_DEV_1},
   {bp_usGpioFxsFxoRst1,        .u.us = BP_GPIO_11_AL},
   {bp_daughterCardList,        .u.ptr = g_96856_dCardList},
   {bp_last}
};
static bp_elem_t g_voice_bcm968360bg[] = {
   {bp_cpBoardId,               .u.cp = "968360BG"},
   {bp_usFxsFxo1SpiSSNum,       .u.us = SPI_DEV_1}, 
   {bp_usGpioFxsFxoRst1,        .u.us = BP_GPIO_11_AL}, 
   {bp_daughterCardList,        .u.ptr = g_96856_dCardList},
   {bp_last}
};
static bp_elem_t g_voice_bcm968560ref[] = {
   {bp_cpBoardId,               .u.cp = "968560REF"},
   {bp_usFxsFxo1SpiSSNum,       .u.us = SPI_DEV_1}, 
   {bp_usGpioFxsFxoRst1,        .u.us = BP_GPIO_11_AL}, 
   {bp_daughterCardList,        .u.ptr = g_96856_dCardList},
   {bp_last}
};
static bp_elem_t g_voice_bcm968360bsff[] = {
   {bp_cpBoardId,               .u.cp = "968360BSFF"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm968360bg},
   {bp_last}
};
static bp_elem_t g_voice_bcm968360bsfp[] = {
   {bp_cpBoardId,               .u.cp = "968360BSFP"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm968360bg},
   {bp_last}
};
static bp_elem_t * g_VoiceBoardParms[] = {
   g_voice_bcm968560sv,
   g_voice_bcm968360bg,
   g_voice_bcm968560ref,
   g_voice_bcm968360bsff,
   g_voice_bcm968360bsfp,
   0
};

#endif

#if !defined(_BCM963268_) && !defined(CONFIG_BCM963268) && !defined(_BCM96838_) && !defined(CONFIG_BCM96838) && !defined(_BCM963138_) && !defined(CONFIG_BCM963138) && !defined(_BCM963381_) && !defined(CONFIG_BCM963381) && !defined(_BCM963148_) && !defined(CONFIG_BCM963148) && !defined(CONFIG_BCM96848) && !defined(_BCM94908_) && !defined(CONFIG_BCM94908) && !defined(_BCM96858_) && !defined(CONFIG_BCM96858) && !defined(_BCM96836_) && !defined(CONFIG_BCM96836) && !defined(_BCM963158_) && !defined(CONFIG_BCM963158) && !defined(CONFIG_BCM96846) && !defined(CONFIG_BCM96856)

static bp_elem_t * g_VoiceBoardParms[]=
{
   0
};

#endif
/* Voice Boardparams End */


/*****************************************************************************
*
*  String and memory manipulation private functions
*
******************************************************************************/
static void bpmemcpy( void* dstptr, const void* srcptr, int size )
{
   char* dstp = dstptr;
   const char* srcp = srcptr;
   int i;
   for( i=0; i < size; i++ )
   {
      *dstp++ = *srcp++;
   }
}

static char * bpstrcpy( char* dest, const char* src)
{
   while(*src)
   {
      *dest++ = *src++;
   }

   *dest = '\0';

   return dest;
}

#if !defined(_CFE_)
static int bpstrlen( char * src )
{
   char *s;

   for(s = src; (s != 0) && (*s != 0); ++s);

   return(s - src);
}
#endif /* !defined(_CFE_) */

#if !defined(_CFE_)
/*****************************************************************************
*
*  voice Daughtercard type to BoardParam Type mapping functions
*
******************************************************************************/
static enum bp_id mapDcRstPinToBpType( BP_RESET_PIN rstPin )
{
   return( bp_usGpioFxsFxoRst1 + (enum bp_id)( rstPin - BP_RESET_FXS1 ) );
}

static enum bp_id mapDcSpiDevIdToBpType( BP_SPI_SIGNAL spiId )
{
   return( bp_usFxsFxo1SpiSSNum + (enum bp_id)( spiId - BP_SPI_SS_B1 ) );
}
#endif /* !defined(_CFE_) */

#if !defined(_CFE_)
/*****************************************************************************
 * Name:          BpGetZSISpiDevID()
 *
 * Description:     This function returns the SPI Device ID for the ZSI daughter
*                   boards based on the current chip.
 *
 * Parameters:    Nothing
 *
 * Returns:       SPI Dev ID for ZSI Daughter Boards
 *
 *****************************************************************************/
static unsigned int BpGetZSISpiDevID( void )
{
#ifdef ZSI_SPI_DEV_ID
   return ZSI_SPI_DEV_ID;
#else
   return BP_NOT_DEFINED;
#endif
}
#endif /* !defined(_CFE_) */


/*****************************************************************************
 * Name:          BpSetDectPopulatedData()
 *
 * Description:     This function sets the g_BpDectPopulated variable. It is
 *                used for the user to specify in the board parameters if the
 *                board DECT is populated or not (1 for populated, 0 for not).
 *
 * Parameters:    int BpData - The data that g_BpDectPopulated will be set to.
 *
 * Returns:       Nothing
 *
 *****************************************************************************/
void BpSetDectPopulatedData( int BpData )
{
   g_BpDectPopulated = BpData;
}

/*****************************************************************************
 * Name:           BpDectPopulated()
 *
 * Description:    This function is used to determine if DECT is populated on
 *                 the board.
 *
 * Parameters:    None
 *
 * Returns:       BP_DECT_POPULATED if DECT is populated, otherwise it will
 *                return BP_DECT_NOT_POPULATED.
 *
 *****************************************************************************/
int BpDectPopulated( void )
{
   return (g_BpDectPopulated ? BP_DECT_POPULATED : BP_DECT_NOT_POPULATED);
}

#if !defined(_CFE_)
/*****************************************************************************
 * Name:          BpGetVoiceParms()
 *
 * Description:     Finds the voice parameters based on the daughter board and
 *                base board IDs and fills the old parameters structure with
 *                information.
 *
 * Parameters:    pszVoiceDaughterCardId - The daughter board ID that is being used.
 *                voiceParms - The old voice parameters structure that must be
 *                             filled with data from the new structure.
 *                pszBaseBoardId - The base board ID that is being used.
 *
 * Returns:       If the board is not found, returns BP_BOARD_ID_NOT_FOUND.
 *                If everything goes properly, returns BP_SUCCESS.
 *
 *****************************************************************************/
int BpGetVoiceParms( char* pszVoiceDaughterCardId, VOICE_BOARD_PARMS* voiceParmsOut, char* pszBaseBoardId )
{
   PVOICE_DAUGHTER_BOARD_PARMS *ppDc;
   VOICE_DAUGHTER_BOARD_PARMS *pDc;
   bp_elem_t *pBpStartElem;

   BP_VOICE_CHANNEL_INTERNAL *dectChanCfg = NULL;
   BP_VOICE_DEVICE *vd = NULL;
   BP_VOICE_DEVICE_INTERNAL *vdi = NULL;

   int gId = 0;
   int deviceCount = 0;
   int i, j, pass;

   unsigned int pcmRxChanMask = 0;
   unsigned int pcmTxChanMask = 0;
   unsigned int apmChanMask = 0;
   unsigned int dectChanId = 0;
   unsigned int pcmChanId = 0;

   /* If we have already initialized the boardparms, return a copy */
   if(vpInitialized)
   {
      *voiceParmsOut = vp;
      return BP_SUCCESS;
   }

   /* Wipe the returned voice devices */
   for(deviceCount = 0; deviceCount < BP_MAX_VOICE_DEVICES; deviceCount++)
   {
      vp.voiceDevice[deviceCount].voiceDeviceType = BP_VD_NONE;
   }

   /* Get start element of voice board params structure */
   if( !(pBpStartElem = BpGetVoiceBoardStartElemPtr(pszBaseBoardId)) )
   {
      /* No matching base board found */
      printk("%s: Configured base board '%s' not found\n", __FUNCTION__, pszBaseBoardId);
      return BP_BOARD_ID_NOT_FOUND;
   }

   /* Get dectcfg pointer */
   if( BpDectPopulated() == BP_DECT_POPULATED )
   {
      dectChanCfg = (BP_VOICE_CHANNEL_INTERNAL *)BpGetSubPtr(bp_iDectCfg, pBpStartElem, bp_last);
   }

   /* Get daughtercard list pointer */
   ppDc = (PVOICE_DAUGHTER_BOARD_PARMS *)BpGetSubPtr(bp_daughterCardList, pBpStartElem, bp_last);
   if( !ppDc )
   {
      /* No matching daughtercard list was found */
      printk("%s: No daughter board configured\n", __FUNCTION__);
      return BP_BOARD_ID_NOT_FOUND;
   }

   /* Iterate through daughter card list */
   for(; *ppDc; ppDc++)
   {
      pDc = *ppDc;

      if( bpstrcmp(pDc->szBoardId, pszVoiceDaughterCardId) == 0 )
      {
         break;
      }
   }
   if(!*ppDc)
   {
      printk("%s: Configured daughter board '%s' not found\n", __FUNCTION__, pszVoiceDaughterCardId);
      return BP_BOARD_ID_NOT_FOUND;
   }

   /* Succesfully found base board + daughter card combination
    * Must now fill the vp structure with data and copy to voiceParms
    * First set base board and daughter board strings */
   bpmemcpy(vp.szBoardId, pszVoiceDaughterCardId, bpstrlen(pszVoiceDaughterCardId));
   bpmemcpy(vp.szBaseBoardId, pszBaseBoardId, bpstrlen(pszBaseBoardId));

   /* Count the configured devices */
   for(i = 0; i < BP_MAX_VOICE_DEVICES; i++)
   {
      vdi = &pDc->voiceDevice[i];
      if( vdi->deviceType == BP_VD_NONE || vdi->deviceType >= BP_VD_MAX )
      {
         break;
      }

      for(j = 0; j < BP_MAX_CHANNELS_PER_DEVICE; j++)
      {
         switch(vdi->channel[j].type)
         {
            case BP_VCTYPE_SLIC:
               vp.numFxsLines++;
               break;
            case BP_VCTYPE_DAA:
               vp.numFxoLines++;
               break;
            default:
               break;
         }
      }
   }
#ifdef CONFIG_BCM_DECT_SUPPORT
   for(i = 0; i < BP_MAX_DECT_DEVICE * BP_MAX_CHANNELS_PER_DEVICE; i++)
   {
      if(dectChanCfg && dectChanCfg[i].type == BP_VCTYPE_DECT)
         vp.numDectLines++;
   }
#endif /* CONFIG_BCM_DECT_SUPPORT */

   /* If DECT is not populated enabled, explicitly set the number of DECT lines
    * to 0 */
   if( BpDectPopulated() != BP_DECT_POPULATED )
   {
      vp.numDectLines = 0;
   }

   /* This prevents the total number of channels from being greater than 8 */
   if(vp.numFxsLines + vp.numFxoLines + vp.numDectLines > 8)
   {
      if(vp.numDectLines == 4)
      {
         /* If there are four DECT lines and it is exceeding limit, can
          * cut two of the DECT lines for board/daughter card combinations
          * with 4 FXS lines such as 963268V30A with Si32260x2.*/
         vp.numDectLines = 2;
      }
      else
      {
         return BP_MAX_CHANNELS_EXCEEDED;
      }
   }

   /* Set the relay GPIO pins */
   vp.pstnRelayCtrl.relayGpio[0] = BpGetSubUs(bp_usGpioVoipRelayCtrl1, pBpStartElem, bp_last);
   vp.pstnRelayCtrl.relayGpio[1] = BpGetSubUs(bp_usGpioVoipRelayCtrl2, pBpStartElem, bp_last);
   for( i = 0, vp.numFailoverRelayPins = 0; i < BP_MAX_RELAY_PINS; i++ )
   {
      if(vp.pstnRelayCtrl.relayGpio[i] != BP_NOT_DEFINED)
      {
         vp.numFailoverRelayPins++;
      }
   }

   /*Set DECT UART to Not Defined always for now. */
   vp.dectUartControl.dectUartGpioTx = BP_NOT_DEFINED;
   vp.dectUartControl.dectUartGpioRx = BP_NOT_DEFINED;

   /* Set the device profile and flags */
   vp.deviceProfile = pDc->deviceProfile;
   vp.flags = pDc->flags;

   /* Iterate through the DECT devices and copy the channel configurations */
   for( deviceCount = 0; BP_MAX_CHANNELS_PER_DEVICE * deviceCount < vp.numDectLines; deviceCount++)
   {
      vd = &vp.voiceDevice[deviceCount];
      vd->voiceDeviceType    = BP_VD_IDECT1;
      vd->audioInterfaceType = BP_VDTYPE_DECT;
      vd->spiCtrl.spiDevId   = 0;
      vd->spiCtrl.spiGpio    = BP_NOT_DEFINED;
      vd->requiresReset      = 1;
      vd->resetGpio          = BpGetSubUs(bp_usGpioDectRst, pBpStartElem, bp_last);
      for( i = 0; i < BP_MAX_CHANNELS_PER_DEVICE; i++ )
      {
         BP_VOICE_CHANNEL          *pCh    = &vd->channel[i];
         BP_VOICE_CHANNEL_INTERNAL *pIntCh = &dectChanCfg[i + deviceCount * BP_MAX_CHANNELS_PER_DEVICE];

         /* Set the relevant parts for each channel */
         pCh->id         = gId++;
         pCh->status     = pIntCh->status;
         pCh->type       = pIntCh->type;
         pCh->sampleComp = pIntCh->sampleComp;
         pCh->sampleRate = pIntCh->sampleRate;
         pCh->cfg.dect.dectChanId = dectChanId++;
      }
   }

   /* Loop through the voice devices twice. On the first pass, setup any
    * channel overrides that are required. On the second pass, finish all the
    * automatic configuration. */
   for(pass = 0; pass < 2; pass++)
   {
      int devIdx = deviceCount;

      /* Device configuration */
      for( i = 0; i < BP_MAX_VOICE_DEVICES; i++ )
      {
         vd  = &vp.voiceDevice[devIdx];
         vdi = &pDc->voiceDevice[i];
         if( vdi->deviceType == BP_VD_NONE )
         {
            break;
         }

         if(pass == 0)
         {
            vd->voiceDeviceType    = vdi->deviceType;
            vd->audioInterfaceType = vdi->audioType;

            /* Retrieve the Reset GPIO */
            vd->requiresReset = (vdi->rstPin == BP_RESET_NOT_REQUIRED) ? 0 : 1;
            vd->resetGpio     = BpGetSubUs( mapDcRstPinToBpType(vdi->rstPin), pBpStartElem, bp_last );

            /* Handle the ZSI/ISI devices */
            if(vdi->SPI_SS_Bx == BP_SPI_SS_NOT_REQUIRED)
            {
               /* Current device _IS_ a ZSI/ISI device. */
               vd->spiCtrl.spiDevId = BpGetZSISpiDevID();

               if(vd->spiCtrl.spiDevId == BP_NOT_DEFINED)
               {
                  /* Failure - Tried to use a ZSI/ISI chip on a board which does not support it*/
                  return BP_NO_ZSI_ON_BOARD_ERR;
               }

               vd->spiCtrl.spiGpio = BP_NOT_DEFINED;
            }
            else
            {
               /* Assign system SPI device ID */
               vd->spiCtrl.spiDevId = BpGetSubUs(mapDcSpiDevIdToBpType(vdi->SPI_SS_Bx), pBpStartElem, bp_last);
               /* Assign SPI associated GPIO pin */
               vd->spiCtrl.spiGpio  = BpGetSlaveSelectGpioNum(vd->spiCtrl.spiDevId);
            }

            /* Handle speical case for Si3239 */
            switch(vdi->deviceType)
            {
               case BP_VD_SILABS_3239:
               case BP_VD_SILABS_32392:
                  /* FIXME - Add SPI retrieval function for SI3239X in boardhal code */
                  vd->spiCtrl.spiDevId = BpGetSubUs(bp_usSi32392SpiSSNum, pBpStartElem, bp_last);
                  vd->spiCtrl.spiGpio  = BpGetSlaveSelectGpioNum( vd->spiCtrl.spiDevId );
                  break;

               default:
                  break;
            }
         }

         /* Channel configuration */
         for( j = 0; j < BP_MAX_CHANNELS_PER_DEVICE; j++ )
         {
            BP_VOICE_CHANNEL          *pCh    = &vd->channel[j];
            BP_VOICE_CHANNEL_INTERNAL *pIntCh = &vdi->channel[j];

            /* Set data on the first pass */
            if(pass == 0)
            {
               /* Copy the relevant parts for each channel */
               pCh->id         = (pIntCh->status == BP_VC_ACTIVE) ? gId++ : -1;
               pCh->status     = pIntCh->status;
               pCh->type       = pIntCh->type;
               pCh->sampleComp = pIntCh->sampleComp;
               pCh->sampleRate = pIntCh->sampleRate;
            }

            if(pIntCh->status != BP_VC_ACTIVE)
               continue;

            /* Set the channel configuration, as required */
            switch(vdi->audioType)
            {
               case BP_VDTYPE_PCM:
                  /* If this is the first pass, ignore channels with no
                   * overrides. If this is the second pass, ignore channels
                   * that have already been setup. */
                  if( (!pIntCh->override.pcm && pass == 0) || (pIntCh->override.pcm && pass == 1) )
                     break;
                  /* Configure PCM timeslots */
                  if( BpPcmTSCfg( pCh, pIntCh, &pcmRxChanMask, &pcmTxChanMask, &pcmChanId ) )
                     return BP_MAX_CHANNELS_EXCEEDED;
                  break;

               case BP_VDTYPE_APM:
                  /* If this is the first pass, ignore channels with no
                   * overrides. If this is the second pass, ignore channels
                   * that have already been setup. */
                  if( (!pIntCh->override.apm && pass == 0) || (pIntCh->override.apm && pass == 1) )
                     break;
                  /* Configure APM */
                  if( BpApmCfg( pCh, pIntCh, &apmChanMask ) )
                     return BP_MAX_CHANNELS_EXCEEDED;
                  break;

               default:
                  break;
            }
         } /* for each channel */

         devIdx++;
      } /* for each device */
   } /* pass */

   *voiceParmsOut = vp;
   vpInitialized = 1;
   return BP_SUCCESS;
}


/*****************************************************************************
 * Name:          BpPcmTsCfg()
 *
 * Description:   Configure the PCM timeslots
 *
 * Parameters:    pCh           - channel to be configured
 *                pIntCh        - internally defined channel parameters
 *                pcmRxChanMask - current receive PCM channel mask
 *                pcmTxChanMask - current transmit PCM channel mask
 *
 * Returns:       0 on success, 1 otherwise
 *
 *****************************************************************************/
static int BpPcmTSCfg( BP_VOICE_CHANNEL *pCh, BP_VOICE_CHANNEL_INTERNAL *pIntCh,
                       unsigned int *pcmRxChanMask, unsigned int *pcmTxChanMask,
                       unsigned int *pcmChanId)
{
   int i, numBytes;

   numBytes = BP_PCM_TS_COUNT(pIntCh->sampleComp, pIntCh->sampleRate);

   /* Make sure all the timeslots start uninitialized */
   for(i = 0; i < 4; i++)
   {
      pCh->cfg.pcm.ts.rxTimeslot[i] = -1;
      pCh->cfg.pcm.ts.txTimeslot[i] = -1;
   }

   if( pIntCh->override.pcm )
   {
      for(i = 0; i < numBytes; i++)
      {
         if( pIntCh->override.pcm->rxTimeslot[i] > 8 * sizeof(*pcmRxChanMask) )
         {
            printk("%s: RX timeslot %d (%d) is larger than timeslot limit\n", __FUNCTION__, i, pIntCh->override.pcm->rxTimeslot[i] );
            return 1;
         }

         if( pIntCh->override.pcm->txTimeslot[i] > 8 * sizeof(*pcmTxChanMask) )
         {
            printk("%s: TX timeslot %d (%d) is larger than timeslot limit\n", __FUNCTION__, i, pIntCh->override.pcm->txTimeslot[i] );
            return 1;
         }

         if( *pcmRxChanMask & (1 << pIntCh->override.pcm->rxTimeslot[i]) )
         {
            printk("%s: RX timeslot %d (%d) is already used\n", __FUNCTION__, i, pIntCh->override.pcm->rxTimeslot[i] );
            return 1;
         }

         if( *pcmTxChanMask & (1 << pIntCh->override.pcm->txTimeslot[i]) )
         {
            printk("%s: TX timeslot %d (%d) is already used\n", __FUNCTION__, i, pIntCh->override.pcm->txTimeslot[i] );
            return 1;
         }

         /* Assign the timeslots and mask */
         pCh->cfg.pcm.ts.rxTimeslot[i] = pIntCh->override.pcm->rxTimeslot[i];
         pCh->cfg.pcm.ts.txTimeslot[i] = pIntCh->override.pcm->txTimeslot[i];

         *pcmRxChanMask |= 1 << pIntCh->override.pcm->rxTimeslot[i];
         *pcmTxChanMask |= 1 << pIntCh->override.pcm->txTimeslot[i];
      }
   }
   else
   {
      /* No support currently for auto channel setup */
      printk("%s: No timeslot specified for PCM device %d\n", __FUNCTION__, pCh->id);
      return 1;
   }

   /* Set the PCM channel and update globally */
   pCh->cfg.pcm.pcmChanId = *pcmChanId;
   *pcmChanId += 1;

   return 0;
}

/*****************************************************************************
 * Name:          BpApmCfg
 *
 * Description:   Configure the APM DMA channels
 *
 * Parameters:    pCh         - channel to be configured
 *                pIntCh      - internally defined channel parameters
 *                apmChanMask - current APM channel mask
 *
 * Returns:       0 on success, nonzero otherwise
 *
 *****************************************************************************/
static int BpApmCfg( BP_VOICE_CHANNEL *pCh, BP_VOICE_CHANNEL_INTERNAL *pIntCh,
                     unsigned int *apmChanMask)
{
   BP_APM_CHAN chan;
   int i;

   if(pIntCh->override.apm)
   {
      chan = pIntCh->override.apm->chan;
      if(*apmChanMask & (1 << chan))
      {
         printk("%s: unable to re-use APM channel %d\n", __FUNCTION__, chan);
         return 1;
      }
   }
   else
   {
      /* Find next free channel */
      for(i = 0; i < BP_APM_CHAN_LAST; i++)
      {
         if((*apmChanMask & (1 << i)) == 0)
         {
            break;
         }
      }
      if(i >= BP_APM_CHAN_LAST)
      {
         printk("%s: no APM channel available\n", __FUNCTION__);
         return 1;
      }
      chan = i;
   }

   pCh->cfg.apm.chan = chan;
   *apmChanMask |= (1 << chan);

   return 0;
}
#endif /* !defined(_CFE_) */

/**************************************************************************
* Name       : BpSetVoiceBoardId
*
* Description: This function find the BOARD_PARAMETERS structure for the
*              specified board id string and assigns it to a global, static
*              variable.
*
* Parameters : [IN] pszVoiceDaughterCardId - Board id string that is saved into NVRAM.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_FOUND - Error, board id input string does not
*                  have a board parameters configuration record.
***************************************************************************/
int BpSetVoiceBoardId( const char *pszVoiceDaughterCardId )
{
   bp_elem_t * pBpStartElem;
   PVOICE_DAUGHTER_BOARD_PARMS *ppDc;

   /* Get start element of voice board params structure - Pass 0 to retrieve base board id in utility function */
   if( !(pBpStartElem = BpGetVoiceBoardStartElemPtr(0)) )
   {
      /* No matching base board found */
      return BP_BOARD_ID_NOT_FOUND;
   }

   /* Get daughtercard list pointer */
   ppDc = (PVOICE_DAUGHTER_BOARD_PARMS *)BpGetSubPtr(bp_daughterCardList, pBpStartElem, bp_last);
   if( !ppDc )
   {
      /* No matching daughtercard list was found */
      return BP_BOARD_ID_NOT_FOUND;
   }

   /* Iterate through daughter card list */
   for(; *ppDc; ppDc++)
   {
      if( (0 == bpstrcmp((*ppDc)->szBoardId, pszVoiceDaughterCardId)))
      {
         bpmemcpy(voiceCurrentDgtrCardCfgId, pszVoiceDaughterCardId, BP_BOARD_ID_LEN);
         return BP_SUCCESS;
      }
   }

   return BP_BOARD_ID_NOT_FOUND;
} /* BpSetVoiceBoardId */


/**************************************************************************
* Name       : BpGetVoiceBoardId
*
* Description: This function returns the current board id strings.
*
* Parameters : [OUT] pszVoiceDaughterCardId - Address of a buffer that the board id
*                  string is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
***************************************************************************/
int BpGetVoiceBoardId( char *pszVoiceDaughterCardId )
{
   int i;

   if (0 == bpstrcmp(voiceCurrentDgtrCardCfgId, VOICE_BOARD_ID_DEFAULT))
   {
      return -1;
   }

   for (i = 0; i < BP_BOARD_ID_LEN; i++)
   {
      pszVoiceDaughterCardId[i] = voiceCurrentDgtrCardCfgId[i];
   }

   return 0;
}

int BpGetNumVoiceBoardIds(const char *pszBaseBoardId)
{
   PVOICE_DAUGHTER_BOARD_PARMS *ppDc;
   bp_elem_t * pBpStartElem;
   int i = 0;

   /* Get start element of voice board params structure */
   if( !(pBpStartElem = BpGetVoiceBoardStartElemPtr(pszBaseBoardId)) )
      return 0;

   /* Get daughtercard list pointer */
   ppDc = (PVOICE_DAUGHTER_BOARD_PARMS *)BpGetSubPtr(bp_daughterCardList, pBpStartElem, bp_last);

   for (; ppDc && *ppDc; ppDc++)
      i++;
   return i;
}

char *BpGetVoiceBoardIdNameByIndex(int i, const char *pszBaseBoardId)
{
   PVOICE_DAUGHTER_BOARD_PARMS *ppDc;
   bp_elem_t * pBpStartElem;

   /* Get start element of voice board params structure */
   if( !(pBpStartElem = BpGetVoiceBoardStartElemPtr(pszBaseBoardId)) )
      return "(unsupported)";

   /* Get daughtercard list pointer */
   ppDc = (PVOICE_DAUGHTER_BOARD_PARMS *)BpGetSubPtr(bp_daughterCardList, pBpStartElem, bp_last);
   for(; ppDc && *ppDc && i > 0; i--, ppDc++);

   if(ppDc && *ppDc)
      return (*ppDc)->szBoardId;
   return "(null)";
}

/**************************************************************************
* Name       : BpGetVoiceBoardIds
*
* Description: This function returns all of the supported voice board id strings.
*
* Parameters : [OUT] pszVoiceDaughterCardIds - Address of a buffer that the board id
*                  strings are returned in.  Each id starts at BP_BOARD_ID_LEN
*                  boundary.
*              [IN] nBoardIdsSize - Number of BP_BOARD_ID_LEN elements that
*                  were allocated in pszBoardIds.
*              [IN] pszBaseBoardId - Name of base Board ID to associate Voice
*                  Board ID with.
*
* Returns    : Number of board id strings returned.
***************************************************************************/
int BpGetVoiceBoardIds( char *pszVoiceDaughterCardIds, int nBoardIdsSize, char *pszBaseBoardId )
{
   int count = 0;
   bp_elem_t * pBpStartElem;
   PVOICE_DAUGHTER_BOARD_PARMS *ppDc;

   /* Get start element of voice board params structure */
   if( !(pBpStartElem = BpGetVoiceBoardStartElemPtr(pszBaseBoardId)) )
   {
      /* No matching base board found */
      return 0;
   }

   /* Get daughtercard list pointer */
   ppDc = (PVOICE_DAUGHTER_BOARD_PARMS *)BpGetSubPtr(bp_daughterCardList, pBpStartElem, bp_last);
   if( !ppDc )
   {
      /* No matching daughtercard list was found */
      return count;
   }

   /* Iterate through daughter card list */
   for(; (*ppDc != 0) && (*ppDc != 0) && (nBoardIdsSize != 0); ppDc++)
   {
      /* Copy over daughtercard Ids */
      bpstrcpy(pszVoiceDaughterCardIds, (*ppDc)->szBoardId);
      pszVoiceDaughterCardIds += BP_BOARD_ID_LEN;
      nBoardIdsSize--;
      count++;
   }

   return( count );
} /* BpGetVoiceBoardIds */


/**************************************************************************
* Name       : BpGetVoiceDectType
*
* Description: This function returns whether or not Dect is supported on a given board.
*
* Parameters : [IN] pszBaseBoardId - Name of the base Board ID
*
* Returns    : Status indicating if the base board supports dect.
***************************************************************************/
int BpGetVoiceDectType( const char *pszBaseBoardId )
{
   bp_elem_t * pBpStartElem;
   BP_VOICE_CHANNEL_INTERNAL * dectChanCfg = 0;

   /* Get start element of voice board params structure */
   if( !(pBpStartElem = BpGetVoiceBoardStartElemPtr(pszBaseBoardId)) )
   {
      /* No matching base board found */
      return BP_VOICE_NO_DECT;
   }

   /* Get IDECT Cfg */
   dectChanCfg = (BP_VOICE_CHANNEL_INTERNAL *)BpGetSubPtr(bp_iDectCfg, pBpStartElem, bp_last);
   if( dectChanCfg )
   {
      return BP_VOICE_INT_DECT;
   }

   return BP_VOICE_NO_DECT;
}

/**************************************************************************
* Name       : BpGetVoiceBoardStartElemPtr
*
* Description: This function returns the start element of a voice board params stucture
*              when given a baseboard id. If base boardid is not specified then the
*              currently active base board id is retreived and used
*
* Parameters : [IN] pszBaseBoardId - Name of the base Board ID
*
* Returns    : Start element of matching voice boardparams structure.
***************************************************************************/
static bp_elem_t * BpGetVoiceBoardStartElemPtr( const char * pszBaseBoardId )
{
   int bpPtrIndex;
   char * baseBoardId;
   char boardIdStr[BP_BOARD_ID_LEN];

   /* Get Base board Id if not specified */
   if( !pszBaseBoardId )
   {
      if ( BpGetBoardId(boardIdStr) != BP_SUCCESS )
      {
         /* No matching base board found */
         return 0;
      }
   }
   else
   {
      /* Copy over specified base board id */
      bpstrcpy(boardIdStr, pszBaseBoardId);
   }

   /* Iterate through list of voice board params to find matching structure to base board */
   for( bpPtrIndex=0; g_VoiceBoardParms[bpPtrIndex]; bpPtrIndex++ )
   {
      baseBoardId = BpGetSubCp(bp_cpBoardId, g_VoiceBoardParms[bpPtrIndex], bp_last);
      if( baseBoardId && (0 == bpstrcmp(baseBoardId, boardIdStr)) )
      {
         /* Found the matching board */
         break;
      }
   }

   return g_VoiceBoardParms[bpPtrIndex];
}

#if !defined(_CFE_)
/**************************************************************************
* Name       : BpGetSlaveSelectGpioNum
*
* Description: This function returns the gpio number associated with a particular
*              SPI slave select
*
* Parameters : ssNum - Slave select ID
*
* Returns    : Start element of matching voice boardparams structure.
***************************************************************************/
static unsigned short BpGetSlaveSelectGpioNum( BP_SPI_PORT ssNum)
{
   bp_elem_t * pElem;

   for( pElem = g_pCurrentBp; pElem && (pElem->id != bp_last); pElem++ )
   {
      /* check for spi slave select definition.bp_usSpiSlaveSelectNum must be follwed by bp_usSpiSlaveSelectGpioNum */
      if( pElem->id == bp_usSpiSlaveSelectNum && pElem->u.us == (unsigned short)ssNum )
      {
         pElem++;
         if( (pElem->id != bp_last) && (pElem->id == bp_usSpiSlaveSelectGpioNum) )
         {
            /* Return active low for compatibility with legacy code */
            return (pElem->u.us | BP_ACTIVE_LOW);
         }
      }

      /* Assign parent bp if pointer present */
      if( pElem->id == bp_elemTemplate )
      {
         pElem = pElem->u.bp_elemp;
      }
   }
   return BP_NOT_DEFINED;
}
#endif /* !defined(_CFE_) */

/**************************************************************************
* Name       : BpGetVoicePmuxBp
*
* Description: This function returns a filtered version of the voice board params
*              based on the daughter card that is configured. The last element
*              in the filtered boardparams struct is a pointer to the passed-in
*              data side board params struct
*
* Parameters : pCurrentDataBp - pointer to current data boardparams
*
* Returns    : filtered voice boardparams.
***************************************************************************/
bp_elem_t * BpGetVoicePmuxBp( bp_elem_t * pCurrentDataBp )
{
   int i = 0;
   int bSi3239x = 0;
   int bLe954x = 0;
   int bHspi = 0;
   bp_elem_t * pBpStartElem = 0;
   bp_elem_t * pElem;
   PVOICE_DAUGHTER_BOARD_PARMS *ppDc;
   bp_elem_t * pNewElem;
   BP_VOICE_CHANNEL_INTERNAL * dectChanCfg = 0;

   if( !pCurrentDataBp || pCurrentDataBp[0].id != bp_cpBoardId )
   {
      return 0;
   }

   /* Get start element of voice board params structure */
   if( pCurrentDataBp[0].u.cp && !(pBpStartElem = BpGetVoiceBoardStartElemPtr(pCurrentDataBp[0].u.cp)) )
   {
      /* No matching base board found */
      return 0;
   }

   /* Get dectcfg pointer */
   if( BpDectPopulated() == BP_DECT_POPULATED )
   {
      dectChanCfg = (BP_VOICE_CHANNEL_INTERNAL *)BpGetSubPtr(bp_iDectCfg, pBpStartElem, bp_last);
   }

   /* Get daughtercard list pointer */
   ppDc = (PVOICE_DAUGHTER_BOARD_PARMS *)BpGetSubPtr(bp_daughterCardList, pBpStartElem, bp_last);
   if( !ppDc )
   {
      /* No matching daughtercard list was found */
      return 0;
   }

   /* Iterate through daughter card list */
   for(; *ppDc; ppDc++)
   {
      /* If matching voice board is found, break out */
      if( (0 == bpstrcmp((*ppDc)->szBoardId, &voiceCurrentDgtrCardCfgId[0])))
      {
         break;
      }
   }

   /* Return if no dect && ( no dc match || dc match with zero lines ) */
   if( !dectChanCfg && ( !(*ppDc) || bpstrcmp((*ppDc)->szBoardId, VOICECFG_NOSLIC_STR) == 0 ) )
   {
      /* No voice lines configured */
      return 0;
   }

   /* Initialize filtered list and assign to pointer */
   for( i=0; i< BP_VOICE_FILTERED_MAX_SIZE; i++ )
   {
      g_voice_filteredBp[i].id = bp_last;
   }
   pNewElem = &g_voice_filteredBp[0];

   /* 1 - Configure FXS/FXO related interface enables */
   if( (*ppDc)->flags & BP_FLAG_DSP_APMHAL_ENABLE )
   {
      /* Enable APM interface */
      BP_VOICE_ADD_INTERFACE_PINMUX( pNewElem, BP_PINMUX_FNTYPE_APM );
   }

   if( (*ppDc)->flags & BP_FLAG_DSP_PCMHAL_ENABLE )
   {
      /* Enable PCM interface */
      BP_VOICE_ADD_INTERFACE_PINMUX( pNewElem, BP_PINMUX_FNTYPE_PCM );
   }

   /* Iterate through devices to determine device type */
   for( i=0; (i < BP_MAX_VOICE_DEVICES) && ((*ppDc)->voiceDevice[i].deviceType != BP_VD_NONE); i++ )
   {
      switch( (*ppDc)->voiceDevice[i].deviceType )
      {
         case BP_VD_SILABS_3239:
         case BP_VD_SILABS_32392:
         {
            bSi3239x = 1;

            if( !bHspi )
            {
               /* Enable HS_SPI interface */
               BP_VOICE_ADD_INTERFACE_PINMUX( pNewElem, BP_PINMUX_FNTYPE_HS_SPI );
               bHspi = 1;
            }
         }
         break;

         case BP_VD_ZARLINK_9540:
         case BP_VD_ZARLINK_9541:
         {
            bLe954x = 1;
         }
         break;

         default:
         {
            /* PCM SLACs */
            if( (*ppDc)->voiceDevice[i].SPI_SS_Bx != BP_SPI_SS_NOT_REQUIRED && !bHspi )
            {
               /* Enable SPI for PCM SLACS only if no ISI/ZSI && no APM SLICs configured */
               BP_VOICE_ADD_INTERFACE_PINMUX( pNewElem, BP_PINMUX_FNTYPE_HS_SPI );
               bHspi = 1;
            }
         }
         break;
      }
   }

   /* 2 - Configure DECT related interface enable */
   if( dectChanCfg )
   {
      /* Enable DECT interface */
      BP_VOICE_ADD_INTERFACE_PINMUX( pNewElem, BP_PINMUX_FNTYPE_DECT );
   }

   /* 3 - Start adding signals based on interface enables and dc selection */
   pElem = pBpStartElem;
   while( pElem->id != bp_last )
   {
      switch ( pElem->id )
      {
         case bp_usGpioFxsFxoRst1:
         case bp_usGpioFxsFxoRst2:
         case bp_usGpioFxsFxoRst3:
         case bp_usGpioFxsFxoRst4:
         {
            /* Check if PCM interface is enabled. Only copy over elem if it
             * doesnt already exist in filtered list. This accounts for sibling
             * board overrides */
            if( BpIsIntfEnabled(BP_PINMUX_FNTYPE_PCM, &g_voice_filteredBp[0]) &&
                  !BpElemExists( &g_voice_filteredBp[0], pElem->id ) )
            {
               BP_VOICE_ADD_SIGNAL_PINMUX( pNewElem, pElem->id, pElem->u.us );
            }
         }
         break;

         case bp_usGpioVoipRelayCtrl1:
         case bp_usGpioVoipRelayCtrl2:
         {
            /* Only copy over elem if it doesnt already exist in filtered list.
             * This accounts for sibling board overrides */
            if( !BpElemExists( &g_voice_filteredBp[0], pElem->id ) )
            {
               /* TODO: Maybe only add this for FXO enabled devices */
               BP_VOICE_ADD_SIGNAL_PINMUX( pNewElem, pElem->id, pElem->u.us );
            }
         }
         break;

         case bp_usGpioDectRst:
         {
            /* Check if DECT interface is enabled. Only copy over elem if it
             * doesnt already exist in filtered list. This accounts for sibling
             * board overrides */
            if( BpIsIntfEnabled(BP_PINMUX_FNTYPE_DECT, &g_voice_filteredBp[0]) &&
                  !BpElemExists( &g_voice_filteredBp[0], pElem->id ) )
            {
               BP_VOICE_ADD_SIGNAL_PINMUX( pNewElem, pElem->id, pElem->u.us );
            }
         }
         break;

         case bp_usZarIfSclk:
         case bp_usZarIfSdout:
         case bp_usZarIfSdin:
         case bp_usZarIfEnable:
         case bp_usGpioLe9540Reset:
         {
            /* Check if APM and Le954x are enabled */
            if( BpIsIntfEnabled(BP_PINMUX_FNTYPE_APM, &g_voice_filteredBp[0]) && bLe954x )
            {
               BP_VOICE_ADD_SIGNAL_PINMUX( pNewElem, pElem->id, pElem->u.us );
            }
         }
         break;

         case bp_usHvgMaxPwm:
         case bp_usSi32392SpiSSNum:
         {
            /* Check if APM and Si3239x are enabled */
            if( BpIsIntfEnabled(BP_PINMUX_FNTYPE_APM, &g_voice_filteredBp[0]) && bSi3239x &&
                  !BpElemExists( &g_voice_filteredBp[0], pElem->id ) )
            {
               BP_VOICE_ADD_SIGNAL_PINMUX( pNewElem, pElem->id, pElem->u.us );
            }
         }
         break;

         default:
         break;
      }

      pElem++;

      /* Assign parent bp if pointer present */
      if( pElem->id == bp_elemTemplate )
      {
         pElem = pElem->u.bp_elemp;
      }
   }

   /* 4 - Add pointer to data bp at the end of voice bp */
   pNewElem->id = bp_elemTemplate;
   pNewElem++->u.bp_elemp = pCurrentDataBp;
   pNewElem->id = bp_last;
   return ( &g_voice_filteredBp[0] );
}

/**************************************************************************
* Name       : BpIsIntfEnabled
*
* Description: Checks if a particular interface is enabled in boardparms
*
* Parameters : interfaceFlag - Flag representing interface
*              pBoardParms   - pointer to boardparams
*
* Returns    : 1 if found, 0 otherwise
***************************************************************************/
static int BpIsIntfEnabled( unsigned int interfaceFlag, bp_elem_t * pBoardParms )
{
   if( !pBoardParms )
   {
      return 0;
   }

   for( ; pBoardParms->id != bp_last; pBoardParms++ )
   {
      if( (pBoardParms->id == bp_ulInterfaceEnable) && (pBoardParms->u.ul == interfaceFlag) )
      {
         return 1;
      }
   }

   return 0;
}

/**************************************************************************
* Name       : BpElemExists
*
* Description: Checks if element with specific id already exists in bp
*
* Parameters : pBoardParms - pointer to boardparams
*              id   - element id to look for
*
* Returns    : 1 if found, 0 otherwise
***************************************************************************/
static int BpElemExists( bp_elem_t * pBoardParms, enum bp_id  id )
{
   bp_elem_t * tempPtr = pBoardParms;
   bp_elem_t * pFoundElem;

   pFoundElem = BpGetElem(id, &tempPtr, bp_last);
   return (id == pFoundElem->id);
}

#if !defined(_CFE_)
EXPORT_SYMBOL(BpSetDectPopulatedData);
EXPORT_SYMBOL(BpDectPopulated);
EXPORT_SYMBOL(BpGetVoiceParms);
EXPORT_SYMBOL(BpSetVoiceBoardId);
EXPORT_SYMBOL(BpGetVoiceBoardId);
EXPORT_SYMBOL(BpGetVoiceBoardIds);
EXPORT_SYMBOL(BpGetVoiceDectType);
#endif /* !defined(_CFE_) */
