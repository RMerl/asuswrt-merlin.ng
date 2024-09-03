/*
<:copyright-BRCM:2020:DUAL/GPL:standard

   Copyright (c) 2020 Broadcom 
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
#include "pmc_drv.h"
#include "pmc_cpu.h"
#include "BPCM.h"

int pmc_cpu_init(unsigned int cpu_id)
{
	BPCM_PWR_ZONE_N_STATUS sts;
	int rc;

	rc = ReadBPCMRegister(cpu_pmb[cpu_id], BPCMZoneStsRegOffset(0), &sts.Reg32);
	if (rc == 0 && sts.Bits.reset_state) 
		PowerOnZone(cpu_pmb[cpu_id], 0);

    return rc;
}

int pmc_cpu_shutdown(unsigned int cpu_id)
{
	return PowerOffZone(cpu_pmb[cpu_id], 0); // XXX repower flag ignored
}

