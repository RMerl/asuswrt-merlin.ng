/*
   <:copyright-BRCM:2024:DUAL/GPL:standard
   
      Copyright (c) 2024 Broadcom 
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

/*
 *  Created on: Jan/2026
 *
 *  PHY driver for external MxL (Maxlinear) switch ports using SMDIO/FAPI-over-MDIO.
 *  Used for VLAN-tagged external switch ports (e.g., RT-BE82M with MxL86252C).
 *
 *  The MxL switch does not expose a standard PHY MDIO interface for quick link
 *  status reads.  Instead, it uses a proprietary SMDIO (Switch-MDIO) protocol
 *  layered on top of MDIO Clause-45, accessible at a fixed MDIO PHY address.
 *  This driver implements the minimum SMDIO transaction needed to call the
 *  FAPI GSW_PortLinkCfgGet command and extract link, speed, and duplex state.
 *
 *  SMDIO register map (MXL_SWITCH_MDIO_ADDR, MMD=0x1e):
 *    reg[0x0000] = CTRL  : bit15=BUSY, bits[14:0]=command/opcode
 *    reg[0x0001] = LEN   : data size in bytes (write) / return code (read)
 *    reg[0x0002..] = DATA: command parameters and response fields
 *
 *  GSW_portLinkCfg_t layout (packed, 30 bytes):
 *    bytes[0-1]  : nPortId  (u16)
 *    bytes[2]    : bDuplexForce (u8)
 *    bytes[3-6]  : eDuplex  (int32 LE)
 *    bytes[7]    : bSpeedForce (u8)
 *    bytes[8-11] : eSpeed   (int32 LE)
 *    bytes[12]   : bLinkForce (u8)
 *    bytes[13-16]: eLink    (int32 LE)
 *    ... (remaining fields not used for link detection)
 *
 *  Key field extraction from 16-bit MDIO response words (data starts at reg[2]):
 *    eDuplex = (reg[3] >> 8) & 0xFF   (0=FULL, 1=HALF, 2=AUTO)
 *    eSpeed  = reg[6] & 0xFF          (0=10M,  1=100M, 3=1G, 4=2.5G, 5=5G, 6=10G)
 *    eLink   = (reg[8] >> 8) & 0xFF   (0=UP,   1=DOWN, 2=AUTO)
 */

#include "phy_drv.h"
#include "phy_drv_mii.h"
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/delay.h>

/* MxL switch MDIO address and MMD for SMDIO protocol */
#define MXL_SWITCH_MDIO_ADDR    0x10
#define MXL_SWITCH_MMD          0x1e

/* SMDIO register offsets (within MMD 0x1e) */
#define SMDIO_REG_CTRL          0x0000
#define SMDIO_REG_LEN           0x0001
#define SMDIO_REG_DATA0         0x0002  /* first data register */

/* SMDIO CTRL register bit definitions */
#define SMDIO_CTRL_BUSY         (1 << 15)

/* SMDIO low-level commands (written to CTRL[14:0] with BUSY set) */
#define SMDIO_CMD_RST_DATA      0x0008  /* reset/clear all data registers */

/* GSW FAPI command codes */
#define GSW_COMMON_MAGIC        0x0100
#define GSW_CMD_PORTLINKCFGGET  (GSW_COMMON_MAGIC + 0x5)  /* = 0x0105 */

/* sizeof(GSW_portLinkCfg_t) packed = 30 bytes */
#define GSW_PORTLINKCFG_SIZE    30

/* Max BUSY-wait iterations (~10 ms at 10 us per iteration) */
#define SMDIO_BUSY_MAX_ITER     1000
#define SMDIO_BUSY_DELAY_US     10

/*
 * GSW_portLink_t enum values
 * (from docs/user_src_ref/libhostapi/switch_hostapi/include/gswip/gsw.h)
 */
#define GSW_PORT_LINK_UP        0
#define GSW_PORT_LINK_DOWN      1

/*
 * GSW_portSpeed_t enum values
 */
#define GSW_PORT_SPEED_10       0
#define GSW_PORT_SPEED_100      1
#define GSW_PORT_SPEED_200      2
#define GSW_PORT_SPEED_1000     3
#define GSW_PORT_SPEED_2500     4
#define GSW_PORT_SPEED_5000     5
#define GSW_PORT_SPEED_10000    6

/*
 * GSW_portDuplex_t enum values
 */
#define GSW_DUPLEX_FULL         0
#define GSW_DUPLEX_HALF         1

/* Serialize all SMDIO transactions across all MxL ext PHY instances */
DEFINE_MUTEX(mxlswitch_mutex);
EXPORT_SYMBOL(mxlswitch_mutex);

unsigned long mxl_mdio_user_lock_time = 0;
EXPORT_SYMBOL(mxl_mdio_user_lock_time);

/* -----------------------------------------------------------------------
 * Low-level MDIO CL45 helpers using phy_dev's bus_drv at a fixed address
 * -----------------------------------------------------------------------*/

static int mxl_cl45_read(phy_dev_t *phy_dev, uint16_t reg, uint16_t *val)
{
    int ret;

    if (!phy_dev->bus_drv)
        return -1;
    ret = phy_dev->bus_drv->c45_read(MXL_SWITCH_MDIO_ADDR, MXL_SWITCH_MMD, reg, val);
    if (ret)
        printk("mxl_ext: cl45_read  addr=0x%02x mmd=0x%02x reg=0x%04x FAILED ret=%d\n",
               MXL_SWITCH_MDIO_ADDR, MXL_SWITCH_MMD, reg, ret);
    return ret;
}

static int mxl_cl45_write(phy_dev_t *phy_dev, uint16_t reg, uint16_t val)
{
    int ret;

    if (!phy_dev->bus_drv)
        return -1;
    ret = phy_dev->bus_drv->c45_write(MXL_SWITCH_MDIO_ADDR, MXL_SWITCH_MMD, reg, val);
    if (ret)
        printk("mxl_ext: cl45_write addr=0x%02x mmd=0x%02x reg=0x%04x val=0x%04x FAILED ret=%d\n",
               MXL_SWITCH_MDIO_ADDR, MXL_SWITCH_MMD, reg, val, ret);
    return ret;
}

/* -----------------------------------------------------------------------
 * SMDIO protocol helpers
 * -----------------------------------------------------------------------*/

/*
 * Wait for the SMDIO CTRL busy bit to clear.
 * Returns 0 on success, -ETIMEDOUT if the switch does not respond.
 */
static int smdio_wait_ready(phy_dev_t *phy_dev)
{
    int i;
    uint16_t ctrl;

    for (i = 0; i < SMDIO_BUSY_MAX_ITER; i++) {
        if (mxl_cl45_read(phy_dev, SMDIO_REG_CTRL, &ctrl))
            return -EIO;
        if (!(ctrl & SMDIO_CTRL_BUSY))
            return 0;
        udelay(SMDIO_BUSY_DELAY_US);
    }
    printk("mxl_ext: smdio_wait_ready TIMEOUT after %d iterations ctrl=0x%04x\n",
           SMDIO_BUSY_MAX_ITER, ctrl);
    return -ETIMEDOUT;
}

/*
 * Execute one SMDIO command:
 *   1. Wait for switch to be ready
 *   2. Issue RST_DATA to zero the data register area
 *   3. Write the input data word(s)
 *   4. Send the command with the payload size
 *   5. Wait for completion and read the return code
 *
 * On success, the response is available in DATA registers starting at
 * SMDIO_REG_DATA0 and the function returns 0.  On error returns < 0.
 */
static int smdio_send_cmd(phy_dev_t *phy_dev, uint16_t cmd,
                          uint16_t *in_words, int in_count,
                          uint16_t size_bytes)
{
    int i, ret;
    uint16_t retcode;

    /* 1. Wait for switch to be idle */
    ret = smdio_wait_ready(phy_dev);
    if (ret) {
        printk("mxl_ext: smdio_send_cmd initial wait failed ret=%d\n", ret);
        return ret;
    }

    /* 2. Reset data area (all bytes to zero) */
    if (mxl_cl45_write(phy_dev, SMDIO_REG_LEN, 0))
        return -EIO;
    if (mxl_cl45_write(phy_dev, SMDIO_REG_CTRL, SMDIO_CTRL_BUSY | SMDIO_CMD_RST_DATA))
        return -EIO;
    ret = smdio_wait_ready(phy_dev);
    if (ret) {
        printk("mxl_ext: smdio_send_cmd RST_DATA wait failed ret=%d\n", ret);
        return ret;
    }

    /* 3. Write non-zero input words into data registers */
    for (i = 0; i < in_count; i++) {
        if (in_words[i] == 0)
            continue;
        if (mxl_cl45_write(phy_dev, SMDIO_REG_DATA0 + i, in_words[i]))
            return -EIO;
    }

    /* 4. Send the command: write size to LEN, then command|BUSY to CTRL */
    if (mxl_cl45_write(phy_dev, SMDIO_REG_LEN, size_bytes))
        return -EIO;
    if (mxl_cl45_write(phy_dev, SMDIO_REG_CTRL, SMDIO_CTRL_BUSY | cmd))
        return -EIO;

    /* 5. Wait for completion */
    ret = smdio_wait_ready(phy_dev);
    if (ret) {
        printk("mxl_ext: smdio_send_cmd command wait failed ret=%d\n", ret);
        return ret;
    }

    /* Read return code from LEN register.
     * On success the switch overwrites LEN with 0.  If LEN still holds our
     * pre-written size_bytes value it means the CTRL write was silently
     * dropped (bus contention) and the command never executed. */
    if (mxl_cl45_read(phy_dev, SMDIO_REG_LEN, &retcode))
        return -EIO;

    if ((int16_t)retcode < 0) {
        printk("mxl_ext: smdio_send_cmd switch returned error retcode=%d\n", (int16_t)retcode);
        return (int16_t)retcode;
    }

    if (retcode != 0) {
        /* LEN was not overwritten by the switch; the command was likely
         * never received (CTRL write dropped due to MDIO bus contention).
         * Treat as a transient failure so the caller skips this poll cycle
         * rather than acting on stale all-zeros DATA registers. */
        printk("mxl_ext: smdio_send_cmd suspicious retcode=%u (expected 0) -- CTRL write may have been dropped, discarding\n",
               retcode);
        return -EAGAIN;
    }

    return 0;
}

/* -----------------------------------------------------------------------
 * PHY addr to MxL switch port mapping
 * -----------------------------------------------------------------------*/

/*
 * Map the DTS "reg" value (phy_dev->addr) to the MxL switch port number.
 *
 * Convention: reg = MXL_SWITCH_MDIO_ADDR + mxl_port, so the port number
 * is simply addr - MXL_SWITCH_MDIO_ADDR.  
 *   For RT-BE82M (per mxl_lanport_maps.txt):
 *   reg=0x12 -> p2 (LAN1), reg=0x13 -> p3 (LAN2),
 *   reg=0x14 -> p4 (LAN3), reg=0x15 -> p5 (LAN4).
 * addr==0x10 (the switch itself) gives port 0, rejected by the > 0 check.
 */
static int phy_addr_to_mxl_port(int phy_addr)
{
    int port = phy_addr - MXL_SWITCH_MDIO_ADDR;

    if (port <= 0)
        return -1;
    return port;
}

/* -----------------------------------------------------------------------
 * PHY driver operations
 * -----------------------------------------------------------------------*/

/*
 * mxl_ext_phy_read_status - read link/speed/duplex from MxL switch port
 *
 * Sends GSW_PortLinkCfgGet FAPI command via SMDIO and decodes the packed
 * GSW_portLinkCfg_t response to update phy_dev->link/speed/duplex.
 *
 * MDIO word layout for the response (data starts at reg[SMDIO_REG_DATA0]):
 *   reg[2] = data[0]: nPortId          (u16 LE)
 *   reg[3] = data[1]: {bDuplexForce, eDuplex[0]}
 *   reg[4] = data[2]: {eDuplex[1], eDuplex[2]}
 *   reg[5] = data[3]: {eDuplex[3], bSpeedForce}
 *   reg[6] = data[4]: {eSpeed[0], eSpeed[1]}
 *   reg[7] = data[5]: {eSpeed[2], eSpeed[3]}
 *   reg[8] = data[6]: {bLinkForce, eLink[0]}
 *
 * Key extractions:
 *   eDuplex = (reg[3] >> 8) & 0xFF
 *   eSpeed  = reg[6] & 0xFF
 *   eLink   = (reg[8] >> 8) & 0xFF
 */
static int mxl_ext_phy_read_status(phy_dev_t *phy_dev)
{
    int mxl_port = phy_addr_to_mxl_port(phy_dev->addr);
    int old_link = phy_dev->link;
    uint16_t in_words[1];
    uint16_t reg2, reg3, reg6, reg8;
    int eLink, eSpeed, eDuplex;
    int ret;

    if (mxl_port < 0)
        return 0;

    /* Input: only data[0] = nPortId (u16) is non-zero */
    in_words[0] = (uint16_t)mxl_port;

#ifdef PHY_MXL_EXT
    mutex_lock(&mxlswitch_mutex);
    if (mxl_mdio_user_lock_time != 0) {
        /* avoid user-api stuck and cause dead lock */
        if (time_after(jiffies, mxl_mdio_user_lock_time + 5 * HZ)) {
            printk(KERN_WARNING "mxl_ext_phy: user space lock timeout (> 5s)! forcing lock recovery.\\n");
            mxl_mdio_user_lock_time = 0; 
        } else {
            mutex_unlock(&mxlswitch_mutex);
            return 0; /* user-api is on access, skip polling this time */
        }
    }
#endif

    ret = smdio_send_cmd(phy_dev, GSW_CMD_PORTLINKCFGGET,
                         in_words, ARRAY_SIZE(in_words),
                         GSW_PORTLINKCFG_SIZE);
    if (ret) {
        mutex_unlock(&mxlswitch_mutex);
        printk("mxl_ext: port p%d (phy addr 0x%x): PortLinkCfgGet failed ret=%d, skipping poll\n",
               mxl_port, phy_dev->addr, ret);
        return 0;
    }

    /*
     * Read the response words carrying nPortId echo, eDuplex, eSpeed, eLink.
     * Data registers are at SMDIO_REG_DATA0 + word_offset:
     *   word offset 0 (reg[2]) -> nPortId echo (should match mxl_port)
     *   word offset 1 (reg[3]) -> eDuplex in bits[15:8]
     *   word offset 4 (reg[6]) -> eSpeed  in bits[7:0]
     *   word offset 6 (reg[8]) -> eLink   in bits[15:8]
     */
    if (mxl_cl45_read(phy_dev, SMDIO_REG_DATA0 + 0, &reg2) ||
        mxl_cl45_read(phy_dev, SMDIO_REG_DATA0 + 1, &reg3) ||
        mxl_cl45_read(phy_dev, SMDIO_REG_DATA0 + 4, &reg6) ||
        mxl_cl45_read(phy_dev, SMDIO_REG_DATA0 + 6, &reg8)) {
        mutex_unlock(&mxlswitch_mutex);
        printk("mxl_ext: port p%d (phy addr 0x%x): data register read failed\n",
               mxl_port, phy_dev->addr);
        return 0;
    }

    mutex_unlock(&mxlswitch_mutex);

    eDuplex = (reg3 >> 8) & 0xFF;
    eSpeed  = reg6 & 0xFF;
    eLink   = (reg8 >> 8) & 0xFF;

    /* Sanity-check eLink: valid values are 0 (UP) or 1 (DOWN) */
    if (eLink > 1) {
        printk("mxl_ext: port p%d (phy addr 0x%x): unexpected eLink=%d, skipping poll\n",
               mxl_port, phy_dev->addr, eLink);
        return 0;
    }

    /* Update link state */
    phy_dev->link = (eLink == GSW_PORT_LINK_UP) ? 1 : 0;

    /* Convert GSW speed enum to PHY speed */
    switch (eSpeed) {
        case GSW_PORT_SPEED_10:
            phy_dev->speed = PHY_SPEED_10;
            break;
        case GSW_PORT_SPEED_100:
            phy_dev->speed = PHY_SPEED_100;
            break;
        case GSW_PORT_SPEED_1000:
            phy_dev->speed = PHY_SPEED_1000;
            break;
        case GSW_PORT_SPEED_2500:
            phy_dev->speed = PHY_SPEED_2500;
            break;
        case GSW_PORT_SPEED_5000:
            phy_dev->speed = PHY_SPEED_5000;
            break;
        case GSW_PORT_SPEED_10000:
            phy_dev->speed = PHY_SPEED_10000;
            break;
        default:
            phy_dev->speed = PHY_SPEED_UNKNOWN;
            break;
    }

    /* Convert GSW duplex enum to PHY duplex */
    phy_dev->duplex = (eDuplex == GSW_DUPLEX_FULL) ? PHY_DUPLEX_FULL : PHY_DUPLEX_HALF;

    if (old_link != phy_dev->link) {
        printk("MxL ext port p%d (phy addr 0x%x): link %s, speed %d Mbps, duplex %s\n",
               mxl_port, phy_dev->addr,
               phy_dev->link ? "UP" : "DOWN",
               phy_dev->link ? phy_speed_2_mbps(phy_dev->speed) : 0,
               phy_dev->link ? (phy_dev->duplex == PHY_DUPLEX_FULL ? "full" : "half") : "n/a");
    }

    return 0;
}

static int mxl_ext_phy_init(phy_dev_t *phy_dev)
{
    int mxl_port = phy_addr_to_mxl_port(phy_dev->addr);

    printk("mxl_ext_phy_init: phy addr 0x%x -> MxL port p%d\n",
           phy_dev->addr, mxl_port);

    mxl_ext_phy_read_status(phy_dev);

    return 0;
}

static int mxl_ext_phy_power_get(phy_dev_t *phy_dev, int *enable)
{
    /* External switch ports are always powered on */
    *enable = 1;
    return 0;
}

static int mxl_ext_phy_power_set(phy_dev_t *phy_dev, int enable)
{
    /* Cannot independently control power of external switch GPHY ports */
    return 0;
}

phy_drv_t phy_drv_mxl_ext =
{
    .phy_type    = PHY_TYPE_EXT2,
    .name        = "MXL_EXT",
    .read_status = mxl_ext_phy_read_status,
    .init        = mxl_ext_phy_init,
    .power_get   = mxl_ext_phy_power_get,
    .power_set   = mxl_ext_phy_power_set,
};
