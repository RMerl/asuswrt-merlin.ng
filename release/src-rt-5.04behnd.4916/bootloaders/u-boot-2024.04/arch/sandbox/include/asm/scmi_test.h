/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020, Linaro Limited
 */

#ifndef __SANDBOX_SCMI_TEST_H
#define __SANDBOX_SCMI_TEST_H

#include <power-domain.h>

struct udevice;
struct sandbox_scmi_agent;
struct sandbox_scmi_service;

/**
 * struct sandbox_scmi_pwd
 * @id:		Identifier of the power domain used in the SCMI protocol
 * @pstate::	Power state of the domain
 */
struct sandbox_scmi_pwd {
	uint id;
	u32 pstate;
};

/**
 * struct sandbox_scmi_clk - Simulated clock exposed by SCMI
 * @id:		Identifier of the clock used in the SCMI protocol
 * @enabled:	Clock state: true if enabled, false if disabled
 * @rate:	Clock rate in Hertz
 */
struct sandbox_scmi_clk {
	bool enabled;
	ulong rate;
};

/**
 * struct sandbox_scmi_reset - Simulated reset controller exposed by SCMI
 * @id:		Identifier of the reset controller used in the SCMI protocol
 * @asserted:	Reset control state: true if asserted, false if desasserted
 */
struct sandbox_scmi_reset {
	uint id;
	bool asserted;
};

/**
 * struct sandbox_scmi_voltd - Simulated voltage regulator exposed by SCMI
 * @id:		Identifier of the voltage domain used in the SCMI protocol
 * @enabled:	Regulator state: true if on, false if off
 * @voltage_uv:	Regulator current voltage in microvoltd (uV)
 */
struct sandbox_scmi_voltd {
	uint id;
	bool enabled;
	int voltage_uv;
};

/**
 * struct sandbox_scmi_agent - Simulated SCMI service seen by SCMI agent
 * @pwdom_version: Implemented power domain protocol version
 * @pwdom_count:   Simulated power domains array size
 * @clk:	Simulated clocks
 * @clk_count:	Simulated clocks array size
 * @reset:	Simulated reset domains
 * @reset_count: Simulated reset domains array size
 * @voltd:	 Simulated voltage domains (regulators)
 * @voltd_count: Simulated voltage domains array size
 */
struct sandbox_scmi_agent {
	int pwdom_version;
	struct sandbox_scmi_pwd *pwdom;
	size_t pwdom_count;
	struct sandbox_scmi_clk *clk;
	size_t clk_count;
	struct sandbox_scmi_reset *reset;
	size_t reset_count;
	struct sandbox_scmi_voltd *voltd;
	size_t voltd_count;
};

/**
 * struct sandbox_scmi_service - Reference to simutaed SCMI agents/services
 * @agent:		Pointer to SCMI sandbox agent or NULL if not probed
 */
struct sandbox_scmi_service {
	struct sandbox_scmi_agent *agent;
};

/**
 * struct sandbox_scmi_devices - Reference to devices probed through SCMI
 * @pwdom:		Array of power domains
 * @pwdom_count:	Number of power domains probed
 * @clk:		Array the clock devices
 * @clk_count:		Number of clock devices probed
 * @reset:		Array the reset controller devices
 * @reset_count:	Number of reset controller devices probed
 * @regul:		Array regulator devices
 * @regul_count:	Number of regulator devices probed
 */
struct sandbox_scmi_devices {
	struct power_domain *pwdom;
	size_t pwdom_count;
	struct clk *clk;
	size_t clk_count;
	struct reset_ctl *reset;
	size_t reset_count;
	struct udevice **regul;
	size_t regul_count;
};

#ifdef CONFIG_SCMI_FIRMWARE
/**
 * sandbox_scmi_channel_id - Get the channel id
 * @dev:	Reference to the SCMI protocol device
 *
 * Return:	Channel id
 */
unsigned int sandbox_scmi_channel_id(struct udevice *dev);

/**
 * sandbox_scmi_service_ctx - Get the simulated SCMI services context
 * sandbox_scmi_agent_ctx - Get the simulated SCMI agent context
 * @dev:	Reference to the test agent
 * @return:	Reference to backend simulated resources state
 */
struct sandbox_scmi_agent *sandbox_scmi_agent_ctx(struct udevice *dev);

/**
 * sandbox_scmi_devices_ctx - Get references to devices accessed through SCMI
 * @dev:	Reference to the test device used get test resources
 * @return:	Reference to the devices probed by the SCMI test
 */
struct sandbox_scmi_devices *sandbox_scmi_devices_ctx(struct udevice *dev);
#else
inline unsigned int sandbox_scmi_channel_id(struct udevice *dev);
{
	return 0;
}

static struct sandbox_scmi_agent *sandbox_scmi_agent_ctx(struct udevice *dev)
{
	return NULL;
}

static inline
struct sandbox_scmi_devices *sandbox_scmi_devices_ctx(struct udevice *dev)
{
	return NULL;
}
#endif /* CONFIG_SCMI_FIRMWARE */
#endif /* __SANDBOX_SCMI_TEST_H */
