/*
 * +--------------------------------------------------------------------------+
 * HND command definitions used and shared by WLAN external modules (DHD/WL)
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
 * $Id: hnd_linux.c 826923 2023-06-28 11:06:01Z $
 * +--------------------------------------------------------------------------+
 */

#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>
#ifdef BCM_ROUTER
#include <bcmpcie.h>
#endif
#if defined(MLO_IPC)
#include <mlo_ipc.h>
#endif /* MLO_IPC */

/*
 * +--------------------------------------------------------------------------+
 * HND_LINUX
 * Allocate and initialize data objects shared by WLAN modules (DHD/WL)
 * +--------------------------------------------------------------------------+
 */

#ifdef BCM_ROUTER
#define BCM_MAX_RADIO		(PCIE_IPC_AP_UNIT_MAX)

struct bcm_radio_unit {
	void *drvr;
	bcm_panic_cb_t panic_cb;
};

struct bcm_radio {
	struct bcm_radio_unit radio_unit[BCM_MAX_RADIO];
};

static struct bcm_radio bcm_radio_g;

void
bcm_radio_register(uint8 unit, void *drvr, bcm_panic_cb_t panic_cb)
{
	ASSERT(!bcm_radio_g.radio_unit[unit].drvr);
	bcm_radio_g.radio_unit[unit].drvr = drvr;
	bcm_radio_g.radio_unit[unit].panic_cb = panic_cb;
}

void
bcm_radio_unregister(uint8 unit, void *drvr)
{
	ASSERT(bcm_radio_g.radio_unit[unit].drvr == drvr);
	bcm_radio_g.radio_unit[unit].drvr = NULL;
	bcm_radio_g.radio_unit[unit].panic_cb = NULL;
}

void
bcm_radio_panic_hook(void)
{
	struct bcm_radio_unit *radio;
	uint8 unit;

	printf(">>>>>>>>>>> BROADCOM WLAN PANIC INFO <<<<<<<<<<<\n");
	for (unit = 0; unit < BCM_MAX_RADIO; unit++) {
		radio = &bcm_radio_g.radio_unit[unit];
		if (!radio->drvr)
			continue;

		printf("RADIO <%u>:\n", unit);

		if (radio->panic_cb)
			radio->panic_cb(radio->drvr);
		printf("================================================\n");
	}
}
#endif /* BCM_ROUTER */

int // Invoked from hnd_module_init()
hnd_linux_init(void)
{
	int ret = BCME_OK;

#if defined(MLO_IPC)
	ret = mlo_ipc_init();
#endif /* MLO_IPC */

	return ret;
} // hnd_linux_init()

void // Invoked from hnd_module_exit()
hnd_linux_exit(void)
{

#if defined(MLO_IPC)
	mlo_ipc_exit();
#endif /* MLO_IPC */

} // hnd_linux_exit()
