/*
<:copyright-BRCM:2022:DUAL/GPL:standard

   Copyright (c) 2022 Broadcom 
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
/****************************************************************************
 * Clock RPC Service Driver
 *
 * Author: Dima Mamut <dima.mamut@broadcom.com>
*****************************************************************************/
#include "pmc_drv.h"
#include "clk_rst.h"
#include "pmc_sdhci.h"
#include "board.h"
#include <asm/div64.h>

int pmc_sdhci_set_base_clk( uint64_t freq, struct device_node *node )
{
   return 0;
}
EXPORT_SYMBOL(pmc_sdhci_set_base_clk);

