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
  uint16_t batch_count;
  void *ctx;
  struct bcmspu_test_info *test_info;
};

#endif /* _BCMSPUMSG_H_ */


