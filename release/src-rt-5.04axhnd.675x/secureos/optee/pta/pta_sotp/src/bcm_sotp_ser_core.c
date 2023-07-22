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
#include "bcm_sotp_ser_core.h"
#include <kernel/pseudo_ta.h>
#include <io.h>
#include <trace.h>

/*
 * CRC32 utility definition & function
 */
#define POLYNOMIAL		0x04c11db7
#define MSB_BIT			(1 << 31)
#define CRC_TABLE_SIZE		256
#define BIT_24			24
#define BIT_8			8
#define XOR_VAL			0xFFFFFFFF
#define CRC_INIT_VAL		0xFFFFFFFF

#ifdef USE_HW_CRC
uint32_t  crc_table[CRC_TABLE_SIZE];
int crc_init_state;

static void crc_init(void)
{
	uint32_t remainder;
	uint32_t dividend;
	uint8_t  bit;

	/* Compute the remainder of each possible dividend. */
	for (dividend = 0; dividend < CRC_TABLE_SIZE; ++dividend) {

		/* Start with the dividend followed by zeros. */
		remainder = (dividend << BIT_24);

		/* Perform modulo-2 division, a bit at a time. */
		for (bit = BIT_8; bit > 0; --bit) {
			/* Try to divide the current data bit. */
			if (remainder & MSB_BIT)
				remainder = (remainder << 1) ^ POLYNOMIAL;
			else
				remainder = (remainder << 1);
		}
		/* Store the result into the table.*/
		crc_table[dividend] = remainder;
	}
	crc_init_state = 1;
}

static uint32_t reflect(uint32_t data, uint8_t nbits)
{
	uint32_t reflection;
	int i;

	reflection = 0;
	for (i = 0; i < nbits; i++) {

		/* If the LSB bit is set, set the reflection of it.*/
		if (data & 0x01)
			reflection |= (1 << ((nbits - 1) - i));
		data = (data >> 1);
	}
	return reflection;
}

static uint32_t calc_crc32(uint8_t *input, uint32_t length)
{
	uint32_t cur_val = CRC_INIT_VAL;
	uint8_t data, temp_val;
	uint32_t i;
	int j;

	if (!crc_init_state)
		crc_init();

	for (i = 0; i < length/4; i++) {
		for (j = 3; j >= 0; j--) {
			temp_val = input[(4 * i) + j];
			temp_val = reflect(temp_val, 8);
			data = temp_val ^ (cur_val >> BIT_24);
			cur_val = crc_table[data] ^ (cur_val << BIT_8);
		}
	}
	cur_val = reflect(cur_val, 32);
	cur_val = cur_val ^ XOR_VAL;
	return cur_val;
}
#else
/* SW CRC implementation - Common with bootloader and linux */
unsigned int crc32_table[256] = {
	0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
	0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
	0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
	0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
	0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
	0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
	0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
	0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
	0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
	0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
	0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
	0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
	0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
	0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
	0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
	0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
	0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
	0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
	0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
	0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
	0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
	0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
	0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
	0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
	0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
	0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
	0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
	0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
	0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
	0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
	0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
	0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
	0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
	0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
	0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
	0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
	0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
	0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
	0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
	0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
	0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
	0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
	0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
	0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
	0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
	0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
	0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
	0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
	0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
	0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
	0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
	0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
	0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
	0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
	0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
	0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
	0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
	0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
	0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
	0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
	0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
	0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
	0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
	0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};

static uint32_t __attribute__ ((noinline)) get_crc32(unsigned char *pdata, uint32_t size, uint32_t crc)
{
	while (size-- > 0)
		crc = (crc >> 8) ^ crc32_table[(crc ^ *pdata++) & 0xff];
	return crc;
}

static uint32_t __attribute__ ((noinline)) calc_crc32(uint8_t *input, uint32_t length)
{
	uint32_t cur_val = CRC_INIT_VAL;
	cur_val = get_crc32( input, length, cur_val );
	return cur_val;
}
#endif

static OTP_STATUS validate_write_data(sotp_state_t *sotp_state,
				uint32_t row_addr, uint32_t data, uint32_t skip_ecc)
{
	int ret;
	sotp_ser_data_t sotp_row = sotp_state->row;

	bcm_iproc_sotp_mem_read( row_addr, !skip_ecc,
					&sotp_row.data64);
	/* check for ECC error starting from row 12 */
	if ((row_addr >= SOTP_SER_DAUTH_ROW_BEGIN) &&
				(sotp_row.data64 & SOTP_ECC_ERR_DETECT))
		ret = IPROC_OTP_INVALID;
	else {
		/* ensure data written is same on read here */
		if (data == sotp_row.data32[0])
			ret = IPROC_OTP_VALID;
		else
			ret = IPROC_OTP_INVALID;
	}
	return ret;
}

static OTP_STATUS mark_failed_rows(sotp_state_t *sotp_state,
							uint32_t row_addr)
{
	int ret = IPROC_OTP_INVALID;
	sotp_ser_data_t sotp_row = sotp_state->row;

	sotp_row.data64 = SOTP_FAIL_BITS;
	bcm_iproc_sotp_mem_write( row_addr, 0,
				 sotp_row.data64);
	bcm_iproc_sotp_mem_read( row_addr, 0,
					&sotp_row.data64);
	if (sotp_row.data64 & SOTP_FAIL_BITS)
		ret = IPROC_OTP_VALID;

	return ret;
}

TEE_Result bcm_check_sotp_access(sotp_state_t *sotp_state)
{
	TEE_Result ret;
	sotp_ser_data_t sotp_row = sotp_state->row;

	bcm_iproc_sotp_mem_read( SOTP_DEV_CONFIG, 0,
				&sotp_row.data64);

	/*
	 * ISAO bits should be checked and the access should be decided.
	 * If ISAO bits are 0, then the access should be valid
	 * If ISAO bits are set, then the access should be valid only for AB
	 * devices
	 */
	if (sotp_row.data32[0] & (ISAO_BITS)) {
		/* if ISAO bits are set, then check chip state */
                ret = bcm_iproc_sotp_is_accessable();
	} else
		ret = TEE_SUCCESS;

	return ret;
}

uint32_t map_addr_2_section(uint32_t row_addr, uint32_t num_of_rows)
{
	uint32_t otp_section = OTP_SECTION_INVALID;
	/*
	 * It is essential to use the number of rows as 8 even though
	 * the key data is less than 256 bits ( example 128 bits).
	 * The crc row will be computed based on 8 rows, including the
	 * unused rows will be used for crc calculation
	 */
	if ((row_addr >= SOTP_SER_DAUTH_ROW_BEGIN) && (num_of_rows != 8)) {
		EMSG("Invalid num_of_rows for key section\n");
		return otp_section;
	}

	if (row_addr < SOTP_SER_DAUTH_ROW_BEGIN) {
		IMSG("Device Cfg section\n");
		return OTP_SECTION_CFG;
	}

	/*
	 * check for the row_addr validity. It should match with one of the
	 * key rows start address
	 */
	if ((row_addr == SOTP_SER_DAUTH_ROW_BEGIN) ||
			(row_addr == SOTP_SER_HMAC_ROW_BEGIN) ||
			(row_addr == SOTP_SER_AES_ROW_BEGIN)) {
		DMSG("BOOT key section\n");
		otp_section = OTP_SECTION_BOOT_KEY;
	} else if ((row_addr == SOTP_SER_CUST_REG_1_BEGIN) ||
			(row_addr == SOTP_SER_CUST_REG_2_BEGIN) ||
			(row_addr == SOTP_SER_CUST_REG_3_BEGIN)) {
		DMSG("Cust key section\n");
		otp_section = OTP_SECTION_CUST_KEY;
	} else if (row_addr == SOTP_SER_CUST_REG_4_BEGIN) {
		DMSG("Reserved key section\n");
		otp_section = OTP_SECTION_RESD_KEY;
	}

	return otp_section;
}

#ifdef USE_HW_CRC
TEE_Result bcm_sotp_get_crc_status(int row)
{
	TEE_Result status;
	uint32_t crc_status_val = 0;
	int index = -1;

	crc_status_val = bcm_iproc_sotp_get_status1();
	crc_status_val = crc_status_val >> SOTP_SER_START_SECTION;

	if ((row >= SOTP_SER_DAUTH_ROW_BEGIN) &&
					(row <= SOTP_SER_DAUTH_ROW_END)) {
		index = (1 << 0);
	} else if ((row >= SOTP_SER_HMAC_ROW_BEGIN) &&
					(row <= SOTP_SER_HMAC_ROW_END)) {
		index = (1 << 1);
	} else if ((row >= SOTP_SER_AES_ROW_BEGIN) &&
					(row <= SOTP_SER_AES_ROW_END)) {
		index = (1 << 2);
	} else if ((row >= SOTP_SER_CUST_REG_1_BEGIN) &&
					(row <= SOTP_SER_CUST_REG_1_END)) {
		index = (1 << 3);
	} else if ((row >= SOTP_SER_CUST_REG_2_BEGIN) &&
					(row <= SOTP_SER_CUST_REG_2_END)) {
		index = (1 << 4);
	} else if ((row >= SOTP_SER_CUST_REG_3_BEGIN) &&
					(row <= SOTP_SER_CUST_REG_3_END)) {
		index = (1 << 5);
	} else if ((row >= SOTP_SER_CUST_REG_4_BEGIN) &&
					(row <= SOTP_SER_CUST_REG_4_END)) {
		index = (1 << 6);
	} else {
		/* ignore CRC check for the other rows */
		return TEE_SUCCESS;
	}

	if (index & crc_status_val) {
		EMSG("CRC error[%d] %x\n", row, crc_status_val);
		status = TEE_ERROR_BAD_STATE;
	} else
		status = TEE_SUCCESS;

	return status;
}
#endif


OTP_STATUS bcm_read_data_internal(sotp_state_t *sotp_state, uint8_t *data,
		uint32_t num_of_rows, uint32_t row_addr, uint32_t row_type, uint32_t skip_ecc)
{
	uint32_t i = 0;
	uint32_t row_val = 0;
	uint32_t crc_val = 0xFFFFFFFF;
#ifndef USE_HW_CRC				
	uint32_t crc_calc = 0xFFFFFFFF;
#endif	
	OTP_STATUS ret = IPROC_OTP_INVALID;
	sotp_ser_data_t sotp_row = sotp_state->row;
	uint32_t failed_rows = 0;
	uint32_t start_addr = row_addr;
	uint8_t *orig_data = data;

	sotp_row.data64 = 0;

	while ((i < num_of_rows) && (failed_rows <= 3)) {
		bcm_iproc_sotp_mem_read( row_addr, !skip_ecc,
							&sotp_row.data64);
		row_val |= sotp_row.data32[0];
		DMSG("[%d-%d]:0x%08X-0x%08X\n", row_addr, i, sotp_row.data32[1],
							sotp_row.data32[0]);
		if (!(sotp_row.data64 & SOTP_ECC_ERR_DETECT) &&
					!(sotp_row.data64 & SOTP_FAIL_BITS)) {
			if (data) {
				data[0] = sotp_row.data8[0];
				data[1] = sotp_row.data8[1];
				data[2] = sotp_row.data8[2];
				data[3] = sotp_row.data8[3];
				data = data + 4;
			}
			i++;
		} else {
			/*
			 * In case of fail bit set, read the redundant rows
			 * for keys section
			 */
			if (start_addr > SOTP_SER_DAUTH_ROW_BEGIN) {
				IMSG("Row read:%d error, skip\n", row_addr);
				failed_rows++;
			}
		}
		row_addr++;
	}

	/* if all the rows are not read, then it indicates an error */
	if (i < num_of_rows)
		return IPROC_OTP_INVALID;

	/* If it is a key section read and is not empty, then verify crc value as well */
	if( !row_val )
		crc_val = 0;
	else if ((row_type != OTP_SECTION_CFG) && (num_of_rows == 8)) {
		while (failed_rows < 3) {
			bcm_iproc_sotp_mem_read( row_addr, 1,
						&sotp_row.data64);
			crc_val = sotp_row.data32[0];
			DMSG("CRC[%d-%d]:0x%08X-0x%08X\n", row_addr, i,
			     sotp_row.data32[1], sotp_row.data32[0]);
			if (!(sotp_row.data64 & SOTP_ECC_ERR_DETECT) &&
					!(sotp_row.data64 & SOTP_FAIL_BITS)) {
#ifndef USE_HW_CRC				
				/* Calculate CRC */
				crc_calc = calc_crc32(orig_data, sizeof(uint32_t) * num_of_rows);
				if( crc_calc != crc_val ) {
					/* Clear CRC val to signal CRC failiure */
					IMSG("CRC ERROR while reading from row %d, exp:0x%08x calc:0x%08x!!!!\n", row_addr, crc_val, crc_calc);
					crc_val = 0;
				}
#endif				

				/* crc data is valid. exit the loop */
				break;
			}
			/* next row (redundant row) */
			failed_rows++;
			row_addr++;
		}
	}

	/* check the integrity of the data */
	if (row_val && crc_val) 
		ret = IPROC_OTP_VALID;
	else if (!row_val && !crc_val)
		ret = IPROC_OTP_ERASED;
	else
		ret = IPROC_OTP_INVALID;

	return ret;
}

OTP_STATUS write_data_internal(sotp_state_t *sotp_state, uint32_t *data,
		uint32_t num_of_rows, uint32_t row_addr, uint32_t row_type, uint32_t skip_ecc)
{
	uint32_t i = 0;
	OTP_STATUS ret = IPROC_OTP_VALID;
	sotp_ser_data_t sotp_row = sotp_state->row;
	uint32_t failed_rows = 0;

	while (i < num_of_rows) {

		sotp_row.data32[0] = data[i];

		bcm_iproc_sotp_mem_write( row_addr, !skip_ecc,
					sotp_row.data64);

		sotp_row.data32[0] = 0;
		/* Read back data to verify */
		ret = validate_write_data(sotp_state, row_addr, data[i], skip_ecc);
		if (ret == IPROC_OTP_VALID) {
			/* select next row */
			i++;
		} else {
			IMSG("Writing to Redundant Row (%d-%d)\n", row_addr,
								failed_rows);
			/* Attempt to use redundant rows */
			if (failed_rows >= 3) {
				EMSG("All redundant rows used. Write error\n");
				ret = IPROC_OTP_INVALID;
				break;
			}

			/* Set the FAIL bits in the row that failed ECC chk */
			if (mark_failed_rows(sotp_state, row_addr) ==
					IPROC_OTP_INVALID) {
				EMSG("Setting Fail bits Error!!!\n");
				ret = IPROC_OTP_INVALID;
				break;
			}
			failed_rows++;
		}
		row_addr++;
	}

	/* if all the rows are not written, then it indicates an error */
	if (ret != IPROC_OTP_VALID) {
		EMSG("Write operation error, Exit.\n");
		return ret;
	}

	if ((row_type != OTP_SECTION_CFG) && (num_of_rows == 8)) {
		uint32_t crc_val;

		crc_val = calc_crc32((unsigned char *)data,
						sizeof(uint32_t) * num_of_rows);
		IMSG("CRC value: %x\n", crc_val);
		while (failed_rows < 3) {
			sotp_row.data32[0] = crc_val;
			bcm_iproc_sotp_mem_write(
						row_addr, 1, sotp_row.data64);

			/* Read back data to verify */
			ret = validate_write_data(sotp_state, row_addr,
								crc_val, 0);
			if (ret == IPROC_OTP_VALID)
				break;
			IMSG("Using Redundant row for CRC (%d-%d)\n",
							row_addr, failed_rows);
			/* Attempt to use redundant rows */
			/* Set the FAIL bits */
			if (mark_failed_rows(sotp_state, row_addr) ==
				IPROC_OTP_INVALID) {
				EMSG("Setting Fail bits Error!!!\n");
				ret = IPROC_OTP_INVALID;
				break;
			}
			failed_rows++;
			row_addr++;
		}

		if (failed_rows >= 3) {
			EMSG("All redundant rows used (crc). Write error\n");
			ret = IPROC_OTP_INVALID;
		}
	}

	return ret;
}

uint32_t bcm_set_lock_internal(sotp_state_t *sotp_state, uint32_t row,
				 uint32_t num_of_rows, uint32_t flags)
{
	uint32_t region = 0;
	uint32_t lock_addr = 0;
	uint32_t ret = SOTP_SER_INVALID_COMMAND;
	sotp_ser_data_t sotp_row = sotp_state->row;

	IMSG("Num of Rows:%d Rows:%d Flags:%x\n",
		num_of_rows, row, flags);

	if (num_of_rows > SOTP_SER_MAX_LOCK_ROWS) {
		EMSG("Invalid num of rows, Should be <= 12\n");
		ret = SOTP_SER_INVALID_ARGS;
		goto end_int_lock;
	}

	region = row / SOTP_SER_ROWS_PER_REGION;

	/* If we are locking a row in the key sections then lock entire section */
	if (region >= SOTP_SER_CONFIG_REGION)
		num_of_rows = SOTP_SER_MAX_LOCK_ROWS;
	else
		num_of_rows = SOTP_SER_ROWS_PER_REGION;

	if (flags & SOTP_SER_FLAGS_WRITE_LOCK) {
		if (region < SOTP_SER_MAX_REGION_PER_LOCK_REG)
			lock_addr = SOTP_SER_WRAPPER_ROW_0;
		else
			lock_addr = SOTP_SER_WRAPPER_ROW_1;
	} else if (flags & SOTP_SER_FLAGS_READ_LOCK) {
#ifdef USE_PERMANENT_READLOCK		
		if (region < SOTP_SER_MAX_REGION_PER_LOCK_REG)
			lock_addr = SOTP_SER_PROG_ROW_0;
		else
			lock_addr = SOTP_SER_PROG_ROW_1;
#else
		ret = bcm_iproc_sotp_set_temp_rdlock( row , num_of_rows);
		goto end_int_lock;
#endif						
	}
	/*
	 * We have 5 cases for calculating the Write lock value
	 * Case 1: Region < 7 Rows:[27-0], Lock bits:[13-0] Lock Row = 8
	 * Case 2: 7 < Region < 16 Rows:[63-28], Lock bits:[31-14] Lock Row = 8
	 * Case 3: 17 < Region < 19 Row:[64], Lock bits:[37-32] Lock Rows = 8
	 * Case 4: Region = 19 Row:[87-76],Lock bits:[43-38] Lock Rows = 8,9
	 * Case 5: 20 < Region <= 25 Rows:[111-88], Lock bits:[55-44]
	 *         Lock Row = 9
	 *
	 * Lock bits = 2 x Region
	 */

	sotp_row.data64 = 0;
	if (region < SOTP_SER_CONFIG_REGION) {
		/* Case 1 - All configuration sections         */
		/* Locking will be limited to one region only  */
		sotp_row.data32[0] = (SOTP_SER_WR_LOCK_BITS << (region * 2));
	} else if (region <= SOTP_SER_DWORD0_REGION) {
		/* Case 2 -
		 * Keys section, updating Lock register 8, Bits 31-0
		 */

		/*
		 * Each Key section has 3 regions including redundant rows.
		 * Locking will be performed on all the 3 regions.
		 */
		sotp_row.data32[0] = (SOTP_SER_WR_LOCK_BITS << (region * 2));
		sotp_row.data32[0] |= (SOTP_SER_WR_LOCK_BITS <<
					((region * 2) + SOTP_SER_KEY_REG_1));
		sotp_row.data32[0] |= (SOTP_SER_WR_LOCK_BITS <<
					((region * 2) + SOTP_SER_KEY_REG_2));
	} else if ((region > SOTP_SER_DWORD0_REGION) &&
					(region < SOTP_SER_DWORD1_REGION)) {
		/* Case 3
		 * Keys section, updating Lock register 8, Bits 37-32
		 */
		/* Get the region value for updating upper of 64bit data */
		region = region % SOTP_SER_REG_PER_LOCK_REG;
		sotp_row.data32[1] = (SOTP_SER_WR_LOCK_BITS << (region * 2));
		sotp_row.data32[1] |= (SOTP_SER_WR_LOCK_BITS <<
					((region * 2) + SOTP_SER_KEY_REG_1));
		sotp_row.data32[1] |= (SOTP_SER_WR_LOCK_BITS <<
					((region * 2) + SOTP_SER_KEY_REG_2));
	} else if (region == SOTP_SER_DWORD1_REGION) {
		/* Case 4 - Specical Case
		 * Key section, Updating Lock register 8 and 9, Bits 43-38
		 */
		/* Get the region value for updating upper of 64bit data */
		region = region % SOTP_SER_REG_PER_LOCK_REG;

		/* Update lock bits 39-38 for Rows 79-76 */
		sotp_row.data32[1] = (SOTP_SER_WR_LOCK_BITS << (region * 2));
		/* set lock bit 40 for Rows 81-80 */
		sotp_row.data32[1] |=
				(1 << ((region * 2) + SOTP_SER_KEY_REG_1));

		IMSG("Special lock value[%d]:%08X%08X\n", lock_addr,
							sotp_row.data32[1],
							sotp_row.data32[0]);
		bcm_iproc_sotp_mem_write( lock_addr, 0,
					 sotp_row.data64);

		lock_addr++;
		sotp_row.data64 = 0;

		/* set lock bit 41 (Bit 0 of Lock Row 9) for Rows 83-82 */
		region = 0;
		sotp_row.data32[0] = (1 << region);
		/* update lock bits 43-42 for Rows 87-84 */
		sotp_row.data32[0] |= (SOTP_SER_WR_LOCK_BITS <<
				((region * 2) + SOTP_SER_KEY_REG_0));

	} else if ((region > SOTP_SER_DWORD1_REGION) &&
					(region <= SOTP_SER_MAX_REGION)) {
		/* Case 5
		 * Keys section, updating Lock register 9. Bits 14-1
		 */
		uint32_t mod_region = (region * 2) % SOTP_SER_LOCK_BIT_SIZE;

		sotp_row.data32[0] = (SOTP_SER_WR_LOCK_BITS << mod_region);
		sotp_row.data32[0] |= (SOTP_SER_WR_LOCK_BITS <<
					(mod_region + SOTP_SER_KEY_REG_1));
		sotp_row.data32[0] |= (SOTP_SER_WR_LOCK_BITS <<
					(mod_region + SOTP_SER_KEY_REG_2));
	} else {
		EMSG("Error invalid region specified.\n");
		ret = SOTP_SER_INVALID_ARGS;
		goto end_int_lock;
	}

	ret = SOTP_SER_OK;
	bcm_iproc_sotp_mem_write( lock_addr, 0,
				sotp_row.data64);
	IMSG("Region:%x, lock value[%d]:%08X%08X\n",
			region,
			lock_addr,
			sotp_row.data32[1],
			sotp_row.data32[0]);

end_int_lock:
	return ret;
}
