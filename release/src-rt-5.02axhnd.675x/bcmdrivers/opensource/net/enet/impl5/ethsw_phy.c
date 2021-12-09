/*
 <:copyright-BRCM:2011:DUAL/GPL:standard
 
    Copyright (c) 2011 Broadcom 
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
#define _BCMENET_LOCAL_
#include <board.h>
#include "bcm_OS_Deps.h"
#include "bcmmii.h"
#include "bcmenet.h"
#include "bcmsw.h"
#include "boardparms.h"
#include "bcmswaccess.h"
#include "bcm_map_part.h"
#include "ethsw_phy.h"

extern struct semaphore bcm_ethlock_switch_config;
extern uint8_t port_in_loopback_mode[TOTAL_SWITCH_PORTS];
int ethsw_phy_load_firmware(int phyId);

#if defined(SWITCH_REG_SINGLE_SERDES_CNTRL)
#include "pmc/pmc_drv.h"

#if defined(SERDES_2P5G_CAPABLE)

u8 firmware_8486x[] =
#include "BCM8486-A0-v00-01-01-TCM.hex"
;

u8 firmware_8488x[] =
#include "BCM8488-A0-v01-00-03-TCM.hex"
;

typedef struct phyDesc {
    int devId;
    u8 *firmware;
    char *devName;
    int flag;
#define CL45PHY_10G_CAP (1<<0)
#define CL45PHY_BRCM2P5G_CAP (1<<1)
} phyDesc_t;

static phyDesc_t phyDesc[] = {
    {0xae025048, firmware_8486x, "84860", CL45PHY_BRCM2P5G_CAP},
    {0xae025040, firmware_8486x, "84861", CL45PHY_BRCM2P5G_CAP|CL45PHY_10G_CAP},
    {0xae025158, firmware_8488x, "84880"},
    {0xae025150, firmware_8488x, "84881", CL45PHY_10G_CAP},
}, *mountedCl45Phy;
#define IsBrcm2P5GPhy(pdesc) (pdesc->flag & CL45PHY_BRCM2P5G_CAP)

static u16 serdesRef50mVco6p25 [] =
{   
    0x8000, 0x0c2f,
    0x8308, 0xc000,
    0x8050, 0x5740,
    0x8051, 0x01d0,
    0x8052, 0x19f0,
    0x8053, 0xaab0,
    0x8054, 0x8821,
    0x8055, 0x0044,
    0x8056, 0x8000,
    0x8057, 0x0872,
    0x8058, 0x0000,

    0x8106, 0x0020,
    0x8054, 0x8021,
    0x8054, 0x8821,
}; 

static u16 serdesSet2p5GFiber [] =
{
    0x0010, 0x0C2F,       /* disable pll start sequencer */
    0x8300, 0x0149,       /* enable fiber mode, signal_detect_en, invert signal detect(b2) */
    0x8308, 0xC010,       /* Force 2.5G Fiber, enable 50MHz refclk */
    0x834a, 0x0001,       /* Set os2 mode */
    0x0000, 0x0140,       /* disable AN, set 1G mode */
    0x0010, 0x2C2F,       /* enable pll start sequencer */
}; 

static u16 serdesSet1GForcedFiber [] =
{
    0x0010, 0x0c2f,     /* disable pll start sequencer */
    0x8300, 0x0109,     /* Force Invert Signal Polarity */
    0x8473, 0x1251,
    0x834a, 0x0003,
    0x0000, 0x0140,
    0x0010, 0x2c2f,     /* enable pll start sequencer */
};

static u16 serdesSet1GFiber [] =
{
    0x0010, 0x0c2f,     /* disable pll start sequencer */
    0x8300, 0x0149,     /* Force Auto Detect, Invert Signal Polarity */
    0x8473, 0x1251,
    0x834a, 0x0003,
    0x0000, 0x1140,
    0x0010, 0x2c2f,     /* enable pll start sequencer */
};

static u16 serdesSet100MFiber [] =
{
    0x0010, 0x0c2f,     /* disable pll start sequencer */
    0x8300, 0x0009,     /* enable fiber mode */
    0x8400, 0x014b,     /* enable fxmode */
    0x8402, 0x0880,     /* set disable_idle_correlator */
    0x8473, 0x1251,     /* Set step_one[1:0] */
    0x834a, 0x0003,     /* set os5 mode */
    0x0010, 0x2c2f,     /* enable pll start sequencer */
};

static u16 serdesSetAutoSGMII [] =
{
    0x0010, 0x0c2f,     /* disable pll start sequencer */
    0x8300, 0x0100,
    0x8301, 0x0007,
    0x8473, 0x1251,
    0x834a, 0x0003,
    0x0000, 0x1140,
    0x0010, 0x2c2f,     /* enable pll start sequencer */
};

static u16 serdesSet100MForcedSGMII [] = 
{
    0x0010, 0x0c2f,     /* disable pll start sequencer */
    0x8300, 0x0100,
    0x8301, 0x0007,
    0x8473, 0x1251,
    0x834a, 0x0003,
    0x0000, 0x2100,
    0x0010, 0x2c2f,     /* enable pll start sequencer */
};
#else
static u16 serdesSet1GFiber [] =
{
    0x0010, 0x0c2f,
    0x8182, 0x4000,     /* This and next lines are for yield rate improvement */
    0x8186, 0x003c,
    0x8300, 0x015d,     /* Force Auto Detect, Invert Signal Polarity */
    0x8301, 0x7,
    0x0,    0x1140,
    0x0010, 0x2c2f
};

static u16 serdesSet100MFiber [] =
{
    0x0010, 0x0c2f,
    0x8182, 0x4000,     /* This and next lines are for yield rate improvement */
    0x8186, 0x003c,
    0x8300, 0xd,
    0x8400, 0x14b,
    0x8402, 0x880,
    0x0010, 0x2c2f,
};

static u16 serdesSetAutoSGMII [] =
{
    0x0010, 0x0c2f,
    0x8182, 0x4000,     /* This and next lines are for yield rate improvement */
    0x8186, 0x003c,
    0x8300, 0x0100,
    0x0,    0x1140,
    0x0010, 0x2c2f
};
#endif

#if defined(CONFIG_I2C)
static void ethsw_conf_copper_sfp(int speed);
#endif

enum
{
    SFP_NO_MODULE,
    SFP_FIBER,
    SFP_COPPER,
};

static PHY_STAT ethsw_serdes_stat(int phyId);
static int sfp_module_type = SFP_NO_MODULE;
static int serdes_config_speed = BMCR_ANENABLE;
static int16 sfpDetectionGpio = -1;
static int16 phyExtResetGpio = BP_GPIO_NONE;
static int ethsw_sfp_link_status_changed;

enum {SFP_MODULE_OUT, SFP_MODULE_IN, SFP_LINK_UP};
static int sfp_status = SFP_MODULE_OUT;
static int serdes_power_mode = SERDES_BASIC_POWER_SAVING;

static void config_serdes(int phyId, u16 seq[], int seqSize)
{
    int i;

    seqSize /= sizeof(seq[0]);
    for(i=0; i<seqSize; i+=2)
    {
        ethsw_phy_write_reg_val(phyId, seq[i], seq[i+1], 1);
    }
}

static void ethsw_phy_autong_restart(int phyId)
{
    u16 val16;
    ethsw_phy_rreg(phyId, MII_CONTROL, &val16);
    val16 |= MII_CONTROL_RESTART_AUTONEG;
    ethsw_phy_wreg(phyId, MII_CONTROL, &val16);
}

int ethsw_phy_reset(int phyId)
{
    u16 v16;
    u32 reg;
    int i=0;

    /* Reset PHY to clear status first */
    reg = IsCL45GPhy(phyId)? CL45_REG_IEEE_CTRL: MII_CONTROL;

    v16= MII_CONTROL_RESET;
    ethsw_phy_wreg(phyId, reg, &v16);
    for(ethsw_phy_rreg(phyId, reg, &v16); v16 & MII_CONTROL_RESET;
            ethsw_phy_rreg(phyId, reg, &v16))
    {
        if (++i > 20) {printk("Failed to reset 0x%x\n", phyId); return 0;}
        msleep(100);
    }

    return 1;
}

#if defined(SERDES_2P5G_CAPABLE)
static int combophy_config_speed = BMCR_ANENABLE;
void ethsw_config_serdes_2p5g(int phyId)
{
    ethsw_phy_reset(phyId);
    config_serdes(phyId, serdesRef50mVco6p25, sizeof(serdesRef50mVco6p25));
    msleep(1);
    config_serdes(phyId, serdesSet2p5GFiber, sizeof(serdesSet2p5GFiber));
    ethsw_phy_autong_restart(phyId);
    msleep(50);
}

void ethsw_config_serdes_1kx_forced(int phyId)
{
    ethsw_phy_reset(phyId);
    config_serdes(phyId, serdesRef50mVco6p25, sizeof(serdesRef50mVco6p25));
    msleep(1);
    config_serdes(phyId, serdesSet1GForcedFiber, sizeof(serdesSet1GForcedFiber));
    msleep(50);
}

void ethsw_config_serdes_100M_forced_sgmii(int phyId)
{
    config_serdes(phyId, serdesSet100MForcedSGMII, sizeof(serdesSetAutoSGMII));
    ethsw_phy_autong_restart(phyId);
    msleep(50);
}

static void cl45_program_phy_seq(int phyId, uint32 seq[], int size)
{
    int i; 
    uint32 devAddr = 0, reg;
    static int idx = 0;

    for (i=0; i<size; i+=2)
    {
        if (seq[i] == 0xffffffff)
        {
            devAddr = seq[i+1] << 16;
            continue;
        }

        if (seq[i] < 0x10000)
        {
            reg = devAddr|seq[i];
        }
        else
        {
            reg = seq[i];
        }
        ethsw_phy_write_reg_val(phyId, reg, seq[i+1], 1);
    }
    idx++;
}

static uint32 halt_sequence1[] = 
{
    0xffffffff, 0x001e,   /* Set DeviceAddress 0x1e  */

    0x418c, 0x0000,
    0x4188, 0x48f0, 
    0x4186, 0x8000,
    0x4181, 0x017c,
    0x4181, 0x0040,
};

static uint32 halt_sequence2[] = 
{
	0xC3000000, 0x00000010,
	0xFFFF0000, 0xE59F1018,
	0xFFFF0004, 0xEE091F11,
	0xFFFF0008, 0xE3A00000,
	0xFFFF000C, 0xE3A01806,
	0xFFFF0010, 0xE8A00002,
	0xFFFF0014, 0xE1500001,
	0xFFFF0018, 0x3AFFFFFC,
	0xFFFF001C, 0xEAFFFFFE,
	0xFFFF0020, 0x00040021,
};

void halt_phy_fw_proc(int phyId)
{
    cl45_program_phy_seq(phyId, halt_sequence1, ARRAY_SIZE(halt_sequence1));
    cl45_program_phy_seq(phyId, halt_sequence2, ARRAY_SIZE(halt_sequence2));
}

static u16 get_word(char *fm, u16 *data)
{
    char buf[8];
    unsigned long res;
    int rc;

    memcpy(&buf[0], &fm[2], 2);
    memcpy(&buf[2], &fm[0], 2);
    buf[4] = 0;
    rc = kstrtoul(buf, 16, &res);
    *data = res;
    return rc;
}

static u32 fmCksum = 0;
int ethsw_phy_verify_firmware(int phy_id, u8 *firmware)
{
    u16 data_lo, data_hi;
    u32 v32, v32b, cksum = 0;
    int i, j, rc = 0;

    printk("   Verifing loaded firmware ...\n    ");
    for( i = 0, j = 0; i < strlen(firmware); i+=8)
    {
        get_word(&firmware[i], &data_lo);
        get_word(&firmware[i+4], &data_hi);
        v32b = (data_hi<<16)|data_lo;

        ethsw_phy_rreg32(phy_id, i/2, &v32, ETHCTL_FLAG_ACCESS_32BIT);

        cksum += v32;
        if ( v32 != v32b)
        {
            if((-rc) < 20)
                printk("    \n******* ERROR: PHY Firmware Loaded Mismatch: Addr: 0x%08x, Read: 0x%08x, Written: 0x%08x\n",
                i/2, v32, v32b);
            rc--;
        }

        if ((i%4000) == 0)
        {
            printk(".");
            j++;
        }

        if (j == 60)
        {
            printk("\n    ");
            j = 0;
        }
    }

    printk("\n");
    if (rc)
    {
        printk("    Total %d values read back wrong\n", -rc);
    }
    else if (cksum != fmCksum)
    {
        printk("    Wrong check sum: Rd:%08x != Wt:%08x\n", cksum, fmCksum);
    }
    else
    {
        printk("    Downloaded image read back check passed. Simple Checksum 0x%08x\n", cksum);
    }
    ethsw_phy_write_reg_val(phy_id, CL45_REG_DNLD_CTRL, 0, 1);
    return rc;
}

int ethsw_phy_load_firmware(int phyId)
{
    int rc = 0, i, j;
    u16 data_lo, data_hi;
    u32 phyDevId;
    u8 *firmware;

    phyDevId = (ethsw_phy_read_reg_val(phyId, CL45_REG(MII_DEVID1), 1) << 16)|
            ethsw_phy_read_reg_val(phyId, CL45_REG(MII_DEVID2), 1);

    for (i=0; i<ARRAY_SIZE(phyDesc); i++)
    {
        if (phyDesc[i].devId == phyDevId)
        {
            firmware = phyDesc[i].firmware;
            mountedCl45Phy = &phyDesc[i];
            printk("Found external PHY %s\n", phyDesc[i].devName);
            break;
        }
    }

    if (i==ARRAY_SIZE(phyDesc))
    {
        printk("Error: Unsupported PHY is attached: PHY_ID: %x, Device ID: %08x\n", 
            phyId, phyDevId);
        return -1;
    }

    /* Start loading firmware in */
    ethsw_phy_write_reg_val(phyId, CL45_REG_DNLD_ADDR_HI, 0, 1);
    ethsw_phy_write_reg_val(phyId, CL45_REG_DNLD_ADDR_LO, 0, 1);
    ethsw_phy_write_reg_val(phyId, CL45_REG_DNLD_CTRL, CL45_DNLD_DNLD_DWORD, 1);

    printk("Loading PHY firmware for device 0x%x ... \n    ", phyId);

    for( i = 0, j = 0; i < strlen(firmware); i+=8)
    {
        if ((rc = get_word(&firmware[i], &data_lo))) goto wrongFile;
        if ((rc = get_word(&firmware[i+4], &data_hi))) goto wrongFile;
        fmCksum += (data_hi<<16)|data_lo;
        ethsw_phy_write_reg_val(phyId, CL45_REG_DNLD_DATA_HI, data_hi, 1);
        ethsw_phy_write_reg_val(phyId, CL45_REG_DNLD_DATA_LO, data_lo, 1);
        if ((i%4000) == 0)
        {
            printk(".");
            j++;
        }

        if (j == 60)
        {
            printk("\n    ");
            j = 0;
        }
    }
    printk("\n    Done. Firmware Size = %d:0x%x bytes, Simple Checksum: 0x%08x\n", i/2, i/2, fmCksum);
    ethsw_phy_write_reg_val(phyId, CL45_REG_DNLD_CTRL, 0, 1);
    return 0;

wrongFile:
    printk("    ****** ERROR: Unknow string read from head file: offset: %d, hex: %c%c%c%c\n", i, 
        firmware[i], firmware[i+1], firmware[i+2], firmware[i+3]);
    ethsw_phy_write_reg_val(phyId, CL45_REG_DNLD_CTRL, 0, 1);
    return -1;
}

static int ethsw_cl45phy_init(int phyId)
{
    u16 v16;
    long myJiff;

    if (!IsCL45GPhy(phyId)) return -1;
    
    /* Halt PHY processor */
    halt_phy_fw_proc(phyId);

    /* Get Processor out of reset ? */
    ethsw_phy_write_reg_val(phyId, 0x1e4181, 0, 1);

    ethsw_phy_load_firmware(phyId);
    //ethsw_phy_verify_firmware(phyId);

    /* set Start Address VINIHI = 0 */
    ethsw_phy_write_reg_val(phyId, 0xc3000000, 0, 1);

    /* Reset and start processor */
    ethsw_phy_write_reg_val(phyId, 0x1a008, 0, 1); /* Set mail box */
    ethsw_phy_write_reg_val(phyId, 0x1e8004, 0x5555, 1);

    /* Issue reset */
    ethsw_phy_reset(phyId);

    /* Verify process is running */
    myJiff = jiffies;
    while(1)
    {
        ethsw_phy_read_reg(phyId, CL45_REG_IEEE_CTRL, &v16, 1);
        if (v16 == 0x2040) break;
        if ((1000*(jiffies - myJiff))/HZ >= 2000)
        {
            printk("****** ERROR: PHY Firmware Processor Failed to Start Running.\n");
            goto failure;
        }
    }
    printk("    PHY successfully came out of reset\n");

    /* Verify good CRC of loaded firmware */
    myJiff = jiffies;
    do 
    {
        if (v16 == 0x2040) break;
        if ((1000*(jiffies - myJiff))/HZ >= 3000)
        {
            printk("    ****** ERROR: PHY Firmware Processor Failed to Start Running.\n");
            goto failure;
        }
        ethsw_phy_read_reg(phyId, CL45_REG_PRIV_STATUS, &v16, 1);
        v16 &= CL45_REG_STATUS_CRC_CHK_MASK;
    }   while (v16 == CL45_REG_STATUS_CRC_CHK_CHKING);

    if (v16 == CL45_REG_STATUS_CRC_CHK_BAD)
    {
        printk("    ****** ERROR: PHY Firmware Loading Ended with Bad CRC.\n");
        goto failure;
    }
    printk("    PHY Firmware is loaded with Good CRC.\n");

    ethsw_phy_read_reg(phyId, CL45_REG_FMWR_REV, &v16, 1);
    printk ("    PHY Fimware Version(Main.Branch.Build): %d.%02d.%02d, ",
            (v16&CL45_REG_FW_VER_MAIN_MASK) >> CL45_REG_FW_VER_MAIN_SHFT,
            (v16&CL45_REG_FW_VER_BRCH_MASK),
            (v16&CL45_REG_FW_VER_BLD_MASK) >> CL45_REG_FW_VER_BLD_SHFT);

    ethsw_phy_read_reg(phyId, CL45_REG_FMWR_DATE, &v16, 1);
    printk ("Data(M/D/Y):%02d/%02d/%04d\n",
            (v16&CL45_REG_FMWR_MNTH_MASK)>>CL45_REG_FMWR_MNTH_SHFT,
            (v16&CL45_REG_FMWR_DAY_MASK)>>CL45_REG_FMWR_DAY_SHFT,
            (v16&CL45_REG_FMWR_YEAR_MASK)+2000);

    printk ("    PHY Firmware Loaded Successfully\n");

    return 0;
failure:
    return -1;

}

#define SET_1GFD(v) do { \
    ethsw_phy_rreg(phyId, CL45_REG_1G_CTL, &v16); \
    v16 &= ~CL45_REG_1G_CTL_1G_ADV_MASK; \
    if (v>0) v16 |= CL45_REG_1G_CTL_1G_FD_ADV; \
    ethsw_phy_wreg(phyId, CL45_REG_1G_CTL, &v16);} while(0)

#define SET_100MFD(v) do { \
    ethsw_phy_rreg(phyId, CL45_REG_COP_AN, &v16); \
    v16 &= ~CL45_REG_COP_AN_100M_ADV_MASK; \
    if (v>0) v16 |= CL45_REG_COP_AN_100M_FD_ADV; \
    ethsw_phy_wreg(phyId, CL45_REG_COP_AN, &v16);} while(0)

#define SET_2P5G(v) do { \
    if (IsBrcm2P5GPhy(mountedCl45Phy)) \
    { \
        ethsw_phy_rreg(phyId, CL45_REG_MGB_AN_CTL, &v16); \
        v16 = v>0? v16|CL45_REG_MGB_AN_2P5G_ADV|CL45_REG_MGB_ENABLE: \
        v16&~(CL45_REG_MGB_AN_2P5G_ADV|CL45_REG_MGB_ENABLE); \
        ethsw_phy_wreg(phyId, CL45_REG_MGB_AN_CTL, &v16); \
    } \
    else \
    { \
        ethsw_phy_rreg(phyId, CL45REG_10GBASET_AN_DEF_CTL, &v16); \
        v16 = v>0? v16|CL45_10GAN_2P5G_ABL: v16&~CL45_10GAN_2P5G_ABL; \
        ethsw_phy_wreg(phyId, CL45REG_10GBASET_AN_DEF_CTL, &v16); \
    } } while(0)

#define SET_10G(v) do { \
        ethsw_phy_rreg(phyIdExt, CL45REG_10GBASET_AN_DEF_CTL, &v16); \
        v16 &= ~CL45_10GAN_5G_ABL; \
        v16 = v>0? v16|CL45_10GAN_10G_ABL : v16&~CL45_10GAN_10G_ABL; \
        ethsw_phy_wreg(phyIdExt, CL45REG_10GBASET_AN_DEF_CTL, &v16);} while(0)

static void ethsw_combophy_init(int phyId, int phyIdExt)
{
    int speed = combophy_config_speed;
    ethsw_cl45phy_init(phyIdExt);

#if !defined(SERDES_10G_CAPABLE)
    {   /* Clear 5G/10G AN Advertisement bits */
        u16 v16;
        SET_10G(0);
    }
#endif
    combophy_config_speed = -1; /* Set to force initial configuration */
    ethsw_combophy_set_speed(phyId, phyIdExt, speed);
}

static PHY_STAT ethsw_combophy_stat(int phyId, int phyIdExt)
{
    PHY_STAT ps;
    u16 v16;

    ethsw_phy_rreg(phyIdExt, CL45_REG_UDEF_STATUS, &v16);
    memset(&ps, 0, sizeof(ps));
    if ((ps.lnk = (v16 & CL45_UDEF_STATUS_COPPER_LINK)>0) == 0)
    {
        if (serdes_power_mode > SERDES_NO_POWER_SAVING)
        {
            ETHSW_POWERDOWN_SERDES(phyId);
        }
        serdes_config_speed =  BMCR_ANENABLE;
        return ps;
    }

    if (serdes_config_speed == BMCR_ANENABLE)
    {
        ps.cfgAng = 1;
        ETHSW_POWERUP_SERDES(phyId);
    }

    switch (v16 & CL45_UDEF_STATUS_COPPER_SPD_M)
    {
        case CL45_UDEF_STATUS_COPPER_SPD_2P5G:
            if (serdes_config_speed != (BMCR_SPEED2500 | BMCR_FULLDPLX))
            {
                if (serdes_config_speed != BMCR_ANENABLE) ethsw_sfp_link_status_changed = 1;
                serdes_config_speed = BMCR_SPEED2500 | BMCR_FULLDPLX;
                ethsw_config_serdes_2p5g(phyId);
            }
            ps.cfgSpd2500Fdx = 1;
            ps.spd2500 = 1;
            ps.fdx = 1;
            break;
        case CL45_UDEF_STATUS_COPPER_SPD_1G:
            if (serdes_config_speed != (BMCR_SPEED1000 | BMCR_FULLDPLX))
            {
                if (serdes_config_speed != BMCR_ANENABLE) ethsw_sfp_link_status_changed = 1;
                serdes_config_speed = BMCR_SPEED1000 | BMCR_FULLDPLX;
                ethsw_config_serdes_1kx_forced(phyId);
            }
            ps.cfgSpd1000Fdx = 1;
            ps.spd1000 = 1;
            ps.fdx = 1;
            break;
        case CL45_UDEF_STATUS_COPPER_SPD_100M:
            if (serdes_config_speed != (BMCR_SPEED100 | BMCR_FULLDPLX))
            {
                if (serdes_config_speed != BMCR_ANENABLE) ethsw_sfp_link_status_changed = 1;
                serdes_config_speed = BMCR_SPEED100 | BMCR_FULLDPLX;
                ethsw_config_serdes_100M_forced_sgmii(phyId);
            }
            ps.cfgSpd100Fdx = 1;
            ps.spd100 = 1;
            ps.fdx = 1;
            break;
    }

    return ps;
}

void ethsw_combophy_get_speed(int phyId, int phyIdExt, int *phycfg, int *speed, int *duplex)
{
    PHY_STAT ps;

    ps = ethsw_combophy_stat(phyId, phyIdExt);
    if (phycfg)
    {
        switch(combophy_config_speed)
        {
            case BMCR_ANENABLE:
                *phycfg = PHY_CFG_AUTO_NEGO|PHY_CFG_2500FD|PHY_CFG_1000FD|PHY_CFG_100FD;
                break;
            case BMCR_SPEED2500|BMCR_FULLDPLX:
                *phycfg = PHY_CFG_2500FD;
                break;
            case BMCR_SPEED1000|BMCR_FULLDPLX:
                *phycfg = PHY_CFG_1000FD;
                break;
            case BMCR_SPEED100|BMCR_FULLDPLX:
                *phycfg = PHY_CFG_100FD;
                break;
            default:
                break;
        }
    }
    if (speed) *speed = !ps.lnk? 0: ps.spd2500? 2500: ps.spd1000? 1000: 100;
    if (duplex) *duplex = ps.fdx;
}

static void ethsw_cl45phy_set_speed(int phyId, int speed)
{
    u16 v16;

    speed &= ~BMCR_ANRESTART;
    if (speed != BMCR_ANENABLE &&
#if defined(SERDES_2P5G_CAPABLE)
            speed != (BMCR_SPEED2500 | BMCR_FULLDPLX) &&
#endif
            speed != (BMCR_SPEED1000 | BMCR_FULLDPLX) &&
            speed != (BMCR_SPEED100 | BMCR_FULLDPLX))
    {
        printk("Not Supported Speed %x attempted to set on this interface\n", speed);
        return;
    }

    if (speed == combophy_config_speed) return;;
    combophy_config_speed = speed;

    switch (combophy_config_speed)
    {
        case  BMCR_ANENABLE:
            SET_2P5G(1);
            SET_1GFD(1);
            SET_100MFD(1);
            break;

        case BMCR_SPEED2500|BMCR_FULLDPLX:
            SET_2P5G(1);
            SET_1GFD(0);
            SET_100MFD(0);
            break;

            case BMCR_SPEED1000|BMCR_FULLDPLX:
            SET_2P5G(0);
            SET_1GFD(1);
            SET_100MFD(0);
            break;

            case BMCR_SPEED100|BMCR_FULLDPLX:
            SET_2P5G(0);
            SET_1GFD(0);
            SET_100MFD(1);
            break;
    }

    if (!IsBrcm2P5GPhy(mountedCl45Phy))
    {
        /* Restart 10G level AN for IEEE2.5G */
        ethsw_phy_rreg(phyId, CL45REG_10GBASET_AN_CTL, &v16);
        v16 |= CL45_REG_10G_AN_ENABLE | CL45_REG_10G_AN_RESTART;
        ethsw_phy_wreg(phyId, CL45REG_10GBASET_AN_CTL, &v16);
    }

    if (phyExtResetGpio != BP_GPIO_NONE) {
        u16 uTimeOut = 0;
        /* clear bit 12 of 0x07 0x0020 (10GBASE-T AN Control)
           so we do not advertise 10G ability */
        ethsw_phy_rreg(phyId, 0x070020, &v16);
        v16 &= 0xefff;
        ethsw_phy_wreg(phyId, 0x070020, &v16);

        /* enable pair swapping a->d, b->c, c->b, d->a */
        do {
           ethsw_phy_rreg(phyId, 0x1e4037, &v16); //status reg
           uTimeOut++;
        }
        while (v16 != 0x0004 && v16 != 0x0008 && uTimeOut < 5000);
        printk("uTimeOut for status = %d\n", uTimeOut); 
        v16 = 0x001b;
        ethsw_phy_wreg(phyId, 0x1e4039, &v16); //data reg 2
        v16 = 0x8001; //CMD_SET_PAIR_SWAP
        ethsw_phy_wreg(phyId, 0x1e4005, &v16); //command reg
    }

    /* Restart Auto Negotiation for lG&Lower, MGB2.5G */
    ethsw_phy_rreg(phyId, CL45_REG_1G100M_CTL, &v16);
    v16 |= CL45_REG_1G100M_AN_ENABLED | CL45_REG_1G100M_AN_RESTART;
    ethsw_phy_wreg(phyId, CL45_REG_1G100M_CTL, &v16);

    return;
}

void ethsw_combophy_set_speed(int phyId, int phyIdExt, int speed)
{
    ethsw_cl45phy_set_speed(phyIdExt, speed);
}

#endif

int bcmeapi_ioctl_ethsw_combophy_mode(struct ethswctl_data *e, int phy_id, int phy_id_ext)
{
#if defined(SERDES_2P5G_CAPABLE)
    if (e->type == TYPE_SET)
    {
        ethsw_combophy_set_speed(phy_id, phy_id_ext, e->phycfg);
    }
    else
    {
        ethsw_combophy_get_speed(phy_id, phy_id_ext, &e->phycfg, &e->speed, &e->duplex);
    }
#endif
    return BCM_E_NONE;
}

void ethsw_config_serdes_1kx(int phyId)
{
    ethsw_phy_reset(phyId);
#if defined(SERDES_2P5G_CAPABLE)
    config_serdes(phyId, serdesRef50mVco6p25, sizeof(serdesRef50mVco6p25));
    msleep(1);
#endif
    config_serdes(phyId, serdesSet1GFiber, sizeof(serdesSet1GFiber));
    ethsw_phy_autong_restart(phyId);
    msleep(50);
}

void ethsw_config_serdes_100fx(int phyId)
{
    ethsw_phy_reset(phyId);
#if defined(SERDES_2P5G_CAPABLE)
    config_serdes(phyId, serdesRef50mVco6p25, sizeof(serdesRef50mVco6p25));
    msleep(1);
#endif
    config_serdes(phyId, serdesSet100MFiber, sizeof(serdesSet100MFiber));
    msleep(80);
}

void ethsw_config_serdes_auto_sgmii(int phyId)
{
    config_serdes(phyId, serdesSetAutoSGMII, sizeof(serdesSetAutoSGMII));
    ethsw_phy_autong_restart(phyId);
    msleep(50);
}

static void sgmiiResCal(int phyId)
{
    int val;
    u16 v16;

    v16 = RX_AFE_CTRL2_DIV4 | RX_AFE_CTRL2_DIV10;
    ethsw_phy_wreg(phyId, RX_AFE_CTRL2, &v16);

    if(GetRCalSetting(RCAL_1UM_VERT, &val) != kPMC_NO_ERROR)
    {
        printk("AVS is not turned on, leave SGMII termination resistor values as current default\n");
        ethsw_phy_rreg(phyId, PLL_AFE_CTRL1, &v16);
        printk("    PLL_PON: 0x%04x; ", (v16 & PLL_AFE_PLL_PON_MASK) >> PLL_AFE_PLL_PON_SHIFT);
        ethsw_phy_rreg(phyId, TX_AFE_CTRL2, &v16);
        printk("TX_PON: 0x%04x; ", (v16 & TX_AFE_TX_PON_MASK) >> TX_AFE_TX_PON_SHIFT);
        ethsw_phy_rreg(phyId, RX_AFE_CTRL0, &v16);
        printk("RX_PON: 0x%04x\n", (v16 & RX_AFE_RX_PON_MASK) >> RX_AFE_RX_PON_SHIFT);
        return;
    }

    val &= 0xf;
    printk("Setting SGMII Calibration value to 0x%x\n", val);

    ethsw_phy_rreg(phyId, PLL_AFE_CTRL1, &v16);
    v16 = (v16 & (~PLL_AFE_PLL_PON_MASK)) | (val << PLL_AFE_PLL_PON_SHIFT);
    ethsw_phy_wreg(phyId, PLL_AFE_CTRL1, &v16);

    ethsw_phy_rreg(phyId, TX_AFE_CTRL2, &v16);
    v16 = (v16 & (~TX_AFE_TX_PON_MASK)) | (val << TX_AFE_TX_PON_SHIFT);
    ethsw_phy_wreg(phyId, TX_AFE_CTRL2, &v16);

    ethsw_phy_rreg(phyId, RX_AFE_CTRL0, &v16);
    v16 = (v16 & (~RX_AFE_RX_PON_MASK)) | (val << RX_AFE_RX_PON_SHIFT);
    ethsw_phy_wreg(phyId, RX_AFE_CTRL0, &v16);
}

void ethsw_powerup_serdes(int powerLevel, int phyId)
{
    static int curPwrLvl=SERDES_POWER_DOWN;
    u32 val32;
    u16 val16;

    if (powerLevel == curPwrLvl)
        return;

    val32 = *(u32 *)SWITCH_REG_SINGLE_SERDES_CNTRL;
    switch(powerLevel)
    {
        case SERDES_POWER_ON:
            val32 |= SWITCH_REG_SERDES_RESETPLL|SWITCH_REG_SERDES_RESETMDIO|SWITCH_REG_SERDES_RESET;
            val32 &= ~(SWITCH_REG_SERDES_IQQD|SWITCH_REG_SERDES_PWRDWN);
            *(u32 *)SWITCH_REG_SINGLE_SERDES_CNTRL = val32;
            msleep(1);
            val32 &= ~(SWITCH_REG_SERDES_RESETPLL|SWITCH_REG_SERDES_RESETMDIO|SWITCH_REG_SERDES_RESET);
            *(u32 *)SWITCH_REG_SINGLE_SERDES_CNTRL = val32;

            /* Do dummy MDIO read to work around ASIC problem */
            ethsw_phy_rreg(phyId, 0, &val16);
            break;
        case SERDES_POWER_STANDBY:
            if (val32 & SWITCH_REG_SERDES_IQQD) {
                val32 |= SWITCH_REG_SERDES_PWRDWN;
                val32 &= ~SWITCH_REG_SERDES_IQQD;
                *(u32 *)SWITCH_REG_SINGLE_SERDES_CNTRL = val32;
                msleep(1);
            }
            // note lack of break here
        case SERDES_POWER_DOWN:
            val32 |= SWITCH_REG_SERDES_PWRDWN|SWITCH_REG_SERDES_RESETPLL|
                SWITCH_REG_SERDES_RESETMDIO|SWITCH_REG_SERDES_RESET;
            *(u32 *)SWITCH_REG_SINGLE_SERDES_CNTRL = val32;
            break;
        case SERDES_POWER_DOWN_FORCE:
            val32 |= SWITCH_REG_SERDES_PWRDWN|SWITCH_REG_SERDES_RESETPLL|
                SWITCH_REG_SERDES_RESETMDIO|SWITCH_REG_SERDES_RESET| 
                SWITCH_REG_SERDES_IQQD;
            *(u32 *)SWITCH_REG_SINGLE_SERDES_CNTRL = val32;
            break;            
        default:
            printk("Wrong power level request to Serdes module\n");
            return;
    }
    curPwrLvl = powerLevel;
}

void ethsw_init_serdes(void)
{
    int muxExtPort, phyId, phyIdExt, unit;
    u16 val16;

    for (unit = 0; unit < BP_MAX_ENET_MACS; unit++) {
        for (muxExtPort = 0; muxExtPort < BCMENET_CROSSBAR_MAX_EXT_PORTS; muxExtPort++) {
            enet_cb_port_get_phyids(unit, muxExtPort, &phyId, &phyIdExt);

            if( !IsSerdes(phyId) )
            {
                continue;
            }

                    /* to bring 49408eap board 2.5G phy out of reset */
            if (BpGetPhyResetGpio(unit, BP_CROSSBAR_PORT_TO_PHY_PORT(muxExtPort), &phyExtResetGpio) == BP_SUCCESS) {
                //printk("BpGetExtPhyResetGpio: muxExtPort %d phyResetGpio %d\n", muxExtPort, phyExtResetGpio);
                if (phyExtResetGpio != BP_GPIO_NONE) {
                    kerSysSetGpioDir(phyExtResetGpio);
                    kerSysSetGpioState(phyExtResetGpio, kGpioActive);
                    msleep(20);
                }
            }
            ETHSW_POWERUP_SERDES(phyId);
            ethsw_sfp_restore_from_power_saving(phyId);

            /* Enable GPIO 36 Input */
            if (BpGetSgmiiGpios(&val16) != BP_SUCCESS)
            {
                printk("Error: GPIO pin for Serdes not defined\n");
                return;
            }

            if(val16 != 28 && val16 != 36)
            {
                printk("Error: GPIO Pin %d for Serdes is not supported, correct boardparams.c definition.\n", val16);
                return;
            }

            if (val16 == 28)
            {
                MISC->miscSGMIIFiberDetect = 0;
                printk("GPIO Pin 28 is assigned to Serdes Fiber Signal Detection.\n");
            }
            else
            {
                MISC->miscSGMIIFiberDetect = MISC_SGMII_FIBER_GPIO36;
                printk("GPIO 36 is assigned to Serdes Fiber Signal Detection.\n");
            }

            /* read back for testing */
            ethsw_phy_rreg(phyId, MII_CONTROL, &val16);

            if (phyExtResetGpio != BP_GPIO_NONE) {
            /* for 49408EAP warm start issue */
            msleep(10);
        }

            /* Calibrate SGMII termination resistors */
            sgmiiResCal(phyId);

#if defined(SERDES_2P5G_CAPABLE)
            if (phyIdExt)
            {
                ethsw_combophy_init(phyId, phyIdExt);
            }
#endif
            ethsw_sfp_init(phyId);
        }
    }

}

int ethsw_serdes_get_cfgspeed(int phyId)
{
    return serdes_config_speed;
}
    
/* Quick check for link up/down only without checking speed */
static inline int ethsw_phyid_sfp_link_only(int phyId)
{
    uint16 v16;

    ethsw_phy_rreg(phyId, MII_STATUS, &v16);
    return (v16 & MII_STATUS_LINK) != 0;
}

PHY_STAT ethsw_serdes_conf_stats(int phyId)
{
    PHY_STAT ps;

    memset(&ps, 0, sizeof(ps));
    ps.cfgAng = serdes_config_speed == BMCR_ANENABLE;
    if (ps.cfgAng) {
        ps.cfgSpd1000Fdx = ps.cfgSpd100Fdx = 1;
    }
    else {
        ps.cfgSpd2500Fdx = (serdes_config_speed == (BMCR_SPEED2500|BMCR_FULLDPLX));
        ps.cfgSpd1000Fdx = (serdes_config_speed == (BMCR_SPEED1000|BMCR_FULLDPLX));
        ps.cfgSpd100Fdx = (serdes_config_speed == (BMCR_SPEED100|BMCR_FULLDPLX));
    }

    return ps;
}

static PHY_STAT ethsw_serdes_link_stats(int phyId)
{
    PHY_STAT ps;
    uint16 v16;

    ps = ethsw_serdes_conf_stats(phyId);
    if (ps.cfgAng)
    {
        ps.cfgSpd1000Fdx = ps.cfgSpd100Fdx = 1;
    }
    else {
        ps.cfgSpd1000Fdx = (serdes_config_speed == (BMCR_SPEED1000|BMCR_FULLDPLX));
        ps.cfgSpd100Fdx = (serdes_config_speed == (BMCR_SPEED100|BMCR_FULLDPLX));
    }

    ethsw_phy_rreg(phyId, MII_STATUS, &v16);
    ps.lnk = (v16 & MII_STATUS_LINK) != 0;

    if(ps.lnk)
    {
        ethsw_phy_rreg(phyId, MIIEX_DIGITAL_STATUS_1000X, &v16);
        ps.spd2500 = (v16 & MIIEX_SPEED) == MIIEX_SPD2500;
        ps.spd1000 = (v16 & MIIEX_SPEED) == MIIEX_SPD1000;
        ps.spd100 = (v16 & MIIEX_SPEED) == MIIEX_SPD100;
        ps.spd10 = (v16 & MIIEX_SPEED) == MIIEX_SPD10;
        ps.fdx = (v16 & MIIEX_DUPLEX) > 0;
    }

    return ps;
}

void ethsw_serdes_get_speed(int unit, int port, int cb_port, int *phycfg, int *speed, int *duplex)
{
    PHY_STAT ps;

    ps = ethsw_phy_stat(unit, port, cb_port);
    if (phycfg)
    {
        switch(serdes_config_speed)
        {
            case BMCR_ANENABLE:
                *phycfg = PHY_CFG_AUTO_NEGO|
#if defined(SERDES_2P5G_CAPABLE)
                    PHY_CFG_2500FD|
#endif
                    PHY_CFG_1000FD|PHY_CFG_100FD;
                break;
            case BMCR_SPEED1000|BMCR_FULLDPLX:
                *phycfg = PHY_CFG_1000FD;
                break;
            case BMCR_SPEED100|BMCR_FULLDPLX:
                *phycfg = PHY_CFG_100FD;
                break;
            default:
                break;
        }
    }
    if (speed) *speed = (sfp_status!=SFP_LINK_UP || !ps.lnk)?0: ps.spd2500?2500: ps.spd1000?1000: ps.spd100?100:10;
    if (duplex) *duplex = ps.fdx > 0;
}

#define SERDES_AUTO_DETECT_INT_MS 200
#define SERDES_FIBRL_SETTLE_DELAY_MS 400
/* 
    The testing result shows lower speed will be easier to link up
   during the fibre insertion, thus we are doing retry of the highest
   speed when linked in non highest speed.
*/
static void ethsw_serdes_speed_detect(int phyId)
{
    static int retry = 0;

    if (sfp_module_type != SFP_FIBER || serdes_config_speed != BMCR_ANENABLE) return;

#if defined(SERDES_2P5G_CAPABLE)
    /* First see if we can link at 2.5G */
    ethsw_config_serdes_2p5g(phyId);
    msleep(SERDES_AUTO_DETECT_INT_MS);
    if (ethsw_phyid_sfp_link_only(phyId)) return;
#endif
    
    /* See if we can link at 1kx */
    ethsw_config_serdes_1kx(phyId);
    msleep(SERDES_AUTO_DETECT_INT_MS);
    if (ethsw_phyid_sfp_link_only(phyId))
    {
#if defined(SERDES_2P5G_CAPABLE)
        goto LinkUp;
#else
        return;
#endif
    }

    /* See if we can link at 100Mbps */
    ethsw_config_serdes_100fx(phyId);
    msleep(SERDES_AUTO_DETECT_INT_MS);
    if (ethsw_phyid_sfp_link_only(phyId)) goto LinkUp;

    goto NoLinkUp;

LinkUp:
    if (retry) return; /* If we retried already, return; */

    /* Otherwise, take a sleep to let fibre settle down, then retry higher speed */
    retry++;
    msleep(SERDES_FIBRL_SETTLE_DELAY_MS);
    ethsw_serdes_speed_detect(phyId);
    goto end;

NoLinkUp:
    /* 
        No link up here.
        Set speed to highest when in NO_POWER_SAVING_MODE until next detection
    */
    if( serdes_power_mode == SERDES_NO_POWER_SAVING)
    {
#if defined(SERDES_2P5G_CAPABLE)
        ethsw_config_serdes_2p5g(phyId);
#else
        ethsw_config_serdes_1kx(phyId);
#endif
    }
end:
    retry = 0;
}

void ethsw_sfp_restore_from_power_saving(int phyId)
{
    if (sfp_module_type == SFP_NO_MODULE)
        return;

#if defined(SERDES_2P5G_CAPABLE)
    config_serdes(phyId, serdesRef50mVco6p25, sizeof(serdesRef50mVco6p25));
    msleep(1);
#endif

#if defined(CONFIG_I2C)
    if(sfp_module_type == SFP_COPPER)
    {
        /* Configure Serdes into SGMII mode */
        ethsw_config_serdes_auto_sgmii(phyId);
    }
    else
#endif
    {
        switch(serdes_config_speed)
        {
            case BMCR_ANENABLE:
                ethsw_serdes_speed_detect(phyId);
                break;
#if defined(SERDES_2P5G_CAPABLE)
            case BMCR_SPEED2500|BMCR_FULLDPLX:
                ethsw_config_serdes_2p5g(phyId);
                break;
#endif
            case BMCR_SPEED1000|BMCR_FULLDPLX:
                ethsw_config_serdes_1kx(phyId);
                break;
            case BMCR_SPEED100|BMCR_FULLDPLX:
                ethsw_config_serdes_100fx(phyId);
                break;
            default:
                break;
        }
    }
}

void ethsw_serdes_set_speed(int phyId, int phyCfg)
{
    phyCfg &= ~BMCR_ANRESTART;
    if (phyCfg != BMCR_ANENABLE &&
            phyCfg != (BMCR_SPEED2500 | BMCR_FULLDPLX) &&
            phyCfg != (BMCR_SPEED1000 | BMCR_FULLDPLX) &&
            phyCfg != (BMCR_SPEED100 | BMCR_FULLDPLX))
    {
        printk("Not Supported phyCfg %x attempted to set to Serdes\n", phyCfg);
        return;
    }

    if (phyCfg == serdes_config_speed) return;;
    serdes_config_speed = phyCfg;

    switch (sfp_module_type)
    {
        case SFP_NO_MODULE:
            break;

        case SFP_FIBER:
            ETHSW_POWERDOWN_SERDES(phyId);
            ETHSW_POWERUP_SERDES(phyId);
            ethsw_sfp_restore_from_power_saving(phyId);
            ethsw_serdes_stat(phyId);
            break;

#if defined(CONFIG_I2C)
        case SFP_COPPER:
            ethsw_conf_copper_sfp(phyCfg);
            break;
#endif

    }
    ethsw_sfp_link_status_changed = 1;
    return;
}

#if defined(CONFIG_I2C)
static int sfp_i2c_module_detect(int phyId)
{
    u32 val32;

    /* If I2C read operation succeeds, I2C module is connected
       and which means it is a copper SFP module */
    if (sfp_i2c_phy_read(0, &val32))
    {
        sfp_module_type = SFP_COPPER;
        ETHSW_POWERUP_SERDES(phyId);
        ethsw_conf_copper_sfp(serdes_config_speed);
        ethsw_config_serdes_auto_sgmii(phyId);

        printk("Copper SFP Module Plugged in\n");
        return 1;
    }
    return 0;
}

static void ethsw_conf_copper_sfp(int speed)
{
    uint16 v16;

    sfp_i2c_phy_write(MII_CONTROL, MII_CONTROL_RESET);     /* Software reset */

    /* Configure SFP PHY into SGMII mode */
    sfp_i2c_phy_write(0x1b, 0x9084);    /* Enable SGMII mode */
    sfp_i2c_phy_write(MII_CONTROL, MII_CONTROL_RESET);     /* Software reset */

    if (speed & BMCR_ANENABLE)
    {
        sfp_i2c_phy_write(0x9, 0x0f00);     /* Advertise 1kBase-T Full/Half-Duplex */
        sfp_i2c_phy_write(0x0, 0x8140);     /* Software reset */
        sfp_i2c_phy_write(0x4, 0x0de1);     /* Adverstize 100/10Base-T Full/Half-Duplex */
        sfp_i2c_phy_write(0x0, 0x9140);     /* Software reset */

        return;
    }

    /* Fixed speed configuration */
    v16 = MII_CONTROL_RESET;
    if (speed & BMCR_SPEED1000)
    {
        v16 |= MII_CONTROL_SPEED_1000M;
    }
    else if (speed & BMCR_SPEED100)
    {
        v16 |= MII_CONTROL_SPEED_100M;
    }

    if (speed & BMCR_FULLDPLX)
    {
        v16 |= MII_CONTROL_DUPLEX_MODE;
    }
    sfp_i2c_phy_write(MII_CONTROL, v16);
}
#endif

void ethsw_serdes_power_mode_set(int phy_id, int mode)
{
    if (serdes_power_mode == mode)
        return;

    serdes_power_mode = mode;

    if (sfp_status == SFP_LINK_UP)
        return;

    if(mode == SERDES_NO_POWER_SAVING)
    {
        ETHSW_POWERUP_SERDES(phy_id);
        ethsw_sfp_restore_from_power_saving(phy_id);
    }
    else if (mode == SERDES_FORCE_OFF)
        ETHSW_POWERDOWN_FORCE_OFF_SERDES(phy_id);
    else
        ETHSW_POWERDOWN_SERDES(phy_id);
}

void ethsw_serdes_power_mode_get(int phy_id, int *mode)
{
    *mode = serdes_power_mode;
}

static int ethsw_sfp_module_detected(void)
{
    if (sfpDetectionGpio != -1)
    {
        return kerSysGetGpioValue(sfpDetectionGpio) == 0;
    }
    else
    {
        return ((*(u32*)SWITCH_SINGLE_SERDES_STAT) & SWITCH_REG_SSER_RXSIG_DET) > 0;
    }
}

/*
   Module detection is not going through SGMII,
   so it can be down even under SGMII power down.
 */
static int ethsw_sfp_module_detect(int phyId)
{
#if defined(CONFIG_I2C)
#define I2CDetectDelay 8
#define I2CInitDetectDelay 8
    static int i2cDetectDelay = 0;
    static int i2cInitDetectDelay = 0;
    if (i2cInitDetectDelay++ < I2CInitDetectDelay) return 0;
#endif

    if ( ethsw_sfp_module_detected() == 0)
    {
        if(sfp_module_type != SFP_NO_MODULE)
        {
            sfp_module_type = SFP_NO_MODULE;
            printk("SFP module unplugged\n");
        }
        return 0;
    }

    if (sfp_module_type == SFP_NO_MODULE)
    {
#if defined(CONFIG_I2C)
        if (!sfp_i2c_module_detect(phyId))
#endif
        {
            /* Configure Serdes into 1000Base-X mode */
            sfp_module_type = SFP_FIBER;

#if defined(CONFIG_I2C)
            i2cDetectDelay = 0;
            printk("Fibre SFP Module Plugged in\n");
#else
            printk("SFP Module Plugged in\n");
#endif
        }
    }
#if defined(CONFIG_I2C)
    else    /* MODULE is plug in */
    {
        /* Work around for some I2C long initialization, do more I2C after Fiber type detected */
        if (sfp_module_type == SFP_FIBER && i2cDetectDelay < I2CDetectDelay)
        {
            if (sfp_i2c_module_detect(phyId))
            {
                ETHSW_POWERDOWN_SERDES(phyId);  /* Power down and up Serdes to reset Serdes Status */
                ETHSW_POWERUP_SERDES(phyId);
                i2cDetectDelay = I2CDetectDelay;
            }
            else
            {
                i2cDetectDelay++;
            }
        }
    }
#endif
    return 1;
}

void ethsw_sfp_init(int phyId)
{
#if defined(CONFIG_I2C)
    volatile u32 val32; 
    u32 v32;
    sfp_i2c_phy_read(0, &v32);    /* Dummy read to trigger I2C hardware prepared */
    val32 = v32;
    msleep(1);
#endif

    if (BpGetSfpDetectGpio(&sfpDetectionGpio) == BP_SUCCESS)
    {
        kerSysSetGpioDirInput(sfpDetectionGpio);
        printk("GPIO Pin %d is configured as SPF MOD_ABS for module insertion detection\n", sfpDetectionGpio);
    }
    else
    {
        sfpDetectionGpio = -1;
        printk("Energy detection is used as SFP module insertion detection\n");
    }

    /* Call the function to init state machine without leaving
       SFP on without control during initialization */
    ethsw_serdes_stat(phyId);
}

static PHY_STAT ethsw_serdes_stat(int phyId)
{
    PHY_STAT ps;

    memset(&ps, 0, sizeof(ps));
    ps = ethsw_serdes_conf_stats(phyId);
    if (serdes_power_mode == SERDES_FORCE_OFF) {
        //should already be powered down -- just exit
        goto sfp_end;
    }

    if (serdes_power_mode > SERDES_NO_POWER_SAVING && sfp_status < SFP_LINK_UP) 
        ETHSW_POWERSTANDBY_SERDES(phyId);

    switch (sfp_status)
    {
        case SFP_MODULE_OUT:
sfp_module_out:
            if(sfp_status == SFP_MODULE_OUT && ethsw_sfp_module_detect(phyId))
                goto sfp_module_in;

            sfp_status = SFP_MODULE_OUT;
            goto sfp_end;

        case SFP_MODULE_IN:
sfp_module_in:
            if(sfp_status >= SFP_MODULE_IN && !ethsw_sfp_module_detect(phyId))
            {
                sfp_status = SFP_MODULE_IN;
                goto sfp_module_out;
            }

            if(sfp_status <= SFP_MODULE_IN)
            {
                if(serdes_power_mode == SERDES_BASIC_POWER_SAVING)
                {
                    ETHSW_POWERUP_SERDES(phyId);
                    ethsw_sfp_restore_from_power_saving(phyId);
                }
                else if (serdes_power_mode == SERDES_NO_POWER_SAVING)
                {
                    ethsw_serdes_speed_detect(phyId);
                }

                ps = ethsw_serdes_link_stats(phyId);
                if(ps.lnk)
                {
                    sfp_status = SFP_MODULE_IN;
                    goto sfp_link_up;
                }
            }
            sfp_status = SFP_MODULE_IN;
            goto sfp_end;

        case SFP_LINK_UP:
sfp_link_up:
            if(sfp_status == SFP_LINK_UP)
            {
                if(!ethsw_sfp_module_detect(phyId)) goto sfp_module_out;
                ps = ethsw_serdes_link_stats(phyId);
                if(!ps.lnk)
                {
                    goto sfp_module_in;
                }
            }
            sfp_status = SFP_LINK_UP;
            goto sfp_end;
    }

sfp_end:
    if( serdes_power_mode > SERDES_NO_POWER_SAVING && sfp_status != SFP_LINK_UP)
        ETHSW_POWERDOWN_SERDES(phyId);
    return ps;
}

#endif

PHY_STAT ethsw_phyid_stat(int phyId)
{
    PHY_STAT ps;
    uint16 v16, misc;
    uint16 ctrl, gig_ctrl;
    uint16 mii_esr = 0;
    uint16 mii_stat = 0, mii_adv = 0, mii_lpa = 0;
    uint16 mii_gb_ctrl = 0, mii_gb_stat = 0;

    memset(&ps, 0, sizeof(ps));
    
    if (!IsPhyConnected(phyId))
    {
        ps.lnk = 1;
        ps.fdx = 1;
        if (IsMII(phyId))
            ps.spd100 = 1;
        else
            ps.spd1000 = 1;
        return ps;
    }

    down(&bcm_ethlock_switch_config);

#if defined(SWITCH_REG_SINGLE_SERDES_CNTRL)
    if( IsSerdes(phyId) )
    {
        /* Create a link down event to notify MAC,
           so that MAC layer don't need to remember previous speed etc. */
        if (ethsw_sfp_link_status_changed)
        {
            ethsw_sfp_link_status_changed = 0;
            goto end;
        }

        ps = ethsw_serdes_stat(phyId);
        goto end;
    }
#endif  /* SWITCH_REG_SINGLE_SERDES_CNTRL */

    ethsw_phy_rreg(phyId, MII_INTERRUPT, &v16);
    ethsw_phy_rreg(phyId, MII_ASR, &v16);

    ethsw_phy_rreg(phyId, MII_BMCR, &ctrl);
    ethsw_phy_rreg(phyId, MII_BMSR, &mii_stat);
    ethsw_phy_rreg(phyId, MII_CTRL1000, &gig_ctrl);

    ps.lnk = (mii_stat & BMSR_LSTATUS) > 0;

    if ((ctrl & BMCR_ANENABLE) == 0) { /* Non Auto Nego case */
        ps.cfgSpd1000Fdx = (ctrl & BMCR_SPEED1000) && !(ctrl & BMCR_SPEED100) && (ctrl & BMCR_FULLDPLX);
        ps.cfgSpd1000Hdx = (ctrl & BMCR_SPEED1000) && !(ctrl & BMCR_SPEED100) && !(ctrl & BMCR_FULLDPLX);
        ps.cfgSpd100Fdx = !(ctrl & BMCR_SPEED1000) && (ctrl & BMCR_SPEED100) && (ctrl & BMCR_FULLDPLX);
        ps.cfgSpd100Hdx = !(ctrl & BMCR_SPEED1000) && (ctrl & BMCR_SPEED100) && !(ctrl & BMCR_FULLDPLX);
        ps.cfgSpd10Fdx = !(ctrl & BMCR_SPEED1000) && !(ctrl & BMCR_SPEED100) && (ctrl & BMCR_FULLDPLX);
        ps.cfgSpd10Hdx = !(ctrl & BMCR_SPEED1000) && !(ctrl & BMCR_SPEED100) && !(ctrl & BMCR_FULLDPLX);

        ps.fdx = (ctrl & BMCR_FULLDPLX) ? 1 : 0;
    
        if(ps.lnk) {
            ps.spd1000 = ps.cfgSpd1000Fdx || ps.cfgSpd1000Hdx;
            ps.spd100 = ps.cfgSpd100Fdx || ps.cfgSpd100Hdx;
            ps.spd10 = ps.cfgSpd10Fdx || ps.cfgSpd10Hdx;
        }
        goto end;
    }
    else { /* Auto Nego case */
        ethsw_phy_rreg(phyId, MII_ADVERTISE, &mii_adv);
        ethsw_phy_rreg(phyId, MII_LPA, &mii_lpa);

        ps.cfgAng = 1;
        ps.cfgSpd1000Fdx = (gig_ctrl & ADVERTISE_1000FULL) > 0;
        ps.cfgSpd1000Hdx = (gig_ctrl & ADVERTISE_1000HALF) > 0;
        ps.cfgSpd100Fdx = (mii_adv & ADVERTISE_100FULL) > 0;
        ps.cfgSpd100Hdx = (mii_adv & (ADVERTISE_100HALF|ADVERTISE_100BASE4)) > 0;
        ps.cfgSpd10Fdx = (mii_adv & ADVERTISE_10FULL) > 0;
        ps.cfgSpd10Hdx = (mii_adv & ADVERTISE_10HALF) > 0;

        if (ps.lnk == 0) goto end;

        if (ps.cfgSpd1000Fdx || ps.cfgSpd1000Hdx) { // only GE phy support ethernet@wirespeed
            // check if ethernet@wirespeed is enabled, reg 0x18, shodow 0b'111, bit4
            misc = 0x7007;
            ethsw_phy_wreg(phyId, 0x18, &misc);
            ethsw_phy_rreg(phyId, 0x18, &misc);
            if(misc & 0x0010) { /* get link speed from ASR if ethernet@wirespeed is enabled */
                if (MII_ASR_1000(v16) && MII_ASR_FDX(v16)) {
                    ps.spd1000 = 1;
                    ps.fdx = 1;
                } else if (MII_ASR_1000(v16) && !MII_ASR_FDX(v16)) {
                    ps.spd1000 = 1;
                    ps.fdx = 0;            
                } else if (MII_ASR_100(v16) && MII_ASR_FDX(v16)) {
                    ps.spd100 = 1;
                    ps.fdx = 1;
                } else if (MII_ASR_100(v16) && !MII_ASR_FDX(v16)) {
                    ps.spd100 = 1;
                    ps.fdx = 0;
                } else if (MII_ASR_10(v16) && MII_ASR_FDX(v16)) {
                    ps.spd10 = 1;
                    ps.fdx = 1;
                } else if (MII_ASR_10(v16) && !MII_ASR_FDX(v16)) {
                    ps.spd10 = 1;
                    ps.fdx = 0;
                }
                goto end;
            }
        }

        // read 1000mb Phy  registers if supported
        if (mii_stat & BMSR_ESTATEN) {
            ethsw_phy_rreg(phyId, MII_ESTATUS, &mii_esr);
            if (mii_esr & (1 << 15 | 1 << 14 |
                        ESTATUS_1000_TFULL | ESTATUS_1000_THALF))
                ethsw_phy_rreg(phyId, MII_CTRL1000, &mii_gb_ctrl);
            ethsw_phy_rreg(phyId, MII_STAT1000, &mii_gb_stat);
        }

        mii_adv &= mii_lpa;
        if ((mii_gb_ctrl & ADVERTISE_1000FULL) &&  // 1000mb Adv
                (mii_gb_stat & LPA_1000FULL))
        {
            ps.spd1000 = 1;
            ps.fdx = 1;
        } else if ((mii_gb_ctrl & ADVERTISE_1000HALF) &&
                (mii_gb_stat & LPA_1000HALF))
        {
            ps.spd1000 = 1;
            ps.fdx = 0;
        } else if (mii_adv & ADVERTISE_100FULL) {  // 100mb adv
            ps.spd100 = 1;
            ps.fdx = 1;
        } else if (mii_adv & ADVERTISE_100BASE4) {
            ps.spd100 = 1;
            ps.fdx = 0;
        } else if (mii_adv & ADVERTISE_100HALF) {
            ps.spd100 = 1;
            ps.fdx = 0;
        } else if (mii_adv & ADVERTISE_10FULL) {
            ps.spd10 = 1;
            ps.fdx = 1;
        }
        else {
            ps.spd10 = 1;
            ps.fdx = 0;
        }
    }
end:
    up(&bcm_ethlock_switch_config);
    return ps;
}

int bcmeapi_ioctl_ethsw_phy_mode(struct ethswctl_data *e, int phy_id)
{
    PHY_STAT ps;

    if (e->type == TYPE_GET)
    {
#if defined(SWITCH_REG_SINGLE_SERDES_CNTRL)
        if ((phy_id & MAC_IFACE) == MAC_IF_SERDES )
        {
            ethsw_serdes_get_speed(e->unit, e->port, e->sub_port, &e->phycfg, &e->speed, &e->duplex);
        }
        else
#endif
        {
            ps = ethsw_phyid_stat(phy_id);
            e->phycfg = (ps.cfgAng? PHY_CFG_AUTO_NEGO:0)|
                (ps.cfgSpd1000Fdx?PHY_CFG_1000FD:0)|(ps.cfgSpd1000Hdx?PHY_CFG_1000HD:0)|
                (ps.cfgSpd100Fdx?PHY_CFG_100FD:0)|(ps.cfgSpd100Hdx?PHY_CFG_100HD:0)|
                (ps.cfgSpd10Fdx?PHY_CFG_10FD:0)|(ps.cfgSpd10Hdx?PHY_CFG_10HD:0);

            if (ps.lnk)
            {
                e->speed = ps.spd2500?2500: ps.spd1000?1000: ps.spd100?100:10;
                e->duplex = ps.fdx > 0; 
            }
            else
            {
                e->speed = 0;
            }
        }

        return BCM_E_NONE;
    }
    else
    {
#if defined(SWITCH_REG_SINGLE_SERDES_CNTRL)
        if ((phy_id & MAC_IFACE) == MAC_IF_SERDES )
        {
            ethsw_serdes_set_speed(phy_id, e->phycfg);
            return BCM_E_NONE;
        }
#endif
        {
            unsigned short bmcr = 0;
            unsigned short nway_advert, gig_ctrl, gig_cap = 0;

            switch (e->speed) 
            {
              case 0:
                bmcr = BMCR_ANENABLE | BMCR_ANRESTART;
                break;
              case 10:
                bmcr = 0;
                break;
              case 100:
                bmcr = BMCR_SPEED100;
                break;
              case 1000:
                bmcr = BMCR_SPEED1000;
                bmcr |= (BMCR_ANENABLE | BMCR_ANRESTART);
                gig_cap = (e->duplex == 1) ? ADVERTISE_1000HALF : ADVERTISE_1000FULL;
                break;
              default:
                bmcr = BMCR_ANENABLE | BMCR_ANRESTART;
                break;
            }

            if (e->speed != 0)
            {
                bmcr |= (e->duplex == 1) ? BMCR_FULLDPLX : 0;
            }

            ethsw_phyport_rreg2(phy_id, MII_ADVERTISE, &nway_advert, 0);
            ethsw_phyport_rreg2(phy_id, MII_CTRL1000, &gig_ctrl, 0);
            gig_ctrl |= (ADVERTISE_1000FULL | ADVERTISE_1000HALF);

            if (e->speed == 1000)
            {
                nway_advert &= ~(ADVERTISE_100BASE4 |
                            ADVERTISE_100FULL |
                            ADVERTISE_100HALF |
                            ADVERTISE_10FULL |
                            ADVERTISE_10HALF );
                gig_ctrl &= ~gig_cap; 
            }
            else 
            {
                nway_advert |= (ADVERTISE_100BASE4 |
                            ADVERTISE_100FULL |
                            ADVERTISE_100HALF |
                            ADVERTISE_10FULL |
                            ADVERTISE_10HALF );
            }

            ethsw_phyport_wreg2(phy_id, MII_ADVERTISE, &nway_advert, 0);
            ethsw_phyport_wreg2(phy_id, MII_CTRL1000, &gig_ctrl, 0);
            ethsw_phyport_wreg2(phy_id, MII_BMCR, &bmcr, 0);
        }
    }

    e->ret_val = 0;
    return BCM_E_NONE;
}

PHY_STAT ethsw_phy_stat(int unit, int port, int cb_port)
{
    PHY_STAT phys = {0};
    int phyId;

    if ((unit == 0) && port_in_loopback_mode[port])
    {
        return phys;
    }

    if ( cb_port == BP_CROSSBAR_NOT_DEFINED )
    {
        phyId = enet_sw_port_to_phyid(unit, port);
    }
    else
    {
        phyId = enet_cb_port_to_phyid(unit, cb_port);
    }
    
    if ( !IsPhyConnected(phyId) )
    {
        phys = enet_get_ext_phy_stat(unit, port, cb_port);
        /* is phys valid */
        if ( (phys.spd10 | phys.spd100 | phys.spd1000) > 0 )
        {
            return phys;
        }
    }

#if defined(SERDES_2P5G_CAPABLE)
    if (enet_cb_port_has_combo_phy(unit, cb_port))
    {
        int phyIdExt;
        enet_cb_port_get_phyids(unit, cb_port, &phyId, &phyIdExt);
        return ethsw_combophy_stat(phyId, phyIdExt);
    }
    else
#endif
    {
        return ethsw_phyid_stat(phyId);
    }
}

void ethsw_isolate_phy(int phyId, int isolate)
{
    uint16 v16;
    ethsw_phy_rreg(phyId, MII_CONTROL, &v16);
    if (isolate) {
        v16 |= MII_CONTROL_ISOLATE_MII;
    } else {
        v16 &= ~MII_CONTROL_ISOLATE_MII;
    }
    ethsw_phy_wreg(phyId, MII_CONTROL, &v16);
}

void ethsw_isolate_phys_pmap(int pMap, int isolate)
{
    ETHERNET_MAC_INFO *EnetInfo = EnetGetEthernetMacInfo();
    int unit, i, phyId, log_port, cb_port;

    for (unit=0; unit < BP_MAX_ENET_MACS; unit++)
    {
        for (i = 0; i < BP_MAX_SWITCH_PORTS; i++)
        {
            log_port = PHYSICAL_PORT_TO_LOGICAL_PORT(i, unit);
            if ((pMap & (1<<log_port)) == 0) continue;

            cb_port = enet_get_first_crossbar_port(log_port);
            if (cb_port == BP_CROSSBAR_NOT_DEFINED)
            {
                phyId = EnetInfo[unit].sw.phy_id[i];
                ethsw_isolate_phy(phyId, isolate);
            }
            else
            {
                for (;cb_port != BP_CROSSBAR_NOT_DEFINED;
                        cb_port = enet_get_next_crossbar_port(log_port, cb_port))
                {
                    phyId = EnetInfo[unit].sw.crossbar[cb_port].phy_id;
                    ethsw_isolate_phy(phyId, isolate);
                }
            }
        }
    }
    msleep(100);
}



u32 ethsw_phy_read_reg_val(int phy_id, u32 reg, int ext_bit)
{
    u32 v32;
    ethsw_phy_rreg32(phy_id, reg, &v32, ext_bit);
    return v32;
}

void ethsw_phy_write_reg_val(int phy_id, u32 reg, u32 data, int ext_bit)
{
    ethsw_phy_wreg32(phy_id, reg, &data, ext_bit);
}

int ethsw_phy_read_reg(int phy_id, u32 reg, u16 *data, int ext_bit)
{
    u32 _data;
    int rc;

    rc = ethsw_phy_rreg32(phy_id, reg, &_data, ext_bit);
    *data = _data;
    return rc;
}

int ethsw_phy_write_reg(int phy_id, u32 reg, u16 *data, int ext_bit)
{
    u32 _data = *data;
    return ethsw_phy_wreg32(phy_id, reg, &_data, ext_bit);
}

typedef struct cl45_cmd_t {
    int cmd,
        status,
        data[5];
} cl45_cmd_t;

/* Broadcom MII Extended Register Access Driver */
/* Do not invoke this directly, use ethsw_ephy_shadow_rw instead */
void _ethsw_ephy_shadow_rw(int phy_id, int bank, uint16 reg, uint16 *data, int write)
{
    uint16 v16;
    
    switch (bank)
    {
        case 1: 
            v16 = 0x8b;
            ethsw_phy_wreg(phy_id, BRCM_MIIEXT_BANK, &v16);
            break;
        case 2:
        case 3:
            v16 = 0xf;
            ethsw_phy_wreg(phy_id, BRCM_MIIEXT_BANK, &v16);
            break;
    }

    if (bank == 3)
    {
        if (write)
        {
            ethsw_phy_wreg(phy_id, 0xe, &reg);
            ethsw_phy_wreg(phy_id, 0xf, data);
        }
        else
        {
            ethsw_phy_wreg(phy_id, 0xe, &reg);
            ethsw_phy_rreg(phy_id, 0xf, data);
        }
    }
    else
    {
        if (write)
        {
            ethsw_phy_wreg(phy_id, reg, data);
        }
        else 
        {
            ethsw_phy_rreg(phy_id, reg, data);
        }
    }
    v16 = 0xb;
    ethsw_phy_wreg(phy_id, BRCM_MIIEXT_BANK, &v16);

    return;
}

void ethsw_ephy_write_bank1_reg(int phy_id, uint16 reg, uint16 val)
{
    ethsw_ephy_shadow_write(phy_id, 1, reg, &val);
}

void ethsw_ephy_write_bank2_reg(int phy_id, uint16 reg, uint16 val)
{
    ethsw_ephy_shadow_write(phy_id, 2, reg, &val);
}

void ethsw_ephy_write_bank3_reg(int phy_id, uint16 reg, uint16 val)
{
    ethsw_ephy_shadow_write(phy_id, 3, reg, &val);
}

DEFINE_MUTEX(bcm_phy_exp_mutex);
/*  
    Main Entrance of PHY register access function.
    All wrapper function should come to here.
    return: 0-Good; -1: Bad 
*/ 
int _ethsw_phy_exp_rw_reg_flag(int phy_id, u32 reg, u32 *data_p, int ext, int write)
{
    u32 bank, v32, offset;
    int rc;

    /* If the register falls within standard MII address 
        or address is CL45 address, call standard drivers */
    if (reg <= BRCM_MIIEXT_BANK || reg > 0xffff || (ext & ETHCTL_FLAG_ACCESS_32BIT))
    {
        return ethsw_phy_rw_reg32_chip(phy_id, reg, data_p, ext, !write);
    }

    bank = reg & BRCM_MIIEXT_BANK_MASK;
    offset = (reg & BRCM_MIIEXT_OFF_MASK) + BRCM_MIIEXT_OFFSET;
    mutex_lock(&bcm_phy_exp_mutex);
    /* Set Bank Address */
    rc = ethsw_phy_rw_reg32_chip(phy_id, BRCM_MIIEXT_BANK, &bank, ext, 0);

    /* Set offset address */
    rc += ethsw_phy_rw_reg32_chip(phy_id, offset, data_p, ext, !write);

    /* Set Bank back to default for standard access */
    if(bank != BRCM_MIIEXT_DEF_BANK || offset == BRCM_MIIEXT_OFFSET)
    {
        v32 = BRCM_MIIEXT_DEF_BANK;
        rc += ethsw_phy_rw_reg32_chip(phy_id, BRCM_MIIEXT_BANK, &v32, ext, 0);
    }
    mutex_unlock(&bcm_phy_exp_mutex);
    return rc;
}
MODULE_LICENSE("GPL");

