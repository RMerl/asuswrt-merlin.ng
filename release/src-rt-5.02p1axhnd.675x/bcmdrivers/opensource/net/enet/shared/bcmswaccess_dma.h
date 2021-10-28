/*
<:copyright-BRCM:2002:DUAL/GPL:standard

   Copyright (c) 2002 Broadcom 
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
#ifndef _BCMSWACCESS_DMA_H_
#define _BCMSWACCESS_DMA_H_

void bcmsw_spi_rreg(int bus_num, int spi_ss, int chip_id, int page, int reg, uint8 *data, int len);
void bcmsw_spi_wreg(int bus_num, int spi_ss, int chip_id, int page, int reg, uint8 *data, int len);
int bcmeapi_ioctl_ethsw_spiaccess(int bus_num, int spi_id, int chip_id, struct ethswctl_data *e); 

#endif /* _BCMSWACCESS_DMA_H_ */

