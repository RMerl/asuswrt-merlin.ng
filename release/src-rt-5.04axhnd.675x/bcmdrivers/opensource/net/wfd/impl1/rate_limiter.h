/*
<:copyright-BRCM:2018:DUAL/GPL:standard 

   Copyright (c) 2018 Broadcom 
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
#ifndef __RATE_LIMITER_H__
#define __RATE_LIMITER_H__

#define RL_DIR_TX 0
#define RL_DIR_RX 1

void rate_limiter_init(void);
uint32_t rl_should_drop(uint32_t wfd_id, uint32_t if_id, int dir,  uint32_t size);
uint32_t rl_chain_check_and_drop(uint32_t wfd_id, uint32_t if_id, int dir, struct sk_buff *skb);

#endif

