#include <linux/platform_device.h>
#include <linux/printk.h>
#include "pmc_core_api.h"

int bcm_thermal_get_temperature(int *temp)
{
    int ret, adc = -1;

    ret = GetPVTKH2(kTEMPERATURE, 0, &adc);
    if (ret)
    {
        printk("Failed to get RAIL 0 temperature: ret=%d\n", ret);
        return ret;
    }

    *temp = pmc_convert_pvtmon(kTEMPERATURE, adc);

    return 0;  
}

int bcm_thermal_state_notify(int trip, int enable)
{
    return 0;
}

int thermal_driver_chip_probe(struct platform_device *plat_dev)
{
    return 0;
}

int thermal_driver_chip_remove(struct platform_device *plat_dev)
{
    return 0;
}
