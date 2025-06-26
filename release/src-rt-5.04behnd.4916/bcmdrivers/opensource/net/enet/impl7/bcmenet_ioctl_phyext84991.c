#include <linux/version.h>
#include "enet.h"
#include "port.h"
#include "enet_dbg.h"
#include <bcm/bcmswapitypes.h>
#include <board.h>
#include <bcmnet.h>
#ifdef SF2_DEVICE
#include "sf2.h"
#endif
#if defined(DSL_DEVICES)
#include "phy_drv_dsl_phy.h"
#include "phy_drv_dsl_serdes.h"
void extlh_link_change_handler(enetx_port_t *port, int linkstatus, int speed, int duplex);
#endif
#include "opticaldet.h"
#include "bcmenet_common.h"
#include "crossbar_dev.h"
#include "phy_drv.h"
#include "phy_drv_brcm.h"
#include "phy_macsec_common.h"
#include "clk_rst.h"
#include "bcmenet_ioctl_phyext84991.h"

#define PHY_READ(a, b, c, d)        if ((ret = mdio_read_c45_register(a, b, c, d))) goto Exit;
#define PHY_WRITE(a, b, c, d)       if ((ret = mdio_write_c45_register(a, b, c, d))) goto Exit;

static int copper_speed_map[] = {0, 2500, 100, 5000, 1000, 0, 10000, 10};
int configured_an_enable = PHY_CFG_AN_AUTO;     /* AN Configuration Status; Auto; On; Off */
int configured_current_inter_phy_type = INTER_PHY_TYPE_AUTO;     /* Manually configured */

typedef struct
{
    uint16_t reserved1:1;
    uint16_t copper_detected:1;
    uint16_t copper_speed:3;
    uint16_t copper_link_status:1;
    uint16_t reserved2:3;
    uint16_t frame_type:2;
    uint16_t reserved3:2;
    uint16_t mac_side_link_status:1;
    uint16_t crc_check_status:2;
} ext84991_status_reg_t;

int phyext84991_power_get(uint32_t phy_addr, int *enable)
{
    uint16_t val;
    int ret = 0;

    PHY_READ(phy_addr, 0x1e, 0x401a, &val);

    *enable = (val & (1 << 7)) ? 0 : 1; /* Copper disable bit */

Exit:
    if (ret)    ret = -EFAULT;

    return ret;
}

int phyext84991_power_set(uint32_t phy_addr, int enable)
{
    uint16_t val;
    int ret = 0;

    PHY_READ(phy_addr, 0x1e, 0x401a, &val);

    if (enable)
        val &= ~(1 << 7); /* Copper enable */
    else
        val |= (1 << 7); /*  Copper disable */

    PHY_WRITE(phy_addr, 0x1e, 0x401a, val);

Exit:
    if (ret)    ret = -EFAULT;

    return ret;
}

int phyext84991_phy_reset(uint32_t phy_addr)
{
    uint16_t val;
    int ret = 0, i;

    PHY_WRITE(phy_addr, 0x01, 0x0000, 0x8000);

    i = 1000;
    do
    {
        udelay(2000);
        ret = mdio_read_c45_register(phy_addr, 0x01, 0x0000, &val);
        if ((val & 0xFFFF) == 0x2040)
			break;
    } while (i--);

Exit:
    if (ret)    ret = -EFAULT;

    return ret;
}


/* Command codes */
#define CMD_GET_PAIR_SWAP                           0x8000
#define CMD_SET_PAIR_SWAP                           0x8001
#define CMD_GET_1588_ENABLE                         0x8004
#define CMD_SET_1588_ENABLE                         0x8005
#define CMD_GET_SHORT_REACH_MODE_ENABLE             0x8006
#define CMD_SET_SHORT_REACH_MODE_ENABLE             0x8007
#define CMD_GET_EEE_MODE                            0x8008
#define CMD_SET_EEE_MODE                            0x8009
#define CMD_GET_EMI_MODE_ENABLE                     0x800A
#define CMD_SET_EMI_MODE_ENABLE                     0x800B
#define CMD_GET_SUB_LF_RF_STATUS                    0x800D
#define CMD_GET_SERDES_KR_MODE_ENABLE               0x800E
#define CMD_SET_SERDES_KR_MODE_ENABLE               0x800F
#define CMD_CLEAR_SUB_LF_RF                         0x8010
#define CMD_SET_SUB_LF_RF                           0x8011
#define CMD_READ_INDIRECT_GPHY_REG_BITS             0x8014
#define CMD_WRITE_INDIRECT_GPHY_REG_BITS            0x8015
#define CMD_GET_XFI_2P5G_5G_MODE                    0x8016
#define CMD_SET_XFI_2P5G_5G_MODE                    0x8017
#define CMD_GET_TWO_PAIR_1G_MODE                    0x8018
#define CMD_SET_TWO_PAIR_1G_MODE                    0x8019
#define CMD_SET_EEE_STATISTICS                      0x801A
#define CMD_GET_EEE_STATISTICS                      0x801B
#define CMD_SET_JUMBO_PACKET                        0x801C
#define CMD_GET_JUMBO_PACKET                        0x801D
#define CMD_GET_MSE                                 0x801E
#define CMD_GET_PAUSE_FRAME_MODE                    0x801F
#define CMD_SET_PAUSE_FRAME_MODE                    0x8020
#define CMD_GET_LED_TYPE                            0x8021
#define CMD_SET_LED_TYPE                            0x8022
#define CMD_GET_MGBASE_T_802_3BZ_TYPE               0x8023
#define CMD_SET_MGBASE_T_802_3BZ_TYPE               0x8024
#define CMD_GET_MSE_GPHY                            0x8025
#define CMD_SET_USXGMII                             0x8026
#define CMD_GET_USXGMII                             0x8027
#define CMD_GET_XL_MODE                             0x8029
#define CMD_SET_XL_MODE                             0x802A
#define CMD_GET_XFI_TX_FILTERS                      0x802B
#define CMD_SET_XFI_TX_FILTERS                      0x802C
#define CMD_GET_XFI_POLARITY                        0x802D
#define CMD_SET_XFI_POLARITY                        0x802E
#define CMD_GET_CURRENT_VOLTAGE                     0x802F
#define CMD_GET_SNR                                 0x8030
#define CMD_GET_CURRENT_TEMP                        0x8031
#define CMD_SET_UPPER_TEMP_WARNING_LEVEL            0x8032
#define CMD_GET_UPPER_TEMP_WARNING_LEVEL            0x8033
#define CMD_SET_LOWER_TEMP_WARNING_LEVEL            0x8034
#define CMD_GET_LOWER_TEMP_WARNING_LEVEL            0x8035
#define CMD_GET_HW_FR_EMI_MODE_ENABLE               0x803A
#define CMD_SET_HW_FR_EMI_MODE_ENABLE               0x803B
#define CMD_GET_CUSTOMER_REQUESTED_TX_PWR_ADJUST    0x8040
#define CMD_SET_CUSTOMER_REQUESTED_TX_PWR_ADJUST    0x8041
#define CMD_GET_DYNAMIC_PARTITION_SELECT            0x8042
#define CMD_SET_DYNAMIC_PARTITION_SELECT            0x8043
#define CMD_SET_WOL_ENABLE                          0x805A
#define CMD_GET_WOL_ENABLE                          0x805B
#define CMD_SET_MACSEC_ENABLE                       0x805E
#define CMD_GET_MACSEC_ENABLE                       0x805F
#define CMD_RESET_STAT_LOG                          0xC017

/* Command hanlder status codes */
#define CMD_RECEIVED                                0x0001
#define CMD_IN_PROGRESS                             0x0002
#define CMD_COMPLETE_PASS                           0x0004
#define CMD_COMPLETE_ERROR                          0x0008
#define CMD_SYSTEM_BUSY                             0xBBBB

/* Fixups for 5499x phys */
#define ID1_5499X                                   0x35900000
#define ID1_50901E                                  0xf7a60000
#define ID1_84991                                   0xf7a60000
#define ID1_MASK                                    0xffff0000
#define SUPER_I_DEFAULT                             (1<<15)
#define SUPER_I_BLACKFIN                            (1<<8)
#define SUPER_I_LANAI                               (1<<10)
#define SUPER_I_NIIHAU                              (1<<10)
#define CHANGE_STRAP_STATUS                         (1<<1)

#define XFI_MODE_IDLE_STUFFING           0 /* Idle Stuffing mode over XFI interface */
#define XFI_MODE_BASE_X                  1 /* 2.5GBase-X or 5GBase-X */
#define XFI_MODE_BASE_R                  2 /* 2.5GBase-R or 5GBase-R */

int phyext84991_autogreeen = 0;    /* By default, AutoGreeen mode for EEE is not enable for phyext84991 */

static int _wait_for_cmd_ready(uint32_t phy_addr)
{
    int ret, i;
    uint16_t val;

    for (i = 0; i < 1000; i++)
    {
        /* Read status of command */
        PHY_READ(phy_addr, 0x1e, 0x4037, &val);

        if (val != CMD_IN_PROGRESS && val != CMD_SYSTEM_BUSY)
            return 0;

        udelay(2000);
    }

    printk("Timed out waiting for command ready");

Exit:
    return -1;
}

static uint16_t _wait_for_cmd_complete(uint32_t phy_addr)
{
    int ret, i;
    uint16_t val = 0;

    for (i = 0; i < 1000; i++)
    {
        /* Read status of command */
        PHY_READ(phy_addr, 0x1e, 0x4037, &val);

        if (val == CMD_COMPLETE_PASS || val == CMD_COMPLETE_ERROR)
            goto Exit;

        udelay(2000);
    }

    printk("Timed out waiting for command complete\n");

Exit:
    return val;
}

static int cmd_handler(uint32_t phy_addr, uint16_t cmd_code, uint16_t *data1, uint16_t *data2, uint16_t *data3, uint16_t *data4, uint16_t *data5)
{
    int ret;
    uint16_t cmd_status = 0;

    /* Make sure command interface is open */
    if ((ret = _wait_for_cmd_ready(phy_addr)))
        goto Exit;

    switch (cmd_code)
    {
    case CMD_SET_PAIR_SWAP:
    case CMD_SET_1588_ENABLE:
    case CMD_SET_SHORT_REACH_MODE_ENABLE:
    case CMD_SET_EEE_MODE:
    case CMD_SET_EMI_MODE_ENABLE:
    case CMD_SET_SERDES_KR_MODE_ENABLE:
    case CMD_CLEAR_SUB_LF_RF:
    case CMD_SET_SUB_LF_RF:
    case CMD_WRITE_INDIRECT_GPHY_REG_BITS:
    case CMD_SET_XFI_2P5G_5G_MODE:
    case CMD_SET_TWO_PAIR_1G_MODE:
    case CMD_SET_PAUSE_FRAME_MODE:
    case CMD_SET_LED_TYPE:
    case CMD_SET_MGBASE_T_802_3BZ_TYPE:
    case CMD_SET_USXGMII:
    case CMD_SET_EEE_STATISTICS:
    case CMD_SET_JUMBO_PACKET:
    case CMD_SET_XL_MODE:
    case CMD_SET_XFI_TX_FILTERS:
    case CMD_SET_XFI_POLARITY:
    case CMD_SET_UPPER_TEMP_WARNING_LEVEL:
    case CMD_SET_LOWER_TEMP_WARNING_LEVEL:
    case CMD_SET_HW_FR_EMI_MODE_ENABLE:
    case CMD_SET_CUSTOMER_REQUESTED_TX_PWR_ADJUST:
    case CMD_SET_DYNAMIC_PARTITION_SELECT:
    case CMD_SET_WOL_ENABLE:
    case CMD_SET_MACSEC_ENABLE:
    {
        if (data1)
            PHY_WRITE(phy_addr, 0x1e, 0x4038, *data1);
        if (data2)
            PHY_WRITE(phy_addr, 0x1e, 0x4039, *data2);
        if (data3)
            PHY_WRITE(phy_addr, 0x1e, 0x403a, *data3);
        if (data4)
            PHY_WRITE(phy_addr, 0x1e, 0x403b, *data4);
        if (data5)
            PHY_WRITE(phy_addr, 0x1e, 0x403c, *data5);

        PHY_WRITE(phy_addr, 0x1e, 0x4005, cmd_code);
        cmd_status = _wait_for_cmd_complete(phy_addr);

        break;
    }
    case CMD_GET_PAIR_SWAP:
    case CMD_GET_1588_ENABLE:
    case CMD_GET_SHORT_REACH_MODE_ENABLE:
    case CMD_GET_EEE_MODE:
    case CMD_GET_EMI_MODE_ENABLE:
    case CMD_GET_SERDES_KR_MODE_ENABLE:
    case CMD_GET_SUB_LF_RF_STATUS:
    case CMD_GET_XFI_2P5G_5G_MODE:
    case CMD_GET_TWO_PAIR_1G_MODE:
    case CMD_GET_PAUSE_FRAME_MODE:
    case CMD_GET_LED_TYPE:
    case CMD_GET_MGBASE_T_802_3BZ_TYPE:
    case CMD_GET_MSE_GPHY:
    case CMD_GET_USXGMII:
    case CMD_GET_JUMBO_PACKET:
    case CMD_GET_MSE:
    case CMD_GET_XL_MODE:
    case CMD_GET_XFI_TX_FILTERS:
    case CMD_GET_XFI_POLARITY:
    case CMD_GET_CURRENT_VOLTAGE:
    case CMD_GET_SNR:
    case CMD_GET_CURRENT_TEMP:
    case CMD_GET_UPPER_TEMP_WARNING_LEVEL:
    case CMD_GET_LOWER_TEMP_WARNING_LEVEL:
    case CMD_GET_HW_FR_EMI_MODE_ENABLE:
    case CMD_GET_CUSTOMER_REQUESTED_TX_PWR_ADJUST:
    case CMD_GET_DYNAMIC_PARTITION_SELECT:
    case CMD_GET_WOL_ENABLE:
    case CMD_GET_MACSEC_ENABLE:
    case CMD_RESET_STAT_LOG:
    {
        PHY_WRITE(phy_addr, 0x1e, 0x4005, cmd_code);
        cmd_status = _wait_for_cmd_complete(phy_addr);

        if (data1)
            PHY_READ(phy_addr, 0x1e, 0x4038, data1);
        if (data2)
            PHY_READ(phy_addr, 0x1e, 0x4039, data2);
        if (data3)
            PHY_READ(phy_addr, 0x1e, 0x403a, data3);
        if (data4)
            PHY_READ(phy_addr, 0x1e, 0x403b, data4);
        if (data5)
            PHY_READ(phy_addr, 0x1e, 0x403c, data5);

        break;
    }
    case CMD_READ_INDIRECT_GPHY_REG_BITS:
    {
        if (data1)
            PHY_WRITE(phy_addr, 0x1e, 0x4038, *data1);
        if (data2)
            PHY_WRITE(phy_addr, 0x1e, 0x4039, *data2);
        if (data3)
            PHY_WRITE(phy_addr, 0x1e, 0x403a, *data3);
        if (data4)
            PHY_WRITE(phy_addr, 0x1e, 0x403b, *data4);

        PHY_WRITE(phy_addr, 0x1e, 0x4005, cmd_code);
        cmd_status = _wait_for_cmd_complete(phy_addr);

        if (data5)
            PHY_READ(phy_addr, 0x1e, 0x403c, data5);

        break;
    }
    case CMD_GET_EEE_STATISTICS:
    {
        if (data1)
            PHY_WRITE(phy_addr, 0x1e, 0x4038, *data1);

        PHY_WRITE(phy_addr, 0x1e, 0x4005, cmd_code);
        cmd_status = _wait_for_cmd_complete(phy_addr);

        if (data2)
            PHY_READ(phy_addr, 0x1e, 0x4039, data2);
        if (data3)
            PHY_READ(phy_addr, 0x1e, 0x403a, data3);
        if (data4)
            PHY_READ(phy_addr, 0x1e, 0x403b, data4);
        if (data5)
            PHY_READ(phy_addr, 0x1e, 0x403c, data5);

        break;
    }
    default:
        printk("Unsupported cmd code: 0x%x\n", cmd_code);
        break;
    }

    if (cmd_status != CMD_COMPLETE_PASS)
    {
        printk("Failed to execute cmd code: 0x%x\n", cmd_code);
        return 0;
    }

Exit:
    return ret;
}

#define EEE_MODE_DISABLED               0 /* EEE Disabled*/
#define EEE_MODE_NATIVE_EEE             1 /* Native EEE */
#define EEE_MODE_AUTOGREEEN_FIXED       2 /* AutoGrEEEn Fixed Latency */
#define EEE_MODE_AUTOGREEEN_VARIABLE    3 /* AutoGrEEEn Variable Latency */

/* Note: AutoGrEEEn mode is not supported when idle stuffing is enabled on 2.5/5G rates */

static int _phy_eee_modes_get(uint32_t phy_addr, uint16_t *modes)
{
    int ret = 0;
    uint16_t data1, data2, data3, data4;

    /* Get EEE modes */
    if ((ret = cmd_handler(phy_addr, CMD_GET_EEE_MODE, &data1, &data2, &data3, &data4, NULL)))
        goto Exit;

    *modes = data1;

Exit:
    return ret;
}

static int _phy_super_isolate_niihau(uint32_t phy_addr, int isolate)
{
    int ret;
    uint16_t data;

    /* Read the status register */
    PHY_READ(phy_addr, 0x1e, 0x401c, &data);

    if (isolate)
        data |= SUPER_I_NIIHAU;
    else
        data &= ~SUPER_I_NIIHAU;

    PHY_WRITE(phy_addr, 0x1e, 0x401c, data);

    return 0;
Exit:
    return ret;
}

static int _phy_eee_mode_set(uint32_t phy_addr, uint32_t caps)
{
    int ret = 0;
    uint16_t modes = 0, data1, data2, data3, data4;
    uint8_t mode = phyext84991_autogreeen ? EEE_MODE_AUTOGREEEN_FIXED : EEE_MODE_NATIVE_EEE;

    data1 = XFI_MODE_BASE_X;
    data2 = XFI_MODE_BASE_X;

    if ((ret = cmd_handler(phy_addr, CMD_GET_XFI_2P5G_5G_MODE, &data1, &data2, NULL, NULL, NULL)))
        goto Exit;

    /* 10G          bits 0:1 */
    /* 100M/1G      bits 2:3 */
    /* 2.5G         bits 4:5 */
    /* 5G           bits 6:7 */
    modes |= ((caps & PHY_CAP_100_HALF) || (caps & PHY_CAP_100_FULL)) ? (mode << 2) : 0;
    modes |= ((caps & PHY_CAP_1000_HALF) || (caps & PHY_CAP_1000_FULL)) ? (mode << 2) : 0;
    modes |= ((caps & PHY_CAP_2500) ? ((data1 == XFI_MODE_IDLE_STUFFING ? EEE_MODE_NATIVE_EEE : mode) << 4) : 0);
    modes |= ((caps & PHY_CAP_5000) ? ((data2 == XFI_MODE_IDLE_STUFFING ? EEE_MODE_NATIVE_EEE : mode) << 6) : 0);
    modes |= ((caps & PHY_CAP_10000)) ? (mode << 0) : 0;

    data1 = modes;  /* Bitmap of EEE modes per speed */
    data2 = 0x0000; /* AutoGrEEEn High Threshold */
    data3 = 0x7a12; /* AutoGrEEEn Low Threshold */
    data4 = 0x0480; /* AutoGrEEEn Latency */

    /* Set EEE mode */
    if ((ret = cmd_handler(phy_addr, CMD_SET_EEE_MODE, &data1, &data2, &data3, &data4, NULL)))
        goto Exit;

    PHY_WRITE(phy_addr, 0x01, 0xa88c, data2);   /* AutoGrEEEn High Threshold */
    PHY_WRITE(phy_addr, 0x01, 0xa88d, data3);   /* AutoGrEEEn Low Threshold */
    PHY_WRITE(phy_addr, 0x01, 0xa88e, data4);   /* AutoGrEEEn Latency */

    _phy_super_isolate_niihau(phy_addr, 1);  /* Requested in datasheet, or EEE will fail in power on link up */
    udelay(2000);
    _phy_super_isolate_niihau(phy_addr, 0);

Exit:
    return ret;
}

static int _phy_caps_get(uint32_t phy_addr, int caps_type, uint32_t *pcaps)
{
    uint16_t val = 0;
    uint32_t caps = 0;
    int ret = 0;

    if ((caps_type != CAPS_TYPE_ADVERTISE) && (caps_type != CAPS_TYPE_SUPPORTED))
        goto Exit;

    if (caps_type == CAPS_TYPE_SUPPORTED)
    {
        caps |= PHY_CAP_AUTONEG | PHY_CAP_PAUSE | PHY_CAP_PAUSE_ASYM | PHY_CAP_REPEATER;
        caps |= PHY_CAP_2500 | PHY_CAP_5000  | PHY_CAP_10000;

        /* MII status */
        PHY_READ(phy_addr, 0x07, 0xffe1, &val);

        if (val & (1 << 11))
            caps |= PHY_CAP_10_HALF;
        if (val & (1 << 12))
            caps |= PHY_CAP_10_FULL;
        if (val & (1 << 13))
            caps |= PHY_CAP_100_HALF;
        if (val & (1 << 14))
            caps |= PHY_CAP_100_FULL;

        /* MII extended status */
        PHY_READ(phy_addr, 0x07, 0xffef, &val);

        if (val & (1 << 12))
            caps |= PHY_CAP_1000_HALF;
        if (val & (1 << 13))
            caps |= PHY_CAP_1000_FULL;

        *pcaps = caps;

        return 0;
    }

    /* 1000Base-T/100Base-TX MII control */
    PHY_READ(phy_addr, 0x07, 0xffe0, &val);

    if (val & (1 << 12))
        caps |= PHY_CAP_AUTONEG;

    /* Copper auto-negotiation advertisement */
    PHY_READ(phy_addr, 0x07, 0xffe4, &val);

    if (val & (1 << 10))
        caps |= PHY_CAP_PAUSE;

    if (val & (1 << 11))
        caps |= PHY_CAP_PAUSE_ASYM;

    if (val & (1 << 5))
        caps |= PHY_CAP_10_HALF;

    if (val & (1 << 6))
        caps |= PHY_CAP_10_FULL;

    if (val & (1 << 7))
        caps |= PHY_CAP_100_HALF;

    if (val & (1 << 8))
        caps |= PHY_CAP_100_FULL;

    /* 1000Base-T control */
    PHY_READ(phy_addr, 0x07, 0xffe9, &val);

    if (val & (1 << 8))
        caps |= PHY_CAP_1000_HALF;

    if (val & (1 << 9))
        caps |= PHY_CAP_1000_FULL;

    if (val & (1 << 10))
        caps |= PHY_CAP_REPEATER;

    /* 10GBase-T AN control */
    PHY_READ(phy_addr, 0x07, 0x0020, &val);

    if (val & (1 << 7))
        caps |= PHY_CAP_2500;

    if (val & (1 << 8))
        caps |= PHY_CAP_5000;

    if (val & (1 << 12))
        caps |= PHY_CAP_10000;

    *pcaps = caps;

Exit:
    if (ret)    ret = -EFAULT;

    return ret;
}

static int _phy_caps_set(uint32_t phy_addr, uint32_t caps)
{
    int ret = 0;
    uint16_t val, modes;

    if (caps & (PHY_CAP_1000_HALF | PHY_CAP_1000_FULL | PHY_CAP_2500 | PHY_CAP_5000 | PHY_CAP_10000))
        caps |= PHY_CAP_AUTONEG;

    /* Copper auto-negotiation advertisement */
    PHY_READ(phy_addr, 0x07, 0xffe4, &val);

    val &= ~((1 << 5) | (1 << 6) | (1 << 7) | (1 << 8) | (1 << 10) | (1 << 11));

    if (caps & PHY_CAP_10_HALF)
        val |= (1 << 5);

    if (caps & PHY_CAP_10_FULL)
        val |= (1 << 6);

    if (caps & PHY_CAP_100_HALF)
        val |= (1 << 7);

    if (caps & PHY_CAP_100_FULL)
        val |= (1 << 8);

    if (caps & PHY_CAP_PAUSE)
        val |= (1 << 10);

    if (caps & PHY_CAP_PAUSE_ASYM)
        val |= (1 << 11);

    PHY_WRITE(phy_addr, 0x07, 0xffe4, val);

    /* 1000Base-T control */
    PHY_READ(phy_addr, 0x07, 0xffe9, &val);

    val &= ~((1 << 8) | (1 << 9) | (1 << 10));

    if (caps & PHY_CAP_1000_HALF)
        val |= (1 << 8);

    if (caps & PHY_CAP_1000_FULL)
        val |= (1 << 9);

    if (caps & PHY_CAP_REPEATER)
        val |= (1 << 10);

    PHY_WRITE(phy_addr, 0x07, 0xffe9, val);

    /* 10GBase-T AN control */
    PHY_READ(phy_addr, 0x07, 0x0020, &val);

    val &= ~((1 << 7) | (1 << 8) | (1 << 12) | (1 << 13));

    if (caps & PHY_CAP_2500)
        val |= (1 << 7);

    if (caps & PHY_CAP_5000)
        val |= (1 << 8);

    if (caps & PHY_CAP_10000)
        val |= (1 << 12);

    if (caps & PHY_CAP_REPEATER)
        val |= (1 << 13);

    PHY_WRITE(phy_addr, 0x07, 0x0020, val);

    /* 1000Base-T/100Base-TX MII control */
    PHY_READ(phy_addr, 0x07, 0xffe0, &val);

    val &= ~((1 << 6) | (1 << 8) | (1 << 12) | (1 << 13));

    if (caps & (PHY_CAP_1000_HALF | PHY_CAP_1000_FULL))
        val |= (1 << 6);
    else if (caps & (PHY_CAP_100_HALF | PHY_CAP_100_FULL))
        val |= (1 << 13);

    if (caps & (PHY_CAP_10_FULL | PHY_CAP_100_FULL | PHY_CAP_1000_FULL))
        val |= (1 << 8);

    if (caps & PHY_CAP_AUTONEG)
        val |= (1 << 12);

    PHY_WRITE(phy_addr, 0x07, 0xffe0, val);

    if (!(caps & PHY_CAP_AUTONEG))
        goto Exit;

    /* Check if EEE mode is configured */
    if ((ret = _phy_eee_modes_get(phy_addr, &modes)))
        goto Exit;

    /* Reset the EEE mode according to the phy capabilites, if it was set before */
    if (modes && (ret = _phy_eee_mode_set(phy_addr, caps)))
        goto Exit;

    /* Restart auto negotiation */
    val |= (1 << 9);
    PHY_WRITE(phy_addr, 0x07, 0xffe0, val);

Exit:
    if (ret)    ret = -EFAULT;

    return ret;
}

static int _phy_an_restart(uint32_t phy_addr)
{
    int ret;
    uint16_t val;

    PHY_READ(phy_addr, 0x07, 0xffe0, &val);
    val |= (1 << 12) | (1 << 9);
    PHY_WRITE(phy_addr, 0x07, 0xffe0, val);

Exit:
    return ret;
}

int phyext84991_mediatype(struct ethctl_data *ethctl)
{
    uint32_t advPhyCaps;
    phy_speed_t speed;
    phy_duplex_t duplex;
	ext84991_status_reg_t *sts;
    uint16_t val, auto_nego_hcd;
	uint32_t phy_addr = ethctl->phy_addr;
    int ret = 0;

    if (ethctl->op == ETHGETEXT84991MEDIATYPE)
    { /* Get media-type */
        if (_phy_caps_get(phy_addr, CAPS_TYPE_ADVERTISE, &advPhyCaps))
            return -EFAULT;

        speed = phy_caps_to_auto_speed(advPhyCaps);
        duplex = (advPhyCaps & (PHY_CAP_AUTONEG|PHY_CAP_10000|PHY_CAP_5000|PHY_CAP_2500|
                    PHY_CAP_1000_FULL|PHY_CAP_100_FULL|PHY_CAP_10_FULL))? PHY_DUPLEX_FULL : PHY_DUPLEX_HALF;

        ethctl->advPhyCaps = advPhyCaps;
        ethctl->cfgSpeed = phy_speed_2_mbps(speed);

        if (duplex == PHY_DUPLEX_FULL)
            ethctl->cfgDuplex = 1;
        else
            ethctl->cfgDuplex = 0;

        if (_phy_caps_get(phy_addr, CAPS_TYPE_SUPPORTED, &ethctl->phyCap))
            return -EFAULT;

        /* Status Register*/
        PHY_READ(phy_addr, 0x1e, 0x400d, &val);
        sts = (ext84991_status_reg_t *)&val;

        if (sts->copper_link_status)
        {
            speed = copper_speed_map[sts->copper_speed];
            if (speed == PHY_SPEED_UNKNOWN)
                return -EFAULT;
            else
                ethctl->speed = phy_speed_2_mbps(speed);

            if (ethctl->speed <= PHY_SPEED_1000)
            {
                /* Aux status summary Register*/
                PHY_READ(phy_addr, 0x07, 0x9009, &val);
                auto_nego_hcd = (val >> 8) & 0x7;
                if (auto_nego_hcd == 0x2 || auto_nego_hcd == 0x5 || auto_nego_hcd == 0x7)
                    ethctl->duplex = 1;
                else 
                    ethctl->duplex = 0;
            } else
                ethctl->duplex = 1;
        }
        else
        {
            ethctl->speed = 0;
            ethctl->duplex = 0;
        }

        ethctl->config_speed_mode = configured_current_inter_phy_type; 
        ethctl->config_an_enable = configured_an_enable;
    } else
    { /* Set media-type */
        uint32_t curAdvCaps, supported_caps = 0;

        duplex = ethctl->duplex ? PHY_DUPLEX_FULL : PHY_DUPLEX_HALF;

        if (_phy_caps_get(phy_addr, CAPS_TYPE_ADVERTISE, &curAdvCaps))
            return -EFAULT;

        switch (ethctl->speed)
        {
            case 10: case 100: case 1000: case 2500: case 5000: case 10000:
                speed = phy_mbps_2_speed(ethctl->speed); break;
            case 0:     speed = PHY_SPEED_AUTO; break;
            default:    speed = PHY_SPEED_UNKNOWN;
        }

        if (_phy_caps_get(phy_addr, CAPS_TYPE_SUPPORTED, &supported_caps))
            return -EFAULT;

        if (ethctl->advPhyCaps == PHY_CAP_AUTONEG)    /* Set full support speed if Auto flag only */
        {
            ethctl->advPhyCaps |= supported_caps;
        } else if (ethctl->advPhyCaps == 0)         /* If not set, for the backward compatibility */
            ethctl->advPhyCaps = phy_speed_to_cap(speed, duplex);

        /* Set non speed flags from current flag set */
        ethctl->advPhyCaps = (ethctl->advPhyCaps & PHY_CAP_PURE_SPEED_CAPS) | (curAdvCaps & PHY_CAP_NON_SPEED_CAPS);

        if ((ethctl->advPhyCaps & supported_caps) == 0)
        {
            printk("Not Supported Speed %dmbps attempted\n", ethctl->speed);
            return -EFAULT;
        }

        if (_phy_caps_set(phy_addr, ethctl->advPhyCaps))
        {
            ethctl->ret_val = RET_ERR_NOT_SUPPORTED;
            return -EFAULT;
        }
    }

Exit:
    if (ret)    ret = -EFAULT;

    return ret;
}

int phyext84991_phy_eee_set(uint32_t phy_addr, int enable)
{
    uint32_t caps;

    if (_phy_caps_get(phy_addr, CAPS_TYPE_ADVERTISE, &caps))
        return -EFAULT;

    if (_phy_eee_mode_set(phy_addr, enable ? caps : 0))
        return -EFAULT;

    /* Restart auto negotiation to kick off EEE settings */
    if (_phy_an_restart(phy_addr))
        return -EFAULT;

    return 0;
}

int phyext84991_phy_eee_get(uint32_t phy_addr, int *enable)
{
    uint16_t val;

   /* local EEE Status */
    if (_phy_eee_modes_get(phy_addr, &val))
        return -EFAULT;

    *enable = !!val;

    return 0;
}

int phyext84991_phy_eee_autogreeen_read(uint32_t phy_addr, int *autogreeen)
{
    int ret = 0;
    uint16_t val, mode, modes;
	ext84991_status_reg_t *sts;
    phy_speed_t speed;

    /* Read configured EEE modes */
    if (_phy_eee_modes_get(phy_addr, &modes))
        return -EFAULT;

    /* Get Current link speed */
    PHY_READ(phy_addr, 0x1e, 0x400d, &val);
    sts = (ext84991_status_reg_t *)&val;

    speed = copper_speed_map[sts->copper_speed];
    switch (speed)
    {
    case PHY_SPEED_10000:
        mode = (modes >> 0) & 0x3;
        break;
    case PHY_SPEED_5000:
        mode = (modes >> 6) & 0x3;
        break;
    case PHY_SPEED_2500:
        mode = (modes >> 4) & 0x3;
        break;
    case PHY_SPEED_1000:
    case PHY_SPEED_100:
        mode = (modes >> 2) & 0x3;
        break;
    default:
        mode = 0;
        break;
    }

    *autogreeen = mode >= EEE_MODE_AUTOGREEEN_FIXED ? 1 : 0;

Exit:
    if (ret)    ret = -EFAULT;

    return ret;
}

int phyext84991_phy_eee_mode_set(uint32_t phy_addr, int autogreeen)
{
    int enable;

    phyext84991_autogreeen = autogreeen;
    if (phyext84991_phy_eee_get(phy_addr, &enable))
        return -EFAULT;

    if (enable)
        if (phyext84991_phy_eee_set(phy_addr, enable))
            return -EFAULT;

    return 0;
}
