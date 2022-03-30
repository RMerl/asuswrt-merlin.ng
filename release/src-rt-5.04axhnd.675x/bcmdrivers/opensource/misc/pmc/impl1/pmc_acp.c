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
#include "BPCM.h"

int ubus_cfg_entry_set(int pmb_addr, uint8_t entry_id,
		uint32_t addr_in, uint32_t addr_out, uint8_t dst_pid,
		uint8_t size_shift, uint8_t enable)
{
	BPCM_UBUS_CFG_REG ubus_cfg;
	int ret;

	if (pmb_addr == 0)
		return -1;

	ret = ReadBPCMRegister(pmb_addr,
			UBUSBPCMRegOffset(cfg[entry_id]),
			&ubus_cfg.Regs32.word0);
	if (ret)
		return ret;

	ret = ReadBPCMRegister(pmb_addr,
			UBUSBPCMRegOffset(cfg[entry_id]) + 1,
			&ubus_cfg.Regs32.word1);
	if (ret)
		return ret;

	ubus_cfg.Bits.addr_in = addr_in >> 8;
	ubus_cfg.Bits.addr_out = addr_out >> 8;
	ubus_cfg.Bits.pid = dst_pid;
	ubus_cfg.Bits.size = size_shift;
	ubus_cfg.Bits.cmddta = 0;
	ubus_cfg.Bits.en = enable;

	ret = WriteBPCMRegister(pmb_addr,
			UBUSBPCMRegOffset(cfg[entry_id]),
			ubus_cfg.Regs32.word0);
	if (ret)
		return ret;

	ret = WriteBPCMRegister(pmb_addr,
			UBUSBPCMRegOffset(cfg[entry_id]) + 1,
			ubus_cfg.Regs32.word1);
	return ret;
}

