#ifndef __DPIMGMT_H_INCLUDED__
#define __DPIMGMT_H_INCLUDED__
/*
<:copyright-BRCM:2014:DUAL/GPL:standard

   Copyright (c) 2014 Broadcom 
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
#include <bcmdpi.h>
#include <linux/dpi_ctk.h>

#define DPI_PARENTAL_MAX        32
#define DPI_INVALID_IDX DPI_PARENTAL_MAX

#define DPI_CONFIG_ADD          0
#define DPI_CONFIG_DEL          1

int dm_config_option(unsigned short option, unsigned short value);
int dm_config_parental(int action, DpictlParentalConfig_t *cfg);
int dm_construct(void);

uint32_t dm_lookup(struct sk_buff *skb);

#endif /* __DPIMGMT_H_INCLUDED__ */
