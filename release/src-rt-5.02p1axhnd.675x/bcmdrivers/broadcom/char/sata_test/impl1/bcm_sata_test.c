/*
 * <:copyright-BRCM:2014:proprietary:standard
 * 
 *    Copyright (c) 2014 Broadcom 
 *    All Rights Reserved
 * 
 *  This program is the proprietary software of Broadcom and/or its
 *  licensors, and may only be used, duplicated, modified or distributed pursuant
 *  to the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied), right
 *  to use, or waiver of any kind with respect to the Software, and Broadcom
 *  expressly reserves all rights in and to the Software and all intellectual
 *  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 *  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 *  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 * 
 *  Except as expressly set forth in the Authorized License,
 * 
 *  1. This program, including its structure, sequence and organization,
 *     constitutes the valuable trade secrets of Broadcom, and you shall use
 *     all reasonable efforts to protect the confidentiality thereof, and to
 *     use this information only in connection with your use of Broadcom
 *     integrated circuit products.
 * 
 *  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
 *     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
 *     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
 *     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
 *     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
 *     PERFORMANCE OF THE SOFTWARE.
 * 
 *  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
 *     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
 *     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 *     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
 *     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
 *     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
 *     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
 *     LIMITED REMEDY.
 * :> 
 */


/*
 ****************************************************************************
 * File Name  : bcm_sata_test.c
 *
 * Description: This file contains the initilzation and test routines
 * to enable sata controller on bcm63xxx boards.
 *
 *
 ***************************************************************************/

#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/dma-mapping.h>
#include <linux/clkdev.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/bug.h>
#include <asm/uaccess.h>

#include <bcm_intr.h>
#include <bcm_map_part.h>
#include <pmc_sata.h>
#include <bcm_sata_test.h>


/* macros to read write, to registers, with memory barriers to avoid reordering */
#define BDEV_RD(x)      (*((volatile uint32_t *)(x))); mb()
#define BDEV_WR(x, y)   do { *((volatile uint32_t *)(x)) = (y); mb(); } while (0)


#define SATA_HBA_BASE_ADDR          SATA_BASE

#define SATA_TOP_CTRL               (SATA_HBA_BASE_ADDR+0x0040)
#define SATA_PORT0_PCB              (SATA_HBA_BASE_ADDR+0x0100)
#define SATA_AHCI_BASE              (SATA_HBA_BASE_ADDR+0x2000)
#define SATA_AHCI_GHC               (SATA_HBA_BASE_ADDR+0x2000)
#define SATA_PORT0_AHCI_S1          (SATA_HBA_BASE_ADDR+0x2100)
#define SATA_PORT0_AHCI_S2          (SATA_HBA_BASE_ADDR+0x2120)

#define SATA_AHCI_GHC_PHYS          (SATA_PHYS_BASE+0x2000)

#define SATA_MEM_SIZE               0x00002000

/* SATA_TOP_CTRL regsiters */
#define SATA_TOP_CTRL_BUS_CTRL      (SATA_TOP_CTRL+0x04)

/* SATA_PORT0_AHCI_S1 registers */
#define SATA_PORT0_AHCI_S1_PXIS     (SATA_PORT0_AHCI_S1+0x10)
#define SATA_PORT0_AHCI_S1_PXIE     (SATA_PORT0_AHCI_S1+0x14)
#define SATA_PORT0_AHCI_S1_PXCMD    (SATA_PORT0_AHCI_S1+0x18)

/* SATA_PORT0_AHCI_S2 registers */
#define SATA_PORT0_AHCI_S2_PXSSTS     (SATA_PORT0_AHCI_S2+0x8)
#define SATA_PORT0_AHCI_S2_PXSCTL     (SATA_PORT0_AHCI_S2+0xC)

/* GHC regs */
#define GHC_HBA_CAP                 (SATA_AHCI_GHC+0x00) /* host capabilities */
#define GHC_GLOBAL_HBA_CONTROL      (SATA_AHCI_GHC+0x04) /* global host control */
#define GHC_INTERRUPT_STATUS        (SATA_AHCI_GHC+0x08) /* interrupt status */
#define GHC_PORTS_IMPLEMENTED       (SATA_AHCI_GHC+0x0c) /* bitmap of implemented ports */
#define GHC_HOST_VERSION            (SATA_AHCI_GHC+0x10) /* AHCI spec. version compliancy */

/* SATA_PORT0_LEG_S3 regs */
#define SATA_PORT0_LEG_S3_SATA_CONTROL2     (SATA_HBA_BASE_ADDR+0x2984)

/* SATA_PORT0_CTRL regs */
#define SATA_PORT0_CTRL_PCTRL4      (SATA_HBA_BASE_ADDR+0x2710)


/* Phy reg */
#define PORT0_SATA3_PCB_BLOCK_ADDR  (SATA_PORT0_PCB+0x023C)
#define PORT0_SATA3_PCB_REG0        (SATA_PORT0_PCB+0x0200)

#define PCB_REG(x) (uint32_t)(PORT0_SATA3_PCB_REG0 + x*4)

/* SATA PHY regsister banks-accessible through MDIO/PCB */
#define SATA3_BLOCK_0_REG_BANK  0x0000 
#define SATA3_BLOCK_1_REG_BANK  0x0010
#define SATA3_BLOCK_2_REG_BANK  0x0020
#define SATA3_PLL_REG_BANK_0    0x0050
#define SATA3_PLL_REG_BANK_1    0x0060 
#define SATA3_TX_REG_BANK       0x0070
#define SATA3_RX_REG_BANK       0x00B0
#define SATA3_AEQ_REG_BANK      0x00D0
#define SATA3_AEQSLCAL_REG_BANK 0x00E0
#define SATA3_OOB_REG_BANK      0x0150
#define SATA3_TXPMD_REG_BANK    0x01a0
#define SATA3_RXPMD_REG_BANK    0x01c0


struct tx_amplitude{
    uint16_t reg_value;
    uint16_t mvolts;
};

#define TX_AMP_500MV_MAX_COUNT 45
#define TX_AMP_1000MV_MAX_COUNT 52

struct tx_amplitude tx_amp_500mv[TX_AMP_500MV_MAX_COUNT] = { {0x3, 220}, {0x4, 239}, {0x5, 258},
    {0x6, 277}, {0x7, 295}, {0x8, 313}, {0x9, 330}, {0xa, 347}, {0xb, 364},
    {0xc, 380}, {0xd, 396}, {0xe, 412}, {0xf, 427}, {0x10, 442}, {0x11, 457},
    {0x12, 472}, {0x13, 486}, {0x14, 500}, {0x15, 514}, {0x16, 527}, {0x17, 541},
    {0x18, 554}, {0x19, 566}, {0x1a, 579}, {0x1b, 591}, {0x1c, 603}, {0x1d, 615},
    {0x1e, 627}, {0x1f, 639}, {0x20, 650}, {0x21, 661}, {0x22, 672}, {0x23, 683},
    {0x24, 694}, {0x25, 704}, {0x26, 714}, {0x27, 724}, {0x28, 734}, {0x29, 744},
    {0x2a, 754}, {0x2b, 763}, {0x2c, 773}, {0x2d, 782}, {0x2e, 791}, {0x2f, 744} };

struct tx_amplitude tx_amp_1000mv[TX_AMP_1000MV_MAX_COUNT] = { {0xc, 650}, {0xd, 667}, {0xe, 683},
    {0xf, 699}, {0x10, 714}, {0x11, 729}, {0x12, 744}, {0x13, 759}, {0x14, 773},
    {0x15, 787}, {0x16, 800}, {0x17, 813}, {0x18, 826}, {0x19, 839}, {0x1a, 851},
    {0x1b, 863}, {0x1c, 875}, {0x1d, 887}, {0x1e, 898}, {0x1f, 909}, {0x20, 920},
    {0x21, 931}, {0x22, 941}, {0x23, 951}, {0x24, 962}, {0x25, 971}, {0x26, 981},
    {0x27, 991}, {0x28, 1000}, {0x29, 1009}, {0x2a, 1018}, {0x2b, 1027}, {0x2c, 1036},
    {0x2d, 1044}, {0x2e, 1053}, {0x2f, 1061}, {0x30, 1069}, {0x31, 1077}, {0x32, 1085},
    {0x33, 1092}, {0x34, 1100}, {0x35, 1107}, {0x36, 1115}, {0x37, 1122}, {0x38, 1129}, 
    {0x39, 1136}, {0x3a, 1143}, {0x3b, 1150}, {0x3c, 1156}, {0x3d, 1163}, {0x3e, 1169}, 
    {0x3f, 1176} };


struct sata_config{
 uint16_t ssc_enabled;
 uint16_t sata_mode;
 uint16_t tx_amp_mvolts;
};
 
struct sata_config g_sata_config;
static uint32_t g_tx_reset_needed = 0;

#if 0
static  void sata_hw_init(void)
{
    WriteBPCMRegister(PMB_ADDR_SATA, BPCMRegOffset(misc_control), 0);
    WriteBPCMRegister(PMB_ADDR_SATA, BPCMRegOffset(sr_control), ~0);
    WriteBPCMRegister(PMB_ADDR_SATA, BPCMRegOffset(sr_control), 0);
}
#endif

static void pcb_write(uint32_t pcb_block, uint32_t reg_addr, uint32_t value)
{
    BDEV_WR(PORT0_SATA3_PCB_BLOCK_ADDR, pcb_block);
    mdelay(1);
    BDEV_WR(reg_addr, value);
#if 0
    /*write again */
    BDEV_WR(reg_addr, value);
#endif
    mdelay(1);
}

static uint32_t pcb_read(uint32_t pcb_block, uint32_t reg_addr)
{
    uint32_t value;

    BDEV_WR(PORT0_SATA3_PCB_BLOCK_ADDR, pcb_block);
    mdelay(1);
    /*read twice due to bug in 63138A0 */
    value = BDEV_RD(reg_addr);
    value = BDEV_RD(reg_addr);

    return value;
}

static void GetFreqLock( void )
{
    uint32_t regData;
    int i = 10;

    //printk("writing PORT0_SATA3_PCB_BLOCK_ADDR\n");

    pcb_write(0x60, PCB_REG(7), 0x873);

    pcb_write(0x60, PCB_REG(6), 0xc000);

    pcb_write(0x50, PCB_REG(1), 0x3089);
    udelay(100);
    pcb_write(0x50, PCB_REG(1), 0x3088);
    udelay(1000);
    //// Done with PLL ratio change and re-tunning

    pcb_write(0xE0, PCB_REG(2), 0x3000);
    pcb_write(0xE0, PCB_REG(6), 0x3000);

    udelay(1000);
    pcb_write(0x50, PCB_REG(3), 0x32);

    pcb_write(0x50, PCB_REG(4), 0xA);

    pcb_write(0x50, PCB_REG(6), 0x64);

    udelay(1000);
    BDEV_WR(PORT0_SATA3_PCB_BLOCK_ADDR, 0x00);
    wmb();

    regData = BDEV_RD(PCB_REG(1));

    while (i && ((regData & 0x1000) == 0))
    {
        regData = BDEV_RD(PCB_REG(1));
        udelay(1000);
        i--;
    }
    printk("SATA PLL lock for port0 detected %0x...\n", regData);
}

static void sata_sim_init(void)
{
    BDEV_WR(GHC_GLOBAL_HBA_CONTROL, 0x80000001);
    mdelay(1);
    BDEV_WR(GHC_GLOBAL_HBA_CONTROL, 0x80000000);
    mdelay(10);

    BDEV_WR(SATA_PORT0_AHCI_S1_PXIS, 0x7fffffff);
    BDEV_WR(GHC_INTERRUPT_STATUS, 0x7fffffff);
    BDEV_WR(SATA_PORT0_AHCI_S1_PXIE, 0x7fffffff);

    BDEV_WR(SATA_PORT0_AHCI_S1_PXCMD, 0x00000010);
    /* setup endianess */
    BDEV_WR(SATA_TOP_CTRL_BUS_CTRL, 0x00000000);
}


static void write_serdes(uint32_t bank, uint32_t reg_num, uint32_t data)
{
    pcb_write(bank, PCB_REG(reg_num), data);
}

static uint32_t read_serdes(uint32_t bank, uint32_t reg_num)
{
    return pcb_read(bank,  PCB_REG(reg_num));
}



static void bcm_sata_ssc_set(uint32_t enable)
{
    uint32_t rvalue;

    if(enable)
    {   
        /* enable SSC */
        rvalue = pcb_read(SATA3_TXPMD_REG_BANK, PCB_REG(1));
        rvalue |= 0x3;
        pcb_write(SATA3_TXPMD_REG_BANK, PCB_REG(1), rvalue);
    }
    else
    {
        /* disable SSC */
        rvalue = pcb_read(SATA3_TXPMD_REG_BANK, PCB_REG(1));
        rvalue &= ~(0x3);
        pcb_write(SATA3_TXPMD_REG_BANK, PCB_REG(1), rvalue);
    }
    g_sata_config.ssc_enabled = enable;
}

static uint32_t bcm_sata_ssc_get(void)
{
    uint32_t rvalue;

    rvalue = pcb_read(SATA3_TXPMD_REG_BANK, PCB_REG(1));
    if((rvalue & 0x3) == 0x3)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/* Set/Get SATA MODE GEN1(1.5 Gbps), GEN2(3.0 Gbps), GEN3(6.0 Gbps) */
static void bcm_sata_speed_mode_set(uint32_t mode)
{
    uint32_t rvalue1;
    uint32_t rvalue2;

    rvalue1 = pcb_read(SATA3_BLOCK_0_REG_BANK,PCB_REG(0xE));
    rvalue1 &= 0xFFFFCBEF;

    rvalue2 = pcb_read(SATA3_BLOCK_1_REG_BANK,PCB_REG(0x3));
    rvalue2 &= 0xFFFFFFEC;


    switch(mode)
    {
        case SATA_MODE_GEN1i:
        case SATA_MODE_GEN1m:
            /* 1.5 Gbps */
            rvalue1 |= 0x400;
            rvalue2 |= 0x10;
            break;
        case SATA_MODE_GEN2i:
        case SATA_MODE_GEN2m:
            /* 3.0 Gbps */
            rvalue1 |= 0x1400;
            rvalue2 |= 0x11;
            break;
        case SATA_MODE_GEN3i:
            /* 6.0 Gbps */
            rvalue1 |= 0x2400;
            rvalue2 |= 0x12;
            break;
        default:
            printk("Error:unsupported sata_mode=%d\n", mode);
            return;
    }

    g_sata_config.sata_mode = mode;
    pcb_write(SATA3_BLOCK_0_REG_BANK, PCB_REG(0xE), rvalue1);
    pcb_write(SATA3_BLOCK_1_REG_BANK, PCB_REG(0x3), rvalue2);
}

static uint32_t bcm_sata_speed_mode_get(void)
{

    uint32_t rvalue1;
    uint32_t rvalue2;
    uint32_t mode;

    rvalue1 = pcb_read(SATA3_BLOCK_0_REG_BANK, PCB_REG(0xE));
    //rvalue1 &= 0xFFFFCBEF;

    rvalue2 = pcb_read(SATA3_BLOCK_1_REG_BANK, PCB_REG(0x3));
    //rvalue2 &= 0xFFFFFFEC;

    /*TODO how to differentiate bwteeen i & m modes */    

    if(((rvalue1 & 0x400) == 0x400) && ((rvalue2 & 0x10) == 0x10))
    {
        mode =SATA_MODE_GEN1i;
    }
    else if(((rvalue1 & 0x1400) == 0x1400) && ((rvalue2 & 0x11) == 0x11))
    {
        mode =SATA_MODE_GEN2i;
    }
    else if(((rvalue1 & 0x2400) == 0x2400) && ((rvalue2 & 0x12) == 0x12))
    {
        mode =SATA_MODE_GEN3i;
    }
    else
    {
        /*unknown */
        mode=SATA_MODE_MAX;
    }

    return mode;
}

static void bcm_sata_tx_amp_set(uint32_t mvolts)
{
    uint32_t rvalue;
    uint32_t reg1_value = 0;
    uint32_t mode;
    int i;

    if(mvolts > 650)
    {
        /* use 1000mV mode */
        mode = 0x00;
        for(i=0; i< TX_AMP_1000MV_MAX_COUNT; i++)
        {
            if(tx_amp_1000mv[i].mvolts >= mvolts)
            {
                reg1_value =  tx_amp_1000mv[i].reg_value;
                break;
            }
        } 
    }
    else
    {
        /* use 500mV mode */
        mode = 0x10;
        for(i=0; i< TX_AMP_500MV_MAX_COUNT; i++)
        {
            if(tx_amp_500mv[i].mvolts >= mvolts)
            {
                reg1_value =  tx_amp_500mv[i].reg_value;
                break;
            }
        }
    }
    
    if (reg1_value)
    {
        /*configure bit 1*/
        rvalue = pcb_read(SATA3_TX_REG_BANK, PCB_REG(2));
        rvalue &= ~(0x10);
        rvalue |= mode;
        pcb_write(SATA3_TX_REG_BANK, PCB_REG(2), rvalue);

        /*configure bits 8-13*/
        rvalue = pcb_read(SATA3_TX_REG_BANK, PCB_REG(1));
        rvalue &= 0xC0FF;
        rvalue |= (reg1_value << 8);
        pcb_write(SATA3_TX_REG_BANK, PCB_REG(1), rvalue);

        g_sata_config.tx_amp_mvolts = mvolts;
    }
    else
    {
        printk("%s: Error: unsupported mvolts=%d\n",__func__, mvolts);
    }
}

static uint32_t bcm_sata_tx_amp_get(void)
{
    uint32_t mode; 
    uint32_t mvolts = 0; 
    uint32_t rvalue;
    int i;

    rvalue = pcb_read(SATA3_TX_REG_BANK, PCB_REG(2));
    mode = rvalue & 0x10;
    
    rvalue = pcb_read(SATA3_TX_REG_BANK, PCB_REG(1));
    rvalue &= 0x3F00;
    rvalue = rvalue >>8;

    if(mode)
    {
        /*check in 500mV */
        for(i=0; i< TX_AMP_500MV_MAX_COUNT; i++)
        {
            if(rvalue == tx_amp_500mv[i].reg_value)
            {
                mvolts =  tx_amp_500mv[i].mvolts;
                break;
            }
        }
    }
    else
    {
        /*check in 1000mV */
        for(i=0; i< TX_AMP_1000MV_MAX_COUNT; i++)
        {
            if(rvalue == tx_amp_1000mv[i].reg_value)
            {
                mvolts =  tx_amp_1000mv[i].mvolts;
                break;
            }
        }
    }

    return mvolts;
}

static int bcm_sata_test_mode_init(void)
{
    printk("++++ Powering on SATA block\n");

    pmc_sata_power_up();
    mdelay(1);

    //sata_hw_init();
    //mdelay(1);

    GetFreqLock();
    mdelay(1);

    sata_sim_init();
    mdelay(1);

    printk("+++ SATA block ready for testmode\n");
    return 0;
}

static void bcm_sata_restore_config(void )
{
        /* restore settings from g_sata_config */
        bcm_sata_speed_mode_set(g_sata_config.sata_mode);
        bcm_sata_ssc_set(g_sata_config.ssc_enabled);
        bcm_sata_tx_amp_set(g_sata_config.tx_amp_mvolts);
}

static int bcm_sata_test_mode_reset(uint32_t restore)
{
    sata_sim_init();
    mdelay(1);

    if(restore)
    {
        bcm_sata_restore_config();
    }
    mdelay(1);

    return 0;
}

/* TX tests */

static void bcm_sata_test_lftp(void)
{
    write_serdes(SATA3_TX_REG_BANK, 0x9, 0x871e);
    write_serdes(SATA3_TX_REG_BANK, 0xa, 0x71e3);
    write_serdes(SATA3_TX_REG_BANK, 0xb, 0x1e38);
    write_serdes(SATA3_TX_REG_BANK, 0xc, 0xe387);
    write_serdes(SATA3_TX_REG_BANK, 0xd, 0x3871);
}

static void bcm_sata_test_lbp(void)
{
    write_serdes(SATA3_TX_REG_BANK, 0x9, 0x2f6c);
    write_serdes(SATA3_TX_REG_BANK, 0xa, 0xf6c4);
    write_serdes(SATA3_TX_REG_BANK, 0xb, 0xacc2);
    write_serdes(SATA3_TX_REG_BANK, 0xc, 0xcb2c);
    write_serdes(SATA3_TX_REG_BANK, 0xd, 0x32ca);
}

static void bcm_sata_test_hftp(void)
{
    write_serdes(SATA3_TX_REG_BANK, 0x9, 0x5555);
    write_serdes(SATA3_TX_REG_BANK, 0xa, 0x5555);
    write_serdes(SATA3_TX_REG_BANK, 0xb, 0x5555);
    write_serdes(SATA3_TX_REG_BANK, 0xc, 0x5555);
    write_serdes(SATA3_TX_REG_BANK, 0xd, 0x5555);
}

static void bcm_sata_test_mftp(void)
{
    write_serdes(SATA3_TX_REG_BANK, 0x9, 0x3333);
    write_serdes(SATA3_TX_REG_BANK, 0xa, 0x3333);
    write_serdes(SATA3_TX_REG_BANK, 0xb, 0x3333);
    write_serdes(SATA3_TX_REG_BANK, 0xc, 0x3333);
    write_serdes(SATA3_TX_REG_BANK, 0xd, 0x3333);
}


static void bcm_sata_test_prbs(uint32_t prbs_type)
{

    if(prbs_type == 0)
    {
        write_serdes(SATA3_BLOCK_1_REG_BANK, 0x8, 0x88);
    }
    else if(prbs_type == 1)
    {
        write_serdes(SATA3_BLOCK_1_REG_BANK, 0x8, 0x99);
    }
    else if(prbs_type == 2)
    {
        write_serdes(SATA3_BLOCK_1_REG_BANK, 0x8, 0xaa);
    }
    else if(prbs_type == 3)
    {
        write_serdes(SATA3_BLOCK_1_REG_BANK, 0x8, 0xbb);
    }
    else
    {
        printk("Error:unsupported prbs_type=%x\n", prbs_type);
        return;
    }

    write_serdes(SATA3_TX_REG_BANK, 0x0, 0x2);
}


static int bcm_sata_test_PHY_TSG_MOI(struct sata_test_params *test_params)
{
    uint32_t tx_amp_setting_0;
    int ret = 0;
    
    if(g_tx_reset_needed)
    {
        bcm_sata_test_mode_reset(1);
        g_tx_reset_needed = 0;
    }

    /* override  SAPIS_TX_ENA */
    write_serdes(SATA3_BLOCK_0_REG_BANK, 0xd, 0x0600);

    tx_amp_setting_0 =read_serdes(SATA3_TX_REG_BANK, 0x1);
    //printk(" Tx_amp_setting_0 =%x\n", tx_amp_setting_0);

    /*enable TX mdio mode on port 0 */
    write_serdes(SATA3_TX_REG_BANK, 0x0, 0x21);
    /*'Enable 80-bit (16bit x 5 registers) long pattern on port 0 */
    write_serdes(SATA3_TX_REG_BANK, 0x5, 0x1f);

    tx_amp_setting_0 =read_serdes(SATA3_TX_REG_BANK, 0x1);
    //printk(" Tx_amp_setting_0 =%x\n", tx_amp_setting_0);

    switch(test_params->test_type)
    {
        case SATA_PHY_TSG_MOI_TEST_LFTP:
            bcm_sata_test_lftp();
            break;
        case SATA_PHY_TSG_MOI_TEST_LBP:
            bcm_sata_test_lbp();
            break;
        case SATA_PHY_TSG_MOI_TEST_HFTP:
            bcm_sata_test_hftp();
            break;
        case SATA_PHY_TSG_MOI_TEST_MFTP:
            bcm_sata_test_mftp();
            break;
        case SATA_PHY_TSG_MOI_TEST_PRBS:
            bcm_sata_test_prbs(test_params->param1);
            break;
        default:
            printk("Error %s:Unsupported test=%u\n",__FUNCTION__, test_params->test_type);
            ret = -1;
    } 
    return ret;
}

/* RX tests */

static void bcm_rx_bist_l_set(uint32_t enable)
{
    uint32_t rvalue;

    if(enable)
    {
        rvalue = BDEV_RD(SATA_PORT0_CTRL_PCTRL4);
        rvalue |= 0x80000000; /*set bit 31*/
        BDEV_WR(SATA_PORT0_CTRL_PCTRL4, rvalue);

        rvalue = BDEV_RD(SATA_PORT0_LEG_S3_SATA_CONTROL2);
        rvalue |= 0x40;/*set bit 6*/
        BDEV_WR(SATA_PORT0_LEG_S3_SATA_CONTROL2, rvalue);
    }
    else
    {
        rvalue = BDEV_RD(SATA_PORT0_CTRL_PCTRL4);
        rvalue &= ~(0x80000000); /*clear bit 31*/
        BDEV_WR(SATA_PORT0_CTRL_PCTRL4, rvalue);

        rvalue = BDEV_RD(SATA_PORT0_LEG_S3_SATA_CONTROL2);
        rvalue &= ~(0x40);/*clear bit 6*/
        BDEV_WR(SATA_PORT0_LEG_S3_SATA_CONTROL2, rvalue);
    }
}

static void bcm_sata_test_rx_bist_l_gen1(struct sata_test_params *test_params)
{
    uint32_t rvalue;

    BDEV_WR(SATA_PORT0_AHCI_S2_PXSCTL, 0x010);

    rvalue = BDEV_RD(SATA_PORT0_AHCI_S1_PXCMD);
    rvalue &= ~0x3;
    BDEV_WR(SATA_PORT0_AHCI_S1_PXCMD, rvalue);
    
    mdelay(5);

    rvalue = BDEV_RD(SATA_PORT0_AHCI_S1_PXCMD);
    rvalue |= 0x3;
    BDEV_WR(SATA_PORT0_AHCI_S1_PXCMD, rvalue);

    bcm_rx_bist_l_set(1);
}

static void bcm_sata_test_rx_bist_l_gen2(struct sata_test_params *test_params)
{
    uint32_t rvalue;

    BDEV_WR(SATA_PORT0_AHCI_S2_PXSCTL, 0x020);

    rvalue = BDEV_RD(SATA_PORT0_AHCI_S1_PXCMD);
    rvalue &= ~0x3;
    BDEV_WR(SATA_PORT0_AHCI_S1_PXCMD, rvalue);

    mdelay(5);

    rvalue = BDEV_RD(SATA_PORT0_AHCI_S1_PXCMD);
    rvalue |= 0x3;
    BDEV_WR(SATA_PORT0_AHCI_S1_PXCMD, rvalue);

    bcm_rx_bist_l_set(1);
}

static void bcm_sata_test_rx_bist_l_gen3(struct sata_test_params *test_params)
{
    uint32_t rvalue;

    BDEV_WR(SATA_PORT0_AHCI_S2_PXSCTL, 0x030);

    rvalue = BDEV_RD(SATA_PORT0_AHCI_S1_PXCMD);
    rvalue &= ~0x3;
    BDEV_WR(SATA_PORT0_AHCI_S1_PXCMD, rvalue);

    mdelay(5);

    rvalue = BDEV_RD(SATA_PORT0_AHCI_S1_PXCMD);
    rvalue |= 0x3;
    BDEV_WR(SATA_PORT0_AHCI_S1_PXCMD, rvalue);

    bcm_rx_bist_l_set(1);
}

static int bcm_sata_test_rx(struct sata_test_params *test_params)
{
    int ret = 0;

    bcm_sata_test_mode_reset(0);
    g_tx_reset_needed = 1;

    switch(test_params->test_type)
    {
        case SATA_RX_TEST_BIST_L_GEN1:
            bcm_sata_test_rx_bist_l_gen1(test_params);
            break;
        case SATA_RX_TEST_BIST_L_GEN2:
            bcm_sata_test_rx_bist_l_gen2(test_params);
            break;
        case SATA_RX_TEST_BIST_L_GEN3:
            bcm_sata_test_rx_bist_l_gen3(test_params);
            break;

        default:
            printk("Error %s:Unsupported test=%u\n",__FUNCTION__, test_params->test_type);
            ret = -1;
    } 
    return ret;
}

/* Interface for custom SATA PHY regsiter settings */
static int bcm_sata_rw_phy_reg(struct sata_test_params *test_params)
{
    uint32_t rvalue ;
    int ret = 0;

    switch(test_params->test_type)
    {
        case SATA_PHY_REG_READ:
            printk("%s:PHY READ: REG_BANK<0x%x> REG_NUM(0x%x) \n", __func__,
                    test_params->param1,test_params->param2);
            rvalue= pcb_read(test_params->param1, PCB_REG(test_params->param2));
            printk("\n%s: Register Value =0x%x\n\n", __func__, rvalue);
            break;

        case SATA_PHY_REG_WRITE:
            printk("%s:PHY WRITE: REG_BANK<0x%x> REG_NUM(0x%x) VALUE(0x%x) \n", __func__,
                    test_params->param1,test_params->param2, test_params->param3);
            pcb_write(test_params->param1, PCB_REG(test_params->param2), test_params->param3);
            break;

        default:
            printk("Error %s:Unsupported test=%u\n",__FUNCTION__, test_params->test_type);
            ret = -1;
    }
    return ret;
}

static int bcm_sata_configuration_set(struct sata_test_params *test_params)
{
    switch(test_params->param_type)
    {
        case SATA_TEST_CONFIG_SSC:
            bcm_sata_ssc_set(test_params->param1);
            break;

        case SATA_TEST_CONFIG_SATAMODE:
            bcm_sata_speed_mode_set(test_params->param1);
            break;

        case SATA_TEST_CONFIG_TXAMP:
            bcm_sata_tx_amp_set(test_params->param1);
            break;

        default:
            printk("Error %s:Unsupported test parameter=%u\n",__FUNCTION__, test_params->param_type);
    }
    return 0;
}

static void bcm_sata_configuration_get(struct sata_test_params *test_params)
{
    if(test_params->test_mode != SATA_TESTMODE_STOP)
    {
        test_params->ssc_enabled = bcm_sata_ssc_get();;
        test_params->tx_amp_mvolts = bcm_sata_tx_amp_get();

        bcm_sata_speed_mode_get();
        test_params->sata_mode = g_sata_config.sata_mode;
    }
}

static int bcm_sata_testmode_stop(struct sata_test_params *test_params)
{
    printk("++++ Powering off SATA block\n");
    pmc_sata_power_down();
    return 0;
}

static int bcm_sata_test_main(struct sata_test_params *test_params)
{
    int ret = -1;

    if(test_params->test_mode == SATA_TESTMODE_START)
    {
        bcm_sata_test_mode_init();
        return 0;
    }

    switch(test_params->test_mode)
    {
        case SATA_TESTMODE_CONFIGURATION:
            ret = bcm_sata_configuration_set(test_params);
            break;

        case SATA_TESTMODE_PHY_TSG_MOI: 
            ret = bcm_sata_test_PHY_TSG_MOI(test_params);
            break;
        case SATA_TESTMODE_RX:
            ret = bcm_sata_test_rx(test_params);
            break;

        case SATA_TESTMODE_RW_PHY_REGS:
            ret = bcm_sata_rw_phy_reg(test_params);
            break;

        case SATA_TESTMODE_STOP:
            ret = bcm_sata_testmode_stop(test_params);
            break;

        default: 
            printk("Error %s:Unsupported test_mode=%u\n",__FUNCTION__, test_params->test_mode);
    }

    bcm_sata_configuration_get(test_params);

    return ret;
}

static int bcm_sata_test_ioctl(void *arg)
{
    int ret;
    struct sata_test_params test_params;

    //printk("Entered %s\n",__FUNCTION__);

    if( arg == NULL)
    {
        printk(KERN_ERR "%s: Error:NULL test_params\n", __FUNCTION__);
        return -1;
    }

    if(copy_from_user((void*)&test_params, arg, sizeof(test_params)))
    {
        printk(KERN_ERR "%s: Error copy from user failed arg=%p\n", __FUNCTION__, arg);
        return -1;

    }

    /*TODO: add lock here if multiple programs access this simultaneously*/
    ret = bcm_sata_test_main(&test_params);


    if(copy_to_user(arg,(void*)&test_params, sizeof(test_params)))
    {
        printk(KERN_ERR "%s: Error copy to user failed arg=%p\n", __FUNCTION__, arg);
        return -1;
    }
    
    return ret;
}

extern int (*bcm_sata_test_ioctl_fn)(void *);


static __init int bcm_sata_test_init(void)
{
    bcm_sata_test_ioctl_fn = bcm_sata_test_ioctl;
    return 0;
}

static void bcm_sata_test_cleanup(void)
{
    bcm_sata_test_ioctl_fn = NULL;
}
/***************************************************************************
* MACRO to call driver initialization and cleanup functions.
***************************************************************************/
module_init(bcm_sata_test_init);
module_exit(bcm_sata_test_cleanup);

MODULE_LICENSE("proprietary");
