#include <dm.h>
#include "pmc_drv.h"

static int boot_temp = -1;

static int get_pvtmon_temperature(int *val)
{
    int res, adc = -1;

    res = GetPVT(kTEMPERATURE, 0, &adc);
    if (!res)
        *val = pmc_convert_pvtmon(kTEMPERATURE, adc);

    return res;
}

int bcm_thermal_is_temperature_safe(void)
{
    int temp = 0, ret;

    if (boot_temp <= 0)
        return 1;

    ret = get_pvtmon_temperature(&temp);
    if (ret || temp < boot_temp * 1000)
        return 1;

    printf("Temperature is too high: %d.%dC, above threshold: %dC\n", temp / 1000, temp % 1000, boot_temp);

    return 0;
}

static int bcm_thermal_probe(struct udevice *udev)
{
    ofnode node = udev->node;
    const void *val;

    if (!ofnode_valid(node))
        return -ENODEV;

    val = ofnode_get_property(node, "boot-temp", NULL);
    if (val)
        boot_temp = be32_to_cpup(val);

    return 0;
}

void bcm_thermal_init(void)
{
	struct udevice *dev;

	uclass_get_device_by_driver(UCLASS_NOP, DM_GET_DRIVER(thermal_drv), &dev);
}

static const struct udevice_id bcm_thermal_ids[] = {
    { .compatible = "brcm,therm" },
    { }
};

U_BOOT_DRIVER(thermal_drv) = {
    .name = "bcm_thermal",
    .id = UCLASS_NOP,
    .of_match = bcm_thermal_ids,
    .probe = bcm_thermal_probe,
};
