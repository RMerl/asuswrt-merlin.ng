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
#include "bcm_sotp_ser.h"
#include "bcm_sotp_ser_core.h"
#include <io.h>
#include <kernel/pseudo_ta.h>
#include <trace.h>


#define TA_NAME		"tee_sotp_service.ta"

sotp_state_t sotp_state;


/*
 * Trusted Application Entry Points
 */

static TEE_Result create_ta(void)
{
	DMSG("create entry point for static ta \"%s\"", TA_NAME);
	return TEE_SUCCESS;
}

static void destroy_ta(void)
{
	DMSG("destroy entry point for static ta \"%s\"", TA_NAME);
}

static TEE_Result open_session(uint32_t nParamTypes __unused,
		TEE_Param pParams[4] __unused, void **ppSessionContext __unused)
{
	DMSG("open entry point for static ta \"%s\"", TA_NAME);
	return TEE_SUCCESS;
}

static void close_session(void *pSessionContext __unused)
{
	DMSG("close entry point for static ta \"%s\"", TA_NAME);
}


static TEE_Result sotp_service_read(
			uint32_t nParamTypes,
			TEE_Param pParams[TEE_NUM_PARAMS])
{
	uint32_t row_addr;
	uint32_t num_of_rows;
	uint32_t flags;
	uint32_t read_status;
	uint32_t *ret_val;
	uint32_t row_type;
	uint8_t *data;
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
					TEE_PARAM_TYPE_MEMREF_INOUT,
					TEE_PARAM_TYPE_VALUE_INPUT,
					TEE_PARAM_TYPE_VALUE_OUTPUT);

	if (exp_param_types != nParamTypes) {
		EMSG("Invalid Param types\n");
		return TEE_ERROR_BAD_PARAMETERS;
	}

	row_addr = pParams[0].value.a;
	num_of_rows = pParams[0].value.b;
	flags = pParams[2].value.a;
	data = (uint8_t *)pParams[1].memref.buffer;
		EMSG("data ptr = %p\n", data);
	ret_val = (uint32_t *)&pParams[3].value.a;

	*ret_val = SOTP_SER_NOT_SUPPORTED;

	/* check the row, length for validity */
	if (row_addr > SOTP_SER_END_ROW) {
		EMSG("Invalid SOTP ROW addr:%x\n", row_addr);
		*ret_val = SOTP_SER_INVALID_ROW_ADDR;
		return TEE_ERROR_BAD_PARAMETERS;
	}

	if (num_of_rows > SOTP_SER_MAX_ROWS) {
		EMSG("Num of rows is invalid:%x\n", num_of_rows);
		*ret_val = SOTP_SER_INVALID_NUM_OF_ROWS;
		return TEE_ERROR_BAD_PARAMETERS;
	}

	row_type = map_addr_2_section(row_addr, num_of_rows);
	if (row_type == OTP_SECTION_INVALID) {
		EMSG("SOTP Read operation Abort. Invalid input data\n");
		*ret_val = SOTP_SER_INVALID_NUM_OF_ROWS;
		return TEE_ERROR_BAD_PARAMETERS;
	}

	read_status = bcm_read_data_internal(&sotp_state, data, num_of_rows,
					row_addr, row_type, (flags & SOTP_SER_FLAGS_SKIP_ECC));

	if (read_status == IPROC_OTP_VALID)
		*ret_val = SOTP_SER_ROW_VALID;
	else if (read_status == IPROC_OTP_INVALID)
		*ret_val = SOTP_SER_VERIFY_DATA_ERROR;
	else if (read_status == IPROC_OTP_ERASED)
		*ret_val = SOTP_SER_ROW_EMPTY;

#ifdef USE_HW_CRC		
	/* Check the CRC values, if the keys section is read*/
	if ((num_of_rows == SOTP_SER_KEY_ROWS_COUNT) &&
		(read_status == IPROC_OTP_VALID) &&
		(row_addr >= SOTP_SER_DAUTH_ROW_BEGIN))  {
		if (bcm_sotp_get_crc_status(row_addr) != SOTP_CRC_STATUS_OK)
			*ret_val = SOTP_SER_VERIFY_CRC_ERROR;
	}
#endif						

	switch (*ret_val) {
	case SOTP_SER_ROW_EMPTY:
		IMSG("ROWS are EMPTY\n");
		break;
	case SOTP_SER_ROW_VALID:
		IMSG("ROWS are VALID\n");
		break;
	case SOTP_SER_VERIFY_CRC_ERROR:
		IMSG("CRC ERROR!!!!\n");
		break;
	case SOTP_SER_VERIFY_DATA_ERROR:
		IMSG("DATA ERROR!!!\n");
		break;
	default:
		IMSG("Unknown Read status!!!\n");
	}

	DMSG("sotp read done, ret value:%d\n", *ret_val);
	return TEE_SUCCESS;
}

static TEE_Result sotp_service_write(
			uint32_t nParamTypes,
			TEE_Param pParams[TEE_NUM_PARAMS])
{
	OTP_STATUS sotp_ret;
	uint32_t row;
	uint32_t num_of_rows;
	uint32_t flags;
	uint32_t *data;
	uint32_t *ret_val;
	uint32_t row_type;
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
					TEE_PARAM_TYPE_MEMREF_INOUT,
					TEE_PARAM_TYPE_VALUE_INPUT,
					TEE_PARAM_TYPE_VALUE_OUTPUT);
	TEE_Result ret = TEE_SUCCESS;

	DMSG("Entering TA SOTP Service write\n");
	if (exp_param_types != nParamTypes)
		return TEE_ERROR_BAD_PARAMETERS;

	row = pParams[0].value.a;
	num_of_rows = pParams[0].value.b;
	data = (uint32_t *)pParams[1].memref.buffer;
	flags = pParams[2].value.a;
	ret_val = (uint32_t *)&pParams[3].value.a;

	ret = bcm_check_sotp_access(&sotp_state);
	if (ret != TEE_SUCCESS) {
		EMSG("Error access failed, cannot write\n");
		*ret_val = SOTP_SER_ACCESS_ERROR;
		return ret;
	}

	/* check the row, length for validity */
	if (row > SOTP_SER_END_ROW) {
		EMSG("Invalid ROW addr:%x\n", row);
		*ret_val = SOTP_SER_INVALID_ROW_ADDR;
		return TEE_ERROR_BAD_PARAMETERS;
	}

	if (num_of_rows > SOTP_SER_MAX_ROWS) {
		EMSG("Num of rows is invalid:%x\n", num_of_rows);
		*ret_val = SOTP_SER_INVALID_NUM_OF_ROWS;
		return TEE_ERROR_BAD_PARAMETERS;
	}

	row_type = map_addr_2_section(row, num_of_rows);
	if (row_type == OTP_SECTION_INVALID) {
		EMSG("SOTP Write operation Abort. Invalid input data\n");
		*ret_val = SOTP_SER_INVALID_NUM_OF_ROWS;
		return TEE_ERROR_BAD_PARAMETERS;
	}

	/* check if the rows are already programmed */
	sotp_ret = bcm_read_data_internal(&sotp_state, 0, num_of_rows, row,
						row_type, 0);
	if (sotp_ret != IPROC_OTP_ERASED) {
		if (sotp_ret == IPROC_OTP_VALID) {
			if (!(flags & SOTP_SER_FLAGS_SKIP_ECC)) {
				EMSG("SOTP row %d are already programmed\n", row);
				*ret_val = SOTP_SER_WRITE_NOT_BLANK_ERROR;
				return TEE_ERROR_BAD_STATE;
			}
		} else {
			EMSG("SOTP row %d is invalid\n", row);
			*ret_val = SOTP_SER_WRITE_DATA_ERROR;
			return TEE_ERROR_BAD_STATE;
		}
	}

	if (flags & SOTP_SER_FLAGS_GENERATE) {
		EMSG("Generate data internally not implemented\n");
		*ret_val = SOTP_SER_NOT_SUPPORTED;
		return TEE_ERROR_NOT_IMPLEMENTED;
	}

	sotp_ret = write_data_internal(&sotp_state, data, num_of_rows, row,
					row_type, (flags & SOTP_SER_FLAGS_SKIP_ECC));
	if (sotp_ret != IPROC_OTP_VALID) {
		EMSG("SOTP write operation failed\n");
		*ret_val = SOTP_SER_WRITE_DATA_ERROR;
		ret = TEE_ERROR_BAD_STATE;
	} else {
		*ret_val = SOTP_SER_OK;
		ret = TEE_SUCCESS;
	}

	/* set the write lock (for keys), if write lock flag is set */
	if ((ret == TEE_SUCCESS) && (row_type != OTP_SECTION_CFG) &&
					(flags & SOTP_SER_FLAGS_WRITE_LOCK)) {
		*ret_val = bcm_set_lock_internal(&sotp_state, row, num_of_rows,
					 flags);
		if (*ret_val != SOTP_SER_OK)
			ret = TEE_ERROR_BAD_STATE;
	}

	DMSG("sotp write completed:%d\n", ret);
	return ret;
}

static TEE_Result sotp_service_lock(uint32_t nParamTypes,
				     TEE_Param pParams[TEE_NUM_PARAMS])
{
	uint32_t row;
	uint32_t num_of_rows;
	uint32_t flags;
	uint32_t *ret_val;
	TEE_Result ret;
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
					TEE_PARAM_TYPE_VALUE_INPUT,
					TEE_PARAM_TYPE_VALUE_OUTPUT,
					TEE_PARAM_TYPE_NONE);
	IMSG("Entering TA SOTP Service lock\n");
	if (exp_param_types != nParamTypes)
		return TEE_ERROR_BAD_PARAMETERS;

	row = pParams[0].value.a;
	num_of_rows = pParams[0].value.b;
	flags = pParams[1].value.a;
	ret_val = (uint32_t *)&pParams[2].value.a;
	*ret_val = bcm_set_lock_internal(&sotp_state, row, num_of_rows,
					 flags);
	if (*ret_val != SOTP_SER_OK)
		ret = TEE_ERROR_BAD_STATE;
	else
		ret = TEE_SUCCESS;

	return ret;
}

static TEE_Result sotp_service_init(
			uint32_t nParamTypes __unused,
			TEE_Param pParams[TEE_NUM_PARAMS] __unused)
{

	return TEE_SUCCESS;
}

static TEE_Result invoke_command(void *pSessionContext __unused,
		uint32_t nCommandID, uint32_t nParamTypes, TEE_Param pParams[4])
{
	TEE_Result res;

	IMSG("command entry point[%d] for \"%s\"", nCommandID, TA_NAME);

	switch (nCommandID) {
	case CMD_SOTP_SERVICE_INIT:
		res = sotp_service_init(nParamTypes, pParams);
		break;
	case CMD_SOTP_SERVICE_READ:
		res = sotp_service_read(nParamTypes, pParams);
		break;
	case CMD_SOTP_SERVICE_WRITE:
		res = sotp_service_write(nParamTypes, pParams);
		break;
	case CMD_SOTP_SERVICE_LOCK:
		res = sotp_service_lock(nParamTypes, pParams);
		break;
	default:
		EMSG("%d Not supported\n", nCommandID);
		res = TEE_ERROR_NOT_SUPPORTED;
		break;
	}
	return res;
}

pseudo_ta_register(.uuid = SOTP_SERVICE_UUID, .name = TA_NAME,
		   .flags = PTA_DEFAULT_FLAGS,
		   .create_entry_point = create_ta,
		   .destroy_entry_point = destroy_ta,
		   .open_session_entry_point = open_session,
		   .close_session_entry_point = close_session,
		   .invoke_command_entry_point = invoke_command);
