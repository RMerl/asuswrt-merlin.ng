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
#include <ctype.h>
#include <sys/socket.h>

#include "parser.h"
#include "lib/hci.h"
#include "lib/hci_lib.h"

#define LMP_U8(frm)  (p_get_u8(frm))
#define LMP_U16(frm) (btohs(htons(p_get_u16(frm))))
#define LMP_U32(frm) (btohl(htonl(p_get_u32(frm))))

static enum {
	IN_RAND,
	COMB_KEY_M,
	COMB_KEY_S,
	AU_RAND_M,
	AU_RAND_S,
	SRES_M,
	SRES_S,
} pairing_state = IN_RAND;

static struct {
	uint8_t in_rand[16];
	uint8_t comb_key_m[16];
	uint8_t comb_key_s[16];
	uint8_t au_rand_m[16];
	uint8_t au_rand_s[16];
	uint8_t sres_m[4];
	uint8_t sres_s[4];
} pairing_data;

static inline void pairing_data_dump(void)
{
	int i;

	p_indent(6, NULL);
	printf("IN_RAND  ");
	for (i = 0; i < 16; i++)
		printf("%2.2x", pairing_data.in_rand[i]);
	printf("\n");

	p_indent(6, NULL);
	printf("COMB_KEY ");
	for (i = 0; i < 16; i++)
		printf("%2.2x", pairing_data.comb_key_m[i]);
	printf(" (M)\n");

	p_indent(6, NULL);
	printf("COMB_KEY ");
	for (i = 0; i < 16; i++)
		printf("%2.2x", pairing_data.comb_key_s[i]);
	printf(" (S)\n");

	p_indent(6, NULL);
	printf("AU_RAND  ");
	for (i = 0; i < 16; i++)
		printf("%2.2x", pairing_data.au_rand_m[i]);
	printf(" SRES ");
	for (i = 0; i < 4; i++)
		printf("%2.2x", pairing_data.sres_m[i]);
	printf(" (M)\n");

	p_indent(6, NULL);
	printf("AU_RAND  ");
	for (i = 0; i < 16; i++)
		printf("%2.2x", pairing_data.au_rand_s[i]);
	printf(" SRES ");
	for (i = 0; i < 4; i++)
		printf("%2.2x", pairing_data.sres_s[i]);
	printf(" (S)\n");
}

static inline void in_rand(struct frame *frm)
{
	uint8_t *val = frm->ptr;

	memcpy(pairing_data.in_rand, val, 16);
	pairing_state = COMB_KEY_M;
}

static inline void comb_key(struct frame *frm)
{
	uint8_t *val = frm->ptr;

	switch (pairing_state) {
	case COMB_KEY_M:
		memcpy(pairing_data.comb_key_m, val, 16);
		pairing_state = COMB_KEY_S;
		break;
	case COMB_KEY_S:
		memcpy(pairing_data.comb_key_s, val, 16);
		pairing_state = AU_RAND_M;
		break;
	case IN_RAND:
	case AU_RAND_M:
	case AU_RAND_S:
	case SRES_M:
	case SRES_S:
	default:
		pairing_state = IN_RAND;
		break;
	}
}

static inline void au_rand(struct frame *frm)
{
	uint8_t *val = frm->ptr;

	switch (pairing_state) {
	case AU_RAND_M:
		memcpy(pairing_data.au_rand_m, val, 16);
		pairing_state = SRES_M;
		break;
	case AU_RAND_S:
		memcpy(pairing_data.au_rand_s, val, 16);
		pairing_state = SRES_S;
		break;
	case COMB_KEY_M:
	case COMB_KEY_S:
	case IN_RAND:
	case SRES_M:
	case SRES_S:
	default:
		pairing_state = IN_RAND;
		break;
	}
}

static inline void sres(struct frame *frm)
{
	uint8_t *val = frm->ptr;

	switch (pairing_state) {
	case SRES_M:
		memcpy(pairing_data.sres_m, val, 4);
		pairing_state = AU_RAND_S;
		break;
	case SRES_S:
		memcpy(pairing_data.sres_s, val, 4);
		pairing_state = IN_RAND;
		pairing_data_dump();
		break;
	case COMB_KEY_M:
	case COMB_KEY_S:
	case IN_RAND:
	case AU_RAND_M:
	case AU_RAND_S:
	default:
		pairing_state = IN_RAND;
		break;
	}
}

static char *opcode2str(uint16_t opcode)
{
	switch (opcode) {
	case 1:
		return "name_req";
	case 2:
		return "name_res";
	case 3:
		return "accepted";
	case 4:
		return "not_accepted";
	case 5:
		return "clkoffset_req";
	case 6:
		return "clkoffset_res";
	case 7:
		return "detach";
	case 8:
		return "in_rand";
	case 9:
		return "comb_key";
	case 10:
		return "unit_key";
	case 11:
		return "au_rand";
	case 12:
		return "sres";
	case 13:
		return "temp_rand";
	case 14:
		return "temp_key";
	case 15:
		return "encryption_mode_req";
	case 16:
		return "encryption_key_size_req";
	case 17:
		return "start_encryption_req";
	case 18:
		return "stop_encryption_req";
	case 19:
		return "switch_req";
	case 20:
		return "hold";
	case 21:
		return "hold_req";
	case 22:
		return "sniff";
	case 23:
		return "sniff_req";
	case 24:
		return "unsniff_req";
	case 25:
		return "park_req";
	case 26:
		return "park";
	case 27:
		return "set_broadcast_scan_window";
	case 28:
		return "modify_beacon";
	case 29:
		return "unpark_BD_ADDR_req";
	case 30:
		return "unpark_PM_ADDR_req";
	case 31:
		return "incr_power_req";
	case 32:
		return "decr_power_req";
	case 33:
		return "max_power";
	case 34:
		return "min_power";
	case 35:
		return "auto_rate";
	case 36:
		return "preferred_rate";
	case 37:
		return "version_req";
	case 38:
		return "version_res";
	case 39:
		return "feature_req";
	case 40:
		return "feature_res";
	case 41:
		return "quality_of_service";
	case 42:
		return "quality_of_service_req";
	case 43:
		return "SCO_link_req";
	case 44:
		return "remove_SCO_link_req";
	case 45:
		return "max_slot";
	case 46:
		return "max_slot_req";
	case 47:
		return "timing_accuracy_req";
	case 48:
		return "timing_accuracy_res";
	case 49:
		return "setup_complete";
	case 50:
		return "use_semi_permanent_key";
	case 51:
		return "host_connection_req";
	case 52:
		return "slot_offset";
	case 53:
		return "page_mode_req";
	case 54:
		return "page_scan_mode_req";
	case 55:
		return "supervision_timeout";
	case 56:
		return "test_activate";
	case 57:
		return "test_control";
	case 58:
		return "encryption_key_size_mask_req";
	case 59:
		return "encryption_key_size_mask_res";
	case 60:
		return "set_AFH";
	case 61:
		return "encapsulated_header";
	case 62:
		return "encapsulated_payload";
	case 63:
		return "simple_pairing_confirm";
	case 64:
		return "simple_pairing_number";
	case 65:
		return "DHkey_check";
	case 127 + (1 << 7):
		return "accepted_ext";
	case 127 + (2 << 7):
		return "not_accepted_ext";
	case 127 + (3 << 7):
		return "features_req_ext";
	case 127 + (4 << 7):
		return "features_res_ext";
	case 127 + (11 << 7):
		return "packet_type_table_req";
	case 127 + (12 << 7):
		return "eSCO_link_req";
	case 127 + (13 << 7):
		return "remove_eSCO_link_req";
	case 127 + (16 << 7):
		return "channel_classification_req";
	case 127 + (17 << 7):
		return "channel_classification";
	case 127 + (21 << 7):
		return "sniff_subrating_req";
	case 127 + (22 << 7):
		return "sniff_subrating_res";
	case 127 + (23 << 7):
		return "pause_encryption_req";
	case 127 + (24 << 7):
		return "resume_encryption_req";
	case 127 + (25 << 7):
		return "IO_capability_req";
	case 127 + (26 << 7):
		return "IO_capability_res";
	case 127 + (27 << 7):
		return "numeric_comparison_failed";
	case 127 + (28 << 7):
		return "passkey_failed";
	case 127 + (29 << 7):
		return "oob_failed";
	case 127 + (30 << 7):
		return "keypress_notification";
	default:
		return "unknown";
	}
}

static inline void name_req_dump(int level, struct frame *frm)
{
	uint8_t offset = LMP_U8(frm);

	p_indent(level, frm);
	printf("name offset %d\n", offset);
}

static inline void name_res_dump(int level, struct frame *frm)
{
	uint8_t offset = LMP_U8(frm);
	uint8_t length = LMP_U8(frm);
	uint8_t *name = frm->ptr;
	int i, size;

	frm->ptr += 14;
	frm->len -= 14;

	p_indent(level, frm);
	printf("name offset %d\n", offset);

	p_indent(level, frm);
	printf("name length %d\n", length);

	size = length - offset;
	if (size > 14)
		size = 14;

	p_indent(level, frm);
	printf("name fragment '");
	for (i = 0; i < size; i++)
		if (isprint(name[i]))
			printf("%c", name[i]);
		else
			printf(".");
	printf("'\n");
}

static inline void accepted_dump(int level, struct frame *frm)
{
	uint8_t opcode = LMP_U8(frm);

	p_indent(level, frm);
	printf("op code %d (%s)\n", opcode, opcode2str(opcode));
}

static inline void not_accepted_dump(int level, struct frame *frm)
{
	uint8_t opcode = LMP_U8(frm);
	uint8_t error = LMP_U8(frm);

	p_indent(level, frm);
	printf("op code %d (%s)\n", opcode, opcode2str(opcode));

	p_indent(level, frm);
	printf("error code 0x%2.2x\n", error);
}

static inline void clkoffset_dump(int level, struct frame *frm)
{
	uint16_t clkoffset = LMP_U16(frm);

	p_indent(level, frm);
	printf("clock offset 0x%4.4x\n", clkoffset);
}

static inline void detach_dump(int level, struct frame *frm)
{
	uint8_t error = LMP_U8(frm);

	p_indent(level, frm);
	printf("error code 0x%2.2x\n", error);
}

static inline void random_number_dump(int level, struct frame *frm)
{
	uint8_t *number = frm->ptr;
	int i;

	frm->ptr += 16;
	frm->len -= 16;

	p_indent(level, frm);
	printf("random number ");
	for (i = 0; i < 16; i++)
		printf("%2.2x", number[i]);
	printf("\n");
}

static inline void key_dump(int level, struct frame *frm)
{
	uint8_t *key = frm->ptr;
	int i;

	frm->ptr += 16;
	frm->len -= 16;

	p_indent(level, frm);
	printf("key ");
	for (i = 0; i < 16; i++)
		printf("%2.2x", key[i]);
	printf("\n");
}

static inline void auth_resp_dump(int level, struct frame *frm)
{
	uint8_t *resp = frm->ptr;
	int i;

	frm->ptr += 4;
	frm->ptr -= 4;

	p_indent(level, frm);
	printf("authentication response ");
	for (i = 0; i < 4; i++)
		printf("%2.2x", resp[i]);
	printf("\n");
}

static inline void encryption_mode_req_dump(int level, struct frame *frm)
{
	uint8_t mode = LMP_U8(frm);

	p_indent(level, frm);
	printf("encryption mode %d\n", mode);
}

static inline void encryption_key_size_req_dump(int level, struct frame *frm)
{
	uint8_t keysize = LMP_U8(frm);

	p_indent(level, frm);
	printf("key size %d\n", keysize);
}

static inline void switch_req_dump(int level, struct frame *frm)
{
	uint32_t instant = LMP_U32(frm);

	p_indent(level, frm);
	printf("switch instant 0x%4.4x\n", instant);
}

static inline void hold_dump(int level, struct frame *frm)
{
	uint16_t time = LMP_U16(frm);
	uint32_t instant = LMP_U32(frm);

	p_indent(level, frm);
	printf("hold time 0x%4.4x\n", time);

	p_indent(level, frm);
	printf("hold instant 0x%4.4x\n", instant);
}

static inline void sniff_req_dump(int level, struct frame *frm)
{
	uint8_t timing = LMP_U8(frm);
	uint16_t dsniff = LMP_U16(frm);
	uint16_t tsniff = LMP_U16(frm);
	uint16_t attempt = LMP_U16(frm);
	uint16_t timeout = LMP_U16(frm);

	p_indent(level, frm);
	printf("timing control flags 0x%2.2x\n", timing);

	p_indent(level, frm);
	printf("D_sniff %d T_sniff %d\n", dsniff, tsniff);

	p_indent(level, frm);
	printf("sniff attempt %d\n", attempt);

	p_indent(level, frm);
	printf("sniff timeout %d\n", timeout);
}

static inline void park_req_dump(int level, struct frame *frm)
{
	uint8_t timing = LMP_U8(frm);
	uint16_t db = LMP_U16(frm);
	uint16_t tb = LMP_U16(frm);
	uint8_t nb = LMP_U8(frm);
	uint8_t xb = LMP_U8(frm);
	uint8_t pmaddr = LMP_U8(frm);
	uint8_t araddr = LMP_U8(frm);
	uint8_t nbsleep = LMP_U8(frm);
	uint8_t dbsleep = LMP_U8(frm);
	uint8_t daccess = LMP_U8(frm);
	uint8_t taccess = LMP_U8(frm);
	uint8_t nslots = LMP_U8(frm);
	uint8_t npoll = LMP_U8(frm);
	uint8_t access = LMP_U8(frm);

	p_indent(level, frm);
	printf("timing control flags 0x%2.2x\n", timing);

	p_indent(level, frm);
	printf("D_B %d T_B %d N_B %d X_B %d\n", db, tb, nb, xb);

	p_indent(level, frm);
	printf("PM_ADDR %d AR_ADDR %d\n", pmaddr, araddr);

	p_indent(level, frm);
	printf("N_Bsleep %d D_Bsleep %d\n", nbsleep, dbsleep);

	p_indent(level, frm);
	printf("D_access %d T_access %d\n", daccess, taccess);

	p_indent(level, frm);
	printf("N_acc-slots %d N_poll %d\n", nslots, npoll);

	p_indent(level, frm);
	printf("M_access %d\n", access & 0x0f);

	p_indent(level, frm);
	printf("access scheme 0x%2.2x\n", access >> 4);
}

static inline void modify_beacon_dump(int level, struct frame *frm)
{
	uint8_t timing = LMP_U8(frm);
	uint16_t db = LMP_U16(frm);
	uint16_t tb = LMP_U16(frm);
	uint8_t nb = LMP_U8(frm);
	uint8_t xb = LMP_U8(frm);
	uint8_t daccess = LMP_U8(frm);
	uint8_t taccess = LMP_U8(frm);
	uint8_t nslots = LMP_U8(frm);
	uint8_t npoll = LMP_U8(frm);
	uint8_t access = LMP_U8(frm);

	p_indent(level, frm);
	printf("timing control flags 0x%2.2x\n", timing);

	p_indent(level, frm);
	printf("D_B %d T_B %d N_B %d X_B %d\n", db, tb, nb, xb);

	p_indent(level, frm);
	printf("D_access %d T_access %d\n", daccess, taccess);

	p_indent(level, frm);
	printf("N_acc-slots %d N_poll %d\n", nslots, npoll);

	p_indent(level, frm);
	printf("M_access %d\n", access & 0x0f);

	p_indent(level, frm);
	printf("access scheme 0x%2.2x\n", access >> 4);
}

static inline void power_req_dump(int level, struct frame *frm)
{
	uint8_t val = LMP_U8(frm);

	p_indent(level, frm);
	printf("future use 0x%2.2x\n", val);
}

static inline void preferred_rate_dump(int level, struct frame *frm)
{
	uint8_t rate = LMP_U8(frm);

	p_indent(level, frm);
	printf("data rate 0x%2.2x\n", rate);

	p_indent(level, frm);
	printf("Basic: ");

	printf("%suse FEC, ", rate & 0x01 ? "do not " : "");

	switch ((rate >> 1) & 0x03) {
	case 0x00:
		printf("no packet-size preference\n");
		break;
	case 0x01:
		printf("use 1-slot packets\n");
		break;
	case 0x02:
		printf("use 3-slot packets\n");
		break;
	case 0x03:
		printf("use 5-slot packets\n");
		break;
	}

	p_indent(level, frm);
	printf("EDR: ");

	switch ((rate >> 3) & 0x03) {
	case 0x00:
		printf("use DM1 packets, ");
		break;
	case 0x01:
		printf("use 2 Mbps packets, ");
		break;
	case 0x02:
		printf("use 3 Mbps packets, ");
		break;
	case 0x03:
		printf("reserved, \n");
		break;
	}

	switch ((rate >> 5) & 0x03) {
	case 0x00:
		printf("no packet-size preference\n");
		break;
	case 0x01:
		printf("use 1-slot packets\n");
		break;
	case 0x02:
		printf("use 3-slot packets\n");
		break;
	case 0x03:
		printf("use 5-slot packets\n");
		break;
	}
}

static inline void version_dump(int level, struct frame *frm)
{
	uint8_t ver = LMP_U8(frm);
	uint16_t compid = LMP_U16(frm);
	uint16_t subver = LMP_U16(frm);
	char *tmp;

	p_indent(level, frm);
	tmp = lmp_vertostr(ver);
	printf("VersNr %d (%s)\n", ver, tmp);
	bt_free(tmp);

	p_indent(level, frm);
	printf("CompId %d (%s)\n", compid, bt_compidtostr(compid));

	p_indent(level, frm);
	printf("SubVersNr %d\n", subver);
}

static inline void features_dump(int level, struct frame *frm)
{
	uint8_t *features = frm->ptr;
	int i;

	frm->ptr += 8;
	frm->len -= 8;

	p_indent(level, frm);
	printf("features");
	for (i = 0; i < 8; i++)
		printf(" 0x%2.2x", features[i]);
	printf("\n");
}

static inline void set_afh_dump(int level, struct frame *frm)
{
	uint32_t instant = LMP_U32(frm);
	uint8_t mode = LMP_U8(frm);
	uint8_t *map = frm->ptr;
	int i;

	frm->ptr += 10;
	frm->len -= 10;

	p_indent(level, frm);
	printf("AFH_instant 0x%04x\n", instant);

	p_indent(level, frm);
	printf("AFH_mode %d\n", mode);

	p_indent(level, frm);
	printf("AFH_channel_map 0x");
	for (i = 0; i < 10; i++)
		printf("%2.2x", map[i]);
	printf("\n");
}

static inline void encapsulated_header_dump(int level, struct frame *frm)
{
	uint8_t major = LMP_U8(frm);
	uint8_t minor = LMP_U8(frm);
	uint8_t length = LMP_U8(frm);

	p_indent(level, frm);
	printf("major type %d minor type %d payload length %d\n",
						major, minor, length);

	if (major == 1 && minor == 1) {
		p_indent(level, frm);
		printf("P-192 Public Key\n");
	}
}

static inline void encapsulated_payload_dump(int level, struct frame *frm)
{
	uint8_t *value = frm->ptr;
	int i;

	frm->ptr += 16;
	frm->len -= 16;

	p_indent(level, frm);
	printf("data ");
	for (i = 0; i < 16; i++)
		printf("%2.2x", value[i]);
	printf("\n");
}

static inline void simple_pairing_confirm_dump(int level, struct frame *frm)
{
	uint8_t *value = frm->ptr;
	int i;

	frm->ptr += 16;
	frm->len -= 16;

	p_indent(level, frm);
	printf("commitment value ");
	for (i = 0; i < 16; i++)
		printf("%2.2x", value[i]);
	printf("\n");
}

static inline void simple_pairing_number_dump(int level, struct frame *frm)
{
	uint8_t *value = frm->ptr;
	int i;

	frm->ptr += 16;
	frm->len -= 16;

	p_indent(level, frm);
	printf("nounce value ");
	for (i = 0; i < 16; i++)
		printf("%2.2x", value[i]);
	printf("\n");
}

static inline void dhkey_check_dump(int level, struct frame *frm)
{
	uint8_t *value = frm->ptr;
	int i;

	frm->ptr += 16;
	frm->len -= 16;

	p_indent(level, frm);
	printf("confirmation value ");
	for (i = 0; i < 16; i++)
		printf("%2.2x", value[i]);
	printf("\n");
}

static inline void accepted_ext_dump(int level, struct frame *frm)
{
	uint16_t opcode = LMP_U8(frm) + (LMP_U8(frm) << 7);

	p_indent(level, frm);
	printf("op code %d/%d (%s)\n", opcode & 0x7f, opcode >> 7, opcode2str(opcode));
}

static inline void not_accepted_ext_dump(int level, struct frame *frm)
{
	uint16_t opcode = LMP_U8(frm) + (LMP_U8(frm) << 7);
	uint8_t error = LMP_U8(frm);

	p_indent(level, frm);
	printf("op code %d/%d (%s)\n", opcode & 0x7f, opcode >> 7, opcode2str(opcode));

	p_indent(level, frm);
	printf("error code 0x%2.2x\n", error);
}

static inline void features_ext_dump(int level, struct frame *frm)
{
	uint8_t page = LMP_U8(frm);
	uint8_t max = LMP_U8(frm);
	uint8_t *features = frm->ptr;
	int i;

	frm->ptr += 8;
	frm->len -= 8;

	p_indent(level, frm);
	printf("features page %d\n", page);

	p_indent(level, frm);
	printf("max supported page %d\n", max);

	p_indent(level, frm);
	printf("extended features");
	for (i = 0; i < 8; i++)
		printf(" 0x%2.2x", features[i]);
	printf("\n");
}

static inline void quality_of_service_dump(int level, struct frame *frm)
{
	uint16_t interval = LMP_U16(frm);
	uint8_t nbc = LMP_U8(frm);

	p_indent(level, frm);
	printf("poll interval %d\n", interval);

	p_indent(level, frm);
	printf("N_BC %d\n", nbc);
}

static inline void sco_link_req_dump(int level, struct frame *frm)
{
	uint8_t handle = LMP_U8(frm);
	uint8_t timing = LMP_U8(frm);
	uint8_t dsco = LMP_U8(frm);
	uint8_t tsco = LMP_U8(frm);
	uint8_t packet = LMP_U8(frm);
	uint8_t airmode = LMP_U8(frm);

	p_indent(level, frm);
	printf("SCO handle %d\n", handle);

	p_indent(level, frm);
	printf("timing control flags 0x%2.2x\n", timing);

	p_indent(level, frm);
	printf("D_SCO %d T_SCO %d\n", dsco, tsco);

	p_indent(level, frm);
	printf("SCO packet 0x%2.2x\n", packet);

	p_indent(level, frm);
	printf("air mode 0x%2.2x\n", airmode);
}

static inline void remove_sco_link_req_dump(int level, struct frame *frm)
{
	uint8_t handle = LMP_U8(frm);
	uint8_t error = LMP_U8(frm);

	p_indent(level, frm);
	printf("SCO handle %d\n", handle);

	p_indent(level, frm);
	printf("error code 0x%2.2x\n", error);
}

static inline void max_slots_dump(int level, struct frame *frm)
{
	uint8_t slots = LMP_U8(frm);

	p_indent(level, frm);
	printf("max slots %d\n", slots);
}

static inline void timing_accuracy_dump(int level, struct frame *frm)
{
	uint8_t drift = LMP_U8(frm);
	uint8_t jitter = LMP_U8(frm);

	p_indent(level, frm);
	printf("drift %d\n", drift);

	p_indent(level, frm);
	printf("jitter %d\n", jitter);
}

static inline void slot_offset_dump(int level, struct frame *frm)
{
	uint16_t offset = LMP_U16(frm);
	char addr[18];

	p_ba2str((bdaddr_t *) frm->ptr, addr);

	p_indent(level, frm);
	printf("slot offset %d\n", offset);

	p_indent(level, frm);
	printf("BD_ADDR %s\n", addr);
}

static inline void page_mode_dump(int level, struct frame *frm)
{
	uint8_t scheme = LMP_U8(frm);
	uint8_t settings = LMP_U8(frm);

	p_indent(level, frm);
	printf("page scheme %d\n", scheme);

	p_indent(level, frm);
	printf("page scheme settings %d\n", settings);
}

static inline void supervision_timeout_dump(int level, struct frame *frm)
{
	uint16_t timeout = LMP_U16(frm);

	p_indent(level, frm);
	printf("supervision timeout %d\n", timeout);
}

static inline void test_control_dump(int level, struct frame *frm)
{
	uint8_t scenario = LMP_U8(frm);
	uint8_t hopping = LMP_U8(frm);
	uint8_t txfreq = LMP_U8(frm);
	uint8_t rxfreq = LMP_U8(frm);
	uint8_t power = LMP_U8(frm);
	uint8_t poll = LMP_U8(frm);
	uint8_t packet = LMP_U8(frm);
	uint16_t length = LMP_U16(frm);

	p_indent(level, frm);
	printf("test scenario %d\n", scenario);

	p_indent(level, frm);
	printf("hopping mode %d\n", hopping);

	p_indent(level, frm);
	printf("TX frequency %d\n", txfreq);

	p_indent(level, frm);
	printf("RX frequency %d\n", rxfreq);

	p_indent(level, frm);
	printf("power control mode %d\n", power);

	p_indent(level, frm);
	printf("poll period %d\n", poll);

	p_indent(level, frm);
	printf("poll period %d\n", poll);

	p_indent(level, frm);
	printf("packet type 0x%2.2x\n", packet);

	p_indent(level, frm);
	printf("length of test data %d\n", length);
}

static inline void encryption_key_size_mask_res_dump(int level, struct frame *frm)
{
	uint16_t mask = LMP_U16(frm);

	p_indent(level, frm);
	printf("key size mask 0x%4.4x\n", mask);
}

static inline void packet_type_table_dump(int level, struct frame *frm)
{
	uint8_t type = LMP_U8(frm);

	p_indent(level, frm);
	printf("packet type table %d ", type);
	switch (type) {
	case 0:
		printf("(1Mbps only)\n");
		break;
	case 1:
		printf("(2/3Mbps)\n");
		break;
	default:
		printf("(Reserved)\n");
		break;
	}
}

static inline void esco_link_req_dump(int level, struct frame *frm)
{
	uint8_t handle = LMP_U8(frm);
	uint8_t ltaddr = LMP_U8(frm);
	uint8_t timing = LMP_U8(frm);
	uint8_t desco = LMP_U8(frm);
	uint8_t tesco = LMP_U8(frm);
	uint8_t wesco = LMP_U8(frm);
	uint8_t mspkt = LMP_U8(frm);
	uint8_t smpkt = LMP_U8(frm);
	uint16_t mslen = LMP_U16(frm);
	uint16_t smlen = LMP_U16(frm);
	uint8_t airmode = LMP_U8(frm);
	uint8_t negstate = LMP_U8(frm);

	p_indent(level, frm);
	printf("eSCO handle %d\n", handle);

	p_indent(level, frm);
	printf("eSCO LT_ADDR %d\n", ltaddr);

	p_indent(level, frm);
	printf("timing control flags 0x%2.2x\n", timing);

	p_indent(level, frm);
	printf("D_eSCO %d T_eSCO %d W_eSCO %d\n", desco, tesco, wesco);

	p_indent(level, frm);
	printf("eSCO M->S packet type 0x%2.2x length %d\n", mspkt, mslen);

	p_indent(level, frm);
	printf("eSCO S->M packet type 0x%2.2x length %d\n", smpkt, smlen);

	p_indent(level, frm);
	printf("air mode 0x%2.2x\n", airmode);

	p_indent(level, frm);
	printf("negotiation state 0x%2.2x\n", negstate);
}

static inline void remove_esco_link_req_dump(int level, struct frame *frm)
{
	uint8_t handle = LMP_U8(frm);
	uint8_t error = LMP_U8(frm);

	p_indent(level, frm);
	printf("eSCO handle %d\n", handle);

	p_indent(level, frm);
	printf("error code 0x%2.2x\n", error);
}

static inline void channel_classification_req_dump(int level, struct frame *frm)
{
	uint8_t mode = LMP_U8(frm);
	uint16_t min = LMP_U16(frm);
	uint16_t max = LMP_U16(frm);

	p_indent(level, frm);
	printf("AFH reporting mode %d\n", mode);

	p_indent(level, frm);
	printf("AFH min interval 0x%4.4x\n", min);

	p_indent(level, frm);
	printf("AFH max interval 0x%4.4x\n", max);
}

static inline void channel_classification_dump(int level, struct frame *frm)
{
	uint8_t *map = frm->ptr;
	int i;

	frm->ptr += 10;
	frm->len -= 10;

	p_indent(level, frm);
	printf("AFH channel classification 0x");
	for (i = 0; i < 10; i++)
		printf("%2.2x", map[i]);
	printf("\n");
}

static inline void sniff_subrating_dump(int level, struct frame *frm)
{
	uint8_t subrate = LMP_U8(frm);
	uint16_t timeout = LMP_U16(frm);
	uint32_t instant = LMP_U32(frm);

	p_indent(level, frm);
	printf("max subrate %d\n", subrate);

	p_indent(level, frm);
	printf("min sniff timeout %d\n", timeout);

	p_indent(level, frm);
	printf("subrate instant 0x%4.4x\n", instant);
}

static inline void io_capability_dump(int level, struct frame *frm)
{
	uint8_t capability = LMP_U8(frm);
	uint8_t oob_data = LMP_U8(frm);
	uint8_t authentication = LMP_U8(frm);

	p_indent(level, frm);
	printf("capability 0x%2.2x oob 0x%2.2x auth 0x%2.2x\n",
				capability, oob_data, authentication);
}

static inline void keypress_notification_dump(int level, struct frame *frm)
{
	uint8_t value = LMP_U8(frm);

	p_indent(level, frm);
	printf("notification value %d\n", value);
}

void lmp_dump(int level, struct frame *frm)
{
	uint8_t tmp, tid;
	uint16_t opcode;

	p_indent(level, frm);

	tmp = LMP_U8(frm);
	tid = tmp & 0x01;
	opcode = (tmp & 0xfe) >> 1;
	if (opcode > 123) {
		tmp = LMP_U8(frm);
		opcode += tmp << 7;
	}

	printf("LMP(%c): %s(%c): ", frm->master ? 's' : 'r',
				opcode2str(opcode), tid ? 's' : 'm');

	if (opcode > 123)
		printf("op code %d/%d", opcode & 0x7f, opcode >> 7);
	else
		printf("op code %d", opcode);

	if (frm->handle > 17)
		printf(" handle %d\n", frm->handle);
	else
		printf("\n");

	if (!(parser.flags & DUMP_VERBOSE)) {
		raw_dump(level, frm);
		return;
	}

	switch (opcode) {
	case 1:
		name_req_dump(level + 1, frm);
		return;
	case 2:
		name_res_dump(level + 1, frm);
		return;
	case 3:
		accepted_dump(level + 1, frm);
		return;
	case 4:
		not_accepted_dump(level + 1, frm);
		return;
	case 6:
		clkoffset_dump(level + 1, frm);
		return;
	case 7:
		detach_dump(level + 1, frm);
		return;
	case 8:
		in_rand(frm);
		random_number_dump(level + 1, frm);
		return;
	case 9:
		comb_key(frm);
		random_number_dump(level + 1, frm);
		return;
	case 11:
		au_rand(frm);
		random_number_dump(level + 1, frm);
		return;
	case 12:
		sres(frm);
		auth_resp_dump(level + 1, frm);
		return;
	case 13:
	case 17:
		random_number_dump(level + 1, frm);
		return;
	case 10:
	case 14:
		key_dump(level + 1, frm);
		return;
	case 15:
		encryption_mode_req_dump(level + 1, frm);
		return;
	case 16:
		encryption_key_size_req_dump(level + 1, frm);
		return;
	case 19:
		switch_req_dump(level + 1, frm);
		return;
	case 20:
	case 21:
		hold_dump(level + 1, frm);
		return;
	case 23:
		sniff_req_dump(level + 1, frm);
		return;
	case 25:
		park_req_dump(level + 1, frm);
		return;
	case 28:
		modify_beacon_dump(level + 1, frm);
		return;
	case 31:
	case 32:
		power_req_dump(level + 1, frm);
		return;
	case 36:
		preferred_rate_dump(level + 1, frm);
		return;
	case 37:
	case 38:
		version_dump(level + 1, frm);
		return;
	case 39:
	case 40:
		features_dump(level + 1, frm);
		return;
	case 41:
	case 42:
		quality_of_service_dump(level + 1, frm);
		return;
	case 43:
		sco_link_req_dump(level + 1, frm);
		return;
	case 44:
		remove_sco_link_req_dump(level + 1, frm);
		return;
	case 45:
	case 46:
		max_slots_dump(level + 1, frm);
		return;
	case 48:
		timing_accuracy_dump(level + 1, frm);
		return;
	case 52:
		slot_offset_dump(level + 1, frm);
		return;
	case 53:
	case 54:
		page_mode_dump(level + 1, frm);
		return;
	case 55:
		supervision_timeout_dump(level + 1, frm);
		return;
	case 57:
		test_control_dump(level + 1, frm);
		return;
	case 59:
		encryption_key_size_mask_res_dump(level + 1, frm);
		return;
	case 60:
		set_afh_dump(level + 1, frm);
		return;
	case 61:
		encapsulated_header_dump(level + 1, frm);
		return;
	case 62:
		encapsulated_payload_dump(level + 1, frm);
		return;
	case 63:
		simple_pairing_confirm_dump(level + 1, frm);
		return;
	case 64:
		simple_pairing_number_dump(level + 1, frm);
		return;
	case 65:
		dhkey_check_dump(level + 1, frm);
		return;
	case 5:
	case 18:
	case 24:
	case 33:
	case 34:
	case 35:
	case 47:
	case 49:
	case 50:
	case 51:
	case 56:
	case 58:
	case 127 + (23 << 7):
	case 127 + (24 << 7):
	case 127 + (27 << 7):
	case 127 + (28 << 7):
	case 127 + (29 << 7):
		return;
	case 127 + (1 << 7):
		accepted_ext_dump(level + 1, frm);
		return;
	case 127 + (2 << 7):
		not_accepted_ext_dump(level + 1, frm);
		return;
	case 127 + (3 << 7):
	case 127 + (4 << 7):
		features_ext_dump(level + 1, frm);
		return;
	case 127 + (11 << 7):
		packet_type_table_dump(level + 1, frm);
		return;
	case 127 + (12 << 7):
		esco_link_req_dump(level + 1, frm);
		return;
	case 127 + (13 << 7):
		remove_esco_link_req_dump(level + 1, frm);
		return;
	case 127 + (16 << 7):
		channel_classification_req_dump(level + 1, frm);
		return;
	case 127 + (17 << 7):
		channel_classification_dump(level + 1, frm);
		return;
	case 127 + (21 << 7):
	case 127 + (22 << 7):
		sniff_subrating_dump(level + 1, frm);
		return;
	case 127 + (25 << 7):
	case 127 + (26 << 7):
		io_capability_dump(level + 1, frm);
		return;
	case 127 + (30 << 7):
		keypress_notification_dump(level + 1, frm);
		return;
	}

	raw_dump(level, frm);
}
