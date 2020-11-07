#include <linux/device.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/module.h>
#include <linux/thermal.h>
#include <linux/platform_device.h>
#include <linux/cpu.h>
#include <linux/unistd.h>
#include <pmc_drv.h>
#include <bcm_map_part.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0)
typedef unsigned long tempmc_t;
#define Thermal_zone_bind_cooling_device thermal_zone_bind_cooling_device
#else
typedef int tempmc_t;
#define Thermal_zone_bind_cooling_device(a, b, c, d, e)		\
	thermal_zone_bind_cooling_device(a, b, c, d, e, THERMAL_WEIGHT_DEFAULT)
#endif

void cpufreq_set_freq_max(unsigned maxdiv);

#if !defined(CONFIG_BCM_CPUIDLE_CLK_DIVIDER) && \
	!(defined(CONFIG_CPU_FREQ) && defined(CONFIG_BCM_KF_ARM_BCM963XX))
void cpufreq_set_freq_max(unsigned maxdiv)
{
#if !defined (CONFIG_BCM963158) && !defined (CONFIG_BCM947622) && !defined (CONFIG_BCM963178)
	BIUCTRL->cluster_clk_pattern[0] = maxdiv == 1 ? 0xffffffff : 0x55555555;
	BIUCTRL->cluster_clk_ctrl[0] = 1 << 4;	// enable user clock-patterns
#endif
}
#endif

// TEMPERATURE LIMITS
#define BROADCOM_THERMAL_NUM_TRIP_TEMPERATURES                         5

#define BROADCOM_THERMAL_LOW_TEMPERATURE_COMPENSATION_MILLICELSIUS     15000
#define BROADCOM_THERMAL_LOW_TEMPERATURE_HYSTERESIS_MILLICELSIUS       10000

// QUAD/TRIPLE CPU is meant to apply to 4908, 63158, 47622, 63178
#define BROADCOM_THERMAL_HIGH_TEMPERATURE_QUAD_CPU_COMPENSATION_1_MILLICELSIUS  115000
#define BROADCOM_THERMAL_HIGH_TEMPERATURE_QUAD_CPU_COMPENSATION_2_MILLICELSIUS  125000

// DUAL CPU is meant to apply to the 4906
#define BROADCOM_THERMAL_HIGH_TEMPERATURE_DUAL_CPU_COMPENSATION_1_MILLICELSIUS  100000
#define BROADCOM_THERMAL_HIGH_TEMPERATURE_DUAL_CPU_COMPENSATION_2_MILLICELSIUS  110000

#define BROADCOM_THERMAL_HIGH_TEMPERATURE_HYSTERESIS_MILLICELSIUS      5000

/*-------------------------*
 *   CPU Cooling Devices   *
 *-------------------------*/

#ifdef CONFIG_BCM94908
/* CPU PROCESSORS COLD COMPENSATION DEVICE */
static int broadcom_cpu_cold_compensation_lastAnnouncement = 2;

int broadcom_cpu_cold_compensation_get_max_state (struct thermal_cooling_device *dev, unsigned long *states)
{
  *states = 2;
  return 0;
}

int broadcom_cpu_cold_compensation_get_cur_state (struct thermal_cooling_device *dev, unsigned long *currentState)
{
  *currentState = 0;
  return 0;
}

int broadcom_cpu_cold_compensation_set_cur_state (struct thermal_cooling_device *dev, unsigned long state)
{
  switch (state)
  {
    case 0:
      if (broadcom_cpu_cold_compensation_lastAnnouncement != 0) {
        dev_crit(&dev->device,"Handling Cold\n");
        RecloseAVS (1);
        broadcom_cpu_cold_compensation_lastAnnouncement = 0;
      }
    case 1:
      break;
    case 2:
      if (broadcom_cpu_cold_compensation_lastAnnouncement != 2) {
        dev_crit(&dev->device,"Go normal\n");
        RecloseAVS (0);
        broadcom_cpu_cold_compensation_lastAnnouncement = 2;
      }
      break;
  }
  return 0;
}

struct thermal_cooling_device_ops broadcomCpuColdCompensationOps =
{
  .get_max_state = broadcom_cpu_cold_compensation_get_max_state,
  .get_cur_state = broadcom_cpu_cold_compensation_get_cur_state,
  .set_cur_state = broadcom_cpu_cold_compensation_set_cur_state,
};
#endif

/* CPU PROCESSORS COOLING DEVICE */

int broadcom_cpu_cooling_get_max_state (struct thermal_cooling_device *dev, unsigned long *states)
{
  *states = 3;
  return 0;
}

int broadcom_cpu_cooling_get_cur_state (struct thermal_cooling_device *dev, unsigned long *currentState)
{
  *currentState = 0;
  return 0;
}

// mark cpu as not-present to prevent other code from onlining;
// (duplicates [unexported] set_cpu_present for module);
// also update sysfs cpu offline status
static void brcm_cpu_absent(unsigned int cpu)
{
  cpumask_clear_cpu(cpu, (struct cpumask *)cpu_present_mask);
  get_cpu_device(cpu)->offline = true;
}

// mark cpu as present to allow other code from onlining;
// (duplicates [unexported] set_cpu_present for module);
// also update sysfs cpu offline status
static void brcm_cpu_present(unsigned int cpu)
{
  cpumask_set_cpu(cpu, (struct cpumask *)cpu_present_mask);
  get_cpu_device(cpu)->offline = false;
}

int broadcom_cpu_cooling_set_cur_state (struct thermal_cooling_device *dev, unsigned long state)
{
  int cpuIndex, rc;

  switch (state)
  {
    case 0:
      for (cpuIndex = 1; cpuIndex < num_possible_cpus(); cpuIndex++) {
          if (!cpu_online(cpuIndex)) {
            dev_crit(&dev->device,"take CPU#%d online\n", cpuIndex);
	  brcm_cpu_present(cpuIndex); // mark present before cpu_up
	  if ((rc = cpu_up(cpuIndex)) != 0)
	    brcm_cpu_absent(cpuIndex);
        }
      }
      break;
    case 1:
      break;
    case 2:
      // Take the last possible CPU offline
      cpuIndex = num_possible_cpus() - 1;
      if (cpuIndex < 1)
        break;
      if (cpu_online(cpuIndex)) {
        dev_crit(&dev->device,"take CPU#%d offline\n", cpuIndex);
        if ((rc = cpu_down(cpuIndex)) == 0)
          brcm_cpu_absent(cpuIndex);
      }
      break;
    case 3:
      for (cpuIndex = 1; cpuIndex < num_possible_cpus(); cpuIndex++) {
          if (cpu_online(cpuIndex)) {
            dev_crit(&dev->device,"take CPU#%d offline\n", cpuIndex);
	  if ((rc = cpu_down(cpuIndex)) == 0)
	    brcm_cpu_absent(cpuIndex);
        }
      }
      break;
  }
  return 0;
}

struct thermal_cooling_device_ops broadcomCpuCoolingOps =
{
  .get_max_state = broadcom_cpu_cooling_get_max_state,
  .get_cur_state = broadcom_cpu_cooling_get_cur_state,
  .set_cur_state = broadcom_cpu_cooling_set_cur_state,
};

/* CPU FREQUENCY COOLING DEVICE */

static int broadcom_cpufreq_cooling_lastAnnouncement = 0;

int broadcom_cpufreq_cooling_get_max_state (struct thermal_cooling_device *dev, unsigned long *states)
{
  *states = 2;
  return 0;
}

int broadcom_cpufreq_cooling_get_cur_state (struct thermal_cooling_device *dev, unsigned long *currentState)
{
  *currentState = 0;
  return 0;
}

int broadcom_cpufreq_cooling_set_cur_state (struct thermal_cooling_device *dev, unsigned long state)
{
  switch (state)
  {
    case 0:
      if (broadcom_cpufreq_cooling_lastAnnouncement != 0) {
        dev_crit(&dev->device,"Go to max frequency\n");
        cpufreq_set_freq_max (1);
        broadcom_cpufreq_cooling_lastAnnouncement = 0;
      }
    case 1:
      break;
    case 2:
      if (broadcom_cpufreq_cooling_lastAnnouncement != 2) {
        dev_crit(&dev->device,"Go to low frequency\n");
        cpufreq_set_freq_max (2);
        broadcom_cpufreq_cooling_lastAnnouncement = 2;
      }
      break;
  }
  return 0;
}

struct thermal_cooling_device_ops broadcomCpuFreqCoolingOps =
{
  .get_max_state = broadcom_cpufreq_cooling_get_max_state,
  .get_cur_state = broadcom_cpufreq_cooling_get_cur_state,
  .set_cur_state = broadcom_cpufreq_cooling_set_cur_state,
};
/*------------------------*
 * End CPU Cooling Device *
 *------------------------*/


/*--------------------*
 * Thermal Sensor     *
 *--------------------*/
 
struct volatile_thermal {
	uint32 __iomem volatile *tempSensor;
};

static int tripTemperatures[BROADCOM_THERMAL_NUM_TRIP_TEMPERATURES];

// #define DEBUG_TEMPERATURE
#ifdef DEBUG_TEMPERATURE
static int dbg_temperature = -999;
module_param(dbg_temperature, int, 0644);
#endif // #ifdef DEBUG_TEMPERATURE

int broadcomTempDrv_get_temp (struct thermal_zone_device *thermDev, tempmc_t *tempMillicelsius)
{
  int tempMC;
#ifdef DEBUG_TEMPERATURE
  *tempMillicelsius = (dbg_temperature > -999 ? dbg_temperature : 40) * 1000;
#elif defined (CONFIG_BCM963158) || defined (CONFIG_BCM947622) || defined (CONFIG_BCM963178)
  int ret, adc = -1;

  ret = GetPVT(kTEMPERATURE, 0, &adc);
  if (ret) {
    dev_err(&thermDev->device, "Failed to get RAIL 0 temperature, ret=%d\n", ret);
    return ret;
  }
  *tempMillicelsius = pmc_convert_pvtmon(kTEMPERATURE, adc);

  ret = BIUCFG->bac.cpu_therm_temp;
  if (!(ret & (1<<31))) {
    dev_err(&thermDev->device, "Failed to get CPU temperature, ret=%d\n", ret);
    return -1;
  }
  ret = pmc_convert_pvtmon(kTEMPERATURE, ret & 0x3ff);
  if (ret > (int)*tempMillicelsius)
    *tempMillicelsius = ret;
#else
  struct volatile_thermal *tempSensor = (struct volatile_thermal * ) thermDev->devdata;
  int regVal = *tempSensor->tempSensor;
  regVal &= 0x000003ff; 
  *tempMillicelsius = (4133500 - regVal * 4906) / 10;
#endif
  tempMC = (int) *tempMillicelsius; 
  dev_dbg(&thermDev->device, "Temperature in Celsius              %d.%03d\n",
          tempMC / 1000, (tempMC < 0 ? -tempMC : tempMC) % 1000);
  return 0;  
}

int broadcomTempDrv_get_trip_type(struct thermal_zone_device *thermalZoneDev, int trip, enum thermal_trip_type *type)
{
   switch (trip) {
     case 0:
     case 1:
     case 2:
       *type = THERMAL_TRIP_ACTIVE;
       break;
     default:
      *type = THERMAL_TRIP_ACTIVE;
   }
   
   return 0;
}

int broadcomTempDrv_set_trip_temp (struct thermal_zone_device *thermalZoneDev, int trip, tempmc_t tempMillicelsius) 
{
  if ((trip >= 0) && (trip < BROADCOM_THERMAL_NUM_TRIP_TEMPERATURES))
  {
    tripTemperatures[trip] = tempMillicelsius;
  }
  return 0;
}


int broadcomTempDrv_get_trip_temp (struct thermal_zone_device *thermalZoneDev, int trip, tempmc_t *tempMillicelsius)
{
   if ((trip >= 0) && (trip < BROADCOM_THERMAL_NUM_TRIP_TEMPERATURES))
   {
     *tempMillicelsius = tripTemperatures[trip];
     return 0;
   }

   return -1;
}

int broadcomTempDrv_get_trip_hyst (struct thermal_zone_device *thermalZoneDev, int trip, tempmc_t *tempMillicelsius)
{
   switch (trip) {
     case 0:
     case 1:
     case 2:
     default:
      *tempMillicelsius = 0;
   }
   
   return 0;
}

int broadcomTempDrv_notify (struct thermal_zone_device *thermalZoneDev, int trip, enum thermal_trip_type type)
{
  return 0;
}
  
struct thermal_zone_device_ops thermalOps =
{
  .get_temp = broadcomTempDrv_get_temp,
  .set_trip_temp = broadcomTempDrv_set_trip_temp,
  .get_trip_type = broadcomTempDrv_get_trip_type,
  .get_trip_temp = broadcomTempDrv_get_trip_temp,
  .get_trip_hyst = broadcomTempDrv_get_trip_hyst,
  .notify = broadcomTempDrv_notify,
};

static struct {
#ifdef CONFIG_BCM94908
  struct thermal_cooling_device *cpuCoolCompDev;
#endif
  struct thermal_cooling_device *cpuCoolDev;
  struct thermal_cooling_device *cpuFreqDev;
  struct thermal_zone_device *thermalZoneDev;
} broadcomTempDrv_data;

static void broadcomTempDrv_cleanup(void)
{
#ifdef CONFIG_BCM94908
  if (!IS_ERR_OR_NULL(broadcomTempDrv_data.cpuCoolCompDev))
    thermal_cooling_device_unregister(broadcomTempDrv_data.cpuCoolCompDev);
#endif
  if (!IS_ERR_OR_NULL(broadcomTempDrv_data.cpuCoolDev))
    thermal_cooling_device_unregister(broadcomTempDrv_data.cpuCoolDev);
  if (!IS_ERR_OR_NULL(broadcomTempDrv_data.cpuFreqDev))
    thermal_cooling_device_unregister(broadcomTempDrv_data.cpuFreqDev);
  if (!IS_ERR_OR_NULL(broadcomTempDrv_data.thermalZoneDev))
    thermal_zone_device_unregister(broadcomTempDrv_data.thermalZoneDev);

  memset(&broadcomTempDrv_data, 0, sizeof(broadcomTempDrv_data));
}

// the activate and register function
int broadcomTempDrv_init (struct platform_device *platDev)
{
  /* structs allocated by devm_ are automatically freed when device exits */
  struct volatile_thermal *tempSensor = NULL;
  struct thermal_zone_params *zoneParams = NULL;
  int numCpus = num_possible_cpus();

  dev_crit(&platDev->dev, "init (CPU count %d %d %d %d)\n", 
    num_present_cpus(),
    num_online_cpus(),
    num_possible_cpus(),
    num_active_cpus());

  if (numCpus == 2)
  {
    broadcomTempDrv_set_trip_temp (NULL, 0, BROADCOM_THERMAL_LOW_TEMPERATURE_COMPENSATION_MILLICELSIUS);
    broadcomTempDrv_set_trip_temp (NULL, 1, BROADCOM_THERMAL_LOW_TEMPERATURE_HYSTERESIS_MILLICELSIUS + BROADCOM_THERMAL_LOW_TEMPERATURE_COMPENSATION_MILLICELSIUS);
    broadcomTempDrv_set_trip_temp (NULL, 2, BROADCOM_THERMAL_HIGH_TEMPERATURE_DUAL_CPU_COMPENSATION_1_MILLICELSIUS - BROADCOM_THERMAL_HIGH_TEMPERATURE_HYSTERESIS_MILLICELSIUS);
    broadcomTempDrv_set_trip_temp (NULL, 3, BROADCOM_THERMAL_HIGH_TEMPERATURE_DUAL_CPU_COMPENSATION_1_MILLICELSIUS);
    broadcomTempDrv_set_trip_temp (NULL, 4, BROADCOM_THERMAL_HIGH_TEMPERATURE_DUAL_CPU_COMPENSATION_2_MILLICELSIUS);
  }
  else if (numCpus == 4 || numCpus == 3)
  {
    broadcomTempDrv_set_trip_temp (NULL, 0, BROADCOM_THERMAL_LOW_TEMPERATURE_COMPENSATION_MILLICELSIUS);
    broadcomTempDrv_set_trip_temp (NULL, 1, BROADCOM_THERMAL_LOW_TEMPERATURE_HYSTERESIS_MILLICELSIUS + BROADCOM_THERMAL_LOW_TEMPERATURE_COMPENSATION_MILLICELSIUS);
    broadcomTempDrv_set_trip_temp (NULL, 2, BROADCOM_THERMAL_HIGH_TEMPERATURE_QUAD_CPU_COMPENSATION_1_MILLICELSIUS - BROADCOM_THERMAL_HIGH_TEMPERATURE_HYSTERESIS_MILLICELSIUS);
    broadcomTempDrv_set_trip_temp (NULL, 3, BROADCOM_THERMAL_HIGH_TEMPERATURE_QUAD_CPU_COMPENSATION_1_MILLICELSIUS);
    broadcomTempDrv_set_trip_temp (NULL, 4, BROADCOM_THERMAL_HIGH_TEMPERATURE_QUAD_CPU_COMPENSATION_2_MILLICELSIUS);
  }
  else 
  {
    dev_err(&platDev->dev, "Can't handle %d CPUs\n", numCpus);
    goto error_out;
  }

  tempSensor = devm_kzalloc(&platDev->dev, sizeof(struct volatile_thermal), GFP_KERNEL);
  zoneParams = devm_kzalloc(&platDev->dev, sizeof(struct thermal_zone_params), GFP_KERNEL);
  if (!tempSensor || !zoneParams)
  {
    dev_err(&platDev->dev, "Can't allocate dev memory\n");
    goto error_out;
  }

  strcpy (zoneParams->governor_name, "step_wise");
  zoneParams->no_hwmon = true;
  zoneParams->num_tbps = 0;

#ifdef CONFIG_BCM94908
  tempSensor->tempSensor = &BIUCTRL->therm_throttle_temp;
  broadcomTempDrv_data.cpuCoolCompDev =
    thermal_cooling_device_register("passive", platDev, &broadcomCpuColdCompensationOps);
  if (IS_ERR(broadcomTempDrv_data.cpuCoolCompDev))
  {
    dev_err(&platDev->dev, "Can't create cooling device cpuCoolComp\n");
    goto error_out;
  }
#endif

  broadcomTempDrv_data.cpuCoolDev =
    thermal_cooling_device_register("passive", platDev, &broadcomCpuCoolingOps);
  if (IS_ERR(broadcomTempDrv_data.cpuCoolDev))
  {
    dev_err(&platDev->dev, "Can't create cooling device cpuCool\n");
    goto error_out;
  }

  if (numCpus == 2) 
  {
    broadcomTempDrv_data.cpuFreqDev =
      thermal_cooling_device_register("passive", platDev, &broadcomCpuFreqCoolingOps);

    if (IS_ERR(broadcomTempDrv_data.cpuFreqDev))
    {
      dev_err(&platDev->dev, "Can't create cooling device cpuFreq\n");
      goto error_out;
    }
  }  

  broadcomTempDrv_data.thermalZoneDev = thermal_zone_device_register(
    "broadcomThermalDrv", /* name */
    5,                    /* trips */
    0,                    /* mask */
    (void *)tempSensor,   /* device data */ 
    &thermalOps,
    zoneParams,           /* thermal zone params */
    1000,                 /* passive delay */
    1000                  /* polling delay */
    );

  if (IS_ERR(broadcomTempDrv_data.thermalZoneDev))
  {
    dev_err(&platDev->dev, "Failed to register thermal zone device %ld\n",
            PTR_ERR(broadcomTempDrv_data.thermalZoneDev));
    goto error_out;
  }

  platform_set_drvdata(platDev, &broadcomTempDrv_data);

#ifdef CONFIG_BCM94908
  // Configure cold compensation for all known boards
  Thermal_zone_bind_cooling_device(broadcomTempDrv_data.thermalZoneDev, 0 /*trip*/,
                                   broadcomTempDrv_data.cpuCoolCompDev,
                                   1, 1); // Go to high gain
  Thermal_zone_bind_cooling_device(broadcomTempDrv_data.thermalZoneDev, 1 /*trip*/,
                                   broadcomTempDrv_data.cpuCoolCompDev,
                                   2, 2); // Go to normal gain
#endif

  if (numCpus == 4 || numCpus == 3)
  {
    // configure heat compensation for 4908 (and other 4 or 3 CPU units)
    Thermal_zone_bind_cooling_device(broadcomTempDrv_data.thermalZoneDev, 2 /*trip*/,
                                     broadcomTempDrv_data.cpuCoolDev,
                                     1, 1); // do nothing state
    Thermal_zone_bind_cooling_device(broadcomTempDrv_data.thermalZoneDev, 3 /*trip*/,
                                     broadcomTempDrv_data.cpuCoolDev,
                                     2, 2); // Shut down 1 CPU
    Thermal_zone_bind_cooling_device(broadcomTempDrv_data.thermalZoneDev, 4 /*trip*/,
                                     broadcomTempDrv_data.cpuCoolDev, 
                                     3, 3); // Shut down all but 1 CPU
  }
  else
  {
    // configure heat compensation for 4906 (and other 2 CPU units)
    Thermal_zone_bind_cooling_device(broadcomTempDrv_data.thermalZoneDev, 2 /*trip*/,
                                     broadcomTempDrv_data.cpuCoolDev,
                                     1, 1); // do nothing
    Thermal_zone_bind_cooling_device(broadcomTempDrv_data.thermalZoneDev, 3 /*trip*/,
                                     broadcomTempDrv_data.cpuCoolDev,
                                     3, 3); // Shut down all but 1 CPU

    Thermal_zone_bind_cooling_device(broadcomTempDrv_data.thermalZoneDev, 2 /*trip*/,
                                     broadcomTempDrv_data.cpuFreqDev,
                                     1, 1); // do nothing
    Thermal_zone_bind_cooling_device(broadcomTempDrv_data.thermalZoneDev, 4 /*trip*/,
                                     broadcomTempDrv_data.cpuFreqDev,
                                     2, 2); // Turn down CPU freq on last CPU
  }

  return 0;

error_out:
  broadcomTempDrv_cleanup();
  return -1;
}

int broadcomTempDrv_remove (struct platform_device *platDev)
{
  broadcomTempDrv_cleanup();
  return 0;
}

static const struct of_device_id broadcom_thermal_id_table[] = {
	{ .compatible = "brcm,therm" },
	{}
};

struct platform_driver broadcomThermalDriver = {
  .probe = broadcomTempDrv_init,  
  .remove = broadcomTempDrv_remove,
  .driver = {
    .name = "broadcomThermalDrv",
    .of_match_table = broadcom_thermal_id_table  },
};

module_platform_driver(broadcomThermalDriver);

MODULE_DESCRIPTION("Broadcom thermal driver");
MODULE_LICENSE("GPL");
