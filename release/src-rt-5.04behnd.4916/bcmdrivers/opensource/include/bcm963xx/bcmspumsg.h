/*
    Copyright 2020 Broadcom Corporation

    <:label-BRCM:2020:DUAL/GPL:standard

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

/***************************************************************************
 * File Name  : bcmspumsg.h
 *
 * Description: This file contains the definitions, structures for the SPU
 *              message used between SPU and its DMA engine.
 ***************************************************************************/

#if !defined(_BCMSPUMSG_H_)
#define _BCMSPUMSG_H_

#include <linux/blog.h>

enum bcmspu_message_type {
  BRCM_MESSAGE_SINGLE = 0,
  BRCM_MESSAGE_BATCH,
  BRCM_MESSAGE_BATCH_PREP_ONLY,
  BRCM_MESSAGE_BATCH_TOGGLE_ONLY,
  BRCM_MESSAGE_MAX,
};

struct bcmspu_test_info {
  uint16_t chan;
  uint16_t vec_id;
  uint8_t pattern;
  dma_addr_t src_dma;
  dma_addr_t dst_dma;
};

struct bcmspu_message {
  struct list_head list;
  struct scatterlist *src;
  struct scatterlist *dst;
  int error;
  void (*rx_callback)(struct bcmspu_message *msg);
  enum bcmspu_message_type type;
  void *desc_ptr;
  void *next_ptr;
  uint16_t batch_count;
  void *ctx;
  struct bcmspu_test_info *test_info;
};

struct bcmspu_offload_resp {
  uint32_t session_buff_info;
  void *bufp;
  void *data;
  uint32_t data_len;
  uint32_t digest_len;
  void *dest_digest;
};

typedef int (*spu_offload_cbk)(struct bcmspu_offload_resp *resp_info);

/* will support 32 US and 32 DS sesssion */

#define MAX_SPU_OFFLOAD_SESSION_LOG2  5
#define MAX_SPU_OFFLOAD_US_SESSIONS   (1 << MAX_SPU_OFFLOAD_SESSION_LOG2)
#define MAX_SPU_OFFLOAD_DS_SESSIONS   (1 << MAX_SPU_OFFLOAD_SESSION_LOG2)

#define MAX_SPU_OFFLOAD_SESSIONS   (MAX_SPU_OFFLOAD_US_SESSIONS + MAX_SPU_OFFLOAD_DS_SESSIONS)

#define MAX_SPU_HEADER_SIZE        200
#define OUTER_HEADER_SIZE          40

struct bcmspu_offload_parm {
   uint32_t esp_spi;
   uint32_t seq_lo;
   uint32_t seq_hi;

   union {
      struct {
         uint8_t is_enc      : 1;
         uint8_t is_esn      : 1;
         uint8_t esp_o_udp   : 1;
         uint8_t data_limit  : 1;
         uint8_t long_bitmap : 1;
         uint8_t reserved    : 3;
      };
      uint8_t u8_0;
   };

   uint8_t session_id;
   union {
      uint8_t fixed_hdr_size;
      uint8_t outer_hdr_size;
   };
   uint8_t key_size;
   uint8_t digest_size;
   uint8_t iv_size;
   uint8_t blog_chan_id;

   uint64_t pkts;
   uint64_t bytes;
   uint8_t spu_header[MAX_SPU_HEADER_SIZE];
   uint8_t outer_header[OUTER_HEADER_SIZE];
};

struct spu_offload_parm_args {
   uint32_t session_id;
#if defined(CONFIG_BLOG)
   Blog_t *blog_p;
#endif
   struct bcmspu_offload_parm *parm;
};

struct spu_offload_tracker {
    uint64_t lft_bytes;
    uint64_t lft_pkts;
    uint32_t replay_window;
    uint32_t replay;
    uint32_t seq_lo;
    uint32_t seq_hi;
};

struct spu_offload_stats_args {
    uint32_t session_id;
    struct spu_offload_tracker *stats;
};


#if defined(CONFIG_BCM_SPU_HW_OFFLOAD)
typedef int (*spu_offload_parm_update)(struct spu_offload_parm_args *arg);

struct bcmspu_offload_setup {
   int enable;
   spu_offload_parm_update parm_func;
};
#endif

#endif /* _BCMSPUMSG_H_ */


