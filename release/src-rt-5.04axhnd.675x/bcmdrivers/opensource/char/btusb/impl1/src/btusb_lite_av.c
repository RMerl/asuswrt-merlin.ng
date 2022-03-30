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

#include <linux/moduleparam.h>
#include "btusb.h"
#include "bd.h"
#include "btpcm_api.h"
#include "btsbc_api.h"
#ifdef BTUSB_LITE_SEC
#include "btsec_api.h"
#endif

struct btusb_lite_av_sbc_param
{
    int frequency;
    unsigned char nb_blocks;
    unsigned char nb_subbands;
    unsigned char mode;
    unsigned char allocation;
    unsigned char bitpool_min;
    unsigned char bitpool_max;
};


/*  Codec (From BT Spec) */
#define A2D_MEDIA_TYPE_AUDIO        0x00

#define A2D_MEDIA_CT_SBC            0x00    /* SBC Codec */
#define A2D_MEDIA_CT_VEND           0xFF    /* Vendor specific */

/*  SBC Codec (From BT Spec) */
#define CODEC_SBC_LOSC              6

#define CODEC_SBC_FREQ_MASK         0xF0
#define CODEC_SBC_FREQ_48           0x10
#define CODEC_SBC_FREQ_44           0x20
#define CODEC_SBC_FREQ_32           0x40
#define CODEC_SBC_FREQ_16           0x80

#define CODEC_MODE_MASK             0x0F
#define CODEC_MODE_JOIN_STEREO      0x01
#define CODEC_MODE_STEREO           0x02
#define CODEC_MODE_DUAL             0x04
#define CODEC_MODE_MONO             0x08

#define CODEC_SBC_BLOCK_MASK        0xF0
#define CODEC_SBC_BLOCK_16          0x10
#define CODEC_SBC_BLOCK_12          0x20
#define CODEC_SBC_BLOCK_8           0x40
#define CODEC_SBC_BLOCK_4           0x80

#define CODEC_SBC_NBBAND_MASK       0x0C
#define CODEC_SBC_NBBAND_8          0x04
#define CODEC_SBC_NBBAND_4          0x08

#define CODEC_SBC_ALLOC_MASK        0x03
#define CODEC_SBC_ALLOC_LOUDNESS    0x01
#define CODEC_SBC_ALLOC_SNR         0x02

/* SEC codec */
#ifdef BTUSB_LITE_SEC
#define A2D_MEDIA_CT_SEC            0x07    /* Internal SEC Codec type */

struct btusb_lite_av_sec_param
{
    int frequency;
    unsigned char mode;
};
#endif

/* BAV Packet Header Info*/
#define BSA_SV_AV_BAV_HEADER_SIZE       (1 /*BSA_SV_AV_BAV_PACKET_HEADER_SIZE*/ + \
                                         1 /*BTA_AV_SBC_HDR_SIZE*/ + \
                                         12 /*AVDT_MEDIA_HDR_SIZE*/)
#define BSA_SV_AV_BAV_PACKET_HEADER_SBC_MASK    0x80    /* SBC Data */
#define AVDT_MEDIA_OCTET1                       0x80    /* first byte of media packet header */
#define BSA_SV_AV_BAV_RTP_PAYLOAD_TYPE          0x60    /* First Dynamic Payload type */
/* AFBT stack vendor lib requires an l2cap header and length in assemble the BAV frames */
#define BSA_BAV_L2C_HDR_SZ 4
#define BSA_BAV_L2C_FAKE_CID    0x1234    /* l2cap CID for BAV frames */

/*
 * Globals
 */
int pcm0_mute = 0;
module_param(pcm0_mute, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(pcm0_mute, "Mute PCM channel 0");

#define SILENSE_PCM_BUF_SIZE    (2 * 128) /* 128 samples, Stereo */
static const unsigned short btusb_lite_silence_pcm[SILENSE_PCM_BUF_SIZE] = {0};

/*
 * Local functions
 */
static int btusb_lite_av_parse_sbc_codec(struct btusb_lite_av_sbc_param *p_sbc, UINT8 *p_codec);
static int btusb_lite_sbc_get_bitpool(struct btusb_lite_av_sbc_param *p_sbc_param, int target_bitrate);
static void btusb_lite_av_pcm_cback(void *p_opaque, void *p_data, int nb_pcm_frames);
static void btusb_lite_av_send_packet(struct btusb *p_dev, BT_HDR *p_msg);

static int btusb_lite_av_sbc_start(struct btusb *p_dev, UINT8 scb_idx,
        tBTA_AV_AUDIO_CODEC_INFO*p_codec_cfg);

static int btusb_lite_av_parse_vendor_codec(UINT8 *p_codec_info, UINT32 *p_vendor_id, UINT16 *p_vendor_codec_id);

#ifdef BTUSB_LITE_SEC
static int btusb_lite_av_parse_sec_codec(struct btusb_lite_av_sec_param *p_sec_param,
        UINT8 *p_codec_info);
static int btusb_lite_av_sec_start(struct btusb *p_dev, UINT8 scb_idx,
        tBTA_AV_AUDIO_CODEC_INFO*p_codec_cfg);
#endif

static void btusb_lite_clb_send(struct btusb *p_dev, BT_HDR *p_msg, UINT8 avdt_handle,
        UINT8 m_pt, UINT8 option, UINT32 timestamp);

/*******************************************************************************
**
** Function         btusb_lite_av_add
**
** Description      Add (Sync) an AV channel.
**
** Returns          None.
**
*******************************************************************************/
void btusb_lite_av_add(struct btusb *p_dev, tBTA_AV_SYNC_INFO *p_sync_info,
        UINT8 multi_av_supported, UINT16 curr_mtu)
{
    struct btusb_lite_av_cb *p_av_cb = &p_dev->lite_cb.s.av;
    struct btusb_lite_av_scb *p_av_scb;
    int rv;

    p_av_cb->stack_mtu = curr_mtu;   /* Update MTU */
    p_av_cb->curr_mtu = curr_mtu;   /* Update MTU */

    if (p_sync_info->hdi >= BTA_AV_NUM_STRS)
    {
        BTUSB_ERR("Bad AV Index=%d\n", p_sync_info->hdi);
        return;
    }

#if (BTU_MULTI_AV_INCLUDED == TRUE)
    p_av_cb->multi_av &= ~(BTA_AV_MULTI_AV_SUPPORTED);
    p_av_cb->multi_av |= multi_av_supported;
#endif

    p_av_scb = &p_av_cb->scb[p_sync_info->hdi];

    p_av_scb->avdt_handle = p_sync_info->avdt_handle;
    p_av_scb->chnl = p_sync_info->chnl;
    p_av_scb->codec_type = p_sync_info->codec_type;
    p_av_scb->cong = p_sync_info->cong;
    p_av_scb->hdi = p_sync_info->hdi;
    p_av_scb->hndl = p_sync_info->hndl;
    p_av_scb->l2c_bufs = p_sync_info->l2c_bufs;
    p_av_scb->l2c_cid = p_sync_info->l2c_cid;
    memcpy(p_av_scb->peer_addr, p_sync_info->peer_addr, BD_ADDR_LEN);

    if (p_av_cb->pcm.state == PCM_CLOSED)
    {
        /* Open the PCM Channel */
        rv = btpcm_open(p_dev->lite_cb.p_btpcm);
        if (rv < 0)
        {
            BTUSB_ERR("btpcm_open failed\n");
            return;
        }
        p_av_cb->pcm.state = PCM_OPENED;
        p_av_cb->pcm.frequency = -1;
    }
}

/*******************************************************************************
**
** Function         btusb_lite_av_remove
**
** Description      Remove (Cleanup) an AV channel.
**
** Returns          None.
**
*******************************************************************************/
void btusb_lite_av_remove(struct btusb *p_dev, UINT8 scb_idx,
        UINT8 audio_open_cnt, UINT16 curr_mtu)
{
    struct btusb_lite_av_cb *p_av_cb = &p_dev->lite_cb.s.av;
    struct btusb_lite_av_scb *p_av_scb;
    int av_scb;
    int cleanup_needed = 1;
    int rv;

    p_av_cb->curr_mtu = curr_mtu;   /* Update MTU */
    p_av_cb->audio_open_cnt = audio_open_cnt;   /* Update audio_open_cnt */

    if (scb_idx >= BTA_AV_NUM_STRS)
    {
        BTUSB_ERR("Bad Index=%d\n", scb_idx);
        return;
    }

    p_av_scb = &p_av_cb->scb[scb_idx];

    /* Remove AVDT CCB and SCB */
    btusb_lite_avdt_remove_scb(p_dev, p_av_scb->avdt_handle, NULL);

    /* Clear the AV Stream Control Clock */
    memset(p_av_scb, 0, sizeof(*p_av_scb));

    /* Check this is the last AV channel removed */
    p_av_scb = &p_av_cb->scb[0];
    for (av_scb = 0 ; av_scb < BTA_AV_NUM_STRS ; av_scb++, p_av_scb++)
    {
        if (p_av_scb->hndl)
        {
            cleanup_needed = 0;
            break;
        }
    }

    if (cleanup_needed)
    {
        if (p_av_cb->pcm.state == PCM_STARTED)
        {
            /* Stop the PCM Channel */
            rv = btpcm_stop(p_dev->lite_cb.p_btpcm);
            if (rv < 0)
            {
                BTUSB_ERR("btpcm_close failed\n");
            }
            p_av_cb->pcm.state = PCM_CONFIGURED;
        }

        if (p_av_cb->pcm.state != PCM_CLOSED)
        {
            /* Close the PCM Channel */
            rv = btpcm_close(p_dev->lite_cb.p_btpcm);
            if (rv < 0)
            {
                BTUSB_ERR("btpcm_close failed\n");
            }
            p_av_cb->pcm.state = PCM_CLOSED;
        }

        if (p_av_cb->encoder.opened)
        {
            switch(p_av_cb->encoder.type)
            {
            case A2D_MEDIA_CT_SBC:
                btsbc_free(p_av_cb->encoder.channel);
                break;
#ifdef BTUSB_LITE_SEC
            case A2D_MEDIA_CT_SEC:
                btsec_free(p_av_cb->encoder.channel);
                break;
#endif
            default:
                BTUSB_ERR("Unknown Encoder type=%d\n", p_av_cb->encoder.encoder.codec_type);
                break;
            }
            p_av_cb->encoder.opened = 0;
        }
    }
}

/*******************************************************************************
**
** Function         btusb_lite_av_start
**
** Description      Start AV
**
** Returns          None.
**
*******************************************************************************/
void btusb_lite_av_start(struct btusb *p_dev, UINT8 scb_idx, UINT8 start_stop_flag,
        UINT8 audio_open_cnt, tBTA_AV_AUDIO_CODEC_INFO *p_codec_cfg, UINT8 streaming_type)
{
    struct btusb_lite_av_cb *p_av_cb = &p_dev->lite_cb.s.av;
    UINT32 vendor_id;
    UINT16 vendor_codec_id;
    struct btusb_lite_av_scb *p_av_scb;


    if (scb_idx >= BTA_AV_NUM_STRS)
    {
        BTUSB_ERR("Bad scb_idx=%d", scb_idx);
        return;
    }

    /* streaming_type - regular A2DP or CLB */
    p_av_cb->streaming_type = streaming_type;

    if (p_av_cb->streaming_type == AV_LITE_STREAMING_AVDT)
        p_av_cb->p_stream_func = btusb_lite_avdt_send;
    else if (p_av_cb->streaming_type == AV_LITE_STREAMING_CLB)
        p_av_cb->p_stream_func = btusb_lite_clb_send;

    p_av_scb = &p_av_cb->scb[scb_idx];

    if (start_stop_flag)
    {
        p_av_cb->scb[scb_idx].started = FALSE;
        BTUSB_ERR("start_stop_flag TODO!!!");
    }
    else
    {
        /* If the Codec Type is SBC */
        if (p_codec_cfg->codec_type == A2D_MEDIA_CT_SBC)
        {
            if (btusb_lite_av_sbc_start(p_dev, scb_idx, p_codec_cfg) < 0)
            {
                BTUSB_ERR("SBC Stream not started\n");
                return;
            }
        }
        /* Else if the Codec Type is a Vendor Specific Codec */
        else if (p_codec_cfg->codec_type == A2D_MEDIA_CT_VEND)
        {
            if (btusb_lite_av_parse_vendor_codec(p_codec_cfg->codec_info, &vendor_id, &vendor_codec_id) < 0)
            {
                BTUSB_ERR("Unable to parse Vendor Codec\n");
                return;
            }
#ifdef BTUSB_LITE_SEC
            /* If This is the SEC Encoder */
            if ((vendor_id == BTSEC_VENDOR_ID) &&
                (vendor_codec_id == BTSEC_VENDOR_CODEC_ID))
            {
                btusb_lite_av_sec_start(p_dev, scb_idx, p_codec_cfg);
            }
            else
#endif
            /* Add other Vendor Specific Vendor coder Here... */
            {
                BTUSB_ERR("Unsupported Codec VendorId=0x%08x VendorCodecId=0x%04x\n", vendor_id, vendor_codec_id);
            }
        }
        else
        {
            BTUSB_ERR("Unsupported Encoder type=%d\n", p_codec_cfg->codec_type);
        }
    }
}

/*******************************************************************************
**
** Function         btusb_lite_av_parse_vendor_codec
**
** Description      Parse Vendor Id and Vendor Codec Id from Codec information
**
** Returns          Status, VendorId and VendorCodecId
**
*******************************************************************************/
static int btusb_lite_av_parse_vendor_codec(UINT8 *p_codec_info, UINT32 *p_vendor_id, UINT16 *p_vendor_codec_id)
{
    UINT8 byte;
    UINT32 vendor_id;
    UINT16 vendor_codec_id;

    if (!p_codec_info || !p_vendor_id || !p_vendor_codec_id)
    {
        BTUSB_ERR("Bad parameter\n");
        return -1;
    }

    STREAM_TO_UINT8(byte, p_codec_info);               /* Extract LOSC */
    if (byte < (1 + 1 + 4 + 2)) /* Media Type, Media Codec Type, Vid, VCId */
    {
        BTUSB_ERR("Codec LOSC=%d too small\n", byte);
        return -1;
    }

    STREAM_TO_UINT8(byte, p_codec_info);               /* Extract Media Type */
    if (byte != A2D_MEDIA_TYPE_AUDIO)
    {
        BTUSB_ERR("Unsupported Media Type=0x%x\n", byte);
        return -1;
    }

    STREAM_TO_UINT8(byte, p_codec_info);               /* Extract Media Codec Type */
    if (byte != A2D_MEDIA_CT_VEND)
    {
        BTUSB_ERR("Media codec Type=0x%x is not Vendor(0xFF)\n", byte);
        return -1;
    }

    STREAM_TO_UINT32(vendor_id, p_codec_info);         /* Extract Vendor Id */
    STREAM_TO_UINT16(vendor_codec_id, p_codec_info);   /* Extract Vendor Codec Id */

    *p_vendor_id = vendor_id;
    *p_vendor_codec_id = vendor_codec_id;

    BTUSB_INFO("Extracted Codec VendorId=0x%08x VendorCodecId=0x%04x\n", vendor_id, vendor_codec_id);

    return 0;
}

/*******************************************************************************
**
** Function         btusb_lite_av_sbc_start
**
** Description      Start AV SBC Stream
**
** Returns          Status
**
*******************************************************************************/
static int btusb_lite_av_sbc_start(struct btusb *p_dev, UINT8 scb_idx,
        tBTA_AV_AUDIO_CODEC_INFO*p_codec_cfg)
{
    struct btusb_lite_av_cb *p_av_cb = &p_dev->lite_cb.s.av;
    struct btusb_lite_encoder_ccb *p_encoder;
    struct btusb_lite_av_scb *p_av_scb;
    int nb_sbc_frames;
    int rv;
    int bitpool;
    struct btusb_lite_av_sbc_param sbc_param;
    int av_header_len;

    /* Parse SBC Codec Info */
    if (btusb_lite_av_parse_sbc_codec(&sbc_param, p_codec_cfg->codec_info) < 0)
    {
        BTUSB_ERR("Bad SBC Codec. Stream not started\n");
        return -1;
    }

    if (scb_idx >= BTA_AV_NUM_STRS)
    {
        BTUSB_ERR("Bad scb_idx=%d", scb_idx);
        return -1;
    }
    p_av_scb = &p_av_cb->scb[scb_idx];

    /* Calculate the BitPool for this BitRate */
    bitpool = btusb_lite_sbc_get_bitpool(&sbc_param, p_codec_cfg->bit_rate);
    if (bitpool <= 0)
    {
        BTUSB_ERR("btusb_lite_sbc_get_bitpool return wrong bitpool=%d\n", bitpool);
        return -1;
    }
    BTUSB_INFO("SBC BitPool=%d\n", bitpool);

    p_av_cb->timestamp = 0; /* Reset TimeStamp */
    p_av_cb->option = 0; /* No specific Option (RTP and Media Payload Header presents) */
    p_av_cb->m_pt = AVDT_RTP_PAYLOAD_TYPE | A2D_MEDIA_CT_SBC;
    p_av_cb->m_pt &= ~AVDT_MARKER_SET;

    /* Calculate Packet Header Size (HCI, L2CAP, RTP and MediaPayloadHeader) */
    p_av_cb->header_len = BTUSB_LITE_HCI_ACL_HDR_SIZE +
                          BTUSB_LITE_L2CAP_HDR_SIZE +
                          BTUSB_LITE_RTP_SIZE +
                          BTUSB_LITE_SCMS_SIZE +
                          BTUSB_LITE_MEDIA_SIZE;
    /* Calculate AV Header Size */
    av_header_len = BTUSB_LITE_L2CAP_HDR_SIZE +
                    BTUSB_LITE_RTP_SIZE +
                    BTUSB_LITE_SCMS_SIZE +
                    BTUSB_LITE_MEDIA_SIZE;

    /* clear the congestion flag: full stack made it congested when opening */
    p_av_scb->cong = FALSE;
    p_av_scb->started = TRUE;

    /* Get reference to AV's encoder */
    p_encoder = &p_av_cb->encoder;

    if (p_encoder->opened == 0)
    {
        /* Allocate an SBC Channel */
        rv = btsbc_alloc();
        if (rv < 0)
        {
            BTUSB_ERR("btsbc_alloc failed\n");
            return -1;
        }
        p_encoder->opened = 1;
        p_encoder->channel = rv;
        p_encoder->type = A2D_MEDIA_CT_SBC;
    }

    BTUSB_DBG("frequency=%d, nb_blocks=%d, nb_subbands=%d, mode=%d, allocation=%d, bitpool=%d \n",
        sbc_param.frequency, sbc_param.nb_blocks, sbc_param.nb_subbands,
        sbc_param.mode, sbc_param.allocation, bitpool);

    /* Configure the SBC Channel */
    rv = btsbc_config(p_encoder->channel,
            sbc_param.frequency,
            sbc_param.nb_blocks,
            sbc_param.nb_subbands,
            sbc_param.mode,
            sbc_param.allocation,
            (unsigned char)bitpool);
    BTUSB_DBG("sbc_frame_size=%d calculated by btsbc_config()\n", rv);

    if (rv <= 0)
    {
        BTUSB_ERR("btsbc_config failed\n");
        btsbc_free(p_encoder->channel);
        p_encoder->opened = 0;
        return -1;
    }

    /* Save the calculated SBC Frame size */
    p_encoder->encoded_frame_size = rv;
    BTUSB_INFO("encoded_frame_size=%d\n", rv);

    /* Configure the PCM Channel */
    rv = btpcm_config(p_dev->lite_cb.p_btpcm,
            p_dev,
            sbc_param.frequency,
            sbc_param.mode==CODEC_MODE_MONO?1:2,
            16, /* SBC Encoder requires 16 bits per sample */
            btusb_lite_av_pcm_cback);
    if (rv < 0)
    {
        BTUSB_ERR("btpcm_config failed\n");
        return -1;
    }


    /* Calculate and save the PCM frame size */
    p_encoder->pcm_frame_size = sbc_param.nb_blocks * sbc_param.nb_subbands;
    BTUSB_INFO("pcm_frame_size=%d\n", p_encoder->pcm_frame_size);

#if 0
    /* Calculate nb_sbc_frames depending on MTU */
    nb_sbc_frames = (p_av_cb->curr_mtu - av_header_len) / p_encoder->encoded_frame_size;
#else
    nb_sbc_frames = 10;
    if(p_av_cb->stack_mtu < (p_encoder->encoded_frame_size * nb_sbc_frames + av_header_len))
    {   /* if 10 SBC frame is bigger than mtu size, should change nb_sbc_frames value to fit in mtu */
        nb_sbc_frames = (p_av_cb->stack_mtu - av_header_len) / p_encoder->encoded_frame_size;
        p_av_cb->curr_mtu = p_av_cb->stack_mtu;
    }
    else
    {
        p_av_cb->curr_mtu = p_encoder->encoded_frame_size * nb_sbc_frames + av_header_len;
    }
#endif
    BTUSB_INFO("mtu:%d, nb_sbc_frames:%d, pcm_frame_size:%d\n",
              p_av_cb->curr_mtu, nb_sbc_frames, p_encoder->pcm_frame_size);

    /* Calculate the size of the Payload */
    p_av_cb->payload_len = nb_sbc_frames * p_encoder->encoded_frame_size;

    BTUSB_INFO("nb_sbc_frames=%d payload_len=%d\n", nb_sbc_frames, p_av_cb->payload_len);

    /* Start the PCM stream */
    rv = btpcm_start(p_dev->lite_cb.p_btpcm, p_encoder->pcm_frame_size, nb_sbc_frames, 0);
    if (rv < 0)
    {
        BTUSB_ERR("btpcm_start failed\n");
        return -1;
    }
    p_av_cb->pcm.state = PCM_STARTED;
    return 0;
}


#ifdef BTUSB_LITE_SEC
/*******************************************************************************
**
** Function         btusb_lite_av_parse_vendor_codec
**
** Description      Parse Vendor Id and Vendor Codec Id from Codec information
**
** Returns          Status, VendorId and VendorCodecId
**
*******************************************************************************/
static int btusb_lite_av_parse_sec_codec(struct btusb_lite_av_sec_param *p_sec_param,
        UINT8 *p_codec_info)
{
    UINT8 byte;
    UINT32 vendor_id;
    UINT16 vendor_codec_id;

    if (!p_codec_info || !p_sec_param)
    {
        BTUSB_ERR("Bad parameter\n");
        return -1;
    }

    /* Extract/Ignore parameters already checked */
    STREAM_TO_UINT8(byte, p_codec_info);               /* Extract LOSC */
    STREAM_TO_UINT8(byte, p_codec_info);               /* Extract Media Type */
    STREAM_TO_UINT8(byte, p_codec_info);               /* Extract Media Codec Type */
    STREAM_TO_UINT32(vendor_id, p_codec_info);         /* Extract Vendor Id */
    STREAM_TO_UINT16(vendor_codec_id, p_codec_info);   /* Extract Vendor Codec Id */

    STREAM_TO_UINT8(byte, p_codec_info);               /* SEC codec configuration */

    /* Check Frequency */
    switch(byte & BTSEC_FREQ_MASK)
    {
    case BTSEC_FREQ_48K:
        p_sec_param->frequency = 48000;
        break;
    case BTSEC_FREQ_44K:
        p_sec_param->frequency = 44100;
        break;
    case BTSEC_FREQ_32K:
        p_sec_param->frequency = 32000;
        break;
    default:
        BTUSB_ERR("SEC Frequency=0x%x unsupported\n", byte & BTSEC_FREQ_MASK);
        return -1;
    }

    /* Check Mode */
    switch(byte & BTSEC_MODE_MASK)
    {
    case BTSEC_MODE_MONO:
        p_sec_param->mode = BTSEC_MODE_MONO;
        break;
    case BTSEC_MODE_STEREO:
        p_sec_param->mode = BTSEC_MODE_STEREO;
        break;
    default:
        BTUSB_ERR("SEC Mode=0x%x unsupported\n", byte & BTSEC_FREQ_MASK);
        return -1;
    }

    BTUSB_INFO("SEC Frequency=%d Mode=%s\n", p_sec_param->frequency,
            p_sec_param->mode == BTSEC_MODE_MONO?"Mono":"Stereo");

    return 0;
}

/*******************************************************************************
**
** Function         btusb_lite_av_sec_start
**
** Description      Start AV SBC Stream
**
** Returns          Status
**
*******************************************************************************/
static int btusb_lite_av_sec_start(struct btusb *p_dev, UINT8 scb_idx,
        tBTA_AV_AUDIO_CODEC_INFO*p_codec_cfg)
{
    struct btusb_lite_av_cb *p_av_cb = &p_dev->lite_cb.s.av;
    struct btusb_lite_encoder_ccb *p_encoder;
    struct btusb_lite_av_scb *p_av_scb;
    int nb_sec_frames;
    int rv;
    struct btusb_lite_av_sec_param sec_param;
    int av_header_len;

    /* Parse SBC Codec Info */
    if (btusb_lite_av_parse_sec_codec(&sec_param, p_codec_cfg->codec_info) < 0)
    {
        BTUSB_ERR("Bad SEC Codec parameters. Stream not started\n");
        return -1;
    }

    if (scb_idx >= BTA_AV_NUM_STRS)
    {
        BTUSB_ERR("Bad scb_idx=%d", scb_idx);
        return -1;
    }
    p_av_scb = &p_av_cb->scb[scb_idx];

    BTUSB_INFO("SEC Bitrate =%d\n", p_codec_cfg->bit_rate);

    p_av_cb->timestamp = 0; /* Reset TimeStamp */
    p_av_cb->option = BTUSB_LITE_AVDT_OPT_NO_MPH; /* No Media Payload Header */
    p_av_cb->m_pt = AVDT_RTP_PAYLOAD_TYPE | A2D_MEDIA_CT_VEND;
    p_av_cb->m_pt &= ~AVDT_MARKER_SET;

    /* Calculate Packet Header Size (HCI, L2CAP, RTP) */
    p_av_cb->header_len = BTUSB_LITE_HCI_ACL_HDR_SIZE +
                          BTUSB_LITE_L2CAP_HDR_SIZE +
                          BTUSB_LITE_RTP_SIZE;
    /* Calculate AV Header Size */
    av_header_len = BTUSB_LITE_L2CAP_HDR_SIZE +
                    BTUSB_LITE_RTP_SIZE;

    /* Clear the congestion flag: full stack made it congested when opening */
    p_av_scb->cong = FALSE;
    p_av_scb->started = TRUE;

    /* Get reference to AV's encoder */
    p_encoder = &p_av_cb->encoder;

    if (p_encoder->opened == 0)
    {
        /* Allocate a SEC Channel */
        rv = btsec_alloc();
        if (rv < 0)
        {
            BTUSB_ERR("btsec_alloc failed\n");
            return -1;
        }
        p_encoder->opened = 1;
        p_encoder->channel = rv;
        p_encoder->type = A2D_MEDIA_CT_SEC;

    }

    /* Configure the SEC Channel */
    rv = btsec_config(p_encoder->channel,
            sec_param.frequency,
            sec_param.mode,
            p_codec_cfg->bit_rate * 1000);
    if (rv <= 0)
    {
        BTUSB_ERR("btsec_config failed\n");
        btsec_free(p_encoder->channel);
        p_encoder->opened = 0;
        return -1;
    }

    /* Save the calculated SEC Frame size */
    p_encoder->encoded_frame_size = rv;
    BTUSB_INFO("encoded_frame_size=%d\n", rv);

    /* Configure the PCM Channel */
    rv = btpcm_config(p_dev->lite_cb.p_btpcm,
            p_dev,
            sec_param.frequency,
            sec_param.mode==BTSEC_MODE_MONO?1:2,
            16, /* SBC Encoder requires 16 bits per sample */
            btusb_lite_av_pcm_cback);
    if (rv < 0)
    {
        BTUSB_ERR("btpcm_config failed\n");
        btsec_free(p_encoder->channel);
        p_encoder->opened = 0;
        return -1;
    }

    /* SEC requires a fixed number of PCM Samples */
    p_encoder->pcm_frame_size = BTSEC_FRAME_SIZE;
    BTUSB_INFO("pcm_frame_size=%d\n", p_encoder->pcm_frame_size);



#if 0
    /* Calculate nb_sec_frames depending on MTU */
    nb_sec_frames = (p_av_cb->curr_mtu - av_header_len) / p_encoder->encoded_frame_size;
#else
    nb_sec_frames = 10;
    if(p_av_cb->stack_mtu < (p_encoder->encoded_frame_size * nb_sec_frames + av_header_len))
    {   /* if 10 SEC frames is bigger than mtu value, should use mtu */
        nb_sec_frames = (p_av_cb->stack_mtu - av_header_len) / p_encoder->encoded_frame_size;
        p_av_cb->curr_mtu = p_av_cb->stack_mtu;
    }
    else
    {
        p_av_cb->curr_mtu = p_encoder->encoded_frame_size * nb_sec_frames + av_header_len;
    }
#endif

    /* Calculate the size of the Payload */
    p_av_cb->payload_len = nb_sec_frames * p_encoder->encoded_frame_size;

    BTUSB_INFO("nb_sec_frames=%d payload_len=%d\n", nb_sec_frames, p_av_cb->payload_len);

    /* Start the PCM stream */
    rv = btpcm_start(p_dev->lite_cb.p_btpcm,
            p_encoder->pcm_frame_size, nb_sec_frames, 0);
    if (rv < 0)
    {
        BTUSB_ERR("btpcm_start failed\n");
        btsec_free(p_encoder->channel);
        p_encoder->opened = 0;
        return -1;
    }
    p_av_cb->pcm.state = PCM_STARTED;
    return 0;
}
#endif


/*******************************************************************************
**
** Function         btusb_lite_av_stop
**
** Description      Start AV
**
** Returns          None.
**
*******************************************************************************/
void btusb_lite_av_stop(struct btusb *p_dev, UINT8 scb_idx, UINT8 audio_open_cnt)
{
    struct btusb_lite_av_cb *p_av_cb = &p_dev->lite_cb.s.av;
    struct btusb_lite_av_scb *p_av_scb;
    int rv;

    BTUSB_INFO("scb_idx=%d audio_open_cnt=%d\n", scb_idx, audio_open_cnt);

    if (scb_idx >= BTA_AV_NUM_STRS)
    {
        BTUSB_ERR("Bad scb_idx=%d", scb_idx);
        return;
    }

    p_av_cb->audio_open_cnt = audio_open_cnt;

    p_av_scb = &p_av_cb->scb[scb_idx];

    p_av_cb->scb[scb_idx].started = FALSE;

    if (p_av_cb->pcm.state != PCM_STARTED)
    {
        BTUSB_ERR("BTPCM was not started\n");
        return;
    }

    /* Stop the PCM stream */
    rv = btpcm_stop(p_dev->lite_cb.p_btpcm);
    if (rv < 0)
    {
        BTUSB_ERR("btpcm_stop failed\n");
        return;
    }
    p_av_cb->pcm.state = PCM_CONFIGURED;
}

/*******************************************************************************
**
** Function         btusb_lite_av_suspend
**
** Description      Suspend AV
**
** Returns          none.
**
*******************************************************************************/
void btusb_lite_av_suspend(struct btusb *p_dev, UINT8 scb_idx, UINT8 audio_open_cnt)
{
    struct btusb_lite_av_cb *p_av_cb = &p_dev->lite_cb.s.av;
    struct btusb_lite_av_scb *p_av_scb;
    int rv;

    BTUSB_INFO("scb_idx=%d audio_open_cnt=%d\n", scb_idx, audio_open_cnt);

    if (scb_idx >= BTA_AV_NUM_STRS)
    {
        BTUSB_ERR("Bad scb_idx=%d", scb_idx);
        return;
    }

    p_av_cb->audio_open_cnt = audio_open_cnt;

    p_av_scb = &p_av_cb->scb[scb_idx];

    p_av_cb->scb[scb_idx].started = FALSE;

    if (p_av_cb->pcm.state != PCM_STARTED)
    {
        BTUSB_ERR("BTPCM was not started\n");
        return;
    }

    /* Stop the PCM stream */
    rv = btpcm_stop(p_dev->lite_cb.p_btpcm);
    if (rv < 0)
    {
        BTUSB_ERR("btpcm_stop failed\n");
        return;
    }
    p_av_cb->pcm.state = PCM_CONFIGURED;
}

/*******************************************************************************
**
** Function         btusb_lite_av_parse_sbc_codec
**
** Description      Parse an SBC A2DP Codec
**
** Returns          Status
**
*******************************************************************************/
static int btusb_lite_av_parse_sbc_codec(struct btusb_lite_av_sbc_param *p_sbc, UINT8 *p_codec)
{
    UINT8 byte;
    unsigned char codec_freq;
    unsigned char codec_blocks;
    unsigned char codec_subbands;
    unsigned char codec_mode;
    unsigned char codec_alloc;
    unsigned char bitpool_min;
    unsigned char bitpool_max;

    if (p_sbc == NULL)
    {
        BTUSB_ERR("p_sbc is NULL\n");
        return -1;
    }

    /* Extract LOSC */
    byte = *p_codec++;
    if (byte != CODEC_SBC_LOSC)
    {
        BTUSB_ERR("Bad SBC LOSC=%d", byte);
        return -1;
    }

    p_codec++; /* Ignore MT */

    /* Extract Codec Type */
    byte = *p_codec++;
    if (byte != A2D_MEDIA_CT_SBC)
    {
        BTUSB_ERR("Bad SBC codec type=%d", byte);
        return -1;
    }

    /* Extract Freq & Mode */
    byte = *p_codec++;
    codec_freq = byte & CODEC_SBC_FREQ_MASK;
    codec_mode = byte & CODEC_MODE_MASK;

    /* Extract NbBlock NbSubBand and Alloc Method */
    byte = *p_codec++;
    codec_blocks = byte & CODEC_SBC_BLOCK_MASK;
    codec_subbands = byte & CODEC_SBC_NBBAND_MASK;
    codec_alloc  = byte & CODEC_SBC_ALLOC_MASK;

    bitpool_min = *p_codec++;
    bitpool_max = *p_codec++;

    switch(codec_freq)
    {
    case CODEC_SBC_FREQ_48:
        BTUSB_INFO("SBC Freq=48K\n");
        p_sbc->frequency = 48000;
        break;
    case CODEC_SBC_FREQ_44:
        BTUSB_INFO("SBC Freq=44.1K\n");
        p_sbc->frequency = 44100;
        break;
    case CODEC_SBC_FREQ_32:
        BTUSB_INFO("SBC Freq=32K\n");
        p_sbc->frequency = 32000;
        break;
    case CODEC_SBC_FREQ_16:
        BTUSB_INFO("SBC Freq=16K\n");
        p_sbc->frequency = 16000;
        break;
    default:
        BTUSB_INFO("Bad SBC Freq=%d\n", codec_freq);
        return -1;
    }

    switch(codec_mode)
    {
    case CODEC_MODE_JOIN_STEREO:
        BTUSB_INFO("SBC Join Stereo\n");
        p_sbc->mode = CODEC_MODE_JOIN_STEREO;
        break;
    case CODEC_MODE_STEREO:
        BTUSB_INFO("SBC Stereo\n");
        p_sbc->mode = CODEC_MODE_STEREO;
        break;
    case CODEC_MODE_DUAL:
        BTUSB_INFO("SBC Dual\n");
        p_sbc->mode = CODEC_MODE_DUAL;
        break;
    case CODEC_MODE_MONO:
        BTUSB_INFO("SBC Mono\n");
        p_sbc->mode = CODEC_MODE_MONO;
        break;
    default:
        BTUSB_INFO("Bad SBC mode=%d\n", codec_mode);
        return -1;
    }

    switch(codec_blocks)
    {
    case CODEC_SBC_BLOCK_16:
        BTUSB_INFO("SBC Block=16\n");
        p_sbc->nb_blocks = 16;
        break;
    case CODEC_SBC_BLOCK_12:
        BTUSB_INFO("SBC Block=12\n");
        p_sbc->nb_blocks = 12;
        break;
    case CODEC_SBC_BLOCK_8:
        BTUSB_INFO("SBC Block=8\n");
        p_sbc->nb_blocks = 8;
        break;
    case CODEC_SBC_BLOCK_4:
        BTUSB_INFO("SBC Block=4\n");
        p_sbc->nb_blocks = 4;
        break;
    default:
        BTUSB_INFO("Bad SBC Block=%d\n", codec_blocks);
        return -1;
    }

    switch(codec_subbands)
    {
    case CODEC_SBC_NBBAND_8:
        BTUSB_INFO("SBC NbSubBand=8\n");
        p_sbc->nb_subbands = 8;
        break;
    case CODEC_SBC_NBBAND_4:
        BTUSB_INFO("SBC NbSubBand=4\n");
        p_sbc->nb_subbands = 4;
        break;
    default:
        BTUSB_INFO("Bad SBC NbSubBand=%d\n", codec_blocks);
        return -1;
    }

    switch(codec_alloc)
    {
    case CODEC_SBC_ALLOC_LOUDNESS:
        BTUSB_INFO("SBC Loudness\n");
        p_sbc->allocation = CODEC_SBC_ALLOC_LOUDNESS;
        break;
    case CODEC_SBC_ALLOC_SNR:
        BTUSB_INFO("SBC SNR\n");
        p_sbc->allocation = CODEC_SBC_ALLOC_SNR;
        break;
    default:
        BTUSB_INFO("Bad SBC AllocMethod=%d\n", codec_blocks);
        return -1;
    }

    BTUSB_INFO("BitpoolMin=%d BitpoolMax=%d\n", bitpool_min, bitpool_max);

    p_sbc->bitpool_min = bitpool_min;
    p_sbc->bitpool_max = bitpool_max;

    return 0;
}

/*******************************************************************************
 **
 ** Function        btusb_lite_sbc_get_bitpool
 **
 ** Description     Calculate the BitPool for a specified BitRate (and other parameters)
 **
 ** Returns         Void
 **
 *******************************************************************************/
static int btusb_lite_sbc_get_bitpool(struct btusb_lite_av_sbc_param *p_sbc_param, int target_bitrate)
{
    int nb_channels;
    int frame_length;
    int bitpool = p_sbc_param->bitpool_max + 1;
    int bitrate;

    /* Required number of channels */
    if (p_sbc_param->mode == CODEC_MODE_MONO)
        nb_channels = 1;
    else
        nb_channels = 2;

    target_bitrate *= 1000; /* Bitrate from app is in Kbps */

    do
    {
        bitpool--;  /* Reduce Bit Pool by one */

        /* Calculate common SBC Frame length */
        frame_length = 4 + (4 * p_sbc_param->nb_subbands * nb_channels) / 8;

        /* Add specific SBC Frame length (depending on mode) */
        switch(p_sbc_param->mode)
        {
        case CODEC_MODE_MONO:
        case CODEC_MODE_DUAL:
            frame_length += (p_sbc_param->nb_blocks * nb_channels * bitpool) / 8;
            break;
        case CODEC_MODE_JOIN_STEREO:
            frame_length += (p_sbc_param->nb_subbands + p_sbc_param->nb_blocks * bitpool) / 8;
            break;
        case CODEC_MODE_STEREO:
            frame_length += (p_sbc_param->nb_blocks * bitpool) / 8;
            break;
        }

        /* Calculate bit rate */
        bitrate = 8 * frame_length * p_sbc_param->frequency / p_sbc_param->nb_subbands / p_sbc_param->nb_blocks;

    } while (bitrate > target_bitrate); /* While bitrate is too big */

    BTUSB_INFO("final bitpool=%d frame_length=%d bitrate=%d\n", bitpool, frame_length, bitrate);

    return (int)bitpool;
}

/*******************************************************************************
 **
 ** Function         btusb_lite_av_pcm_cback
 **
 ** Description      BTUSB Lte AV PCM Callback function.
 **
 ** Returns          Void
 **
 *******************************************************************************/
static void btusb_lite_av_pcm_cback(void *p_opaque, void *p_data, int nb_pcm_frames)
{
    struct btusb *p_dev = p_opaque;
    struct btusb_lite_av_cb *p_av_cb;
    int pcm_frame_size_byte;
    void *p_dest;
    int written_enc_size;
    struct btusb_lite_encoder_ccb *p_encoder;

    if (!p_dev)
    {
        BTUSB_ERR("Null p_dev\n");
        return;
    }

    if (!p_data)
    {
        BTUSB_ERR("Null p_data\n");
        return;
    }

    /* Get Reference on the SBC Stream (which is the same than the Encoder channel) */
    p_av_cb = &p_dev->lite_cb.s.av;

    if (p_av_cb->pcm.state != PCM_STARTED)
    {
        BTUSB_ERR("BTPCM is not started\n");
        btpcm_stop(p_dev->lite_cb.p_btpcm);
        return;
    }

    /* Get reference to AV's encoder */
    p_encoder = &p_av_cb->encoder;

    BTUSB_DBG("nb_pcm_frames=%d, pcm_frame_size=%d, encoded_frame_size=%d \n",
        nb_pcm_frames, p_av_cb->encoder.pcm_frame_size, p_av_cb->encoder.encoded_frame_size);

    /* Calculate the size (in byte) of an Input PCM buffer (holding one encoded frame) */
    pcm_frame_size_byte = p_av_cb->encoder.pcm_frame_size;
    pcm_frame_size_byte *= 2; /* Stereo */
    pcm_frame_size_byte *= 2; /* 16 bits per sample */

    /* Sanity Check */
    if (pcm_frame_size_byte == 0)
    {
        BTUSB_ERR("Bad PCM Frame size=%d\n", pcm_frame_size_byte);
        return;
    }

    /*
     * No PCM data in a timer period.
     * Need to send AV data in a working buffer if it exists
     */
    if (nb_pcm_frames == 0)
    {
        BTUSB_DBG("No PCM data, send AV data in a working buffer. \n");
        if (p_av_cb->p_buf_working)
        {
            if (p_av_cb->p_buf_working->len)
            {
                /*  For AV channel, send the packet */
                btusb_lite_av_send_packet((struct btusb *)p_dev, p_av_cb->p_buf_working);

                /* A new working buffer must be allocated */
                p_av_cb->p_buf_working = NULL;
            }
        }
        return;
    }

    /* While received buffer is not empty */
    while (nb_pcm_frames)
    {
        /* Check if there are enough remaining frames in the buffer */
        if ((nb_pcm_frames * 2 * 2) < pcm_frame_size_byte)
        {
            BTUSB_ERR("Bad nb_pcm_frames=%d\n", nb_pcm_frames);
            return;
        }

        /* If no working buffer allocated */
        if (!p_av_cb->p_buf_working)
        {
            /* Get a buffer from the pool */
            p_av_cb->p_buf_working = (BT_HDR *)GKI_getbuf(sizeof(BT_HDR) + p_av_cb->header_len + p_av_cb->payload_len);
            if(unlikely(p_av_cb->p_buf_working == NULL))
            {
                BTUSB_ERR("Unable to get GKI buffer - sent fail\n");
                return;
            }

           if (unlikely(dbgflags & BTUSB_GKI_CHK_MSG) &&
                unlikely(GKI_buffer_status(p_av_cb->p_buf_working) != BUF_STATUS_UNLINKED))
            {
                BTUSB_ERR("buffer != BUF_STATUS_UNLINKED 0x%p\n", p_av_cb->p_buf_working);
                return;
            }

            /* Skip headers */
            p_av_cb->p_buf_working->offset = p_av_cb->header_len;
            p_av_cb->p_buf_working->len = 0;
            p_av_cb->p_buf_working->layer_specific = 0; /* Used to store the number of Encoded Frames */
        }

        /* Fill the ACL Packet with SBC Frames */
        do
        {
            /* Get Write address */
            p_dest = (UINT8 *)(p_av_cb->p_buf_working + 1) + p_av_cb->p_buf_working->offset +
                    p_av_cb->p_buf_working->len;

            if (p_encoder->type == A2D_MEDIA_CT_SBC)
            {
                /* Encode one PCM frame with SBC Encoder*/
                btsbc_encode(p_encoder->channel,
                        /* If Mute => Zero filled PCM sample*/
                        /* Otherwise => regular PCM data */
                        pcm0_mute?btusb_lite_silence_pcm:p_data,
                        pcm_frame_size_byte,
                        p_dest,                 /* SBC Output buffer */
                        p_av_cb->encoder.encoded_frame_size, /* Expected Output SBC frame size */
                        &written_enc_size);
            }
#ifdef BTUSB_LITE_SEC
            else if (p_encoder->type == A2D_MEDIA_CT_SEC)
            {
                /* Encode one PCM frame with SEC Encoder*/
                written_enc_size = btsec_encode(p_av_cb->encoder.channel,
                        /* If Mute => Zero filled PCM sample*/
                        /* Otherwise => regular PCM data */
                        pcm0_mute?btusb_lite_silence_pcm:p_data,
                        pcm_frame_size_byte,
                        p_dest,                 /* SEC Output buffer */
                        p_encoder->encoded_frame_size);/* Expected Output SEC frame size */
            }
#endif
            else
            {
                BTUSB_ERR("Bad Encoding TYPE:%d\n", p_encoder->type);
            }

            if (written_enc_size != p_av_cb->encoder.encoded_frame_size)
            {
                BTUSB_ERR("Bad Encoded Frame length=%d (expected=%d)\n",
                        written_enc_size, p_av_cb->encoder.encoded_frame_size);
            }

            /* Update Encoded packet length */
            p_av_cb->p_buf_working->len += (UINT16)p_av_cb->encoder.encoded_frame_size;

            /* One more Encoded Frame */
            p_av_cb->p_buf_working->layer_specific++;

            p_data += pcm_frame_size_byte; /* Jump to the next PCM sample */
            nb_pcm_frames -= pcm_frame_size_byte / 4; /* Update number of remaining samples */

        } while (nb_pcm_frames &&
                 (p_av_cb->p_buf_working->layer_specific < A2D_SBC_HDR_NUM_MSK) &&
                 ((p_av_cb->p_buf_working->len + p_av_cb->encoder.encoded_frame_size + BTUSB_LITE_AVDT_HDR_SIZE)
                  < p_av_cb->curr_mtu));

        /* A2DP streaming */
        if (p_av_cb->streaming_type == AV_LITE_STREAMING_AVDT)
        {
            /* If no more room to store an encoded frame */
            if (p_av_cb->encoder.encoded_frame_size > (p_av_cb->curr_mtu - p_av_cb->p_buf_working->len - BTUSB_LITE_AVDT_HDR_SIZE))
            {
                BTUSB_DBG("nb_pcm_fr=%d nb_sbc_fr=%d len=%d sbc_fr_size=%d mtu=%d nb_seq=%d\n",
                        nb_pcm_frames, p_av_cb->p_buf_working->layer_specific,
                        p_av_cb->p_buf_working->len, p_av_cb->encoder.encoded_frame_size,
                        p_av_cb->curr_mtu, p_av_cb->seq_number);
                /* For AV channel, send the packet */
                btusb_lite_av_send_packet((struct btusb *)p_dev, p_av_cb->p_buf_working);

                /* A new working buffer must be allocated */
                p_av_cb->p_buf_working = NULL;
            }
        }

        /* CLB streaming */
        else if (p_av_cb->streaming_type == AV_LITE_STREAMING_CLB)
        {
            BTUSB_DBG("nb_pcm_fr=%d nb_sbc_fr=%d len=%d sbc_fr_size=%d mtu=%d nb_seq=%d\n",
                    nb_pcm_frames, p_av_cb->p_buf_working->layer_specific,
                    p_av_cb->p_buf_working->len, p_av_cb->encoder.encoded_frame_size,
                    p_av_cb->curr_mtu, p_av_cb->seq_number);

            /* If at least one SBC frame encoded */
            if (p_av_cb->p_buf_working->layer_specific)
            {
                /* For AV channel, send the packet */
                btusb_lite_av_send_packet((struct btusb *)p_dev, p_av_cb->p_buf_working);

                /* A new working buffer must be allocated */
                p_av_cb->p_buf_working = NULL;
            }
            /* Increase sequence number (even if no packet sent) */
            p_av_cb->seq_number++;
        }
        else
        {
            BTUSB_ERR("Bad Streaming Type:%d \n", p_av_cb->streaming_type);
        }
    }
}

/*******************************************************************************
 **
 ** Function        btusb_lite_clb_send
 **
 ** Description     Send a new Connectionless BAV packet to the controller
 **
 ** Returns         Void
 **
 *******************************************************************************/
static void btusb_lite_clb_send(struct btusb *p_dev, BT_HDR *p_msg, UINT8 avdt_handle, UINT8 m_pt, UINT8 option, UINT32 timestamp)
{
    struct btusb_lite_av_cb *p_av_cb = &p_dev->lite_cb.s.av;
    UINT8 *p_data;

    if (p_dev == NULL)
    {
        BTUSB_ERR("p_dev is NULL\n");
        if (p_msg)
            GKI_freebuf(p_msg);
        return;
    }

    if (p_msg == NULL)
    {
        BTUSB_ERR("p_msg is NULL\n");
        return;
    }

    if (p_msg->offset < (BSA_SV_AV_BAV_HEADER_SIZE + BSA_BAV_L2C_HDR_SZ)) /*14+4*/
    {
        BTUSB_ERR("bad offset, p_msg->offset=%d handle=%d", p_msg->offset, avdt_handle);
        GKI_freebuf(p_msg);
        return;
    }

    /* Packet Header - BSA_SV_AV_BAV_HEADER_SIZE - 14 bytes */
    p_msg->offset -= BSA_SV_AV_BAV_HEADER_SIZE;
    p_msg->len += BSA_SV_AV_BAV_HEADER_SIZE;
    p_data = (UINT8 *)(p_msg + 1) + p_msg->offset;

    UINT8_TO_STREAM(p_data, BSA_SV_AV_BAV_PACKET_HEADER_SBC_MASK);

    UINT8_TO_BE_STREAM(p_data, AVDT_MEDIA_OCTET1); /* Version, Padding, Ext, CSRC */
    UINT8_TO_BE_STREAM(p_data, BSA_SV_AV_BAV_RTP_PAYLOAD_TYPE);
    UINT16_TO_BE_STREAM(p_data, p_av_cb->seq_number);
    UINT32_TO_BE_STREAM(p_data, 0); /* todo: TimeStamp */
    UINT32_TO_BE_STREAM(p_data, 0); /* todo: SSRC */
    /* Number of SBC Frames */
    UINT8_TO_BE_STREAM(p_data, p_msg->layer_specific & A2D_SBC_HDR_NUM_MSK);

    /* Packet Header, BSA_BAV_L2C_HDR_SZ = 4 */
    p_msg->offset -= BSA_BAV_L2C_HDR_SZ;
    p_msg->len += BSA_BAV_L2C_HDR_SZ;
    p_data = (UINT8 *)(p_msg + 1) + p_msg->offset;

    UINT16_TO_STREAM(p_data, p_msg->len - BSA_BAV_L2C_HDR_SZ);
    UINT16_TO_STREAM(p_data, BSA_BAV_L2C_FAKE_CID);

    BTUSB_DBG("len=%d, offset=%d, nb_frame=%d, seq_number=%d \n",
        p_msg->len, p_msg->offset,  p_msg->layer_specific, p_av_cb->seq_number);
    BTUSB_DBG("p_msg->len should be 678 - AFBT compatible");

    if (btusb_lite_hci_acl_send(p_dev, p_msg, 1 /*LTAddr*/) < 0)
    {
        BTUSB_ERR("Fail-btusb_lite_hci_acl_send() ");
        return;
    }
}

/*******************************************************************************
 **
 ** Function        btusb_lite_av_send_packet
 **
 ** Description     Send a new BAV packet to the controller
 **
 ** Returns         Void
 **
 *******************************************************************************/
static void btusb_lite_av_send_packet(struct btusb *p_dev, BT_HDR *p_msg)
{
    struct btusb_lite_av_cb *p_av_cb;
    struct btusb_lite_av_scb *p_av_scb;
    int stream;
    int nb_started_streams;
    BT_HDR *p_msg_dup;

    if (!p_dev || !p_msg)
    {
        BTUSB_ERR("Bad reference p_dev=%p p_msg=%p\n", p_dev, p_msg);
        return;
    }

    /* Sanity */
    if (p_msg->len == 0)
    {
        BTUSB_ERR("Length is 0=%d\n", p_msg->len);
        GKI_freebuf(p_msg); /* Free this ACL buffer */
        return;
    }

    /* Get Reference on the AV Streams */
    p_av_cb = &p_dev->lite_cb.s.av;

    /* Update TimeStamp */
    p_av_cb->timestamp += p_msg->layer_specific * p_av_cb->encoder.pcm_frame_size;

    nb_started_streams = 0;
    /* Count how many AV stream are started */
    for (stream = 0, p_av_scb = p_av_cb->scb ; stream < BTA_AV_NUM_STRS ; stream++, p_av_scb++)
    {
        if (p_av_scb->started)
        {
            nb_started_streams++;   /* One more started stream */
        }
    }

    if (nb_started_streams == 0)
    {
        BTUSB_ERR("No Started AV stream found\n");
        GKI_freebuf(p_msg); /* Free this ACL buffer */
        return;
    }
    else if (nb_started_streams == 1)
    {
        p_msg_dup = NULL;
    }
    else
    {
        /*
         * Duplicate the AV packet
         */
        /* Get a buffer from the pool */
        p_msg_dup = (BT_HDR *)GKI_getbuf(sizeof(BT_HDR) + p_msg->offset + p_msg->offset);
        if(p_msg_dup)
        {
            if (unlikely(dbgflags & BTUSB_GKI_CHK_MSG) &&
                unlikely(GKI_buffer_status(p_msg_dup) != BUF_STATUS_UNLINKED))
            {
                BTUSB_ERR("buffer != BUF_STATUS_UNLINKED 0x%p\n", p_msg_dup);
                p_msg_dup = NULL; /* Do not use this buffer */
            }
            if(p_msg_dup)
            {
                /* Duplicate all the data (Header, and payload */
                memcpy(p_msg_dup, p_msg, sizeof(BT_HDR) + p_msg->offset + p_msg->offset);
            }
        }
        if (nb_started_streams > 2)
        {
            BTUSB_ERR("nb_started_streams=%d force it to 2\n", nb_started_streams);
            nb_started_streams = 2;
        }
    }

    /* For every AV stream Started */
    for (stream = 0, p_av_scb = p_av_cb->scb ; stream < BTA_AV_NUM_STRS ; stream++, p_av_scb++)
    {
        if (p_av_scb->started)
        {
            if (p_msg)
            {
                /* Send the original packet to AVDT/CLB */
                p_av_cb->p_stream_func(p_dev, p_msg, p_av_scb->avdt_handle,
                        p_av_cb->m_pt, p_av_cb->option, p_av_cb->timestamp);
                p_msg = NULL;
            }
            else if (p_msg_dup)
            {
                /* Send the duplicated packet to AVDT/CLB */
                p_av_cb->p_stream_func(p_dev, p_msg_dup, p_av_scb->avdt_handle,
                        p_av_cb->m_pt, p_av_cb->option, p_av_cb->timestamp);
                p_msg_dup = NULL;
            }
            else
            {
                BTUSB_ERR("No AV data to send for AV stream=%d \n", stream);
            }
        }
    }
}

