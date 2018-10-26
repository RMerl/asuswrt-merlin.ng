/*
<:copyright-BRCM:2015:GPL/GPL:spu

   Copyright (c) 2015 Broadcom 
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

#ifndef _PDC_DEBUG_H
#define _PDC_DEBUG_H

#include "pdc.h"

void pdc_print_buf(const char *prefix_str, uint8_t *data, uint16_t len);
void pdc_setup_debugfs(struct platform_device *pdev);
void pdc_free_debugfs(struct platform_device *pdev);
void pdc_free_debugfs_stats(struct platform_device *pdev);
#endif
