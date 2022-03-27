/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2004-2011  Marcel Holtmann <marcel@holtmann.org>
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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

#define CSR_U8(frm)  (p_get_u8(frm))
#define CSR_U16(frm) (btohs(htons(p_get_u16(frm))))
#define CSR_U32(frm) ((CSR_U16(frm) << 16) + CSR_U16(frm))
#define CSR_S16(frm) (btohs(htons(p_get_u16(frm))))

static char *type2str(uint16_t type)
{
	switch (type) {
	case 0x0000:
		return "Get req";
	case 0x0001:
		return "Get rsp";
	case 0x0002:
		return "Set req";
	default:
		return "Reserved";
	}
}

static inline void valueless_dump(int level, char *str, struct frame *frm)
{
	p_indent(level, frm);
	printf("%s\n", str);
}

static inline void complex_dump(int level, char *str, struct frame *frm)
{
	p_indent(level, frm);
	printf("%s\n", str);

	raw_dump(level, frm);
}

static inline void bool_dump(int level, char *str, struct frame *frm)
{
	uint16_t value;

	value = CSR_U16(frm);

	p_indent(level, frm);
	printf("%s: value %s (%d)\n", str, value ? "TRUE" : "FALSE", value);
}

static inline void int8_dump(int level, char *str, struct frame *frm)
{
	int16_t value;

	value = CSR_S16(frm);

	p_indent(level, frm);
	printf("%s: value %d (0x%2.2x)\n", str, value, value);
}

static inline void int16_dump(int level, char *str, struct frame *frm)
{
	int16_t value;

	value = CSR_S16(frm);

	p_indent(level, frm);
	printf("%s: value %d (0x%2.2x)\n", str, value, value);
}

static inline void uint16_dump(int level, char *str, struct frame *frm)
{
	uint16_t value;

	value = CSR_U16(frm);

	p_indent(level, frm);
	printf("%s: value %d (0x%4.4x)\n", str, value, value);
}

static inline void uint32_dump(int level, char *str, struct frame *frm)
{
	uint32_t value;

	value = CSR_U32(frm);

	p_indent(level, frm);
	printf("%s: value %d (0x%4.4x)\n", str, value, value);
}

static inline void bdaddr_dump(int level, char *str, struct frame *frm)
{
	char addr[18];

	p_ba2str(frm->ptr, addr);

	p_indent(level, frm);
	printf("%s: bdaddr %s\n", str, addr);
}

static inline void features_dump(int level, char *str, struct frame *frm)
{
	unsigned char features[8];
	int i;

	memcpy(features, frm->ptr, 8);

	p_indent(level, frm);
	printf("%s: features", str);
	for (i = 0; i < 8; i++)
		printf(" 0x%02x", features[i]);
	printf("\n");
}

static inline void commands_dump(int level, char *str, struct frame *frm)
{
	unsigned char commands[64];
	unsigned int i;

	memcpy(commands, frm->ptr, frm->len);

	p_indent(level, frm);
	printf("%s: commands", str);
	for (i = 0; i < frm->len; i++)
		printf(" 0x%02x", commands[i]);
	printf("\n");
}

static inline void handle_length_dump(int level, char *str, struct frame *frm)
{
	uint16_t handle, length;

	handle = CSR_U16(frm);
	length = CSR_U16(frm);

	p_indent(level, frm);
	printf("%s: handle %d length %d\n", str, handle, length);
}

static inline void handle_clock_dump(int level, char *str, struct frame *frm)
{
	uint16_t handle;
	uint32_t clock;

	handle = CSR_U16(frm);
	clock  = CSR_U32(frm);

	p_indent(level, frm);
	printf("%s: handle %d clock 0x%4.4x\n", str, handle, clock);
}

static inline void radiotest_dump(int level, char *str, struct frame *frm)
{
	uint16_t testid;

	testid = CSR_U16(frm);

	p_indent(level, frm);
	printf("%s: test id %d\n", str, testid);

	raw_dump(level, frm);
}

static inline void psmemtype_dump(int level, char *str, struct frame *frm)
{
	uint16_t store, type;

	store = CSR_U16(frm);
	type  = CSR_U16(frm);

	p_indent(level, frm);
	printf("%s: store 0x%4.4x type %d\n", str, store, type);
}

static inline void psnext_dump(int level, char *str, struct frame *frm)
{
	uint16_t key, stores, next;

	key    = CSR_U16(frm);
	stores = CSR_U16(frm);
	next   = CSR_U16(frm);

	p_indent(level, frm);
	printf("%s: key 0x%4.4x stores 0x%4.4x next 0x%4.4x\n", str, key, stores, next);
}

static inline void pssize_dump(int level, char *str, struct frame *frm)
{
	uint16_t key, length;

	key    = CSR_U16(frm);
	length = CSR_U16(frm);

	p_indent(level, frm);
	printf("%s: key 0x%4.4x %s 0x%4.4x\n", str, key,
				frm->in ? "len" : "stores", length);
}

static inline void psstores_dump(int level, char *str, struct frame *frm)
{
	uint16_t key, stores;

	key    = CSR_U16(frm);
	stores = CSR_U16(frm);

	p_indent(level, frm);
	printf("%s: key 0x%4.4x stores 0x%4.4x\n", str, key, stores);
}

static inline void pskey_dump(int level, struct frame *frm)
{
	uint16_t key, length, stores;

	key    = CSR_U16(frm);
	length = CSR_U16(frm);
	stores = CSR_U16(frm);

	p_indent(level, frm);
	printf("PSKEY: key 0x%4.4x len %d stores 0x%4.4x\n", key, length, stores);

	switch (key) {
	case 0x0001:
		bdaddr_dump(level + 1, "BDADDR", frm);
		break;
	case 0x0002:
		uint16_dump(level + 1, "COUNTRYCODE", frm);
		break;
	case 0x0003:
		uint32_dump(level + 1, "CLASSOFDEVICE", frm);
		break;
	case 0x0004:
		uint16_dump(level + 1, "DEVICE_DRIFT", frm);
		break;
	case 0x0005:
		uint16_dump(level + 1, "DEVICE_JITTER", frm);
		break;
	case 0x000d:
		uint16_dump(level + 1, "MAX_ACLS", frm);
		break;
	case 0x000e:
		uint16_dump(level + 1, "MAX_SCOS", frm);
		break;
	case 0x000f:
		uint16_dump(level + 1, "MAX_REMOTE_MASTERS", frm);
		break;
	case 0x00da:
		uint16_dump(level + 1, "ENC_KEY_LMIN", frm);
		break;
	case 0x00db:
		uint16_dump(level + 1, "ENC_KEY_LMAX", frm);
		break;
	case 0x00ef:
		features_dump(level + 1, "LOCAL_SUPPORTED_FEATURES", frm);
		break;
	case 0x0106:
		commands_dump(level + 1, "LOCAL_SUPPORTED_COMMANDS", frm);
		break;
	case 0x010d:
		uint16_dump(level + 1, "HCI_LMP_LOCAL_VERSION", frm);
		break;
	case 0x010e:
		uint16_dump(level + 1, "LMP_REMOTE_VERSION", frm);
		break;
	case 0x01a5:
		bool_dump(level + 1, "HOSTIO_USE_HCI_EXTN", frm);
		break;
	case 0x01ab:
		bool_dump(level + 1, "HOSTIO_MAP_SCO_PCM", frm);
		break;
	case 0x01be:
		uint16_dump(level + 1, "UART_BAUDRATE", frm);
		break;
	case 0x01f6:
		uint16_dump(level + 1, "ANA_FTRIM", frm);
		break;
	case 0x01f9:
		uint16_dump(level + 1, "HOST_INTERFACE", frm);
		break;
	case 0x01fe:
		uint16_dump(level + 1, "ANA_FREQ", frm);
		break;
	case 0x02be:
		uint16_dump(level + 1, "USB_VENDOR_ID", frm);
		break;
	case 0x02bf:
		uint16_dump(level + 1, "USB_PRODUCT_ID", frm);
		break;
	case 0x02cb:
		uint16_dump(level + 1, "USB_DFU_PRODUCT_ID", frm);
		break;
	case 0x03cd:
		int16_dump(level + 1, "INITIAL_BOOTMODE", frm);
		break;
	default:
		raw_dump(level + 1, frm);
		break;
	}
}

static inline void bccmd_dump(int level, struct frame *frm)
{
	uint16_t type, length, seqno, varid, status;

	type   = CSR_U16(frm);
	length = CSR_U16(frm);
	seqno  = CSR_U16(frm);
	varid  = CSR_U16(frm);
	status = CSR_U16(frm);

	p_indent(level, frm);
	printf("BCCMD: %s: len %d seqno %d varid 0x%4.4x status %d\n",
			type2str(type), length, seqno, varid, status);

	if (!(parser.flags & DUMP_VERBOSE)) {
		raw_dump(level + 1, frm);
		return;
	}

	switch (varid) {
	case 0x000b:
		valueless_dump(level + 1, "PS_CLR_ALL", frm);
		break;
	case 0x000c:
		valueless_dump(level + 1, "PS_FACTORY_SET", frm);
		break;
	case 0x082d:
		uint16_dump(level + 1, "PS_CLR_ALL_STORES", frm);
		break;
	case 0x2801:
		uint16_dump(level + 1, "BC01_STATUS", frm);
		break;
	case 0x2819:
		uint16_dump(level + 1, "BUILDID", frm);
		break;
	case 0x281a:
		uint16_dump(level + 1, "CHIPVER", frm);
		break;
	case 0x281b:
		uint16_dump(level + 1, "CHIPREV", frm);
		break;
	case 0x2825:
		uint16_dump(level + 1, "INTERFACE_VERSION", frm);
		break;
	case 0x282a:
		uint16_dump(level + 1, "RAND", frm);
		break;
	case 0x282c:
		uint16_dump(level + 1, "MAX_CRYPT_KEY_LENGTH", frm);
		break;
	case 0x2833:
		uint16_dump(level + 1, "E2_APP_SIZE", frm);
		break;
	case 0x2836:
		uint16_dump(level + 1, "CHIPANAREV", frm);
		break;
	case 0x2838:
		uint16_dump(level + 1, "BUILDID_LOADER", frm);
		break;
	case 0x2c00:
		uint32_dump(level + 1, "BT_CLOCK", frm);
		break;
	case 0x3005:
		psnext_dump(level + 1, "PS_NEXT", frm);
		break;
	case 0x3006:
		pssize_dump(level + 1, "PS_SIZE", frm);
		break;
	case 0x3008:
		handle_length_dump(level + 1, "CRYPT_KEY_LENGTH", frm);
		break;
	case 0x3009:
		handle_clock_dump(level + 1, "PICONET_INSTANCE", frm);
		break;
	case 0x300a:
		complex_dump(level + 1, "GET_CLR_EVT", frm);
		break;
	case 0x300b:
		complex_dump(level + 1, "GET_NEXT_BUILDDEF", frm);
		break;
	case 0x300e:
		complex_dump(level + 1, "E2_DEVICE", frm);
		break;
	case 0x300f:
		complex_dump(level + 1, "E2_APP_DATA", frm);
		break;
	case 0x3012:
		psmemtype_dump(level + 1, "PS_MEMORY_TYPE", frm);
		break;
	case 0x301c:
		complex_dump(level + 1, "READ_BUILD_NAME", frm);
		break;
	case 0x4001:
		valueless_dump(level + 1, "COLD_RESET", frm);
		break;
	case 0x4002:
		valueless_dump(level + 1, "WARM_RESET", frm);
		break;
	case 0x4003:
		valueless_dump(level + 1, "COLD_HALT", frm);
		break;
	case 0x4004:
		valueless_dump(level + 1, "WARM_HALT", frm);
		break;
	case 0x4005:
		valueless_dump(level + 1, "INIT_BT_STACK", frm);
		break;
	case 0x4006:
		valueless_dump(level + 1, "ACTIVATE_BT_STACK", frm);
		break;
	case 0x4007:
		valueless_dump(level + 1, "ENABLE_TX", frm);
		break;
	case 0x4008:
		valueless_dump(level + 1, "DISABLE_TX", frm);
		break;
	case 0x4009:
		valueless_dump(level + 1, "RECAL", frm);
		break;
	case 0x400d:
		valueless_dump(level + 1, "PS_FACTORY_RESTORE", frm);
		break;
	case 0x400e:
		valueless_dump(level + 1, "PS_FACTORY_RESTORE_ALL", frm);
		break;
	case 0x400f:
		valueless_dump(level + 1, "PS_DEFRAG_RESET", frm);
		break;
	case 0x4011:
		valueless_dump(level + 1, "HOPPING_ON", frm);
		break;
	case 0x4012:
		valueless_dump(level + 1, "CANCEL_PAGE", frm);
		break;
	case 0x4818:
		uint16_dump(level + 1, "PS_CLR", frm);
		break;
	case 0x481c:
		uint16_dump(level + 1, "MAP_SCO_PCM", frm);
		break;
	case 0x482e:
		uint16_dump(level + 1, "SINGLE_CHAN", frm);
		break;
	case 0x5004:
		radiotest_dump(level + 1, "RADIOTEST", frm);
		break;
	case 0x500c:
		psstores_dump(level + 1, "PS_CLR_STORES", frm);
		break;
	case 0x6000:
		valueless_dump(level + 1, "NO_VARIABLE", frm);
		break;
	case 0x6802:
		uint16_dump(level + 1, "CONFIG_UART", frm);
		break;
	case 0x6805:
		uint16_dump(level + 1, "PANIC_ARG", frm);
		break;
	case 0x6806:
		uint16_dump(level + 1, "FAULT_ARG", frm);
		break;
	case 0x6827:
		int8_dump(level + 1, "MAX_TX_POWER", frm);
		break;
	case 0x682b:
		int8_dump(level + 1, "DEFAULT_TX_POWER", frm);
		break;
	case 0x7003:
		pskey_dump(level + 1, frm);
		break;
	default:
		raw_dump(level + 1, frm);
		break;
	}
}

static char *cid2str(uint8_t cid)
{
	switch (cid & 0x3f) {
	case 0:
		return "BCSP Internal";
	case 1:
		return "BCSP Link";
	case 2:
		return "BCCMD";
	case 3:
		return "HQ";
	case 4:
		return "Device Mgt";
	case 5:
		return "HCI Cmd/Evt";
	case 6:
		return "HCI ACL";
	case 7:
		return "HCI SCO";
	case 8:
		return "L2CAP";
	case 9:
		return "RFCOMM";
	case 10:
		return "SDP";
	case 11:
		return "Debug";
	case 12:
		return "DFU";
	case 13:
		return "VM";
	case 14:
		return "Unused";
	case 15:
		return "Reserved";
	default:
		return "Unknown";
	}
}

static char *frag2str(uint8_t frag)
{
	switch (frag & 0xc0) {
	case 0x00:
		return " middle fragment";
	case 0x40:
		return " first fragment";
	case 0x80:
		return " last fragment";
	default:
		return "";
	}
}

void csr_dump(int level, struct frame *frm)
{
	uint8_t desc, cid, type;
	uint16_t handle, master, addr;

	desc = CSR_U8(frm);

	cid = desc & 0x3f;

	switch (cid) {
	case 2:
		bccmd_dump(level, frm);
		break;

	case 20:
		type = CSR_U8(frm);

		if (!p_filter(FILT_LMP)) {
			switch (type) {
			case 0x0f:
				frm->handle =  ((uint8_t *) frm->ptr)[17];
				frm->master = 0;
				frm->len--;
				lmp_dump(level, frm);
				return;
			case 0x10:
				frm->handle = ((uint8_t *) frm->ptr)[17];
				frm->master = 1;
				frm->len--;
				lmp_dump(level, frm);
				return;
			case 0x12:
				handle = CSR_U16(frm);
				master = CSR_U16(frm);
				addr = CSR_U16(frm);
				p_indent(level, frm);
				printf("FHS: handle %d addr %d (%s)\n", handle,
					addr, master ? "master" : "slave");
				if (!master) {
					char addr[18];
					p_ba2str((bdaddr_t *) frm->ptr, addr);
					p_indent(level + 1, frm);
					printf("bdaddr %s class "
						"0x%2.2x%2.2x%2.2x\n", addr,
						((uint8_t *) frm->ptr)[8],
						((uint8_t *) frm->ptr)[7],
						((uint8_t *) frm->ptr)[6]);
				}
				return;
			case 0x7b:
				p_indent(level, frm);
				printf("LMP(r): duplicate (same SEQN)\n");
				return;
			}
		}

		p_indent(level, frm);
		printf("CSR: Debug (type 0x%2.2x)\n", type);
		raw_dump(level, frm);
		break;

	default:
		p_indent(level, frm);
		printf("CSR: %s (channel %d)%s\n", cid2str(cid), cid, frag2str(desc));
		raw_dump(level, frm);
		break;
	}
}
