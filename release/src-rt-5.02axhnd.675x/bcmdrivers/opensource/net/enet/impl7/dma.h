/*
   <:copyright-BRCM:2016:DUAL/GPL:standard
   
      Copyright (c) 2016 Broadcom 
      All Rights Reserved
   
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
