/*
* <:copyright-BRCM:2022:DUAL/GPL:standard
* 
*    Copyright (c) 2022 Broadcom 
*    All Rights Reserved
* 
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
* 
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
* 
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
* 
* :> 
*/

#include <linux/module.h>

#if defined(CONFIG_BRCM_SMC_BOOT)
int biu_ch_freq_get(unsigned int ch, unsigned int *freq)
{
    return 0;
}
EXPORT_SYMBOL(biu_ch_freq_get);

int ubus_decode_pcie_wnd_cfg(u32 base, u32 size, u32 core)
{
    /* Not supported */
    return -1;
}
EXPORT_SYMBOL(ubus_decode_pcie_wnd_cfg);
#endif
