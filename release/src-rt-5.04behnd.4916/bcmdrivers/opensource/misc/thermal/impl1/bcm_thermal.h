#ifndef _BCM_THERMAL_H
#define _BCM_THERMAL_H

#define NUM_TRIPS                   12
#define DEFAULT_HYSTERESIS          2

#ifdef CONFIG_BCM_PON
#define USER_NOTIFY_CMD  "/etc/init.d/thermal.sh"
#endif

int bcm_thermal_get_temperature(int *temp);
int bcm_thermal_state_notify(int trip, int enable);
int thermal_driver_chip_probe(struct platform_device *plat_dev);
int thermal_driver_chip_remove(struct platform_device *plat_dev);

#endif
