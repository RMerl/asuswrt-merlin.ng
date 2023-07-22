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

#include <assert.h>
#include <bcm_sotp.h>
#include <initcall.h>
#include <io.h>
#include <kernel/delay.h>
#include <mm/core_memprot.h>
#include <platform_config.h>
#include <util.h>

#define SOTP_ROWS_PER_REGION			4

#define SOTP_PROG_CONTROL			0x0
#define SOTP_PROG_CONTROL__OTP_CPU_MODE_EN	BIT(15)
#define SOTP_PROG_CONTROL__OTP_ECC_WREN		BIT(8)
#define SOTP_PROG_CONTROL__OTP_DISABLE_ECC	BIT(9)
#define SOTP_ADDR__OTP_ROW_ADDR_R		6

#define SOTP_WRDATA_0			0x4
#define SOTP_WRDATA_1			0x8
#define SOTP_ADDR				0xc

#define SOTP_CTRL_0				0x10
#define SOTP_CTRL_0__START			1
#define SOTP_CTRL_0__OTP_CMD			(0x1F << 1)
#define SOTP_PROG_ENABLE			(2 << 1)
#define SOTP_PROG_WORD				(10 << 1)
#define SOTP_READ				0
#define SOTP_CTRL_0__OTP_PROG_E			(1 << 21)
#define SOTP_CTRL_0__OTP_CMD_R			1

#define SOTP_STAT_0				0x18
#define SOTP_STAT_0__PROG_OK			BIT(2)
#define SOTP_STAT_0__FDONE      		BIT(3)

#define SOTP_STATUS_1	        		0x1c
#define SOTP_STATUS_1__CMD_DONE			BIT(1)
#define SOTP_STATUS_1__ECC_DET			BIT(17)

#define SOTP_RDDATA_0	        		0x20
#define SOTP_RDDATA_1	        		0x24
#define SOTP_SOTP_CHIP_STATES			0x28

#define SOTP_SOTP_OTP_WR_LOCK			0x38
#define SOTP_SOTP_OTP_RD_LOCK			0x3C

#define SOTP_CHIP_CTRL                                     0x4c
#define SOTP_CHIP_CTRL__CLEAR_SYSCTRL_ALL_MASTER_NS        (1 << 0)
#define SOTP_CHIP_CTRL__SW_OVERRIDE_CHIP_STATES            (1 << 4)
#define SOTP_CHIP_CTRL__SW_MANU_PROG                       (1 << 5)
#define SOTP_CHIP_CTRL__SW_CID_PROG                        (1 << 6)
#define SOTP_CHIP_CTRL__SW_AB_DEVICE                       (1 << 8)

#define CHIP_STATE_UNASSIGNED                                   2
#define CHIP_STATE_UNPROGRAMMED                         1
#define CHIP_STATE_AB_PROD                                      0x10
#define CHIP_STATE_AB_DEV                                       0x20

#define SOTP_ADDR_MASK						0x3ff
#define SOTP_DATA_MASK						0xFFFFFFFF
#define SOTP_ERR_MASK						0x1ff00000000

#define SOTP_TIMEOUT_US				300
#define MAX_TIME_OUT 30000

static vaddr_t bcm_sotp_base;

static TEE_Result otp_status_done_wait(vaddr_t addr, uint32_t bit)
{
	uint32_t timeout;
	uint32_t reg_val;

	timeout = 0;
	do {
		reg_val = io_read32(addr);
		timeout++;
	} while (((reg_val & bit) == 0) &&
		timeout < MAX_TIME_OUT);

	if (timeout == MAX_TIME_OUT)
		return TEE_ERROR_BUSY;
	return TEE_SUCCESS;
}

TEE_Result bcm_iproc_sotp_mem_read(uint32_t row_addr, uint32_t sotp_add_ecc,
				uint64_t *rdata)
{
	uint64_t read_data = 0;
	uint32_t reg_val = 0;
	TEE_Result ret = TEE_SUCCESS;

	assert(bcm_sotp_base);
	/* Check for FDONE status */
	ret = otp_status_done_wait((bcm_sotp_base + SOTP_STAT_0),
				   SOTP_STAT_0__FDONE);
	if (ret) {
		EMSG("FDONE status done wait failed");
		return ret;
	}

	/* Enable OTP access by CPU */
	io_setbits32((bcm_sotp_base + SOTP_PROG_CONTROL),
		     SOTP_PROG_CONTROL__OTP_CPU_MODE_EN);

	if (sotp_add_ecc == 1) {
		io_clrbits32((bcm_sotp_base + SOTP_PROG_CONTROL),
			     SOTP_PROG_CONTROL__OTP_DISABLE_ECC);
	} else {
		io_setbits32((bcm_sotp_base + SOTP_PROG_CONTROL),
			     SOTP_PROG_CONTROL__OTP_DISABLE_ECC);
	}

	/* 10 bit row address */
	reg_val = (row_addr & SOTP_ADDR_MASK) << SOTP_ADDR__OTP_ROW_ADDR_R;
	io_write32((bcm_sotp_base + SOTP_ADDR), reg_val);
	reg_val = SOTP_READ;
	io_write32((bcm_sotp_base + SOTP_CTRL_0), reg_val);

	/* Start bit to tell SOTP to send command to the OTP controller */
	io_setbits32((bcm_sotp_base + SOTP_CTRL_0), SOTP_CTRL_0__START);

	/* Wait for SOTP command done to be set */
	ret = otp_status_done_wait((bcm_sotp_base + SOTP_STAT_0),
				   SOTP_STATUS_1__CMD_DONE);
	if (ret) {
		EMSG("FDONE cmd done wait failed\n");
		return ret;
	}

	DMSG("CMD Done\n");

	/* Clr Start bit after command done */
	io_clrbits32((bcm_sotp_base + SOTP_CTRL_0), SOTP_CTRL_0__START);
	read_data = io_read32(bcm_sotp_base + SOTP_RDDATA_1);
	read_data = ((read_data & 0x1ff) << 32);
	read_data |= io_read32(bcm_sotp_base + SOTP_RDDATA_0);

	reg_val = io_read32(bcm_sotp_base + SOTP_STATUS_1);
	/* no ECC check till row 15 */
	if ((row_addr > 15) && (reg_val & SOTP_STATUS_1__ECC_DET)) {
		EMSG("SOTP ECC ERROR Detected ROW %d\n", row_addr);
		read_data = SOTP_ECC_ERR_DETECT;
	}

	/* Command done is cleared */
	io_setbits32((bcm_sotp_base + SOTP_STATUS_1), SOTP_STATUS_1__CMD_DONE);
	io_clrbits32((bcm_sotp_base + SOTP_PROG_CONTROL),
		     SOTP_PROG_CONTROL__OTP_CPU_MODE_EN);
	DMSG("read done\n");

	*rdata = read_data;
	return ret;
}

TEE_Result bcm_iproc_sotp_is_accessable(void)
{
	TEE_Result ret = TEE_ERROR_BAD_STATE;
	uint32_t chip_state_default =
			(CHIP_STATE_UNASSIGNED|CHIP_STATE_UNPROGRAMMED);
	uint32_t chip_state = io_read32(bcm_sotp_base +
						SOTP_SOTP_CHIP_STATES);
	if (chip_state_default & chip_state )
		ret = TEE_SUCCESS;

	DMSG("Secure SOTP access is %s\n", ((ret == TEE_SUCCESS) ?
	                                "enabled" : "disabed"));
	return ret;

}

static TEE_Result bcm_iproc_sotp_set_temp_lock( uint32_t lock_reg_offset, uint32_t row, uint32_t num_of_rows )
{
	uint32_t lock_val = io_read32(bcm_sotp_base + lock_reg_offset);
	uint32_t region = row / SOTP_ROWS_PER_REGION;
	uint32_t max_regions = region + (num_of_rows / SOTP_ROWS_PER_REGION);

	for( ; region < max_regions; region++ )
	{
		lock_val |=  1<<region;
	}
	io_write32( (bcm_sotp_base + lock_reg_offset), lock_val);

	return TEE_SUCCESS;
}

TEE_Result bcm_iproc_sotp_set_temp_rdlock( uint32_t row, uint32_t num_of_rows )
{
	return(bcm_iproc_sotp_set_temp_lock(SOTP_SOTP_OTP_RD_LOCK, row, num_of_rows));
}

TEE_Result bcm_iproc_sotp_set_temp_wrlock( uint32_t row, uint32_t num_of_rows )
{
	return(bcm_iproc_sotp_set_temp_lock(SOTP_SOTP_OTP_WR_LOCK, row, num_of_rows));
}

TEE_Result bcm_iproc_sotp_mem_write(uint32_t addr,
			uint32_t sotp_add_ecc, uint64_t wdata)
{
	uint32_t loop;
	uint32_t prog_array[4] = {0x0F, 0x04, 0x08, 0x0D};
	int ret = 0;
	uint32_t chip_state_default =
			(CHIP_STATE_UNASSIGNED|CHIP_STATE_UNPROGRAMMED);
	uint32_t chip_state = io_read32(bcm_sotp_base +
						SOTP_SOTP_CHIP_STATES);
	uint32_t chip_ctrl_default = 0;
	uint32_t exceptions = 0;

	DMSG("Enter iproc-sotp-write\n");
	/*
	 * The override settings is required to allow the customer to program
	 * the application specific keys into SOTP, before the conversion to
	 * one of the AB modes.
	 * At the end of write operation, the chip ctrl settings will restored
	 * to the state prior to write call
	 */
	if (chip_state & chip_state_default) {
		uint32_t chip_ctrl;

		exceptions = thread_mask_exceptions(THREAD_EXCP_FOREIGN_INTR);
		chip_ctrl_default = io_read32(bcm_sotp_base +
							SOTP_CHIP_CTRL);
		DMSG("enable special prog mode\n");

		chip_ctrl =
			SOTP_CHIP_CTRL__SW_OVERRIDE_CHIP_STATES |
			SOTP_CHIP_CTRL__SW_MANU_PROG |
			SOTP_CHIP_CTRL__SW_CID_PROG |
			SOTP_CHIP_CTRL__SW_AB_DEVICE |
			SOTP_CHIP_CTRL__CLEAR_SYSCTRL_ALL_MASTER_NS;
		io_write32( (bcm_sotp_base + SOTP_CHIP_CTRL), chip_ctrl );
	}
	/* Check for FDONE status */
	ret = otp_status_done_wait(
				(bcm_sotp_base + SOTP_STAT_0),
				SOTP_STAT_0__FDONE);
	if (ret) {
		EMSG("FDONE status failed\n");
		goto end;
	}
	/* Enable OTP access by CPU */
	io_setbits32((bcm_sotp_base + SOTP_PROG_CONTROL),
			SOTP_PROG_CONTROL__OTP_CPU_MODE_EN);

	if (addr > 15) {
		if (sotp_add_ecc == 0)
			io_clrbits32((bcm_sotp_base +
				SOTP_PROG_CONTROL),
				SOTP_PROG_CONTROL__OTP_ECC_WREN);
		if (sotp_add_ecc == 1)
			io_setbits32((bcm_sotp_base +
				SOTP_PROG_CONTROL),
				SOTP_PROG_CONTROL__OTP_ECC_WREN);
	} else
		io_clrbits32(
			(bcm_sotp_base + SOTP_PROG_CONTROL),
				SOTP_PROG_CONTROL__OTP_ECC_WREN);

	io_clrbits32((bcm_sotp_base + SOTP_CTRL_0),
					SOTP_CTRL_0__OTP_CMD);
	io_setbits32((bcm_sotp_base + SOTP_CTRL_0),
					SOTP_PROG_ENABLE);
	/*
	 * In order to avoid unintentional writes programming of the OTP array,
	 * the OTP Controller must be put into programming mode before it will
	 * accept program commands. This is done by writing 0xF, 0x4, 0x8, 0xD
	 * with program commands prior to starting the actual programming
	 * sequence
	 */

	for (loop = 0; loop < 4; loop++) {
		io_write32((bcm_sotp_base + SOTP_WRDATA_0), prog_array[loop]);
		/*
		 * Start bit to tell SOTP to send command to the OTP
		 * controller
		 */
		io_setbits32((bcm_sotp_base + SOTP_CTRL_0),
				SOTP_CTRL_0__START);

		/* Wait for SOTP command done to be set */
		ret = otp_status_done_wait(
				(bcm_sotp_base + SOTP_STATUS_1),
				SOTP_STATUS_1__CMD_DONE);
		if (ret)
			goto end;

		/* Command done is cleared w1c */
		io_setbits32((bcm_sotp_base + SOTP_STATUS_1),
				SOTP_STATUS_1__CMD_DONE);
		/* Clr Start bit after command done */
		io_clrbits32((bcm_sotp_base + SOTP_CTRL_0),
				SOTP_CTRL_0__START);
	}
	/* Check for PROGOK */
	ret = otp_status_done_wait(
				(bcm_sotp_base + SOTP_STAT_0),
				SOTP_STAT_0__PROG_OK);
	if (ret) {
		EMSG("PROGOK failed:%x\n",
		     io_read32(bcm_sotp_base + SOTP_STAT_0));
		goto end;
	}
	/* Set  10 bit row address */
	io_write32((bcm_sotp_base + SOTP_ADDR), ((addr & SOTP_ADDR_MASK) << 6));
	/* Set SOTP Row data */
	io_write32((bcm_sotp_base + SOTP_WRDATA_0), (wdata & SOTP_DATA_MASK));
	/* Set SOTP ECC and error bits */
	io_write32((bcm_sotp_base + SOTP_WRDATA_1), ((wdata & SOTP_ERR_MASK) >> 32));
	/* Set prog_word command */
	io_write32((bcm_sotp_base + SOTP_CTRL_0), SOTP_PROG_WORD);
	/* Start bit to tell SOTP to send command to the OTP controller */
	io_setbits32((bcm_sotp_base + SOTP_CTRL_0),
						SOTP_CTRL_0__START);
	/* Wait for SOTP command done to be set */
	ret = otp_status_done_wait(
				(bcm_sotp_base + SOTP_STATUS_1),
				SOTP_STATUS_1__CMD_DONE);
	if (ret) {
		EMSG("SOTP cmd done error\n");
		goto end;
	}

	/* Command done is cleared w1c */
	io_setbits32((bcm_sotp_base + SOTP_STATUS_1),
					SOTP_STATUS_1__CMD_DONE);
	/* disable OTP acces by CPU */
	io_clrbits32((bcm_sotp_base + SOTP_PROG_CONTROL),
			SOTP_PROG_CONTROL__OTP_CPU_MODE_EN);
	/* Clr Start bit after command done */
	io_clrbits32((bcm_sotp_base + SOTP_CTRL_0),
			SOTP_CTRL_0__START);
end:
	if (chip_state & chip_state_default) {
		io_write32((bcm_sotp_base + SOTP_CHIP_CTRL), chip_ctrl_default);
		thread_unmask_exceptions(exceptions);
	}

	DMSG("Exit iproc sotp write :%d\n", ret);
	return ret;
}

uint32_t bcm_iproc_sotp_get_status1(void)
{
	return(io_read32(bcm_sotp_base + SOTP_STATUS_1));
}

static TEE_Result bcm_sotp_init(void)
{
	bcm_sotp_base = (vaddr_t)phys_to_virt(SOTP_BASE, MEM_AREA_IO_SEC);

	DMSG("bcm_sotp init done\n");
	return TEE_SUCCESS;
}

driver_init(bcm_sotp_init);
