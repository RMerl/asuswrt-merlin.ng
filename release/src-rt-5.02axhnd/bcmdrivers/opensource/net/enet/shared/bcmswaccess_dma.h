/*
<:copyright-BRCM:2002:DUAL/GPL:standard

   Copyright (c) 2002 Broadcom 
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
#ifndef _BCMSWACCESS_DMA_H_
#define _BCMSWACCESS_DMA_H_

void bcmsw_spi_rreg(int bus_num, int spi_ss, int chip_id, int page, int reg, uint8 *data, int len);
void bcmsw_spi_wreg(int bus_num, int spi_ss, int chip_id, int page, int reg, uint8 *data, int len);
int bcmeapi_ioctl_ethsw_spiaccess(int bus_num, int spi_id, int chip_id, struct ethswctl_data *e); 

#endif /* _BCMSWACCESS_DMA_H_ */

