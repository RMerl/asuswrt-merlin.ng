/*
<:copyright-BRCM:2020:DUAL/GPL:standard 

   Copyright (c) 2020 Broadcom 
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

