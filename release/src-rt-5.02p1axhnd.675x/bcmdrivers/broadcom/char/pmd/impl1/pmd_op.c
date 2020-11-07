/*---------------------------------------------------------------------------

<:copyright-BRCM:2013:proprietary:standard 

   Copyright (c) 2013 Broadcom 
   All Rights Reserved

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
 ------------------------------------------------------------------------- */

#include <linux/module.h>
#include <linux/kernel.h>
//#include <linux/types.h>
#include <linux/path.h>
#include <linux/namei.h>
#include <linux/fs.h>
#include <linux/slab.h>  /* kzalloc() */
#include <bcm_OS_Deps.h>
#include <linux/bcm_log.h>
#include "ru.h"
#include "ld_lia.h"
#include "blocks.h"
#include "pmd_op.h"

#include <bcmsfp_i2c.h>
#include "pmd_cal.h"
#include "pmd.h"
#include "dev_id.h"
#include "pmd_dump_data.h"
#include "pmd_temp_cal.h"
#include "pmd_msg.h"
#include "otp.h"
#include "opticaldet.h"

#define PMD_REG_FILE_PATH       "/data/pmd_reg" /*keeps the register mapping */
#define PMD_DUMP_DATA_FILE_PATH "/data/pmd_dump_data" /*keeps the pmd dump data*/
#define PMD_FW_IMG_FILE_PATH    "/etc/pmd_fw_img.bin"
#define SW_DOWNLOAD_CHUNK       16
#define MAX_READ_SIZE           24
#define MIN_TRANSACTION_SIZE    4
#define OTP_SECURE_BOOT_BIT     11

#define PMD_CLIENT_NUM 5
const char* client_string[PMD_CLIENT_NUM] = {"REG", "IRAM", "DRAM", "CAL", "TEMP2APD"};

uint16_t pmd_temp_apd_conv[APD_TEMP_TABLE_SIZE] = {0};

/* SFF-8472 address map */
uint8_t A0h[256], A2h[256];

/* I2C internal addressing */
#define CSR_ADDR 0x7
#define CONFIG_REG_WRITE   0x0
#define CONFIG_REG_READ    0x1

/* B0 PMA space address */
#define PMA_ADDR_REG 0x0    /* 2b'00 - reg */
#define PMA_ADDR_IRAM 0x2   /* 2b'10 - iram */
#define PMA_ADDR_DRAM 0x3   /* 2b'11 - dram */

#define TEMP_LEN_STEP (SW_DOWNLOAD_CHUNK/sizeof(uint32_t))
#define APD_TEMP_LEN_STEP (SW_DOWNLOAD_CHUNK/sizeof(uint16_t))

/* Check if given offset is valid or not */
static inline int check_dword_offset(uint32_t offset)
{
    if (offset & 3) {
        BCM_LOG_ERROR(BCM_LOG_ID_PMD, "Invalid offset. Should be 4 byte aligned \n");
        return -1;
    }
    return 0;
}

static uint32_t file_checksum(const char *file, int filelen)
{
    uint32_t checksum = 0;
    uint16_t *ptr = (uint16_t *)file;
    int len = filelen;

    while (len > 0)
    {
        if (len == 1)
            checksum += file[filelen-1];
        else
            checksum += *ptr;
        ptr++;
        len -= 2;
    }
#ifdef PMD_FULL_DEBUG
    BCM_LOG_DEBUG(BCM_LOG_ID_PMD, "file checksum %d \n", checksum);
#endif
    return checksum;
}


int pmd_op_set_output_buf(char *in_buf, char * out_buf, uint16_t len)
{
    uint16_t tmp_len = len + (len >> 2);
    int i, j = 0;

    for (i = 0; i < tmp_len; i++)
    {
        if (i % 5 == 4)
            continue;
        out_buf[j] = in_buf[i];
        j++;
    }

    return 0;
}

/******************************************************************************
 * Register direct i2c access functions
  ******************************************************************************/
int pmd_op_i2c(pmd_dev_client client, uint16_t offset, unsigned char *buf, uint16_t len, int read_op)
{
    unsigned char * i2c_buf;
    uint16_t length_with_redundancy = 0;
    uint16_t length_to_buf = 0;
    uint16_t num_of_transaction, len_mod_max_read_size, i, offset_in_buf;
    bool error_flag = 0;
    int rc = 0, i2c_bus = 0;
#ifdef PMD_FULL_DEBUG
    BCM_LOG_DEBUG(BCM_LOG_ID_PMD, "%s client %s offset %x in length %d \n",
            read_op ? "read" : "write", client_string[client], offset, len);
#endif

    if( opticaldet_get_xpon_i2c_bus_num(&i2c_bus) != OPTICALDET_SUCCESS )
        return -1;

    /*pad to 4 byte  (len % 24)  max transacion size is 24 byte evry transacion must read minimum 4 byte*/
    if (read_op && ((len % MAX_READ_SIZE) < MIN_TRANSACTION_SIZE) && (len % MAX_READ_SIZE != 0))
        len = len + 4 - (len % MAX_READ_SIZE);/*pad to 4 byte*/

    if (check_dword_offset((uint32_t)offset))
        return -1;

    if (len > MAX_READ_SIZE)
    	i2c_buf = kzalloc(MAX_READ_SIZE + (MAX_READ_SIZE >> 2) - 1 + PMD_I2C_HEADER, GFP_KERNEL);
    else
        i2c_buf = kzalloc(len + (len >> 2) - 1 + PMD_I2C_HEADER, GFP_KERNEL);
    if (!i2c_buf)
        return -1;

    i2c_buf[0] = CSR_ADDR;

    /* register offset */
#ifdef __LITTLE_ENDIAN
    i2c_buf[4] = (uint8_t)(offset >> 8);
    i2c_buf[5] = (uint8_t)offset;
#else
    memcpy(i2c_buf + 4, &offset, 2);
#endif

    if (client == pmd_iram_map)
        i2c_buf[3] = PMA_ADDR_IRAM;
    else if (client == pmd_dram_map)
        i2c_buf[3] = PMA_ADDR_DRAM;
    else
        i2c_buf[3] = PMA_ADDR_REG;

    if (read_op)
    {
        num_of_transaction = (len / MAX_READ_SIZE) + 1;
        len_mod_max_read_size = len % MAX_READ_SIZE;
        for (i = 0; i < num_of_transaction ; i++)
        {
        	i2c_buf[1] = CONFIG_REG_READ;
        	if (i != 0)
            {
        		i2c_buf[0] = CSR_ADDR;
        		if (client == pmd_iram_map)
            	    i2c_buf[3] = PMA_ADDR_IRAM;
            	else if (client == pmd_dram_map)
            	    i2c_buf[3] = PMA_ADDR_DRAM;
            	else
            	    i2c_buf[3] = PMA_ADDR_REG;

        		offset_in_buf = offset +  i * MAX_READ_SIZE;
#ifdef __LITTLE_ENDIAN
                i2c_buf[4] = (uint8_t)(offset_in_buf >> 8);
                i2c_buf[5] = (uint8_t)offset_in_buf;
#else
                memcpy(i2c_buf + 4, &offset_in_buf, 2);
#endif

            }
        	if (i == (num_of_transaction - 1))/*last iteration*/
            {
            	length_with_redundancy = len_mod_max_read_size + (len_mod_max_read_size >> 2) - 1;
                length_to_buf = len_mod_max_read_size;
            }
            else
            {
            	length_with_redundancy = MAX_TRANSACTION_SIZE;
                length_to_buf = MAX_READ_SIZE;
            }

            if (length_to_buf == 0)
                break;

            rc = bcmsfp_read(i2c_bus, client, i2c_buf, length_with_redundancy);
            if (rc != length_with_redundancy)
            {
                error_flag = 1;
                break;
            }
            pmd_op_set_output_buf(i2c_buf, buf + i * MAX_READ_SIZE, length_to_buf);
        }
    }
    else
    {
        i2c_buf[1] = CONFIG_REG_WRITE;
        length_with_redundancy = len + PMD_I2C_HEADER;

        /* copy the data itself */
        memcpy(i2c_buf + PMD_I2C_HEADER, buf, len);
        rc = bcmsfp_write(i2c_bus, client, i2c_buf, length_with_redundancy) ;
        if(rc != length_with_redundancy)
            error_flag = 1;
    }
    kfree(i2c_buf);
    if (error_flag)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_PMD, "Error in %s operation to PMD. rc = %d\n", (read_op ? "read" : "write"), rc);
        return -1;
    }
    return 0;
}

int ru_reg_write(uint16_t addr, uint32_t val)
{
#ifdef __LITTLE_ENDIAN
    val = swab32(val);
#endif
    return pmd_op_i2c(pmd_reg_map, addr, (unsigned char *)&val, 4, PMD_WRITE_OP);
}

int ru_reg_read(uint16_t addr, uint32_t * val)
{
    int rc;
    rc = pmd_op_i2c(pmd_reg_map, addr, (unsigned char *)val, 4, PMD_READ_OP);
#ifdef __LITTLE_ENDIAN
    *val = swab32(*val);
#endif
    return rc;
}

int pmd_op_file(pmd_dev_client client, unsigned char * buf, int len, bool read_op)
{
    struct file *fp = NULL;
    mm_segment_t fs;
    int rc = 0;
    int flags, mode = 0;

#ifdef PMD_FULL_DEBUG
    BCM_LOG_DEBUG(BCM_LOG_ID_PMD, "client %s %s operation in length %d \n", client_string[client], (read_op ? "read" : "write"), len);
#endif
    if (read_op)
        flags = O_RDONLY;
    else
    {
        flags = O_RDWR | O_TRUNC | O_CREAT;
        mode = S_IRUSR | S_IWUSR;
    }

    fs = get_fs();
    set_fs(get_ds());

    switch (client)
    {
    case pmd_reg_map:
        fp = filp_open(PMD_REG_FILE_PATH, flags, mode);
        break;
    case pmd_iram_map:
        fp = filp_open(PMD_FW_IMG_FILE_PATH, flags, mode);
        break;
    case pmd_dump_data_map:
        fp = filp_open(PMD_DUMP_DATA_FILE_PATH, flags, mode);
        break;
    default:
    	BCM_LOG_ERROR(BCM_LOG_ID_PMD, "client is out of range \n");
        return -1;
    }

    if (IS_ERR(fp))
    {
    	BCM_LOG_DEBUG(BCM_LOG_ID_PMD, "Failed to open %s file. Error = %ld\n", client_string[client], PTR_ERR(fp));
        return -1;
    }

    if (read_op)
    {
        memset(buf, 0x00, len);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)
        if (fp->f_op && fp->f_op->read)
#endif
        {
            fp->f_pos = 0;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,0,0))
            if ((rc = (int)vfs_read(fp, (void *) buf, len, &fp->f_pos)) <= 0)
#else
            if ((rc = (int)fp->f_op->read(fp, (void *) buf, len, &fp->f_pos)) <= 0)
#endif
            {
            	BCM_LOG_ERROR(BCM_LOG_ID_PMD, "Failed to read from %s\n", client_string[client]);
            }
        }
    }
    else
    {
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)
        if (fp->f_op && fp->f_op->write)
#endif
        {
            fp->f_pos = 0;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,0,0))
            if((rc = (int) vfs_write(fp, (void *) buf, len, &fp->f_pos)) == len)
#else
            if ((rc = (int) fp->f_op->write(fp, (void *) buf, len, &fp->f_pos)) == len)
#endif

            {
            	BCM_LOG_DEBUG(BCM_LOG_ID_PMD, "len %d write %d \n", len, rc);
                vfs_fsync(fp, 0);
            }
            else
            	BCM_LOG_ERROR(BCM_LOG_ID_PMD, "Failed to write to %s\n", client_string[client]);
        }
    }

    filp_close(fp, NULL);
    if (rc != len)
    {
    	BCM_LOG_ERROR(BCM_LOG_ID_PMD, "%s operation performed on only %d out of %d asked \n",
                client_string[client], rc, len);
        rc = -1;
    }
    else
        rc = 0;

    set_fs(fs);
    return rc;
}

int pmd_op_get_file_len(pmd_dev_client client, uint16_t * len)
{
    struct path p;
    struct kstat ks;
    const char * file;

    switch (client)
    {
    case pmd_reg_map:
        file = PMD_REG_FILE_PATH;
        break;
    case pmd_iram_map:
        file = PMD_FW_IMG_FILE_PATH;
        break;
    default:
        BCM_LOG_DEBUG(BCM_LOG_ID_PMD, "client is out of range \n");
        return -1;
    }

    if (kern_path(file, LOOKUP_FOLLOW, &p))
    {
        BCM_LOG_DEBUG(BCM_LOG_ID_PMD, "Fail to open %s \n", client_string[client]);
        return -1;
    }
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0)
    vfs_getattr(&p, &ks);
#else
    vfs_getattr(p.mnt, p.dentry, &ks);
#endif
    *len = ks.size;
#ifdef PMD_FULL_DEBUG
    BCM_LOG_DEBUG(BCM_LOG_ID_PMD, "File %s len = %d \n",file, *len);
#endif
    return 0;
}

int pmd_op_temp_conv_table_download(const uint32_t *const_ptr)
{
    int rc;
    int i;
    uint32_t cksum = 0;
    uint32_t tmp = 0;
    uint32_t fwcksum = 0;
    uint16_t offset = 0;
    int loop = 0;
    uint16_t buf_len = 0;
    uint16_t reserved_len = TEMP_TABLE_SIZE;
    uint32_t ptr[TEMP_TABLE_SIZE];

    memcpy(ptr, const_ptr, sizeof(uint32_t) * TEMP_TABLE_SIZE);

    tmp = TEMP_TABLE_SIZE;
    cksum += file_checksum(((char *)&tmp), sizeof(uint32_t));
#ifdef __LITTLE_ENDIAN
    tmp = swab32(tmp);
#endif
    rc = pmd_op_i2c(pmd_dram_map, TEMP_TABLE_ADDR, ((char *)&tmp), sizeof(uint32_t), PMD_WRITE_OP);
    if (rc)
        goto exit;

    tmp = TEMP_TABLE_LOWEST_TEMP;
    cksum += file_checksum(((char *)&tmp), sizeof(uint32_t));
#ifdef __LITTLE_ENDIAN
    tmp = swab32(tmp);
#endif
    rc = pmd_op_i2c(pmd_dram_map, TEMP_TABLE_ADDR + sizeof(uint32_t), ((char *)&tmp), sizeof(uint32_t), PMD_WRITE_OP);
    if (rc)
        goto exit;

    cksum += file_checksum(((char *)(ptr)), sizeof(uint32_t)* TEMP_TABLE_SIZE);

#ifdef __LITTLE_ENDIAN
    for (i = 0; i < TEMP_TABLE_SIZE; i++)
    {
    	*(ptr+i) = swab32(*(ptr+i));
    }
#endif

    loop = (TEMP_TABLE_SIZE % TEMP_LEN_STEP)? ((TEMP_TABLE_SIZE/TEMP_LEN_STEP) + 1): \
                (TEMP_TABLE_SIZE/TEMP_LEN_STEP);

    for (i = 0; i < loop; i++)
    {
        offset = TEMP_TABLE_ADDR + 2 * sizeof(uint32_t) + i * SW_DOWNLOAD_CHUNK;
        buf_len = SW_DOWNLOAD_CHUNK;
        if (reserved_len < TEMP_LEN_STEP)
        {
            buf_len = reserved_len * sizeof(uint32_t);
        }
        else
        {
            reserved_len -= TEMP_LEN_STEP;
        }

        rc = pmd_op_i2c(pmd_dram_map, offset, (char *)(ptr + i * TEMP_LEN_STEP), buf_len, PMD_WRITE_OP);
        if (rc)
            goto exit;
    }

    rc = pmd_msg_handler(hmid_temp_table_checksum_get, (uint16_t *)&fwcksum, sizeof(uint32_t));
    if (rc)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_PMD, " Error get temperature table checksum from PMD \n");
        goto exit;
    }

#ifdef __LITTLE_ENDIAN
    fwcksum = (fwcksum & 0x0000ffffUL) << 16 | (fwcksum & 0xffff0000UL) >> 16;
#endif

    if (fwcksum != cksum)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_PMD, " Temperature table checksum error. fwcksum=%x, cksum=%x \n", fwcksum, cksum);
        rc = -1;
    }

exit:
    return rc;
}

int pmd_op_temp_apd_conv_table_download(const uint16_t *ptr)
{
    int rc;
    int i;
    uint32_t tmp = 0;
    uint16_t offset = 0;
    int loop = 0;
    uint16_t buf_len = 0;
    uint16_t reserved_len = APD_TEMP_TABLE_SIZE;

    tmp = APD_TEMP_TABLE_SIZE;
#ifdef __LITTLE_ENDIAN
    tmp = swab32(tmp);
#endif
    rc = pmd_op_i2c(pmd_dram_map, APD_TEMP_TABLE_ADDR, ((char *)&tmp), sizeof(uint32_t), PMD_WRITE_OP);
    if (rc)
        goto exit;
    
    tmp = APD_TEMP_TABLE_LOWEST_TEMP;
#ifdef __LITTLE_ENDIAN
    tmp = swab32(tmp);
#endif
    rc = pmd_op_i2c(pmd_dram_map, APD_TEMP_TABLE_ADDR + sizeof(uint32_t), ((char *)&tmp), sizeof(uint32_t),
        PMD_WRITE_OP);
    if (rc)
        goto exit;

    tmp = 1;
#ifdef __LITTLE_ENDIAN
    tmp = swab32(tmp);
#endif
    rc = pmd_op_i2c(pmd_dram_map, APD_TEMP_TABLE_ADDR + (sizeof(uint32_t) * 2), ((char *)&tmp), sizeof(uint32_t),
        PMD_WRITE_OP);
    if (rc)
        goto exit;

    loop = (APD_TEMP_TABLE_SIZE % APD_TEMP_LEN_STEP)? ((APD_TEMP_TABLE_SIZE/APD_TEMP_LEN_STEP) + 1): \
                (APD_TEMP_TABLE_SIZE/APD_TEMP_LEN_STEP);

    for (i = 0; i < loop; i++)
    {
        offset = APD_TEMP_TABLE_ADDR + 3 * sizeof(uint32_t) + i * SW_DOWNLOAD_CHUNK;
        buf_len = SW_DOWNLOAD_CHUNK;
        if (reserved_len < TEMP_LEN_STEP)
        {
            buf_len = reserved_len * sizeof(uint32_t);
        }
        else
        {
            reserved_len -= TEMP_LEN_STEP;
        }
        
        rc = pmd_op_i2c(pmd_dram_map, offset, (char *)(ptr + i * APD_TEMP_LEN_STEP), buf_len, PMD_WRITE_OP);
        if (rc)
            goto exit;
    }

exit:
    return rc;
}


int pmd_op_sw_download(uint32_t *crc)
{
    unsigned char* file;
    int rc;
    uint16_t len;
    int i, loops, remainder, alloc_len;
    uint32_t otp_val;

    rc = RU_REG_READ(OTP, OTP_CONFIG_DIRECT_1, &otp_val);
    if(((otp_val >> OTP_SECURE_BOOT_BIT) & 1) == 0)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_PMD, "\nPMD ERROR - Device doesn't support secure boot mode. Initialization halted.\n");
        return -1;
    }

    rc |= pmd_op_get_file_len(pmd_iram_map, &len);
    if (rc)
    {
    	printk("\npmd_op_get_file_len failed\n");
        return rc;
    }

    alloc_len = len - 4;
    /* Padding to dword */
    if (len % 4)
        alloc_len = len + (4 -len % 4);

    file = kzalloc(alloc_len, GFP_KERNEL);
    if (!file)
    {
    	printk("\nkzalloc failed\n");
        return -1;
    }

    rc = pmd_op_file(pmd_iram_map, file , len, PMD_READ_OP);
    if (rc)
    {
    	printk("\npmd_op_file failed\n");
    	goto exit;
    }

    RU_REG_WRITE(LD_LIA,LD_LIA_PARAM,alloc_len);
    *crc = *(uint32_t*) (file+alloc_len);
    loops = alloc_len /SW_DOWNLOAD_CHUNK;
    remainder = alloc_len % SW_DOWNLOAD_CHUNK;

    for (i = 0; i < loops + 1; i++)
    {
        int chunk = (i == loops) ? remainder : SW_DOWNLOAD_CHUNK;
        int chunk_offset = i * SW_DOWNLOAD_CHUNK;

        if (i == loops && !remainder)
            break;

        rc |= pmd_op_i2c(pmd_iram_map, chunk_offset, file + chunk_offset, chunk, PMD_WRITE_OP);
    }
exit:
    kfree(file);
    return rc;
}

int pmd_dump_data()
{
    int i;
    const char * hexstring = "0123456789ABCDEF";
	int rc = 0;
    int size = 0;
    int index = 0;
    uint8_t buf[PMD_BUF_MAX_SIZE];
    char *data, *hex;
    int count;
    pmd_dumpdata_struct pmd_dumpdata[] =
    {
            {.name = "pin_mux", .addres = PIN_MUX_ADDR, .len = PIN_MUX_SIZE, pmd_reg_map},
            {.name = "otp", .addres = OTP_ADDR, .len = OTP_SIZE, pmd_reg_map},
            {.name = "misc", .addres = MISC_ADDR, .len = MISC_SIZE, pmd_reg_map},
            {.name = "bcl", .addres = BSL_ADDR, .len = BSL_SIZE, pmd_reg_map},
            {.name = "crt", .addres = CRT_ADDR, .len = CRT_SIZE, pmd_reg_map},
            {.name = "adf", .addres = ADF_ADDR, .len = ADF_SIZE, pmd_reg_map},
            {.name = "apd_adc", .addres = APD_ADC_ADDR, .len = APD_ADC_SIZE, pmd_reg_map},
            {.name = "ld_lia", .addres = LD_LIA_ADDR, .len = LD_LIA_SIZE ,pmd_reg_map},
            {.name = "esc", .addres = ESC_ADDR, .len = ESC_SIZE, pmd_reg_map},
            {.name = "ldc", .addres = LDC_ADDR, .len = LDC_SIZE, pmd_reg_map},
            {.name = "lrl", .addres = LRL_ADDR, .len = LRL_SIZE ,pmd_reg_map},
            {.name = "tmod", .addres = TMON_ADDR, .len = TMON_SIZE, pmd_reg_map},

            {.name = "pmd_debug", .addres = PMD_DEBUG_ADDR, .len = PMD_DEBUG_SIZE, pmd_dram_map},
			{.name = "pmd_config_rec_1", .addres = PMD_CONFIG_REC_ADDR, .len = PMD_BUF_MAX_SIZE, pmd_dram_map},/*divide pmd config to 2, reg read is limit to 300 byte*/
            {.name = "pmd_config_rec_2", .addres = PMD_CONFIG_REC_ADDR + PMD_BUF_MAX_SIZE, .len = (PMD_CONFIG_REC_SIZE - 300), pmd_dram_map},
            {.name = "pmd_stat_rec", .addres = PMD_STAT_REC_ADDR, .len = PMD_STAT_REC_SIZE, pmd_dram_map}
    };

    for(count = 0; count < (NUM_OF_REG_MODULS + NUM_OF_STRUCTS) ; count++)
        size += pmd_dumpdata[count].len ;

    data = kzalloc(size, GFP_KERNEL);
    hex = kzalloc(size * 2, GFP_KERNEL);
    if (!data || ! hex)
        return -1;

    for(count = 0; count < (NUM_OF_REG_MODULS + NUM_OF_STRUCTS); count++)
    {
        rc = pmd_op_i2c(pmd_dumpdata[count].client, pmd_dumpdata[count].addres, buf, pmd_dumpdata[count].len, PMD_READ_OP);
        if (rc)
            goto exit;

        memcpy(data + index, buf, pmd_dumpdata[count].len);
        index += pmd_dumpdata[count].len;
    }

    for(i = 0; i < size; i++)
    {
        hex[2 * i] = hexstring[(data[i] >> 4) & 0xF];
        hex[2 * i + 1] = hexstring[data[i] & 0xF];
    }

    rc = pmd_op_file(pmd_dump_data_map, (unsigned char *)hex, 2*size, PMD_WRITE_OP);

    if (rc)
        goto exit;

    printk(" file: data:pmd_dumpdata created\n");//cang printk
exit:
    kfree (data);
    kfree (hex);
    return rc;
}
