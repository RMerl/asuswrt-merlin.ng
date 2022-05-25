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
#ifdef __KERNEL__
#include <linux/kernel.h>
#include <linux/export.h>
#endif


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
                                        &g_voiceBoard_SI32281_ISI_LCCB,   \
                                        &g_voiceBoard_SI32284_ISI_LCCB,   \
                                        &g_voiceBoard_SI32267

#define SLICSLAC_LIST_ZSI_NOFXO         &g_voiceBoard_LE88536_ZSI,        \
                                        &g_voiceBoard_LE9641_ZSI_BB,      \
                                        &g_voiceBoard_LE9642_ZSI_BB,      \
                                        &g_voiceBoard_LE9642_ZSI_IB,      \
                                        &g_voiceBoard_LE9642_ZSI_TB,      \
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
                                        &g_voiceBoard_LE9622_IB,          \
                                        &g_voiceBoard_ZL88601,            \
                                        &g_voiceBoard_ZL88601x2,          \
                                        &g_voiceBoard_SI32176,            \
                                        &g_voiceBoard_SI32176_LCQC,       \
                                        &g_voiceBoard_SI32282_LCCB,       \
                                        &g_voiceBoard_SI32184_LCBC,       \
                                        &g_voiceBoard_SI32286_LCCB,       \
                                        &g_voiceBoard_SI32260,            \
                                        &g_voiceBoard_SI32260_LCUB,       \
                                        &g_voiceBoard_SI32260_LCQC

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
#ifdef __KERNEL__
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
extern int bcm_spi_get_slaveid(int);

/* ---- Private Functions ------------------------------------------------ */
static void bpmemcpy( void* dstptr, const void* srcptr, int size );
static char * bpstrcpy( char* dest, const char* src );
static bp_elem_t * BpGetVoiceBoardStartElemPtr(const char * pszBaseBoardId );

#ifdef __KERNEL__
static int bpstrlen( char * src );
static int BpPcmTSCfg( BP_VOICE_CHANNEL *pCh, BP_VOICE_CHANNEL_INTERNAL *pIntCh,
                       unsigned int *pcmRxChanMask, unsigned int *pcmTxChanMask,
                       unsigned int *pcmChanId);
#endif

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

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_SI32281_ISI_LCCB =
{
   VOICECFG_SI32281_ISI_LCCB_STR,   /*Daughter board ID */
   {
      {
         /* Device Type */
         BP_VD_SILABS_32281_ISI,
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

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_SI32260_LCUB =
{
   VOICECFG_SI32260_LCUB_STR,   /*Daughter card ID */
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
   BP_VD_LCUB,
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

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_LE9642_ZSI_TB =
{
   VOICECFG_LE9642_ZSI_TB_STR,   /* szBoardId */
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
   BP_VD_TRIPLE_BUCKBOOST,
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

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_LE9622_IB =
{
   VOICECFG_LE9622_IB_STR,   /* szBoardId */
   {
      {
         /* Device Type */
         BP_VD_ZARLINK_9622_IB,
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
   BP_VD_INVBOOST,
   /* General-purpose flags */
   ( BP_FLAG_DSP_PCMHAL_ENABLE )
};

VOICE_DAUGHTER_BOARD_PARMS g_voiceBoard_LE9642_ZSI_IB =
{
   VOICECFG_LE9642_ZSI_IB_STR,   /* szBoardId */
   {
      {
         /* Device Type */
         BP_VD_ZARLINK_9642_ZSI_IB,
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

BP_VOICE_CHANNEL_INTERNAL g_iDectStdCfg[BP_MAX_DECT_DEVICE * BP_MAX_CHANNELS_PER_DEVICE] =
{
   BP_CHAN( BP_VC_ACTIVE, BP_VCTYPE_DECT, BP_VC_COMP_LINEAR, BP_VC_16KHZ ),
   BP_CHAN( BP_VC_ACTIVE, BP_VCTYPE_DECT, BP_VC_COMP_LINEAR, BP_VC_16KHZ ),
   BP_CHAN( BP_VC_ACTIVE, BP_VCTYPE_DECT, BP_VC_COMP_LINEAR, BP_VC_16KHZ ),
   BP_CHAN( BP_VC_ACTIVE, BP_VCTYPE_DECT, BP_VC_COMP_LINEAR, BP_VC_16KHZ ),
};

static VOICE_DAUGHTER_BOARD_PARMS *g_dCardList[] = {
   SLICSLAC_LIST_COMMON_FXO,
   SLICSLAC_LIST_4FXS_FXO,
   &g_voiceBoard_ZL88601x4_8FXS,
   0
};
/* End of Daughter Card Definitions */


/*
 * -------------------------- Voice Mother Board Configs ------------------------------
 */

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
   {bp_iDectCfg,                .u.ptr = g_iDectStdCfg},
   {bp_daughterCardList,        .u.ptr = g_963138_dCardListFull},
   {bp_last}
};

static bp_elem_t g_voice_bcm963138dvt_p300[] = {
   {bp_cpBoardId,               .u.cp = "963138DVT_P300"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm963138dvt},
   {bp_last}
};

static bp_elem_t g_voice_bcm963138ref_bmu[] = {
   {bp_cpBoardId,               .u.cp = "963138REF_BMU"},
   {bp_daughterCardList,        .u.ptr = g_963138_dCardListNoFxoRst},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm963138dvt},
   {bp_last}
};

static bp_elem_t g_voice_bcm963138ref_bgw[] = {
   {bp_cpBoardId,               .u.cp = "963138REF_BGW"},
   {bp_daughterCardList,        .u.ptr = g_963138_dCardListNoFxoRst},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm963138ref_bmu},
   {bp_last}
};

static bp_elem_t g_voice_bcm963138bmu_p202[] = {
   {bp_cpBoardId,               .u.cp = "963138BMU_P202"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm963138ref_bmu},
   {bp_last}
};

static bp_elem_t g_voice_bcm963138ref_p402[] = {
   {bp_cpBoardId,               .u.cp = "963138REF_P402"},
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
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm963138ref_p402},
   {bp_last}
};

static bp_elem_t g_voice_bcm963138_gfast2[] = {
   {bp_cpBoardId,               .u.cp = "963138GFAST2"},
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
   {bp_iDectCfg,                .u.ptr = g_iDectStdCfg},
   {bp_daughterCardList,        .u.ptr = g_963138_dCardListFull},
   {bp_last}
};

static bp_elem_t g_voice_bcm963138rref_rnc[] = {
   {bp_cpBoardId,               .u.cp = "63138RREF_RNC"},
   {bp_iDectCfg,                .u.ptr = g_iDectStdCfg},
   {bp_daughterCardList,        .u.ptr = g_963138_dCardListFull},
   {bp_last}
};

static VOICE_DAUGHTER_BOARD_PARMS * g_963138ref_lte_dCardList[] = {
   SLICSLAC_LIST_COMMON_FXO,
   0
};

static bp_elem_t g_voice_bcm963138ref_lte[] = {
   {bp_cpBoardId,               .u.cp = "963138REF_LTE"},
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
   {bp_daughterCardList,        .u.ptr = g_963138ref_rnc_dCardList},
   {bp_last}
};

static bp_elem_t g_voice_bcm963138ref_rncP400[] = {
   {bp_cpBoardId,               .u.cp = "138REFrncP400"},
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
   0
};

static VOICE_DAUGHTER_BOARD_PARMS * g_963148_dCardListFullNoFxoRst[] = {
   SLICSLAC_LIST_COMMON_NOFXO,
   0
};

static bp_elem_t g_voice_bcm963148dvt[] = {
   {bp_cpBoardId,               .u.cp = "963148DVT"},
   {bp_iDectCfg,                .u.ptr = g_iDectStdCfg},
   {bp_daughterCardList,        .u.ptr = g_963148_dCardListFull},
   {bp_last}
};

static bp_elem_t g_voice_bcm963148dvt_p300[] = {
   {bp_cpBoardId,               .u.cp = "963148DVT_P300"},
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

#if defined(_BCM94908_) || defined(CONFIG_BCM94908)

static VOICE_DAUGHTER_BOARD_PARMS * g_962118_dCardList[] = {
   SLICSLAC_LIST_COMMON_FXO,
   0
};

static bp_elem_t g_voice_bcm962118ref[] = {
   {bp_cpBoardId,               .u.cp = "962118REF"},
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
   {bp_daughterCardList,        .u.ptr = g_96858_dCardList},
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
   {bp_daughterCardList,        .u.ptr = g_96846_dCardList},
   {bp_last}
};
static bp_elem_t g_voice_bcm96846refp[] = {
   {bp_cpBoardId,               .u.cp = "968460REFP"},
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
static bp_elem_t g_voice_bcm968462ref[] = {
   {bp_cpBoardId,               .u.cp = "968462REF"},
   {bp_daughterCardList,        .u.ptr = g_968461PRW_dCardList},
   {bp_last}
};
static bp_elem_t g_voice_bcm968462spw[] = {
   {bp_cpBoardId,               .u.cp = "968462SPW"},
   {bp_daughterCardList,        .u.ptr = g_968461PRW_dCardList},
   {bp_last}
};
static bp_elem_t g_voice_bcm968460rgw[] = {
   {bp_cpBoardId,               .u.cp = "968460RGW"},
   {bp_daughterCardList,        .u.ptr = g_968461PRW_dCardList},
   {bp_last}
};

static bp_elem_t g_voice_bcm968462egr[] = {
   {bp_cpBoardId,               .u.cp = "968462EGR"},
   {bp_daughterCardList,        .u.ptr = g_968461PRW_dCardList},
   {bp_last}
};

static bp_elem_t * g_VoiceBoardParms[] = {
   g_voice_bcm968460sv,
   g_voice_bcm96846refp,
   g_voice_bcm96846ref,
   g_voice_bcm968461prw,
   g_voice_bcm968462ref,
   g_voice_bcm968462spw,
   g_voice_bcm968462egr,
   g_voice_bcm968460rgw,
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
   {bp_daughterCardList,        .u.ptr = g_963158_dCardList},
   {bp_last}
};

static bp_elem_t g_voice_bcm963158ref1d[] = {
   {bp_cpBoardId,               .u.cp = "963158REF1D"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm963158ref1},
   {bp_last}
};


static bp_elem_t g_voice_bcm963158ref2[] = {
   {bp_cpBoardId,               .u.cp = "963158REF2"},
   {bp_daughterCardList,        .u.ptr = g_963158_dCardList},
   {bp_last}
};

static bp_elem_t g_voice_bcm963158ref2d[] = {
   {bp_cpBoardId,               .u.cp = "963158REF2D"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm963158ref2},
   {bp_last}
};


static bp_elem_t g_voice_bcm963158dvt[] = {
   {bp_cpBoardId,               .u.cp = "963158DVT"},
   {bp_daughterCardList,        .u.ptr = g_963158_dCardList},
   {bp_last}
};

static bp_elem_t g_voice_bcm963158ref3d[] = {
   {bp_cpBoardId,               .u.cp = "963158REF3D"},
   {bp_daughterCardList,        .u.ptr = g_963158_dCardList},
   {bp_last}
};

static bp_elem_t g_voice_bcm963153ref4d[] = {
   {bp_cpBoardId,               .u.cp = "963153REF4D"},
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

static bp_elem_t g_voice_bcm963158ref3d_p200  [] = {
   {bp_cpBoardId,               .u.cp = "158REF3D_P200"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm963158ref3d},
   {bp_last}
};

static bp_elem_t * g_VoiceBoardParms[] = {
   g_voice_bcm963158ref1,
   g_voice_bcm963158ref1d, 
   g_voice_bcm963158ref2,
   g_voice_bcm963158ref2d,
   g_voice_bcm963158dvt,
   g_voice_bcm963158ref3d,
   g_voice_bcm963153ref4d,
   g_voice_bcm963158ref3,
   g_voice_bcm963158ref3_p20x,
   g_voice_bcm963158ref3d_p200,
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
   {bp_daughterCardList,        .u.ptr = g_96856_dCardList},
   {bp_last}
};
static bp_elem_t g_voice_bcm968360bg[] = {
   {bp_cpBoardId,               .u.cp = "968360BG"},
   {bp_daughterCardList,        .u.ptr = g_96856_dCardList},
   {bp_last}
};
static bp_elem_t g_voice_bcm968560ref[] = {
   {bp_cpBoardId,               .u.cp = "968560REF"},
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

#if defined(_BCM963178_) || defined(CONFIG_BCM963178)

static VOICE_DAUGHTER_BOARD_PARMS * g_963178_dCardList[] = {
   SLICSLAC_LIST_COMMON_NOFXO,
   0
};
static VOICE_DAUGHTER_BOARD_PARMS * g_963178REF3_dCardList[] = {
   &g_voiceBoard_NOSLIC,
   &g_voiceBoard_LE9642_ZSI_BB,
   0
};
static bp_elem_t g_voice_bcm963178sv[] = {
   {bp_cpBoardId,               .u.cp = "963178SV"},
   {bp_daughterCardList,        .u.ptr = g_963178_dCardList},
   {bp_last}
};
static bp_elem_t g_voice_bcm963178sv_p200[] = {
   {bp_cpBoardId,               .u.cp = "963178SV_P200"},
   {bp_daughterCardList,        .u.ptr = g_963178_dCardList},
   {bp_last}
};
static bp_elem_t g_voice_bcm963178ref1[] = {
   {bp_cpBoardId,               .u.cp = "963178REF1"},
   {bp_daughterCardList,        .u.ptr = g_963178_dCardList},
   {bp_last}
};
static bp_elem_t g_voice_bcm963178ref3[] = {
   {bp_cpBoardId,               .u.cp = "963178REF3"},
   {bp_daughterCardList,        .u.ptr = g_963178REF3_dCardList},
   {bp_last}
};
static bp_elem_t g_voice_bcm963178ref2[] = {
   {bp_cpBoardId,               .u.cp = "963178REF2"},
   {bp_daughterCardList,        .u.ptr = g_963178_dCardList},
   {bp_last}
};
static bp_elem_t g_voice_bcm963178ref2_p200[] = {
   {bp_cpBoardId,               .u.cp = "963178REF2_P200"},
   {bp_daughterCardList,        .u.ptr = g_963178_dCardList},
   {bp_last}
};
static bp_elem_t g_voice_bcm963178ref2_p300[] = {
   {bp_cpBoardId,               .u.cp = "963178REF2_P300"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm963178ref2_p200},
   {bp_last}
};

static bp_elem_t g_voice_bcm963178ref6[] = {
   {bp_cpBoardId,               .u.cp = "963178REF6"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm963178ref2_p200},
   {bp_last}
};


static bp_elem_t g_voice_bcm963178ref5[] = {
   {bp_cpBoardId,               .u.cp = "963178REF5"},
   {bp_daughterCardList,        .u.ptr = g_963178REF3_dCardList},
   {bp_last}
};

static bp_elem_t g_voice_bcm963178ref3_s772[] = {
   {bp_cpBoardId,               .u.cp = "963178REF3_S772"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm963178ref3},
   {bp_last}
};

static bp_elem_t * g_VoiceBoardParms[] = {
   g_voice_bcm963178sv, 
   g_voice_bcm963178sv_p200, 
   g_voice_bcm963178ref1,
   g_voice_bcm963178ref3,
   g_voice_bcm963178ref2,
   g_voice_bcm963178ref2_p200,
   g_voice_bcm963178ref2_p300,
   g_voice_bcm963178ref5,
   g_voice_bcm963178ref6,
   g_voice_bcm963178ref3_s772,
   0
};

#endif

#if defined(_BCM947622_) || defined(CONFIG_BCM947622)

static VOICE_DAUGHTER_BOARD_PARMS * g_947622_dCardList[] = {
   SLICSLAC_LIST_COMMON_NOFXO,   
   0
};

static bp_elem_t g_voice_bcm947622sv[] = {
   {bp_cpBoardId,               .u.cp = "947622SV"},
   {bp_daughterCardList,        .u.ptr = g_947622_dCardList},
   {bp_last}
};

static bp_elem_t g_voice_bcm96755ref1[] = {
   {bp_cpBoardId,               .u.cp = "96755REF1"},
   {bp_daughterCardList,        .u.ptr = g_947622_dCardList},  
   {bp_last}
};

static bp_elem_t g_voice_bcm96755ref1_sg[] = {
   {bp_cpBoardId,               .u.cp = "96755REF1_SG"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm96755ref1},
   {bp_last}
};

static bp_elem_t * g_VoiceBoardParms[] = {
   g_voice_bcm947622sv, 
   g_voice_bcm96755ref1,
   g_voice_bcm96755ref1_sg,
   0
};

#endif

#if defined(_BCM96878_) || defined(CONFIG_BCM96878)

static VOICE_DAUGHTER_BOARD_PARMS * g_96878_dCardList[] = {
   SLICSLAC_LIST_COMMON_NOFXO,
   0
};
static VOICE_DAUGHTER_BOARD_PARMS * g_968781REF_dCardList[] = {
   &g_voiceBoard_NOSLIC,
   &g_voiceBoard_LE9641_ZSI_BB,
   0
};
static VOICE_DAUGHTER_BOARD_PARMS * g_968782GREF_dCardList[] = {
   &g_voiceBoard_NOSLIC,
   &g_voiceBoard_LE9642_ZSI_BB,
   0
};
static bp_elem_t g_voice_bcm968781xsv[] = {
   {bp_cpBoardId,               .u.cp = "968781XSV"},
   {bp_daughterCardList,        .u.ptr = g_96878_dCardList},
   {bp_last}
};
/* The following boards use ZSI interface which is multiplexed internally with PCM */
/* They do not need SPI pins */
static bp_elem_t g_voice_bcm968781ref[] = {
   {bp_cpBoardId,               .u.cp = "968781REF"},
   {bp_daughterCardList,        .u.ptr = g_968781REF_dCardList},
   {bp_last}
};
static bp_elem_t g_voice_bcm968781refs[] = {
   {bp_cpBoardId,               .u.cp = "968781REFS"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm968781ref},
   {bp_last}
};
static bp_elem_t g_voice_bcm968782ref[] = {
   {bp_cpBoardId,               .u.cp = "968782REF"},
   {bp_daughterCardList,        .u.ptr = g_968781REF_dCardList},
   {bp_last}
};
static bp_elem_t g_voice_bcm968782ref2[] = {
   {bp_cpBoardId,               .u.cp = "968782REF2"},
   {bp_daughterCardList,        .u.ptr = g_968781REF_dCardList},
   {bp_last}
};
static bp_elem_t g_voice_bcm968781HREF[] = {
   {bp_cpBoardId,               .u.cp = "968781HREF"},
   {bp_daughterCardList,        .u.ptr = g_968781REF_dCardList},
   {bp_last}
};
static bp_elem_t g_voice_bcm968781REF_4GPHY[] = {
   {bp_cpBoardId,               .u.cp = "968781REF_4GPHY"},
   {bp_daughterCardList,        .u.ptr = g_968781REF_dCardList},
   {bp_last}
};
static bp_elem_t g_voice_bcm968782gref[] = {
   {bp_cpBoardId,               .u.cp = "968782GREF"},
   {bp_daughterCardList,        .u.ptr = g_968782GREF_dCardList},
   {bp_last}
};
static bp_elem_t g_voice_bcm968782xsv[] = {
   {bp_cpBoardId,               .u.cp = "968782XSV"},
   {bp_elemTemplate,            .u.bp_elemp = g_voice_bcm968781xsv},
   {bp_last}
};
static bp_elem_t g_voice_bcm968782gfm[] = {
   {bp_cpBoardId,               .u.cp = "968782GFM"},
   {bp_daughterCardList,        .u.ptr = g_968782GREF_dCardList},
   {bp_last}
};
static bp_elem_t * g_VoiceBoardParms[] = {
   g_voice_bcm968781xsv,
   g_voice_bcm968781ref,
   g_voice_bcm968781refs,
   g_voice_bcm968782ref,
   g_voice_bcm968782ref2,
   g_voice_bcm968781HREF,
   g_voice_bcm968781REF_4GPHY,
   g_voice_bcm968782gref,
   g_voice_bcm968782xsv,
   g_voice_bcm968782gfm,
   0
};
#endif

#if defined(_BCM963146_) || defined(CONFIG_BCM963146)

static VOICE_DAUGHTER_BOARD_PARMS * g_963146_dCardList[] = {
   SLICSLAC_LIST_COMMON_NOFXO,
   0
};
static bp_elem_t g_voice_bcm963146ref1d[] = {
   {bp_cpBoardId,               .u.cp = "963146REF1D"},
   {bp_daughterCardList,        .u.ptr = g_963146_dCardList},
   {bp_last}
};
static bp_elem_t g_voice_bcm963146ref2d[] = {
   {bp_cpBoardId,               .u.cp = "963146REF2D"},
   {bp_daughterCardList,        .u.ptr = g_963146_dCardList},
   {bp_last}
};
static bp_elem_t * g_VoiceBoardParms[] = {
   g_voice_bcm963146ref1d,
   g_voice_bcm963146ref2d,
   0
};

#endif

#if defined(_BCM94912_) || defined(CONFIG_BCM94912)

static VOICE_DAUGHTER_BOARD_PARMS * g_94915_dCardList[] = {
   SLICSLAC_LIST_COMMON_NOFXO,
   0
};
static bp_elem_t g_voice_bcm94915ref1d[] = {
   {bp_cpBoardId,               .u.cp = "94915REF1D"},
   {bp_daughterCardList,        .u.ptr = g_94915_dCardList},
   {bp_last}
};

static bp_elem_t * g_VoiceBoardParms[] = {
   g_voice_bcm94915ref1d,
   0
};

#endif



#if defined(_BCM96855_) || defined(CONFIG_BCM96855)

static VOICE_DAUGHTER_BOARD_PARMS * g_96855_9641zsi_dCardList[] = {
   &g_voiceBoard_NOSLIC,
   &g_voiceBoard_LE9641_ZSI_BB,
   0
};

static VOICE_DAUGHTER_BOARD_PARMS * g_96855_9642zsi_dCardList[] = {
   &g_voiceBoard_NOSLIC,
   &g_voiceBoard_LE9642_ZSI_BB,
   0
};

static VOICE_DAUGHTER_BOARD_PARMS * g_96855_dCardList[] = {
   SLICSLAC_LIST_COMMON_NOFXO,
   0
};

static bp_elem_t g_voice_bcm968550xsv[] = {
   {bp_cpBoardId,               .u.cp = "968550XSV"},
   {bp_usFxsFxo1SpiSSNum,       .u.us = SPI_DEV_0},
   {bp_usGpioFxsFxoRst1,        .u.us = BP_GPIO_15_AL},
   {bp_daughterCardList,        .u.ptr = g_96855_dCardList},
   {bp_last}
};

static bp_elem_t g_voice_bcm968550xsv_pcix2[] = {
   {bp_cpBoardId,               .u.cp = "968550XSV_PCIX2"},
   {bp_usFxsFxo1SpiSSNum,       .u.us = SPI_DEV_0},
   {bp_usGpioFxsFxoRst1,        .u.us = BP_GPIO_15_AL},
   {bp_daughterCardList,        .u.ptr = g_96855_dCardList},
   {bp_last}
};

/* The following board uses ZSI interface which is multiplexed internally with PCM */
/* It does not need SPI pins */
static bp_elem_t g_voice_bcm968550dv21[] = {
   {bp_cpBoardId,               .u.cp = "968550DV21"},
   {bp_daughterCardList,        .u.ptr = g_96855_9642zsi_dCardList},
   {bp_last}
};
static bp_elem_t g_voice_bcm968552cref[] = {
   {bp_cpBoardId,               .u.cp = "968552CREF"},
   {bp_daughterCardList,        .u.ptr = g_96855_9641zsi_dCardList},
   {bp_last}
};

static bp_elem_t * g_VoiceBoardParms[] = {
   g_voice_bcm968550xsv,
   g_voice_bcm968550xsv_pcix2,
   g_voice_bcm968550dv21,
   g_voice_bcm968552cref,
   0
};

#endif



#if !defined(_BCM963138_) && !defined(CONFIG_BCM963138) && !defined(_BCM963148_) && !defined(CONFIG_BCM963148) \
   && !defined(_BCM94908_) && !defined(CONFIG_BCM94908) && !defined(_BCM96858_) && !defined(CONFIG_BCM96858) \
   && !defined(_BCM963158_) && !defined(CONFIG_BCM963158) && !defined(CONFIG_BCM96846) && !defined(CONFIG_BCM96856) \
   && !defined(CONFIG_BCM963178) && !defined(CONFIG_BCM947622) && !defined(CONFIG_BCM96878) && !defined(CONFIG_BCM963146) \
   && !defined(CONFIG_BCM94912) && !defined(CONFIG_BCM96855)

static bp_elem_t * g_VoiceBoardParms[]=
{
   0
};

#endif
/* Voice Boardparams End */

#ifdef CONFIG_DT_SUPPORT_ONLY
/**************************************************************************
* Name       : bpstrcmp
*
* Description: String compare for this file so it does not depend on an OS.
*
* Parameters : [IN] dest - destination string
*              [IN] src - source string
*
* Returns    : -1 - dest < src, 1 - dest > src, 0 dest == src
***************************************************************************/
int bpstrcmp(const char *dest,const char *src)
{
    while (*src && *dest) {
        if (*dest < *src) return -1;
        if (*dest > *src) return 1;
        dest++;
        src++;
    }

    if (*dest && !*src) return 1;
    if (!*dest && *src) return -1;
    return 0;
} /* bpstrcmp */

#include <asm/bug.h>
/**************************************************************************
* Name       : BpGetElem
*
* Description: Private function to walk through the profile
*              and find the desired entry
*
* Parameters : [IN] id             - id to search for
*              [IN/OUT] pstartElem - where to start and where it was found
*              [IN] stopAtId       - id to stop at if the searched id is not found
*                                    (allows grouping and repeated ids)
*
* Returns    : ptr to entry found or to last entry otherwise
***************************************************************************/
bp_elem_t * BpGetElem(enum bp_id id, bp_elem_t **pstartElem, enum bp_id stopAtId)
{
    bp_elem_t * pelem;
#ifdef CONFIG_BP_PHYS_INTF
    unsigned short intfId = -1;
    bp_elem_t *pIntfStart = NULL, *pBpIntfStart = NULL;
#endif
    
    BUG_ON(pstartElem == NULL);

    pelem = *pstartElem;
    while(1) 
    {
        if( pelem->id == bp_last || pelem->id == stopAtId )
            break;

        if( pelem->id == id ) 
        {
#ifdef CONFIG_BP_PHYS_INTF
            /* check if this id belong to an interface. If so, check if it is the intf being override */
            if( pIntfStart != NULL && BpIsPhyIntfInitDone() )
            {
                 pBpIntfStart = BpGetIntfStart(intfId);
                 if( pBpIntfStart == NULL ) /* Something really wrong here. Should always find the intf */
                 {
                     printk("BpGetElem can not find intf %d!!!\n", intfId);
                     break;
                 }

                 if( pBpIntfStart != pIntfStart ) /* this intf is from template, ignore it */
                 {
                     pelem++;
                     (*pstartElem)++;
                     continue;
                 }
             }
             break;
#else
             break;
#endif
        }

#ifdef CONFIG_BP_PHYS_INTF
        // save the intf id and its element ptr
        if ( bp_usIntfId == pelem->id )
        {
            intfId = pelem->u.us;
            pIntfStart = pelem;
        }
        if ( bp_usIntfEnd == pelem->id )
        {
            intfId = -1;
            pIntfStart = NULL;
        }
#endif

        // found template so jump to it
        // any entries after bp_elemTemplate will be ignored
        if ( bp_elemTemplate == pelem->id ) {
            *pstartElem = pelem->u.bp_elemp;
            pelem = *pstartElem;
            // ignoring the first element of this new array
            // because it is always bp_cpBoardId
        }

        pelem++;
        (*pstartElem)++;
    }

    return pelem; 
}

/**************************************************************************
* Name       : BpGetSubCp
*
* Description: Private function to get an char * entry from the profile
*              can be used to search an id within a group by specifying stop id
*
* Parameters : [IN] id         - id to search for
*              [IN] pstartElem - where to start from
*              [IN] stopAtId   - id to stop at if the searched id is not found
*                                (allows grouping and repeated ids)
*
* Returns    : the unsigned char * from the entry
***************************************************************************/
char *BpGetSubCp(enum bp_id id, bp_elem_t *pstartElem, enum bp_id stopAtId )
{
    bp_elem_t *pelem;

    pelem = BpGetElem(id, &pstartElem, stopAtId);
    if (id == pelem->id) { 
        return pelem->u.cp;
    } else { 
        return (char *)BP_NOT_DEFINED;
    }
}

/**************************************************************************
* Name       : BpGetSubPtr
*
* Description: Private function to get an void * entry from the profile
*              can be used to search an id within a group by specifying stop id
*
* Parameters : [IN] id         - id to search for
*              [IN] pstartElem - where to start from
*              [IN] stopAtId   - id to stop at if the searched id is not found
*                                (allows grouping and repeated ids)
*
* Returns    : the unsigned char * from the entry
***************************************************************************/
void *BpGetSubPtr(enum bp_id id, bp_elem_t *pstartElem, enum bp_id stopAtId )
{
    bp_elem_t *pelem;

    pelem = BpGetElem(id, &pstartElem, stopAtId);
    if (id == pelem->id) {
        return pelem->u.ptr;
    } else {
        return (void *)0;
    }
}
#endif

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

#ifdef __KERNEL__
static int bpstrlen( char * src )
{
   char *s;

   for(s = src; (s != 0) && (*s != 0); ++s);

   return(s - src);
}
#endif

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

#ifdef __KERNEL__
/*****************************************************************************
 * Name:          find_daughtercard()
 *
 * Description:   Finds the daughter card pointer 
 *
 * Parameters:    szDaughterCardId - The daughter board ID that is being used.
 *
 * Returns:       If the daughter card is not found, returns NULL.
 *                If everything goes properly, returns its pointer.
 *
 *****************************************************************************/
static VOICE_DAUGHTER_BOARD_PARMS *find_daughtercard(char* szDaughterCardId)
{
   int i;
   VOICE_DAUGHTER_BOARD_PARMS *pDc;
   for (i=0;;i++) {
      pDc= g_dCardList[i];
      if (!pDc)
        return NULL;
      if( bpstrcmp(pDc->szBoardId, szDaughterCardId) == 0 )
         break;
   }
   return pDc;
}

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
   VOICE_DAUGHTER_BOARD_PARMS *pDc;

   BP_VOICE_CHANNEL_INTERNAL *dectChanCfg = NULL;
   BP_VOICE_DEVICE *vd = NULL;
   BP_VOICE_DEVICE_INTERNAL *vdi = NULL;

   int gId = 0;
   int deviceCount = 0;
   int i, j, pass;

   unsigned int pcmRxChanMask = 0;
   unsigned int pcmTxChanMask = 0;
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

   /* Succesfully found base board + daughter card combination
    * Must now fill the vp structure with data and copy to voiceParms
    * First set base board and daughter board strings */
   bpmemcpy(vp.szBoardId, pszVoiceDaughterCardId, bpstrlen(pszVoiceDaughterCardId));
   bpmemcpy(vp.szBaseBoardId, pszBaseBoardId, bpstrlen(pszBaseBoardId));

   /* Get dectcfg pointer */
   if( BpDectPopulated() == BP_DECT_POPULATED )
   {
      dectChanCfg = g_iDectStdCfg;
   }

   pDc = find_daughtercard(pszVoiceDaughterCardId);
   if (!pDc)
   {
      /* No matching daughtercard list was found */
      printk("%s: Configured daughter board '%s' not found\n", __FUNCTION__, pszVoiceDaughterCardId);
      return BP_BOARD_ID_NOT_FOUND;
   }

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
      vd->requiresReset      = 1;

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

            /* Handle the ZSI/ISI devices */
            if(vdi->SPI_SS_Bx == BP_SPI_SS_NOT_REQUIRED)
            {
               /* Current device _IS_ a ZSI/ISI device. */
               vd->spiCtrl.spiDevId = bcm_spi_get_slaveid(BP_SPI_SS_ZSIISI);
               if(vd->spiCtrl.spiDevId == BP_NOT_DEFINED)
               {
                  /* Failure - Tried to use a ZSI/ISI chip on a board which does not support it*/
                  return BP_NO_ZSI_ON_BOARD_ERR;
               }

               /* Assign SPI associated index on header */
               vd->spiCtrl.spiIndex = 0;
            }
            else
            {
               /* Assign system SPI device ID */
               vd->spiCtrl.spiDevId = bcm_spi_get_slaveid(vdi->SPI_SS_Bx);
               if(vd->spiCtrl.spiDevId == BP_NOT_DEFINED)
               {
                  /* Failure - Tried to use a ZSI/ISI chip on a board which does not support it*/
                  return BP_VALUE_NOT_DEFINED;
               }
               /* Assign SPI associated index on header */
               vd->spiCtrl.spiIndex = vdi->SPI_SS_Bx;
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
#endif

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
   int i;
   VOICE_DAUGHTER_BOARD_PARMS *pDc;
   for (i=0;;i++) {
      pDc= g_dCardList[i];
      if (!pDc)
        return BP_BOARD_ID_NOT_FOUND;
      if( bpstrcmp(pDc->szBoardId, pszVoiceDaughterCardId) == 0 ) {
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

#ifdef __KERNEL__
EXPORT_SYMBOL(BpSetDectPopulatedData);
EXPORT_SYMBOL(BpDectPopulated);
EXPORT_SYMBOL(BpGetVoiceParms);
EXPORT_SYMBOL(BpSetVoiceBoardId);
EXPORT_SYMBOL(BpGetVoiceBoardId);
EXPORT_SYMBOL(BpGetVoiceBoardIds);
EXPORT_SYMBOL(BpGetVoiceDectType);
#endif
