#include <linux/device.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/module.h>
#include <linux/thermal.h>
#include <linux/platform_device.h>
#include <linux/unistd.h>
#include <linux/reboot.h>
#include <linux/umh.h>
#include <linux/version.h>
#include "bcm_thermal.h"

static int threshold[NUM_TRIPS] = { 0 };
static int hysteresis[NUM_TRIPS] = { 0 };
static int last_announcement[NUM_TRIPS] = { 0 };
struct thermal_cooling_device *cooling_dev[NUM_TRIPS];
struct thermal_zone_device *zone_dev;

static int is_last_trip(int trip)
{
    return trip == NUM_TRIPS - 1;
}

static int user_state_notify(int trip, int enable)
{
    int ret = 0;

#ifdef USER_NOTIFY_CMD
    char _trip[16] = { 0 };
    char _enable[16] = { 0 };
    char *envp[] = { NULL };
    char *argv[] = { USER_NOTIFY_CMD, _trip, _enable, NULL };

    sprintf(_trip, "%d", trip);
    sprintf(_enable, "%d", enable);

    ret = call_usermodehelper(argv[0], argv, envp, UMH_WAIT_EXEC);
#endif

    return ret;
}

static int _bcm_thermal_get_temperature(int *temp)
{
    int ret = 0;

#ifdef TEMP_TEST
    static int tmp = 0;
    static int up = 1;

    if (tmp == 130)
        up = 0;

    if (tmp == 0)
        up = 1;

    *temp = tmp * 1000;
    if (up)
        tmp++;
    else
        tmp--;

    return 0;
#else
    ret = bcm_thermal_get_temperature(temp);
#endif

    return ret;
}

#define TEMP_SAMPLES    5
static int temp_samples[TEMP_SAMPLES] = { 0 };

static int get_avg_temp(struct thermal_zone_device *dev, int *temp)
{
    static int sample_index = 0;
    static int samples = 0;
    int temp_total = 0, temp_cur;
    int i, ret;

    ret = _bcm_thermal_get_temperature(&temp_cur);
    if (ret)
        return -1;

    if (samples < 5)
        samples++;

    temp_samples[sample_index] = temp_cur;

    for (i = 0; i < samples; i++)
        temp_total += temp_samples[i];

    *temp = temp_total / samples;

    sample_index++;

    if (sample_index == TEMP_SAMPLES)
        sample_index = 0;

    return 0;
}

static int get_trip_by_dev_id(int dev_id)
{
    int i;

    for (i = 0; i < NUM_TRIPS; i++)
    {
        if (cooling_dev[i] && cooling_dev[i]->id == dev_id)
            return i;
    }

    printk("Error finding the trip number for cooling device %d\n", dev_id);

    return -1;
}

static int get_max_state(struct thermal_cooling_device *dev, unsigned long *states)
{
    *states = 1;

    return 0;
}

static int get_cur_state(struct thermal_cooling_device *dev, unsigned long *state)
{
    int trip = get_trip_by_dev_id(dev->id);

    if (trip < 0)
        return 0;

    *state = last_announcement[trip];

    return 0;
}

static int set_cur_state(struct thermal_cooling_device *dev, unsigned long state)
{
    int trip = get_trip_by_dev_id(dev->id);

    if (trip < 0)
        return 0;

    if (last_announcement[trip] == state)
        return 0;
    else
        last_announcement[trip] = state;

    if (state)
        printk("Trip %d: temperature passed above threshold: %d mC\n", trip, threshold[trip]);
    else
        printk("Trip %d: temperature passed below hysteresis: %d mC\n", trip, threshold[trip] - hysteresis[trip]);

    if (is_last_trip(trip))
        orderly_reboot();

    bcm_thermal_state_notify(trip, state ? 1 : 0);
    user_state_notify(trip, state ? 1 : 0);

    return 0;
}

static struct thermal_cooling_device_ops cooling_ops =
{
    .get_max_state = get_max_state,
    .get_cur_state = get_cur_state,
    .set_cur_state = set_cur_state,
};

// #define DEBUG_TEMPERATURE
#ifdef DEBUG_TEMPERATURE
static int dbg_temperature = -999;
module_param(dbg_temperature, int, 0644);
#endif // #ifdef DEBUG_TEMPERATURE

static int get_temp(struct thermal_zone_device *dev, int *temp)
{
    int _temp, ret;
    int trip = get_trip_by_dev_id(dev->id);

    if (is_last_trip(trip))
        ret = _bcm_thermal_get_temperature(&_temp);
    else
        ret = get_avg_temp(dev, &_temp);

#ifdef DEBUG_TEMPERATURE
    _temp = (dbg_temperature > -999 ? dbg_temperature : 40) * 1000;
    ret = 0;
#endif

    if (ret)
        return ret;

    dev_dbg(&dev->device, "Temperature in Celsius: %d.%03d\n",
        _temp / 1000, (_temp < 0 ? -_temp : _temp) % 1000);

    *temp = _temp;

    return 0;  
}

static int get_trip_type(struct thermal_zone_device *dev, int trip, enum thermal_trip_type *type)
{
    *type = THERMAL_TRIP_ACTIVE;

    return 0;
}

static int get_trip_temp(struct thermal_zone_device *dev, int trip, int *temp)
{
    *temp = threshold[trip];

    return 0;
}

static int set_trip_temp(struct thermal_zone_device *dev, int trip, int temp) 
{
    threshold[trip] = temp;

    return 0;
}


int get_trip_hyst(struct thermal_zone_device *dev, int trip, int *temp)
{
    *temp = hysteresis[trip];

    return 0;
}

static int set_trip_hyst(struct thermal_zone_device *dev, int trip, int temp)
{
    hysteresis[trip] = temp;

    return 0;
}

struct thermal_zone_device_ops thermal_ops =
{
    .get_temp = get_temp,
    .get_trip_type = get_trip_type,
    .get_trip_temp = get_trip_temp,
    .set_trip_temp = set_trip_temp,
    .get_trip_hyst = get_trip_hyst,
    .set_trip_hyst = set_trip_hyst,
};

static void thermal_driver_cleanup(void)
{
    int i;

    for (i = 0; i < NUM_TRIPS; i++)
    {
        if (cooling_dev[i])
            thermal_cooling_device_unregister(cooling_dev[i]);
    }

    if (zone_dev)
        thermal_zone_device_unregister(zone_dev);
}

static const bool dt_property_read_bool(struct device_node *node, const char *propname)
{
    return of_get_property(node, propname, NULL) ? 1 : 0;
}

static const uint32_t dt_property_read_u32_default(struct device_node *node, const char *propname, uint32_t defval)
{
    const uint32_t *val = of_get_property(node, propname, NULL);

    if (!val)
        return defval;

    return be32_to_cpup(val);
}

static int thermal_driver_probe(struct platform_device *plat_dev)
{
    struct device *dev = &plat_dev->dev;
    struct device_node *np = dev->of_node;
    /* structs allocated by devm_ are automatically freed when device exits */
    struct thermal_zone_params *zone_params = NULL;
    char property_name[16];
    int i, reboot_temp, boot_temp;

    thermal_driver_chip_probe(plat_dev);

    zone_params = devm_kzalloc(&plat_dev->dev, sizeof(struct thermal_zone_params), GFP_KERNEL);
    if (!zone_params)
    {
        dev_err(&plat_dev->dev, "Can't allocate dev memory\n");
        goto Error;
    }

    strcpy(zone_params->governor_name, "bang_bang");
    zone_params->no_hwmon = true;
    zone_params->num_tbps = 0;

    reboot_temp = 1000 * dt_property_read_u32_default(np, "reboot-temp", 0);
    boot_temp = 1000 * dt_property_read_u32_default(np, "boot-temp", 0);

    for (i = 0; i < NUM_TRIPS; i++)
    {
        sprintf(property_name, "threshold%d", i);
        threshold[i] = 1000 * dt_property_read_u32_default(np, property_name, 0);
        sprintf(property_name, "hysteresis%d", i);
        hysteresis[i] = 1000 * dt_property_read_u32_default(np, property_name, DEFAULT_HYSTERESIS);
        sprintf(property_name, "enabled%d", i);
        last_announcement[i] = dt_property_read_bool(np, property_name);

        /* last trip is reserved for reboot */
        if (reboot_temp && is_last_trip(i))
        {
            threshold[i] = reboot_temp;
            hysteresis[i] = reboot_temp - boot_temp;
        }

        if (!threshold[i])
            continue;

        dev_err(&plat_dev->dev, "Trip %d: threshold=%d mC hysteresis=%d mC\n", i, threshold[i], hysteresis[i]);

        sprintf(property_name, "cooling%d", i);
        cooling_dev[i] =
            thermal_cooling_device_register(property_name, plat_dev, &cooling_ops);
        if (IS_ERR(cooling_dev[i]))
        {
            cooling_dev[i] = NULL;
            dev_err(&plat_dev->dev, "Can't create cooling device for trip %d\n", i);
            goto Error;
        }
    }

    zone_dev = thermal_zone_device_register(
        "Broadcom_Thermal",                     /* name */
        NUM_TRIPS,                              /* trips */
        0,                                      /* mask */
        NULL,                                   /* device data */ 
        &thermal_ops,
        zone_params,                             /* thermal zone params */
        1000,                                   /* passive delay */
        1000                                    /* polling delay */
        );

    if (IS_ERR(zone_dev))
    {
        dev_err(&plat_dev->dev, "Failed to register thermal zone device %ld\n",
            PTR_ERR(zone_dev));
        goto Error;
    }

    for (i = 0; i < NUM_TRIPS; i++)
    {
        thermal_zone_bind_cooling_device(zone_dev, i, cooling_dev[i],
            THERMAL_NO_LIMIT, THERMAL_NO_LIMIT, THERMAL_WEIGHT_DEFAULT);
    }

#if (LINUX_VERSION_CODE > KERNEL_VERSION(5,15,0))
    thermal_zone_device_enable(zone_dev);
#endif

    return 0;

Error:
    thermal_driver_cleanup();
    return -1;
}

static int thermal_driver_remove(struct platform_device *plat_dev)
{
    thermal_driver_cleanup();
    thermal_driver_chip_remove(plat_dev);

    return 0;
}

static const struct of_device_id broadcom_thermal_id_table[] = {
    { .compatible = "brcm,therm" },
    {}
};

struct platform_driver broadcom_thermal_driver = {
    .probe = thermal_driver_probe,  
    .remove = thermal_driver_remove,
    .driver = {
        .name = "bcm_thermal_drv",
        .of_match_table = broadcom_thermal_id_table
    },
};

module_platform_driver(broadcom_thermal_driver);

MODULE_DESCRIPTION("Broadcom thermal driver");
MODULE_LICENSE("GPL");
