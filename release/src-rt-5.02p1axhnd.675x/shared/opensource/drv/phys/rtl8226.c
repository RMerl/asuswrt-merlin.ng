/*
 * Intergate rtl8226 driver to BCM4908
 */

#include "phy_drv.h"
#include "phy_drv_mii.h"
#include "bcm_gpio.h"

#include "nic_rtl8226.h"
#include "nic_rtl8226b_init.h"

extern phy_drv_t phy_drv_rtl8226;
static uint32_t enabled_phys;

#define BUS_WRITE(a, b, c, d)       if ((ret = _bus_write(a, b, c, d))) goto Exit;
#define BUS_WRITE_ALL(a, b, c, d)   if ((ret = _bus_write_all(a, b, c, d))) goto Exit;
#define BUS_WRITE_AND_VERIFY_ALL(a, b, c, d, e)   if ((ret = _bus_write_and_verify_all(a, b, c, d, e))) goto Exit;

#define PHY_READ(a, b, c, d)        if ((ret = phy_bus_c45_read(a, b, c, d))) goto Exit;
#define PHY_WRITE(a, b, c, d)       if ((ret = phy_bus_c45_write(a, b, c, d))) goto Exit;

extern int _bus_read(uint32_t addr, uint16_t dev, uint16_t reg, uint16_t *val)
{
    return phy_drv_rtl8226.bus_drv->c45_read(addr, dev, reg, val);
}

extern int _bus_write(uint32_t addr, uint16_t dev, uint16_t reg, uint16_t val)
{
    return phy_drv_rtl8226.bus_drv->c45_write(addr, dev, reg, val);
}

static int _phy_power_get(phy_dev_t *phy_dev, int *enable)
{
    uint16_t val;
    int ret;

    PHY_READ(phy_dev, 0x01, 0x0000, &val);

    *enable = (val & (1 << 11)) ? 0 : 1;

Exit:
    return ret;
}

static int _phy_power_set(phy_dev_t *phy_dev, int enable)
{
#if 1
		printk("[%s-%d]: return RTL8226B Power set.\n",__FUNCTION__,__LINE__);
		return 0;
#endif

    uint16_t val;
    int ret;

    PHY_READ(phy_dev, 0x01, 0x0000, &val);

    if (enable)
        val &= ~(1 << 11); /* Power up */
    else
        val |= (1 << 11); /* Power down */

    PHY_WRITE(phy_dev, 0x01, 0x0000, val);

Exit:
    return ret;
}

static int _phy_apd_get(phy_dev_t *phy_dev, int *enable)
{
	BOOL status = FAILURE;
	BOOL val = FAILURE;

	status = Rtl8226_linkDownPowerSavingEnable_get(phy_dev->addr, &val);
	 if (status != SUCCESS)
	    goto Exit;

	 *enable = val;

Exit:
    return status;
}

static int _phy_apd_set(phy_dev_t *phy_dev, int enable)
{
	BOOL status = FAILURE;

	status = Rtl8226_linkDownPowerSavingEnable_set(phy_dev->addr, enable);
	if (status != SUCCESS)
		goto Exit;

Exit:
    return status;
}

static int _phy_pair_swap_set(phy_dev_t *phy_dev, int enable)
{
	BOOL status = FAILURE;

	status = Rtl8226_mdiSwapEnable_set(phy_dev->addr, enable);
	if (status != SUCCESS)
        goto Exit;

Exit:
    return status;
}

static int _phy_eee_get(phy_dev_t *phy_dev, int *enable)
{
	BOOL status = FAILURE;
	PHY_EEE_ENABLE pEeeEnable;

	status = Rtl8226_eeeEnable_get(phy_dev->addr, &pEeeEnable);
	if (status != SUCCESS)
        goto Exit;

    *enable = (pEeeEnable.EEE_100) || (pEeeEnable.EEE_1000) || (pEeeEnable.EEE_2_5G);

Exit:
    return status;
}

static int _phy_caps_get(phy_dev_t *phy_dev, int caps_type, uint32_t *pcaps);

static int _phy_eee_set(phy_dev_t *phy_dev, int enable)
{
	BOOL status = FAILURE;

	PHY_LINK_ABILITY phylinkability;
	PHY_EEE_ENABLE EeeEnable;
	memset(&phylinkability, 0x0, sizeof(phylinkability));
	memset(&EeeEnable, 0x0, sizeof(EeeEnable));

	status = Rtl8226_autoNegoAbility_get(phy_dev->addr, &phylinkability);
	if (status != SUCCESS)
        goto Exit;

	EeeEnable.EEE_100 = !!(phylinkability.Full_100 || phylinkability.Half_100);
	EeeEnable.EEE_1000 = !!(phylinkability.Full_1000);
	EeeEnable.EEE_2_5G = !!(phylinkability.adv_2_5G);

	status = Rtl8226_eeeEnable_set(phy_dev->addr, &EeeEnable);
	if (status != SUCCESS)
        goto Exit;

Exit:
    return status;
}

static int _phy_eee_resolution_get(phy_dev_t *phy_dev, int *enable)
{
    int ret;
    uint16_t val;

    /* EEE Resolution Status */
    PHY_READ(phy_dev, 7, 0x803e, &val);

    /* Check if the link partner auto-negotiated EEE capability */
    *enable = val & 0x3e ? 1 : 0;

Exit:
    return ret;
}


static int _phy_read_status(phy_dev_t *phy_dev)
{
	BOOL status = FAILURE;
	BOOL link = FAILURE;
	BOOL duplex = FAILURE;
	uint16_t speed;

    phy_dev->link = 0;
    phy_dev->speed = PHY_SPEED_UNKNOWN;
    phy_dev->duplex = PHY_DUPLEX_UNKNOWN;
    //phy_dev->pause_rx = 0;
    //phy_dev->pause_tx = 0;

	status = Rtl8226_is_link(phy_dev->addr, &link);
	if (status != SUCCESS)
        goto Exit;

    /* Copper link status */
    phy_dev->link = link;

    if (!phy_dev->link)
        goto Exit;

	/* Copper speed */
	status = Rtl8226_speed_get(phy_dev->addr, &speed);
	if (status != SUCCESS)
        goto Exit;
	
	switch(speed)
	{
		case LINK_SPEED_10M:
		    phy_dev->speed = PHY_SPEED_10;
		    break;
		case LINK_SPEED_100M:
		    phy_dev->speed = PHY_SPEED_100;
		    break;
		case LINK_SPEED_1G:
		    phy_dev->speed = PHY_SPEED_1000;
		    break;
		case LINK_SPEED_2P5G:
		    phy_dev->speed = PHY_SPEED_2500;
		    break;

		default:
		    status = FAILURE;
		    break;
	}

	/* Copper duplex */
	status = Rtl8226_duplex_get(phy_dev->addr, &duplex);
	if (status != SUCCESS)
        goto Exit;

	if(duplex)
		phy_dev->duplex = PHY_DUPLEX_FULL;
	else
		phy_dev->duplex = PHY_DUPLEX_HALF;

    //phy_dev->pause_rx = ((val >> 1) & 0x1);
    //phy_dev->pause_tx = ((val >> 0) & 0x1);

Exit:
    return status;
}

int _phy_caps_get(phy_dev_t *phy_dev, int caps_type, uint32_t *pcaps)
{
    uint32_t caps = 0;

	BOOL status = FAILURE;
	PHY_LINK_ABILITY phylinkability;
	memset(&phylinkability, 0x0, sizeof(phylinkability));

    if ((caps_type != CAPS_TYPE_ADVERTISE) 
        && (caps_type != CAPS_TYPE_SUPPORTED))
        goto Exit;

	status = Rtl8226_autoNegoAbility_get(phy_dev->addr, &phylinkability);
	if (status != SUCCESS)
		goto Exit;

    if (phylinkability.FC)
        caps |= PHY_CAP_PAUSE;

    if (phylinkability.AsyFC)
        caps |= PHY_CAP_PAUSE_ASYM;

	if (phylinkability.Half_10)
        caps |= PHY_CAP_10_HALF;

    if (phylinkability.Full_10)
        caps |= PHY_CAP_10_FULL;

    if (phylinkability.Half_100)
        caps |= PHY_CAP_100_HALF;

    if (phylinkability.Full_100)
        caps |= PHY_CAP_100_FULL;

    if (phylinkability.Full_1000)
        caps |= PHY_CAP_1000_FULL;

    if (phylinkability.adv_2_5G)
        caps |= PHY_CAP_2500;

    *pcaps = caps;
Exit:
    return status;
}

static int _phy_caps_set(phy_dev_t *phy_dev, uint32_t caps)
{
	BOOL status = FAILURE;
	PHY_LINK_ABILITY phylinkability;
	memset(&phylinkability, 0x0, sizeof(phylinkability));

    /* Don't advertise 5G/10G speeds when the mac in HSGMII mode */
    if (phy_dev->mii_type == PHY_MII_TYPE_HSGMII)
        caps &= ~(PHY_CAP_5000 | PHY_CAP_10000);

    /* Don't advertise 2.5G/5G/10G speeds when the mac in SGMII mode */
    if (phy_dev->mii_type == PHY_MII_TYPE_SGMII)
        caps &= ~(PHY_CAP_2500 | PHY_CAP_5000 | PHY_CAP_10000);


    if (caps & PHY_CAP_10_HALF)
        phylinkability.Half_10 = 1;

    if (caps & PHY_CAP_10_FULL)
        phylinkability.Full_10 = 1;

	if (caps & PHY_CAP_100_HALF)
        phylinkability.Half_100 = 1;

    if (caps & PHY_CAP_100_FULL)
        phylinkability.Full_100 = 1;

    if (caps & PHY_CAP_1000_FULL)
        phylinkability.Full_1000 = 1;

    if (caps & PHY_CAP_2500)
        phylinkability.adv_2_5G = 1;

	if (caps & PHY_CAP_PAUSE)
        phylinkability.FC = 1;

    if (caps & PHY_CAP_PAUSE_ASYM)
        phylinkability.AsyFC = 1;
	
	status = Rtl8226_autoNegoAbility_set(phy_dev->addr, &phylinkability);
	if (status != SUCCESS)
		goto Exit;

Exit:
    return status;
}

static int _phy_speed_set(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex)
{
    int ret;
    uint32_t caps;

    if (speed == PHY_SPEED_UNKNOWN)
    {
        speed = PHY_SPEED_2500;
    }

    if ((ret = _phy_caps_get(phy_dev, CAPS_TYPE_ADVERTISE, &caps)))
        goto Exit;

    caps &= ~(PHY_CAP_10_HALF | PHY_CAP_10_FULL |
        PHY_CAP_100_HALF | PHY_CAP_100_FULL |    
        PHY_CAP_1000_FULL |PHY_CAP_2500);

    //caps |= PHY_CAP_AUTONEG;

    switch (speed)
    {
	    case PHY_SPEED_2500:
	        caps |= PHY_CAP_2500;
	    case PHY_SPEED_1000:
	        caps |= PHY_CAP_1000_FULL;
	    case PHY_SPEED_100:
	        caps |= PHY_CAP_100_HALF | ((duplex == PHY_DUPLEX_FULL) ? PHY_CAP_100_FULL : 0);
	        break;
		case PHY_SPEED_10:
	        caps |= PHY_CAP_10_HALF | ((duplex == PHY_DUPLEX_FULL) ? PHY_CAP_10_FULL : 0);
	        break;
	    default:
	        printk("Ignoring unsupported speed\n");
	        goto Exit;
	        break;
    }

    if ((ret = _phy_caps_set(phy_dev, caps)))
        goto Exit;

Exit:
    return ret;
}

static int _phy_phyid_get(phy_dev_t *phy_dev, uint32_t *phyid)
{
    int ret;
    uint16_t phyid1 = 0, phyid2 = 0;

	PHY_READ(phy_dev, 0x01, 0x0002, &phyid1);
    PHY_READ(phy_dev, 0x01, 0x0003, &phyid2);

    *phyid = phyid1 << 16 | phyid2;

Exit:
    return ret;
}

static int _phy_init(phy_dev_t *phy_dev)
{
    BOOL status = FAILURE;

	PHY_LINK_ABILITY phylinkability;
	memset(&phylinkability, 0x0, sizeof(phylinkability));

	status = Rtl8226_phy_reset(phy_dev->addr);
	if (status != SUCCESS)
        goto Exit;
		
	status = Rtl8226b_phy_init(phy_dev->addr, NULL, 0);
	if (status != SUCCESS)
        goto Exit;

	printk("[%s-%d]: RTL8226 PHY initialize successfully.\n",__FUNCTION__,__LINE__);

Exit:
    return status;
}

static void phy_reset_lift(phy_dev_t *phy_dev)
{
	int i;
    if (phy_dev->reset_gpio == -1)
        return;

    printk(" Lift PHY at address %d out of Reset by GPIO:%d Active %s\n", 
        phy_dev->addr, phy_dev->reset_gpio,
        phy_dev->reset_gpio_active_hi? "High": "Low");
    bcm_gpio_set_dir(phy_dev->reset_gpio, 1);
    bcm_gpio_set_data(phy_dev->reset_gpio, phy_dev->reset_gpio_active_hi);
	
	/* At least 70ms for a complete PHY reset */
    for(i=0;i<10;i++) udelay(8000);
    bcm_gpio_set_data(phy_dev->reset_gpio, !phy_dev->reset_gpio_active_hi);
} 

static void phys_reset_and_readOUI(uint32_t phy_map)
{
    int i;
	uint32_t phyid = 0;
    phy_dev_t *phy_dev;

    for (i = 0; i < 32; i++) {
        if (!(phy_map & (1 << i)))
            continue;
        phy_dev = phy_dev_get(PHY_TYPE_RTL8226, i);
        phy_reset_lift(phy_dev);
		_phy_phyid_get(phy_dev, &phyid);
		printk("[%s-%d]:ODM Read RTL8226 Device ID=0x%x\n",__FUNCTION__,__LINE__,phyid);
    }
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

#if 0
static int _phy_drv_init(phy_drv_t *phy_drv)
{
	//Reset phy and Read phy device ID
	phys_reset_and_readOUI(enabled_phys);	

    phy_drv->initialized = 1;

    return 0;
}
#endif

phy_drv_t phy_drv_rtl8226 =
{
	.phy_type = PHY_TYPE_RTL8226,
	.name = "RTL8226",
	.init = _phy_init,
	.dev_add = _phy_dev_add,
	.dev_del = _phy_dev_del,
	//.drv_init = _phy_drv_init,
	.eee_get = _phy_eee_get,
	.eee_set = _phy_eee_set,
	.eee_resolution_get = _phy_eee_resolution_get,
	.pair_swap_set = _phy_pair_swap_set,
	.power_get = _phy_power_get,
	.power_set = _phy_power_set,
	.apd_get = _phy_apd_get,
	.apd_set = _phy_apd_set,
	.read_status = _phy_read_status,
	.phyid_get = _phy_phyid_get,
	.caps_get = _phy_caps_get,
	.caps_set = _phy_caps_set,
	.speed_set = _phy_speed_set,
};
EXPORT_SYMBOL(_bus_read);
EXPORT_SYMBOL(_bus_write);
