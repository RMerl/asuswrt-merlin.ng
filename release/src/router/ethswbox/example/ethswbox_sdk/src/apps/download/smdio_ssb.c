/******************************************************************************

    Copyright 2022 Maxlinear

    SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0-only)

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.
******************************************************************************/
/**
   \file smdio_ssb.c
    GPHY mode flashless load FW example.

*/
#include "smdio_ssb.h"
#include "smdio_access.h"

#include <os_linux.h>
#include <os_types.h>
#include <stdbool.h>

#define SB_PDI_CTRL 0xE100
#define SB_PDI_ADDR 0xE101
#define SB_PDI_DATA 0xE102
#define SB_PDI_STAT 0xE103

#define SB1_ADDR 0x7800
#define SB_PDI_CTRL_RD 0x01
#define SB_PDI_CTRL_WR 0x02
#define SB_PDI_RST 0x0
#define min(a, b) ((a) < (b) ? (a) : (b))

#define FW_DL_FLASHLESS_MAGIC_ER 0x0
#define FW_DL_FLASHLESS_MAGIC 0xC33C

#define SMDIO_GPHY_ID 0x1F

#define SMDIO_PHY_NR 8
static uint8_t SMDIO_PHY_ID[SMDIO_PHY_NR] = {16, 17, 18, 19, 20, 21, 22, 23};

/**
 * Reset SB PDI registers
 */
static void smdio_ssb_pdi_reset(uint8_t lid, uint8_t phy)
{
    smdio_write(lid, phy, SB_PDI_CTRL, SB_PDI_RST);
    smdio_write(lid, phy, SB_PDI_ADDR, SB_PDI_RST);
    smdio_write(lid, phy, SB_PDI_DATA, SB_PDI_RST);
}

int ssb_flashless_load(uint8_t lid, char *fw_path)
{
    int ret, i;
    FILE *fwin;
    int filesize;
    uint8_t *pDataBuf;

#if 0
	/* SMDIO Interface Initialization */
	smdio_init();
#endif

    /* Open, read the FW to be updated */
    fwin = fopen(fw_path, "rb");
    if (fwin == NULL)
    {
        printf("Failed to open FW file \"%s\".\n", fw_path);
        return -errno;
    }

    fseek(fwin, 0L, SEEK_END);
    filesize = ftell(fwin);
    printf("FW size: %d bytes\n", filesize);
    rewind(fwin);

    pDataBuf = (uint8_t *)malloc(filesize);
    if (pDataBuf == NULL)
    {
        printf("Failed to allocate memory (malloc) \n");
        ret = fclose(fwin);
        return -errno;
    }

    ret = fread(pDataBuf, 1, filesize, fwin);
    if (ret != filesize)
    {
        printf("Failed to read FW file \"%s\".\n", fw_path);
        free(pDataBuf);
        ret = fclose(fwin);
        return -errno;
    }
    fclose(fwin);

    /* Update the FW thru the the SMDIO interface */
    ret = smdio_ssb_write(lid, SMDIO_GPHY_ID, (uint8_t *)pDataBuf, filesize);
    free(pDataBuf);
    if (ret != 0)
    {
        printf("FW Upload Failed - FW Write Failed\n");
        return -errno;
    }

    /* Successful update: the device is rebooting in GPY mode
       which takes less that 2 seconds. We wait for 2 seconds
       that the device is up before to check the registers */
    sleep(2);
    ret = check_registers(lid);
    if (ret != 0)
    {
        printf("FW Upload Failed - Register Read Failed\n");
        return -errno;
    }
    printf("FW Upload Sucessful\n");

    return ret;
}

/**
 * Check ID and FW registers after FW Download
 */
int check_registers(uint8_t lid)
{
    int ret, i;

    for (int i = 0; i < SMDIO_PHY_NR; i++)
    {
        if ((mdio_read(lid, SMDIO_PHY_ID[i], 3) == 0xffff) || (mdio_read(lid, SMDIO_PHY_ID[i], 0x1e) == 0xffff))
            return -1;
        i++;
    }

    return 0;
}

/**
 * Target side will set PDI STAT to 0x0, after host side set it to 0x1
 */
static bool smdio_ssb_wait_pdi_stat_is_dwn_magic(uint8_t lid, uint32_t phy)
{
    /* Test 10000 times to avoid endless loop */
    int loop = 0;
    int val = 0;

    while (loop < 10000)
    {
        val = smdio_read(lid, phy, SB_PDI_STAT);
        if ((val == FW_DL_FLASHLESS_MAGIC) || (val == FW_DL_FLASHLESS_MAGIC_ER))
            return true;
        loop++;
    }

    return false;
}

static bool smdio_ssb_wait_pdi_stat_is_zero(uint8_t lid, uint32_t phy)
{
    /* Test 10000 times to avoid endless loop */
    int loop = 0;

    while (loop < 10000)
    {
        if (smdio_read(lid, phy, SB_PDI_STAT) == 0x0)
            return true;
        loop++;
    }

    return false;
}

/**
 * Write data to SSB from offset 0x0
 *
 * pdata - data pointer to be writen to SB
 * len   - size of data
 *
 * return size of data writen to SB if successful
 */
int smdio_ssb_write(uint8_t lid, uint8_t phy, uint8_t *pdata, uint32_t len)
{
    uint32_t word_idx = 0;
    uint32_t slice_cnt = 0;
    uint16_t data_arr[8] = {0};
    uint8_t num = 0;

    if (!pdata)
    {
        printf("Data can not be NULL\n");
        return -EINVAL;
    }

    /* Initialize SMDIO and SSB PDI */
    smdio_ssb_pdi_reset(lid, phy);

    /* Wait for target MCUBoot to select the next slice */
    if (!smdio_ssb_wait_pdi_stat_is_dwn_magic(lid, phy))
    {
        printf("Target MCUBoot not responsive\n");
        return -ECANCELED;
    }

    /* Trigger the write operation */
    smdio_write(lid, phy, SB_PDI_CTRL, SB_PDI_CTRL_WR);

    uint32_t idx = 0;
    while (idx < len)
    {
        num = 0;
        do
        {
            /*Target MCUBoot selected slice and ready
            16 bits data */
            uint16_t fdata = 0x0;
            if (idx + 1 < len)
            {
                fdata = ((pdata[idx + 1]) << 8) | pdata[idx];
                idx += 2;
            }
            else if (idx < len)
            { /* last byte of data, padding high 8bits with 0s */
                fdata |= (uint16_t)pdata[idx];
                idx++;
            }
            else
            { /* no more data */
                break;
            }
            data_arr[num] = fdata;
            num++;
        } while (num < 8);
        smdio_cont_write(lid, phy, SB_PDI_DATA, data_arr, num);

        word_idx += num;
        if (word_idx == 16384)
        { /*  32KB is done, need to set SB PDI addr to 0x7800 */
            smdio_write(lid, phy, SB_PDI_CTRL, SB_PDI_RST);
            smdio_write(lid, phy, SB_PDI_ADDR, SB1_ADDR);
            /* Continue to write SB1 32KB */
            smdio_write(lid, phy, SB_PDI_CTRL, SB_PDI_CTRL_WR);
        }
        else if (word_idx == 32768)
        { /* One slice is done */
            smdio_ssb_pdi_reset(lid, phy);
            /* Notidy target MCUBoot one slice is done */
            smdio_write(lid, phy, SB_PDI_STAT, 0x1);
            sleep(0.003);

            /* Prepare for next slice */
            slice_cnt++;
            word_idx = 0;
            smdio_ssb_pdi_reset(lid, phy);

            /* Wait for target MCUBoot to select the next slice */
            if (!smdio_ssb_wait_pdi_stat_is_dwn_magic(lid, phy))
            {
                printf("Target MCUBoot not responsive\n");
                return -ECANCELED;
            }
            /* Trigger next slice write */
            smdio_write(lid, phy, SB_PDI_CTRL, SB_PDI_CTRL_WR);
        }
    }

    /* all data wroten to SSB
    loop until to slice 7 to allow MCUBoot to know firmware download completed */
    smdio_write(lid, phy, SB_PDI_CTRL, SB_PDI_RST);
    while (slice_cnt < 8)
    {
        smdio_write(lid, phy, SB_PDI_STAT, 0x1);
        slice_cnt++;
        if (slice_cnt == 8)
            break;

        sleep(0.003);
        /* Wait for target MCUBoot to select the next slice */
        if (!smdio_ssb_wait_pdi_stat_is_dwn_magic(lid, phy))
        {
            printf("Target MCUBoot not responsive\n");
            return -ECANCELED;
        }
    }

    smdio_ssb_pdi_reset(lid, phy);

    return 0;
}

#ifdef SMDIO_TEST_TARGET
/**
 * Read data from SSB offset 0x0
 * data patter is increased by 1 for one uint32_t
 * Maximum is one slice SB size 64KB
 * It is used for only for testing purpose
 *
 * len - number of uint32_t value to be read
 *
 * return size of data read from SB if successful
 */
int smdio_ssb_read_verify((uint8_t lid, uint8_t phy, uint32_t len)
{
    uint32_t word_idx = 0;
    uint32_t val = 0;

    if (len < 1 || len > 16384)
    {
        printf("data size overflow one slice SB size\n");
        return -EINVAL;
    }

    /* Initialize SMDIO and SSB PDI */
    smdio_ssb_pdi_reset(lid, phy);

    /* Wait for target MCUBoot to select the next slice */
    if (!smdio_ssb_wait_pdi_stat_is_zero(lid, phy))
    {
        printf("Target MCUBoot not responsive\n");
        return -ECANCELED;
    }
    /* Trigger read operation */
    smdio_write(lid, phy, SB_PDI_CTRL, SB_PDI_CTRL_RD);
    for (uint32_t idx = 0; idx < len; idx++)
    {
        /* Lower 16 bits write to lower address */
        uint32_t lowData = smdio_read(lid, phy, SB_PDI_DATA) & 0xFFFF;
        uint32_t highData = (smdio_read(lid, phy, SB_PDI_DATA) & 0xFFFF) << 16;
        val = lowData | highData;
        if (val != idx)
        {
            printf("%dth uint32_t value is not expected\n", idx);
            return -1;
        }

        if (idx % 32 == 0)
            printf(".");
        word_idx += 2;
        if (word_idx == 16384)
        { /* 32KB is done, need to set SB PDI addr to 0x7800 */
            smdio_write(lid, phy, SB_PDI_CTRL, SB_PDI_RST);
            smdio_write(lid, phy, SB_PDI_ADDR, SB1_ADDR);
            /**/ Continue to write SB1 32KB * /
                smdio_write(lid, phy, SB_PDI_CTRL, SB_PDI_CTRL_RD);
        }
    }

    printf("%d uint32_t value are verified successfully\n", len);
    smdio_ssb_pdi_reset(lid, phy);

    return len;
}
#endif

/**
 * Write data to SSB from offset 0x0
 * data patter is increased by 1 for one uint32_t
 * It is used for only for testing purpose
 *
 * len - number of uint32_t value to be writen
 *
 * return size of data writen to SSB if successful
 */
int smdio_ssb_write_verify(uint8_t lid, uint8_t phy, uint32_t len)
{
    uint32_t word_idx = 0;
    uint32_t slice_cnt = 0;
    uint16_t data_arr[8] = {0};
    uint8_t num = 0;

    if (len < 1 || len > 0x20000)
    {
        printf("data size overflow one slice SB size\n");
        return -EINVAL;
    }

    smdio_ssb_pdi_reset(lid, phy);

    /* Trigger write */
    smdio_write(lid, phy, SB_PDI_CTRL, SB_PDI_CTRL_WR);
    uint32_t idx = 0;
    while (idx < len)
    {

        num = 0;
        for (uint8_t i = 0; i < 4; i++)
        {
            if (idx + i < len)
            {
                data_arr[2 * i] = (0x20f00000 + (idx + i) * 4) & 0xFFFF;
                data_arr[2 * i + 1] = ((0x20f00000 + (idx + i) * 4) >> 16) & 0xFFFF;
                num += 2;
            }
        }
        smdio_cont_write(lid, phy, SB_PDI_DATA, data_arr, num);
        idx += num / 2;
        word_idx += num;

        if (word_idx == 16384)
        { /* 32KB is done, need to set SB PDI addr to 0x7800 */
            smdio_write(lid, phy, SB_PDI_CTRL, SB_PDI_RST);
            smdio_write(lid, phy, SB_PDI_ADDR, SB1_ADDR);
            /* Continue to write SB1 32KB */
            smdio_write(lid, phy, SB_PDI_CTRL, SB_PDI_CTRL_WR);
        }
        else if (word_idx == 32768)
        { /* One slice is done */
            smdio_ssb_pdi_reset(lid, phy);
            /* Notidy target MCUBoot one slice is done */
            smdio_write(lid, phy, SB_PDI_STAT, 0x1);
            sleep(0.003);

            /* Prepare for next slice */
            slice_cnt++;
            word_idx = 0;

            /* Wait for target MCUBoot to select the next slice */
            if (!smdio_ssb_wait_pdi_stat_is_zero(lid, phy))
            {
                printf("Target MCUBoot not responsive\n");
                return -ECANCELED;
            }
            /* Trigger next slice write */
            smdio_write(lid, phy, SB_PDI_CTRL, SB_PDI_CTRL_WR);
        }
    }

    /* all data written to SSB
    loop until to slice 8 to allow MCUBoot to know firmware download completed */
    smdio_write(lid, phy, SB_PDI_CTRL, SB_PDI_RST);
    while (slice_cnt <= 8)
    {
        smdio_write(lid, phy, SB_PDI_STAT, 0x1);
        slice_cnt++;
        sleep(0.003);
        /* Wait for target MCUBoot to select the next slice */
        if (!smdio_ssb_wait_pdi_stat_is_zero(lid, phy))
        {
            printf("Target MCUBoot not responsive\n");
            return -ECANCELED;
        }
    }

    smdio_ssb_pdi_reset(lid, phy);

    return len;
}
