/******************************************************************************

   Copyright 2023-2024 MaxLinear, Inc.

   For licensing information, see the file 'LICENSE' in the root folder of
   this software module.

******************************************************************************/

#include "host_adapt.h"
#include "host_api_impl.h"

int sys_misc_fw_update(const GSW_Device_t *dev)
{
	return gsw_api_wrap(dev,
			    SYS_MISC_FW_UPDATE,
			    NULL,
			    0,
			    0,
			    0);
}

int sys_misc_fw_version(const GSW_Device_t *dev, struct sys_fw_image_version *sys_img_ver)
{
	return gsw_api_wrap(dev,
			    SYS_MISC_FW_VERSION,
			    sys_img_ver,
			    sizeof(*sys_img_ver),
			    0,
			    sizeof(*sys_img_ver));
}

int sys_misc_pvt_temp(const GSW_Device_t *dev, struct sys_sensor_value *sys_temp_val)
{
	return gsw_api_wrap(dev,
			    SYS_MISC_PVT_TEMP,
			    sys_temp_val,
			    sizeof(*sys_temp_val),
			    0,
			    sizeof(*sys_temp_val));
}

int sys_misc_pvt_voltage(const GSW_Device_t *dev, struct sys_sensor_value *sys_voltage)
{
	return gsw_api_wrap(dev,
			    SYS_MISC_PVT_VOLTAGE,
			    sys_voltage,
			    sizeof(*sys_voltage),
			    0,
			    sizeof(*sys_voltage));
}

int sys_misc_delay(const GSW_Device_t *dev, struct sys_delay *pdelay)
{
	return gsw_api_wrap(dev,
			    SYS_MISC_DELAY,
			    pdelay,
			    sizeof(*pdelay),
			    0,
			    0);
}

int sys_misc_gpio_configure(const GSW_Device_t *dev, struct sys_gpio_config *sys_gpio_conf)
{
	return gsw_api_wrap(dev,
			    SYS_MISC_GPIO_CONFIGURE,
			    sys_gpio_conf,
			    sizeof(*sys_gpio_conf),
			    0,
			    0);
}

int sys_misc_reboot(const GSW_Device_t *dev)
{
	return gsw_api_wrap(dev,
			    SYS_MISC_REBOOT,
			    NULL,
			    0,
			    0,
			    0);
}

int sys_misc_reg_rd(const GSW_Device_t *dev, struct sys_reg_rw *sys_reg)
{
	return gsw_api_wrap(dev,
			    SYS_MISC_REG_RD,
			    sys_reg,
			    sizeof(*sys_reg),
			    0,
			    sizeof(*sys_reg));
}

int sys_misc_reg_wr(const GSW_Device_t *dev, struct sys_reg_rw *sys_reg)
{
	return gsw_api_wrap(dev,
			    SYS_MISC_REG_WR,
			    sys_reg,
			    sizeof(*sys_reg),
			    0,
			    0);
}

int sys_misc_reg_mod(const GSW_Device_t *dev, struct sys_reg_mod *sys_reg)
{
	return gsw_api_wrap(dev,
			    SYS_MISC_REG_MOD,
			    sys_reg,
			    sizeof(*sys_reg),
			    0,
			    0);
}

int sys_misc_cml_clk_get(const GSW_Device_t *dev, struct sys_cml_clk *clk)
{
	return gsw_api_wrap(dev,
			    SYS_MISC_CML_CLK_GET,
			    clk,
			    sizeof(*clk),
			    0,
			    sizeof(*clk));
}

int sys_misc_cml_clk_set(const GSW_Device_t *dev, struct sys_cml_clk *clk)
{
	return gsw_api_wrap(dev,
			    SYS_MISC_CML_CLK_SET,
			    clk,
			    sizeof(*clk),
			    0,
			    0);
}

int sys_misc_sfp_get(const GSW_Device_t *dev, struct sys_sfp_cfg *cfg)
{
	return gsw_api_wrap(dev,
			    SYS_MISC_SFP_GET,
			    cfg,
			    sizeof(*cfg),
			    0,
			    sizeof(*cfg));
}

int sys_misc_sfp_set(const GSW_Device_t *dev, struct sys_sfp_cfg *cfg)
{
	return gsw_api_wrap(dev,
			    SYS_MISC_SFP_SET,
			    cfg,
			    sizeof(*cfg),
			    0,
			    sizeof(*cfg));
}

int loop_detect_passive_start(const GSW_Device_t *dev,
			      struct loop_detect_passive_cfg *cfg)
{
	return gsw_api_wrap(dev,
			    SYS_MISC_LOOP_PASSIVE_START,
			    cfg,
			    sizeof(*cfg),
			    0,
			    0);
}

int loop_detect_passive_stop(const GSW_Device_t *dev)
{
	return gsw_api_wrap(dev,
			    SYS_MISC_LOOP_PASSIVE_STOP,
			    NULL,
			    0,
			    0,
			    0);
}

int loop_detect_active_start(const GSW_Device_t *dev,
			     struct loop_detect_active_cfg *cfg)
{
	return gsw_api_wrap(dev,
			    SYS_MISC_LOOP_ACTIVE_START,
			    cfg,
			    sizeof(*cfg),
			    0,
			    0);
}

int loop_detect_active_stop(const GSW_Device_t *dev)
{
	return gsw_api_wrap(dev,
			    SYS_MISC_LOOP_ACTIVE_STOP,
			    NULL,
			    0,
			    0,
			    0);
}

int loop_prevention_start(const GSW_Device_t *dev,
			  struct loop_prevention_cfg *cfg)
{
	return gsw_api_wrap(dev,
			    SYS_MISC_LOOP_PREVENTION_START,
			    cfg,
			    sizeof(*cfg),
			    0,
			    0);
}

int loop_prevention_stop(const GSW_Device_t *dev)
{
	return gsw_api_wrap(dev,
			    SYS_MISC_LOOP_PREVENTION_STOP,
			    NULL,
			    0,
			    0,
			    0);
}


int mxl_led_sys_cfg(const GSW_Device_t *dev, struct mxl_led_sys_cfg *cfg)
{
	return gsw_api_wrap(dev,
			    SYS_MISC_SYS_LED_CFG,
			    cfg,
			    sizeof(*cfg),
			    0,
			    0);
}
