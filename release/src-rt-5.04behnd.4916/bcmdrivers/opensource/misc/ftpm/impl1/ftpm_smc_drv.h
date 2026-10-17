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

#ifndef __FTPM_SMC_DRV_H__
#define __FTPM_SMC_DRV_H__

#include <linux/dma-mapping.h>
#include <linux/tpm.h>

#define MAX_COMMAND_SIZE       4096
#define MAX_RESPONSE_SIZE      4096
#define MAX_TRANSACTION_SIZE   (MAX_COMMAND_SIZE + MAX_RESPONSE_SIZE)

#define FTPM_RPC_TIMEOUT (10 * 60) // 10 minutes

enum ftpm_smc_command_id {
    FTPM_SMC_SUBMIT_COMMAND  = 0,     // Submit TPM command
    FTPM_SMC_MAX_FUNC_NUM
};

struct ftpm_smc_rpc_msg {
	/* Word 0 */
	uint32_t header;
	union {
		struct {
			/* Word 1 */
			uint32_t address_low;
			/* Word 2 */
			uint32_t address_high;
			/* Word 3 */
			uint32_t length;
		} request;
		struct {
			/* Word 1 */
			uint32_t reserved0;
			/* Word 2 */
			uint32_t return_code;
			/* Word 3 */
			uint32_t length;
		} response;
	} body;
} __packed;

struct ftpm_smc_private {
	struct tpm_chip *chip;
	int tunnel_id;
	u8 *shm_buf;
	dma_addr_t shm_buf_dma_addr;
	u8 resp_buf[MAX_RESPONSE_SIZE];
	size_t resp_len;
};

#endif /* __FTPM_SMC_DRV_H__ */
