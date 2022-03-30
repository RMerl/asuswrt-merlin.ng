/*
<:copyright-BRCM:2013:DUAL/GPL:standard 

   Copyright (c) 2013 Broadcom 
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

#ifndef PMC_DSL_H
#define PMC_DSL_H

#include "pmc_ver.h"

#define CH01_CFG	0
#define CH23_CFG	1
#define CH45_CFG	2

typedef struct {
    uint32_t Reg;
    uint16_t mdiv0;
    uint16_t mdiv1;
    uint8_t  mdiv_override0; 
    uint8_t  mdiv_override1;
    uint8_t  mdel0;
    uint8_t  mdel1;
}pll_ch_cfg_t;

int pmc_dsl_power_up(void);
int pmc_dsl_power_down(void);
int pmc_dsl_clock_set(int flag);	/* flag = 0 => turn off, turn on otherwise */
int pmc_dsl_mips_enable(int flag);	/* flag = 0 => turn off, turn on otherwise */
int pmc_dsl_mipscore_enable(int flag, int core);
int pmc_dsl_core_reset(void);
int BcmXdslGetAfePLLChOffset(unsigned int chanId);
int ReadVDSL3PLLChCfg(int chOffset, uint32_t *pChCfg);
int WriteVDSL3PLLChCfg(int chOffset, uint32_t chCfg);
int ReadVDSL3PLLMdiv(unsigned int chanId, pll_ch_cfg_t *pChCfg);
int ModifyVDSL3PLLMdiv(unsigned int chanId, pll_ch_cfg_t chCfg);
int ResetVDSL3PLL(void);
int SetVDSL3Ndiv(int ndiv);
int ReadVDSL3PLLNdiv(uint32_t *reg, int *ndiv);
int ReleaseVDSL3PLLResetb(void);
int ReleaseVDSL3PLLPostResetb(void);
int WaitForLockVDSL3PLL(void);
int SetVDSL3HoldCh1(unsigned int chanId, int value);
int SetVDSL3EnablCh1(unsigned int chanId, int value);


#endif //#ifndef PMC_DSL_H
