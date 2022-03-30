/*
    Copyright 2020 Broadcom Corporation

    <:label-BRCM:2020:DUAL/GPL:standard
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
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


