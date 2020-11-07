/*
 * Broadcom Home Gateway Reference Design
 * Broadcom Passpoint Webpage functions
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
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
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: passpoint_gui.c 490618 2014-07-11 11:23:19Z $
 */

#ifdef WEBS
#include <webs.h>
#include <uemf.h>
#include <ej.h>
#else /* !WEBS */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <httpd.h>
#endif /* WEBS */

#include <bcmnvram.h>
#include <shutils.h>
#include <netconf.h>
#include <bcmcvar.h>
#include <802.11.h>
#include <dirent.h>

#include "confmtd_utils.h"
#include "common_utils.h"
#include "passpoint_enc_dec.h"
#include "passpoint_nvparse.h"
#include "passpoint_gui.h"

#ifdef __CONFIG_HSPOT__

/* ----------------------- Extern Variables and Functions Declarations  ------------------------ */
extern char *webs_buf;
extern int webs_buf_offset;
extern int ret_code;
extern char post_buf[POST_BUF_SIZE];
extern const char * const apply_header;
extern const char * const apply_footer;

extern inline void sys_reboot(void);
extern char* strncpy_n(char *destination, const char *source, size_t num);
extern int valid_range(webs_t wp, char *value, struct variable *v);
extern int valid_name(webs_t wp, char *value, struct variable *v);
extern int valid_choice(webs_t wp, char *value, struct variable *v);
extern int valid_hex(webs_t wp, char *value, struct variable *v);
extern int valid_octet(webs_t wp, char *value, struct variable *v);
extern char* make_wl_prefix(char *prefix, int prefix_size, int mode, char *ifname);
extern int copy_wl_index_to_unindex(webs_t wp, char_t *urlPrefix, char_t *webDir,
	int arg, char_t *url, char_t *path, char_t *query);

/* ----------------------- Extern Variables and Functions Declarations  ------------------------ */

/* --------------------------------- Constatnts & Macros  --------------------------------- */
#ifndef min
#define min(a, b)	(((a) < (b)) ? (a) : (b))
#endif // endif

#define	HTTP_ERR_BAD_REQUEST		400
#define IMAGE_HEADER_BYTE_OFFSET	24

/* Different Buffer Sizes */
#define NVNAME_BUFF			64
#define NVVAL_BUFF			2048
#define ITEM_BUFF			512
#define NAME_BUFF			256
#define CODE_BUFF			20
#define MAX_IMAGE_FILE_SIZE		15360

/* WEBS Strings */
#define WEBS_TAB			"<td></td>"
#define SELECTED_STR			"selected"
#define REDONLY_STR			"readonly"

/* Input Control Heading Strings */
#define IP_DSCP_VAL			"DSCP Value"
#define IP_UP_VAL			"UP Value"
#define IP_LOW_VAL			"Low value"
#define IP_HIGH_VAL			"High value"
#define IP_ENCODE_TYPE			"Encoding Type"
#define IP_NHRQ_NAME			"NAI Home Realm Query Name"
#define IP_OPER_NAME			"Operator Name"
#define IP_LANG_CODE			"Language Code"
#define IP_LINK_STATUS			"Link Status"
#define IP_SYMM_LINK			"Symmetric Link"
#define IP_AT_CAP			"At Capacity"
#define IP_DOWN_SPEED			"Down Link Speed"
#define IP_UP_SPEED			"Up Link Speed"
#define IP_NETAUTH_TYPE			"Network Authentication Type"
#define IP_REDIR_URL			"Redirection URL"
#define IP_REALM_NAME			"Realm Name"
#define IP_REALM_INFO			"Realm Information"
#define IP_VENUE_NAME			"Venue Name"
#define IP_OUI_NAME			"OUI Name"
#define IP_OUI_ISBEACON			"Is Beacon"
#define IP_MCC				"Country Code"
#define IP_MNC				"Network Code"
#define IP_CONCAP_PROTOCOL		"Connection Capability Protocol"
#define IP_CONCAP_PORT			"Connection Capability Port"
#define IP_CONCAP_STATUS		"Connection Capability Status"
#define IP_OSU_FRNDLY_NAME		"OSU Friendly Name"
#define IP_OSU_URI			"OSU URI"
#define IP_OSU_NAI			"OSU NAI"
#define IP_OSU_METHOD			"OSU Method"
#define IP_OSU_ICON			"OSU Icon"
#define IP_OSU_SERV_DESC		"OSU Service Description"

/* WAN Matrics */
#define WANMETRICS_FORMAT		"%s:%s:%s=%s>%s=%s>%s=%s"

/* --------------------------------- Constatnts & Macros  --------------------------------- */

/* ----------------------------------- ERROR Strings ------------------------------------ */
#define DSCP_ERR_EMPTY_RANGE		"DSCP Range cannot be empty"
#define DSCP_ERR_EMPTY_LOW_VAL		"Low value cannot be empty"
#define DSCP_ERR_EMPTY_HIGH_VAL		"High value cannot be empty"
#define DSCP_ERR_SPECF_LOW_VAL		"Specify a valid Low value"
#define DSCP_ERR_SPECF_HIGH_VAL		"Specify a valid High value"
#define DSCP_ERR_LOW_GR8ER_HIGH		"High Value should be >= Low Value"
#define DSCP_ERR_RANGE_OVRLAP		"DSCP Ranges should be Non-Overlapping"

#define HSERR_ICON_UPDATE		"Error during Icon Update<br>"
#define HSERR_ICON_SUCCESS		"Icon Update complete. Rebooting...<br>"
#define HSERR_ARGV_PRNT			"Insufficient args\n"
#define HSERR_INVALID_INPUT		"Invalid <b>%s</b>: Specify a valid %s<br>"
#define HSERR_OUI_MAX_3_BEACON		"Invalid <b>%s</b>: Max number of OUI in "\
					"beacon can be 3.<br>"
#define HSERR_EMPTY_INPUT		"Invalid <b>%s</b>: must specify a %s<br>"
#define HSERR_INVALID_DSCP_RANGE	"Invalid <b>DSCP Range # %d</b>: %s. "\
					"Making Range #%d as default 255 - 255.<br>"
#define HSERR_INVALID_DSCP_DUP		"Invalid <b>%s</b>: "\
					"Duplicate DSCP value in DSCP Execption<br>"
/* ----------------------------------- ERROR Strings ------------------------------------ */

/* -------------------------------- Control Names Strings --------------------------------- */
/* Control Names */
#define NVNM_WL_HSFLAG			"wl_hsflag"
#define NVNM_WL_WANMETRICS		"wl_wanmetrics"
#define NVNM_WL_HOMEQLIST		"wl_homeqlist"
#define NVNM_WL_OSU_FRNDNAME		"wl_osu_frndname"
#define NVNM_WL_OSU_FRNDNAME_HID	"wl_osu_frndname_hid"
#define NVNM_WL_OSU_URI			"wl_osu_uri"
#define NVNM_WL_OSU_NAI			"wl_osu_nai"
#define NVNM_WL_OSU_METHOD		"wl_osu_method"
#define NVNM_WL_OSU_ICONS		"wl_osu_icons"
#define NVNM_WL_OSU_ICONS_HID		"wl_osu_icons_hid"
#define NVNM_WL_OSU_SERVDESC		"wl_osu_servdesc"
#define NVNM_WL_QOSMAPIE		"wl_qosmapie"
#define NVNM_WL_CONCAPLIST		"wl_concaplist"
#define NVNM_WL_NETAUTHLIST		"wl_netauthlist"
#define NVNM_WL_VENUELIST		"wl_venuelist"
#define NVNM_WL_OUILIST			"wl_ouilist"
#define NVNM_WL_3GPPLIST		"wl_3gpplist"
#define NVNM_WL_REALMLIST		"wl_realmlist"
/* -------------------------------- Control Names Strings --------------------------------- */

/* -------------------------------- Structure Definitions ----------------------------------- */

/* Structure to hold Venue Groups & Display Strings */
typedef struct {
	int vanue_group_code;
	int vanue_type_count;
	int vanue_type_offset;
	char* vanue_group_description;
} vanue_group_assignments;

/* Structure to hold Venue Types for each Venue Group & Display Strings */
typedef struct {
	int vanue_group_code;
	int vanue_type_code;
	char* vanue_type_description;
} vanue_type_assignments;

/* Structure to hold EAP Method ID & Display Strings */
typedef struct {
	uint8 eap_method_val;
	char* eap_method_name;
} eap_method_assignments;

/* Structure to hold Auth ID & Display Strings */
typedef struct {
	int auth_id_code;
	int auth_param_count;
	int auth_param_offset;
	char* auth_id_description;
} auth_id_assignments;

/* Structure to hold Auth Params for each Auth ID & Display Strings */
typedef struct {
	int auth_id_code;
	int auth_param_code;
	char* auth_param_description;
} auth_param_assignments;

/* Structure to hold the icon file names and language code */
typedef struct {
	char icon_filename[BCM_DECODE_HSPOT_ANQP_MAX_ICON_FILENAME_LENGTH + 1];
	char icon_lang[VENUE_LANGUAGE_CODE_SIZE + 1];
} icon_data_t;

/* Structure to hold the icon filelist */
typedef struct {
	int file_count;
	icon_data_t icons[CODE_BUFF];
} icon_list_t;
/* -------------------------------- Structure Definitions ----------------------------------- */

/* ---------------------------------- Global Variables ------------------------------------- */
#define VENUE_GROUP_COUNT			13
vanue_group_assignments vanue_groups[] = {
	{0, 2, 0, "Unspecified"},
	{1, 17, 2, "Assembly"},
	{2, 10, 19, "Business"},
	{3, 5, 29, "Educational"},
	{4, 2, 34, "Factory and Industrial"},
	{5, 7, 36, "Institutional"},
	{6, 7, 43, "Mercantile"},
	{7, 6, 50, "Residential"},
	{8, 2, 56, "Storage"},
	{9, 2, 58, "Utility and Miscellaneous"},
	{10, 9, 60, "Vehicular"},
	{11, 8, 69, "Outdoor"},
	{12, 1, 77, "Reserved"}
};

vanue_type_assignments vanue_types[] = {
	{0, 0, "Unspecified"},
	{0, 1, "Reserved"},

	{1, 0, "Unspecified Assembly"},
	{1, 1, "Areana"},
	{1, 2, "Stadium"},
	{1, 3, "Passage Terminal(e.g., airport, bus, ferry, train station)"},
	{1, 4, "Amphitheter"},
	{1, 5, "Amusement Park"},
	{1, 6, "Place of Worship"},
	{1, 7, "Convention Center"},
	{1, 8, "Library"},
	{1, 9, "Museum"},
	{1, 10, "Restaurant"},
	{1, 11, "Theater"},
	{1, 12, "Bar"},
	{1, 13, "Coffee Shop"},
	{1, 14, "Zoo or Aquarium"},
	{1, 15, "Emergency Coordination Center"},
	{1, 16, "Reserved"},

	{2, 0, "Unspecified Business"},
	{2, 1, "Doctor or Dentist office"},
	{2, 2, "Bank"},
	{2, 3, "Fire Station"},
	{2, 4, "Police Station"},
	{2, 6, "Post Office"},
	{2, 7, "Professional Office"},
	{2, 8, "Research and Development Facility"},
	{2, 9, "Attorney Office"},
	{2, 10, "Reserved"},

	{3, 0, "Unspecified Educational"},
	{3, 1, "School, Primary"},
	{3, 2, "School, Secondary"},
	{3, 3, "University or College"},
	{3, 4, "Reserved"},

	{4, 0, "Unspecified Factory and Industrial"},
	{4, 1, "Reserved"},

	{5, 0, "Unspecified Institutional"},
	{5, 1, "Hospital"},
	{5, 2, "Long Term Care Facility(e.g., Nursing home, Hospice, etc.)"},
	{5, 3, "Alcohol and Drug Rehabilitation Center"},
	{5, 4, "Group Home"},
	{5, 5, "Prison or Jail"},
	{5, 6, "Reserved"},

	{6, 0, "Unspecified Mercantile"},
	{6, 1, "Retail Store"},
	{6, 2, "Grocery Market"},
	{6, 3, "Automative Service Station"},
	{6, 4, "Shopping Mall"},
	{6, 5, "Gas Station"},
	{6, 6, "Reserved"},

	{7, 0, "Unspecified Residential"},
	{7, 1, "Private Residence"},
	{7, 2, "Hotel or Motel"},
	{7, 3, "Dormitory"},
	{7, 4, "Boarding House"},
	{7, 5, "Reserved"},

	{8, 0, "Unspecified Storage"},
	{8, 1, "Reserved"},

	{9, 0, "Unspecified Utility and Miscellaneous"},
	{9, 1, "Reserved"},

	{10, 0, "Unspecified Vehicular"},
	{10, 1, "Automobile or Truck"},
	{10, 2, "Airplane"},
	{10, 3, "Bus"},
	{10, 4, "Ferry"},
	{10, 5, "Ship or Boat"},
	{10, 6, "Train"},
	{10, 7, "Motor Bike"},
	{10, 8, "Reserved"},

	{11, 0, "Unspecified Outdoor"},
	{11, 1, "Muni-mesh Network"},
	{11, 2, "City Park"},
	{11, 3, "Rest Area"},
	{11, 4, "Traffic Control"},
	{11, 5, "Bus Stop"},
	{11, 6, "Kiosk"},
	{11, 7, "Reserved"},

	{12, 0, "Reserved"}
};

#define EAP_METHOD_COUNT	11
eap_method_assignments eap_methods[] = {
	{-1, "Select"},
	{13, "EAP-TLS"},
	{17, "EAP-LEAP"},
	{18, "EAP-SIM"},
	{21, "EAP-TTLS"},
	{23, "EAP-AKA"},
	{25, "EAP-PEAP"},
	{43, "EAP-FAST"},
	{47, "EAP-PSK"},
	{50, "EAP-AKAP"},
	{254, "EAP-EXTENDED"}
};

#define AUTH_ID_COUNT		4
auth_id_assignments auth_ids[] = {
	{2, 5, 0, "Non-EAP Inner Auth Method"},
	{3, 8, 5, "Inner Auth Type"},
	{5, 11, 13, "Credential Type"},
	{6, 11, 24, "Tunnle EAP Method Type"},
};

auth_param_assignments auth_params[] = {
	{2, -1, "Select"},
	{2, 1, "PAP"},
	{2, 2, "CHAP"},
	{2, 3, "MSCHAP"},
	{2, 4, "MSCHAPV2"},

	{3, -1, "Select"},
	{3, 17, "LEAP"},
	{3, 25, "PEAP"},
	{3, 13, "EAP-TLS"},
	{3, 43, "EAP-FAST"},
	{3, 18, "EAP-SIM"},
	{3, 21, "EAP-TTLS"},
	{3, 23, "EAP-AKA"},

	{5, -1, "Select"},
	{5, 1, "SIM"},
	{5, 2, "USIM"},
	{5, 3, "NFC Secure Element"},
	{5, 4, "Hardware Token"},
	{5, 5, "Soft Token"},
	{5, 6, "Certificate"},
	{5, 7, "Username Password"},
	{5, 8, "Reserved"},
	{5, 9, "Anonymous"},
	{5, 10, "Vendor Specific"},

	{6, -1, "Select"},
	{6, 1, "SIM"},
	{6, 2, "USIM"},
	{6, 3, "NFC Secure Element"},
	{6, 4, "Hardware Token"},
	{6, 5, "Soft Token"},
	{6, 6, "Certificate"},
	{6, 7, "Username Password"},
	{6, 8, "Reserved"},
	{6, 9, "Anonymous"},
	{6, 10, "Vendor Specific"}
};

/* Array for holding icon file names */
icon_list_t g_icon_files;
/* ---------------------------------- Global Variables ------------------------------------ */

/* ========================= UTILITY FUNCTIONS ============================= */
static int
delete_icon_file(char *filename)
{
	assert(filename);

	char fname[NAME_BUFF];
	ret_code = 1;
	sprintf(fname, USERDEFINED_ICONPATH"%s", filename);
	if (remove(fname) == -1)
		ret_code = -1;
	return ret_code;
}

static int
validate_icon(char *filename)
{
	assert(filename);

	/* From File Extension decide, if valid Image File */
	char *pch = strrchr(filename, '.');
	if (pch != NULL) {
		if (!strcmp(pch, EXT_BMP) ||
			!strcmp(pch, EXT_DIB) ||
			!strcmp(pch, EXT_RLE) ||
			!strcmp(pch, EXT_JPG) ||
			!strcmp(pch, EXT_JPEG) ||
			!strcmp(pch, EXT_JPE) ||
			!strcmp(pch, EXT_JFIF) ||
			!strcmp(pch, EXT_GIF) ||
			!strcmp(pch, EXT_EMF) ||
			!strcmp(pch, EXT_WMF) ||
			!strcmp(pch, EXT_TIF) ||
			!strcmp(pch, EXT_TIFF) ||
			!strcmp(pch, EXT_PNG) ||
			!strcmp(pch, EXT_ICO))
				return 1;
	}
	return 0;
}

static int
get_icon_files(char* path, icon_list_t *out_list)
{
	assert(path);
	assert(out_list);

	DIR* FD;
	struct dirent* in_file;
	int iter_icon = 0, iter_path = 0;
	char *tokenParse = NULL, *saveptr = NULL;

	memset(&g_icon_files, 0, sizeof(g_icon_files));

	/* Parse Directory Paths */
	while ((iter_icon < CODE_BUFF) &&
		((tokenParse = strtok_r(iter_path ? NULL : path, ";", &saveptr)) != NULL)) {

		/* Open the Directory */
		FD = opendir(tokenParse);

		/* Read Filenames from the Directory */
		while ((iter_icon < CODE_BUFF) &&
			(FD && ((in_file = readdir(FD)) != NULL))) {

			/* Validate Icon Filetype */
			if (validate_icon(in_file->d_name)) {

			strncpy_n(out_list->icons[iter_icon].icon_filename,
				in_file->d_name,
				BCM_DECODE_HSPOT_ANQP_MAX_ICON_FILENAME_LENGTH +1);
			strncpy_n(out_list->icons[iter_icon].icon_lang,
				strstr(in_file->d_name, "_zxx.") ?
				LANG_ZXX : ENGLISH, VENUE_LANGUAGE_CODE_SIZE +1);

			/* Fill Global Icon Filename Array as well */
			strncpy_n(g_icon_files.icons[iter_icon].icon_filename,
				out_list->icons[iter_icon].icon_filename,
				BCM_DECODE_HSPOT_ANQP_MAX_ICON_FILENAME_LENGTH +1);
			strncpy_n(g_icon_files.icons[iter_icon].icon_lang,
				out_list->icons[iter_icon].icon_lang,
				VENUE_LANGUAGE_CODE_SIZE +1);

			iter_icon++;
			}
		}

		out_list->file_count = iter_icon;

		/* Close the Directory */
		if (FD)
			closedir(FD);

		iter_path++;

	}

	return out_list->file_count;
}
/* ========================= UTILITY FUNCTIONS ============================= */

/* ====================== MIME HANDLER FUNCTIONS =========================== */
void
do_uploadIcons_post(char *url, FILE *stream, int len, char *boundary)
{
	assert(url);
	assert(stream);

	FILE *outfp = NULL;
	char *iconfilename = url, *ptr = NULL;
	char fname[NAME_BUFF], fpath[NAME_BUFF], *fnamep = NULL, *streambuf = NULL;
	char *tokenparse = NULL, *saveptr = NULL, *ch = NULL, *hdr = NULL, *bndry = NULL;
	int count = 0, total = 0, actual_size = 0, iter_file = 0;
	ret_code = 0;

	/* perform the delete operation if any. */
	ptr = strsep(&iconfilename, "?");
	if (iconfilename != NULL) {
		/* Start deleting the icon file */
		while ((tokenparse =
			strtok_r(iter_file ? NULL : iconfilename, ";", &saveptr)) != NULL) {
			ret_code = delete_icon_file(tokenparse);
			iter_file++;
		}
	}

	/* Get query */
	/* Look for our part */
	if (!fgets(post_buf, min(len + 1, sizeof(post_buf)), stream))
		goto DO_UPLOADICON_POST_DONE;
	len -= strlen(post_buf);

	streambuf = (char *)malloc(len);
	if (!streambuf)
		goto DO_UPLOADICON_POST_DONE;

	total = len;
	while ((count = safe_fread(streambuf, 1, len, stream))) {
		total -= count;
		if (total <= 0)
			break;
	}

	while ((ch = strstr(streambuf, "filename=\"")) != NULL) {
		outfp = NULL, fnamep = NULL, hdr = NULL, bndry = NULL;
		memset(fname, 0, sizeof(fname));
		memset(fpath, 0, sizeof(fpath));
		actual_size = 0;

		/* Search for Next File */
		fnamep = ch + strlen("filename=\"");
		memset(streambuf, 'a', fnamep-streambuf);
		ch = strstr(streambuf, "\"");
		memcpy(fname, fnamep, ch-fnamep);
		fname[ch-fnamep] = '\0';
		memset(fnamep, 'a', strlen(fname) + CODE_BUFF);

		if (strlen(fname)) {
			/* Validate Icon Filetype */
			if (!validate_icon(fname))
				continue;

			/* Open File */
			sprintf(fpath, USERDEFINED_ICONPATH"%s", fname);
			outfp = fopen(fpath, "wb");
			if (!outfp)
				goto DO_UPLOADICON_POST_DONE;

			/* Find Header */
			hdr = strstr(streambuf, "\r\n");
			if (hdr) {
				memset(streambuf, 'a', hdr-streambuf);
				hdr += 4; /* +4 for 2 CR,LF */
			}

			/* Find Boundary */
			if ((bndry = (char *)memmem(streambuf, len,
				boundary, strlen(boundary))) != NULL) {
				bndry -= 4; /* -4 for 2 CR,LF */
				actual_size = bndry - hdr;
				if (actual_size > MAX_IMAGE_FILE_SIZE) {
					ret_code = -1;
					goto DO_UPLOADICON_POST_DONE;
				}
			}

			/* Write Image Data */
			if (hdr)
				safe_fwrite(hdr, 1, actual_size, outfp);

			/* Close File */
			if (outfp) {
				fclose(outfp);
				outfp = NULL;
			}
			memset(streambuf, 'a', bndry-streambuf);
		}
	}

DO_UPLOADICON_POST_DONE:
	confmtd_backup();
	if (outfp) {
		fclose(outfp);
		outfp = NULL;
	}
	if (streambuf) {
		free(streambuf);
		streambuf = NULL;
	}
}

void
do_uploadIcons_cgi(char *url, FILE *stream)
{
	assert(url);
	assert(stream);
	websHeader(stream);
	websWrite(stream, (char_t *) apply_header);
	if (ret_code)
		websWrite(stream, HSERR_ICON_UPDATE);
	else
		websWrite(stream, HSERR_ICON_SUCCESS);

	websWrite(stream, (char_t *) apply_footer, "passpoint.asp");
	websFooter(stream);
	websDone(stream, 200);

	/* Reboot if successful */
	if (ret_code == 0)
		sys_reboot();
}

void
do_passpoint_asp(char *url, FILE *stream)
{
	char *path = NULL, *query = NULL;

	assert(stream);
	assert(url);

	/* Parse path */
	query = url;
	path = strsep(&query, "?") ? : url;

	copy_wl_index_to_unindex(stream, NULL, NULL, 0, url, path, query);

	/* Reset CGI */
	init_cgi(NULL);
}
/* ====================== MIME HANDLER FUNCTIONS =========================== */

/* ==================== PRINT & VALIDATE FUNCTIONS ========================= */
/* ---------------------- 0. Passpoint hsflag - VALIDATE ------------------- */
void
validate_wl_hsflag(webs_t wp, char *value, struct variable *v, char *varname)
{
	struct variable fields[] = {
		{ name: "wl_hs2en",
		longname:"BSS Passpoint Enable",
		argv: ARGV("0", "1") },

		{ name: "wl_proxyarp",
		longname:"Proxy ARP Enable",
		argv: ARGV("0", "1") },

		{ name: "wl_u11en",
		longname:"BSS 802.11u Interworking Status",
		argv: ARGV("0", "1") },

		{ name: "wl_iwint",
		longname:"Internet Access",
		argv: ARGV("0", "1") },

		{ name: "wl_iwasra",
		longname:"ASRA Access",
		argv: ARGV("0", "1") },

		{ name: "wl_p2pie",
		longname:"P2P IE",
		argv: ARGV("0", "1") },

		{ name: "wl_p2pcross",
		longname:"P2P Cross",
		argv: ARGV("0", "1") },

		{ name: "wl_osenen",
		longname:"OSU OSEN Security",
		argv: ARGV("0", "1") },

		{ name: "wl_dgaf",
		longname:"DGAF",
		argv: ARGV("0", "1") }
	};

	/* bitflags array should be in the same order as fields array */
	int bitflags[] = {
		HSFLG_HS_EN,
		HSFLG_PROXY_ARP,
		HSFLG_U11_EN,
		HSFLG_IWINT_EN,
		HSFLG_IWASRA_EN,
		HSFLG_P2P,
		HSFLG_P2P_CRS,
		HSFLG_OSEN,
		HSFLG_DGAF_DS
	};

	int idx = 0, total_size = 9;
	char* val = NULL;

	assert(v);

	for (idx = 0; idx < total_size; idx++) {
		if ((val = websGetVar(wp, fields[idx].name, NULL)) != NULL)
			nvram_set_bitflag("wl_hsflag", bitflags[idx], atoi(val));
	}

	ret_code = 0;
	return;
}

/* ------------- 1. Network Authentication Type List - PRINT --------------- */
int
ej_print_wl_netauthlist(int eid, webs_t wp, int argc, char_t **argv)
{
	bcm_decode_anqp_network_authentication_type_t *nat = NULL;

	int iter, last, ret = 0, showURL = 0;
	bool valid = FALSE, noNvram = FALSE;
	char prefix[] = PREFIX_DEF;

	/* Parse Args */
	if (ejArgs(argc, argv, "%d %d", &iter, &last) < 2) {
		websError(wp, HTTP_ERR_BAD_REQUEST, HSERR_ARGV_PRNT);
		return -1;
	}

	/* Make Prefix */
	if (!make_wl_prefix(prefix, sizeof(prefix), 1, NULL))
		return -1;

	/* Parse NVRAM */
	nat = (bcm_decode_anqp_network_authentication_type_t*)
		malloc(sizeof(bcm_decode_anqp_network_authentication_type_t));
	if (!nat)
		return -1;
	memset(nat, 0, sizeof(*nat));
	if (!decode_u11_netauth_list(prefix, nat))
		noNvram = TRUE;

	/* Start Printing */
	for (; iter <= last; iter++) {

		/* Evaluate if entry is valid */
		valid = ((iter < nat->numAuthenticationType) && !noNvram)? TRUE : FALSE;
		showURL = (valid && (nat->unit[iter].urlLen > 0)) ? TRUE : FALSE;

		/* Printing Header */
		ret += websWrite(wp, "<tr>");
		ret += websWrite(wp, "<th width=\"310\" valign=\"top\" rowspan=\"1\"");
		ret += websWrite(wp, "onMouseOver=\"return overlib('Set Network Authentication Type"
			" List to the BSS supported by the interface.', LEFT);\"");
		ret += websWrite(wp, "onMouseOut=\"return nd();\">");
		ret += websWrite(wp, "<input type=\"hidden\" name=\"wl_netauthlist_%d_\" "
			"value=\"1\">", iter);
		ret += websWrite(wp, " &nbsp;&nbsp;");
		ret += websWrite(wp, "</th>");
		ret += websWrite(wp, "<td>&nbsp;&nbsp;</td>");

		/* Print nettype */
		ret += websWrite(wp, "<td><select id=\"wl_netauthlist_%d_0\" "
		"name=\"wl_netauthlist_%d_0\" "
		"onChange=\"onchange_netauthtype(%d)\"> "
		"<option value=\"na\" %s>Not Configured</option> "
		"<option value=\"accepttc\" %s>Acceptance of Terms and Conditions</option> "
		"<option value=\"online\" %s>Online Enrollment</option> "
		"<option value=\"httpred\" %s>HTTP/HTTPS Redirection</option> "
		"<option value=\"dnsred\" %s>DNS Redirection</option> "
		"</select></td>", iter, iter, iter,
		(valid ? (nat->unit[iter].type == (uint8)NATI_UNSPECIFIED ?
		SELECTED_STR : EMPTY_STR) : SELECTED_STR),
		(valid ? (nat->unit[iter].type == (uint8)NATI_ACCEPTANCE_OF_TERMS_CONDITIONS ?
		SELECTED_STR : EMPTY_STR) : EMPTY_STR),
		(valid ? (nat->unit[iter].type == (uint8)NATI_ONLINE_ENROLLMENT_SUPPORTED ?
		SELECTED_STR : EMPTY_STR) : EMPTY_STR),
		(valid ? (nat->unit[iter].type == (uint8)NATI_HTTP_HTTPS_REDIRECTION ?
		SELECTED_STR : EMPTY_STR) : EMPTY_STR),
		(valid ? (nat->unit[iter].type == (uint8)NATI_DNS_REDIRECTION ?
		SELECTED_STR : EMPTY_STR) : EMPTY_STR));

		ret += websWrite(wp, WEBS_TAB);

		/* Print redirecturl */
		ret += websWrite(wp, "<td><input id=\"wl_netauthlist_%d_1\" "
		"name=\"wl_netauthlist_%d_1\" value=\"%s\" size=\"32\" "
		"maxlength=\"128\" %s ></td>", iter, iter,
		valid ? (char*)nat->unit[iter].url : EMPTY_STR, showURL ? EMPTY_STR : REDONLY_STR);

		ret += websWrite(wp, "</tr>");
	}

	/* Clean up */
	if (nat) {
		free(nat);
		nat = NULL;
	}

	return ret;
}
REG_EJ_HANDLER(print_wl_netauthlist);
/* ------------ 1. Network Authentication Type List - VALIDATE ------------- */
void
validate_wl_netauthlist(webs_t wp, char *value, struct variable *v, char *varname)
{
	struct variable fields[] = {
		{ name: "wl_netauthlist_%d_%d",
		longname: IP_NETAUTH_TYPE,
		argv: ARGV(NATI_NA, NATI_ACCEPTTC, NATI_ONLINE, NATI_HTTPRED, NATI_DNSRED) },

		{ name: "wl_netauthlist_%d_%d",
		longname: IP_REDIR_URL,
		argv: ARGV("0", "128") },
	};

	int count, iter;
	char nvram_val[NVVAL_BUFF], item_value[ITEM_BUFF];
	char row_name[NVNAME_BUFF], col_name[NVNAME_BUFF];
	char* val = NULL;

	char redirecturl[NAME_BUFF], nettype[CODE_BUFF];

	assert(v);

	ret_code = EINVAL;

	if (!value) {
		ret_code = 0;
		return;
	}
	count = atoi(value);

	memset(nvram_val, 0, sizeof(nvram_val));

	for (iter = 0; iter < count; iter++) {

		memset(row_name, 0, sizeof(row_name));
		snprintf(row_name, sizeof(row_name), "%s_%d_", NVNM_WL_NETAUTHLIST, iter);

		memset(col_name, 0, sizeof(col_name));
		memset(nettype, 0, sizeof(nettype));
		if ((val = websGetVar(wp, strcat_r(row_name, "0", col_name), NULL)) != NULL)
			if (!strcmp(val, NATI_NA) || !strcmp(val, NATI_ACCEPTTC) ||
				!strcmp(val, NATI_ONLINE) || !strcmp(val, NATI_HTTPRED) ||
				!strcmp(val, NATI_DNSRED))
				strncpy_n(nettype, val, CODE_BUFF);

		memset(col_name, 0, sizeof(col_name));
		memset(redirecturl, 0, sizeof(redirecturl));
		if ((val = websGetVar(wp, strcat_r(row_name, "1", col_name), NULL)) != NULL)
			strncpy_n(redirecturl, val, NAME_BUFF);

		/* Delete entry if all fields are blank */
		if (!strcmp(nettype, NATI_NA)) {
			continue;
		}

		/* Check individual fields */
		if (!valid_choice(wp, nettype, &fields[0])) {
			websBufferWrite(wp, HSERR_INVALID_INPUT,
				fields[0].longname, fields[0].longname);
			continue;
		}
		if (!strcmp(nettype, NATI_HTTPRED) || !strcmp(nettype, NATI_DNSRED)) {
			if (!valid_name(wp, redirecturl, &fields[1])) {
				websBufferWrite(wp, HSERR_INVALID_INPUT,
					fields[1].longname, fields[1].longname);
				continue;
			}
		}

		/* Do it */
		memset(item_value, 0, sizeof(item_value));
		snprintf(item_value, sizeof(item_value), "%s=%s", nettype, redirecturl);

		if (strlen(nvram_val))
			strncat(nvram_val, "+", min(1, NVVAL_BUFF-strlen(nvram_val)));

		strncat(nvram_val, item_value, min(strlen(item_value),
			NVVAL_BUFF-strlen(nvram_val)));
	}

	nvram_set(NVNM_WL_NETAUTHLIST, nvram_val);

	ret_code = 0;
	return;
}
/* ------------------ 2. Realm List - AuthID - ONCHANGE -------------------- */
int
ej_authid_change(int eid, webs_t wp, int argc, char_t** argv)
{
	int authidcode, ret = 0, iter;

	/* Parse Args */
	if (ejArgs(argc, argv, "%d", &authidcode) < 1) {
		websError(wp, HTTP_ERR_BAD_REQUEST, HSERR_ARGV_PRNT);
		ret = -1;
	}

	/* Create HTML stringArray for Auth Param Name, according to Auth ID */
	websWrite(wp, "\t\t var authTypes = new Array(");
	for (iter = auth_ids[authidcode].auth_param_offset;
		iter < auth_ids[authidcode].auth_param_offset +
			auth_ids[authidcode].auth_param_count;
		iter++) {
			websWrite(wp, "\"%s\"", auth_params[iter].auth_param_description);
			if (iter < (auth_ids[authidcode].auth_param_offset +
				auth_ids[authidcode].auth_param_count - 1))
				websWrite(wp, ", ");
		}
	websWrite(wp, ");\n");

	/* Create HTML stringArray for Auth Param ID, according to Auth ID */
	websWrite(wp, "\t\t var authTypesCode = new Array(");
	for (iter = auth_ids[authidcode].auth_param_offset;
		iter < auth_ids[authidcode].auth_param_offset +
			auth_ids[authidcode].auth_param_count;
		iter++) {
			websWrite(wp, "\"%d\"", auth_params[iter].auth_param_code);
			if (iter < (auth_ids[authidcode].auth_param_offset +
				auth_ids[authidcode].auth_param_count - 1))
				websWrite(wp, ", ");
		}

	websWrite(wp, ");\n");

	return ret;
}
REG_EJ_HANDLER(authid_change);
/* --------------------- 2. Realm List - POPUP PRINT ----------------------- */
int
ej_print_popup_realm(int eid, webs_t wp, int argc, char **argv)
{
	bcm_decode_anqp_nai_realm_list_ex_t *nat = NULL;
	bcm_decode_anqp_nai_realm_data_ex_t *realm_nat = NULL;

	int ret = 0, selected_realm = 0, last = 0;
	int iter_eap = 0, iter_auth = 0, index_eap = 0, index_auth = 0, index_param = 0;
	int valid_eap = 0, valid_auth = 0, selected_auth_id = 0;
	char prefix[] = PREFIX_DEF;
	bool noNvram = FALSE;

	/* Parse Args */
	if (ejArgs(argc, argv, "%d%d%d", &selected_realm, &iter_eap, &last) < 1) {
		websError(wp, HTTP_ERR_BAD_REQUEST, HSERR_ARGV_PRNT);
		return -1;
	}

	/* Make Prefix */
	if (!make_wl_prefix(prefix, sizeof(prefix), 1, NULL))
		return -1;

	/* Parse NVRAM */
	nat = (bcm_decode_anqp_nai_realm_list_ex_t*)
		malloc(sizeof(bcm_decode_anqp_nai_realm_list_ex_t));
	if (!nat)
		return -1;
	memset(nat, 0, sizeof(*nat));
	if (!decode_u11_realm_list(prefix, nat))
		noNvram = TRUE;
	else
		realm_nat = &nat->realm[selected_realm];

	/* Start Printing */
	websWrite(wp, "var test = new String(\"");
	websWrite(wp, "<table id=\'eaplisttblid\'>");
	websWrite(wp, "<tr>");
	websWrite(wp, "<td>Eap Name</td>");
	websWrite(wp, "<td>Auth Id</td>");
	websWrite(wp, "<td>Auth Param</td>");
	websWrite(wp, "</tr>");

	/* Print Eap & Auth for selected Realm */
	for (; iter_eap <= last; iter_eap++) {

		/* Evaluate if entry is valid */
		valid_eap = (!noNvram) ? ((iter_eap < realm_nat->eapCount) ? TRUE : FALSE) : 0;
		valid_auth = valid_eap ? realm_nat->eap[iter_eap].authCount : 0;
		selected_auth_id = 0;

		websWrite(wp, "<tr>");
		websWrite(wp, "<td><select id=\'eap_method_%d\' name=\'eap_method_%d\'>",
			iter_eap, iter_eap);

		/* Print Eap Method pulldown */
		for (index_eap = 0; index_eap < EAP_METHOD_COUNT; index_eap++) {

			websWrite(wp, "<option value=\'%d\' %s>%s</option>",
				eap_methods[index_eap].eap_method_val,
				(valid_eap ? (realm_nat->eap[iter_eap].eapMethod ==
				(uint8)eap_methods[index_eap].eap_method_val ?
				SELECTED_STR : EMPTY_STR) : EMPTY_STR),
				eap_methods[index_eap].eap_method_name);
		}

		websWrite(wp, "</select></td>");
		websWrite(wp, "<td><select id=\'auth_id_%d\' name=\'auth_id_%d\' "
			"onChange=\'window.opener.onchange_authid(%d,%d);\'>",
			iter_eap, iter_eap, iter_eap, 1000);

		/* Print 1st Auth ID pulldown */
		for (index_auth = 0; index_auth < AUTH_ID_COUNT; index_auth++) {

		iter_auth = 0;
		websWrite(wp, "<option value=\'%d\' %s>%s</option>",
			auth_ids[index_auth].auth_id_code,
			(valid_auth ? (realm_nat->eap[iter_eap].auth[iter_auth].id ==
			(uint8)auth_ids[index_auth].auth_id_code ?
			SELECTED_STR : EMPTY_STR) : EMPTY_STR),
			auth_ids[index_auth].auth_id_description);

		if (realm_nat->eap[iter_eap].auth[iter_auth].id ==
			(uint8)auth_ids[index_auth].auth_id_code)
			selected_auth_id = index_auth;
		}
		websWrite(wp, "</select></td>");

		/* Print 1st Auth Param pulldown */
		websWrite(wp, "<td><select id=\'auth_param_%d\' name=\'auth_param_%d\'>",
		iter_eap, iter_eap);

		for (index_param = auth_ids[selected_auth_id].auth_param_offset;
			index_param < auth_ids[selected_auth_id].auth_param_offset +
				auth_ids[selected_auth_id].auth_param_count;
			index_param++) {

		iter_auth = 0;
		websWrite(wp, "<option value=\'%d\' %s>%s</option>",
		auth_params[index_param].auth_param_code,
		(valid_auth ? (auth_params[index_param].auth_param_code ==
		(uint8)realm_nat->eap[iter_eap].auth[iter_auth].value[0] ?
		SELECTED_STR : EMPTY_STR) : EMPTY_STR),
		auth_params[index_param].auth_param_description);
		}

		valid_auth--;

		websWrite(wp, "</select></td>");
		websWrite(wp, "</tr>");

		/* Print remaining(2nd, 3rd) Auth ID and Auth Param */
		for (iter_auth = 1; iter_auth < last; iter_auth++) {

		selected_auth_id = 0;

		/* Print remaining(2nd, 3rd) Auth ID */
		websWrite(wp, "<tr><td></td>");
		websWrite(wp, "<td><select id=\'auth_id_%d_%d\' name=\'auth_id_%d_%d\' "
		"onChange=\'window.opener.onchange_authid(%d,%d);\'>",
		iter_eap, iter_auth, iter_eap, iter_auth, iter_eap, iter_auth);

		for (index_auth = 0; index_auth < AUTH_ID_COUNT; index_auth++) {

		websWrite(wp, "<option value=\'%d\' %s>%s</option>",
			auth_ids[index_auth].auth_id_code,
			(valid_auth ? (realm_nat->eap[iter_eap].auth[iter_auth].id ==
			(uint8)auth_ids[index_auth].auth_id_code ?
			SELECTED_STR : EMPTY_STR) : EMPTY_STR),
			auth_ids[index_auth].auth_id_description);

		if (realm_nat->eap[iter_eap].auth[iter_auth].id ==
			(uint8)auth_ids[index_auth].auth_id_code)
			selected_auth_id = index_auth;
		}
		websWrite(wp, "</select></td>");

		/* Print remaining(2nd, 3rd) Auth Param */
		websWrite(wp, "<td><select id=\'auth_param_%d_%d\' name=\'auth_param_%d_%d\'>",
		iter_eap, iter_auth, iter_eap, iter_auth);

		for (index_param = auth_ids[selected_auth_id].auth_param_offset;
			index_param < auth_ids[selected_auth_id].auth_param_offset +
				auth_ids[selected_auth_id].auth_param_count;
			index_param++)
			websWrite(wp, "<option value=\'%d\' %s>%s</option>",
			auth_params[index_param].auth_param_code,
			(valid_auth ? (auth_params[index_param].auth_param_code ==
			(uint8)realm_nat->eap[iter_eap].auth[iter_auth].value[0] ?
			SELECTED_STR : EMPTY_STR) : EMPTY_STR),
			auth_params[index_param].auth_param_description);
		websWrite(wp, "</select></td>");
		websWrite(wp, "</tr>");

		valid_auth--;

		}
	}

	/* Print Save & Cancel Buttons */
	websWrite(wp, "<tr><td></td><td></td><td></td></tr>"
		"<tr><td></td><td></td><td></td></tr>");
	websWrite(wp, "<tr><td></td><td></td><td><input "
		"type=button value=Save onClick=\'window.opener.onsave_eapauth();\'>");
	websWrite(wp, "<input type=button value=Cancel "
		"onClick=\'window.opener.onclose_eapauth();\'></td><tr>");
	websWrite(wp, "</table>");
	websWrite(wp, "\");\n");

	/* Clean up */
	if (nat) {
		free(nat);
		nat = NULL;
	}

	return ret;
}
REG_EJ_HANDLER(print_popup_realm);
/* ------------------------- 2. Realm List - PRINT ------------------------- */
int
ej_print_wl_realmlist(int eid, webs_t wp, int argc, char **argv)
{
	bcm_decode_anqp_nai_realm_list_ex_t *nat = NULL;

	int iter, last, ret = 0;
	bool valid = FALSE, noNvram = FALSE;
	char prefix[] = PREFIX_DEF;

	/* Parse Args */
	if (ejArgs(argc, argv, "%d %d", &iter, &last) < 2) {
		websError(wp, HTTP_ERR_BAD_REQUEST, HSERR_ARGV_PRNT);
		return -1;
	}

	/* Make Prefix */
	if (!make_wl_prefix(prefix, sizeof(prefix), 1, NULL))
		return -1;

	/* Parse NVRAM */
	nat = (bcm_decode_anqp_nai_realm_list_ex_t*)
		malloc(sizeof(bcm_decode_anqp_nai_realm_list_ex_t));
	if (!nat)
		return -1;
	memset(nat, 0, sizeof(*nat));
	if (!decode_u11_realm_list(prefix, nat))
		noNvram = TRUE;

	/* Start Printing */
	for (; iter <= last; iter++) {

		/* Evaluate if entry is valid */
		valid = ((iter < nat->realmCount) && !noNvram)? TRUE : FALSE;

		/* Printing Header */
		ret += websWrite(wp, "<tr>");
		ret += websWrite(wp, "<th width=\"310\" valign=\"top\" rowspan=\"1\"");
		ret += websWrite(wp, "onMouseOver=\"return overlib('Set Realm List "
			"to the BSS supported by the interface.', LEFT);\"");
		ret += websWrite(wp, "onMouseOut=\"return nd();\">");
		ret += websWrite(wp, "<input type=\"hidden\" name=\"wl_realmlist_%d_\" "
			"value=\"4\">", iter);
		ret += websWrite(wp, " &nbsp;&nbsp;");
		ret += websWrite(wp, "</th>");
		ret += websWrite(wp, "<td>&nbsp;&nbsp;</td>");

		/* Print realmname */
		ret += websWrite(wp, "<td><input id=\"wl_realmlist_%d_0\" "
		"name=\"wl_realmlist_%d_0\" value=\"%s\" size=\"32\" maxlength=\"255\"></td>",
		iter, iter, valid ? (char*)nat->realm[iter].realm : EMPTY_STR);

		ret += websWrite(wp, WEBS_TAB);

		/* Print encode */
		ret += websWrite(wp, "<td><select name=\"wl_realmlist_%d_1\"> "
		"<option value=\"0\" %s>RFC4282</option> "
		"<option value=\"1\" %s>UTF8</option> "
		"</select></td>", iter,
		(valid ? (nat->realm[iter].encoding ==
		REALM_ENCODING_RFC4282 ? SELECTED_STR : EMPTY_STR) : EMPTY_STR),
		(valid ? (nat->realm[iter].encoding ==
		REALM_ENCODING_UTF8 ? SELECTED_STR : EMPTY_STR) : EMPTY_STR));

		ret += websWrite(wp, WEBS_TAB);

		/* Print realminfo */
		ret += websWrite(wp, "<td><input id=\"wl_realmlist_%d_2\" "
		"name=\"wl_realmlist_%d_2\" value=\"%s\" size=\"64\" maxlength=\"1024\" "
		"readonly ></td>",
		iter, iter, valid ? (char*)nat->realm[iter].realmInfo : EMPTY_STR);

		ret += websWrite(wp, WEBS_TAB);

		/* Print modify button */
		ret += websWrite(wp, "<td><input name=\"wl_realmlist_%d_3\" "
		"type=button value=Modify onClick=\"onmodify_eapauth(%d);\"></td>",
		iter, iter);

		/* Print delete button */
		ret += websWrite(wp, "<td><input name=\"wl_realmlist_%d_4\" "
		"type=button value=Delete onClick=\"ondelete_eapauth(%d);\"></td>",
		iter, iter);

		/* Hidden Control */
		ret += websWrite(wp, "<td><input id=\"wl_realmlist_%d_5\" "
		"name=\"wl_realmlist_%d_5\" value=\"%s\" style=\"display:none;\"></td>",
		iter, iter, valid ? (char*)nat->realm[iter].eapInfo : EMPTY_STR);

		ret += websWrite(wp, "</tr>");
	}

	/* Clean up */
	if (nat) {
		free(nat);
		nat = NULL;
	}

	return ret;
}
REG_EJ_HANDLER(print_wl_realmlist);
/* ------------------------ 2. Realm List - VALIDATE ----------------------- */
void
validate_wl_realmlist(webs_t wp, char *value, struct variable *v, char *varname)
{
	struct variable fields[] = {
		{ name: "wl_realmlist_%d_%d",
		longname: IP_REALM_NAME,
		argv: ARGV("0", "255") },

		{ name: "wl_realmlist_%d_%d",
		longname: IP_ENCODE_TYPE,
		argv: ARGV("0", "1") },

		{ name: "wl_realmlist_%d_%d",
		longname: IP_REALM_INFO,
		argv: ARGV("0", "1024") }
	};

	int count, iter;
	char nvram_val[NVVAL_BUFF], item_value[NVVAL_BUFF];
	char row_name[NVNAME_BUFF], col_name[NVNAME_BUFF];
	char* val = NULL;
	char realmname[NAME_BUFF], encode[CODE_BUFF], realminfo[NVVAL_BUFF];

	assert(v);

	ret_code = EINVAL;

	if (!value) {
		ret_code = 0;
		return;
	}
	count = atoi(value);

	memset(nvram_val, 0, sizeof(nvram_val));

	for (iter = 0; iter < count; iter++) {

		memset(row_name, 0, sizeof(row_name));
		snprintf(row_name, sizeof(row_name), "%s_%d_", NVNM_WL_REALMLIST, iter);

		memset(col_name, 0, sizeof(col_name));
		memset(realmname, 0, sizeof(realmname));
		if ((val = websGetVar(wp, strcat_r(row_name, "0", col_name), NULL)) != NULL)
				strncpy_n(realmname, val, NAME_BUFF);

		memset(col_name, 0, sizeof(col_name));
		memset(encode, 0, sizeof(encode));
		if ((val = websGetVar(wp, strcat_r(row_name, "1", col_name), NULL)) != NULL)
			if (!strcmp(val, "0") || !strcmp(val, "1"))
				strncpy_n(encode, val, CODE_BUFF);

		memset(col_name, 0, sizeof(col_name));
		memset(realminfo, 0, sizeof(realminfo));
		if ((val = websGetVar(wp, strcat_r(row_name, "5", col_name), NULL)) != NULL)
				strncpy_n(realminfo, val, NVVAL_BUFF);

		/* Delete entry if all fields are blank */
		if (!strlen(realmname) && !strlen(realminfo)) {
			continue;
		}

		/* Check individual fields */
		if (!valid_name(wp, realmname, &fields[0])) {
			websBufferWrite(wp, HSERR_INVALID_INPUT,
				fields[0].longname, fields[0].longname);
			continue;
		}
		if (!valid_choice(wp, encode, &fields[1])) {
			websBufferWrite(wp, HSERR_INVALID_INPUT,
				fields[1].longname, fields[1].longname);
			continue;
		}
		if (!valid_name(wp, realminfo, &fields[2])) {
			websBufferWrite(wp, HSERR_INVALID_INPUT,
				fields[2].longname, fields[2].longname);
			continue;
		}

		/* Do it */
		memset(item_value, 0, sizeof(item_value));
		snprintf(item_value, sizeof(item_value), "%s+%s+%s", realmname, encode, realminfo);

		if (strlen(nvram_val))
			strncat(nvram_val, "?", min(1, NVVAL_BUFF-strlen(nvram_val)));

		strncat(nvram_val, item_value, min(strlen(item_value),
			NVVAL_BUFF-strlen(nvram_val)));
	}

	nvram_set(NVNM_WL_REALMLIST, nvram_val);

	ret_code = 0;

	return;
}
/* -------------------- 3. Venue Group & Type - ONCHANGE ------------------- */
int
ej_vanuegrp_change(int eid, webs_t wp, int argc, char_t** argv)
{
	int vanueGroupCode, ret = 0, index;

	/* Parse Args */
	if (ejArgs(argc, argv, "%d", &vanueGroupCode) < 1) {
		websError(wp, HTTP_ERR_BAD_REQUEST, HSERR_ARGV_PRNT);
		ret = -1;
	}

	websWrite(wp, "\t\t var groupTypes = new Array(");
	for (index = vanue_groups[vanueGroupCode].vanue_type_offset;
	index < vanue_groups[vanueGroupCode].vanue_type_offset +
		vanue_groups[vanueGroupCode].vanue_type_count;
	index++) {
		websWrite(wp, "\"%s\"", vanue_types[index].vanue_type_description);
		if (index < (vanue_groups[vanueGroupCode].vanue_type_offset +
			vanue_groups[vanueGroupCode].vanue_type_count - 1))
			websWrite(wp, ", ");
	}
	websWrite(wp, ");\n");

	websWrite(wp, "\t\t var groupTypesCode = new Array(");
	for (index = vanue_groups[vanueGroupCode].vanue_type_offset;
	index < vanue_groups[vanueGroupCode].vanue_type_offset +
		vanue_groups[vanueGroupCode].vanue_type_count;
	index++) {
		websWrite(wp, "\"%d\"", vanue_types[index].vanue_type_code);
		if (index < (vanue_groups[vanueGroupCode].vanue_type_offset +
			vanue_groups[vanueGroupCode].vanue_type_count - 1))
			websWrite(wp, ", ");
	}
	websWrite(wp, ");\n");

	return ret;
}
REG_EJ_HANDLER(vanuegrp_change);
/* ---------------------- 3. Venue Group & Type - PRINT -------------------- */
int
ej_print_wl_venuegrp_type(int eid, webs_t wp, int argc, char_t** argv)
{
	int index, vgcode, ret = 0;
	char vgstr[CODE_BUFF];
	char vtstr[CODE_BUFF];
	char* vgcodestr = nvram_get("wl_venuegrp");
	vgcode = atoi(vgcodestr);

	/* Printing Header */
	ret += websWrite(wp, "<tr>");
	ret += websWrite(wp, "<th width=\"310\"");
	ret += websWrite(wp, "onMouseOver=\"return overlib('Set Venue Group "
		"to the BSS supported by the interface.', LEFT);\"");
	ret += websWrite(wp, "onMouseOut=\"return nd();\">");
	ret += websWrite(wp, "Venue Group:&nbsp;&nbsp;");
	ret += websWrite(wp, "</th>");
	ret += websWrite(wp, "<td>&nbsp;&nbsp;</td>");

	/* Print VenueGroup */
	ret += websWrite(wp, "<td>");
	ret += websWrite(wp, "<select name=\"wl_venuegrp\" "
		"onChange = \"onchange_vanuegrp();\">\"");

	for (index = 0; index < VENUE_GROUP_COUNT; index++) {
		memset(vgstr, 0, sizeof(vgstr));
		sprintf(vgstr, "%d", index);
		ret += websWrite(wp, "<option value=\"%d\" %s>%s</option>",
			vanue_groups[index].vanue_group_code,
			(nvram_match("wl_venuegrp", vgstr))? SELECTED_STR : EMPTY_STR,
			vanue_groups[index].vanue_group_description);
	}
	ret += websWrite(wp, "</select></td></tr>");

	/* Printing Header */
	ret += websWrite(wp, "<tr>");
	ret += websWrite(wp, "<th width=\"310\"");
	ret += websWrite(wp, "onMouseOver=\"return overlib('Set Venue Type "
		"to the BSS supported by the interface.', LEFT);\"");
	ret += websWrite(wp, "onMouseOut=\"return nd();\">");
	ret += websWrite(wp, "Venue Type:&nbsp;&nbsp;");
	ret += websWrite(wp, "</th>");
	ret += websWrite(wp, "<td>&nbsp;&nbsp;</td>");

	/* Print VenueType */
	ret += websWrite(wp, "<td>");
	ret += websWrite(wp, "<select name=\"wl_venuetype\">");

	for (index = vanue_groups[vgcode].vanue_type_offset;
	index < vanue_groups[vgcode].vanue_type_offset +
		vanue_groups[vgcode].vanue_type_count;
	index++) {
		memset(vtstr, 0, sizeof(vtstr));
		sprintf(vtstr, "%d", vanue_types[index].vanue_type_code);
		ret += websWrite(wp, "<option value=\"%d\"%s>%s</option>",
			vanue_types[index].vanue_type_code,
			(nvram_match("wl_venuetype", vtstr) ? SELECTED_STR : EMPTY_STR),
			vanue_types[index].vanue_type_description);
	}

	ret += websWrite(wp, "</select></td></tr>");
	return ret;
}
REG_EJ_HANDLER(print_wl_venuegrp_type);
/* ---------------------- 4. Venue Name List - PRINT ----------------------- */
int
ej_print_wl_venuelist(int eid, webs_t wp, int argc, char_t **argv)
{
	bcm_decode_anqp_venue_name_t* nat = NULL;

	int iter, last, ret = 0;
	bool valid = FALSE, noNvram = FALSE;
	char prefix[] = PREFIX_DEF, err_nvram[NVNAME_BUFF] = {0};

	/* Parse Args */
	if (ejArgs(argc, argv, "%d %d", &iter, &last) < 2) {
		websError(wp, HTTP_ERR_BAD_REQUEST, HSERR_ARGV_PRNT);
		return -1;
	}

	/* Make Prefix */
	if (!make_wl_prefix(prefix, sizeof(prefix), 1, NULL))
		return -1;

	/* Parse NVRAM */
	nat = (bcm_decode_anqp_venue_name_t*)
		malloc(sizeof(bcm_decode_anqp_venue_name_t));
	if (!nat)
		return -1;
	memset(nat, 0, sizeof(*nat));
	if (!decode_u11_venue_list(prefix, nat, err_nvram))
		noNvram = TRUE;

	/* Start Printing */
	for (; iter <= last; iter++) {

		/* Evaluate if entry is valid */
		valid = ((iter < nat->numVenueName) && !noNvram)? TRUE : FALSE;

		/* Printing Header */
		ret += websWrite(wp, "<tr>");
		ret += websWrite(wp, "<th width=\"310\" valign=\"top\" rowspan=\"1\"");
		ret += websWrite(wp, "onMouseOver=\"return overlib('Set Venue Name "
			"List to the BSS supported by the interface.', LEFT);\"");
		ret += websWrite(wp, "onMouseOut=\"return nd();\">");
		ret += websWrite(wp, "<input type=\"hidden\" name=\"wl_venuelist_%d_\" "
			"value=\"2\">", iter);
		ret += websWrite(wp, " &nbsp;&nbsp;");
		ret += websWrite(wp, "</th>");
		ret += websWrite(wp, "<td>&nbsp;&nbsp;</td>");

		/* Print oper */
		ret += websWrite(wp, "<td><input name=\"wl_venuelist_%d_0\" "
		"value=\"%s\" size=\"64\" maxlength=\"255\"></td>", iter,
		valid ? nat->venueName[iter].name : EMPTY_STR);

		ret += websWrite(wp, WEBS_TAB);

		/* Print lang */
		ret += websWrite(wp, "<td><input name=\"wl_venuelist_%d_1\" "
		"value=\"%s\" size=\"5\" maxlength=\"5\"></td>", iter,
		valid ? nat->venueName[iter].lang : EMPTY_STR);

		ret += websWrite(wp, "</tr>");
	}

	/* Clean up */
	if (nat) {
		free(nat);
		nat = NULL;
	}

	return ret;
}
REG_EJ_HANDLER(print_wl_venuelist);
/* --------------------- 4. Venue Name List - VALIDATE --------------------- */
void
validate_wl_venuelist(webs_t wp, char *value, struct variable *v, char *varname)
{
	struct variable fields[] = {
		{ name: "wl_venuelist_%d_%d",
		longname: IP_VENUE_NAME,
		argv: ARGV("0", "255") },

		{ name: "wl_venuelist_%d_%d",
		longname: IP_LANG_CODE,
		argv: ARGV("0", "3") },
	};

	int count, iter;
	char nvram_val[NVVAL_BUFF], ptrUTF8[NVVAL_BUFF], item_value[ITEM_BUFF];
	char row_name[NVNAME_BUFF], col_name[NVNAME_BUFF];
	char* val = NULL;

	char venue[NAME_BUFF], lang[CODE_BUFF];

	assert(v);

	ret_code = EINVAL;

	if (!value) {
		ret_code = 0;
		return;
	}
	count = atoi(value);

	memset(ptrUTF8, 0, sizeof(ptrUTF8));
	memset(nvram_val, 0, sizeof(nvram_val));

	for (iter = 0; iter < count; iter++) {

		memset(row_name, 0, sizeof(row_name));
		snprintf(row_name, sizeof(row_name), "%s_%d_", NVNM_WL_VENUELIST, iter);

		memset(col_name, 0, sizeof(col_name));
		memset(venue, 0, sizeof(venue));
		if ((val = websGetVar(wp, strcat_r(row_name, "0", col_name), NULL)) != NULL)
			strncpy_n(venue, val, NAME_BUFF);

		memset(col_name, 0, sizeof(col_name));
		memset(lang, 0, sizeof(lang));
		if ((val = websGetVar(wp, strcat_r(row_name, "1", col_name), NULL)) != NULL)
			strncpy_n(lang, val, CODE_BUFF);

		/* Delete entry if all fields are blank */
		if (!strlen(venue) && !strlen(lang)) {
			continue;
		}

		/* Fill in empty fields with default values or check for mendetory fields */
		if (!strlen(venue) && (strlen(lang) > 0)) {
			websBufferWrite(wp, HSERR_EMPTY_INPUT,
				fields[0].longname, fields[0].longname);
			continue;
		}

		if ((strlen(venue) > 0) && !strlen(lang)) {
			websBufferWrite(wp, HSERR_EMPTY_INPUT,
				fields[1].longname, fields[1].longname);
			continue;
		}

		/* Check individual fields */
		if (!valid_name(wp, venue, &fields[0])) {
			websBufferWrite(wp, HSERR_INVALID_INPUT,
				fields[0].longname, fields[0].longname);
			continue;
		}
		if (!valid_name(wp, lang, &fields[1])) {
			websBufferWrite(wp, HSERR_INVALID_INPUT,
				fields[1].longname, fields[1].longname);
			continue;
		}

		/* Do it */
		memset(item_value, 0, sizeof(item_value));
		snprintf(item_value, sizeof(item_value), "%s!%s", venue, lang);

		if (strlen(nvram_val))
			strncat(nvram_val, "|", min(1, NVVAL_BUFF-strlen(nvram_val)));

		strncat(nvram_val, item_value, min(strlen(item_value),
			NVVAL_BUFF-strlen(nvram_val)));
	}

	bytes_to_hex((uchar*)nvram_val, NVVAL_BUFF,
		(uchar*)ptrUTF8, NVVAL_BUFF);

	nvram_set(NVNM_WL_VENUELIST, ptrUTF8);

	ret_code = 0;

	return;
}
/* ------------------ 5. Roaming Consortium List - PRINT ------------------- */
int
ej_print_wl_ouilist(int eid, webs_t wp, int argc, char_t **argv)
{
	bcm_decode_anqp_roaming_consortium_ex_t *nat = NULL;

	int iter, iter_oi = 0, last, ret = 0, beacon_count = 0;
	bool valid = FALSE, noNvram = FALSE;
	char prefix[] = PREFIX_DEF, hexstr[NVNAME_BUFF] = {0};

	/* Parse Args */
	if (ejArgs(argc, argv, "%d %d", &iter, &last) < 2) {
		websError(wp, HTTP_ERR_BAD_REQUEST, HSERR_ARGV_PRNT);
		return -1;
	}

	/* Make Prefix */
	if (!make_wl_prefix(prefix, sizeof(prefix), 1, NULL))
		return -1;

	/* Parse NVRAM */
	nat = (bcm_decode_anqp_roaming_consortium_ex_t*)
		malloc(sizeof(bcm_decode_anqp_roaming_consortium_ex_t));
	if (!nat)
		return -1;
	memset(nat, 0, sizeof(*nat));
	if (!decode_u11_oui_list(prefix, nat))
		noNvram = TRUE;

	/* Start Printing */
	for (; iter <= last; iter++) {
		memset(hexstr, 0, sizeof(hexstr));

		/* Evaluate if entry is valid */
		valid = ((iter < nat->numOi) && !noNvram)? TRUE : FALSE;

		for (iter_oi = 0; iter_oi < nat->oi[iter].oiLen; iter_oi++) {
			append_numto_hexStr(hexstr, NVNAME_BUFF,
				nat->oi[iter].oi[iter_oi]);
		}

		/* Printing Header */
		ret += websWrite(wp, "<tr>");
		ret += websWrite(wp, "<th width=\"310\" valign=\"top\" rowspan=\"1\"");
		ret += websWrite(wp, "onMouseOver=\"return overlib('Set Roaming Consortium "
			"List to the BSS supported by the interface.', LEFT);\"");
		ret += websWrite(wp, "onMouseOut=\"return nd();\">");
		ret += websWrite(wp, "<input type=\"hidden\" name=\"wl_ouilist_%d_\" "
			"value=\"1\">", iter);
		ret += websWrite(wp, " &nbsp;&nbsp;");
		ret += websWrite(wp, "</th>");
		ret += websWrite(wp, "<td>&nbsp;&nbsp;</td>");

		/* Print oi */
		ret += websWrite(wp, "<td><input name=\"wl_ouilist_%d_0\" "
		"value=\"%s\" size=\"16\" maxlength=\"16\"></td>", iter,
		valid ? hexstr : EMPTY_STR);

		/* Print is_beacon */
		ret += websWrite(wp, "<td><input name=\"wl_ouilist_%d_1\""
		"type=\"checkbox\" %s onClick = \"return validate_max_3_oui()\" ></td>", iter,
		valid && nat->oi[iter].isBeacon ? "Checked" : EMPTY_STR);
		ret += websWrite(wp, "</tr>");

		if (valid && nat->oi[iter].isBeacon)
			beacon_count++;

		/* Print Hidden Control */
		if (iter == last) {
			ret += websWrite(wp, "<tr><td>");
			ret += websWrite(wp, "<input id=\"wl_ouilist_bc\" name=\"wl_ouilist_bc\""
					"style=\"display:none\" value=\"%d\">", beacon_count);
			ret += websWrite(wp, "</td></tr>");
		}
	}

	/* Clean up */
	if (nat) {
		free(nat);
		nat = NULL;
	}

	return ret;
}
REG_EJ_HANDLER(print_wl_ouilist);
/* ----------------- 5. Roaming Consortium List - VALIDATE ----------------- */
void
validate_wl_ouilist(webs_t wp, char *value, struct variable *v, char *varname)
{
	struct variable fields[] = {
		{ name: "wl_ouilist_%d_%d",
		longname: IP_OUI_NAME,
		argv: ARGV("2", "14") },

		{ name: "wl_ouilist_%d_%d",
		longname: IP_OUI_ISBEACON,
		argv: ARGV("0", "1") }
	};

	int count, iter, is_becon;
	char nvram_val[NVVAL_BUFF], item_value[ITEM_BUFF];
	char row_name[NVNAME_BUFF], col_name[NVNAME_BUFF];
	char* val = NULL;

	char oi[NAME_BUFF];

	assert(v);

	ret_code = EINVAL;

	if (!value) {
		ret_code = 0;
		return;
	}
	count = atoi(value);

	memset(nvram_val, 0, sizeof(nvram_val));

	/* Check for beacon_count entry. If beacon_count > 3 return */
	memset(row_name, 0, sizeof(row_name));
	snprintf(row_name, sizeof(row_name), "%s_bc", NVNM_WL_OUILIST);

	if ((val = websGetVar(wp, row_name, NULL)) != NULL) {
		if (atoi(val) > 3) {
			websBufferWrite(wp, HSERR_OUI_MAX_3_BEACON,
				fields[1].longname);
			return;
		}
	}

	for (iter = 0; iter < count; iter++) {

		memset(row_name, 0, sizeof(row_name));
		snprintf(row_name, sizeof(row_name), "%s_%d_", NVNM_WL_OUILIST, iter);

		memset(col_name, 0, sizeof(col_name));
		memset(oi, 0, sizeof(oi));
		if ((val = websGetVar(wp, strcat_r(row_name, "0", col_name), NULL)) != NULL)
			strncpy_n(oi, val, NAME_BUFF);

		is_becon = 0;
		memset(col_name, 0, sizeof(col_name));
		if ((val = websGetVar(wp, strcat_r(row_name, "1", col_name), NULL)) != NULL)
			is_becon = 1;

		/* Delete entry if all fields are blank */
		if (!strlen(oi)) {
			continue;
		}

		/* Check individual fields */
		if (!valid_hex(wp, oi, &fields[0])) {
			websBufferWrite(wp, HSERR_INVALID_INPUT,
				fields[0].longname, fields[0].longname);
			continue;
		}

		/* Do it */
		memset(item_value, 0, sizeof(item_value));
		snprintf(item_value, sizeof(item_value), "%s:%d", oi, is_becon);

		if (strlen(nvram_val))
			strncat(nvram_val, ";", min(1, NVVAL_BUFF-strlen(nvram_val)));

		strncat(nvram_val, item_value, min(strlen(item_value),
			NVVAL_BUFF-strlen(nvram_val)));
	}

	nvram_set(NVNM_WL_OUILIST, nvram_val);

	ret_code = 0;

	return;
}
/* ------------- 6. 3GPP Cellular Network Info List - PRINT ---------------- */
int
ej_print_wl_3gpplist(int eid, webs_t wp, int argc, char_t **argv)
{
	bcm_decode_anqp_3gpp_cellular_network_t* nat = NULL;

	int iter, last, ret = 0;
	bool valid = FALSE, noNvram = FALSE;
	char prefix[] = PREFIX_DEF;

	/* Parse Args */
	if (ejArgs(argc, argv, "%d %d", &iter, &last) < 2) {
		websError(wp, HTTP_ERR_BAD_REQUEST, HSERR_ARGV_PRNT);
		return -1;
	}

	/* Make Prefix */
	if (!make_wl_prefix(prefix, sizeof(prefix), 1, NULL))
		return -1;

	/* Parse NVRAM */
	nat = (bcm_decode_anqp_3gpp_cellular_network_t*)
		malloc(sizeof(bcm_decode_anqp_3gpp_cellular_network_t));
	if (!nat)
		return -1;
	memset(nat, 0, sizeof(*nat));
	if (!decode_u11_3gpp_list(prefix, nat))
		noNvram = TRUE;

	/* Start Printing */
	for (; iter <= last; iter++) {

		/* Evaluate if entry is valid */
		valid = ((iter < nat->plmnCount) && !noNvram)? TRUE : FALSE;

		/* Printing Header */
		ret += websWrite(wp, "<tr>");
		ret += websWrite(wp, "<th width=\"310\" valign=\"top\" rowspan=\"1\"");
		ret += websWrite(wp, "onMouseOver=\"return overlib('Set 3GPP Cellular Network "
			"Information List to the BSS supported by the interface.', LEFT);\"");
		ret += websWrite(wp, "onMouseOut=\"return nd();\">");
		ret += websWrite(wp, "<input type=\"hidden\" name=\"wl_3gpplist_%d_\" "
			"value=\"2\">", iter);
		ret += websWrite(wp, " &nbsp;&nbsp;");
		ret += websWrite(wp, "</th>");
		ret += websWrite(wp, "<td>&nbsp;&nbsp;</td>");

		/* Print mcc */
		ret += websWrite(wp, "<td><input name=\"wl_3gpplist_%d_0\" "
		"value=\"%s\" size=\"5\" maxlength=\"5\"></td>", iter,
		valid ? nat->plmn[iter].mcc : EMPTY_STR);

		ret += websWrite(wp, WEBS_TAB);

		/* Print mnc */
		ret += websWrite(wp, "<td><input name=\"wl_3gpplist_%d_1\" "
		"value=\"%s\" size=\"5\" maxlength=\"5\"></td>", iter,
		valid ? nat->plmn[iter].mnc : EMPTY_STR);

		ret += websWrite(wp, "</tr>");
	}

	/* Clean up */
	if (nat) {
		free(nat);
		nat = NULL;
	}

	return ret;
}
REG_EJ_HANDLER(print_wl_3gpplist);
/* ------------ 6. 3GPP Cellular Network Info List - VALIDATE -------------- */
void
validate_wl_3gpplist(webs_t wp, char *value, struct variable *v, char *varname)
{
	struct variable fields[] = {
		{ name: "wl_3gpplist_%d_%d",
		longname: IP_MCC,
		argv: ARGV("2", "3") },

		{ name: "wl_3gpplist_%d_%d",
		longname: IP_MNC,
		argv: ARGV("2", "3") },
	};

	int count, iter;
	char nvram_val[NVVAL_BUFF], item_value[ITEM_BUFF];
	char row_name[NVNAME_BUFF], col_name[NVNAME_BUFF];
	char* val = NULL;

	char mcc[CODE_BUFF], mnc[CODE_BUFF];

	assert(v);

	ret_code = EINVAL;

	if (!value) {
		ret_code = 0;
		return;
	}
	count = atoi(value);

	memset(nvram_val, 0, sizeof(nvram_val));

	for (iter = 0; iter < count; iter++) {

		memset(row_name, 0, sizeof(row_name));
		snprintf(row_name, sizeof(row_name), "%s_%d_", NVNM_WL_3GPPLIST, iter);

		memset(col_name, 0, sizeof(col_name));
		memset(mcc, 0, sizeof(mcc));
		if ((val = websGetVar(wp, strcat_r(row_name, "0", col_name), NULL)) != NULL)
			strncpy_n(mcc, val, CODE_BUFF);

		memset(col_name, 0, sizeof(col_name));
		memset(mnc, 0, sizeof(mnc));
		if ((val = websGetVar(wp, strcat_r(row_name, "1", col_name), NULL)) != NULL)
			strncpy_n(mnc, val, CODE_BUFF);

		/* Delete entry if all fields are blank */
		if (!strlen(mcc) && !strlen(mnc)) {
			continue;
		}

		/* Fill in empty fields with default values or check for mendetory fields */
		if (!strlen(mcc) && (strlen(mnc) > 0)) {
			websBufferWrite(wp, HSERR_EMPTY_INPUT,
				fields[0].longname, fields[0].longname);
			continue;
		}

		if ((strlen(mcc) > 0) && !strlen(mnc)) {
			websBufferWrite(wp, HSERR_EMPTY_INPUT,
				fields[1].longname, fields[1].longname);
			continue;
		}

		/* Check individual fields */
		if (!valid_name(wp, mcc, &fields[0])) {
			websBufferWrite(wp, HSERR_INVALID_INPUT,
				fields[0].longname, fields[0].longname);
			continue;
		}

		if (!valid_name(wp, mnc, &fields[1])) {
			websBufferWrite(wp, HSERR_INVALID_INPUT,
				fields[1].longname, fields[1].longname);
			continue;
		}

		/* Do it */
		memset(item_value, 0, sizeof(item_value));
		snprintf(item_value, sizeof(item_value), "%s:%s", mcc, mnc);

		if (strlen(nvram_val))
			strncat(nvram_val, ";", min(1, NVVAL_BUFF-strlen(nvram_val)));

		strncat(nvram_val, item_value, min(strlen(item_value),
			NVVAL_BUFF-strlen(nvram_val)));
	}

	nvram_set(NVNM_WL_3GPPLIST, nvram_val);

	ret_code = 0;

	return;
}
/* ------------------------ 7. QoS Map Set IE - PRINT ---------------------- */
int
ej_print_wl_qosmapie(int eid, webs_t wp, int argc, char_t **argv)
{
	bcm_decode_qos_map_t *nat = NULL;

	int iter, last, ret = 0;
	bool valid = FALSE, noNvram = FALSE;
	char prefix[] = PREFIX_DEF;

	/* Parse Args */
	if (ejArgs(argc, argv, "%d %d", &iter, &last) < 2) {
		websError(wp, HTTP_ERR_BAD_REQUEST, HSERR_ARGV_PRNT);
		return -1;
	}

	/* Make Prefix */
	if (!make_wl_prefix(prefix, sizeof(prefix), 1, NULL))
		return -1;

	/* Parse NVRAM */
	nat = (bcm_decode_qos_map_t*)
		malloc(sizeof(bcm_decode_qos_map_t));
	if (!nat)
		return -1;
	memset(nat, 0, sizeof(*nat));
	if (!decode_qosmap_ie(prefix, nat))
		noNvram = TRUE;

	/* Start Printing */

	/* Printing DSCP Exception */
	ret += websWrite(wp, "<tr>");
	ret += websWrite(wp, "<th width=\"310\"");
	ret += websWrite(wp, "onMouseOver=\"return overlib('Set QoS Map Set IE's "
		"DSCP Exception.', LEFT);\"");
	ret += websWrite(wp, "onMouseOut=\"return nd();\">");
	ret += websWrite(wp, "DSCP Exception : &nbsp;&nbsp;");
	ret += websWrite(wp, "</th>");
	ret += websWrite(wp, "<td>&nbsp;&nbsp;</td>");
	for (iter = 0; iter <= last; iter++) {
		ret += websWrite(wp, "<td class=\"label\"># %d</td><td></td>", iter+1);
	}

	/* Printing DSCP */
	ret += websWrite(wp, "<tr>");
	ret += websWrite(wp, "<th width=\"310\"");
	ret += websWrite(wp, "onMouseOver=\"return overlib('Set QoS Map Set IE's "
		"DSCP Exception.', LEFT);\"");
	ret += websWrite(wp, "onMouseOut=\"return nd();\">");
	ret += websWrite(wp, "DSCP : &nbsp;&nbsp;");
	ret += websWrite(wp, "</th>");
	ret += websWrite(wp, "<td>&nbsp;&nbsp;</td>");
	for (iter = 0; iter <= last; iter++) {
		valid = (!noNvram && (nat->exceptCount - iter > 0));
		/* Print DSCP #1 to #8 */
		if (valid)
			ret += websWrite(wp, "<td><input id=\"wl_qos_dscp_%d\" "
			"name=\"wl_qos_dscp_%d\" value=\"%d\" size=\"3\" maxlength=\"3\" ></td>",
			iter, iter, nat->except[iter].dscp);
		else
			ret += websWrite(wp, "<td><input id=\"wl_qos_dscp_%d\" "
			"name=\"wl_qos_dscp_%d\" value=\"%s\" size=\"3\" maxlength=\"3\" ></td>",
			iter, iter, EMPTY_STR);
		if (iter < last)
			ret += websWrite(wp, WEBS_TAB);
		else
			ret += websWrite(wp, "</tr>");
	}

	/* Printing UP */
	ret += websWrite(wp, "<tr>");
	ret += websWrite(wp, "<th width=\"310\"");
	ret += websWrite(wp, "onMouseOver=\"return overlib('Set QoS Map Set IE's "
		"DSCP Exception.', LEFT);\"");
	ret += websWrite(wp, "onMouseOut=\"return nd();\">");
	ret += websWrite(wp, "UP : &nbsp;&nbsp;");
	ret += websWrite(wp, "</th>");
	ret += websWrite(wp, "<td>&nbsp;&nbsp;</td>");
	for (iter = 0; iter <= last; iter++) {
		valid = (!noNvram && (nat->exceptCount - iter > 0));
		/* Print DSCP #1 to #8 */
		if (valid)
			ret += websWrite(wp, "<td><input id=\"wl_qos_up_%d\" "
			"name=\"wl_qos_up_%d\" value=\"%d\" size=\"3\" maxlength=\"3\" ></td>",
			iter, iter, nat->except[iter].up);
		else
			ret += websWrite(wp, "<td><input id=\"wl_qos_up_%d\" "
			"name=\"wl_qos_up_%d\" value=\"%s\" size=\"3\" maxlength=\"3\" ></td>",
			iter, iter, EMPTY_STR);
		if (iter < last)
			ret += websWrite(wp, WEBS_TAB);
		else
			ret += websWrite(wp, "</tr>");
	}

	/* Separator */
	ret += websWrite(wp, "<tr>");
	ret += websWrite(wp, "<th width=\"310\"");
	ret += websWrite(wp, "onMouseOver=\"return overlib('Set QoS Map Set IE's "
		"DSCP Exception and DSCP Range to the BSS supported by the interface.', LEFT);\"");
	ret += websWrite(wp, "onMouseOut=\"return nd();\">");
	ret += websWrite(wp, "&nbsp;&nbsp;");
	ret += websWrite(wp, "</th>");
	ret += websWrite(wp, "<td>&nbsp;&nbsp;</td>");
	ret += websWrite(wp, "</tr>");

	/* Printing User Priority Range */
	ret += websWrite(wp, "<tr>");
	ret += websWrite(wp, "<th width=\"310\"");
	ret += websWrite(wp, "onMouseOver=\"return overlib('Set QoS Map Set IE's "
		"DSCP Range.', LEFT);\"");
	ret += websWrite(wp, "onMouseOut=\"return nd();\">");
	ret += websWrite(wp, "DSCP Range : &nbsp;&nbsp;");
	ret += websWrite(wp, "</th>");
	ret += websWrite(wp, "<td>&nbsp;&nbsp;</td>");
	for (iter = 0; iter <= last; iter++) {
		ret += websWrite(wp, "<td class=\"label\"># %d</td><td></td>", iter+1);
	}

	/* Printing Low Value */
	ret += websWrite(wp, "<tr>");
	ret += websWrite(wp, "<th width=\"310\"");
	ret += websWrite(wp, "onMouseOver=\"return overlib('Set QoS Map Set IE's "
		"DSCP Range.', LEFT);\"");
	ret += websWrite(wp, "onMouseOut=\"return nd();\">");
	ret += websWrite(wp, "Low Value : &nbsp;&nbsp;");
	ret += websWrite(wp, "</th>");
	ret += websWrite(wp, "<td>&nbsp;&nbsp;</td>");
	for (iter = 0; iter <= last; iter++) {
		valid = (!noNvram);
		/* Print Low Value #1 to #8 */
		if (valid)
			ret += websWrite(wp, "<td><input id=\"wl_qos_low_%d\" "
			"name=\"wl_qos_low_%d\" value=\"%d\" size=\"3\" maxlength=\"3\" ></td>",
			iter, iter, nat->up[iter].low);
		else
			ret += websWrite(wp, "<td><input id=\"wl_qos_low_%d\" "
			"name=\"wl_qos_low_%d\" value=\"%s\" size=\"3\" maxlength=\"3\" ></td>",
			iter, iter, EMPTY_STR);
		if (iter < last)
			ret += websWrite(wp, WEBS_TAB);
		else
			ret += websWrite(wp, "</tr>");
	}

	/* Printing High Value */
	ret += websWrite(wp, "<tr>");
	ret += websWrite(wp, "<th width=\"310\"");
	ret += websWrite(wp, "onMouseOver=\"return overlib('Set QoS Map Set IE's "
		"DSCP Range.', LEFT);\"");
	ret += websWrite(wp, "onMouseOut=\"return nd();\">");
	ret += websWrite(wp, "High Value : &nbsp;&nbsp;");
	ret += websWrite(wp, "</th>");
	ret += websWrite(wp, "<td>&nbsp;&nbsp;</td>");
	for (iter = 0; iter <= last; iter++) {
		valid = (!noNvram);
		/* Print High Value #1 to #8 */
		if (valid)
			ret += websWrite(wp, "<td><input id=\"wl_qos_high_%d\" "
			"name=\"wl_qos_high_%d\" value=\"%d\" size=\"3\" maxlength=\"3\" ></td>",
			iter, iter, nat->up[iter].high);
		else
			ret += websWrite(wp, "<td><input id=\"wl_qos_high_%d\" "
			"name=\"wl_qos_high_%d\" value=\"%s\" size=\"3\" maxlength=\"3\" ></td>",
			iter, iter, EMPTY_STR);
		if (iter < last)
			ret += websWrite(wp, WEBS_TAB);
		else
			ret += websWrite(wp, "</tr>");
	}

	/* Clean up */
	if (nat) {
		free(nat);
		nat = NULL;
	}

	return ret;
}
REG_EJ_HANDLER(print_wl_qosmapie);
/* ---------------------- 7. QoS Map Set IE - VALIDATE --------------------- */
void
validate_wl_qosmapie(webs_t wp, char *value, struct variable *v, char *varname)
{
	struct variable fields[] = {
		{ name: "wl_qos_dscp",
		longname: IP_DSCP_VAL,
		argv: ARGV("0", "63") },

		{ name: "wl_qos_up",
		longname: IP_UP_VAL,
		argv: ARGV("0", "7") },

		{ name: "wl_qos_low",
		longname: IP_LOW_VAL,
		argv: ARGV("0", "63") },

		{ name: "wl_qos_high",
		longname: IP_HIGH_VAL,
		argv: ARGV("0", "63") }
	};

	int count, iter, make_FF;
	char nvram_val[NVVAL_BUFF], item_value[CODE_BUFF];
	char row_name[NVNAME_BUFF];
	char* val = NULL;

	char dscp[CODE_BUFF], up[CODE_BUFF], low[CODE_BUFF], high[CODE_BUFF];
	uint8 dscp_val_arr[CODE_BUFF], dscp_rng_arr[CODE_BUFF];
	uint8 dscp_val = 0, up_val = 0, low_val = 0, high_val = 0;
	char dscp_except[ITEM_BUFF], dscp_range[ITEM_BUFF];

	assert(v);

	ret_code = EINVAL;

	if (!value) {
		ret_code = 0;
		return;
	}
	count = atoi(value);

	memset(nvram_val, 0, sizeof(nvram_val));
	memset(dscp_val_arr, 0, sizeof(dscp_val_arr));
	memset(dscp_except, 0, sizeof(dscp_except));

	/* 1. Validate DSCP Exception */
	for (iter = 0; iter < count; iter++) {

		memset(row_name, 0, sizeof(row_name));
		snprintf(row_name, sizeof(row_name), "%s_%d", fields[0].name, iter);
		memset(dscp, 0, CODE_BUFF);
		if ((val = websGetVar(wp, row_name, NULL)) != NULL)
			strncpy_n(dscp, val, CODE_BUFF);
		memset(row_name, 0, sizeof(row_name));
		snprintf(row_name, sizeof(row_name), "%s_%d", fields[1].name, iter);
		memset(up, 0, CODE_BUFF);
		if ((val = websGetVar(wp, row_name, NULL)) != NULL)
			strncpy_n(up, val, CODE_BUFF);

		/* Delete entry if all fields are blank */
		if (!strlen(dscp) && !strlen(up)) {
			continue;
		}

		/* Fill in empty fields with default values or check for mendetory fields */
		if (!strlen(dscp) && (strlen(up) > 0)) {
			websBufferWrite(wp, HSERR_EMPTY_INPUT,
				fields[0].longname, fields[0].longname);
			continue;
		}

		if ((strlen(dscp) > 0) && !strlen(up)) {
			websBufferWrite(wp, HSERR_EMPTY_INPUT,
				fields[1].longname, fields[1].longname);
			continue;
		}

		/* Check individual fields */
		if (!valid_octet(wp, dscp, &fields[0])) {
			websBufferWrite(wp, HSERR_INVALID_INPUT,
				fields[0].longname, fields[0].longname);
			continue;
		}
		if (!valid_range(wp, up, &fields[1])) {
			websBufferWrite(wp, HSERR_INVALID_INPUT,
				fields[1].longname, fields[1].longname);
			continue;
		}

		/* Special fields - DSCP values should be unique */
		dscp_val = (uint8)atoi(dscp); up_val = (uint8)atoi(up);
		if (is_duplicate_octet(dscp_val_arr, iter, dscp_val)) {
			websBufferWrite(wp, HSERR_INVALID_DSCP_DUP, fields[0].longname);
			continue;
		}
		dscp_val_arr[iter] = dscp_val;

		/* Append DSCP and UP to HexString */
		append_numto_hexStr(dscp_except, ITEM_BUFF, dscp_val);
		append_numto_hexStr(dscp_except, ITEM_BUFF, up_val);

	}

	/* Append DSCP Exception HexString to NVRAM String */
	if (strlen(dscp_except) > 0) {
		strncat(nvram_val, dscp_except, min(strlen(dscp_except),
			NVVAL_BUFF-strlen(dscp_except)));
		strncat(nvram_val, "+", min(1, NVVAL_BUFF-strlen(nvram_val)));
	}

	memset(dscp_rng_arr, 0, sizeof(dscp_rng_arr));
	memset(dscp_range, 0, sizeof(dscp_range));

	/* 2. Validate DSCP Range */
	for (iter = 0; iter < count; iter++) {

		make_FF = FALSE;
		memset(row_name, 0, sizeof(row_name));
		snprintf(row_name, sizeof(row_name), "%s_%d", fields[2].name, iter);
		memset(low, 0, CODE_BUFF);
		if ((val = websGetVar(wp, row_name, NULL)) != NULL)
			strncpy_n(low, val, CODE_BUFF);
		memset(row_name, 0, sizeof(row_name));
		snprintf(row_name, sizeof(row_name), "%s_%d", fields[3].name, iter);
		memset(high, 0, CODE_BUFF);
		if ((val = websGetVar(wp, row_name, NULL)) != NULL)
			strncpy_n(high, val, CODE_BUFF);

		/* Delete entry if all fields are blank */
		if (!strlen(low) && !strlen(high)) {
			websBufferWrite(wp, HSERR_INVALID_DSCP_RANGE,
				iter+1, DSCP_ERR_EMPTY_RANGE, iter+1);
			make_FF = TRUE; goto MAKE_DSCP_RANGE_FF;
		}

		/* Fill in empty fields with default values or check for mendetory fields */
		if (!strlen(low) && (strlen(high) > 0)) {
			websBufferWrite(wp, HSERR_INVALID_DSCP_RANGE,
				iter+1, DSCP_ERR_EMPTY_LOW_VAL, iter+1);
			make_FF = TRUE; goto MAKE_DSCP_RANGE_FF;
		}

		if ((strlen(low) > 0) && !strlen(high)) {
			websBufferWrite(wp, HSERR_INVALID_DSCP_RANGE,
				iter+1, DSCP_ERR_EMPTY_HIGH_VAL, iter+1);
			make_FF = TRUE; goto MAKE_DSCP_RANGE_FF;
		}

		/* Check individual fields */
		if (!valid_octet(wp, low, &fields[2])) {
			websBufferWrite(wp, HSERR_INVALID_DSCP_RANGE,
				iter+1, DSCP_ERR_SPECF_LOW_VAL, iter+1);
			make_FF = TRUE; goto MAKE_DSCP_RANGE_FF;
		}
		if (!valid_octet(wp, high, &fields[3])) {
			websBufferWrite(wp, HSERR_INVALID_DSCP_RANGE,
				iter+1, DSCP_ERR_SPECF_HIGH_VAL, iter+1);
			make_FF = TRUE; goto MAKE_DSCP_RANGE_FF;
		}

		/* Special fields - High Value >= Low Value */
		low_val = (uint8)atoi(low); high_val = (uint8)atoi(high);
		if (low_val > high_val) {
			websBufferWrite(wp, HSERR_INVALID_DSCP_RANGE,
				iter+1, DSCP_ERR_LOW_GR8ER_HIGH, iter+1);
			make_FF = TRUE; goto MAKE_DSCP_RANGE_FF;
		}

		/* Special fields - Ranges are Non-Overlapping */
		if (is_octet_range_overlapping(dscp_rng_arr, iter, low_val, high_val)) {
			websBufferWrite(wp, HSERR_INVALID_DSCP_RANGE,
				iter+1, DSCP_ERR_RANGE_OVRLAP, iter+1);
			make_FF = TRUE; goto MAKE_DSCP_RANGE_FF;
		}

MAKE_DSCP_RANGE_FF:
		if (make_FF) {
			low_val = high_val = 255;
			strncpy_n(low, "255", CODE_BUFF);
			strncpy_n(high, "255", CODE_BUFF);
			make_FF = FALSE;
		}

		dscp_rng_arr[(iter*2)] = low_val;
		dscp_rng_arr[(iter*2)+1] = high_val;

		/* Append DSCP Range to a String - Do it */
		memset(item_value, 0, sizeof(item_value));
		snprintf(item_value, sizeof(item_value), "%s,%s", low, high);

		if (strlen(dscp_range))
			strncat(dscp_range, ";", min(1, ITEM_BUFF-strlen(dscp_range)));

		strncat(dscp_range, item_value, min(strlen(item_value),
			ITEM_BUFF-strlen(dscp_range)));
	}

	/* Append DSCP Range String to NVRAM String */
	if (strlen(dscp_range) > 0) {
		strncat(nvram_val, dscp_range, min(strlen(dscp_range),
			NVVAL_BUFF-strlen(dscp_range)));
	}

	nvram_set(NVNM_WL_QOSMAPIE, nvram_val);

	ret_code = 0;

	return;
}
/* ------------------------ 8. Wan Metrics - PRINT ------------------------- */
int
ej_print_wl_wanmetrics(int eid, webs_t wp, int argc, char_t **argv)
{
	bcm_decode_hspot_anqp_wan_metrics_t* nat = NULL;

	int iter, last, ret = 0;
	bool valid = FALSE, noNvram = FALSE;
	char prefix[] = PREFIX_DEF;

	/* Parse Args */
	if (ejArgs(argc, argv, "%d %d", &iter, &last) < 2) {
		websError(wp, HTTP_ERR_BAD_REQUEST, HSERR_ARGV_PRNT);
		return -1;
	}

	/* Make Prefix */
	if (!make_wl_prefix(prefix, sizeof(prefix), 1, NULL))
		return -1;

	/* Parse NVRAM */
	nat = (bcm_decode_hspot_anqp_wan_metrics_t*)
		malloc(sizeof(bcm_decode_hspot_anqp_wan_metrics_t));
	if (!nat)
		return -1;
	memset(nat, 0, sizeof(*nat));
	if (!decode_hspot_wan_metrics(prefix, nat))
		noNvram = TRUE;

	/* Evaluate if entry is valid */
	valid = (!noNvram) ? TRUE : FALSE;

	/* Printing Header */
	ret += websWrite(wp, "<tr>");
	ret += websWrite(wp, "<th width=\"310\" valign=\"top\" rowspan=\"1\"");
	ret += websWrite(wp, "onMouseOver=\"return overlib('Set WAN Metrics "
		"to the BSS supported by the interface.', LEFT);\"");
	ret += websWrite(wp, "onMouseOut=\"return nd();\">");
	ret += websWrite(wp, "<input type=\"hidden\" name=\"wl_wanmetrics_\" value=\"5\">");
	ret += websWrite(wp, " &nbsp;&nbsp;");
	ret += websWrite(wp, "</th>");
	ret += websWrite(wp, "<td>&nbsp;&nbsp;</td>");

	/* Print linkStatus */
	ret += websWrite(wp, "<td><select name=\"wl_wanmetrics_0\"> "
	"<option value=\"1\" %s>Link Up</option> "
	"<option value=\"2\" %s>Link Down</option> "
	"<option value=\"3\" %s>Link Test</option> "
	"</select></td>",
	(valid ? (nat->linkStatus == HSPOT_WAN_LINK_UP ?
	SELECTED_STR : EMPTY_STR) : EMPTY_STR),
	(valid ? (nat->linkStatus == HSPOT_WAN_LINK_DOWN ?
	SELECTED_STR : EMPTY_STR) : EMPTY_STR),
	(valid ? (nat->linkStatus == HSPOT_WAN_LINK_TEST ?
	SELECTED_STR : EMPTY_STR) : EMPTY_STR));

	ret += websWrite(wp, "<td>:</td>");

	/* Print symmetricLink */
	ret += websWrite(wp, "<td><select name=\"wl_wanmetrics_1\"> "
	"<option value=\"0\" %s>Not Symmetric Link</option> "
	"<option value=\"1\" %s>Symmetric Link</option> "
	"</select></td>",
	(valid ? (nat->symmetricLink == HSPOT_WAN_NOT_SYMMETRIC_LINK ?
	SELECTED_STR : EMPTY_STR) : EMPTY_STR),
	(valid ? (nat->symmetricLink == HSPOT_WAN_SYMMETRIC_LINK ?
	SELECTED_STR : EMPTY_STR) : EMPTY_STR));

	ret += websWrite(wp, "<td>:</td>");

	/* Print atCapacity */
	ret += websWrite(wp, "<td><select name=\"wl_wanmetrics_2\"> "
	"<option value=\"0\" %s>Not At Capacity</option> "
	"<option value=\"1\" %s>At Capacity</option> "
	"</select></td>",
	(valid ? (nat->atCapacity == HSPOT_WAN_NOT_AT_CAPACITY ?
	SELECTED_STR : EMPTY_STR) : EMPTY_STR),
	(valid ? (nat->atCapacity == HSPOT_WAN_AT_CAPACITY ?
	SELECTED_STR : EMPTY_STR) : EMPTY_STR));

	ret += websWrite(wp, "<td>=</td>");

	/* Print dlinkSpeed */
	ret += websWrite(wp, "<td><input name=\"wl_wanmetrics_3\" "
	"value=\"%d\" size=\"6\" maxlength=\"6\"></td>", valid ? nat->dlinkSpeed : 0);

	ret += websWrite(wp, "<td>></td>");

	/* Print ulinkSpeed */
	ret += websWrite(wp, "<td><input name=\"wl_wanmetrics_4\" "
	"value=\"%d\" size=\"6\" maxlength=\"6\"></td>", valid ? nat->ulinkSpeed : 0);

	ret += websWrite(wp, "<td>=</td>");

	/* Print dlinkLoad */
	ret += websWrite(wp, "<td><input name=\"wl_wanmetrics_5\" "
	"value=\"%d\" size=\"6\" maxlength=\"6\"></td>", valid ? nat->dlinkLoad : 0);

	ret += websWrite(wp, "<td>></td>");

	/* Print ulinkLoad */
	ret += websWrite(wp, "<td><input name=\"wl_wanmetrics_6\" "
	"value=\"%d\" size=\"6\" maxlength=\"6\"></td>", valid ? nat->ulinkLoad : 0);

	ret += websWrite(wp, "<td>=</td>");

	/* Print lmd */
	ret += websWrite(wp, "<td><input name=\"wl_wanmetrics_7\" "
	"value=\"%d\" size=\"6\" maxlength=\"6\"></td>", valid ? nat->lmd : 0);

	ret += websWrite(wp, "</tr>");

	/* Clean up */
	if (nat) {
		free(nat);
		nat = NULL;
	}

	return ret;
}
REG_EJ_HANDLER(print_wl_wanmetrics);
/* ----------------------- 8. Wan Metrics - VALIDATE ----------------------- */
void
validate_wl_wanmetrics(webs_t wp, char *value, struct variable *v, char *varname)
{
	struct variable fields[] = {
		{ name: "wl_wanmetrics_%d_",
		longname: IP_LINK_STATUS,
		argv: ARGV("1", "2", "3") },

		{ name: "wl_wanmetrics_%d_",
		longname: IP_SYMM_LINK,
		argv: ARGV("0", "1") },

		{ name: "wl_wanmetrics_%d_",
		longname: IP_AT_CAP,
		argv: ARGV("0", "1") },

		{ name: "wl_wanmetrics_%d_",
		longname: IP_DOWN_SPEED,
		argv: ARGV("0", "65535") },

		{ name: "wl_wanmetrics_%d_",
		longname: IP_UP_SPEED,
		argv: ARGV("0", "65535") },

		{ name: "wl_wanmetrics_%d_",
		longname: "DownLink Load",
		argv: ARGV("0", "65535") },

		{ name: "wl_wanmetrics_%d_",
		longname: "UpLink Load",
		argv: ARGV("0", "65535") },

		{ name: "wl_wanmetrics_%d_",
		longname: "LMD",
		argv: ARGV("0", "65535") },
	};

	char nvram_val[NVVAL_BUFF];
	char row_name[NVNAME_BUFF], col_name[NVNAME_BUFF];
	char* val = NULL;

	char p_linkStatus[CODE_BUFF], p_symmetricLink[CODE_BUFF];
	char p_atCapacity[CODE_BUFF], p_dlinkSpeed[CODE_BUFF];
	char p_ulinkSpeed[CODE_BUFF], p_dlinkLoad[CODE_BUFF];
	char p_ulinkLoad[CODE_BUFF], p_lmd[CODE_BUFF];

	assert(v);

	ret_code = EINVAL;

	if (!value) {
		ret_code = 0;
		return;
	}

	memset(row_name, 0, sizeof(row_name));
	snprintf(row_name, sizeof(row_name), "%s_", NVNM_WL_WANMETRICS);

	memset(col_name, 0, sizeof(col_name));
	memset(p_linkStatus, 0, sizeof(p_linkStatus));
	if ((val = websGetVar(wp, strcat_r(row_name, "0", col_name), NULL)) != NULL)
		if (!strcmp(val, "1") || !strcmp(val, "2") || !strcmp(val, "3"))
			strncpy_n(p_linkStatus, val, CODE_BUFF);

	memset(col_name, 0, sizeof(col_name));
	memset(p_symmetricLink, 0, sizeof(p_symmetricLink));
	if ((val = websGetVar(wp, strcat_r(row_name, "1", col_name), NULL)) != NULL)
		if (!strcmp(val, "0") || !strcmp(val, "1"))
			strncpy_n(p_symmetricLink, val, CODE_BUFF);

	memset(col_name, 0, sizeof(col_name));
	memset(p_atCapacity, 0, sizeof(p_atCapacity));
	if ((val = websGetVar(wp, strcat_r(row_name, "2", col_name), NULL)) != NULL)
		if (!strcmp(val, "0") || !strcmp(val, "1"))
			strncpy_n(p_atCapacity, val, CODE_BUFF);

	memset(col_name, 0, sizeof(col_name));
	memset(p_dlinkSpeed, 0, sizeof(p_dlinkSpeed));
	if ((val = websGetVar(wp, strcat_r(row_name, "3", col_name), NULL)) != NULL)
			strncpy_n(p_dlinkSpeed, val, CODE_BUFF);

	memset(col_name, 0, sizeof(col_name));
	memset(p_ulinkSpeed, 0, sizeof(p_ulinkSpeed));
	if ((val = websGetVar(wp, strcat_r(row_name, "4", col_name), NULL)) != NULL)
			strncpy_n(p_ulinkSpeed, val, CODE_BUFF);

	memset(col_name, 0, sizeof(col_name));
	memset(p_dlinkLoad, 0, sizeof(p_dlinkLoad));
	if ((val = websGetVar(wp, strcat_r(row_name, "5", col_name), NULL)) != NULL)
			strncpy_n(p_dlinkLoad, val, CODE_BUFF);

	memset(col_name, 0, sizeof(col_name));
	memset(p_ulinkLoad, 0, sizeof(p_ulinkLoad));
	if ((val = websGetVar(wp, strcat_r(row_name, "6", col_name), NULL)) != NULL)
			strncpy_n(p_ulinkLoad, val, CODE_BUFF);

	memset(col_name, 0, sizeof(col_name));
	memset(p_lmd, 0, sizeof(p_lmd));
	if ((val = websGetVar(wp, strcat_r(row_name, "7", col_name), NULL)) != NULL)
			strncpy_n(p_lmd, val, CODE_BUFF);

	/* Delete entry if all fields are blank */
	if (!strlen(p_linkStatus) && !strlen(p_symmetricLink) &&
		!strlen(p_atCapacity) && !strlen(p_dlinkSpeed) &&
		!strlen(p_ulinkSpeed) && !strlen(p_dlinkLoad) &&
		!strlen(p_ulinkLoad) && !strlen(p_lmd)) {
		return;
	}

	/* Check individual fields */
	if (!valid_choice(wp, p_linkStatus, &fields[0])) {
		websBufferWrite(wp, HSERR_INVALID_INPUT,
			fields[0].longname, fields[0].longname); return;
		}
	if (!valid_choice(wp, p_symmetricLink, &fields[1])) {
		websBufferWrite(wp, HSERR_INVALID_INPUT,
			fields[1].longname, fields[1].longname); return;
		}
	if (!valid_choice(wp, p_atCapacity,		&fields[2])) {
		websBufferWrite(wp, HSERR_INVALID_INPUT,
			fields[2].longname, fields[2].longname); return;
		}
	if (!valid_range (wp, p_dlinkSpeed, &fields[3])) {
		websBufferWrite(wp, HSERR_INVALID_INPUT,
			fields[3].longname, fields[3].longname); return;
		}
	if (!valid_range (wp, p_ulinkSpeed,		&fields[4])) {
		websBufferWrite(wp, HSERR_INVALID_INPUT,
			fields[4].longname, fields[4].longname); return;
		}
	if (!valid_range (wp, p_dlinkLoad, &fields[5])) {
		websBufferWrite(wp, HSERR_INVALID_INPUT,
			fields[5].longname, fields[5].longname); return;
		}
	if (!valid_range (wp, p_ulinkLoad, &fields[6])) {
		websBufferWrite(wp, HSERR_INVALID_INPUT,
			fields[6].longname, fields[6].longname); return;
		}
	if (!valid_range (wp, p_lmd, &fields[7])) {
		websBufferWrite(wp, HSERR_INVALID_INPUT,
			fields[7].longname, fields[7].longname); return;
		}

	/* Do it */
	memset(nvram_val, 0, sizeof(nvram_val));
	snprintf(nvram_val, sizeof(nvram_val), WANMETRICS_FORMAT,
		p_linkStatus, p_symmetricLink, p_atCapacity, p_dlinkSpeed,
		p_ulinkSpeed, p_dlinkLoad, p_ulinkLoad, p_lmd);

	nvram_set(NVNM_WL_WANMETRICS, nvram_val);

	ret_code = 0;

	return;
}
/* ---------------- 9. Operator Fridendly Name List - PRINT ---------------- */
int
ej_print_wl_oplist(int eid, webs_t wp, int argc, char_t **argv)
{
	bcm_decode_hspot_anqp_operator_friendly_name_t* nat;

	int iter, last, ret = 0;
	bool valid = FALSE, noNvram = FALSE;
	char prefix[] = PREFIX_DEF;

	/* Parse Args */
	if (ejArgs(argc, argv, "%d %d", &iter, &last) < 2) {
		websError(wp, HTTP_ERR_BAD_REQUEST, HSERR_ARGV_PRNT);
		return -1;
	}

	/* Make Prefix */
	if (!make_wl_prefix(prefix, sizeof(prefix), 1, NULL))
		return -1;

	/* Parse NVRAM */
	nat = (bcm_decode_hspot_anqp_operator_friendly_name_t*)
		malloc(sizeof(bcm_decode_hspot_anqp_operator_friendly_name_t));
	if (!nat)
		return -1;
	memset(nat, 0, sizeof(*nat));
	if (!decode_hspot_op_list(prefix, nat))
		noNvram = TRUE;

	/* Start Printing */
	for (; iter <= last; iter++) {

		/* Evaluate if entry is valid */
		valid = ((iter < nat->numName) && !noNvram)? TRUE : FALSE;

		/* Printing Header */
		ret += websWrite(wp, "<tr>");
		ret += websWrite(wp, "<th width=\"310\" valign=\"top\" rowspan=\"1\"");
		ret += websWrite(wp, "onMouseOver=\"return overlib('Set Operator Friendly Name List"
			" to the BSS supported by the interface.', LEFT);\"");
		ret += websWrite(wp, "onMouseOut=\"return nd();\">");
		ret += websWrite(wp, "<input type=\"hidden\" name=\"wl_oplist_%d_\" "
			"value=\"2\">", iter);
		ret += websWrite(wp, " &nbsp;&nbsp;");
		ret += websWrite(wp, "</th>");
		ret += websWrite(wp, "<td>&nbsp;&nbsp;</td>");

		/* Print oper */
		ret += websWrite(wp, "<td><input name=\"wl_oplist_%d_0\" "
		"value=\"%s\" size=\"32\" maxlength=\"255\"></td>", iter,
		valid ? nat->duple[iter].name : EMPTY_STR);

		ret += websWrite(wp, WEBS_TAB);

		/* Print lang */
		ret += websWrite(wp, "<td><input name=\"wl_oplist_%d_1\" "
		"value=\"%s\" size=\"5\" maxlength=\"5\"></td>", iter,
		valid ? nat->duple[iter].lang : EMPTY_STR);

		ret += websWrite(wp, "</tr>");

	}

	/* Clean up */
	if (nat) {
		free(nat);
		nat = NULL;
	}

	return ret;
}
REG_EJ_HANDLER(print_wl_oplist);
/* --------------- 9. Operator Fridendly Name List - VALIDATE -------------- */
void
validate_wl_oplist(webs_t wp, char *value, struct variable *v, char *varname)
{
	struct variable fields[] = {
		{ name: "wl_oplist_%d_%d",
		longname: IP_OPER_NAME,
		argv: ARGV("0", "255") },

		{ name: "wl_oplist_%d_%d",
		longname: IP_LANG_CODE,
		argv: ARGV("0", "3") },
	};

	int count, iter;
	char nvram_val[NVVAL_BUFF], item_value[ITEM_BUFF];
	char row_name[NVNAME_BUFF], col_name[NVNAME_BUFF];
	char* val = NULL;

	char oper[NAME_BUFF], lang[CODE_BUFF];

	assert(v);

	ret_code = EINVAL;

	if (!value) {
		ret_code = 0;
		return;
	}
	count = atoi(value);

	memset(nvram_val, 0, sizeof(nvram_val));

	for (iter = 0; iter < count; iter++) {

		memset(row_name, 0, sizeof(row_name));
		snprintf(row_name, sizeof(row_name), "%s_%d_", "wl_oplist", iter);

		memset(col_name, 0, sizeof(col_name));
		memset(oper, 0, sizeof(oper));
		if ((val = websGetVar(wp, strcat_r(row_name, "0", col_name), NULL)) != NULL)
			strncpy_n(oper, val, NAME_BUFF);

		memset(col_name, 0, sizeof(col_name));
		memset(lang, 0, sizeof(lang));
		if ((val = websGetVar(wp, strcat_r(row_name, "1", col_name), NULL)) != NULL)
			strncpy_n(lang, val, CODE_BUFF);

		/* Delete entry if all fields are blank */
		if (!strlen(oper) && !strlen(lang)) {
			continue;
		}

		/* Fill in empty fields with default values or check for mendetory fields */
		if (!strlen(oper) && (strlen(lang) > 0)) {
			websBufferWrite(wp, HSERR_EMPTY_INPUT,
				fields[0].longname, fields[0].longname);
			continue;
		}

		if ((strlen(oper) > 0) && !strlen(lang)) {
			websBufferWrite(wp, HSERR_EMPTY_INPUT,
				fields[1].longname, fields[1].longname);
			continue;
		}

		/* Check individual fields */
		if (!valid_name(wp, oper, &fields[0])) {
			websBufferWrite(wp, HSERR_INVALID_INPUT,
				fields[0].longname, fields[0].longname);
			continue;
		}
		if (!valid_name(wp, lang, &fields[1])) {
			websBufferWrite(wp, HSERR_INVALID_INPUT,
				fields[1].longname, fields[1].longname);
			continue;
		}

		/* Do it */
		memset(item_value, 0, sizeof(item_value));
		snprintf(item_value, sizeof(item_value), "%s!%s", oper, lang);

		if (strlen(nvram_val))
			strncat(nvram_val, "|", min(1, NVVAL_BUFF-strlen(nvram_val)));

		strncat(nvram_val, item_value, min(strlen(item_value),
			NVVAL_BUFF-strlen(nvram_val)));
	}

	nvram_set("wl_oplist", nvram_val);

	ret_code = 0;

	return;
}
/* ----------------- 10. NAI Home Realm Query List - PRINT ----------------- */
int
ej_print_wl_homeqlist(int eid, webs_t wp, int argc, char_t **argv)
{
	bcm_decode_hspot_anqp_nai_home_realm_query_t* nat = NULL;

	int iter, last, ret = 0;
	bool valid = FALSE, noNvram = FALSE;
	char prefix[] = PREFIX_DEF;

	/* Parse Args */
	if (ejArgs(argc, argv, "%d %d", &iter, &last) < 2) {
		websError(wp, HTTP_ERR_BAD_REQUEST, HSERR_ARGV_PRNT);
		return -1;
	}

	/* Make Prefix */
	if (!make_wl_prefix(prefix, sizeof(prefix), 1, NULL))
		return -1;

	/* Parse NVRAM */
	nat = (bcm_decode_hspot_anqp_nai_home_realm_query_t*)
		malloc(sizeof(bcm_decode_hspot_anqp_nai_home_realm_query_t));
	if (!nat)
		return -1;
	memset(nat, 0, sizeof(*nat));
	if (!decode_hspot_homeq_list(prefix, nat))
		noNvram = TRUE;

	/* Start Printing */
	for (; iter <= last; iter++) {

		/* Evaluate if entry is valid */
		valid = ((iter < nat->count) && !noNvram)? TRUE : FALSE;

		/* Printing Header */
		ret += websWrite(wp, "<tr>");
		ret += websWrite(wp, "<th width=\"310\" valign=\"top\" rowspan=\"1\"");
		ret += websWrite(wp, "onMouseOver=\"return overlib('Set NAI Home Realm Query List "
			"to the BSS supported by the interface.', LEFT);\"");
		ret += websWrite(wp, "onMouseOut=\"return nd();\">");
		ret += websWrite(wp, "<input type=\"hidden\" name=\"wl_homeqlist_%d_\" "
			"value=\"1\">", iter);
		ret += websWrite(wp, " &nbsp;&nbsp;");
		ret += websWrite(wp, "</th>");
		ret += websWrite(wp, "<td>&nbsp;&nbsp;</td>");

		/* Print homerealm */
		ret += websWrite(wp, "<td><input name=\"wl_homeqlist_%d_0\" "
		"value=\"%s\" size=\"32\" maxlength=\"255\"></td>", iter,
		valid ? nat->data[iter].name : EMPTY_STR);

		ret += websWrite(wp, WEBS_TAB);

		/* Print encode */
		ret += websWrite(wp, "<td><select name=\"wl_homeqlist_%d_1\"> "
		"<option value=\"rfc4282\" %s>RFC4282</option> "
		"<option value=\"utf8\" %s>UTF8</option> "
		"</select></td>", iter,
		(valid ? (nat->data[iter].encoding == REALM_ENCODING_RFC4282 ?
		SELECTED_STR : EMPTY_STR) : EMPTY_STR),
		(valid ? (nat->data[iter].encoding == REALM_ENCODING_UTF8 ?
		SELECTED_STR : EMPTY_STR) : EMPTY_STR));

		ret += websWrite(wp, "</tr>");
	}

	/* Clean up */
	if (nat) {
		free(nat);
		nat = NULL;
	}

	return ret;
}
REG_EJ_HANDLER(print_wl_homeqlist);
/* ---------------- 10. NAI Home Realm Query List - VALIDATE --------------- */
void
validate_wl_homeqlist(webs_t wp, char *value, struct variable *v, char *varname)
{
	struct variable fields[] = {
		{ name: "wl_homeqlist_%d_%d",
		longname: IP_NHRQ_NAME,
		argv: ARGV("0", "255") },

		{ name: "wl_homeqlist_%d_%d",
		longname: IP_ENCODE_TYPE,
		argv: ARGV(ENC_RFC4282, ENC_UTF8) },
	};

	int count, iter;
	char nvram_val[NVVAL_BUFF], item_value[ITEM_BUFF];
	char row_name[NVNAME_BUFF], col_name[NVNAME_BUFF];
	char* val = NULL;

	char homerealm[NAME_BUFF], encode[CODE_BUFF];

	assert(v);

	ret_code = EINVAL;

	if (!value) {
		ret_code = 0;
		return;
	}
	count = atoi(value);

	memset(nvram_val, 0, sizeof(nvram_val));

	for (iter = 0; iter < count; iter++) {

		memset(row_name, 0, sizeof(row_name));
		snprintf(row_name, sizeof(row_name), "%s_%d_", NVNM_WL_HOMEQLIST, iter);

		memset(col_name, 0, sizeof(col_name));
		memset(homerealm, 0, sizeof(homerealm));
		if ((val = websGetVar(wp, strcat_r(row_name, "0", col_name), NULL)) != NULL)
				strncpy_n(homerealm, val, NAME_BUFF);

		memset(col_name, 0, sizeof(col_name));
		memset(encode, 0, sizeof(encode));
		if ((val = websGetVar(wp, strcat_r(row_name, "1", col_name), NULL)) != NULL)
			if (!strcmp(val, ENC_RFC4282) || !strcmp(val, ENC_UTF8))
				strncpy_n(encode, val, CODE_BUFF);

		/* Delete entry if all fields are blank */
		if (!strlen(homerealm)) {
			continue;
		}

		/* Check individual fields */
		if (!valid_name(wp, homerealm, &fields[0])) {
			websBufferWrite(wp, HSERR_INVALID_INPUT,
				fields[0].longname, fields[0].longname);
			continue;
		}
		if (!valid_choice(wp, encode, &fields[1])) {
			websBufferWrite(wp, HSERR_INVALID_INPUT,
				fields[1].longname, fields[1].longname);
			continue;
		}

		/* Do it */
		memset(item_value, 0, sizeof(item_value));
		snprintf(item_value, sizeof(item_value), "%s:%s", homerealm, encode);

		if (strlen(nvram_val))
			strncat(nvram_val, ";", min(1, NVVAL_BUFF-strlen(nvram_val)));

		strncat(nvram_val, item_value, min(strlen(item_value),
			NVVAL_BUFF-strlen(nvram_val)));
	}

	nvram_set(NVNM_WL_HOMEQLIST, nvram_val);

	ret_code = 0;

	return;
}
/* ---------------- 11. Connection Capability List - PRINT ----------------- */
int
ej_print_wl_concaplist(int eid, webs_t wp, int argc, char_t **argv)
{
	bcm_decode_hspot_anqp_connection_capability_t* nat = NULL;

	int iter, last, ret = 0;
	bool valid = FALSE, noNvram = FALSE;
	char prefix[] = PREFIX_DEF;

	/* Parse Args */
	if (ejArgs(argc, argv, "%d %d", &iter, &last) < 2) {
		websError(wp, HTTP_ERR_BAD_REQUEST, HSERR_ARGV_PRNT);
		return -1;
	}

	/* Make Prefix */
	if (!make_wl_prefix(prefix, sizeof(prefix), 1, NULL))
		return -1;

	/* Parse NVRAM */
	nat = (bcm_decode_hspot_anqp_connection_capability_t*)
		malloc(sizeof(bcm_decode_hspot_anqp_connection_capability_t));
	if (!nat)
		return -1;
	memset(nat, 0, sizeof(*nat));
	if (!decode_hspot_conncap_list(prefix, nat))
		noNvram = TRUE;

	/* Start Printing */
	for (; iter <= last; iter++) {

		/* Evaluate if entry is valid */
		valid = ((iter < nat->numConnectCap) && !noNvram)? TRUE : FALSE;

		/* Printing Header */
		ret += websWrite(wp, "<tr>");
		ret += websWrite(wp, "<th width=\"310\" valign=\"top\" rowspan=\"1\"");
		ret += websWrite(wp, "onMouseOver=\"return overlib('Set Connection Capability List "
			"to the BSS supported by the interface.', LEFT);\"");
		ret += websWrite(wp, "onMouseOut=\"return nd();\">");
		ret += websWrite(wp, "<input type=\"hidden\" name=\"wl_concaplist_%d_\" "
			"value=\"3\">", iter);
		ret += websWrite(wp, " &nbsp;&nbsp;");
		ret += websWrite(wp, "</th>");
		ret += websWrite(wp, "<td>&nbsp;&nbsp;</td>");

		/* Print protocol */
		ret += websWrite(wp, "<td><select name=\"wl_concaplist_%d_0\"> "
		"<option value=\"-1\" %s>Select</option> "
		"<option value=\"1\" %s>ICMP (0x1)</option> "
		"<option value=\"6\" %s>TCP (0x6)</option> "
		"<option value=\"17\" %s>UDP (0x11)</option> "
		"<option value=\"50\" %s>ESP (0x32)</option> "
		"</select></td>", iter,
		(valid ? (nat->tuple[iter].ipProtocol == (uint8)HSPOT_CC_IPPROTO_NONE ?
		SELECTED_STR : EMPTY_STR) : SELECTED_STR),
		(valid ? (nat->tuple[iter].ipProtocol == (uint8)HSPOT_CC_IPPROTO_ICMP ?
		SELECTED_STR : EMPTY_STR) : EMPTY_STR),
		(valid ? (nat->tuple[iter].ipProtocol == (uint8)HSPOT_CC_IPPROTO_TCP ?
		SELECTED_STR : EMPTY_STR) : EMPTY_STR),
		(valid ? (nat->tuple[iter].ipProtocol == (uint8)HSPOT_CC_IPPROTO_UDP ?
		SELECTED_STR : EMPTY_STR) : EMPTY_STR),
		(valid ? (nat->tuple[iter].ipProtocol == (uint8)HSPOT_CC_IPPROTO_ESP ?
		SELECTED_STR : EMPTY_STR) : EMPTY_STR));

		ret += websWrite(wp, WEBS_TAB);

		/* Print port */
		ret += websWrite(wp, "<td><select name=\"wl_concaplist_%d_1\"> "
		"<option value=\"-1\" %s>Select</option> "
		"<option value=\"0\" %s>Reserved (0x0)</option> "
		"<option value=\"20\" %s>FTP	(0x14)</option> "
		"<option value=\"22\" %s>SSH	(0x16)</option> "
		"<option value=\"80\" %s>HTTP (0x50)</option> "
		"<option value=\"443\" %s>HTTPS (0x1BB)</option> "
		"<option value=\"500\" %s>ISAKMP (0x1F4)</option> "
		"<option value=\"1723\" %s>PPTP (0x6BB)</option> "
		"<option value=\"4500\" %s>IPSEC (0x1194)</option> "
		"<option value=\"5060\" %s>SIP (0x13C4)</option> "
		"</select></td>", iter,
		(valid ? (nat->tuple[iter].portNumber == (uint16)HSPOT_CC_PORT_NONE ?
		SELECTED_STR : EMPTY_STR) : SELECTED_STR),
		(valid ? (nat->tuple[iter].portNumber == (uint16)HSPOT_CC_PORT_RESERVED ?
		SELECTED_STR : EMPTY_STR) : EMPTY_STR),
		(valid ? (nat->tuple[iter].portNumber == (uint16)HSPOT_CC_PORT_FTP ?
		SELECTED_STR : EMPTY_STR) : EMPTY_STR),
		(valid ? (nat->tuple[iter].portNumber == (uint16)HSPOT_CC_PORT_SSH ?
		SELECTED_STR : EMPTY_STR) : EMPTY_STR),
		(valid ? (nat->tuple[iter].portNumber == (uint16)HSPOT_CC_PORT_HTTP ?
		SELECTED_STR : EMPTY_STR) : EMPTY_STR),
		(valid ? (nat->tuple[iter].portNumber == (uint16)HSPOT_CC_PORT_HTTPS ?
		SELECTED_STR : EMPTY_STR) : EMPTY_STR),
		(valid ? (nat->tuple[iter].portNumber == (uint16)HSPOT_CC_PORT_ISAKMP ?
		SELECTED_STR : EMPTY_STR) : EMPTY_STR),
		(valid ? (nat->tuple[iter].portNumber == (uint16)HSPOT_CC_PORT_PPTP ?
		SELECTED_STR : EMPTY_STR) : EMPTY_STR),
		(valid ? (nat->tuple[iter].portNumber == (uint16)HSPOT_CC_PORT_IPSEC ?
		SELECTED_STR : EMPTY_STR) : EMPTY_STR),
		(valid ? (nat->tuple[iter].portNumber == (uint16)HSPOT_CC_PORT_SIP ?
		SELECTED_STR : EMPTY_STR) : EMPTY_STR));

		ret += websWrite(wp, WEBS_TAB);

		/* Print status */
		ret += websWrite(wp, "<td><select name=\"wl_concaplist_%d_2\"> "
		"<option value=\"-1\" %s>Select</option> "
		"<option value=\"0\" %s>Closed</option> "
		"<option value=\"1\" %s>Open</option> "
		"<option value=\"2\" %s>Unknown</option> "
		"</select></td>", iter,
		(valid ? (nat->tuple[iter].status == (uint8)HSPOT_CC_STATUS_NONE ?
		SELECTED_STR : EMPTY_STR) : SELECTED_STR),
		(valid ? (nat->tuple[iter].status == (uint8)HSPOT_CC_STATUS_CLOSED ?
		SELECTED_STR : EMPTY_STR) : EMPTY_STR),
		(valid ? (nat->tuple[iter].status == (uint8)HSPOT_CC_STATUS_OPEN ?
		SELECTED_STR : EMPTY_STR) : EMPTY_STR),
		(valid ? (nat->tuple[iter].status == (uint8)HSPOT_CC_STATUS_UNKNOWN ?
		SELECTED_STR : EMPTY_STR) : EMPTY_STR));

		ret += websWrite(wp, "</tr>");
	}

	/* Clean up */
	if (nat) {
		free(nat);
		nat = NULL;
	}

	return ret;
}
REG_EJ_HANDLER(print_wl_concaplist);
/* --------------- 11. Connection Capability List - VALIDATE --------------- */
void
validate_wl_concaplist(webs_t wp, char *value, struct variable *v, char *varname)
{
	struct variable fields[] = {
		{ name: "wl_concaplist_%d_%d",
		longname: IP_CONCAP_PROTOCOL,
		argv: ARGV("-1", "1", "6", "17", "50") },

		{ name: "wl_concaplist_%d_%d",
		longname: IP_CONCAP_PORT,
		argv: ARGV("-1", "0", "20", "22", "80", "443", "500", "1723", "4500", "5060") },

		{ name: "wl_concaplist_%d_%d",
		longname: IP_CONCAP_STATUS,
		argv: ARGV("-1", "0", "1", "2") },
	};

	int count, iter;
	char nvram_val[ITEM_BUFF], item_value[NAME_BUFF];
	char row_name[NVNAME_BUFF], col_name[NVNAME_BUFF];
	char* val = NULL;

	char protocol[CODE_BUFF], port[CODE_BUFF], status[CODE_BUFF];

	assert(v);

	ret_code = EINVAL;

	if (!value) {
		ret_code = 0;
		return;
	}
	count = atoi(value);

	memset(nvram_val, 0, sizeof(nvram_val));

	for (iter = 0; iter < count; iter++) {

		memset(row_name, 0, sizeof(row_name));
		snprintf(row_name, sizeof(row_name), "%s_%d_", NVNM_WL_CONCAPLIST, iter);

		memset(col_name, 0, sizeof(col_name));
		memset(protocol, 0, sizeof(protocol));
		if ((val = websGetVar(wp, strcat_r(row_name, "0", col_name), NULL)) != NULL)
		if (!strcmp(val, "1") || !strcmp(val, "6") ||
			!strcmp(val, "17") || !strcmp(val, "50"))
			strncpy_n(protocol, val, CODE_BUFF);

		memset(col_name, 0, sizeof(col_name));
		memset(port, 0, sizeof(port));
		if ((val = websGetVar(wp, strcat_r(row_name, "1", col_name), NULL)) != NULL)
		if (!strcmp(val, "0") || !strcmp(val, "20") || !strcmp(val, "22") ||
			!strcmp(val, "80") || !strcmp(val, "443") ||!strcmp(val, "500") ||
			!strcmp(val, "1723") || !strcmp(val, "4500") || !strcmp(val, "5060"))
			strncpy_n(port, val, CODE_BUFF);

		memset(col_name, 0, sizeof(col_name));
		memset(status, 0, sizeof(status));
		if ((val = websGetVar(wp, strcat_r(row_name, "2", col_name), NULL)) != NULL)
			if (!strcmp(val, "0") || !strcmp(val, "1") || !strcmp(val, "2"))
				strncpy_n(status, val, CODE_BUFF);

		/* Delete entry if any field is blank */
		if (!strlen(protocol) || !strlen(port)) {
			continue;
		}

		/* Check individual fields */
		if (!valid_choice(wp, protocol, &fields[0])) {
			websBufferWrite(wp, HSERR_INVALID_INPUT,
				fields[0].longname, fields[0].longname);
			continue;
		}
		if (!valid_choice(wp, port, &fields[1])) {
			websBufferWrite(wp, HSERR_INVALID_INPUT,
				fields[1].longname, fields[1].longname);
			continue;
		}
		if (!valid_choice(wp, status, &fields[2])) {
			websBufferWrite(wp, HSERR_INVALID_INPUT,
				fields[2].longname, fields[2].longname);
			continue;
		}

		/* Do it */
		memset(item_value, 0, sizeof(item_value));
		snprintf(item_value, sizeof(item_value), "%s:%s:%s", protocol, port, status);

		if (strlen(nvram_val))
			strncat(nvram_val, ";", min(1, ITEM_BUFF - strlen(nvram_val)));

		strncat(nvram_val, item_value, min(strlen(item_value),
			ITEM_BUFF-strlen(nvram_val)));
	}

	nvram_set(NVNM_WL_CONCAPLIST, nvram_val);

	ret_code = 0;

	return;
}
/* ----------------- 12. OSU Provider List - Icon - ONCHANGE --------------- */
int
ej_icon_change(int eid, webs_t wp, int argc, char_t **argv)
{
	int sel_idx = 0, ret = 0;
	int valid = 1;

	bcm_decode_hspot_anqp_icon_metadata_t metadata;
	memset(&metadata, 0, sizeof(metadata));

	/* Parse Args */
	if (ejArgs(argc, argv, "%d", &sel_idx) < 1) {
		websError(wp, HTTP_ERR_BAD_REQUEST, HSERR_ARGV_PRNT);
		return -1;
	}
	valid =  sel_idx == -1 ? 0 : 1;

	/* Get the Image Filename using sel_idx and read Metadata */
	if (valid)
		get_icon_metadata(ICONPATH, g_icon_files.icons[sel_idx].icon_filename, &metadata);

	ret += websWrite(wp, "var width = new String(\"%d\"); \n",
		valid ? metadata.width : 0);
	ret += websWrite(wp, "var height = new String(\"%d\"); \n",
		valid ? metadata.height : 0);
	ret += websWrite(wp, "var langcode = new String(\"%s\"); \n",
		valid ? metadata.lang : EMPTY_STR);
	ret += websWrite(wp, "var mimetype = new String(\"%s\"); \n",
		valid ? (char*)metadata.type : EMPTY_STR);

	return ret;
}
REG_EJ_HANDLER(icon_change);
/* ------------------ 12. OSU Provider List - POPUP PRINT ------------------ */
int
ej_print_popup_osup(int eid, webs_t wp, int argc, char_t **argv)
{
	icon_list_t *icons_on_ap = NULL;
	bcm_decode_hspot_anqp_osu_provider_list_ex_t *nat = NULL;
	bcm_decode_hspot_anqp_osu_provider_ex_t *provider_nat = NULL;

	int osup_idx = -1, iter = 0, last = 0, ret = 0;
	bool valid = FALSE, noNvram = FALSE;
	char prefix[] = PREFIX_DEF, err_nvram[NVNAME_BUFF] = {0};

	int iter_icons = 0, is_selected = 0, icon_valid = 0;
	char icon_path[NAME_BUFF] = {0};

	/* Parse Args */
	if (ejArgs(argc, argv, "%d%d%d", &osup_idx, &iter, &last) < 3) {
		websError(wp, HTTP_ERR_BAD_REQUEST, HSERR_ARGV_PRNT);
		return -1;
	}

	/* Make Prefix */
	if (!make_wl_prefix(prefix, sizeof(prefix), 1, NULL))
		return -1;

	/* Parse NVRAM */
	nat = (bcm_decode_hspot_anqp_osu_provider_list_ex_t*)
		malloc(sizeof(bcm_decode_hspot_anqp_osu_provider_list_ex_t));
	if (!nat)
		return -1;
	memset(nat, 0, sizeof(*nat));
	if (!decode_hspot_osup_list(prefix, nat, err_nvram))
		noNvram = TRUE;
	else
		provider_nat = &nat->osuProvider[osup_idx];

	/* Get Icon files from Icon directory on AP */
	sprintf(icon_path, "%s%s%s", ICONPATH, ";", USERDEFINED_ICONPATH);
	icons_on_ap = (icon_list_t*) malloc(sizeof(icon_list_t));
	if (!icons_on_ap) {
		ret = -1; goto EJ_PRINT_POPUP_OSUP_DONE;
	}
	memset(icons_on_ap, 0, sizeof(*icons_on_ap));
	if (!get_icon_files(icon_path, icons_on_ap)) {
		ret = -1; goto EJ_PRINT_POPUP_OSUP_DONE;
	}

	/* Evaluate if entry is valid */
	valid = (provider_nat->uriLength &&
		provider_nat->methodLength &&
		provider_nat->name.duple[0].nameLen &&
		provider_nat->iconMetadata[0].filenameLength) ? TRUE : FALSE;

	/* Start Printing */
	ret += websWrite(wp, "var osupdata = new String(\"");
	ret += websWrite(wp, "<table id=\'osupdtlid\'>");
	ret += websWrite(wp, "<thead>");
	ret += websWrite(wp, "<tr> <th></th> <th>Name</th> <th>Lang</th></tr>");
	ret += websWrite(wp, "</thead>");

	/* Print OSU Friendly Names */
	for (iter = 0; iter < last; iter++) {

		ret += websWrite(wp, "<tr>");
		if (!iter)
			ret += websWrite(wp, "<td><p>OSU Friendly Name : <p></td>");
		else
			ret += websWrite(wp, WEBS_TAB);

		/* Print Friendly Name */
		ret += websWrite(wp, "<td><input id=\'fname_%d\' value = \'%s\' size=\'30\'>",
		iter, valid ?
		(char*)provider_nat->name.duple[iter].name : EMPTY_STR);

		ret += websWrite(wp, "</td>");

		/* Print Language Code */
		ret += websWrite(wp, "<td><input id=\'lang_%d\' value = \'%s\' size=\'5\'>",
		iter, valid ?
		(char*)provider_nat->name.duple[iter].lang : EMPTY_STR);

		ret += websWrite(wp, "</td>");
		ret += websWrite(wp, "</tr>");
	}

	/* Print OSU Server URI */
	ret += websWrite(wp, "<tr>");
	ret += websWrite(wp, "<td>OSU Server URI : </td>");
	ret += websWrite(wp, "<td><input id=\'%s\' value = \'%s\' size=\'30\'>",
		"osuserverid", valid ? (char*)provider_nat->uri : EMPTY_STR);
	ret += websWrite(wp, "</td>");
	ret += websWrite(wp, "</tr>");

	/* Print OSU Method */
	ret += websWrite(wp, "<tr></td>");
	ret += websWrite(wp, "<td>OSU Method : </td>");
	ret += websWrite(wp, "<td><select id='osumethodid'>");
	ret += websWrite(wp, "<option value='0' %s>OMA-DM</option>",
		provider_nat->method[0] ? EMPTY_STR : SELECTED_STR);
	ret += websWrite(wp, "<option value='1' %s>SOAP-XML</option>",
		provider_nat->method[0] ? SELECTED_STR : EMPTY_STR);
	ret += websWrite(wp, "</select>");
	ret += websWrite(wp, "</td>");
	ret += websWrite(wp, "</tr>");

	/* Print OSU NAI */
	ret += websWrite(wp, "<tr>");
	ret += websWrite(wp, "<td><p>OSU NAI : <p></td>");
	ret += websWrite(wp, "<td><input id=\'osunai\' value = \'%s\' size=\'30\'>",
		valid ? (char*)provider_nat->nai : EMPTY_STR);
	ret += websWrite(wp, "</td></tr>");

	/* Print OSU Server Description */
	for (iter = 0; iter < last; iter++) {

		ret += websWrite(wp, "<tr>");
		if (!iter)
			ret += websWrite(wp, "<td><p>OSU Service Description : <p></td>");
		else
			ret += websWrite(wp, WEBS_TAB);

		/* Print Friendly Name */
		ret += websWrite(wp, "<td><input id=\'svrdesc_%d\' value = \'%s\' size=\'30\'>",
		iter, valid ?
		(char*)provider_nat->desc.duple[iter].name : EMPTY_STR);

		ret += websWrite(wp, "</td>");

		/* Print Language Code */
		ret += websWrite(wp, "<td><input id=\'svrdesclang_%d\' value = \'%s\' size=\'5\'>",
		iter, valid ?
		(char*)provider_nat->desc.duple[iter].lang : EMPTY_STR);

		ret += websWrite(wp, "</td>");
		ret += websWrite(wp, "</tr>");
	}
	ret += websWrite(wp, "</table>");

	/* Print OSU Server Icons */
	ret += websWrite(wp, "<br/>");
	ret += websWrite(wp, "<table>");
	ret += websWrite(wp, "<tr></tr>");
	ret += websWrite(wp, "<tr><th>Icons</th> <th></th><th>Width</th> "
		"<th></th> <th>Height</th> <th></th> <th>Lang</th> <th>Type</th></tr>");
	for (iter = 0; iter <= last; iter++) {

		is_selected = 0, icon_valid = 0;
		ret += websWrite(wp, "<tr>");
		ret += websWrite(wp, "<td><select id=\'iconsel_%d\' "
			"onChange=\'window.opener.onchange_icon(%d)\'>", iter, iter);

		/* Print Icon pulldown - default value */
		ret += websWrite(wp, "<option value=\'-1\'>Select</option>");

		/* No Icon is selected */
		if (!provider_nat->iconMetadata[iter].filenameLength) {
			icon_valid = 0;
		}

		/* Check if Icon is selected, and fill icon pulldown */
		for (iter_icons = 0; iter_icons < icons_on_ap->file_count; iter_icons++) {

			/* Check for selected icon matching name with icons on AP */
			is_selected = iter < provider_nat->iconMetadataCount ?
				(!strncasecmp(icons_on_ap->icons[iter_icons].icon_filename,
				provider_nat->iconMetadata[iter].filename,
				strlen(icons_on_ap->icons[iter_icons].icon_filename)) ? 1 : 0) : 0;

			/* Remember that icon is valid for this pulldown */
			if (is_selected)
				icon_valid = 1;

			/* Print Icon pulldown */
			ret += websWrite(wp, "<option value=\'%d\' %s>%s</option>", iter_icons,
				is_selected ? SELECTED_STR : EMPTY_STR,
				icons_on_ap->icons[iter_icons].icon_filename);
		}

		/* Print Icon Metadata */
		ret += websWrite(wp, "</select>");
		ret += websWrite(wp, "</td>");

		/* Print Width */
		ret += websWrite(wp, "<td></td><td width=20><input id=\'iwidth_%d\' "
		"value = \'%d\' size=\'8\' readonly>",
		iter, icon_valid ?
		provider_nat->iconMetadata[iter].width : 0);

		ret += websWrite(wp, "</td>");

		/* Print Height */
		ret += websWrite(wp, "<td></td><td width=20><input id=\'iheight_%d\' "
		"value = \'%d\' size=\'8\' readonly>",
		iter, icon_valid ?
		provider_nat->iconMetadata[iter].height : 0);

		ret += websWrite(wp, "</td>");

		/* Print Language Code */
		ret += websWrite(wp, "<td></td><td><input id=\'ilangcode_%d\' "
		"value = \'%s\' size=\'8\' readonly>",
		iter, icon_valid ?
		provider_nat->iconMetadata[iter].lang : EMPTY_STR);

		ret += websWrite(wp, "</td>");

		/* Print MIME Type */
		ret += websWrite(wp, "<td><input id=\'itype_%d\' "
		"value = \'%s\' size=\'11\' readonly>",
		iter, icon_valid ?
		(char*)provider_nat->iconMetadata[iter].type : EMPTY_STR);

		ret += websWrite(wp, "</td></tr>");
	}

	/* Print Save & Cancel Buttons */
	ret += websWrite(wp, "<tr height=\'5\'></tr>");
	ret += websWrite(wp, "<tr><td colspan=\'5\' id=\'errmsg\'></td>");
	ret += websWrite(wp, "<td></td><td></td><td><input "
		"type=button value=Save onClick=\'window.opener.onsave_osup();\'>");
	ret += websWrite(wp, "<input type=button value=Cancel "
		"onClick=\'window.opener.onclose_osup();\'></td></tr>");
	ret += websWrite(wp, "</table>");
	ret += websWrite(wp, "\");\n");

EJ_PRINT_POPUP_OSUP_DONE:
	if (nat) {
		free(nat);
		nat = NULL;
	}
	if (icons_on_ap) {
		free(icons_on_ap);
		icons_on_ap = NULL;
	}

	return ret;
}
REG_EJ_HANDLER(print_popup_osup);
/* --------------------- 12. OSU Provider List - PRINT --------------------- */
int
ej_print_wl_osuplist(int eid, webs_t wp, int argc, char_t **argv)
{
	bcm_decode_hspot_anqp_osu_provider_list_ex_t *nat = NULL;

	int iter, last, ret = 0;
	bool valid = FALSE, noNvram = FALSE;
	char prefix[] = PREFIX_DEF, err_nvram[NVNAME_BUFF] = {0};

	/* Parse Args */
	if (ejArgs(argc, argv, "%d %d", &iter, &last) < 2) {
		websError(wp, HTTP_ERR_BAD_REQUEST, HSERR_ARGV_PRNT);
		return -1;
	}

	/* Make Prefix */
	if (!make_wl_prefix(prefix, sizeof(prefix), 1, NULL))
		return -1;

	/* Parse NVRAM */
	nat = (bcm_decode_hspot_anqp_osu_provider_list_ex_t*)
		malloc(sizeof(bcm_decode_hspot_anqp_osu_provider_list_ex_t));
	if (!nat)
		return -1;
	memset(nat, 0, sizeof(*nat));
	if (!decode_hspot_osup_list(prefix, nat, err_nvram))
		noNvram = TRUE;

	/* Start Printing */
	for (; iter <= last; iter++) {

		/* Evaluate if entry is valid */
		valid = ((iter < nat->osuProviderCount) && !noNvram)? TRUE : FALSE;

		/* Printing Header */
		ret += websWrite(wp, "<tr>");
		ret += websWrite(wp, "<th width=\"310\"");
		ret += websWrite(wp, "onMouseOver=\"return overlib('OSU Provider # %d "
			"'s information.', LEFT);\"", iter+1);
		ret += websWrite(wp, "onMouseOut=\"return nd();\">");
		ret += websWrite(wp, "Provider # %d : &nbsp;&nbsp;", iter+1);
		ret += websWrite(wp, "</th>");
		ret += websWrite(wp, "<td>&nbsp;&nbsp;</td>");

		/* Print Friendly Name */
		ret += websWrite(wp, "<td><input id=\"wl_osu_frndname_%d\" "
		"name=\"wl_osu_frndname_%d\" value=\"%s\" size=\"17\" maxlength=\"255\" "
		"readonly ></td>", iter, iter,
		valid ? (char*)nat->osuProvider[iter].name.duple[0].name : EMPTY_STR);

		ret += websWrite(wp, WEBS_TAB);

		/* Print Server URI */
		ret += websWrite(wp, "<td><input id=\"wl_osu_uri_%d\" "
		"name=\"wl_osu_uri_%d\" value=\"%s\" size=\"32\" maxlength=\"128\" "
		"readonly ></td>", iter, iter,
		valid ? (char*)nat->osuProvider[iter].uri : EMPTY_STR);

		ret += websWrite(wp, WEBS_TAB);

		/* Print OSU NAI */
		ret += websWrite(wp, "<td><input id=\"wl_osu_nai_%d\" "
		"name=\"wl_osu_nai_%d\" value=\"%s\" size=\"17\" maxlength=\"128\" "
		"readonly ></td>", iter, iter,
		valid ? (char*)nat->osuProvider[iter].nai : EMPTY_STR);

		ret += websWrite(wp, WEBS_TAB);

		/* Print OSU Method */
		ret += websWrite(wp, "<td><input id=\"wl_osu_method_%d\" "
		"name=\"wl_osu_method_%d\" value=\"%s\" size=\"10\" maxlength=\"255\" "
		"readonly ></td>", iter, iter,
		valid ? (nat->osuProvider[iter].method[0] ? SOAP_XML : OMA_DM) : EMPTY_STR);

		ret += websWrite(wp, WEBS_TAB);

		/* Print OSU Icon */
		ret += websWrite(wp, "<td><input id=\"wl_osu_icons_%d\" "
		"name=\"wl_osu_icons_%d\" value=\"%s\" size=\"15\" maxlength=\"255\" "
		"readonly ></td>", iter, iter,
		valid ? (char*)nat->osuProvider[iter].iconMetadata[0].filename : EMPTY_STR);

		ret += websWrite(wp, WEBS_TAB);

		/* Print Modify button */
		ret += websWrite(wp, "<td><input name=\"wl_osumodify_%d\" "
		"type=button value=Modify onClick=\"onmodify_osup(%d);\"></td>", iter, iter);

		/* Print Delete button */
		ret += websWrite(wp, "<td><input name=\"wl_osudelete_%d\" "
		"type=button value=Delete onClick=\"ondelete_osup(%d);\"></td>", iter, iter);

		/* Print Hidden Controls */
		ret += websWrite(wp, "<td><input id=\"wl_osu_frndname_hid_%d\" "
		"name=\"wl_osu_frndname_hid_%d\" style=\"display:none;\" value=\'%s\'></td>",
		iter, iter, valid ? (char*)nat->osuProvider[iter].nameInfo : EMPTY_STR);

		ret += websWrite(wp, "<td><input id=\"wl_osu_icons_hid_%d\" "
		"name=\"wl_osu_icons_hid_%d\" style=\"display:none;\" value=\'%s\'></td>",
		iter, iter, valid ? (char*)nat->osuProvider[iter].iconInfo : EMPTY_STR);

		ret += websWrite(wp, "<td><input id=\"wl_osu_servdesc_%d\" "
		"name=\"wl_osu_servdesc_%d\" style=\"display:none;\" value=\'%s\'></td>",
		iter, iter, valid ? (char*)nat->osuProvider[iter].descInfo : EMPTY_STR);

		ret += websWrite(wp, "</tr>");
	}

	/* Clean up */
	if (nat) {
		free(nat);
		nat = NULL;
	}

	return ret;
}
REG_EJ_HANDLER(print_wl_osuplist);
/* -------------------- 12. OSU Provider List - VALIDATE ------------------- */
void
validate_wl_osuplist(webs_t wp, char *value, struct variable *v, char *varname)
{
	struct variable fields[] = {
		{ name: NVNM_WL_OSU_FRNDNAME_HID,
		longname: IP_OSU_FRNDLY_NAME,
		argv: ARGV("0", "2048") },

		{ name: NVNM_WL_OSU_URI,
		longname: IP_OSU_URI,
		argv: ARGV("0", "512") },

		{ name: NVNM_WL_OSU_NAI,
		longname: IP_OSU_NAI,
		argv: ARGV("0", "512") },

		{ name: NVNM_WL_OSU_METHOD,
		longname: IP_OSU_METHOD,
		argv: ARGV("0", "10") },

		{ name: NVNM_WL_OSU_ICONS_HID,
		longname: IP_OSU_ICON,
		argv: ARGV("0", "255") },

		{ name: NVNM_WL_OSU_SERVDESC,
		longname: IP_OSU_SERV_DESC,
		argv: ARGV("0", "2048") }
	};

	int count, iter;
	char nvram_val[NVVAL_BUFF], item_value[NVVAL_BUFF];
	char row_name[NVNAME_BUFF], osu_method[CODE_BUFF];
	char* val = NULL;
	char *frndname, *nai, *method, *icons, *servdesc, *uri;
	assert(v);

	ret_code = EINVAL;

	if (!value) {
		ret_code = 0;
		return;
	}
	frndname = (char*)malloc(NVVAL_BUFF);
	nai = (char*)malloc(NAME_BUFF);
	method = (char*)malloc(CODE_BUFF);
	icons = (char*)malloc(NAME_BUFF);
	servdesc = (char*)malloc(NVVAL_BUFF);
	uri = (char*)malloc(NAME_BUFF);

	if (!frndname || !nai || !method || !icons || !servdesc || !uri)
		goto VALIDATE_OSUPLIST_DONE;

	count = atoi(value);
	memset(nvram_val, 0, sizeof(nvram_val));
	for (iter = 0; iter < count; iter++) {
		memset(row_name, 0, sizeof(row_name));
		snprintf(row_name, sizeof(row_name), "%s_%d", fields[0].name, iter);
		memset(frndname, 0, NVVAL_BUFF);
		if ((val = websGetVar(wp, row_name, NULL)) != NULL)
			strncpy_n(frndname, val, NVVAL_BUFF);

		memset(row_name, 0, sizeof(row_name));
		snprintf(row_name, sizeof(row_name), "%s_%d", fields[1].name, iter);
		memset(uri, 0, NAME_BUFF);
		if ((val = websGetVar(wp, row_name, NULL)) != NULL)
			strncpy_n(uri, val, NAME_BUFF);

		memset(row_name, 0, sizeof(row_name));
		snprintf(row_name, sizeof(row_name), "%s_%d", fields[2].name, iter);
		memset(nai, 0, NAME_BUFF);
		if ((val = websGetVar(wp, row_name, NULL)) != NULL)
			strncpy_n(nai, val, NAME_BUFF);

		memset(row_name, 0, sizeof(row_name));
		snprintf(row_name, sizeof(row_name), "%s_%d", fields[3].name, iter);
		memset(method, 0, CODE_BUFF);
		if ((val = websGetVar(wp, row_name, NULL)) != NULL)
			strncpy_n(method, val, CODE_BUFF);

		memset(row_name, 0, sizeof(row_name));
		snprintf(row_name, sizeof(row_name), "%s_%d", fields[4].name, iter);
		memset(icons, 0, NAME_BUFF);
		if ((val = websGetVar(wp, row_name, NULL)) != NULL)
			strncpy_n(icons, val, NAME_BUFF);

		memset(row_name, 0, sizeof(row_name));
		snprintf(row_name, sizeof(row_name), "%s_%d", fields[5].name, iter);
		memset(servdesc, 0, NVVAL_BUFF);
		if ((val = websGetVar(wp, row_name, NULL)) != NULL)
			strncpy_n(servdesc, val, NVVAL_BUFF);

		/* Delete entry if all fields are blank */
		if (!strlen(frndname) && !strlen(uri) && !strlen(method) && !strlen(icons)) {
			continue;
		}

		/* Fill in empty fields with default values or check for mendetory fields */
		if (!strlen(frndname)) {
			websBufferWrite(wp, HSERR_EMPTY_INPUT,
				fields[0].longname, fields[0].longname);
			continue;
		}
		if (!strlen(uri)) {
			websBufferWrite(wp, HSERR_EMPTY_INPUT,
				fields[1].longname, fields[1].longname);
			continue;
		}
		if (!strlen(method)) {
			websBufferWrite(wp, HSERR_EMPTY_INPUT,
				fields[3].longname, fields[3].longname);
			continue;
		}
		if (!strlen(icons)) {
			websBufferWrite(wp, HSERR_EMPTY_INPUT,
				fields[4].longname, fields[4].longname);
			continue;
		}

		/* Check individual fields */
		if (!valid_name(wp, frndname, &fields[0])) {
			websBufferWrite(wp, HSERR_INVALID_INPUT,
				fields[0].longname, fields[0].longname);
			continue;
		}
		if (!valid_name(wp, uri, &fields[1])) {
			websBufferWrite(wp, HSERR_INVALID_INPUT,
				fields[1].longname, fields[1].longname);
			continue;
		}
		if (!valid_name(wp, nai, &fields[2])) {
			websBufferWrite(wp, HSERR_INVALID_INPUT,
				fields[2].longname, fields[2].longname);
			continue;
		}
		if (!valid_name(wp, method, &fields[3])) {
			websBufferWrite(wp, HSERR_INVALID_INPUT,
				fields[3].longname, fields[3].longname);
			continue;
		}
		if (!valid_name(wp, icons, &fields[4])) {
			websBufferWrite(wp, HSERR_INVALID_INPUT,
				fields[4].longname, fields[4].longname);
			continue;
		}
		if (!valid_name(wp, servdesc, &fields[5])) {
			websBufferWrite(wp, HSERR_INVALID_INPUT,
				fields[5].longname, fields[5].longname);
			continue;
		}

		if (!strncasecmp(method, OMA_DM, strlen(OMA_DM)))
			strncpy_n(osu_method, OMADM_NVVAL, CODE_BUFF);
		else
			strncpy_n(osu_method, SOAP_NVVAL, CODE_BUFF);

		memset(item_value, 0, sizeof(item_value));

		/* Do it */
		if (!strncasecmp(v->name, NVNM_WL_OSU_FRNDNAME,
			strlen(NVNM_WL_OSU_FRNDNAME)))
			snprintf(item_value, sizeof(item_value), "%s", frndname);
		else if (!strncasecmp(v->name, NVNM_WL_OSU_URI,
			strlen(NVNM_WL_OSU_URI)))
			snprintf(item_value, sizeof(item_value), "%s", uri);
		else if (!strncasecmp(v->name, NVNM_WL_OSU_NAI,
			strlen(NVNM_WL_OSU_NAI)))
			snprintf(item_value, sizeof(item_value), "%s", nai);
		else if (!strncasecmp(v->name, NVNM_WL_OSU_METHOD,
			strlen(NVNM_WL_OSU_METHOD)))
			snprintf(item_value, sizeof(item_value), "%s", osu_method);
		else if (!strncasecmp(v->name, NVNM_WL_OSU_ICONS,
			strlen(NVNM_WL_OSU_ICONS)))
			snprintf(item_value, sizeof(item_value), "%s", icons);
		else if (!strncasecmp(v->name, NVNM_WL_OSU_SERVDESC,
			strlen(NVNM_WL_OSU_SERVDESC)))
			snprintf(item_value, sizeof(item_value), "%s", servdesc);

		if (strlen(nvram_val))
			strncat(nvram_val, ";", min(1, NVVAL_BUFF-strlen(nvram_val)));

		strncat(nvram_val, item_value, min(strlen(item_value),
			NVVAL_BUFF-strlen(nvram_val)));
	}

	nvram_set(v->name, nvram_val);

	ret_code = 0;

VALIDATE_OSUPLIST_DONE:
	if (frndname) {
		free(frndname);
		frndname = NULL;
	}
	if (nai) {
		free(nai);
		nai = NULL;
	}
	if (method) {
		free(method);
		method = NULL;
	}
	if (icons) {
		free(icons);
		icons = NULL;
	}
	if (servdesc) {
		free(servdesc);
		servdesc = NULL;
	}
	if (uri) {
		free(uri);
		uri = NULL;
	}

	return;
}
/* -------------------- 13. Available Icon List - PRINT -------------------- */
int
ej_print_iconlist(int eid, webs_t wp, int argc, char_t **argv)
{
	int iter, last, ret = 0;

	icon_list_t *prebuild_icons;
	icon_list_t *userdef_icons;

	/* Parse Args */
	if (ejArgs(argc, argv, "%d %d", &iter, &last) < 2) {
		websError(wp, HTTP_ERR_BAD_REQUEST, HSERR_ARGV_PRNT);
		return -1;
	}

	/* Get Icon files from Icon directory on AP */
	prebuild_icons = (icon_list_t*)malloc(sizeof(icon_list_t));
	userdef_icons = (icon_list_t*)malloc(sizeof(icon_list_t));
	if (!prebuild_icons || !userdef_icons) {
		ret = -1; goto EJ_PRINT_ICONLIST_DONE;
	}
	memset(prebuild_icons, 0, sizeof(*prebuild_icons));
	if (!get_icon_files(ICONPATH, prebuild_icons)) {
		ret = -1; goto EJ_PRINT_ICONLIST_DONE;
	}
	memset(userdef_icons, 0, sizeof(*userdef_icons));
	if (!get_icon_files(USERDEFINED_ICONPATH, userdef_icons)) {
		ret = -1; goto EJ_PRINT_ICONLIST_DONE;
	}

	/* Start Printing */
	ret += websWrite(wp, "<p><table border=\"0\" cellpadding=\"0\" cellspacing=\"0\"><tr>");
	ret += websWrite(wp, "<th width=\"310\" valign=\"top\" rowspan=\"1\"");
	ret += websWrite(wp, "onMouseOver=\"return overlib('Update Icon List.', LEFT);\"");
	ret += websWrite(wp, "onMouseOut=\"return nd();\">");
	ret += websWrite(wp, "Available Icons List :&nbsp;&nbsp;</th>");
	ret += websWrite(wp, "<td>&nbsp;&nbsp;</td>");
	ret += websWrite(wp, "<td class=\"label\">Icon Name</td>");
	ret += websWrite(wp, "<td class=\"label\">Delete</td>");
	ret += websWrite(wp, "</tr>");
	for (iter = 0; iter <= last; iter++) {

		/* Printing Header */
		ret += websWrite(wp, "<tr>");
		ret += websWrite(wp, "<th width=\"310\" valign=\"top\" rowspan=\"1\"");
		ret += websWrite(wp, "onMouseOver=\"return overlib('Update Icon List.', LEFT);\"");
		ret += websWrite(wp, "onMouseOut=\"return nd();\">");
		ret += websWrite(wp, " &nbsp;&nbsp;");
		ret += websWrite(wp, "</th>");
		ret += websWrite(wp, "<td>&nbsp;&nbsp;</td>");

		/* If Icon already present than show the icon name */
		if (iter < prebuild_icons->file_count) {

		ret += websWrite(wp, "<td><input id=\'icon_readonly_%d\' "
			"value = \'%s\' size=\'30\' readonly>", iter,
			(char*)prebuild_icons->icons[iter].icon_filename);
		ret += websWrite(wp, "</td>");
		ret += websWrite(wp, "</tr>");
		}
		/* and provide an option to delete the existing item, if icon user defined. */
		else if ((iter - prebuild_icons->file_count) < userdef_icons->file_count) {

		ret += websWrite(wp, "<td><input id=\'icon_%d\' value = \'%s\' size=\'30\'>", iter,
		(char*)userdef_icons->icons[iter - prebuild_icons->file_count].icon_filename);
		ret += websWrite(wp, "</td>");
		ret += websWrite(wp, "<td><input id=\'icon_del_%d\' type = \'checkbox\'>", iter);
		ret += websWrite(wp, "</td>");
		ret += websWrite(wp, "</tr>");
		}
		/* else provide an option to browse the icon from the system */
		/* and upload to router. */
		else {

		ret += websWrite(wp, "<td><input id=\'icon_browse_%d\' "
			"type=\'file\' name=\'file\'>", iter);
		ret += websWrite(wp, "</td>");
		ret += websWrite(wp, "</tr>");
		}
	}
	ret += websWrite(wp, "</table>");

	ret += websWrite(wp, "<p><table border=\"0\" cellpadding=\"0\" cellspacing=\"0\">");
	ret += websWrite(wp, "<tr>");
	ret += websWrite(wp, "<td width='310'></td>");
	ret += websWrite(wp, "<td>&nbsp;&nbsp;</td>");
	ret += websWrite(wp, "<td><input type='button' value='Update Icons' "
		"onClick=\"onsubmit_iconinfo()\"></td>");
	ret += websWrite(wp, "</tr>");
	ret += websWrite(wp, "</table>");

EJ_PRINT_ICONLIST_DONE:
	if (prebuild_icons) {
		free(prebuild_icons);
		prebuild_icons = NULL;
	}
	if (userdef_icons) {
		free(userdef_icons);
		userdef_icons = NULL;
	}
	return ret;
}
REG_EJ_HANDLER(print_iconlist);
/* ==================== PRINT & VALIDATE FUNCTIONS ========================= */

#endif /* __CONFIG_HSPOT__ */
