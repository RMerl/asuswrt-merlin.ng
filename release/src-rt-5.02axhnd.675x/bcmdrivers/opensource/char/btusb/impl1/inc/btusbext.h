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
 
#ifndef BTUSBEXT_H
#define BTUSBEXT_H

#include <linux/time.h>

/* BTUSB Statistics structure */
typedef struct
{
    /* All URB submit counter */
    unsigned long urb_submit_ok;
    unsigned long urb_submit_err;

    /* All URB out complete */
    unsigned long urb_out_complete;
    unsigned long urb_out_complete_err;

    /* CMD counters */
    unsigned long cmd_submit_ok;
    unsigned long cmd_submit_err;
    unsigned long cmd_complete;
    unsigned long cmd_complete_err;

    /* ACL RX counters */
    unsigned long acl_rx_submit_ok;
    unsigned long acl_rx_submit_err;
    unsigned long acl_rx_complete;
    unsigned long acl_rx_complete_err;
    unsigned long acl_rx_resubmit;
    unsigned long acl_rx_bytes;

    /* DIAG RX counters */
    unsigned long diag_rx_submit_ok;
    unsigned long diag_rx_submit_err;
    unsigned long diag_rx_complete;
    unsigned long diag_rx_complete_err;
    unsigned long diag_rx_resubmit;
    unsigned long diag_rx_bytes;

    /* EVENT counters */
    unsigned long event_submit_ok;
    unsigned long event_submit_err;
    unsigned long event_complete;
    unsigned long event_complete_err;
    unsigned long event_resubmit;
    unsigned long event_bytes;

    /* VOICE RX counters */
    unsigned long voicerx_submit_ok;
    unsigned long voicerx_submit_err;
    unsigned long voicerx_complete;
    unsigned long voicerx_complete_err;
    unsigned long voicerx_bad_frames;
    unsigned long voicerx_raw_bytes;
    unsigned long voicerx_skipped_bytes;
    unsigned long voicerx_split_hdr;
    unsigned long voicerx_bad_hdr;
    unsigned long voicerx_bad_size;

    /* VOICE TX counters */
    unsigned long voicetx_submit_ok;
    unsigned long voicetx_submit_err;
    unsigned long voicetx_nobuf;
    unsigned long voicetx_complete;
    unsigned long voicetx_complete_err;

    /* voice timings */
    unsigned long voice_max_tx_done_delta_time;
    unsigned long voice_min_tx_done_delta_time;
    struct timeval voice_tx_done_delta_time;
    struct timeval voice_last_tx_done_ts;

    /* max delta time between 2 consecutive tx done routine in us */
    unsigned long voice_max_rx_rdy_delta_time;
    unsigned long voice_min_rx_rdy_delta_time;
    struct timeval voice_rx_rdy_delta_time;
    struct timeval voice_last_rx_rdy_ts;

    /* max delta time between 2 consecutive tx done routine in us */
    unsigned long voice_max_rx_feeding_interval;
    unsigned long voice_min_rx_feeding_interval;
    struct timeval voice_rx_feeding_interval;
    struct timeval voice_last_rx_feeding_ts;
} tBTUSB_STATS;

/* IOCTL definitions (shared among all user mode applications, do not modify) */
#define IOCTL_BTWUSB_GET_STATS            0x1001
#define IOCTL_BTWUSB_CLEAR_STATS          0x1002
#define IOCTL_BTWUSB_PUT                  0x1003
#define IOCTL_BTWUSB_PUT_CMD              0x1004
#define IOCTL_BTWUSB_GET                  0x1005
#define IOCTL_BTWUSB_GET_EVENT            0x1006
#define IOCTL_BTWUSB_PUT_VOICE            0x1007
#define IOCTL_BTWUSB_GET_VOICE            0x1008
#define IOCTL_BTWUSB_ADD_VOICE_CHANNEL    0x1009
#define IOCTL_BTWUSB_REMOVE_VOICE_CHANNEL 0x100a
#define IOCTL_BTWUSB_DEV_RESET            0x100b

struct btusb_ioctl_sco_info
{
    unsigned short handle;
    unsigned char burst;
};

#endif
