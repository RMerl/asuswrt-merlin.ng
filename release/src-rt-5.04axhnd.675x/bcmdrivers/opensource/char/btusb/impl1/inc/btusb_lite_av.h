/*
 * <:copyright-BRCM:2015:GPL/GPL:standard
 * 
 *    Copyright (c) 2015 Broadcom 
 *    All Rights Reserved
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 * :>
 */

#ifndef BTUSB_LITE_AV_H
#define BTUSB_LITE_AV_H

typedef UINT8 tBTA_AV_CODEC;    /* codec type */
typedef UINT8 tBTA_AV_CHNL;     /* the channel: audio/video */
typedef UINT8 tBTA_AV_HNDL;     /* the handle: ((hdi + 1)|chnl) */

/* data type for the Audio Codec Information*/
typedef struct
{
    UINT16  bit_rate;       /* SBC encoder bit rate in kbps */
    UINT16  bit_rate_busy;  /* SBC encoder bit rate in kbps */
    UINT16  bit_rate_swampd;/* SBC encoder bit rate in kbps */
    UINT8   busy_level;     /* Busy level indicating the bit-rate to be used */
    UINT8   codec_info[AVDT_CODEC_SIZE];
    UINT8   codec_type;     /* Codec type */
} tBTA_AV_AUDIO_CODEC_INFO;

/* type for AV stream control block on Lite stack*/
struct btusb_lite_av_scb
{
    BUFFER_Q            out_q;          /* used for audio channels only */
    BD_ADDR             peer_addr;      /* peer BD address */
    UINT16              l2c_cid;        /* L2CAP channel ID */
    tBTA_AV_CODEC       codec_type;     /* codec type */
    BOOLEAN             cong;           /* TRUE if AVDTP congested */
    tBTA_AV_CHNL        chnl;           /* the channel: audio/video */
    tBTA_AV_HNDL        hndl;           /* the handle: ((hdi + 1)|chnl) */
    UINT8               avdt_handle;    /* AVDTP handle */
    UINT8               hdi;            /* the index to SCB[] */
    UINT8               l2c_bufs;       /* the number of buffers queued to L2CAP */
    BOOLEAN             started;     /* TRUE if stream started from call-out perspective */
    BOOLEAN             start_stop_flag;/* TRUE when snk is INT and bta_av_start_ok() decides that */
};

/* these bits are defined for btusb_lite_av_cb.multi_av */
#define BTA_AV_MULTI_AV_SUPPORTED   0x01
#define BTA_AV_MULTI_AV_IN_USE      0x02

/*
 * Globals
 */
extern int pcm0_mute;

/*******************************************************************************
**
** Function         btusb_lite_av_add
**
** Description      Add (Sync) an AV channel.
**
**
** Returns          None.
**
*******************************************************************************/
void btusb_lite_av_add(struct btusb *p_dev, tBTA_AV_SYNC_INFO *p_sync_info,
        UINT8 multi_av_supported, UINT16 curr_mtu);

/*******************************************************************************
**
** Function         btusb_lite_av_remove
**
** Description      Remove (Cleanup) an AV channel.
**
**
** Returns          None.
**
*******************************************************************************/
void btusb_lite_av_remove(struct btusb *p_dev, UINT8 scb_idx,
        UINT8 audio_open_cnt, UINT16 curr_mtu);

/*******************************************************************************
**
** Function         btusb_lite_av_start
**
** Description      Start AV
**
**
** Returns          Pointer to scb or NULL if index is out of range or scb
**                  is not allocated.
**
*******************************************************************************/
void btusb_lite_av_start(struct btusb *p_dev, UINT8 scb_idx, UINT8 start_stop_flag,
        UINT8 audio_open_cnt, tBTA_AV_AUDIO_CODEC_INFO *p_codec_cfg, UINT8 streaming_type);

/*******************************************************************************
**
** Function         btusb_lite_av_stop
**
** Description      Start AV
**
**
** Returns          Pointer to scb or NULL if index is out of range or scb
**                  is not allocated.
**
*******************************************************************************/
void btusb_lite_av_stop(struct btusb *p_dev, UINT8 scb_idx, UINT8 audio_open_cnt);

/*******************************************************************************
**
** Function         btusb_lite_av_suspend
**
** Description      Suspend AV
**
** Returns          none.
**
*******************************************************************************/
void btusb_lite_av_suspend(struct btusb *p_dev, UINT8 scb_idx, UINT8 audio_open_cnt);

#endif /* BTUSB_LITE_AV_H*/

