// SPDX-License-Identifier: GPL-2.0
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/brcmstb/brcmstb.h>
#include <linux/brcmstb/avs_dvfs.h>

static const char *pmic_id_to_str(u8 id)
{
	switch (id) {
	case 0:
		return "unused";
	case 1:
		return "15602";
	case 2:
		return "59400";
	default:
		return "unknown";
	}
}

int __init test_dvfs_api_init(void)
{
	struct brcmstb_avs_pmic_info info = { };
	u32 die_temp, ext_therm, overall_power;
	unsigned int i, total_power = 0, w, mw;
	u16 nom_voltage, volt, curr;
	int ret;

	ret = brcmstb_stb_avs_get_pmic_info(&info);
	if (ret) {
		pr_err("%s: failed to get PMIC info: %d\n", __func__, ret);
		return ret;
	}

	pr_info("PMIC information summary\n");
	pr_info(" * PMIC(s): %d\n", info.num_pmic_devices);
	pr_info(" * Regulator(s): %d\n", info.num_regulators);
	pr_info(" * GPIO(s): %d\n", info.num_gpios);
	for (i = 0; i < info.num_pmic_devices; i++) {
		if (!info.ext_infos[i].chip_id)
			continue;

		pr_info(" PMIC #%d\n", i);
		pr_info("    * I2C addr: %d, ID: %d (%s), caps: 0x%02x\n",
			info.ext_infos[i].i2c_addr,
			info.ext_infos[i].chip_id,
			pmic_id_to_str(info.ext_infos[i].chip_id),
			info.ext_infos[i].caps);
		pr_info("    * Version: %c%d\n",
			'A' + (info.ext_infos2[i].version >> 4),
			info.ext_infos2[i].version & 0xf);
		pr_info("    * Variant: %d\n", info.ext_infos2[i].variant);
		pr_info("    * GID: %d\n", info.ext_infos2[i].gid);
		pr_info("    * ESD: 0x%02x\n", info.ext_infos2[i].esd);

		ret = brcmstb_stb_avs_get_pmic_status(i, &die_temp,
						      &ext_therm, &overall_power);
		if (ret) {
			pr_err("%s: failed to get PMIC status: %d\n", __func__, ret);
			continue;
		}

		pr_info("    * die temperature: %d °C\n", die_temp / 1000);
		pr_info("    * external thermistor: %d °C\n", ext_therm / 1000);
		pr_info("    * overall power: %d mW\n", overall_power);
	}

	for (i = 0; i < info.num_regulators; i++) {
		ret = brcmstb_avs_get_pmic_reg_info(i, &nom_voltage);
		if (ret) {
			pr_err("%s: failed to get regulator info: %d\n",
				__func__, ret);
			continue;
		}

		pr_info("Regulator #%d\n", i);
		pr_info(" * nomimal voltage: %d mV\n", nom_voltage);

		ret = brcmstb_avs_get_pmic_reg_status(i, &volt, &curr);
		if (ret) {
			pr_err("%s: failed to get regulator status: %d\n",
				__func__, ret);
			continue;
		}

		pr_info(" * voltage: %d mV, current: %d mA\n",
			volt, curr);
		total_power += (volt * curr);
	}

	w = total_power;
	mw = w / 1000000;

	pr_info(" Total power calculated: %d.%04d W(dc) (%d uW)\n",
		w, mw, total_power);

	return ret;
}
module_init(test_dvfs_api_init);

void __exit test_dvfs_api_exit(void)
{
}
module_exit(test_dvfs_api_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Broadcom STB DVFS API test module");
