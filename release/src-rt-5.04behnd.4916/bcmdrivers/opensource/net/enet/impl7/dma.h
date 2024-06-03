/*
   <:copyright-BRCM:2016:DUAL/GPL:standard

      Copyright (c) 2016 Broadcom
      All Rights Reserved

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


/*
 * DMA API
 */

#ifndef __DMA_H__
#define __DMA_H__

extern int dma_tx(enetx_port_t *port, pNBuff_t pNBuff, int channel);
extern int dma_rx(int rx_channel, unsigned char **pBuf, int *len);
extern int dma_init(enetx_port_t *port);
extern void dma_uninit(enetx_port_t *port);
extern int is_rx_empty(int rx_channel);
extern void fkb_databuf_recycle(FkBuff_t *fkb, void *context);
extern void enet_dma_recycle(pNBuff_t pNBuff, uint32_t context, uint32_t flags);

#endif
