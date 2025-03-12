/******************************************************************************

   Copyright 2023-2024 MaxLinear, Inc.

   For licensing information, see the file 'LICENSE' in the root folder of
   this software module.

******************************************************************************/

#include "host_smdio_ssb.h"
#include "host_adapt.h"

#include "smdio_access.h"

#include <unistd.h>
#include <stdlib.h>

#define SB_PDI_CTRL 0xE100
#define SB_PDI_ADDR 0xE101
#define SB_PDI_DATA 0xE102
#define SB_PDI_STAT 0xE103

#define SB1_ADDR 0x7800
#define SB_PDI_CTRL_RD 0x01
#define SB_PDI_CTRL_WR 0x02
#define SB_PDI_RST 0x0

#define FW_DL_MDIO_START_MAGIC	0xF48F
#define FW_DL_MDIO_RDY_MAGIC	0xC55C
#define RESCUE_IMAGE_HEADER_SIZE 20

#define lif_id 0
#define SMDIO_PHY_NR  8
#define M_SLEEP(x)    usleep((x)*1000)


struct host_smdio_ssb_ops {
	const GSW_Device_t *pdev;

	int (*smdio_write)(const GSW_Device_t *pdev, uint16_t phy_reg, uint16_t phy_reg_data);
	int (*smdio_cont_write)(const GSW_Device_t *pdev, uint16_t phy_reg, uint16_t phy_reg_data[8], uint8_t num);
	int (*smdio_read)(const GSW_Device_t *pdev, uint16_t phy_reg);
};

static struct host_smdio_ssb_ops host_smdio_ops;
static GSW_Device_t gsw_dev = {0};


static int gsw_smdio_read(const GSW_Device_t *pdev, uint16_t phy_reg)
{
	return smdio_read(lif_id, ((GSW_Device_t *)(pdev))->smdio_phy_addr, phy_reg);
}

static int gsw_smdio_write(const GSW_Device_t *pdev, uint16_t phy_reg, uint16_t phy_reg_data)
{
	return smdio_write(lif_id, ((GSW_Device_t *)(pdev))->smdio_phy_addr, phy_reg, phy_reg_data);
}

static int gsw_smdio_cont_write(const GSW_Device_t *pdev, uint16_t phy_reg, uint16_t phy_reg_data[8], uint8_t num)
{
	return smdio_cont_write(lif_id, ((GSW_Device_t *)(pdev))->smdio_phy_addr, phy_reg, phy_reg_data, num);
}


/**
 * Initialize smdio_ssb_ops operation
 */
#define slif_lib "bcm2835"
void host_smdio_ssb_ops_init(const void *pdev)
{
	api_gsw_get_links(slif_lib);
	pdev = gsw_get_struc(lif_id,0);

	host_smdio_ops.pdev = pdev;
	host_smdio_ops.smdio_read = gsw_smdio_read;
	host_smdio_ops.smdio_write = gsw_smdio_write;
	host_smdio_ops.smdio_cont_write = gsw_smdio_cont_write;
}

/**
 * Uninitialize host_smdio_ssb_ops operation
 */
void host_smdio_ssb_ops_uninit()
{
	host_smdio_ops.pdev = NULL;
	host_smdio_ops.smdio_read = NULL;
	host_smdio_ops.smdio_write = NULL;
	host_smdio_ops.smdio_cont_write = NULL;

}

/**
 * Reset SB PDI registers
 */
static void smdio_ssb_pdi_reset()
{
	host_smdio_ops.smdio_write(host_smdio_ops.pdev, SB_PDI_CTRL, SB_PDI_RST);
	host_smdio_ops.smdio_write(host_smdio_ops.pdev, SB_PDI_ADDR, SB_PDI_RST);
	host_smdio_ops.smdio_write(host_smdio_ops.pdev, SB_PDI_DATA, SB_PDI_RST);
}

static bool smdio_ssb_wait_pdi_stat_is_expected(uint16_t exp_val, uint32_t timeout_ms)
{
	int loop = timeout_ms / 10;
	uint16_t sb_pdi_stat;

	while (loop > 0) {
		sb_pdi_stat = host_smdio_ops.smdio_read(host_smdio_ops.pdev, SB_PDI_STAT);
		if (sb_pdi_stat == exp_val)
			return true;
		M_SLEEP(10);
		loop -= 1;
	}
	return false;
}

static int smdio_ssb_wait_pdi_stat_not_expected(uint16_t exp_val, uint16_t *ret_val, uint32_t timeout_ms)
{
	int loop = timeout_ms / 10;
	uint16_t sb_pdi_stat ;

	while (loop > 0) {
		sb_pdi_stat = host_smdio_ops.smdio_read(host_smdio_ops.pdev, SB_PDI_STAT);
		if (sb_pdi_stat != exp_val) {
			*ret_val = sb_pdi_stat;
			return 0;
		}
		M_SLEEP(10);
		loop -= 1;
	}
	return -1;
}

/**
 * Write image data to target which in rescue mode
 * Image start with 4 bytes of image type
 * followed by 4 bytes of image size and 4 bytes
 * of checksum
 *
 * pdata - data pointer to be writen to SB
 *
 * return size of data writen to target if successful
 */
int host_smdio_ssb_rescue_download(uint8_t *pdata, uint32_t timeout_ms)
{
	uint32_t word_idx = 0;
	uint32_t idx = 0;
	uint16_t data_arr[8] = {0};
	uint8_t num = 0;
	uint16_t sb_pdi_stat = FW_DL_MDIO_START_MAGIC;
	uint32_t image_type, image_size_1, image_checksum_1, image_size_2, image_checksum_2, data_size, full_image_size;
	uint16_t *pimage_header;
	int rc = 0;

	if (!pdata) {
		printf("Data can not be NULL\n");
		return -EINVAL;
	}

	//Initialize SMDIO and SSB PDI
	smdio_ssb_pdi_reset();

	//Wait for target to be ready for download
	if (smdio_ssb_wait_pdi_stat_is_expected(FW_DL_MDIO_RDY_MAGIC, 3000)) {
		printf("Target is ready for downloading.\n");
	} else {
		//if timeout, here we can not return timeout code, for compatibility, we still continue
		((GSW_Device_t *)(host_smdio_ops.pdev))->smdio_phy_addr = 0x1F;
	}

	//Send START signal to target which is rescue mode
	host_smdio_ops.smdio_write(host_smdio_ops.pdev, SB_PDI_STAT, sb_pdi_stat);

	sb_pdi_stat += 1;
	if (smdio_ssb_wait_pdi_stat_is_expected(sb_pdi_stat, timeout_ms)) {
		printf("Received START ACK from target, starting...\n");
	} else
		return -ETIMEDOUT;

	sb_pdi_stat = host_smdio_ops.smdio_read(host_smdio_ops.pdev, SB_PDI_STAT);

	image_type = *(uint32_t *)pdata;
	image_size_1 = *((uint32_t *)pdata + 1);
	image_checksum_1 = *((uint32_t *)pdata + 2);
	image_size_2 = *((uint32_t *)pdata + 3);
	image_checksum_2 = *((uint32_t *)pdata + 4);
	printf("image type: %x, size 1: %x, checksum 1: %x, size 2: %x, checksum 2: %x\n", image_type, image_size_1, image_checksum_1, image_size_2, image_checksum_2);

	// send image header (20 bytes)
	pimage_header = (uint16_t *)pdata;
	//Trigger write
	host_smdio_ops.smdio_write(host_smdio_ops.pdev, SB_PDI_CTRL, SB_PDI_CTRL_WR);
	host_smdio_ops.smdio_cont_write(host_smdio_ops.pdev, SB_PDI_DATA, pimage_header, 8);
	host_smdio_ops.smdio_cont_write(host_smdio_ops.pdev, SB_PDI_DATA, pimage_header + 8, 2);
	smdio_ssb_pdi_reset();
	host_smdio_ops.smdio_write(host_smdio_ops.pdev, SB_PDI_STAT, RESCUE_IMAGE_HEADER_SIZE);

	sb_pdi_stat = RESCUE_IMAGE_HEADER_SIZE + 1;
	if (smdio_ssb_wait_pdi_stat_is_expected(sb_pdi_stat, timeout_ms)) {
		printf("Received ACK of image header from target\n");
	} else
		return -ETIMEDOUT;

	pdata += RESCUE_IMAGE_HEADER_SIZE; // skip image headers
	data_size = 0;
	full_image_size = image_size_1 + image_size_2;

	printf("Erase flash\n");
	fflush(stdout);

	if (smdio_ssb_wait_pdi_stat_is_expected(0, timeout_ms)) {

		printf("Program flash\n");
		fflush(stdout);

		//Trigger write
		host_smdio_ops.smdio_write(host_smdio_ops.pdev, SB_PDI_CTRL, SB_PDI_CTRL_WR);
		while (idx < full_image_size) {
			num = 0;
			do {
				//16 bits data
				uint16_t fdata = 0x0;
				if (idx + 1 < full_image_size) {
					fdata = ((pdata[idx + 1]) << 8) | pdata[idx];
					idx += 2;
					data_size += 2;
				} else if (idx < full_image_size) { // last byte of data, padding high 8bits with 0s
					fdata |= (uint16_t)pdata[idx];
					idx++;
					data_size++;
				} else {  // no more data
					break;
				}
				data_arr[num] = fdata;
				num++;
			} while (num < 8);
			host_smdio_ops.smdio_cont_write(host_smdio_ops.pdev, SB_PDI_DATA, data_arr, num);

			word_idx += num;
			// check download is completed?
			if (idx >= full_image_size) {
				// Send data size to target MCUBoot
				smdio_ssb_pdi_reset();
				host_smdio_ops.smdio_write(host_smdio_ops.pdev, SB_PDI_STAT, data_size); // last batch data
				break;
			}

			if (word_idx == 16384) { // 32KB is done, need to set SB PDI addr to 0x7800
				printf(".");
				fflush(stdout);
				host_smdio_ops.smdio_write(host_smdio_ops.pdev, SB_PDI_CTRL, SB_PDI_RST);
				host_smdio_ops.smdio_write(host_smdio_ops.pdev, SB_PDI_ADDR, SB1_ADDR);
				// Continue to write SB1 32KB
				host_smdio_ops.smdio_write(host_smdio_ops.pdev, SB_PDI_CTRL, SB_PDI_CTRL_WR);
			} else if (word_idx == 32760) { // One slice is done: 32768 - 8 Word
				smdio_ssb_pdi_reset();
				printf(".");
				fflush(stdout);

				// Send data size to target MCUBoot
				host_smdio_ops.smdio_write(host_smdio_ops.pdev, SB_PDI_STAT, data_size); //64KB - 16B
				word_idx = 0;
				data_size = 0;
				// Paused here to wait for target MCUBoot to program flash, and then continue
				if (smdio_ssb_wait_pdi_stat_is_expected(0, timeout_ms)) {
					//Trigger write
					host_smdio_ops.smdio_write(host_smdio_ops.pdev, SB_PDI_CTRL, SB_PDI_CTRL_WR);
				} else
					return -ETIMEDOUT;
			}
		}
	} else
		return -ETIMEDOUT;

	rc = smdio_ssb_wait_pdi_stat_not_expected(data_size, &sb_pdi_stat, timeout_ms);
	/* write to avoid slave hang */
	host_smdio_ops.smdio_write(host_smdio_ops.pdev, SB_PDI_STAT, 0x3CC3);
	if (rc == 0 && sb_pdi_stat == 0) {
		printf("\nsuccessfully download firmware to target\n");
		return idx;
	} else {
		printf("\nfailed download firmware to target\n");
		return rc;
	}
}

int ssb_load(char *fw_path)
{
	int ret,i;
	FILE *fwin;
	int filesize;
	uint8_t *pDataBuf;

	host_smdio_ssb_ops_init(&gsw_dev);
	
	fwin = fopen(fw_path, "rb");
	if (fwin == NULL) {
		printf("Failed to open FW file \"%s\".\n", fw_path);
		return -errno;
	}
	fseek(fwin, 0L, SEEK_END);
	filesize = ftell(fwin);
	printf("FW size: %d bytes\n", filesize);
	rewind(fwin);

	pDataBuf = (uint8_t*)malloc(filesize);
	if (pDataBuf == NULL) {
		printf("malloc: Unable to allocate memory.\n");
		ret = fclose(fwin);
		return -errno;
	}

	ret = fread(pDataBuf, 1, filesize, fwin);
	if (ret != filesize)
	{
		free(pDataBuf);
		ret = fclose(fwin);
		return -errno;
	}
	fclose(fwin);

	ret = host_smdio_ssb_rescue_download((uint8_t *) pDataBuf, 70 * 1000);
	free(pDataBuf);
	if ((ret + RESCUE_IMAGE_HEADER_SIZE) != filesize)
	{
		printf("FW Upload Failed - FW Write Failed\n");
		return -errno;
	}

	//Wait for PHY to start
	sleep(5);
	ret = check_registers();
	if (ret != 0)
	{
		printf("FW Upload Failed - Register Read Failed\n");
		return -errno;
	}

	printf("FW Upload Sucessful\n");
	return 0;
}

/**
 * Check ID and FW registers after FW Download
 */
int check_registers()
{
	int ret,i;
	struct mdio_relay_data param = {0};

	for ( i = 0; i < SMDIO_PHY_NR; i++ )
	{
		param.phy = (uint8_t)i;
		param.mmd = (uint8_t)0x0;
		param.reg = (uint16_t)0x3;
		if ( (ret=int_gphy_read(host_smdio_ops.pdev, &param) != 0) || (param.data == 0xffff) )
			return -1;

		param.reg = (uint16_t)0x1e;
		if ( (ret=int_gphy_read(host_smdio_ops.pdev, &param) != 0) || (param.data == 0xffff) )
			return -1;
	}
	return 0;
}