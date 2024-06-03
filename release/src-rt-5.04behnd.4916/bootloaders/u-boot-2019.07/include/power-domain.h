/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 */

#ifndef _POWER_DOMAIN_H
#define _POWER_DOMAIN_H

/**
 * A power domain is a portion of an SoC or chip that is powered by a
 * switchable source of power. In many cases, software has control over the
 * power domain, and can turn the power source on or off. This is typically
 * done to save power by powering off unused devices, or to enable software
 * sequencing of initial powerup at boot. This API provides a means for
 * drivers to turn power domains on and off.
 *
 * A driver that implements UCLASS_POWER_DOMAIN is a power domain controller or
 * provider. A controller will often implement multiple separate power domains,
 * since the hardware it manages often has this capability.
 * power-domain-uclass.h describes the interface which power domain controllers
 * must implement.
 *
 * Depending on the power domain controller hardware, changing the state of a
 * power domain may require performing related operations on other resources.
 * For example, some power domains may require certain clocks to be enabled
 * whenever the power domain is powered on, or during the time when the power
 * domain is transitioning state. These details are implementation-specific
 * and should ideally be encapsulated entirely within the provider driver, or
 * configured through mechanisms (e.g. device tree) that do not require client
 * drivers to provide extra configuration information.
 *
 * Power domain consumers/clients are the drivers for HW modules within the
 * power domain. This header file describes the API used by those drivers.
 *
 * In many cases, a single complex IO controller (e.g. a PCIe controller) will
 * be the sole logic contained within a power domain. In such cases, it is
 * logical for the relevant device driver to directly control that power
 * domain. In other cases, multiple controllers, each with their own driver,
 * may be contained in a single power domain. Any logic require to co-ordinate
 * between drivers for these multiple controllers is beyond the scope of this
 * API at present. Equally, this API does not define or implement any policy
 * by which power domains are managed.
 */

struct udevice;

/**
 * struct power_domain - A handle to (allowing control of) a single power domain.
 *
 * Clients provide storage for power domain handles. The content of the
 * structure is managed solely by the power domain API and power domain
 * drivers. A power domain struct is initialized by "get"ing the power domain
 * struct. The power domain struct is passed to all other power domain APIs to
 * identify which power domain to operate upon.
 *
 * @dev: The device which implements the power domain.
 * @id: The power domain ID within the provider.
 *
 * Currently, the power domain API assumes that a single integer ID is enough
 * to identify and configure any power domain for any power domain provider. If
 * this assumption becomes invalid in the future, the struct could be expanded
 * to either (a) add more fields to allow power domain providers to store
 * additional information, or (b) replace the id field with an opaque pointer,
 * which the provider would dynamically allocate during its .of_xlate op, and
 * process during is .request op. This may require the addition of an extra op
 * to clean up the allocation.
 */
struct power_domain {
	struct udevice *dev;
	/*
	 * Written by of_xlate. We assume a single id is enough for now. In the
	 * future, we might add more fields here.
	 */
	unsigned long id;
};

/**
 * power_domain_get - Get/request the power domain for a device.
 *
 * This looks up and requests a power domain. Each device is assumed to have
 * a single (or, at least one) power domain associated with it somehow, and
 * that domain, or the first/default domain. The mapping of client device to
 * provider power domain may be via device-tree properties, board-provided
 * mapping tables, or some other mechanism.
 *
 * @dev:	The client device.
 * @power_domain	A pointer to a power domain struct to initialize.
 * @return 0 if OK, or a negative error code.
 */
#if CONFIG_IS_ENABLED(POWER_DOMAIN)
int power_domain_get(struct udevice *dev, struct power_domain *power_domain);
#else
static inline
int power_domain_get(struct udevice *dev, struct power_domain *power_domain)
{
	return -ENOSYS;
}
#endif

/**
 * power_domain_get_by_index - Get the indexed power domain for a device.
 *
 * @dev:		The client device.
 * @power_domain:	A pointer to a power domain struct to initialize.
 * @index:		Power domain index to be powered on.
 *
 * @return 0 if OK, or a negative error code.
 */
#if CONFIG_IS_ENABLED(POWER_DOMAIN)
int power_domain_get_by_index(struct udevice *dev,
			      struct power_domain *power_domain, int index);
#else
static inline
int power_domain_get_by_index(struct udevice *dev,
			      struct power_domain *power_domain, int index)
{
	return -ENOSYS;
}
#endif

/**
 * power_domain_free - Free a previously requested power domain.
 *
 * @power_domain:	A power domain struct that was previously successfully
 *		requested by power_domain_get().
 * @return 0 if OK, or a negative error code.
 */
#if CONFIG_IS_ENABLED(POWER_DOMAIN)
int power_domain_free(struct power_domain *power_domain);
#else
static inline int power_domain_free(struct power_domain *power_domain)
{
	return -ENOSYS;
}
#endif

/**
 * power_domain_on - Enable power to a power domain.
 *
 * @power_domain:	A power domain struct that was previously successfully
 *		requested by power_domain_get().
 * @return 0 if OK, or a negative error code.
 */
#if CONFIG_IS_ENABLED(POWER_DOMAIN)
int power_domain_on(struct power_domain *power_domain);
#else
static inline int power_domain_on(struct power_domain *power_domain)
{
	return -ENOSYS;
}
#endif

/**
 * power_domain_off - Disable power ot a power domain.
 *
 * @power_domain:	A power domain struct that was previously successfully
 *		requested by power_domain_get().
 * @return 0 if OK, or a negative error code.
 */
#if CONFIG_IS_ENABLED(POWER_DOMAIN)
int power_domain_off(struct power_domain *power_domain);
#else
static inline int power_domain_off(struct power_domain *power_domain)
{
	return -ENOSYS;
}
#endif

#endif
