/*
 * Customer code to add GPIO control during WLAN start/stop
 *
 * Copyright (C) 2023, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: dhd_custom_gpio.c 821234 2023-02-06 14:16:52Z $
 */

#include <typedefs.h>
#include <linuxver.h>
#include <osl.h>
#include <bcmutils.h>
#include <dngl_stats.h>
#include <dhd.h>
#include <dhd_linux.h>

#include <wlioctl.h>
#if defined(WL_WIRELESS_EXT)
#include <wl_iw.h>
#endif

#ifndef BCMDONGLEHOST
#include <wlc_pub.h>
#include <wl_dbg.h>
#else
#define WL_ERROR(x) printf x
#define WL_TRACE(x)
#endif

#if defined(OOB_INTR_ONLY) || defined(BCMSPI_ANDROID)

#if defined(BCMLXSDMMC)
extern int sdioh_mmc_irq(int irq);
#endif /* (BCMLXSDMMC)  */

#if defined(PLATFORM_MPS)
#include <mach/gpio.h>
#endif /* PLATFORM_MPS */

/* Customer specific Host GPIO defintion  */
static int dhd_oob_gpio_num = -1;

module_param(dhd_oob_gpio_num, int, 0644);
MODULE_PARM_DESC(dhd_oob_gpio_num, "DHD oob gpio number");

/* This function will return:
 *  1) return :  Host gpio interrupt number per customer platform
 *  2) irq_flags_ptr : Type of Host interrupt as Level or Edge
 *
 *  NOTE :
 *  Customer should check his platform definitions
 *  and his Host Interrupt spec
 *  to figure out the proper setting for his platform.
 *  Broadcom provides just reference settings as example.
 *
 */
int dhd_customer_oob_irq_map(void *adapter, unsigned long *irq_flags_ptr)
{
	int  host_oob_irq = 0;

#if defined(CUSTOM_OOB_GPIO_NUM)
	if (dhd_oob_gpio_num < 0) {
		dhd_oob_gpio_num = CUSTOM_OOB_GPIO_NUM;
	}
#endif /* CUSTOMER_OOB_GPIO_NUM */

	if (dhd_oob_gpio_num < 0) {
		WL_ERROR(("%s: ERROR customer specific Host GPIO is NOT defined \n",
		__FUNCTION__));
		return (dhd_oob_gpio_num);
	}

	WL_ERROR(("%s: customer specific Host GPIO number is (%d)\n",
		__FUNCTION__, dhd_oob_gpio_num));

	return (host_oob_irq);
}
#endif /* defined(OOB_INTR_ONLY) || defined(BCMSPI_ANDROID) */

/* Customer function to control hw specific wlan gpios */
int
dhd_customer_gpio_wlan_ctrl(void *adapter, int onoff)
{
	int err = 0;

	return err;
}

#ifdef GET_CUSTOM_MAC_ENABLE
/* Function to get custom MAC address */
int
dhd_custom_get_mac_address(void *adapter, unsigned char *buf)
{
	WL_TRACE(("%s Enter\n", __FUNCTION__));
	if (!buf)
		return -EINVAL;

#ifdef EXAMPLE_GET_MAC
	/* EXAMPLE code */
	{
		struct ether_addr ea_example = {{0x00, 0x11, 0x22, 0x33, 0x44, 0xFF}};
		bcopy((char *)&ea_example, buf, sizeof(struct ether_addr));
	}
#endif /* EXAMPLE_GET_MAC */

	return 0;
}
#endif /* GET_CUSTOM_MAC_ENABLE */

/* Customized Locale table : OPTIONAL feature */
const struct cntry_locales_custom translate_custom_table[] = {
/* Table should be filled out based on custom platform regulatory requirement */
#ifdef EXAMPLE_TABLE
	{"",   "XY", 4},  /* Universal if Country code is unknown or empty */
	{"US", "US", 69}, /* input ISO "US" to : US regrev 69 */
	{"CA", "US", 69}, /* input ISO "CA" to : US regrev 69 */
	{"EU", "EU", 5},  /* European union countries to : EU regrev 05 */
	{"AT", "EU", 5},
	{"BE", "EU", 5},
	{"BG", "EU", 5},
	{"CY", "EU", 5},
	{"CZ", "EU", 5},
	{"DK", "EU", 5},
	{"EE", "EU", 5},
	{"FI", "EU", 5},
	{"FR", "EU", 5},
	{"DE", "EU", 5},
	{"GR", "EU", 5},
	{"HU", "EU", 5},
	{"IE", "EU", 5},
	{"IT", "EU", 5},
	{"LV", "EU", 5},
	{"LI", "EU", 5},
	{"LT", "EU", 5},
	{"LU", "EU", 5},
	{"MT", "EU", 5},
	{"NL", "EU", 5},
	{"PL", "EU", 5},
	{"PT", "EU", 5},
	{"RO", "EU", 5},
	{"SK", "EU", 5},
	{"SI", "EU", 5},
	{"ES", "EU", 5},
	{"SE", "EU", 5},
	{"GB", "EU", 5},
	{"KR", "XY", 3},
	{"AU", "XY", 3},
	{"CN", "XY", 3}, /* input ISO "CN" to : XY regrev 03 */
	{"TW", "XY", 3},
	{"AR", "XY", 3},
	{"MX", "XY", 3},
	{"IL", "IL", 0},
	{"CH", "CH", 0},
	{"TR", "TR", 0},
	{"NO", "NO", 0},
#endif /* EXAMPLE_TABLE */
};

/* Customized Locale convertor
*  input : ISO 3166-1 country abbreviation
*  output: customized cspec
*/
void get_customized_country_code(void *adapter, char *country_iso_code, wl_country_t *cspec)
{
#if defined(OEM_ANDROID)
	int size, i;

	size = ARRAYSIZE(translate_custom_table);

	if (cspec == 0)
		 return;

	if (size == 0)
		 return;

	for (i = 0; i < size; i++) {
		if (strcmp(country_iso_code, translate_custom_table[i].iso_abbrev) == 0) {
			memcpy(cspec->ccode,
				translate_custom_table[i].custom_locale, WLC_CNTRY_BUF_SZ);
			cspec->rev = translate_custom_table[i].custom_locale_rev;
			return;
		}
	}
#ifdef EXAMPLE_TABLE
	/* if no country code matched return first universal code from translate_custom_table */
	memcpy(cspec->ccode, translate_custom_table[0].custom_locale, WLC_CNTRY_BUF_SZ);
	cspec->rev = translate_custom_table[0].custom_locale_rev;
#endif /* EXAMPLE_TABLE */
	return;
#endif /* OEM_ANDROID */
}
