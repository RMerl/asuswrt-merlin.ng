/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * Copyright 2014, ASUSTeK Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */
#include <rtconfig.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <bcmnvram.h>
#include <shared.h>
#include <bled_defs.h>
#include <stdio.h>
#include "shutils.h"

#if defined(RTCONFIG_BLINK_LED)
/**
 * Add/Strip GPIO_BLINK_LED flag to specified led_xxx_gpio.
 * @led_gpio:	pointer to name of "led_xxx_gpio"
 * @return:
 *  	0:	success
 *  otherwisze:	fail
 */
static inline int __update_gpio_nv_var(const char *led_gpio, int add)
{
	char *p;

	if (!led_gpio || !(p = nvram_get(led_gpio)))
		return -1;

	if (add)
		nvram_set_int(led_gpio, atoi(p) | GPIO_BLINK_LED);
	else
		nvram_set_int(led_gpio, atoi(p) & ~GPIO_BLINK_LED);
	return 0;
}

/**
 * Get XXX of led_XXX_gpio.
 * @led_gpio:		pointer to name of "led_xxx_gpio"
 * @return:		pointer to result.
 */
static char *led_gpio_id(const char *led_gpio)
{
	static char buf[BLED_ID_LEN];
	char *p, tmp[32];

	*buf = '\0';
	if (!led_gpio || strncmp(led_gpio, "led_", 4))
		return buf;

	strlcpy(tmp, led_gpio + 4, sizeof(tmp));
	if ((p = strstr(tmp, "_gpio")))
		*p = '\0';
	strlcpy(buf, tmp, sizeof(buf));

	return buf;
}

/**
 * Low-level function to add a LED which blinks in accordance with traffic of net_device
 * which is specified by ifname.
 * @led_gpio:		pointer to name of "led_xxx_gpio"
 * @ifname:		interface name, e.g., br0, vlan2, eth0, etc.
 * @min_blink_speeed:	blink led if speed greater than or equal to min_blink_speeed. (unit: KBytes/s)
 * @interval:		blinking interval. (unit: ms)
 * @return:
 */
int __config_netdev_bled(const char *led_gpio, const char *ifname, unsigned int min_blink_speed, unsigned int interval)
{
	int gpio_nr, fd, r, ret = 0;
	struct ndev_bled nl;
	struct bled_common *bl = &nl.bled;

	if (!ifname || *ifname == '\0')
		return -1;

	if (__update_gpio_nv_var(led_gpio, 1) < 0)
		return -2;

	if ((gpio_nr = extract_gpio_pin(led_gpio)) < 0) {
		ret = -3;
		goto exit___config_netdev_bled;
	}

	if ((fd = open(BLED_DEVNAME, O_RDWR)) < 0) {
		_dprintf("%s: open %s fail. (%s)\n", __func__, BLED_DEVNAME, strerror(errno));
		ret = -4;
		goto exit___config_netdev_bled;
	}

	memset(&nl, 0, sizeof(nl));
	bl->gpio_nr = gpio_nr;
	bl->gpio_api_id = GPIO_API_PLATFORM;
	bl->active_low = !!(nvram_get_int(led_gpio) & GPIO_ACTIVE_LOW);
	bl->state = BLED_STATE_STOP;
	bl->bh_type = BLED_BHTYPE_TIMER;
	bl->mode = BLED_NORMAL_MODE;
	bl->min_blink_speed = min_blink_speed;
	bl->interval = interval;
	strlcpy(bl->id, led_gpio_id(led_gpio), sizeof(bl->id));
	strlcpy(nl.ifname, ifname, sizeof(nl.ifname));

	if ((r = ioctl(fd, BLED_CTL_ADD_NETDEV_BLED, &nl)) < 0 && errno != EEXIST) {
		_dprintf("%s: ioctl(BLED_CTL_ADD_NETDEV_BLED) fail, return %d errno %d (%s)\n",
			__func__, r, errno, strerror(errno));
		close(fd);
		ret = -5;
		goto exit___config_netdev_bled;
	}

	close(fd);

	return 0;

exit___config_netdev_bled:
	__update_gpio_nv_var(led_gpio, 0);
	return ret;
}

/**
 * Simpler interface to add a LED which blinks in accordance with traffic of net_device
 * which is specified by ifname.
 * @led_gpio:	pointer to name of "led_xxx_gpio"
 * @ifname:	interface name, e.g., br0, vlan2, eth0, etc.
 * @return:
 */
int config_netdev_bled(const char *led_gpio, const char *ifname)
{
	unsigned int min_blink_speed = 10;	/* KB/s */
	unsigned int interval = 100;		/* ms */

	return __config_netdev_bled(led_gpio, ifname, min_blink_speed, interval);
}

/**
 * Set user defined pattern to a bled.
 * @led_gpio:	pointer to name of "led_xxx_gpio", the bled must be configured first.
 * 		e.g., config_netdev_bled(), config_swports_bled(), and config_usbbus_bled()
 * @interval:	length of each item in @pattern. (unit: ms)
 * 		BLED_UDEF_PATTERN_MIN_INTERVAL ~ BLED_UDEF_PATTERN_MAX_INTERVAL
 * @pattern:	user defined pattern. e.g., "1 0 1 1 0 0"
 * 		0: turn off LED.
 * 		otherwise: turn on LED.
 * @return:
 */
int set_bled_udef_pattern(const char *led_gpio, unsigned int interval, const char *pattern)
{
	int gpio_nr, fd, r, ret = 0;
	char *str = NULL, *p, *token;
	struct bled_common bl_common, *bl = &bl_common;
	unsigned int curr, *patt = &bl->pattern[0];

	if (!led_gpio || *led_gpio == '\0' || !(nvram_get_int(led_gpio) & GPIO_BLINK_LED) ||
	    interval < BLED_UDEF_PATTERN_MIN_INTERVAL || interval > BLED_UDEF_PATTERN_MAX_INTERVAL ||
	    !pattern || *pattern == '\0')
		return -1;

	if ((gpio_nr = extract_gpio_pin(led_gpio)) < 0) {
		ret = -2;
		goto exit_set_bled_udef_pattern;
	}

	if (!(str = strdup(pattern))) {
		ret = -3;
		goto exit_set_bled_udef_pattern;
	}
	memset(bl, 0, sizeof(*bl));
	bl->gpio_nr = gpio_nr;
	bl->pattern_interval = interval;
	for (curr = 0, p = str; curr < BLED_MAX_NR_PATTERN; curr++, p = NULL) {
		if (!(token = strtok(p, " ")))
			break;
		*patt++ = !!atoi(token);
	}
	if (!curr) {
		_dprintf("%s: %s: pattern not found.\n", __func__, led_gpio);
		ret = -4;
		goto exit_set_bled_udef_pattern;
	}
	bl->nr_pattern = curr;

	if ((fd = open(BLED_DEVNAME, O_RDWR)) < 0) {
		_dprintf("%s: open %s fail. (%s)\n", __func__, BLED_DEVNAME, strerror(errno));
		ret = -5;
		goto exit_set_bled_udef_pattern;
	}

	if ((r = ioctl(fd, BLED_CTL_SET_UDEF_PATTERN, bl)) < 0) {
		_dprintf("%s: ioctl(BLED_CTL_SET_UDEF_PATTERN) fail, return %d errno %d (%s)\n",
			__func__, r, errno, strerror(errno));
		close(fd);
		ret = -6;
		goto exit_set_bled_udef_pattern;
	}

	close(fd);

exit_set_bled_udef_pattern:
	if (str)
		free(str);

	return ret;
}

/**
 * Set user defined pattern whether triggered (correspond to gpio_nr[]) to a bled.
 * @main_led_gpio:	pointer to name of "led_xxx_gpio", the bled must be configured first.
 * 			e.g., config_netdev_bled(), config_swports_bled(), and config_usbbus_bled()
 * @trigger:		e.g., "1 0 1"
 * 			0: turn off LED.
 * 			otherwise: run user defined pattern.
 * @return:
 */
int set_bled_udef_trigger(const char *main_led_gpio, const char *trigger)
{
	int gpio_nr, fd, r, ret = 0;
	char *str = NULL, *p, *token;
	struct bled_common bl_common, *bl = &bl_common;
	unsigned int curr, *m = &bl->trigger[0];

	if (!main_led_gpio || *main_led_gpio == '\0' || !(nvram_get_int(main_led_gpio) & GPIO_BLINK_LED) ||
	    !trigger || *trigger == '\0')
		return -1;

	if ((gpio_nr = extract_gpio_pin(main_led_gpio)) < 0) {
		ret = -2;
		goto exit_set_bled_udef_trigger;
	}

	if (!(str = strdup(trigger))) {
		ret = -3;
		goto exit_set_bled_udef_trigger;
	}
	memset(bl, 0, sizeof(*bl));
	bl->gpio_nr = gpio_nr;
	for (curr = 0, p = str; curr < BLED_MAX_NR_GPIO_PER_BLED; curr++, p = NULL) {
		if (!(token = strtok(p, " ")))
			break;
		*m++ = !!atoi(token);
	}
	if (!curr) {
		_dprintf("%s: %s: trigger not found.\n", __func__, main_led_gpio);
		ret = -4;
		goto exit_set_bled_udef_trigger;
	}

	if ((fd = open(BLED_DEVNAME, O_RDWR)) < 0) {
		_dprintf("%s: open %s fail. (%s)\n", __func__, BLED_DEVNAME, strerror(errno));
		ret = -5;
		goto exit_set_bled_udef_trigger;
	}

	if ((r = ioctl(fd, BLED_CTL_SET_UDEF_TRIGGER, bl)) < 0) {
		_dprintf("%s: ioctl(BLED_CTL_SET_UDEF_TRIGGER) fail, return %d errno %d (%s)\n",
			__func__, r, errno, strerror(errno));
		close(fd);
		ret = -6;
		goto exit_set_bled_udef_trigger;
	}
	close(fd);

exit_set_bled_udef_trigger:
	if (str)
		free(str);

	return ret;
}

/**
 * Set operation mode of a bled.
 * @led_gpio:	pointer to name of "led_xxx_gpio", the bled must be configured first.
 * 		e.g., config_netdev_bled(), config_swports_bled(), and config_usbbus_bled()
 * @mode:	one of bled_mode enumeration, except BLED_MODE_MAX
 * @return:
 */
static int __set_bled_mode(const char *led_gpio, enum bled_mode bled_mode)
{
	int gpio_nr, fd, r, ret = 0;
	struct bled_common bl_common, *bl = &bl_common;

	if (!led_gpio || *led_gpio == '\0' || !(nvram_get_int(led_gpio) & GPIO_BLINK_LED) ||
	    bled_mode < 0 || bled_mode >= BLED_MODE_MAX)
		return -1;

	if ((gpio_nr = extract_gpio_pin(led_gpio)) < 0) {
		ret = -2;
		goto exit___set_bled_mode;
	}

	if ((fd = open(BLED_DEVNAME, O_RDWR)) < 0) {
		_dprintf("%s: open %s fail. (%s)\n", __func__, BLED_DEVNAME, strerror(errno));
		ret = -3;
		goto exit___set_bled_mode;
	}

	memset(bl, 0, sizeof(*bl));
	bl->gpio_nr = gpio_nr;
	bl->mode = bled_mode;

	if ((r = ioctl(fd, BLED_CTL_SET_MODE, bl)) < 0) {
		_dprintf("%s: ioctl(BLED_CTL_SET_MODE) fail, return %d errno %d (%s)\n",
			__func__, r, errno, strerror(errno));
		close(fd);
		ret = -4;
		goto exit___set_bled_mode;
	}

	close(fd);

exit___set_bled_mode:

	return ret;
}

int set_bled_normal_mode(const char *led_gpio)
{
	return __set_bled_mode(led_gpio, BLED_NORMAL_MODE);
}

int set_bled_udef_pattern_mode(const char *led_gpio)
{
	return __set_bled_mode(led_gpio, BLED_UDEF_PATTERN_MODE);
}

/**
 * Low-level function to change state of bled.
 * @led_gpio:	pointer to name of "led_xxx_gpio", it's okay to be NULL or empty as long as none of any GPIO pin is shared by two or more BLED, e.g., TUF-AX4200Q.
 * @gpio_nr:	GPIO# of a LED
 * @state:
 * @return:
 */
static int chg_bled_state(const char *led_gpio, unsigned int gpio_nr, enum bled_state state)
{
	int fd, r;
	struct bled_common bled;

	if (!(gpio_nr & GPIO_BLINK_LED))
		return 0;
	gpio_nr &= 0xFF;
	if (gpio_nr == 0xFF)
		return -1;
	if (state >= BLED_STATE_MAX || state < 0)
		return -2;

	if ((fd = open(BLED_DEVNAME, O_RDWR)) < 0) {
		_dprintf("%s: open %s fail. (%s)\n", __func__, BLED_DEVNAME, strerror(errno));
		return -3;
	}

	memset(&bled, 0, sizeof(bled));
	bled.gpio_nr = gpio_nr;
	bled.state = state;
	strlcpy(bled.id, led_gpio_id(led_gpio), sizeof(bled.id));
	if ((r = ioctl(fd, BLED_CTL_CHG_STATE, &bled)) < 0) {
		_dprintf("%s: ioctl(BLED_CTL_CHG_STATE) fail, return %d errno %d (%s)\n",
			__func__, r, errno, strerror(errno));
		close(fd);
		return -4;
	}
	close(fd);

	return 0;
}

/**
 * Request bled kernel module to start blink specified LED in accordance with checking result.
 * @led_gpio:	pointer to name of "led_xxx_gpio", it's okay to be NULL or empty as long as none of any GPIO pin is shared by two or more BLED, e.g., TUF-AX4200Q.
 * @gpio_nr:	GPIO# of a LED, include flags. (GPIO_BLINK_LED)
 * @return:
 */
int __start_bled(const char *led_gpio, unsigned int gpio_nr)
{
	return chg_bled_state(led_gpio, gpio_nr, BLED_STATE_RUN);
}

/**
 * Request bled kernel module to stop blink specified LED.
 * @led_gpio:	pointer to name of "led_xxx_gpio", it's okay to be NULL or empty as long as none of any GPIO pin is shared by two or more BLED, e.g., TUF-AX4200Q.
 * @gpio_nr:	GPIO# of a LED, include flags. (GPIO_BLINK_LED)
 * @return:
 */
int __stop_bled(const char *led_gpio, unsigned int gpio_nr)
{
	return chg_bled_state(led_gpio, gpio_nr, BLED_STATE_STOP);
}

/**
 * Delete a bled.
 * @led_gpio:	pointer to name of "led_xxx_gpio", it's okay to be NULL or empty as long as none of any GPIO pin is shared by two or more BLED, e.g., TUF-AX4200Q.
 * @gpio_nr:	GPIO# of a LED
 * @return:
 */
int __del_bled(const char *led_gpio, unsigned int gpio_nr)
{
	int fd, r;
	struct bled_common bled;

	gpio_nr &= 0xFF;
	if (gpio_nr == 0xFF)
		return -1;
	if (!led_gpio)
		led_gpio = "";

	if ((fd = open(BLED_DEVNAME, O_RDWR)) < 0)
		return -2;

	memset(&bled, 0, sizeof(bled));
	bled.gpio_nr = gpio_nr;
	strlcpy(bled.id, led_gpio, sizeof(bled.id));
	if ((r = ioctl(fd, BLED_CTL_DEL_BLED, &bled)) < 0) {
		_dprintf("%s: ioctl(BLED_CTL_DEL_BLED) fail, return %d errno %d (%s)\n",
			__func__, r, errno, strerror(errno));
		close(fd);
		return -4;
	}
	close(fd);

	return 0;
}

/**
 * Append a new interface to a exist bled.
 * @led_gpio:	pointer to name of "led_xxx_gpio"
 * @ifname:	interface name, e.g., br0, vlan2, eth0, etc.
 * @return:
 */
int append_netdev_bled_if(const char *led_gpio, const char *ifname)
{
	int gpio_nr, fd, r;
	struct ndev_bled nl;
	struct bled_common *bl = &nl.bled;

	if (!led_gpio || *led_gpio == '\0' || !ifname || *ifname == '\0')
		return -1;

	if (!(nvram_get_int(led_gpio) & GPIO_BLINK_LED))
		return -2;

	if ((gpio_nr = extract_gpio_pin(led_gpio)) < 0)
		return -3;

	if ((fd = open(BLED_DEVNAME, O_RDWR)) < 0) {
		_dprintf("%s: open %s fail. (%s)\n", __func__, BLED_DEVNAME, strerror(errno));
		return -4;
	}

	memset(&nl, 0, sizeof(nl));
	bl->gpio_nr = gpio_nr;
	strlcpy(nl.ifname, ifname, sizeof(nl.ifname));
	strlcpy(bl->id, led_gpio_id(led_gpio), sizeof(bl->id));

	if ((r = ioctl(fd, BLED_CTL_ADD_NETDEV_IF, &nl)) < 0) {
		_dprintf("%s: ioctl(BLED_CTL_ADD_NETDEV_IF) fail, ifname [%s] return %d errno %d (%s)\n",
			__func__, ifname, r, errno, strerror(errno));
		close(fd);
		return -5;
	}

	close(fd);

	return 0;
}

/**
 * Remove a interface from a exist bled.
 * @led_gpio:	pointer to name of "led_xxx_gpio"
 * @ifname:	interface name, e.g., br0, vlan2, eth0, etc.
 * @return:
 */
int remove_netdev_bled_if(const char *led_gpio, const char *ifname)
{
	int gpio_nr, fd, r;
	struct ndev_bled nl;
	struct bled_common *bl = &nl.bled;

	if (!led_gpio || *led_gpio == '\0' || !ifname || *ifname == '\0')
		return -1;

	if (!(nvram_get_int(led_gpio) & GPIO_BLINK_LED))
		return -2;

	if ((gpio_nr = extract_gpio_pin(led_gpio)) < 0)
		return -3;

	if ((fd = open(BLED_DEVNAME, O_RDWR)) < 0) {
		_dprintf("%s: open %s fail. (%s)\n", __func__, BLED_DEVNAME, strerror(errno));
		return -4;
	}

	memset(&nl, 0, sizeof(nl));
	bl->gpio_nr = gpio_nr;
	strlcpy(nl.ifname, ifname, sizeof(nl.ifname));
	strlcpy(bl->id, led_gpio_id(led_gpio), sizeof(bl->id));

	if ((r = ioctl(fd, BLED_CTL_DEL_NETDEV_IF, &nl)) < 0) {
		_dprintf("%s: ioctl(BLED_CTL_DEL_NETDEV_IF) fail, ifname [%s] return %d errno %d (%s)\n",
			__func__, ifname, r, errno, strerror(errno));
		close(fd);
		return -5;
	}

	close(fd);

	return 0;
}

/**
 * Low-level function to add a LED which blinks in accordance with traffic of switch port(s).
 * which is specified by ifname.
 * @led_gpio:		pointer to name of "led_xxx_gpio"
 * @port_mask:		switch port mask
 * @min_blink_speeed:	blink led if speed greater than or equal to min_blink_speeed. (unit: KBytes/s)
 * @interval:		blinking interval. (unit: ms)
 * @sleep:		Run check function in process context or not.
 * 			If platform-specific switch's check function needs to take mutex or semaphore,
 * 			sleep variable must be non-zero value.
 * @return:
 */
int __config_swports_bled(const char *led_gpio, unsigned int port_mask, unsigned int min_blink_speed, unsigned int interval, int sleep)
{
	int gpio_nr, fd, r, ret = 0;
	struct swport_bled sl;
	struct bled_common *bl = &sl.bled;

	if (__update_gpio_nv_var(led_gpio, 1) < 0)
		return -1;

	if ((gpio_nr = extract_gpio_pin(led_gpio)) < 0) {
		ret = -2;
		goto exit___config_swports_bled;
	}

	if ((fd = open(BLED_DEVNAME, O_RDWR)) < 0) {
		_dprintf("%s: open %s fail. (%s)\n", __func__, BLED_DEVNAME, strerror(errno));
		ret = -3;
		goto exit___config_swports_bled;
	}

	memset(&sl, 0, sizeof(sl));
	bl->gpio_nr = gpio_nr;
	bl->gpio_api_id = GPIO_API_PLATFORM;
	bl->active_low = !!(nvram_get_int(led_gpio) & GPIO_ACTIVE_LOW);
	bl->state = BLED_STATE_STOP;
	bl->bh_type = sleep? BLED_BHTYPE_HYBRID:BLED_BHTYPE_TIMER;
	bl->mode = BLED_NORMAL_MODE;
	bl->min_blink_speed = min_blink_speed;
	bl->interval = interval;
	strlcpy(bl->id, led_gpio_id(led_gpio), sizeof(bl->id));
	sl.port_mask = port_mask;

	if ((r = ioctl(fd, BLED_CTL_ADD_SWPORTS_BLED, &sl)) < 0 && errno != EEXIST) {
		_dprintf("%s: ioctl(BLED_CTL_ADD_NETDEV_BLED) fail, return %d errno %d (%s)\n",
			__func__, r, errno, strerror(errno));
		close(fd);
		ret = -4;
		goto exit___config_swports_bled;
	}

	close(fd);

	return 0;

exit___config_swports_bled:

	__update_gpio_nv_var(led_gpio, 0);
	return ret;
}

/**
 * Update port mask of a bled
 * @led_gpio:	pointer to name of "led_xxx_gpio"
 * @port_mask:	port bit-mask
 * @return:
 */
int update_swports_bled(const char *led_gpio, unsigned int port_mask)
{
	const char *iface;
	int gpio_nr, fd, r, i, vport, found;
	unsigned int m;
	struct swport_bled sl;
	struct bled_common *bl = &sl.bled;

	if (!led_gpio || *led_gpio == '\0')
		return -1;

	if (!(nvram_get_int(led_gpio) & GPIO_BLINK_LED))
		return -2;

	if ((gpio_nr = extract_gpio_pin(led_gpio)) < 0)
		return -3;

	if ((fd = open(BLED_DEVNAME, O_RDWR)) < 0) {
		_dprintf("%s: open %s fail. (%s)\n", __func__, BLED_DEVNAME, strerror(errno));
		return -4;
	}

	/* Get old swports bled settings */
	memset(&sl, 0, sizeof(sl));
	bl->gpio_nr = gpio_nr;
	if ((r = ioctl(fd, BLED_CTL_GET_SWPORTS_SETTINGS, &sl)) < 0) {
		_dprintf("%s: ioctl(BLED_CTL_GET_SWPORTS_SETTINGS) fail, return %d errno %d (%s)\n",
			__func__, r, errno, strerror(errno));
	} else {
		/* Find interface backed virtual ports.
		 * If found, remove it from port_mask and add/remove interface to/from bled
		 * based on it is used or not.
		 */
		for (vport = 0, m = port_mask ; m > 0 ; vport++, m >>= 1) {
			if (!(iface = vport_to_iface_name(vport)))
				continue;

			port_mask &= ~(1U << vport);
			for (found = 0, i = 0; !found && i < sl.nr_if; ++i) {
				if (strcmp(iface, sl.ifname[i]))
					continue;
				found = 1;
			}

			if (m & 1) {
				/* add interface to bled. */
				if (found)
					continue;
				append_netdev_bled_if(led_gpio, iface);
			} else {
				/* remove interface from bled. */
				if (!found)
					continue;
				remove_netdev_bled_if(led_gpio, iface);
			}
		}
	}

	memset(&sl, 0, sizeof(sl));
	bl->gpio_nr = gpio_nr;
	sl.port_mask = vportmask_to_rportmask(port_mask);

	if ((r = ioctl(fd, BLED_CTL_UPD_SWPORTS_MASK, &sl)) < 0) {
		_dprintf("%s: ioctl(BLED_CTL_UPD_SWPORTS_MASK) fail, return %d errno %d (%s)\n",
			__func__, r, errno, strerror(errno));
		close(fd);
		return -5;
	}

	close(fd);

	return 0;
}

/**
 * Low-level function to add a LED which blinks in accordance with traffic of usb bus(s)
 * which is specified by bus_mask.
 * @led_gpio:		pointer to name of "led_xxx_gpio"
 * @bus_list:		bus list
 * @min_blink_speeed:	blink led if speed greater than or equal to min_blink_speeed. (unit: KBytes/s)
 * @interval:		blinking interval. (unit: ms)
 * @sleep:		Run check function in process context or not.
 * 			If platform-specific switch's check function needs to take mutex or semaphore,
 * 			sleep variable must be non-zero value.
 * @return:
 */
int __config_usbbus_bled(const char *led_gpio, char *bus_list, unsigned int min_blink_speed, unsigned int interval)
{
	int gpio_nr, fd, r, b, ret = 0;
	char word[100], *next;
	unsigned int bus_mask = 0;
	struct usbbus_bled ul;
	struct bled_common *bl = &ul.bled;

	if (__update_gpio_nv_var(led_gpio, 1) < 0)
		return -1;

	if ((gpio_nr = extract_gpio_pin(led_gpio)) < 0) {
		ret = -2;
		goto exit___config_usbbus_bled;
	}

	if ((fd = open(BLED_DEVNAME, O_RDWR)) < 0) {
		_dprintf("%s: open %s fail. (%s)\n", __func__, BLED_DEVNAME, strerror(errno));
		ret = -3;
		goto exit___config_usbbus_bled;
	}

	foreach(word, bus_list, next) {
		b = atoi(word) - 1;	/* bit0 = bus1 */
		if (b < 0)
			continue;
		bus_mask |= (1U << b);
	}

	memset(&ul, 0, sizeof(ul));
	bl->gpio_nr = gpio_nr;
	bl->gpio_api_id = GPIO_API_PLATFORM;
	bl->active_low = !!(nvram_get_int(led_gpio) & GPIO_ACTIVE_LOW);
	bl->state = BLED_STATE_STOP;
	bl->bh_type = BLED_BHTYPE_TIMER;
	bl->mode = BLED_NORMAL_MODE;
	bl->min_blink_speed = min_blink_speed;
	bl->interval = interval;
	strlcpy(bl->id, led_gpio_id(led_gpio), sizeof(bl->id));
	ul.bus_mask = bus_mask;

	if ((r = ioctl(fd, BLED_CTL_ADD_USBBUS_BLED, &ul)) < 0 && errno != EEXIST) {
		_dprintf("%s: ioctl(BLED_CTL_ADD_USBBUS_BLED) fail, return %d errno %d (%s)\n",
			__func__, r, errno, strerror(errno));
		close(fd);
		ret = -4;
		goto exit___config_usbbus_bled;
	}

	close(fd);

	return 0;

exit___config_usbbus_bled:

	__update_gpio_nv_var(led_gpio, 0);
	return ret;
}

/**
 * Low-level function to add a LED which blinks in accordance with number of interrupt.
 * which is specified by interrupt_list.
 * @led_gpio:		pointer to name of "led_xxx_gpio"
 * @interrupt_list:	interrupt list, e.g., "90 67", etc
 * @min_blink_speeed:	blink led if speed greater than or equal to min_blink_speeed. (unit: count/s)
 * @interval:		blinking interval. (unit: ms)
 * @return:
 */
int __config_interrupt_bled(const char *led_gpio, char *interrupt_list, unsigned int min_blink_speed, unsigned int interval)
{
	int gpio_nr, fd, r, ret = 0;
	unsigned int *p;
	char word[100], *next;
	struct interrupt_bled il;
	struct bled_common *bl = &il.bled;

	if (!interrupt_list || *interrupt_list == '\0')
		return -1;

	if (__update_gpio_nv_var(led_gpio, 1) < 0)
		return -2;

	if ((gpio_nr = extract_gpio_pin(led_gpio)) < 0) {
		ret = -3;
		goto exit___config_interrupt_bled;
	}

	memset(&il, 0, sizeof(il));
	p = &il.interrupt[0];
	foreach(word, interrupt_list, next) {
		*p++ = atoi(word);
		il.nr_interrupt++;
		if (il.nr_interrupt > ARRAY_SIZE(il.interrupt)) {
			ret = -4;
			goto exit___config_interrupt_bled;
		}
	}

	if ((fd = open(BLED_DEVNAME, O_RDWR)) < 0) {
		_dprintf("%s: open %s fail. (%s)\n", __func__, BLED_DEVNAME, strerror(errno));
		ret = -4;
		goto exit___config_interrupt_bled;
	}

	bl->gpio_nr = gpio_nr;
	bl->gpio_api_id = GPIO_API_PLATFORM;
	bl->active_low = !!(nvram_get_int(led_gpio) & GPIO_ACTIVE_LOW);
	bl->state = BLED_STATE_STOP;
	bl->bh_type = BLED_BHTYPE_TIMER;
	bl->mode = BLED_NORMAL_MODE;
	bl->min_blink_speed = min_blink_speed;
	bl->interval = interval;
	strlcpy(bl->id, led_gpio_id(led_gpio), sizeof(bl->id));

	if ((r = ioctl(fd, BLED_CTL_ADD_INTERRUPT_BLED, &il)) < 0 && errno != EEXIST) {
		_dprintf("%s: ioctl(BLED_CTL_ADD_INTERRUPT_BLED) fail, return %d errno %d (%s)\n",
			__func__, r, errno, strerror(errno));
		close(fd);
		ret = -5;
		goto exit___config_interrupt_bled;
	}

	close(fd);

	return 0;

exit___config_interrupt_bled:
	__update_gpio_nv_var(led_gpio, 0);
	return ret;
}

/**
 * Simpler interface to add a LED which blinks in accordance with number of interrupts
 * which is specified by interrupt_list.
 * @led_gpio:		pointer to name of "led_xxx_gpio"
 * @interrupt_list:	interrupt list, e.g., "90 67", etc
 * @return:
 */
int config_interrupt_bled(const char *led_gpio, char *interrupt_list)
{
	unsigned int min_blink_speed = 10;	/* times/s */
	unsigned int interval = 100;		/* ms */

	return __config_interrupt_bled(led_gpio, interrupt_list, min_blink_speed, interval);
}

/**
 * Return bled type.
 * @led_gpio:	pointer to name of "led_xxx_gpio"
 * @return:
 *  >= 0:	enum bled_type
 *  <  0:	fail
 */
static int get_bled_type(const char *led_gpio)
{
	int fd, param, r, v;

	if (!led_gpio || *led_gpio == '\0')
		return -1;
	v = nvram_get_int(led_gpio);
	if (v == 0xFF || !(v & GPIO_BLINK_LED))
		return -2;
	if ((param = extract_gpio_pin(led_gpio)) < 0)
		return -3;

	if ((fd = open(BLED_DEVNAME, O_RDWR)) < 0) {
		_dprintf("%s: open %s fail. (%s)\n", __func__, BLED_DEVNAME, strerror(errno));
		return -4;
	}

	if ((r = ioctl(fd, BLED_CTL_GET_BLED_TYPE, &param)) < 0) {
		_dprintf("%s: ioctl(BLED_CTL_GET_BLED_TYPE) fail, return %d errno %d (%s)\n",
			__func__, r, errno, strerror(errno));
		close(fd);
		return -5;
	}
	close(fd);

	return param;
}

/**
 * Checks whether a led_gpio is swports_bled or not.
 * @led_gpio:	pointer to name of "led_xxx_gpio"
 * @return:
 * 	0:	not swports bled
 *  otherwise:	swports bled
 */
int is_swports_bled(const char *led_gpio)
{
	return (get_bled_type(led_gpio) == BLED_TYPE_SWPORTS_BLED)? 1:0;
}

/**
 * Add a gpio in @led_gpio to an exist bled indexed by @main_led_gpio.
 * @main_led_gpio:	pointer to name of "led_xxx_gpio"
 * @led_gpio:		pointer to name of "led_xxx_gpio"
 * @return:
 * 	0:		success
 *  otherwise:		fail
 */
int add_gpio_to_bled(const char *main_led_gpio, const char *led_gpio)
{
	int gpio_nr, gpio2_nr, fd, r, ret = 0;
	struct bled_common bled, *bl = &bled;

	if ((gpio_nr = extract_gpio_pin(main_led_gpio)) < 0) {
		ret = -1;
		goto exit_add_gpio_to_bled;
	}
	if ((gpio2_nr = extract_gpio_pin(led_gpio)) < 0) {
		ret = -2;
		goto exit_add_gpio_to_bled;
	}

	if ((fd = open(BLED_DEVNAME, O_RDWR)) < 0) {
		_dprintf("%s: open %s fail. (%s)\n", __func__, BLED_DEVNAME, strerror(errno));
		ret = -3;
		goto exit_add_gpio_to_bled;
	}

	memset(&bled, 0, sizeof(bled));
	bl->gpio_nr = gpio_nr;
	bl->active_low = !!(nvram_get_int(led_gpio) & GPIO_ACTIVE_LOW);
	bl->gpio2_nr = gpio2_nr;

	if ((r = ioctl(fd, BLED_CTL_ADD_GPIO, &bled)) < 0 && errno != EEXIST) {
		_dprintf("%s: ioctl(BLED_CTL_ADD_GPIO) fail, return %d errno %d (%s)\n",
			__func__, r, errno, strerror(errno));
		close(fd);
		ret = -4;
		goto exit_add_gpio_to_bled;
	}

	close(fd);

	return 0;

exit_add_gpio_to_bled:
	return ret;
}

/*
 * model dependence
 */
#if (defined(PLN12) || defined(PLAC56))
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(ary) (sizeof(ary) / sizeof((ary)[0]))
#endif

void set_wifiled(int mode)
{
	int i;
	struct wifi_led {
		char *gpio[2];
	} p = {
		.gpio = {
#if defined(PLN12)
			"led_2g_red_gpio",
			NULL
#elif defined(PLAC56)
			"led_2g_green_gpio",
			"led_5g_green_gpio"
#endif
		}
	};

	switch (mode) {
	case 1: /* on */
		for (i = 0; i < ARRAY_SIZE(p.gpio); ++i) {
			if (!p.gpio[i])
				break;

			set_bled_udef_pattern(p.gpio[i], 1000, "1 1");
			set_bled_udef_pattern_mode(p.gpio[i]);
		}
		break;
	case 2: /* off */
		for (i = 0; i < ARRAY_SIZE(p.gpio); ++i) {
			if (!p.gpio[i])
				break;

			set_bled_udef_pattern(p.gpio[i], 1000, "0 0");
			set_bled_udef_pattern_mode(p.gpio[i]);
		}
		break;
	case 3:	/* WPS */
		for (i = 0; i < ARRAY_SIZE(p.gpio); ++i) {
			if (!p.gpio[i])
				break;

			set_bled_udef_pattern(p.gpio[i], 50, "0 1 0 1 0 1 0 1 0 0 0 0 0 0 0 0");
			set_bled_udef_pattern_mode(p.gpio[i]);
		}
		break;
	case 4: /* WPS success */
		for (i = 0; i < ARRAY_SIZE(p.gpio); ++i) {
			if (!p.gpio[i])
				break;

			set_bled_udef_pattern(p.gpio[i], 100, "0 1");
			set_bled_udef_pattern_mode(p.gpio[i]);
		}
		break;
	case 5: /* press reset button */
		for (i = 0; i < ARRAY_SIZE(p.gpio); ++i) {
			if (!p.gpio[i])
				break;

			set_bled_udef_pattern(p.gpio[i], 500, "0 1");
			set_bled_udef_pattern_mode(p.gpio[i]);
		}
		break;
	case 6: /* firmware upgrade */
		for (i = 0; i < ARRAY_SIZE(p.gpio); ++i) {
			if (!p.gpio[i])
				break;

			set_bled_udef_pattern(p.gpio[i], 1000, "0 1");
			set_bled_udef_pattern_mode(p.gpio[i]);
		}
		break;
	default:
		/* normal mode */
		for (i = 0; i < ARRAY_SIZE(p.gpio); ++i) {
			if (!p.gpio[i])
				break;

			set_bled_normal_mode(p.gpio[i]);
		}
	}
}
#elif defined(RTCONFIG_GPIOX3_RGBLED)
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(ary) (sizeof(ary) / sizeof((ary)[0]))
#endif

static unsigned int rgbled_udef_mode = 0;

void set_rgbled(unsigned int mode)
{
	unsigned int cmask = RGBLED_COLOR_MESK, bmask = RGBLED_BLINK_MESK;
	unsigned int c = mode & cmask;
	unsigned int b = mode & bmask;
	char *main_led_gpio = "led_blue_gpio";
	struct udef_pattern {
		unsigned int interval;
		char *pattern;
	} u[] = {
		{1000,	"1 1"},		/* on */
		{500,	"0 1"},		/* off:0.5s, on:0.5s */
		{250,	"0 1 1 1"},	/* off:0.25s, on:0.75s */
		{125,	"0 1"},		/* off:0.125s, on:0.125s */
		{3000,	"0 1"},		/* off:3s, on:3s */
		{0, NULL}
	};
	int uidx = 0;
	char *led_color[] = {
		"0 0 0 0",		/* off: B G R W */
		"1 0 0 0",		/* RGBLED_BLUE */
		"0 1 0 0",		/* RGBLED_GREEN */
		"0 0 1 0",		/* RGBLED_RED */
		"1 1 0 0",		/* RGBLED_NIAGARA_BLUE */
		"0 1 1 0",		/* RGBLED_YELLOW */
		"1 0 1 0",		/* RGBLED_PURPLE */
		"1 1 1 1"		/* RGBLED_WHITE */
	};
	char *udef_trigger = led_color[0];
#ifdef RTCONFIG_SW_CTRL_ALLLED
	int ctrl_led_off = nvram_match("lp55xx_lp5523_user_enable", "1") | nvram_match("AllLED", "0");
#endif

	if (rgbled_udef_mode == 0) {
		led_control(LED_BLUE, LED_ON);
		led_control(LED_GREEN, LED_ON);
		led_control(LED_RED, LED_ON);
		if (RGBLED_WHITE & RGBLED_WLED)
			led_control(LED_WHITE, LED_ON);
		rgbled_udef_mode = 1;
	}

	switch (c) {
	case RGBLED_BLUE:
		udef_trigger = led_color[1];
		break;
	case RGBLED_GREEN:
		udef_trigger = led_color[2];
		break;
	case RGBLED_RED:
		udef_trigger = led_color[3];
		break;
	case RGBLED_NIAGARA_BLUE:
		udef_trigger = led_color[4];
		break;
	case RGBLED_YELLOW:
		udef_trigger = led_color[5];
		break;
	case RGBLED_PURPLE:
		udef_trigger = led_color[6];
		break;
	case RGBLED_WHITE:
		udef_trigger = led_color[7];
		break;
	default:
		;
	}
	if ((c == RGBLED_CONNECTED || c == RGBLED_ETH_BACKHAUL)
	  && b == 0
#ifdef RTCONFIG_SW_CTRL_ALLLED
	  && ctrl_led_off
#else
	  && nvram_match("lp55xx_lp5523_user_enable", "1")
#endif
	)
		udef_trigger = led_color[0];

	switch (b) {
	case RGBLED_SBLINK:
		uidx = 1;
		break;
	case RGBLED_3ON1OFF:
		uidx = 2;
		break;
	case RGBLED_FBLINK:
		uidx = 3;
		break;
	case RGBLED_3ON3OFF:
		uidx = 4;
		break;
	default:
		;
	}

	if (b == RGBLED_ATE_MODE) {
		del_bled(extract_gpio_pin(main_led_gpio));
		__update_gpio_nv_var(main_led_gpio, 0);
	}
	else {
		set_bled_udef_pattern(main_led_gpio, u[uidx].interval, u[uidx].pattern);
		set_bled_udef_trigger(main_led_gpio, udef_trigger);
		set_bled_udef_pattern_mode(main_led_gpio);
	}
}
#elif defined(RTCONFIG_PWMX2_GPIOX1_RGBLED)
#define SYS_PWMCHIP	"/sys/class/pwm/pwmchip0"

struct pwm_conf {
	char type[8];
	char fint[8];
	char config[16];
};

static void set_pwm(int pwmx, struct pwm_conf pc)
{
	char pwmdev[sizeof(SYS_PWMCHIP"/pwmXXX")];

	snprintf(pwmdev, sizeof(pwmdev), "%s/pwm%d", SYS_PWMCHIP, pwmx);
	if (!d_exists(pwmdev))
		doSystem("echo %d > %s/export", pwmx, SYS_PWMCHIP);

	doSystem("echo 0 > %s/mm_enable", pwmdev);
	doSystem("echo 0 > %s/mm_fint", pwmdev);
	if (!strcmp(pc.type, "0"))
		return;

	doSystem("echo %s > %s/mm_type", pc.type, pwmdev);
	doSystem("echo %s > %s/mm_fint", pc.fint, pwmdev);
	doSystem("echo %s > %s/mm_config", pc.config, pwmdev);
	doSystem("echo 1 > %s/mm_enable", pwmdev);
}

void set_rgbled(unsigned int mode)
{
#if defined(TUFAX6000)
	int i;
	struct pwm_conf p[2][5] = {
		{	/* pwm0: Red */
			{      "0",   "0",       "0-0-0"},	/* off */
			/*{"1-128-0", "0-1", "0x3-625-625"},*/	/* QIS run (7 color breathing) */
			/*{"2-128-0",   "0", "0x3-500-500"},*/	/* default ready light (static yellow) */
			{"1-128-0",   "0", "0x3-750-750"},	/* AURA: water flow (booting light) */
			{"4-128-0", "0-1", "0x5-350-350"},	/* AURA: rainbow */
			{"X-XXX-X",   "0", "0xX-XXX-XXX"},	/* AURA: static/breathing */
			{      "3",   "0",  "0x7-5000-1"}	/* ATE mode (static red) */
		},
		{	/* pwm1: Green */
			{      "0",   "0",       "0-0-0"},	/* off */
			/*{  "1-0-0", "1-1", "0x3-625-625"},*/	/* QIS run (7 color breathing) */
			/*{ "2-28-0",   "0", "0x3-500-500"},*/	/* default ready light (static yellow) */
			{ "1-28-0",   "0", "0x3-750-750"},	/* AURA: water flow (booting light) */
			{ "4-32-0", "3-1", "0x5-350-350"},	/* AURA: rainbow */
			{"X-XXX-X",   "0", "0xX-XXX-XXX"},	/* AURA: static/breathing */
			{      "3",   "0",  "0x7-5000-1"}	/* ATE mode (static green) */
		}
	};

	switch (mode) {
	/* start ATE mode */
	case RGBLED_ATE_MODE:
		for (i = 0; i < 2; i++)
			set_pwm(i, p[i][4]);
		break;
	/* static blue */
	case RGBLED_DEFAULT_STANDBY:
		if (nvram_match("rgbled_qis_running", "1"))
			break;
		for (i = 0; i < 2; i++)
			set_pwm(i, p[i][0]);
		led_control(LED_BLUE, LED_ON);
		break;
	/* rainbow */
	case RGBLED_QIS_RUN:
		nvram_set("rgbled_qis_running", "1");
		for (i = 0; i < 2; i++)
			set_pwm(i, p[i][2]);
		led_control(LED_BLUE, LED_OFF);
		break;
	/* blinking blue on/off 0.5s */
	case RGBLED_QIS_FINISH:
		nvram_unset("rgbled_qis_running");
		for (i = 0; i < 2; i++)
			set_pwm(i, p[i][0]);
		for (i = 0; i < 3; i++) {
			led_control(LED_BLUE, LED_ON);
			usleep(500000);
			led_control(LED_BLUE, LED_OFF);
			usleep(500000);
		}
		/* fall through */
	/* AURA */
	case RGBLED_CONFIGURED_STANDBY:
		if (nvram_invmatch("ledg_scheme", "")) {
			char nv[16], buf[64];
			char *ptr = buf, *str = NULL;
			int ledg_scheme = nvram_get_int("ledg_scheme");
			int rgb[3] = { 0 }, rg0 = 0;
			int i = 0, pidx = 0;

			/* parse nvram ledg_rgbXXX */
			snprintf(nv, sizeof(nv), "ledg_rgb%d", ledg_scheme);
			strcpy(buf, nvram_safe_get(nv));
			while ((str = strsep(&ptr, ","))) {
				rgb[i] = atoi(str);
				if (++i > ARRAY_SIZE(rgb))
					break;
			}

			switch (ledg_scheme) {
			/* static */
			case LEDG_SCHEME_STEADY_RED:
			/* breathing */
			case LEDG_SCHEME_PULSATING:
				pidx = 3;
				for (i = 0; i < 2; i++) {
					if (rgb[2] && rgb[i] == 0 && rg0 == 0) {
						snprintf(p[i][pidx].type, sizeof(p[i][pidx].type), "%u-%u-0",
							ledg_scheme == LEDG_SCHEME_PULSATING ? 5 : 6,
							i == 0 ? rgb[2] / 4 : rgb[2] * 2 / 3);
						snprintf(p[i][pidx].config, sizeof(p[i][pidx].config), "%s",
							ledg_scheme == LEDG_SCHEME_PULSATING ? "0x0-40-40" : "0x0-20-20");
						snprintf(p[i][pidx].fint, sizeof(p[i][pidx].fint), "2-1");

						/* Avoid pwm0/1 to enable interrupt at the same time, it is not necessary */
						if (rgb[0] == 0 && rgb[1] == 0)
							rg0 = 1;
					}
					else {
						snprintf(p[i][pidx].type, sizeof(p[i][pidx].type), "%u-%u-0",
							ledg_scheme == LEDG_SCHEME_PULSATING ? 1 : 2,
							i == 0 ? rgb[i] : rgb[i] / 4);
						snprintf(p[i][pidx].config, sizeof(p[i][pidx].config), "%s",
							ledg_scheme == LEDG_SCHEME_PULSATING ? "0x3-625-625" : "0x3-500-500");
						snprintf(p[i][pidx].fint, sizeof(p[i][pidx].fint), "0");
					}
				}
				break;
			/* water flow */
			case LEDG_SCHEME_WATER_FLOW:
				pidx = 1;
				if (nvram_match("odmpid", "TX-AX6000")) {
					snprintf(p[0][pidx].type, sizeof(p[0][pidx].type), "5-11-0");
					snprintf(p[0][pidx].config, sizeof(p[0][pidx].config), "0x0-48-48");
					snprintf(p[0][pidx].fint, sizeof(p[0][pidx].fint), "2-1");
					snprintf(p[1][pidx].type, sizeof(p[1][pidx].type), "1-32-0");
				}
				break;
			/* rainbow */
			case LEDG_SCHEME_RAINBOW:
				pidx = 2;
				break;
			default:
				;
			}

			for (i = 0; i < 2; i++)
				set_pwm(i, p[i][nvram_match("AllLED", "0") ? 0 : pidx]);

			if (rgb[2] == 0 || nvram_match("AllLED", "0"))
				led_control(LED_BLUE, LED_OFF);
		}
		break;
	default:
		;
	}
#endif
}
#endif

#if defined(RTCONFIG_AMAS)
/*
 * LED blinking by bled
 * @led_nvram:		nvram string, e.g. "led_wps_gpio"
 * @led_id:		enum led_id, e.g. LED_WPS
 * @ifname:		interface name, e.g. sta0, apcli0
 * @interval:		blinking interval. (unit: ms)
 * @pattern:		user defined pattern, e.g. "1 0 1 1 0 0"
 */
void led_blinking_by_bled(char *led_nvram, int led_id, char *ifname,
			  unsigned int interval, char *pattern)
{
	if (nvram_match(led_nvram, "255")
	 || (!interval && !(nvram_get_int(led_nvram) & GPIO_BLINK_LED)))
		return;

	/* enable bled */
	if (interval && !(nvram_get_int(led_nvram) & GPIO_BLINK_LED)) {
		config_netdev_bled(led_nvram, ifname);
		led_control(led_id, LED_ON);
	}

	/* blinking */
	if (interval && pattern) {
		set_bled_udef_pattern(led_nvram, interval, pattern);
		set_bled_udef_trigger(led_nvram, "1");
		set_bled_udef_pattern_mode(led_nvram);
	}

	/* disable bled */
	if (!interval && (nvram_get_int(led_nvram) & GPIO_BLINK_LED)) {
		del_bled(extract_gpio_pin(led_nvram));
		__update_gpio_nv_var(led_nvram, 0);
	}
}
#endif
#endif	/* RTCONFIG_BLINK_LED */
