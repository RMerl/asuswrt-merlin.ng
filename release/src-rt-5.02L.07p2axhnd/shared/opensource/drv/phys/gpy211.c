/*
	Integrate INTEL GPY211
*/
#include "phy_drv.h"
#include "phy_drv_mii.h"
#include "phy_drv_sf2.h"
#ifdef MACSEC_SUPPORT
#include "phy_macsec_api.h"
#endif


extern phy_drv_t phy_drv_GPY211;
static uint32_t enabled_phys;
static uint32_t devId = 0;

#define PHY_READ(a, b, c, d)    if (phy_bus_c45_read(a, b, c, d)) return 0;
#define PHY_WRITE(a, b, c, d)   if (phy_bus_c45_write(a, b, c, d)) return 0;

#define dbg(fmt, args...) do{printk("[0x%x:%s:%d] " fmt, (phy_dev)?phy_dev->addr:0, __func__, __LINE__, ##args);} while(0)

static void GPY211_SET_SPEED(phy_dev_t *phy_dev, int speed);


static int _phy_gpy211_caps_get(phy_dev_t *phy_dev, int caps_type, uint32_t *pcaps)
{
    uint32_t caps = 0;
    uint16_t val = 0, ext = 0;

    if (caps_type != CAPS_TYPE_SUPPORTED
            && caps_type != CAPS_TYPE_ADVERTISE)
        return 0;

    if (caps_type == CAPS_TYPE_SUPPORTED)
    {
        caps |= PHY_CAP_AUTONEG;
        caps |= PHY_CAP_PAUSE | PHY_CAP_PAUSE_ASYM;
        caps |= PHY_CAP_10_FULL | PHY_CAP_10_HALF;
        caps |= PHY_CAP_100_FULL | PHY_CAP_100_HALF;
        caps |= PHY_CAP_1000_FULL/* | PHY_CAP_1000_HALF*/;
        caps |= PHY_CAP_2500;

        if (phy_dev->disable_hd){
            caps &= ~(PHY_CAP_10_HALF);
            caps &= ~(PHY_CAP_100_HALF);
            //caps &= ~(PHY_CAP_1000_HALF);
        }

        *pcaps = caps;

        return 0;
    }

    /* reg 0.4 (caps) : 10/100M */
    PHY_READ(phy_dev, 0x0, 0x4, &val);

    if (val & (1 << 5))
        caps |= PHY_CAP_10_HALF;

    if (val & (1 << 6))
        caps |= PHY_CAP_10_FULL;

    if (val & (1 << 7))
        caps |= PHY_CAP_100_HALF;

    if (val & (1 << 8))
        caps |= PHY_CAP_100_FULL;

    if (val & (1 << 10))
        caps |= PHY_CAP_PAUSE;

    if (val & (1 << 11))
        caps |= PHY_CAP_PAUSE_ASYM;

    /* reg 0.1 / 0.9 (caps) : 1G */
    PHY_READ(phy_dev, 0x0, 0x1, &ext);
    if (ext & (1 << 8)) {
        PHY_READ(phy_dev, 0x0, 0x9, &val);

#if 0
        if (val & (1 << 8))
            caps |= PHY_CAP_1000_HALF;
#endif

        if (val & (1 << 9))
            caps |= PHY_CAP_1000_FULL;
    }

    /* reg 7.32 (caps) : 2.5G */
    PHY_READ(phy_dev, 0x7, 0x20, &val);

    if (val & (1 << 7))
        caps |= PHY_CAP_2500;

    /* reg 7.1 */
    PHY_READ(phy_dev, 0x7, 0x1, &val);

    if (val & (1 << 3))
        caps |= PHY_CAP_AUTONEG;

    *pcaps = caps;

    return 0;
}

static int _phy_gpy211_caps_set(phy_dev_t *phy_dev, uint32_t caps)
{
    /* Half duplex is supported for 10/100Mbps only */
    if(caps & PHY_CAP_1000_HALF
        || caps & PHY_CAP_10_FULL || caps & PHY_CAP_10_HALF){
        dbg("WARN: Link's Speed & Duplex mismatch\n");
        return 0;
    }

    if(caps & PHY_CAP_AUTONEG)
        GPY211_SET_SPEED(phy_dev, PHY_SPEED_AUTO);
    else if(caps & PHY_CAP_2500)
        GPY211_SET_SPEED(phy_dev, PHY_SPEED_2500);
    else if(caps & PHY_CAP_1000_FULL)
        GPY211_SET_SPEED(phy_dev, PHY_SPEED_1000);
    else if(caps & PHY_CAP_100_FULL)
        GPY211_SET_SPEED(phy_dev, PHY_SPEED_100);
    else
        GPY211_SET_SPEED(phy_dev, PHY_SPEED_AUTO);

    return 0;
}

/*
typedef enum
{
    PHY_SPEED_UNKNOWN,
    PHY_SPEED_AUTO = PHY_SPEED_UNKNOWN,
    PHY_SPEED_10,
    PHY_SPEED_100,
    PHY_SPEED_1000,
    PHY_SPEED_2500,
    PHY_SPEED_5000,
    PHY_SPEED_10000,
} phy_speed_t;

typedef enum
{
    PHY_DUPLEX_UNKNOWN,
    PHY_DUPLEX_HALF,
    PHY_DUPLEX_FULL,
} phy_duplex_t;
 */
static void GPY211_SET_SPEED(phy_dev_t *phy_dev, int speed)
{
    /*
        check flow as below
        0.4  : 100M
        0.9  : 1G
        7.32 : 2.5G
        1.0  : PMAPMD
        0.0  : PHY STD_CTRL and auto-nego
    */
    if (speed == PHY_SPEED_2500) {
        phy_bus_c45_write(phy_dev, 0x0, 0x4, 0x0c01); // 100M
        phy_bus_c45_write(phy_dev, 0x0, 0x9, 0x0000); // 1G
        phy_bus_c45_write(phy_dev, 0x7, 0x20, 0x40a2); // 2.5G
        phy_bus_c45_write(phy_dev, 0x1, 0x0, 0x2058); // PMAPMD
        phy_bus_c45_write(phy_dev, 0x0, 0x0, 0x3340); // PHY STD_CTRL and auto-nego
        dbg("force 2500M\n");
    }
    else if (speed == PHY_SPEED_1000) {
        phy_bus_c45_write(phy_dev, 0x0, 0x4, 0x0de1); // 100M
        phy_bus_c45_write(phy_dev, 0x0, 0x9, 0x0200); // 1G
        phy_bus_c45_write(phy_dev, 0x7, 0x20, 0x4002); // 2.5G
        phy_bus_c45_write(phy_dev, 0x1, 0x0, 0x0058); // PMAPMD
        phy_bus_c45_write(phy_dev, 0x0, 0x0, 0x1340); // PHY STD_CTRL and auto-nego
        dbg("force 1000M\n");
    }
    else if (speed == PHY_SPEED_100) {
        phy_bus_c45_write(phy_dev, 0x0, 0x4, 0x0de1); // 100M
        phy_bus_c45_write(phy_dev, 0x0, 0x9, 0x0000); // 1G
        phy_bus_c45_write(phy_dev, 0x7, 0x20, 0x4002); // 2.5G
        phy_bus_c45_write(phy_dev, 0x1, 0x0, 0x2018); // PMAPMD
        phy_bus_c45_write(phy_dev, 0x0, 0x0, 0x3300); // PHY STD_CTRL and auto-nego
        dbg("force 100M\n");
    }
    else { // speed == PHY_SPEED_AUTO
        phy_bus_c45_write(phy_dev, 0x0, 0x4, 0x0de1); // 100M
        phy_bus_c45_write(phy_dev, 0x0, 0x9, 0x0200); // 1G
        phy_bus_c45_write(phy_dev, 0x7, 0x20, 0x40a2); // 2.5G
        phy_bus_c45_write(phy_dev, 0x1, 0x0, 0x2058); // PMAPMD
        phy_bus_c45_write(phy_dev, 0x0, 0x0, 0x3340); // PHY STD_CTRL and auto-nego
        dbg("auto-nego mode\n");
    }

    phy_dev->speed = speed;
}

static int _phy_speed_set(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex)
{
    GPY211_SET_SPEED(phy_dev, speed);

    return 0;
}

static int _phy_phyid_get(phy_dev_t *phy_dev, uint32_t *phyid)
{
    uint16_t phyid1, phyid2;

    if(devId != 0){
        //dbg("had gotten devId 0x%8x\n", devId);
        *phyid = devId;
        return 0;
    }

    /* reg 1.2 : PMA_DEVID1 */
    PHY_READ(phy_dev, 0x1, 0x2, &phyid1);

    /* reg 1.3 : PMA_DEVID2 */
    PHY_READ(phy_dev, 0x1, 0x3, &phyid2);

    *phyid = devId = phyid1 << 16 | phyid2;

    return 0;
}

static int _phy_read_status(phy_dev_t *phy_dev)
{
    int link_old, speed_old, duplex_old;
    uint16_t val, lpa, adv, taf;

    link_old = phy_dev->link;
    speed_old = phy_dev->speed;
    duplex_old = phy_dev->duplex;

    phy_dev->link = 0;
    phy_dev->speed = PHY_SPEED_UNKNOWN;
    phy_dev->duplex = PHY_DUPLEX_UNKNOWN;
    phy_dev->pause_rx = 0;
    phy_dev->pause_tx = 0;

    //PHY_READ(phy, STD_STD_STAT);
    PHY_READ(phy_dev, 0x0, 0x1, &val);
    //if (ret & STD_STAT_LS_MASK)
    phy_dev->link = (val & (1 << 2))?1:0;

    if (!phy_dev->link)
        return 0;

#if 1
    //PHY_READ_MMD(phy, MDIO_MMD_AN, ANEG_STAT);
    PHY_READ(phy_dev, 0x7, 0x1, &val);

    //if (val & ANEG_STAT_ANEG_COMPLETE_MASK)
    if (val & (1 << 5)) {
        //PHY_READ_MMD(phy, MDIO_MMD_AN, ANEG_MGBT_AN_STA);
        PHY_READ(phy_dev, 0x7, 0x21, &lpa);

        //PHY_READ_MMD(phy, MDIO_MMD_AN, ANEG_MGBT_AN_CTRL);
        PHY_READ(phy_dev, 0x7, 0x20, &val);

        adv = (val >> 2) & lpa;
        //if (adv & ANEG_MGBT_AN_STA_AB_2G5BT_MASK)
        if (adv & (1 << 5)) {
            phy_dev->speed  = PHY_SPEED_2500;
            phy_dev->duplex = PHY_DUPLEX_FULL;
            return 0;
        }
    }

    //PHY_READ(phy, STD_STD_GSTAT);
    PHY_READ(phy_dev, 0x0, 0xa, &lpa);

    //PHY_READ(phy, STD_STD_GCTRL);
    PHY_READ(phy_dev, 0x0, 0x9, &val);
    adv = (val << 2) & lpa;
#if 0
    //if (adv & STD_GSTAT_MBTHD_MASK)
    if (adv & (1 << 10)) {
        phy_dev->speed  = PHY_SPEED_1000;
        phy_dev->duplex = PHY_DUPLEX_HALF;
        return 0;
    }
    else
#endif
    //if (adv & STD_GSTAT_MBTFD_MASK)
    if (adv & (1 << 11)) {
        phy_dev->speed  = PHY_SPEED_1000;
        phy_dev->duplex = PHY_DUPLEX_FULL;
        return 0;
    }

    //PHY_READ(phy, STD_STD_AN_LPA);
    PHY_READ(phy_dev, 0x0, 0x5, &lpa);
    //FIELD_GET(lpa, STD_AN_LPA_TAF);
    taf = ((lpa & 0xfe0) >> 5);

    //PHY_READ(phy, STD_STD_AN_ADV);
    PHY_READ(phy_dev, 0x0, 0x4, &val);
    //FIELD_GET(val, STD_AN_ADV_TAF);
    taf &= ((val & 0xfe0) >> 5);

    //if (taf & GPY2XX_ADVERTISED_100baseT_Full)
    if (taf & 3)
    {
        phy_dev->speed = SPEED_100;
        phy_dev->duplex = DUPLEX_FULL;
    }
    else if (taf & 2)
    {
        phy_dev->speed = SPEED_100;
        phy_dev->duplex = DUPLEX_HALF;
    }
    else if (taf & 1)
    {
        phy_dev->speed = SPEED_10;
        phy_dev->duplex = DUPLEX_FULL;
    }
    else if (taf & 0)
    {
        phy_dev->speed = SPEED_10;
        phy_dev->duplex = DUPLEX_HALF;
    }
#else
    int speed;

    /* reg 0.24 : MII status */
    //PHY_READ(phy, PHY_PHY_MIISTAT);
    PHY_READ(phy_dev, 0x0, 0x18, &val);
    //FIELD_GET(ret, PHY_MIISTAT_SPEED);
    speed = ((val & 0x7) >> 0);

    //CONST_PHY_MIISTAT_SPEED_TEN
    if(speed & 0x0)
        phy_dev->speed  = PHY_SPEED_10;
    //CONST_PHY_MIISTAT_SPEED_FAST
    else if(speed & 0x1)
        phy_dev->speed  = PHY_SPEED_100;
    //CONST_PHY_MIISTAT_SPEED_GIGA
    else if(speed & 0x2)
        phy_dev->speed  = PHY_SPEED_1000;
    //CONST_PHY_MIISTAT_SPEED_BZ2G5
    else if(speed & 0x4)
        phy_dev->speed  = PHY_SPEED_2500;
    else
        phy_dev->speed  = PHY_SPEED_UNKNOWN;

    //if (FIELD_GET(ret, PHY_MIISTAT_DPX) == CONST_PHY_MIISTAT_DPX_FDX)
    if (((val & 0x8) >> 3) == 0x1)
        phy_dev->duplex = PHY_DUPLEX_FULL;
    else
        phy_dev->duplex = PHY_DUPLEX_HALF;
#endif

    return 0;
}

static int _phy_eee_get(phy_dev_t *phy_dev, int *enable)
{
    uint16_t val;

    // initial
    *enable = 0;

    /* reg 7.60 : 100/1G EEE Status */
    PHY_READ(phy_dev, 0x7, 0x3c, &val);

    // EEE_100BT
    if (val & 0x2) *enable = 1;

    // EEE_1000BT
    if (val & 0x4) *enable = 1;

#if 0
    /* reg 7.62 : 2P5G EEE Status */
    PHY_READ(phy_dev, 0x7, 0x3e, &val);

    // EEE_2P5G
    if (val & 0x1) *enable = 1;
#endif

    return 0;
}

static int _phy_eee_set(phy_dev_t *phy_dev, int enable)
{
    uint16_t val;

    /* reg 7.60 : 100/1G EEE Status */
    PHY_READ(phy_dev, 0x7, 0x3c, &val);

    if (enable == 1)
        val = 0x6;
    else
        val = 0x0;

    PHY_WRITE(phy_dev, 0x7, 0x3c, val);

#if 0
    /* reg 7.62 : 2P5G EEE Status */
    PHY_READ(phy_dev, 0x7, 0x3e, &val);

    if (enable == 1)
        val = 0x1;
    else
        val = 0x0;

    PHY_WRITE(phy_dev, 0x7, 0x3e, val);
#endif

    return 0;
}

static int _phy_eee_resolution_get(phy_dev_t *phy_dev, int *enable)
{
    uint16_t val;

    /* reg 0.24 : MII status */
    PHY_READ(phy_dev, 0x0, 0x18, &val);

    /* Check if the link partner auto-negotiated EEE capability */
    *enable = (val >> 8) ? 1 : 0;

    return 0;
}

static int _phy_power_get(phy_dev_t *phy_dev, int *enable)
{
    uint16_t val;

    PHY_READ(phy_dev, 0x1, 0x0, &val);

    *enable = (val & (1 << 11)) ? 0 : 1;

    return 0;
}

static int _phy_power_set(phy_dev_t *phy_dev, int enable)
{
    uint16_t val;

    PHY_READ(phy_dev, 0x1, 0x0, &val);

    if (enable)
        val &= ~(1 << 11); /* Power up */
    else
        val |= (1 << 11); /* Power down */

    PHY_WRITE(phy_dev, 0x1, 0x0, val);

    return 0;
}

static int _phy_dev_add(phy_dev_t *phy_dev)
{
    enabled_phys |= (1 << (phy_dev->addr));

    return 0;
}

static int _phy_dev_del(phy_dev_t *phy_dev)
{
    enabled_phys &= ~(1 << (phy_dev->addr));

    return 0;
}

static int _phy_init(phy_dev_t *phy_dev)
{
    uint16_t val;

    PHY_READ(phy_dev, 0x1e, 0x8, &val);

    // set SGMII mode to be force
    val &= ~(1 << 12);
    PHY_WRITE(phy_dev, 0x1e, 0x8, val);

    _phy_speed_set(phy_dev, PHY_SPEED_AUTO, PHY_DUPLEX_FULL);

    return 0;
}

static void phys_readOUI(uint32_t phy_map)
{
    int i;
    phy_dev_t *phy_dev = NULL;

    for (i = 0; i < 32; i++)
    {
        if (!(phy_map & (1 << i))) continue;
        phy_dev = phy_dev_get(PHY_TYPE_GPY211, i);
        _phy_phyid_get(phy_dev, &devId);
        dbg("GPY211 Device ID 0x%x\n", devId);
    }
}

static int _phy_drv_init(phy_drv_t *phy_drv)
{
    phys_readOUI(enabled_phys);
    phy_drv->initialized = 1;

    return 0;
}

phy_drv_t phy_drv_GPY211 =
{
    .phy_type = PHY_TYPE_GPY211,
    .name = "GPY211",

    .drv_init = _phy_drv_init,
    .init = _phy_init,
    .dev_add = _phy_dev_add,
    .dev_del = _phy_dev_del,
    .power_get = _phy_power_get,
    .power_set = _phy_power_set,
    .eee_get = _phy_eee_get,
    .eee_set = _phy_eee_set,
    .eee_resolution_get = _phy_eee_resolution_get,
    .read_status = _phy_read_status,
    .phyid_get = _phy_phyid_get,
    .speed_set = _phy_speed_set,
    .caps_get = _phy_gpy211_caps_get,
    .caps_set = _phy_gpy211_caps_set,
};
