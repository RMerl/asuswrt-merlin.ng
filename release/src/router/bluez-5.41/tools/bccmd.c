/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/socket.h>
#include <unistd.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"
#include "lib/hci_lib.h"

#include "src/shared/tty.h"

#include "csr.h"

#define CSR_TRANSPORT_UNKNOWN	0
#define CSR_TRANSPORT_HCI	1
#define CSR_TRANSPORT_USB	2
#define CSR_TRANSPORT_BCSP	3
#define CSR_TRANSPORT_H4	4
#define CSR_TRANSPORT_3WIRE	5

#define CSR_STORES_PSI		(0x0001)
#define CSR_STORES_PSF		(0x0002)
#define CSR_STORES_PSROM	(0x0004)
#define CSR_STORES_PSRAM	(0x0008)
#define CSR_STORES_DEFAULT	(CSR_STORES_PSI | CSR_STORES_PSF)

#define CSR_TYPE_NULL		0
#define CSR_TYPE_COMPLEX	1
#define CSR_TYPE_UINT8		2
#define CSR_TYPE_UINT16		3
#define CSR_TYPE_UINT32		4

#define CSR_TYPE_ARRAY		CSR_TYPE_COMPLEX
#define CSR_TYPE_BDADDR		CSR_TYPE_COMPLEX

static inline int transport_open(int transport, char *device, speed_t bcsp_rate)
{
	switch (transport) {
	case CSR_TRANSPORT_HCI:
		return csr_open_hci(device);
	case CSR_TRANSPORT_USB:
		return csr_open_usb(device);
	case CSR_TRANSPORT_BCSP:
		return csr_open_bcsp(device, bcsp_rate);
	case CSR_TRANSPORT_H4:
		return csr_open_h4(device);
	case CSR_TRANSPORT_3WIRE:
		return csr_open_3wire(device);
	default:
		fprintf(stderr, "Unsupported transport\n");
		return -1;
	}
}

static inline int transport_read(int transport, uint16_t varid, uint8_t *value, uint16_t length)
{
	switch (transport) {
	case CSR_TRANSPORT_HCI:
		return csr_read_hci(varid, value, length);
	case CSR_TRANSPORT_USB:
		return csr_read_usb(varid, value, length);
	case CSR_TRANSPORT_BCSP:
		return csr_read_bcsp(varid, value, length);
	case CSR_TRANSPORT_H4:
		return csr_read_h4(varid, value, length);
	case CSR_TRANSPORT_3WIRE:
		return csr_read_3wire(varid, value, length);
	default:
		errno = EOPNOTSUPP;
		return -1;
	}
}

static inline int transport_write(int transport, uint16_t varid, uint8_t *value, uint16_t length)
{
	switch (transport) {
	case CSR_TRANSPORT_HCI:
		return csr_write_hci(varid, value, length);
	case CSR_TRANSPORT_USB:
		return csr_write_usb(varid, value, length);
	case CSR_TRANSPORT_BCSP:
		return csr_write_bcsp(varid, value, length);
	case CSR_TRANSPORT_H4:
		return csr_write_h4(varid, value, length);
	case CSR_TRANSPORT_3WIRE:
		return csr_write_3wire(varid, value, length);
	default:
		errno = EOPNOTSUPP;
		return -1;
	}
}

static inline void transport_close(int transport)
{
	switch (transport) {
	case CSR_TRANSPORT_HCI:
		csr_close_hci();
		break;
	case CSR_TRANSPORT_USB:
		csr_close_usb();
		break;
	case CSR_TRANSPORT_BCSP:
		csr_close_bcsp();
		break;
	case CSR_TRANSPORT_H4:
		csr_close_h4();
		break;
	case CSR_TRANSPORT_3WIRE:
		csr_close_3wire();
		break;
	}
}

static struct {
	uint16_t pskey;
	int type;
	int size;
	char *str;
} storage[] = {
	{ CSR_PSKEY_BDADDR,                   CSR_TYPE_BDADDR,  8,  "bdaddr"   },
	{ CSR_PSKEY_COUNTRYCODE,              CSR_TYPE_UINT16,  0,  "country"  },
	{ CSR_PSKEY_CLASSOFDEVICE,            CSR_TYPE_UINT32,  0,  "devclass" },
	{ CSR_PSKEY_ENC_KEY_LMIN,             CSR_TYPE_UINT16,  0,  "keymin"   },
	{ CSR_PSKEY_ENC_KEY_LMAX,             CSR_TYPE_UINT16,  0,  "keymax"   },
	{ CSR_PSKEY_LOCAL_SUPPORTED_FEATURES, CSR_TYPE_ARRAY,   8,  "features" },
	{ CSR_PSKEY_LOCAL_SUPPORTED_COMMANDS, CSR_TYPE_ARRAY,   18, "commands" },
	{ CSR_PSKEY_HCI_LMP_LOCAL_VERSION,    CSR_TYPE_UINT16,  0,  "version"  },
	{ CSR_PSKEY_LMP_REMOTE_VERSION,       CSR_TYPE_UINT8,   0,  "remver"   },
	{ CSR_PSKEY_HOSTIO_USE_HCI_EXTN,      CSR_TYPE_UINT16,  0,  "hciextn"  },
	{ CSR_PSKEY_HOSTIO_MAP_SCO_PCM,       CSR_TYPE_UINT16,  0,  "mapsco"   },
	{ CSR_PSKEY_UART_BAUDRATE,            CSR_TYPE_UINT16,  0,  "baudrate" },
	{ CSR_PSKEY_HOST_INTERFACE,           CSR_TYPE_UINT16,  0,  "hostintf" },
	{ CSR_PSKEY_ANA_FREQ,                 CSR_TYPE_UINT16,  0,  "anafreq"  },
	{ CSR_PSKEY_ANA_FTRIM,                CSR_TYPE_UINT16,  0,  "anaftrim" },
	{ CSR_PSKEY_USB_VENDOR_ID,            CSR_TYPE_UINT16,  0,  "usbvid"   },
	{ CSR_PSKEY_USB_PRODUCT_ID,           CSR_TYPE_UINT16,  0,  "usbpid"   },
	{ CSR_PSKEY_USB_DFU_PRODUCT_ID,       CSR_TYPE_UINT16,  0,  "dfupid"   },
	{ CSR_PSKEY_INITIAL_BOOTMODE,         CSR_TYPE_UINT16,  0,  "bootmode" },
	{ 0x0000 },
};

static char *storestostr(uint16_t stores)
{
	switch (stores) {
	case 0x0000:
		return "Default";
	case 0x0001:
		return "psi";
	case 0x0002:
		return "psf";
	case 0x0004:
		return "psrom";
	case 0x0008:
		return "psram";
	default:
		return "Unknown";
	}
}

static char *memorytostr(uint16_t type)
{
	switch (type) {
	case 0x0000:
		return "Flash memory";
	case 0x0001:
		return "EEPROM";
	case 0x0002:
		return "RAM (transient)";
	case 0x0003:
		return "ROM (or \"read-only\" flash memory)";
	default:
		return "Unknown";
	}
}

#define OPT_RANGE(min, max) \
		if (argc < (min)) { errno = EINVAL; return -1; } \
		if (argc > (max)) { errno = E2BIG; return -1; }

static struct option help_options[] = {
	{ "help",	0, 0, 'h' },
	{ 0, 0, 0, 0 }
};

static int opt_help(int argc, char *argv[], int *help)
{
	int opt;

	while ((opt=getopt_long(argc, argv, "+h", help_options, NULL)) != EOF) {
		switch (opt) {
		case 'h':
			if (help)
				*help = 1;
			break;
		}
	}

	return optind;
}

#define OPT_HELP(range, help) \
		opt_help(argc, argv, (help)); \
		argc -= optind; argv += optind; optind = 0; \
		OPT_RANGE((range), (range))

static int cmd_builddef(int transport, int argc, char *argv[])
{
	uint8_t array[8];
	uint16_t def = 0x0000, nextdef = 0x0000;
	int err = 0;

	OPT_HELP(0, NULL);

	printf("Build definitions:\n");

	while (1) {
		memset(array, 0, sizeof(array));
		array[0] = def & 0xff;
		array[1] = def >> 8;

		err = transport_read(transport, CSR_VARID_GET_NEXT_BUILDDEF, array, 8);
		if (err < 0)
			break;

		nextdef = array[2] | (array[3] << 8);

		if (nextdef == 0x0000)
			break;

		def = nextdef;

		printf("0x%04x - %s\n", def, csr_builddeftostr(def));
	}

	return err;
}

static int cmd_keylen(int transport, int argc, char *argv[])
{
	uint8_t array[8];
	uint16_t handle, keylen;
	int err;

	OPT_HELP(1, NULL);

	handle = atoi(argv[0]);

	memset(array, 0, sizeof(array));
	array[0] = handle & 0xff;
	array[1] = handle >> 8;

	err = transport_read(transport, CSR_VARID_CRYPT_KEY_LENGTH, array, 8);
	if (err < 0)
		return -1;

	keylen = array[2] | (array[3] << 8);

	printf("Crypt key length: %d bit\n", keylen * 8);

	return 0;
}

static int cmd_clock(int transport, int argc, char *argv[])
{
	uint8_t array[8];
	uint32_t clock;
	int err;

	OPT_HELP(0, NULL);

	memset(array, 0, sizeof(array));

	err = transport_read(transport, CSR_VARID_BT_CLOCK, array, 8);
	if (err < 0)
		return -1;

	clock = array[2] | (array[3] << 8) | (array[0] << 16) | (array[1] << 24);

	printf("Bluetooth clock: 0x%04x (%d)\n", clock, clock);

	return 0;
}

static int cmd_rand(int transport, int argc, char *argv[])
{
	uint8_t array[8];
	uint16_t rand;
	int err;

	OPT_HELP(0, NULL);

	memset(array, 0, sizeof(array));

	err = transport_read(transport, CSR_VARID_RAND, array, 8);
	if (err < 0)
		return -1;

	rand = array[0] | (array[1] << 8);

	printf("Random number: 0x%02x (%d)\n", rand, rand);

	return 0;
}

static int cmd_chiprev(int transport, int argc, char *argv[])
{
	uint8_t array[8];
	uint16_t rev;
	char *str;
	int err;

	OPT_HELP(0, NULL);

	memset(array, 0, sizeof(array));

	err = transport_read(transport, CSR_VARID_CHIPREV, array, 8);
	if (err < 0)
		return -1;

	rev = array[0] | (array[1] << 8);

	switch (rev) {
	case 0x64:
		str = "BC1 ES";
		break;
	case 0x65:
		str = "BC1";
		break;
	case 0x89:
		str = "BC2-External A";
		break;
	case 0x8a:
		str = "BC2-External B";
		break;
	case 0x28:
		str = "BC2-ROM";
		break;
	case 0x43:
		str = "BC3-Multimedia";
		break;
	case 0x15:
		str = "BC3-ROM";
		break;
	case 0xe2:
		str = "BC3-Flash";
		break;
	case 0x26:
		str = "BC4-External";
		break;
	case 0x30:
		str = "BC4-ROM";
		break;
	default:
		str = "NA";
		break;
	}

	printf("Chip revision: 0x%04x (%s)\n", rev, str);

	return 0;
}

static int cmd_buildname(int transport, int argc, char *argv[])
{
	uint8_t array[131];
	char name[64];
	unsigned int i;
	int err;

	OPT_HELP(0, NULL);

	memset(array, 0, sizeof(array));

	err = transport_read(transport, CSR_VARID_READ_BUILD_NAME, array, 128);
	if (err < 0)
		return -1;

	for (i = 0; i < sizeof(name); i++)
		name[i] = array[(i * 2) + 4];

	printf("Build name: %s\n", name);

	return 0;
}

static int cmd_panicarg(int transport, int argc, char *argv[])
{
	uint8_t array[8];
	uint16_t error;
	int err;

	OPT_HELP(0, NULL);

	memset(array, 0, sizeof(array));

	err = transport_read(transport, CSR_VARID_PANIC_ARG, array, 8);
	if (err < 0)
		return -1;

	error = array[0] | (array[1] << 8);

	printf("Panic code: 0x%02x (%s)\n", error,
					error < 0x100 ? "valid" : "invalid");

	return 0;
}

static int cmd_faultarg(int transport, int argc, char *argv[])
{
	uint8_t array[8];
	uint16_t error;
	int err;

	OPT_HELP(0, NULL);

	memset(array, 0, sizeof(array));

	err = transport_read(transport, CSR_VARID_FAULT_ARG, array, 8);
	if (err < 0)
		return -1;

	error = array[0] | (array[1] << 8);

	printf("Fault code: 0x%02x (%s)\n", error,
					error < 0x100 ? "valid" : "invalid");

	return 0;
}

static int cmd_coldreset(int transport, int argc, char *argv[])
{
	return transport_write(transport, CSR_VARID_COLD_RESET, NULL, 0);
}

static int cmd_warmreset(int transport, int argc, char *argv[])
{
	return transport_write(transport, CSR_VARID_WARM_RESET, NULL, 0);
}

static int cmd_disabletx(int transport, int argc, char *argv[])
{
	return transport_write(transport, CSR_VARID_DISABLE_TX, NULL, 0);
}

static int cmd_enabletx(int transport, int argc, char *argv[])
{
	return transport_write(transport, CSR_VARID_ENABLE_TX, NULL, 0);
}

static int cmd_singlechan(int transport, int argc, char *argv[])
{
	uint8_t array[8];
	uint16_t channel;

	OPT_HELP(1, NULL);

	channel = atoi(argv[0]);

	if (channel > 2401 && channel < 2481)
		channel -= 2402;

	if (channel > 78) {
		errno = EINVAL;
		return -1;
	}

	memset(array, 0, sizeof(array));
	array[0] = channel & 0xff;
	array[1] = channel >> 8;

	return transport_write(transport, CSR_VARID_SINGLE_CHAN, array, 8);
}

static int cmd_hoppingon(int transport, int argc, char *argv[])
{
	return transport_write(transport, CSR_VARID_HOPPING_ON, NULL, 0);
}

static int cmd_rttxdata1(int transport, int argc, char *argv[])
{
	uint8_t array[8];
	uint16_t freq, level;

	OPT_HELP(2, NULL);

	freq = atoi(argv[0]);

	if (!strncasecmp(argv[1], "0x", 2))
		level = strtol(argv[1], NULL, 16);
	else
		level = atoi(argv[1]);

	memset(array, 0, sizeof(array));
	array[0] = 0x04;
	array[1] = 0x00;
	array[2] = freq & 0xff;
	array[3] = freq >> 8;
	array[4] = level & 0xff;
	array[5] = level >> 8;

	return transport_write(transport, CSR_VARID_RADIOTEST, array, 8);
}

static int cmd_radiotest(int transport, int argc, char *argv[])
{
	uint8_t array[8];
	uint16_t freq, level, test;

	OPT_HELP(3, NULL);

	freq = atoi(argv[0]);

	if (!strncasecmp(argv[1], "0x", 2))
		level = strtol(argv[1], NULL, 16);
	else
		level = atoi(argv[1]);

	test = atoi(argv[2]);

	memset(array, 0, sizeof(array));
	array[0] = test & 0xff;
	array[1] = test >> 8;
	array[2] = freq & 0xff;
	array[3] = freq >> 8;
	array[4] = level & 0xff;
	array[5] = level >> 8;

	return transport_write(transport, CSR_VARID_RADIOTEST, array, 8);
}

static int cmd_memtypes(int transport, int argc, char *argv[])
{
	uint8_t array[8];
	uint16_t type, stores[4] = { 0x0001, 0x0002, 0x0004, 0x0008 };
	int i, err;

	OPT_HELP(0, NULL);

	for (i = 0; i < 4; i++) {
		memset(array, 0, sizeof(array));
		array[0] = stores[i] & 0xff;
		array[1] = stores[i] >> 8;

		err = transport_read(transport, CSR_VARID_PS_MEMORY_TYPE, array, 8);
		if (err < 0)
			continue;

		type = array[2] + (array[3] << 8);

		printf("%s (0x%04x) = %s (%d)\n", storestostr(stores[i]),
					stores[i], memorytostr(type), type);
	}

	return 0;
}

static struct option pskey_options[] = {
	{ "stores",	1, 0, 's' },
	{ "reset",	0, 0, 'r' },
	{ "help",	0, 0, 'h' },
	{ 0, 0, 0, 0 }
};

static int opt_pskey(int argc, char *argv[], uint16_t *stores, int *reset, int *help)
{
	int opt;

	while ((opt=getopt_long(argc, argv, "+s:rh", pskey_options, NULL)) != EOF) {
		switch (opt) {
		case 's':
			if (!stores)
				break;
			if (!strcasecmp(optarg, "default"))
				*stores = 0x0000;
			else if (!strcasecmp(optarg, "implementation"))
				*stores = 0x0001;
			else if (!strcasecmp(optarg, "factory"))
				*stores = 0x0002;
			else if (!strcasecmp(optarg, "rom"))
				*stores = 0x0004;
			else if (!strcasecmp(optarg, "ram"))
				*stores = 0x0008;
			else if (!strcasecmp(optarg, "psi"))
				*stores = 0x0001;
			else if (!strcasecmp(optarg, "psf"))
				*stores = 0x0002;
			else if (!strcasecmp(optarg, "psrom"))
				*stores = 0x0004;
			else if (!strcasecmp(optarg, "psram"))
				*stores = 0x0008;
			else if (!strncasecmp(optarg, "0x", 2))
				*stores = strtol(optarg, NULL, 16);
			else
				*stores = atoi(optarg);
			break;

		case 'r':
			if (reset)
				*reset = 1;
			break;

		case 'h':
			if (help)
				*help = 1;
			break;
		}
	}

	return optind;
}

#define OPT_PSKEY(min, max, stores, reset, help) \
		opt_pskey(argc, argv, (stores), (reset), (help)); \
		argc -= optind; argv += optind; optind = 0; \
		OPT_RANGE((min), (max))

static int cmd_psget(int transport, int argc, char *argv[])
{
	uint8_t array[128];
	uint16_t pskey, length, value, stores = CSR_STORES_DEFAULT;
	uint32_t val32;
	int i, err, reset = 0;

	memset(array, 0, sizeof(array));

	OPT_PSKEY(1, 1, &stores, &reset, NULL);

	if (strncasecmp(argv[0], "0x", 2)) {
		pskey = atoi(argv[0]);

		for (i = 0; storage[i].pskey; i++) {
			if (strcasecmp(storage[i].str, argv[0]))
				continue;

			pskey = storage[i].pskey;
			break;
		}
	} else
		pskey = strtol(argv[0] + 2, NULL, 16);

	memset(array, 0, sizeof(array));
	array[0] = pskey & 0xff;
	array[1] = pskey >> 8;
	array[2] = stores & 0xff;
	array[3] = stores >> 8;

	err = transport_read(transport, CSR_VARID_PS_SIZE, array, 8);
	if (err < 0)
		return err;

	length = array[2] + (array[3] << 8);
	if (length + 6 > (int) sizeof(array) / 2)
		return -EIO;

	memset(array, 0, sizeof(array));
	array[0] = pskey & 0xff;
	array[1] = pskey >> 8;
	array[2] = length & 0xff;
	array[3] = length >> 8;
	array[4] = stores & 0xff;
	array[5] = stores >> 8;

	err = transport_read(transport, CSR_VARID_PS, array, (length + 3) * 2);
	if (err < 0)
		return err;

	switch (length) {
	case 1:
		value = array[6] | (array[7] << 8);
		printf("%s: 0x%04x (%d)\n", csr_pskeytostr(pskey), value, value);
		break;

	case 2:
		val32 = array[8] | (array[9] << 8) | (array[6] << 16) | (array[7] << 24);
		printf("%s: 0x%08x (%d)\n", csr_pskeytostr(pskey), val32, val32);
		break;

	default:
		printf("%s:", csr_pskeytostr(pskey));
		for (i = 0; i < length; i++)
			printf(" 0x%02x%02x", array[(i * 2) + 6], array[(i * 2) + 7]);
		printf("\n");
		break;
	}

	if (reset)
		transport_write(transport, CSR_VARID_WARM_RESET, NULL, 0);

	return err;
}

static int cmd_psset(int transport, int argc, char *argv[])
{
	uint8_t array[128];
	uint16_t pskey, length, value, stores = CSR_STORES_PSRAM;
	uint32_t val32;
	int i, err, reset = 0;

	memset(array, 0, sizeof(array));

	OPT_PSKEY(2, 81, &stores, &reset, NULL);

	if (strncasecmp(argv[0], "0x", 2)) {
		pskey = atoi(argv[0]);

		for (i = 0; storage[i].pskey; i++) {
			if (strcasecmp(storage[i].str, argv[0]))
				continue;

			pskey = storage[i].pskey;
			break;
		}
	} else
		pskey = strtol(argv[0] + 2, NULL, 16);

	memset(array, 0, sizeof(array));
	array[0] = pskey & 0xff;
	array[1] = pskey >> 8;
	array[2] = stores & 0xff;
	array[3] = stores >> 8;

	err = transport_read(transport, CSR_VARID_PS_SIZE, array, 8);
	if (err < 0)
		return err;

	length = array[2] + (array[3] << 8);
	if (length + 6 > (int) sizeof(array) / 2)
		return -EIO;

	memset(array, 0, sizeof(array));
	array[0] = pskey & 0xff;
	array[1] = pskey >> 8;
	array[2] = length & 0xff;
	array[3] = length >> 8;
	array[4] = stores & 0xff;
	array[5] = stores >> 8;

	argc--;
	argv++;

	switch (length) {
	case 1:
		if (argc != 1) {
			errno = E2BIG;
			return -1;
		}

		if (!strncasecmp(argv[0], "0x", 2))
			value = strtol(argv[0] + 2, NULL, 16);
		else
			value = atoi(argv[0]);

		array[6] = value & 0xff;
		array[7] = value >> 8;
		break;

	case 2:
		if (argc != 1) {
			errno = E2BIG;
			return -1;
		}

		if (!strncasecmp(argv[0], "0x", 2))
			val32 = strtol(argv[0] + 2, NULL, 16);
		else
			val32 = atoi(argv[0]);

		array[6] = (val32 & 0xff0000) >> 16;
		array[7] = val32 >> 24;
		array[8] = val32 & 0xff;
		array[9] = (val32 & 0xff00) >> 8;
		break;

	default:
		if (argc != length * 2) {
			errno = EINVAL;
			return -1;
		}

		for (i = 0; i < length * 2; i++)
			if (!strncasecmp(argv[0], "0x", 2))
				array[i + 6] = strtol(argv[i] + 2, NULL, 16);
			else
				array[i + 6] = atoi(argv[i]);
		break;
	}

	err = transport_write(transport, CSR_VARID_PS, array, (length + 3) * 2);
	if (err < 0)
		return err;

	if (reset)
		transport_write(transport, CSR_VARID_WARM_RESET, NULL, 0);

	return err;
}

static int cmd_psclr(int transport, int argc, char *argv[])
{
	uint8_t array[8];
	uint16_t pskey, stores = CSR_STORES_PSRAM;
	int i, err, reset = 0;

	OPT_PSKEY(1, 1, &stores, &reset, NULL);

	if (strncasecmp(argv[0], "0x", 2)) {
		pskey = atoi(argv[0]);

		for (i = 0; storage[i].pskey; i++) {
			if (strcasecmp(storage[i].str, argv[0]))
				continue;

			pskey = storage[i].pskey;
			break;
		}
	} else
		pskey = strtol(argv[0] + 2, NULL, 16);

	memset(array, 0, sizeof(array));
	array[0] = pskey & 0xff;
	array[1] = pskey >> 8;
	array[2] = stores & 0xff;
	array[3] = stores >> 8;

	err = transport_write(transport, CSR_VARID_PS_CLR_STORES, array, 8);
	if (err < 0)
		return err;

	if (reset)
		transport_write(transport, CSR_VARID_WARM_RESET, NULL, 0);

	return err;
}

static int cmd_pslist(int transport, int argc, char *argv[])
{
	uint8_t array[8];
	uint16_t pskey = 0x0000, length, stores = CSR_STORES_DEFAULT;
	int err, reset = 0;

	OPT_PSKEY(0, 0, &stores, &reset, NULL);

	while (1) {
		memset(array, 0, sizeof(array));
		array[0] = pskey & 0xff;
		array[1] = pskey >> 8;
		array[2] = stores & 0xff;
		array[3] = stores >> 8;

		err = transport_read(transport, CSR_VARID_PS_NEXT, array, 8);
		if (err < 0)
			break;

		pskey = array[4] + (array[5] << 8);
		if (pskey == 0x0000)
			break;

		memset(array, 0, sizeof(array));
		array[0] = pskey & 0xff;
		array[1] = pskey >> 8;
		array[2] = stores & 0xff;
		array[3] = stores >> 8;

		err = transport_read(transport, CSR_VARID_PS_SIZE, array, 8);
		if (err < 0)
			continue;

		length = array[2] + (array[3] << 8);

		printf("0x%04x - %s (%d bytes)\n", pskey,
					csr_pskeytostr(pskey), length * 2);
	}

	if (reset)
		transport_write(transport, CSR_VARID_WARM_RESET, NULL, 0);

	return 0;
}

static int cmd_psread(int transport, int argc, char *argv[])
{
	uint8_t array[256];
	uint16_t pskey = 0x0000, length, stores = CSR_STORES_DEFAULT;
	char *str, val[7];
	int i, err, reset = 0;

	OPT_PSKEY(0, 0, &stores, &reset, NULL);

	while (1) {
		memset(array, 0, sizeof(array));
		array[0] = pskey & 0xff;
		array[1] = pskey >> 8;
		array[2] = stores & 0xff;
		array[3] = stores >> 8;

		err = transport_read(transport, CSR_VARID_PS_NEXT, array, 8);
		if (err < 0)
			break;

		pskey = array[4] + (array[5] << 8);
		if (pskey == 0x0000)
			break;

		memset(array, 0, sizeof(array));
		array[0] = pskey & 0xff;
		array[1] = pskey >> 8;
		array[2] = stores & 0xff;
		array[3] = stores >> 8;

		err = transport_read(transport, CSR_VARID_PS_SIZE, array, 8);
		if (err < 0)
			continue;

		length = array[2] + (array[3] << 8);
		if (length + 6 > (int) sizeof(array) / 2)
			continue;

		memset(array, 0, sizeof(array));
		array[0] = pskey & 0xff;
		array[1] = pskey >> 8;
		array[2] = length & 0xff;
		array[3] = length >> 8;
		array[4] = stores & 0xff;
		array[5] = stores >> 8;

		err = transport_read(transport, CSR_VARID_PS, array, (length + 3) * 2);
		if (err < 0)
			continue;

		str = csr_pskeytoval(pskey);
		if (!strcasecmp(str, "UNKNOWN")) {
			sprintf(val, "0x%04x", pskey);
			str = NULL;
		}

		printf("// %s%s\n&%04x =", str ? "PSKEY_" : "",
						str ? str : val, pskey);
		for (i = 0; i < length; i++)
			printf(" %02x%02x", array[(i * 2) + 7], array[(i * 2) + 6]);
		printf("\n");
	}

	if (reset)
		transport_write(transport, CSR_VARID_WARM_RESET, NULL, 0);

	return 0;
}

static int cmd_psload(int transport, int argc, char *argv[])
{
	uint8_t array[256];
	uint16_t pskey, length, size, stores = CSR_STORES_PSRAM;
	char *str, val[7];
	int err, reset = 0;

	OPT_PSKEY(1, 1, &stores, &reset, NULL);

	psr_read(argv[0]);

	memset(array, 0, sizeof(array));
	size = sizeof(array) - 6;

	while (psr_get(&pskey, array + 6, &size) == 0) {
		str = csr_pskeytoval(pskey);
		if (!strcasecmp(str, "UNKNOWN")) {
			sprintf(val, "0x%04x", pskey);
			str = NULL;
		}

		printf("Loading %s%s ... ", str ? "PSKEY_" : "",
							str ? str : val);
		fflush(stdout);

		length = size / 2;

		array[0] = pskey & 0xff;
		array[1] = pskey >> 8;
		array[2] = length & 0xff;
		array[3] = length >> 8;
		array[4] = stores & 0xff;
		array[5] = stores >> 8;

		err = transport_write(transport, CSR_VARID_PS, array, size + 6);

		printf("%s\n", err < 0 ? "failed" : "done");

		memset(array, 0, sizeof(array));
		size = sizeof(array) - 6;
	}

	if (reset)
		transport_write(transport, CSR_VARID_WARM_RESET, NULL, 0);

	return 0;
}

static int cmd_pscheck(int transport, int argc, char *argv[])
{
	uint8_t array[256];
	uint16_t pskey, size;
	int i;

	OPT_HELP(1, NULL);

	psr_read(argv[0]);

	while (psr_get(&pskey, array, &size) == 0) {
		printf("0x%04x =", pskey);
		for (i = 0; i < size; i++)
			printf(" 0x%02x", array[i]);
		printf("\n");
	}

	return 0;
}

static int cmd_adc(int transport, int argc, char *argv[])
{
	uint8_t array[8];
	uint16_t mux, value;
	int err;

	OPT_HELP(1, NULL);

	if (!strncasecmp(argv[0], "0x", 2))
		mux = strtol(argv[0], NULL, 16);
	else
		mux = atoi(argv[0]);

	/* Request an ADC read from a particular mux'ed input */
	memset(array, 0, sizeof(array));
	array[0] = mux & 0xff;
	array[1] = mux >> 8;

	err = transport_write(transport, CSR_VARID_ADC, array, 2);
	if (err < 0) {
		errno = -err;
		return -1;
	}

	/* have to wait a short while, then read result */
	usleep(50000);
	err = transport_read(transport, CSR_VARID_ADC_RES, array, 8);
	if (err < 0) {
		errno = -err;
		return -1;
	}

	mux = array[0] | (array[1] << 8);
	value = array[4] | (array[5] << 8);

	printf("ADC value from Mux 0x%02x : 0x%04x (%s)\n", mux, value,
					array[2] == 1 ? "valid" : "invalid");

	return 0;
}

static struct {
	char *str;
	int (*func)(int transport, int argc, char *argv[]);
	char *arg;
	char *doc;
} commands[] = {
	{ "builddef",  cmd_builddef,  "",                    "Get build definitions"          },
	{ "keylen",    cmd_keylen,    "<handle>",            "Get current crypt key length"   },
	{ "clock",     cmd_clock,     "",                    "Get local Bluetooth clock"      },
	{ "rand",      cmd_rand,      "",                    "Get random number"              },
	{ "chiprev",   cmd_chiprev,   "",                    "Get chip revision"              },
	{ "buildname", cmd_buildname, "",                    "Get the full build name"        },
	{ "panicarg",  cmd_panicarg,  "",                    "Get panic code argument"        },
	{ "faultarg",  cmd_faultarg,  "",                    "Get fault code argument"        },
	{ "coldreset", cmd_coldreset, "",                    "Perform cold reset"             },
	{ "warmreset", cmd_warmreset, "",                    "Perform warm reset"             },
	{ "disabletx", cmd_disabletx, "",                    "Disable TX on the device"       },
	{ "enabletx",  cmd_enabletx,  "",                    "Enable TX on the device"        },
	{ "singlechan",cmd_singlechan,"<channel>",           "Lock radio on specific channel" },
	{ "hoppingon", cmd_hoppingon, "",                    "Revert to channel hopping"      },
	{ "rttxdata1", cmd_rttxdata1, "<freq> <level>",      "TXData1 radio test"             },
	{ "radiotest", cmd_radiotest, "<freq> <level> <id>", "Run radio tests"                },
	{ "memtypes",  cmd_memtypes,  NULL,                  "Get memory types"               },
	{ "psget",     cmd_psget,     "<key>",               "Get value for PS key"           },
	{ "psset",     cmd_psset,     "<key> <value>",       "Set value for PS key"           },
	{ "psclr",     cmd_psclr,     "<key>",               "Clear value for PS key"         },
	{ "pslist",    cmd_pslist,    NULL,                  "List all PS keys"               },
	{ "psread",    cmd_psread,    NULL,                  "Read all PS keys"               },
	{ "psload",    cmd_psload,    "<file>",              "Load all PS keys from PSR file" },
	{ "pscheck",   cmd_pscheck,   "<file>",              "Check PSR file"                 },
	{ "adc",       cmd_adc,       "<mux>",               "Read ADC value of <mux> input"  },
	{ NULL }
};

static void usage(void)
{
	int i, pos = 0;

	printf("bccmd - Utility for the CSR BCCMD interface\n\n");
	printf("Usage:\n"
		"\tbccmd [options] <command>\n\n");

	printf("Options:\n"
		"\t-t <transport>     Select the transport\n"
		"\t-d <device>        Select the device\n"
		"\t-b <bcsp rate>     Select the bcsp transfer rate\n"
		"\t-h, --help         Display help\n"
		"\n");

	printf("Transports:\n"
		"\tHCI USB BCSP H4 3WIRE\n\n");

	printf("Commands:\n");
	for (i = 0; commands[i].str; i++)
		printf("\t%-10s %-20s\t%s\n", commands[i].str,
		commands[i].arg ? commands[i].arg : " ",
		commands[i].doc);
	printf("\n");

	printf("Keys:\n\t");
	for (i = 0; storage[i].pskey; i++) {
		printf("%s ", storage[i].str);
		pos += strlen(storage[i].str) + 1;
		if (pos > 60) {
			printf("\n\t");
			pos = 0;
		}
	}
	printf("\n");
}

static struct option main_options[] = {
	{ "transport",	1, 0, 't' },
	{ "device",	1, 0, 'd' },
	{ "bcsprate", 1, 0, 'b'},
	{ "help",	0, 0, 'h' },
	{ 0, 0, 0, 0 }
};

int main(int argc, char *argv[])
{
	char *device = NULL;
	int i, err, opt, transport = CSR_TRANSPORT_HCI;
	speed_t bcsp_rate = B38400;

	while ((opt=getopt_long(argc, argv, "+t:d:i:b:h", main_options, NULL)) != EOF) {
		switch (opt) {
		case 't':
			if (!strcasecmp(optarg, "hci"))
				transport = CSR_TRANSPORT_HCI;
			else if (!strcasecmp(optarg, "usb"))
				transport = CSR_TRANSPORT_USB;
			else if (!strcasecmp(optarg, "bcsp"))
				transport = CSR_TRANSPORT_BCSP;
			else if (!strcasecmp(optarg, "h4"))
				transport = CSR_TRANSPORT_H4;
			else if (!strcasecmp(optarg, "h5"))
				transport = CSR_TRANSPORT_3WIRE;
			else if (!strcasecmp(optarg, "3wire"))
				transport = CSR_TRANSPORT_3WIRE;
			else if (!strcasecmp(optarg, "twutl"))
				transport = CSR_TRANSPORT_3WIRE;
			else
				transport = CSR_TRANSPORT_UNKNOWN;
			break;

		case 'd':
		case 'i':
			device = strdup(optarg);
			break;
		case 'b':
			bcsp_rate = tty_get_speed(atoi(optarg));
			if (!bcsp_rate) {
				printf("Unknown BCSP baud rate specified, defaulting to 38400bps\n");
				bcsp_rate = B38400;
			}
			break;
		case 'h':
		default:
			usage();
			exit(0);
		}
	}

	argc -= optind;
	argv += optind;
	optind = 0;

	if (argc < 1) {
		usage();
		exit(1);
	}

	if (transport_open(transport, device, bcsp_rate) < 0)
		exit(1);

	free(device);

	for (i = 0; commands[i].str; i++) {
		if (strcasecmp(commands[i].str, argv[0]))
			continue;

		err = commands[i].func(transport, argc, argv);

		transport_close(transport);

		if (err < 0) {
			fprintf(stderr, "Can't execute command: %s (%d)\n",
							strerror(errno), errno);
			exit(1);
		}

		exit(0);
	}

	fprintf(stderr, "Unsupported command\n");

	transport_close(transport);

	exit(1);
}
