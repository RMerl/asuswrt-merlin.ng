/*
<:copyright-BRCM:2019:DUAL/GPL:standard 

   Copyright (c) 2019 Broadcom 
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

#ifndef BCM_SOTP_H
#define BCM_SOTP_H

#include <stdint.h>
#include <tee_api.h>

#define ISAO_BITS					(3 << 20)
#define SOTP_DEV_CONFIG					12
#define SOTP_ECC_ERR_DETECT				0x8000000000000000

uint32_t bcm_iproc_sotp_get_status1(void);
TEE_Result bcm_iproc_sotp_mem_read(uint32_t row_addr, uint32_t sotp_add_ecc, uint64_t *rdata);
TEE_Result bcm_iproc_sotp_mem_write(uint32_t addr, uint32_t sotp_add_ecc, uint64_t wdata);
TEE_Result bcm_iproc_sotp_set_temp_rdlock( uint32_t row, uint32_t num_of_rows );
TEE_Result bcm_iproc_sotp_set_temp_wrlock( uint32_t row, uint32_t num_of_rows );
TEE_Result bcm_iproc_sotp_is_accessable(void);
#endif
