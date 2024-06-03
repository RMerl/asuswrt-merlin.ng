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
#include <linux/reboot.h>
#include "pmc_core_api.h"
#include <linux/version.h>
#include "shared_utils.h"
#include <linux/bcm_version_compat.h>

typedef int tempmc_t;
#define Thermal_zone_bind_cooling_device(a, b, c, d, e)		\
	thermal_zone_bind_cooling_device(a, b, c, d, e, THERMAL_WEIGHT_DEFAULT)

#ifndef CONFIG_BCM_PON
static unsigned int __iomem
#if defined(CONFIG_BCM94908) && !defined(CONFIG_BCM_CPUIDLE_CLK_DIVIDER) && \
	!defined(CONFIG_CPU_FREQ)
	*bcr_cluster_clk_ctrl0,
	*bcr_cluster_clk_pattern0,
#endif
	*bcr_bac_cpu_therm_temp;

struct bcm_biucfg_reg_addr
{
	const char *name;
	struct resource *res;
	unsigned int __iomem **paddr;
};

#define REG_ADDR(reg)				\
	{ .name = #reg, .paddr = &bcr_##reg, }
static struct bcm_biucfg_reg_addr bra[] = {
#if defined(CONFIG_BCM94908) && !defined(CONFIG_BCM_CPUIDLE_CLK_DIVIDER) && \
	!defined(CONFIG_CPU_FREQ)
	REG_ADDR(cluster_clk_ctrl0),
	REG_ADDR(cluster_clk_pattern0),
#endif
	REG_ADDR(bac_cpu_therm_temp),
};

static void bcm_biucfg_unmap_reg_addr(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(bra) && *(bra[i].paddr); i++) {
		iounmap(*(bra[i].paddr));
		pr_info("iounmapped reg %s <0x%llx 0x%llx> from %px\n",
			bra[i].name, (unsigned long long)bra[i].res->start,
			(unsigned long long)resource_size(bra[i].res),
			*(bra[i].paddr));
		*(bra[i].paddr) = NULL;
	}
}

static int bcm_biucfg_map_reg_addr(struct platform_device *pdev)
{
	int i, ret;

	pr_info("pdev=%px name=%s num_resources=%d\n",
			pdev, pdev->name, pdev->num_resources);

	for (i = 0; i < pdev->num_resources; i++) {
		struct resource *r = &pdev->resource[i];

		pr_info("i=%d name=%s start=%llx end=%llx flags=0x%lx type=%lu\n",
			i, r->name, (unsigned long long)r->start,
			(unsigned long long)r->end, r->flags, resource_type(r));
	}

	for (i = 0; i < ARRAY_SIZE(bra); i++) {
		bra[i].res = platform_get_resource_byname(pdev,
				IORESOURCE_MEM, bra[i].name);
		if (!bra[i].res) {
			pr_err("Error: failed to get reg %s\n", bra[i].name);
			ret = -ENOENT;
			goto err_out;
		}

		*(bra[i].paddr) = ioremap(bra[i].res->start,
				resource_size(bra[i].res));
		if (!*(bra[i].paddr)) {
			pr_err("Error: failed to ioremap reg %s\n", bra[i].name);
			ret = -ENXIO;
			goto err_out;
		}

		pr_info("ioremapped reg %s <0x%llx 0x%llx> to %px\n",
			bra[i].name, (unsigned long long)bra[i].res->start,
			(unsigned long long)resource_size(bra[i].res),
			*(bra[i].paddr));
	}

	return 0;

err_out:
	bcm_biucfg_unmap_reg_addr();
	return ret;
}
#endif // #ifndef CONFIG_BCM_PON

void bcm_cpufreq_set_freq_max(unsigned maxdiv);

#if !defined(CONFIG_BCM_CPUIDLE_CLK_DIVIDER) && !defined(CONFIG_CPU_FREQ)
void bcm_cpufreq_set_freq_max(unsigned maxdiv)
{
#ifdef CONFIG_BCM94908
	*bcr_cluster_clk_pattern0 = (maxdiv == 1 ? 0xffffffff : 0x55555555);
	*bcr_cluster_clk_ctrl0 = 1 << 4;	// enable user clock-patterns
#endif
}
#endif

// TEMPERATURE LIMITS
#define BROADCOM_THERMAL_NUM_TRIP_TEMPERATURES                         6

#define BROADCOM_THERMAL_LOW_TEMPERATURE_COMPENSATION_MILLICELSIUS     15000
#define BROADCOM_THERMAL_LOW_TEMPERATURE_HYSTERESIS_MILLICELSIUS       10000

#if defined(CONFIG_BCM96813) || defined(CONFIG_BCM963146) || \
	defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178)
#define IS_DSL_SILICON	1
#define BROADCOM_THERMAL_HIGH_TEMPERATURE_COMPENSATION_1_MILLICELSIUS  120000
#define BROADCOM_THERMAL_HIGH_TEMPERATURE_COMPENSATION_2_MILLICELSIUS  125000
#endif

#if defined(CONFIG_BCM947622) || defined(CONFIG_BCM96756) || defined(CONFIG_BCM94912)
#define IS_WIFI_SILICON	1
#define BROADCOM_THERMAL_HIGH_TEMPERATURE_COMPENSATION_1_MILLICELSIUS  110000
#define BROADCOM_THERMAL_HIGH_TEMPERATURE_COMPENSATION_2_MILLICELSIUS  115000
#endif

#ifdef CONFIG_BCM94908
// QUAD CPU is meant to apply to 4908
#define BROADCOM_THERMAL_HIGH_TEMPERATURE_QUAD_CPU_COMPENSATION_1_MILLICELSIUS  115000
#define BROADCOM_THERMAL_HIGH_TEMPERATURE_QUAD_CPU_COMPENSATION_2_MILLICELSIUS  125000

// DUAL CPU is meant to apply to the 4906
#define BROADCOM_THERMAL_HIGH_TEMPERATURE_DUAL_CPU_COMPENSATION_1_MILLICELSIUS  100000
#define BROADCOM_THERMAL_HIGH_TEMPERATURE_DUAL_CPU_COMPENSATION_2_MILLICELSIUS  110000
#endif

#define BROADCOM_THERMAL_HIGH_TEMPERATURE_HYSTERESIS_MILLICELSIUS      2000

int reboot_temperature = -1; /* Will be read from device tree */

/*-------------------------*
 *   CPU Cooling Devices   *
 *-------------------------*/

#ifdef CONFIG_BCM94908
/* CPU PROCESSORS COLD COMPENSATION DEVICE */
static int broadcom_cpu_cold_compensation_lastAnnouncement = 2;

int broadcom_cpu_cold_compensation_get_max_state(struct thermal_cooling_device *dev, unsigned long *states)
{
  *states = 2;
  return 0;
}

int broadcom_cpu_cold_compensation_get_cur_state(struct thermal_cooling_device *dev, unsigned long *currentState)
{
  *currentState = 0;
  return 0;
}

int broadcom_cpu_cold_compensation_set_cur_state(struct thermal_cooling_device *dev, unsigned long state)
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

int broadcom_cpu_cooling_get_max_state(struct thermal_cooling_device *dev, unsigned long *states)
{
  *states = 3;
  return 0;
}

int broadcom_cpu_cooling_get_cur_state(struct thermal_cooling_device *dev, unsigned long *currentState)
{
  *currentState = 0;
  return 0;
}

static struct cpumask brcm_cpu_absent_mask;

// mark cpu as not-present to prevent other code from onlining;
// (duplicates [unexported] set_cpu_present for module);
// also update sysfs cpu offline status
static void brcm_cpu_absent(unsigned int cpu)
{
  cpumask_clear_cpu(cpu, (struct cpumask *)cpu_present_mask);
  get_cpu_device(cpu)->offline = true;
  cpumask_set_cpu(cpu, &brcm_cpu_absent_mask);
}

// mark cpu as present to allow other code from onlining;
// (duplicates [unexported] set_cpu_present for module);
// also update sysfs cpu offline status
static void brcm_cpu_present(unsigned int cpu)
{
  cpumask_set_cpu(cpu, (struct cpumask *)cpu_present_mask);
  get_cpu_device(cpu)->offline = false;
  cpumask_clear_cpu(cpu, &brcm_cpu_absent_mask);
}

int broadcom_cpu_cooling_set_cur_state(struct thermal_cooling_device *dev, unsigned long state)
{
  int cpuIndex, rc;

  switch (state)
  {
    case 0:
#if defined (CONFIG_BCM947622) || defined (CONFIG_BCM963178) || defined (CONFIG_BCM96756)
      for (cpuIndex = 0; cpuIndex < num_possible_cpus() - 1; cpuIndex++) {
#else
      for (cpuIndex = 1; cpuIndex < num_possible_cpus(); cpuIndex++) {
#endif
          if (!cpu_online(cpuIndex)) {
            dev_crit(&dev->device,"take CPU#%d online\n", cpuIndex);
	  brcm_cpu_present(cpuIndex); // mark present before cpu_up
	  if ((rc = add_cpu(cpuIndex)) != 0)
	    brcm_cpu_absent(cpuIndex);
        }
      }
      break;
    case 1:
      break;
    case 2:
#if defined (CONFIG_BCM947622) || defined (CONFIG_BCM963178) || defined (CONFIG_BCM96756)
      // Take the second to last possible CPU offline
      cpuIndex = num_possible_cpus() - 2;
#else
      // Take the last possible CPU offline
      cpuIndex = num_possible_cpus() - 1;
#endif
      if (cpuIndex < 1)
        break;
      if (cpu_online(cpuIndex)) {
        dev_crit(&dev->device,"take CPU#%d offline\n", cpuIndex);
#if defined(CONFIG_HOTPLUG_CPU)
        if ((rc = remove_cpu(cpuIndex)) == 0)
          brcm_cpu_absent(cpuIndex);
#endif
      }
      break;
    case 3:
#if defined (CONFIG_BCM947622) || defined (CONFIG_BCM963178) || defined (CONFIG_BCM96756)
      // Keep the last possible CPU online
      for (cpuIndex = 0; cpuIndex < num_possible_cpus() - 1; cpuIndex++) {
#else
      for (cpuIndex = 1; cpuIndex < num_possible_cpus(); cpuIndex++) {
#endif
          if (cpu_online(cpuIndex)) {
            dev_crit(&dev->device,"take CPU#%d offline\n", cpuIndex);
#if defined(CONFIG_HOTPLUG_CPU)
	  if ((rc = remove_cpu(cpuIndex)) == 0)
	    brcm_cpu_absent(cpuIndex);
#endif
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

int broadcom_cpufreq_cooling_get_max_state(struct thermal_cooling_device *dev, unsigned long *states)
{
  *states = 2;
  return 0;
}

int broadcom_cpufreq_cooling_get_cur_state(struct thermal_cooling_device *dev, unsigned long *currentState)
{
  *currentState = 0;
  return 0;
}

int broadcom_cpufreq_cooling_set_cur_state(struct thermal_cooling_device *dev, unsigned long state)
{
  switch (state)
  {
    case 0:
      if (broadcom_cpufreq_cooling_lastAnnouncement != 0) {
        dev_crit(&dev->device,"Go to max frequency\n");
        bcm_cpufreq_set_freq_max (1);
        broadcom_cpufreq_cooling_lastAnnouncement = 0;
      }
    case 1:
      break;
    case 2:
      if (broadcom_cpufreq_cooling_lastAnnouncement != 2) {
        dev_crit(&dev->device,"Go to low frequency\n");
        bcm_cpufreq_set_freq_max (2);
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


/* REBOOT COOLING DEVICE */

static int broadcom_reboot_cooling_lastAnnouncement = 0;

int broadcom_reboot_cooling_get_max_state(struct thermal_cooling_device *dev, unsigned long *states)
{
  *states = 2;
  return 0;
}

int broadcom_reboot_cooling_get_cur_state(struct thermal_cooling_device *dev, unsigned long *currentState)
{
  *currentState = 0;
  return 0;
}

int broadcom_reboot_cooling_set_cur_state(struct thermal_cooling_device *dev, unsigned long state)
{
  switch (state)
  {
    case 0:
      if (broadcom_reboot_cooling_lastAnnouncement != 0) {
        broadcom_reboot_cooling_lastAnnouncement = 0;
      }
    case 1:
      break;
    case 2:
      if (broadcom_reboot_cooling_lastAnnouncement != 2) {
        broadcom_reboot_cooling_lastAnnouncement = 2;
        dev_crit(&dev->device,"Temperature above %d millicelsius, rebooting...\n", reboot_temperature);
        orderly_reboot();
      }
      break;
  }
  return 0;
}

struct thermal_cooling_device_ops broadcomRebootCoolingOps =
{
  .get_max_state = broadcom_reboot_cooling_get_max_state,
  .get_cur_state = broadcom_reboot_cooling_get_cur_state,
  .set_cur_state = broadcom_reboot_cooling_set_cur_state,
};

/*------------------------*
 * End CPU Cooling Device *
 *------------------------*/

static int tripTemperatures[BROADCOM_THERMAL_NUM_TRIP_TEMPERATURES];

// #define DEBUG_TEMPERATURE
#ifdef DEBUG_TEMPERATURE
static int dbg_temperature = -999;
module_param(dbg_temperature, int, 0644);
#endif // #ifdef DEBUG_TEMPERATURE

static int get_temperature(struct thermal_zone_device *thermDev, tempmc_t *tempMillicelsius)
{
#ifdef DEBUG_TEMPERATURE
  *tempMillicelsius = (dbg_temperature > -999 ? dbg_temperature : 40) * 1000;
#elif defined (CONFIG_BCM963158) || defined (CONFIG_BCM947622) || defined (CONFIG_BCM963178) || defined (CONFIG_BCM96756) || \
  defined(CONFIG_BCM963146) || defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813) || defined(CONFIG_BCM_PON)
  int ret, adc = -1;

  ret = GetPVTKH2(kTEMPERATURE, 0, &adc);
  if (ret) {
    dev_err(&thermDev->device, "Failed to get RAIL 0 temperature, ret=%d\n", ret);
    return ret;
  }
  *tempMillicelsius = pmc_convert_pvtmon(kTEMPERATURE, adc);

#ifndef CONFIG_BCM_PON // Not all PON have a sensor for ARM temperature
  ret = *bcr_bac_cpu_therm_temp;
  if (!(ret & (1<<31))) {
    dev_err(&thermDev->device, "Failed to get CPU temperature, ret=%d\n", ret);
    return -1;
  }
  ret = pmc_convert_pvtmon(kTEMPERATURE, ret & 0x3ff);
  if (ret > (int)*tempMillicelsius)
    *tempMillicelsius = ret;
#endif
#else
  int regVal = *bcr_bac_cpu_therm_temp;
  regVal &= 0x000003ff; 
  *tempMillicelsius = (4133500 - regVal * 4906) / 10; // for 4908 only
#endif
  return 0;  
}

#define TEMP_SAMPLES    5
static tempmc_t temp_samples[TEMP_SAMPLES] = {};

static int get_avg_temperature(struct thermal_zone_device *thermDev, tempmc_t *tempMillicelsius)
{
    static int sample_index = 0;
    static int samples = 0;
    tempmc_t tempTotal = 0, tempCur;
    int i, ret;

    ret = get_temperature(thermDev, &tempCur);
    if (ret)
        return -1;

    if (samples < 5)
        samples++;

    temp_samples[sample_index] = tempCur;

    for (i = 0; i < samples; i++)
        tempTotal += temp_samples[i];

    *tempMillicelsius = tempTotal / samples;

    sample_index++;

    if (sample_index == TEMP_SAMPLES)
        sample_index = 0;

    return 0;
}

int broadcomTempDrv_get_temp(struct thermal_zone_device *thermDev, tempmc_t *tempMillicelsius)
{
  tempmc_t tempMC;
  int ret;

  ret = get_avg_temperature(thermDev, &tempMC);
  if (ret)
  {
    dev_err(&thermDev->device, "Failed to get temperature, ret=%d\n", ret);
    return -1;
  }

  dev_dbg(&thermDev->device, "Temperature in Celsius              %d.%03d\n",
          tempMC / 1000, (tempMC < 0 ? -tempMC : tempMC) % 1000);

  *tempMillicelsius = tempMC;

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

int broadcomTempDrv_set_trip_temp(struct thermal_zone_device *thermalZoneDev, int trip, tempmc_t tempMillicelsius) 
{
  if ((trip >= 0) && (trip < BROADCOM_THERMAL_NUM_TRIP_TEMPERATURES))
  {
    tripTemperatures[trip] = tempMillicelsius;
  }
  return 0;
}


int broadcomTempDrv_get_trip_temp(struct thermal_zone_device *thermalZoneDev, int trip, tempmc_t *tempMillicelsius)
{
   if ((trip >= 0) && (trip < BROADCOM_THERMAL_NUM_TRIP_TEMPERATURES))
   {
     *tempMillicelsius = tripTemperatures[trip];
     return 0;
   }

   return -1;
}

int broadcomTempDrv_get_trip_hyst(struct thermal_zone_device *thermalZoneDev, int trip, tempmc_t *tempMillicelsius)
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

  
struct thermal_zone_device_ops thermalOps =
{
  .get_temp = broadcomTempDrv_get_temp,
  .set_trip_temp = broadcomTempDrv_set_trip_temp,
  .get_trip_type = broadcomTempDrv_get_trip_type,
  .get_trip_temp = broadcomTempDrv_get_trip_temp,
  .get_trip_hyst = broadcomTempDrv_get_trip_hyst,
};

static struct {
#ifdef CONFIG_BCM94908
  struct thermal_cooling_device *cpuCoolCompDev;
#endif
  struct thermal_cooling_device *cpuCoolDev;
  struct thermal_cooling_device *cpuFreqDev;
  struct thermal_cooling_device *rebootDev;
  struct thermal_zone_device *thermalZoneDev;
} broadcomTempDrv_data;

static void broadcomTempDrv_cleanup(void)
{
  int cpu;
#ifdef CONFIG_BCM94908
  if (!IS_ERR_OR_NULL(broadcomTempDrv_data.cpuCoolCompDev))
    thermal_cooling_device_unregister(broadcomTempDrv_data.cpuCoolCompDev);
#endif
  if (!IS_ERR_OR_NULL(broadcomTempDrv_data.cpuCoolDev))
    thermal_cooling_device_unregister(broadcomTempDrv_data.cpuCoolDev);
  if (!IS_ERR_OR_NULL(broadcomTempDrv_data.cpuFreqDev))
    thermal_cooling_device_unregister(broadcomTempDrv_data.cpuFreqDev);
  if (!IS_ERR_OR_NULL(broadcomTempDrv_data.rebootDev))
    thermal_cooling_device_unregister(broadcomTempDrv_data.rebootDev);
  if (!IS_ERR_OR_NULL(broadcomTempDrv_data.thermalZoneDev))
    thermal_zone_device_unregister(broadcomTempDrv_data.thermalZoneDev);

  memset(&broadcomTempDrv_data, 0, sizeof(broadcomTempDrv_data));

  for_each_cpu(cpu, &brcm_cpu_absent_mask) {
    if (cpu_present(cpu))
      continue;
    cpumask_set_cpu(cpu, (struct cpumask *)cpu_present_mask);
    get_cpu_device(cpu)->offline = false;
    add_cpu(cpu);
  }
}

// the activate and register function
int broadcomTempDrv_init(struct platform_device *platDev)
{
  struct device *dev = &platDev->dev;
  struct device_node *np = dev->of_node;
  /* structs allocated by devm_ are automatically freed when device exits */
  struct thermal_zone_params *zoneParams = NULL;
  int numCpus;
  unsigned int chipId = UtilGetChipId();

#ifndef CONFIG_BCM_PON
  if (bcm_biucfg_map_reg_addr(platDev))
    return -1;
#endif

  numCpus = num_possible_cpus();
  dev_crit(&platDev->dev,
    "init chipId 0x%x (CPU cnt present %d online %d possible %d active %d)\n",
    chipId, num_present_cpus(), num_online_cpus(), num_possible_cpus(),
    num_active_cpus());

  if (numCpus < 2)
  {
    dev_err(&platDev->dev, "Can't handle %d CPUs\n", numCpus);
    goto error_out;
  }

#if defined(IS_DSL_SILICON) || defined(IS_WIFI_SILICON)
  {
    int htc1 = BROADCOM_THERMAL_HIGH_TEMPERATURE_COMPENSATION_1_MILLICELSIUS;
    int htc2 = BROADCOM_THERMAL_HIGH_TEMPERATURE_COMPENSATION_2_MILLICELSIUS;

    // 6750 is wifi but defined as CONFIG_BCM963178
    // 4916 is wifi but defined as CONFIG_BCM96813
    if (chipId == 0x6750 || chipId == 0x4916) {
      htc1 -= 10000;	// 10 celsius degree
      htc2 -= 10000;	// 10 celsius degree
    }

    broadcomTempDrv_set_trip_temp(NULL, 2, htc1 -
		  BROADCOM_THERMAL_HIGH_TEMPERATURE_HYSTERESIS_MILLICELSIUS);
    broadcomTempDrv_set_trip_temp(NULL, 3, htc1);
    broadcomTempDrv_set_trip_temp(NULL, 4, htc2);
  }
#elif defined(CONFIG_BCM94908)
  broadcomTempDrv_set_trip_temp(NULL, 0,
		  BROADCOM_THERMAL_LOW_TEMPERATURE_COMPENSATION_MILLICELSIUS);
  broadcomTempDrv_set_trip_temp(NULL, 1,
		  BROADCOM_THERMAL_LOW_TEMPERATURE_HYSTERESIS_MILLICELSIUS +
		  BROADCOM_THERMAL_LOW_TEMPERATURE_COMPENSATION_MILLICELSIUS);

  if (numCpus == 2)
  {
    broadcomTempDrv_set_trip_temp(NULL, 2, BROADCOM_THERMAL_HIGH_TEMPERATURE_DUAL_CPU_COMPENSATION_1_MILLICELSIUS - BROADCOM_THERMAL_HIGH_TEMPERATURE_HYSTERESIS_MILLICELSIUS);
    broadcomTempDrv_set_trip_temp(NULL, 3, BROADCOM_THERMAL_HIGH_TEMPERATURE_DUAL_CPU_COMPENSATION_1_MILLICELSIUS);
    broadcomTempDrv_set_trip_temp(NULL, 4, BROADCOM_THERMAL_HIGH_TEMPERATURE_DUAL_CPU_COMPENSATION_2_MILLICELSIUS);
  }
  else
  {
    broadcomTempDrv_set_trip_temp(NULL, 2, BROADCOM_THERMAL_HIGH_TEMPERATURE_QUAD_CPU_COMPENSATION_1_MILLICELSIUS - BROADCOM_THERMAL_HIGH_TEMPERATURE_HYSTERESIS_MILLICELSIUS);
    broadcomTempDrv_set_trip_temp(NULL, 3, BROADCOM_THERMAL_HIGH_TEMPERATURE_QUAD_CPU_COMPENSATION_1_MILLICELSIUS);
    broadcomTempDrv_set_trip_temp(NULL, 4, BROADCOM_THERMAL_HIGH_TEMPERATURE_QUAD_CPU_COMPENSATION_2_MILLICELSIUS);
  }
#endif

  if (of_property_read_u32(np, "reboot-temperature", &reboot_temperature))
    reboot_temperature = -1;

  if (reboot_temperature > 0)
    broadcomTempDrv_set_trip_temp(NULL, 5, reboot_temperature);

  zoneParams = devm_kzalloc(&platDev->dev, sizeof(struct thermal_zone_params), GFP_KERNEL);
  if (!zoneParams)
  {
    dev_err(&platDev->dev, "Can't allocate dev memory\n");
    goto error_out;
  }

  strcpy (zoneParams->governor_name, "step_wise");
  zoneParams->no_hwmon = true;
  zoneParams->num_tbps = 0;

#ifdef CONFIG_BCM94908
  broadcomTempDrv_data.cpuCoolCompDev =
    thermal_cooling_device_register("passive", platDev, &broadcomCpuColdCompensationOps);
  if (IS_ERR(broadcomTempDrv_data.cpuCoolCompDev))
  {
    dev_err(&platDev->dev, "Can't create cooling device cpuCoolComp\n");
    goto error_out;
  }
#endif

#ifndef CONFIG_BCM_PON
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
#endif

  if (reboot_temperature > 0)
  {
    broadcomTempDrv_data.rebootDev =
      thermal_cooling_device_register("reboot", platDev, &broadcomRebootCoolingOps);
    if (IS_ERR(broadcomTempDrv_data.rebootDev))
    {
      dev_err(&platDev->dev, "Can't create cooling device Reboot\n");
      goto error_out;
    }
  }

  broadcomTempDrv_data.thermalZoneDev = thermal_zone_device_register(
    "broadcomThermalDrv",                   /* name */
    BROADCOM_THERMAL_NUM_TRIP_TEMPERATURES, /* trips */
    0,                                      /* mask */
    NULL,                                   /* device data */ 
    &thermalOps,
    zoneParams,                             /* thermal zone params */
    1000,                                   /* passive delay */
    1000                                    /* polling delay */
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

#ifndef CONFIG_BCM_PON
  if (numCpus > 2)
  {
    // configure heat compensation for 4908 (and other 3 or more CPU units)
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
#endif

  if (reboot_temperature > 0)
  {
    // Configure reboot for all known boards
    Thermal_zone_bind_cooling_device(broadcomTempDrv_data.thermalZoneDev, 5 /*trip*/,
      broadcomTempDrv_data.rebootDev,
      2, 2); // soft reboot

    dev_crit(&platDev->dev, "Registered a cooling device to reboot when temperature exceeds %d millicelsius\n", reboot_temperature);
  }

#if (LINUX_VERSION_CODE > KERNEL_VERSION(5,15,0))
  thermal_zone_device_enable(broadcomTempDrv_data.thermalZoneDev);
#endif

  return 0;

error_out:
  broadcomTempDrv_cleanup();
  return -1;
}

int broadcomTempDrv_remove(struct platform_device *platDev)
{
  broadcomTempDrv_cleanup();
#ifndef CONFIG_BCM_PON
  bcm_biucfg_unmap_reg_addr();
#endif // #ifndef CONFIG_BCM_PON
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
