/*
 * DHD SROM MAP Module
 *
 * Copyright (C) 2022, Broadcom. All Rights Reserved.
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
 * $Id: dhd_srommap.c $
 */

#if defined(BCA_SROMMAP)

/* include files */
#include <typedefs.h>
#include <bcmutils.h>
#include <bcmdevs.h>
#include <siutils.h>
#include <pcie_core.h>
#include <dhd.h>
#include <dhd_bus.h>
#include <dhd_dbg.h>
#include <dhd_pcie.h>
#include <board.h>
#include <bcmnvram.h>
#include <bcmsromio.h>

extern int dhdpcie_downloadvars(dhd_bus_t *bus, void *arg, int len);

#if defined(BCMDBG)
void dump_vars(char *vars, int len)
{
	int j;
	for (j = 0; j < len; j++) {
		if (vars[j] == '\0')
			printf("(0)\n");
		else
			printf("%c", vars[j]);
	}
	printf("\n");
}
#endif

/* This function is to merge bus->vars to incoming memblock (remove dup vars).
 * bus->vars contains local vars, e.g. wlunit from _dhdpcie_get_nvram_params().
 * incoming memblock is nvram vars from initvars_srom_pci().
 */
int
dhd_vars_adjust(struct dhd_bus *bus, char *memblock, uint *total_len)
{
	int bcmerror = BCME_OK;
	uint len = *total_len;
	int i = len - 1;
	char *locbufp = bus->vars;
	int locbuf_len;
	char *mbufp = memblock;
	int mbuf_len;
	char *appbuf = NULL; /* append buf to store vars to be appended */
	char *appbufp = NULL;
	int appbuf_len = 0;

	if (bus->vars) {

#if defined(BCMDBG)
		/* dump bus->vars */
		printf("=> %s: bus->vars: [bus->varsz=%d]\n",
				__FUNCTION__, bus->varsz);
		dump_vars(bus->vars, bus->varsz);

		/* dump memblock */
		printf("=> %s: memblock: [len=%d]\n",
				__FUNCTION__, *total_len);
		dump_vars(memblock, *total_len);
#endif

		while (i >= 0 && memblock[i] == '\0')
			i--;
		len = i+2; /* to tail of memblock */

		/* allocate a tmp buf for storing vars to be appended */
		appbuf = MALLOCZ(bus->dhd->osh, MAX_NVRAMBUF_SIZE);
		if (appbuf == NULL) {
			DHD_ERROR(("%s: Failed to allocate memory %d bytes\n",
				__FUNCTION__, MAX_NVRAMBUF_SIZE));
			return BCME_ERROR;
		}
		appbufp = appbuf;

		locbufp = bcmstrtok(&locbufp, "\0", NULL);
		while (locbufp != NULL) {
			locbuf_len = strlen(locbufp) + 1; /* include '\0' */

			/* start from memblock */
			mbufp = memblock;

			/* compare to memblock */
			mbufp = bcmstrtok(&mbufp, "\0", NULL);
			while (mbufp != NULL) {
				mbuf_len = strlen(mbufp) + 1; /* include '\0' */

				/* compare var name till '=' sign */
				i = 0;
				while ((locbufp[i] != '=') &&
				       (locbufp[i] == mbufp[i]))
					i++;
				if ((locbufp[i] == '=') && (mbufp[i] == '=')) {
					/* var name matched, go to next var */
					goto next_var;
				}
				mbufp += mbuf_len;
				mbufp = bcmstrtok(&mbufp, "\0", NULL);
			}
			/* save the diff var to be appended */
			memcpy(appbufp, locbufp, locbuf_len);
			appbufp += locbuf_len;
			appbuf_len += locbuf_len;
next_var:
			locbufp += locbuf_len;
			locbufp = bcmstrtok(&locbufp, "\0", NULL);
		}

#if defined(BCMDBG)
		printf("=> %s: to be append vars: [len=%d]\n",
			__FUNCTION__, appbuf_len);
		dump_vars(appbuf, appbuf_len);
#endif
		if ((len + appbuf_len) < MAX_NVRAMBUF_SIZE) {
			memcpy(memblock+len, appbuf, appbuf_len);
			*total_len = len + appbuf_len;

#if defined(BCMDBG)
			printf("=> %s: adjusted memblock: [*total_len=%d]\n",
				__FUNCTION__, *total_len);
			dump_vars(memblock, *total_len);
#endif
		} else {
			DHD_ERROR(("%s:nvram size %d is bigger than max:%d\n",
				__FUNCTION__, len + appbuf_len,
				MAX_NVRAMBUF_SIZE));
			bcmerror = BCME_ERROR;
		}
		MFREE(bus->dhd->osh, appbuf, MAX_NVRAMBUF_SIZE);
	}
	return bcmerror;
}

void
dhd_get_cfe_mac(dhd_pub_t *dhd, struct ether_addr *mac)
{
	unsigned long ulId = (unsigned long)('w'<<24) +
			(unsigned long)('l'<<16) +
			dhd_get_instance(dhd);
	kerSysGetMacAddress(mac->octet, ulId);
}

/*
 *  Check and update mutxmax var for dongle.
 */
int
dhd_check_and_set_mutxmax(dhd_pub_t *dhd, char *memblock, uint* len)
{
#define MUTXMAXBUFLEN        12
	char *var = NULL;
	char mutxmaxbuf[MUTXMAXBUFLEN] = "mutxmax=";
	char *locbufp;
	int mutxmax;

	/* If mutxmax nvram is present, need to update the nvramvars */
	/* skip if it is set to auto mode (default) */
	var = nvram_get("mutxmax");
	if (var != NULL) {
		mutxmax = bcm_strtoul(var, NULL, 0);
		if (mutxmax != -1) {
			if (((*len) + 12) >= MAX_NVRAMBUF_SIZE) {
				DHD_ERROR(("%s: nvram size is too big \n",
						__FUNCTION__));
				return BCME_ERROR;
			}

			/* memblock delimiter is END\0 string */
			/* Move END by 12bytes to fit the mutxmax var */
			locbufp = memblock + (*len) - 4;
			memcpy(locbufp + MUTXMAXBUFLEN, locbufp, 4);

			/* append mutxmax var to the list */
			sprintf(mutxmaxbuf + strlen(mutxmaxbuf), "0x%1x",
					mutxmax);
			memcpy(locbufp, mutxmaxbuf, MUTXMAXBUFLEN);
			(*len) = (*len) + MUTXMAXBUFLEN;
		}
	}
	return BCME_OK;
}

int
dhd_check_and_set_mac(dhd_pub_t *dhd, char *memblock, uint* len)
{
	char *locbufp = memblock;
	const char macaddr_zero[] = "macaddr=00:00:00:00:00:00";
	const char macaddr_bcast[] = "macaddr=ff:ff:ff:ff:ff:ff";
	char macaddrbuf[26] = "macaddr=";
	int i, j;
	int appendMac = 1;
	int replaceMac = 0;
	struct ether_addr mac;

	for (i = 0; i < (*len); i++, locbufp++) {
		if (*locbufp == '\0')
			break;
		if (memcmp(locbufp, macaddrbuf, 7) == 0) {
			appendMac = 0;
			if ((memcmp(locbufp, macaddr_zero, 25) == 0) ||
				(memcmp(locbufp, macaddr_bcast, 25) == 0)) {
				/* if macaddr in sromvars is all-zeros or bcast,
				 * dongle won't be initialized successfully.
				 */
				replaceMac = 1;
			}
			break;
		}
		while (*locbufp != '\0') {
			i++;
			locbufp++;
		}
	}

	if (appendMac && (((*len) + 26) >= MAX_NVRAMBUF_SIZE)) {
		DHD_ERROR(("%s: nvram size is too big!\n", __FUNCTION__));
		return BCME_ERROR;
	} else if (appendMac || replaceMac) {
		printf("Replace or append with internal Mac Address.\n");
		dhd_get_cfe_mac(dhd, &mac);
		for (j = 0; j < (ETH_ALEN - 1); j++)
			sprintf(macaddrbuf + strlen(macaddrbuf), "%2.2X:",
					mac.octet[j]);
		sprintf(macaddrbuf + strlen(macaddrbuf), "%2.2X", mac.octet[j]);
		memcpy(memblock + i, macaddrbuf, 25);
		if (appendMac)
			(*len) = (*len) + 26;
	}
	return  BCME_OK;
}

int
dhdpcie_download_map_bin(struct dhd_bus *bus)
{
	int bcmerror = -1;
	void *regsva = (void*)bus->regs;
	char *memblock, *memblock_t = NULL;
	uint len = 0, tl = 0;

	osl_t *osh = si_osh(bus->sih);
	uint16 srom = 0;
	uint16 *sromwindow;

	sromwindow = (uint16 *)srom_offset(bus->sih, regsva);
	/* don't load srommap if SROM is available. */
	if (si_is_sprom_available(bus->sih) &&
	    !sprom_read_pci(osh, bus->sih, sromwindow, 0, &srom, 1, FALSE)) {
		return BCME_OK;
	}

	/* don't load srommap if external nvram is loaded */
	if (si_getdevpathvar(bus->sih, "sromrev") != NULL) {
		printf("%s: ext nvram exists, no srommap!\n", __FUNCTION__);
		return BCME_OK;
	}

	bcmerror = initvars_srom_pci(bus->sih, regsva, &memblock_t, &len);
	if ((bcmerror == BCME_OK) && len && (len < MAX_NVRAMBUF_SIZE)) {
		/* since memblock_t is not MAX_NVRAMBUF_SIZE,
		 * we need to copy it to the MAX holder.
		 */
		memblock = MALLOCZ(bus->dhd->osh, MAX_NVRAMBUF_SIZE);
		if (memblock == NULL) {
			DHD_ERROR(("%s: Failed to allocate memory %d bytes\n",
				__FUNCTION__, MAX_NVRAMBUF_SIZE));
			MFREE(bus->dhd->osh, memblock_t, len);
			return BCME_ERROR;
		}
		memcpy(memblock, memblock_t, len);
		MFREE(bus->dhd->osh, memblock_t, len);

		if ((bcmerror = dhd_vars_adjust(bus, memblock, &len)) ==
				BCME_OK) {
			if (!si_is_sprom_available(bus->sih) ||
				sprom_read_pci(osh, bus->sih, sromwindow, 0,
					&srom, 1, FALSE)) {
				bcmerror = dhd_check_and_set_mac(bus->dhd,
					memblock, &len);
			}
			if (bcmerror == BCME_OK) {
				bcmerror = dhd_check_and_set_mutxmax(bus->dhd,
					memblock, &len);
			}
			/* to make sure the vars are ended with \0 */
			tl = len & 0x3;
			if (tl) {
				tl = 4 - tl;
				len += tl;
				while (tl)
					memblock[len-1-(--tl)] = '\0';
			}
			bcmerror = dhdpcie_downloadvars(bus, memblock, len);
		}
		MFREE(bus->dhd->osh, memblock, MAX_NVRAMBUF_SIZE);
	} else
		DHD_ERROR(("%s download map bin failed\n", __FUNCTION__));

	return bcmerror;
}
#endif /* BCA_SROMMAP */
