/*
    Copyright 2000-2024 Broadcom Corporation

    <:label-BRCM:2024:DUAL/GPL:standard

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
 * File Name  : vd_voice.h
 *
 * Description: This file contains definitions and function prototypes for
 *              the BCM63xx voice board parameter access functions.
 *
 ***************************************************************************/

#ifndef _VD_VOICE_H
#define _VD_VOICE_H

#ifdef __cplusplus
extern "C" {
#endif

/*
** Main structure for defining the board parameters and used by boardHal
** for proper initialization of the DSP and devices (SLACs, etc.)
*/
/* Maximum size of the board id string. */
#define VD_BOARD_ID_LEN                         16
/* Maximum number of devices in the system (on the board).
** Devices can refer to DECT, SLAC/SLIC, or SLAC/DAA combo. */
#define VD_MAX_VOICE_DEVICES                    9

/* Maximum numbers of channels per SLAC. */
#define VD_MAX_CHANNELS_PER_DEVICE              2

/* Maximum number of devices in the system (on the board). */
#define VD_MAX_VOICE_DEVICES                    9

/* Maximum numbers of channels per SLAC. */
#define VD_MAX_CHANNELS_PER_DEVICE              2

/* Maximum number of voice channels in the system.
** This represents the sum of all channels available on the devices in the system */
#define VD_MAX_VOICE_CHAN              (VD_MAX_VOICE_DEVICES * VD_MAX_CHANNELS_PER_DEVICE)

/* Macro that returns the Number of PCM timeslots in use by specificed channel
 * parameters */
#define VD_PCM_TS_COUNT(compression, rate)  \
   ((compression == VD_VC_COMP_LINEAR)      \
      ? (rate == VD_VC_16KHZ ? 4 : 2)       \
      : (rate == VD_VC_16KHZ ? 2 : 1))

#define  VD_VOICE_ZARLINK                       0
#define  VD_VOICE_SILABS                        1

typedef struct
{
   int spiDevId;               /* SPI device id */
   int spiIndex;               /* index number on the header */
} VD_VOICE_SPI_CONTROL;

typedef struct
{
   /* Timeslot overrides for receive and transmit direction. Timeslot indices
   * are byte offsets in the PCM frame, given in the order of DMA copy. If no
   * override is set, timeslots will be automatically computed on bootup. By
   * default, it is assumed the device is big endian. Overrides will be needed
   * for little-endian devices. Unconfigured timeslots will be set to -1 */
   int rxTimeslot[4];
   int txTimeslot[4];
} VD_PCM_TS_CFG;

typedef struct
{
   VD_PCM_TS_CFG ts;        /* timeslot configuration */
   unsigned int  pcmChanId; /* PCM channel ID */
} VD_PCM_CFG;

typedef struct
{
   int chan; /* VD_APM_CHAN - APM channel id */
} VD_APM_CFG;

typedef struct
{
   unsigned int id;          /* global voice channel index */
   int          status;      /* VD_VS_STATUS      - active/inactive */
   int          type;        /* VD_VC_TYPE        - type of device */
   int          sampleComp;  /* VD_VC_SAMPLE_COMP - u-law/a-law (applicable for 8-bit samples) */
   int          sampleRate;  /* VD_VC_SAMPLE_RATE - narrowband/wideband */
   union
   {
      VD_PCM_CFG     pcm;
      VD_APM_CFG     apm;
   } cfg;

} VD_VOICE_CHANNEL;


typedef struct
{
   int                  voiceDeviceType;    /* Specific type of device (Le88276, Si32176, etc.) */
   int                  audioInterfaceType; /* Audio interface type */
   VD_VOICE_SPI_CONTROL spiCtrl;            /* SPI control through dedicated SPI pin or GPIO */
   unsigned int         requiresReset;      /* Does the device requires reset (through GPIO) */
   VD_VOICE_CHANNEL     channel[VD_MAX_CHANNELS_PER_DEVICE]; /* Device channels */

} VD_VOICE_DEVICE;

typedef struct
{
   unsigned short       dectUartGpioTx;
   unsigned short       dectUartGpioRx;
} VD_DECT_UART_CONTROL;

typedef struct VOICE_CARD_PARMS
{
   char                    szBoardId[VD_BOARD_ID_LEN];      /* daughtercard id string */
   unsigned int            numFxsLines;            /* Number of FXS lines in the system */
   unsigned int            numFxoLines;            /* Number of FXO lines in the system */
   VD_VOICE_DEVICE         voiceDevice[VD_MAX_VOICE_DEVICES];  /* Voice devices in the system */
   VD_DECT_UART_CONTROL    dectUartControl;        /* Control for external DECT UART */
   int                     deviceProfile;          /* VD_VOICE_DEVICE_PROFILE - Power supply configuration, if required */
   unsigned int            flags;                  /* General-purpose flags */
}__attribute__((aligned(4),packed)) VOICE_CARD_PARMS;

#ifdef __cplusplus
}
#endif
#endif /* _VD_VOICE_H */

