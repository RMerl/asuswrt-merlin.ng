/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2001-2002  Ricky Yuen <ryuen@qualcomm.com>
 *  Copyright (C) 2003-2011  Marcel Holtmann <marcel@holtmann.org>
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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "sdp.h"

#define SDP_ERROR_RSP                                  0x01
#define SDP_SERVICE_SEARCH_REQ                         0x02
#define SDP_SERVICE_SEARCH_RSP                         0x03
#define SDP_SERVICE_ATTR_REQ                           0x04
#define SDP_SERVICE_ATTR_RSP                           0x05
#define SDP_SERVICE_SEARCH_ATTR_REQ                    0x06
#define SDP_SERVICE_SEARCH_ATTR_RSP                    0x07

typedef struct {
	uint8_t  pid;
	uint16_t tid;
	uint16_t len;
} __attribute__ ((packed)) sdp_pdu_hdr;
#define SDP_PDU_HDR_SIZE 5

/* Data element type descriptor */
#define SDP_DE_NULL   0
#define SDP_DE_UINT   1
#define SDP_DE_INT    2
#define SDP_DE_UUID   3
#define SDP_DE_STRING 4
#define SDP_DE_BOOL   5
#define SDP_DE_SEQ    6
#define SDP_DE_ALT    7
#define SDP_DE_URL    8

/* Data element size index lookup table */
typedef struct {
	int addl_bits;
	int num_bytes;
} sdp_siz_idx_lookup_table_t;

static sdp_siz_idx_lookup_table_t sdp_siz_idx_lookup_table[] = {
	{ 0, 1  }, /* Size index = 0 */
	{ 0, 2  }, /*              1 */
	{ 0, 4  }, /*              2 */
	{ 0, 8  }, /*              3 */
	{ 0, 16 }, /*              4 */
	{ 1, 1  }, /*              5 */
	{ 1, 2  }, /*              6 */
	{ 1, 4  }, /*              7 */
};

/* UUID name lookup table */
typedef struct {
	int   uuid;
	char* name;
} sdp_uuid_nam_lookup_table_t;

static sdp_uuid_nam_lookup_table_t sdp_uuid_nam_lookup_table[] = {
	{ SDP_UUID_SDP,                      "SDP"          },
	{ SDP_UUID_UDP,                      "UDP"          },
	{ SDP_UUID_RFCOMM,                   "RFCOMM"       },
	{ SDP_UUID_TCP,                      "TCP"          },
	{ SDP_UUID_TCS_BIN,                  "TCS-BIN"      },
	{ SDP_UUID_TCS_AT,                   "TCS-AT"       },
	{ SDP_UUID_OBEX,                     "OBEX"         },
	{ SDP_UUID_IP,                       "IP"           },
	{ SDP_UUID_FTP,                      "FTP"          },
	{ SDP_UUID_HTTP,                     "HTTP"         },
	{ SDP_UUID_WSP,                      "WSP"          },
	{ SDP_UUID_L2CAP,                    "L2CAP"        },
	{ SDP_UUID_BNEP,                     "BNEP"         }, /* PAN */
	{ SDP_UUID_HIDP,                     "HIDP"         }, /* HID */
	{ SDP_UUID_AVCTP,                    "AVCTP"        }, /* AVCTP */
	{ SDP_UUID_AVDTP,                    "AVDTP"        }, /* AVDTP */
	{ SDP_UUID_CMTP,                     "CMTP"         }, /* CIP */
	{ SDP_UUID_UDI_C_PLANE,              "UDI_C-Plane"  }, /* UDI */
	{ SDP_UUID_SERVICE_DISCOVERY_SERVER, "SDServer"     },
	{ SDP_UUID_BROWSE_GROUP_DESCRIPTOR,  "BrwsGrpDesc"  },
	{ SDP_UUID_PUBLIC_BROWSE_GROUP,      "PubBrwsGrp"   },
	{ SDP_UUID_SERIAL_PORT,              "SP"           },
	{ SDP_UUID_LAN_ACCESS_PPP,           "LAN"          },
	{ SDP_UUID_DIALUP_NETWORKING,        "DUN"          },
	{ SDP_UUID_IR_MC_SYNC,               "IRMCSync"     },
	{ SDP_UUID_OBEX_OBJECT_PUSH,         "OBEXObjPush"  },
	{ SDP_UUID_OBEX_FILE_TRANSFER,       "OBEXObjTrnsf" },
	{ SDP_UUID_IR_MC_SYNC_COMMAND,       "IRMCSyncCmd"  },
	{ SDP_UUID_HEADSET,                  "Headset"      },
	{ SDP_UUID_CORDLESS_TELEPHONY,       "CordlessTel"  },
	{ SDP_UUID_AUDIO_SOURCE,             "AudioSource"  }, /* A2DP */
	{ SDP_UUID_AUDIO_SINK,               "AudioSink"    }, /* A2DP */
	{ SDP_UUID_AV_REMOTE_TARGET,         "AVRemTarget"  }, /* AVRCP */
	{ SDP_UUID_ADVANCED_AUDIO,           "AdvAudio"     }, /* A2DP */
	{ SDP_UUID_AV_REMOTE,                "AVRemote"     }, /* AVRCP */
	{ SDP_UUID_AV_REMOTE_CONTROLLER,     "AVRemCt"      }, /* AVRCP */
	{ SDP_UUID_INTERCOM,                 "Intercom"     },
	{ SDP_UUID_FAX,                      "Fax"          },
	{ SDP_UUID_HEADSET_AUDIO_GATEWAY,    "Headset AG"   },
	{ SDP_UUID_WAP,                      "WAP"          },
	{ SDP_UUID_WAP_CLIENT,               "WAP Client"   },
	{ SDP_UUID_PANU,                     "PANU"         }, /* PAN */
	{ SDP_UUID_NAP,                      "NAP"          }, /* PAN */
	{ SDP_UUID_GN,                       "GN"           }, /* PAN */
	{ SDP_UUID_DIRECT_PRINTING,          "DirectPrint"  }, /* BPP */
	{ SDP_UUID_REFERENCE_PRINTING,       "RefPrint"     }, /* BPP */
	{ SDP_UUID_IMAGING,                  "Imaging"      }, /* BIP */
	{ SDP_UUID_IMAGING_RESPONDER,        "ImagingResp"  }, /* BIP */
	{ SDP_UUID_HANDSFREE,                "Handsfree"    },
	{ SDP_UUID_HANDSFREE_AUDIO_GATEWAY,  "Handsfree AG" },
	{ SDP_UUID_DIRECT_PRINTING_REF_OBJS, "RefObjsPrint" }, /* BPP */
	{ SDP_UUID_REFLECTED_UI,             "ReflectedUI"  }, /* BPP */
	{ SDP_UUID_BASIC_PRINTING,           "BasicPrint"   }, /* BPP */
	{ SDP_UUID_PRINTING_STATUS,          "PrintStatus"  }, /* BPP */
	{ SDP_UUID_HUMAN_INTERFACE_DEVICE,   "HID"          }, /* HID */
	{ SDP_UUID_HARDCOPY_CABLE_REPLACE,   "HCRP"         }, /* HCRP */
	{ SDP_UUID_HCR_PRINT,                "HCRPrint"     }, /* HCRP */
	{ SDP_UUID_HCR_SCAN,                 "HCRScan"      }, /* HCRP */
	{ SDP_UUID_COMMON_ISDN_ACCESS,       "CIP"          }, /* CIP */
	{ SDP_UUID_UDI_MT,                   "UDI MT"       }, /* UDI */
	{ SDP_UUID_UDI_TA,                   "UDI TA"       }, /* UDI */
	{ SDP_UUID_AUDIO_VIDEO,              "AudioVideo"   }, /* VCP */
	{ SDP_UUID_SIM_ACCESS,               "SAP"          }, /* SAP */
	{ SDP_UUID_PHONEBOOK_ACCESS_PCE,     "PBAP PCE"     }, /* PBAP */
	{ SDP_UUID_PHONEBOOK_ACCESS_PSE,     "PBAP PSE"     }, /* PBAP */
	{ SDP_UUID_PHONEBOOK_ACCESS,         "PBAP"         }, /* PBAP */
	{ SDP_UUID_PNP_INFORMATION,          "PNPInfo"      },
	{ SDP_UUID_GENERIC_NETWORKING,       "Networking"   },
	{ SDP_UUID_GENERIC_FILE_TRANSFER,    "FileTrnsf"    },
	{ SDP_UUID_GENERIC_AUDIO,            "Audio"        },
	{ SDP_UUID_GENERIC_TELEPHONY,        "Telephony"    },
	{ SDP_UUID_UPNP_SERVICE,             "UPNP"         }, /* ESDP */
	{ SDP_UUID_UPNP_IP_SERVICE,          "UPNP IP"      }, /* ESDP */
	{ SDP_UUID_ESDP_UPNP_IP_PAN,         "UPNP PAN"     }, /* ESDP */
	{ SDP_UUID_ESDP_UPNP_IP_LAP,         "UPNP LAP"     }, /* ESDP */
	{ SDP_UUID_ESDP_UPNP_L2CAP,          "UPNP L2CAP"   }, /* ESDP */
	{ SDP_UUID_VIDEO_SOURCE,             "VideoSource"  }, /* VDP */
	{ SDP_UUID_VIDEO_SINK,               "VideoSink"    }, /* VDP */
	{ SDP_UUID_VIDEO_DISTRIBUTION,       "VideoDist"    }, /* VDP */
	{ SDP_UUID_APPLE_AGENT,              "AppleAgent"   },
};

#define SDP_UUID_NAM_LOOKUP_TABLE_SIZE \
	(sizeof(sdp_uuid_nam_lookup_table)/sizeof(sdp_uuid_nam_lookup_table_t))

/* AttrID name lookup table */
typedef struct {
	int   attr_id;
	char* name;
} sdp_attr_id_nam_lookup_table_t;

static sdp_attr_id_nam_lookup_table_t sdp_attr_id_nam_lookup_table[] = {
	{ SDP_ATTR_ID_SERVICE_RECORD_HANDLE,             "SrvRecHndl"         },
	{ SDP_ATTR_ID_SERVICE_CLASS_ID_LIST,             "SrvClassIDList"     },
	{ SDP_ATTR_ID_SERVICE_RECORD_STATE,              "SrvRecState"        },
	{ SDP_ATTR_ID_SERVICE_SERVICE_ID,                "SrvID"              },
	{ SDP_ATTR_ID_PROTOCOL_DESCRIPTOR_LIST,          "ProtocolDescList"   },
	{ SDP_ATTR_ID_BROWSE_GROUP_LIST,                 "BrwGrpList"         },
	{ SDP_ATTR_ID_LANGUAGE_BASE_ATTRIBUTE_ID_LIST,   "LangBaseAttrIDList" },
	{ SDP_ATTR_ID_SERVICE_INFO_TIME_TO_LIVE,         "SrvInfoTimeToLive"  },
	{ SDP_ATTR_ID_SERVICE_AVAILABILITY,              "SrvAvail"           },
	{ SDP_ATTR_ID_BLUETOOTH_PROFILE_DESCRIPTOR_LIST, "BTProfileDescList"  },
	{ SDP_ATTR_ID_DOCUMENTATION_URL,                 "DocURL"             },
	{ SDP_ATTR_ID_CLIENT_EXECUTABLE_URL,             "ClientExeURL"       },
	{ SDP_ATTR_ID_ICON_URL,                          "IconURL"            },
	{ SDP_ATTR_ID_ADDITIONAL_PROTOCOL_DESC_LISTS,    "AdditionalProtocolDescLists" },
	{ SDP_ATTR_ID_SERVICE_NAME,                      "SrvName"            },
	{ SDP_ATTR_ID_SERVICE_DESCRIPTION,               "SrvDesc"            },
	{ SDP_ATTR_ID_PROVIDER_NAME,                     "ProviderName"       },
	{ SDP_ATTR_ID_VERSION_NUMBER_LIST,               "VersionNumList"     },
	{ SDP_ATTR_ID_GROUP_ID,                          "GrpID"              },
	{ SDP_ATTR_ID_SERVICE_DATABASE_STATE,            "SrvDBState"         },
	{ SDP_ATTR_ID_SERVICE_VERSION,                   "SrvVersion"         },
	{ SDP_ATTR_ID_SECURITY_DESCRIPTION,              "SecurityDescription"}, /* PAN */
	{ SDP_ATTR_ID_SUPPORTED_DATA_STORES_LIST,        "SuppDataStoresList" }, /* Synchronization */
	{ SDP_ATTR_ID_SUPPORTED_FORMATS_LIST,            "SuppFormatsList"    }, /* OBEX Object Push */
	{ SDP_ATTR_ID_NET_ACCESS_TYPE,                   "NetAccessType"      }, /* PAN */
	{ SDP_ATTR_ID_MAX_NET_ACCESS_RATE,               "MaxNetAccessRate"   }, /* PAN */
	{ SDP_ATTR_ID_IPV4_SUBNET,                       "IPv4Subnet"         }, /* PAN */
	{ SDP_ATTR_ID_IPV6_SUBNET,                       "IPv6Subnet"         }, /* PAN */
	{ SDP_ATTR_ID_SUPPORTED_CAPABILITIES,            "SuppCapabilities"   }, /* Imaging */
	{ SDP_ATTR_ID_SUPPORTED_FEATURES,                "SuppFeatures"       }, /* Imaging and Hansfree */
	{ SDP_ATTR_ID_SUPPORTED_FUNCTIONS,               "SuppFunctions"      }, /* Imaging */
	{ SDP_ATTR_ID_TOTAL_IMAGING_DATA_CAPACITY,       "SuppTotalCapacity"  }, /* Imaging */
	{ SDP_ATTR_ID_SUPPORTED_REPOSITORIES,            "SuppRepositories"   }, /* PBAP */
};

#define SDP_ATTR_ID_NAM_LOOKUP_TABLE_SIZE \
	(sizeof(sdp_attr_id_nam_lookup_table)/sizeof(sdp_attr_id_nam_lookup_table_t))

char* get_uuid_name(int uuid)
{
	unsigned int i;

	for (i = 0; i < SDP_UUID_NAM_LOOKUP_TABLE_SIZE; i++) {
		if (sdp_uuid_nam_lookup_table[i].uuid == uuid)
			return sdp_uuid_nam_lookup_table[i].name;
	}

	return 0;
}

static inline char* get_attr_id_name(int attr_id)
{
	unsigned int i;

	for (i = 0; i < SDP_ATTR_ID_NAM_LOOKUP_TABLE_SIZE; i++)
		if (sdp_attr_id_nam_lookup_table[i].attr_id == attr_id)
			return sdp_attr_id_nam_lookup_table[i].name;
	return 0;
}

static inline uint8_t parse_de_hdr(struct frame *frm, int *n)
{
	uint8_t de_hdr = p_get_u8(frm);
	uint8_t de_type = de_hdr >> 3;
	uint8_t siz_idx = de_hdr & 0x07;

	/* Get the number of bytes */
	if (sdp_siz_idx_lookup_table[siz_idx].addl_bits) {
		switch(sdp_siz_idx_lookup_table[siz_idx].num_bytes) {
		case 1:
			*n = p_get_u8(frm); break;
		case 2:
			*n = p_get_u16(frm); break;
		case 4:
			*n = p_get_u32(frm); break;
		case 8:
			*n = p_get_u64(frm); break;
		}
	} else
		*n = sdp_siz_idx_lookup_table[siz_idx].num_bytes;

	return de_type;
}

static inline void print_int(uint8_t de_type, int level, int n, struct frame *frm, uint16_t *psm, uint8_t *channel)
{
	uint64_t val, val2;

	switch(de_type) {
	case SDP_DE_UINT:
		printf(" uint");
		break;
	case SDP_DE_INT:
		printf(" int");
		break;
	case SDP_DE_BOOL:
		printf(" bool");
		break;
	}

	switch(n) {
	case 1: /* 8-bit */
		val = p_get_u8(frm);
		if (channel && de_type == SDP_DE_UINT)
			if (*channel == 0)
				*channel = val;
		break;
	case 2: /* 16-bit */
		val = p_get_u16(frm);
		if (psm && de_type == SDP_DE_UINT)
			if (*psm == 0)
				*psm = val;
		break;
	case 4: /* 32-bit */
		val = p_get_u32(frm);
		break;
	case 8: /* 64-bit */
		val = p_get_u64(frm);
		break;
	case 16:/* 128-bit */
		p_get_u128(frm, &val, &val2);
		printf(" 0x%jx", val2);
		if (val < 0x1000000000000000LL)
			printf("0");
		printf("%jx", val);
		return;
	default: /* syntax error */
		printf(" err");
		frm->ptr += n;
		frm->len -= n;
		return;
	}

	printf(" 0x%jx", val);
}

static inline void print_uuid(int n, struct frame *frm, uint16_t *psm, uint8_t *channel)
{
	uint32_t uuid = 0;
	char* s;
	int i;

	switch(n) {
	case 2: /* 16-bit UUID */
		uuid = p_get_u16(frm);
		s = "uuid-16";
		break;
	case 4: /* 32_bit UUID */
		uuid = p_get_u32(frm);
		s = "uuid-32";
		break;
	case 16: /* 128-bit UUID */
		printf(" uuid-128 ");
		for (i = 0; i < 16; i++) {
			printf("%02x", ((unsigned char *) frm->ptr)[i]);
			if (i == 3 || i == 5 || i == 7 || i == 9)
				printf("-");
		}
		frm->ptr += 16;
		frm->len -= 16;
		return;
	default: /* syntax error */
		printf(" *err*");
		frm->ptr += n;
		frm->len -= n;
		return;
	}

	if (psm && *psm > 0 && *psm != 0xffff) {
		set_proto(frm->handle, *psm, 0, uuid);
		*psm = 0xffff;
	}

	if (channel && *channel > 0 && *channel != 0xff) {
		set_proto(frm->handle, *psm, *channel, uuid);
		*channel = 0xff;
	}

	printf(" %s 0x%04x", s, uuid);
	if ((s = get_uuid_name(uuid)))
		printf(" (%s)", s);
}

static inline void print_string(int n, struct frame *frm, const char *name)
{
	int i, hex = 0;

	for (i = 0; i < n; i++) {
		if (i == (n - 1) && ((char *) frm->ptr)[i] == '\0')
			break;

		if (!isprint(((char *) frm->ptr)[i])) {
			hex = 1;
			break;
		}
	}

	printf(" %s", name);
	if (hex) {
		for (i = 0; i < n; i++)
			printf(" %02x", ((unsigned char *) frm->ptr)[i]);
	} else {
		printf(" \"");
		for (i = 0; i < n; i++)
			printf("%c", ((char *) frm->ptr)[i]);
		printf("\"");
	}

	frm->ptr += n;
	frm->len -= n;
}

static inline void print_de(int, struct frame *frm, int *split, uint16_t *psm, uint8_t *channel);

static inline void print_des(uint8_t de_type, int level, int n, struct frame *frm, int *split, uint16_t *psm, uint8_t *channel)
{
	int len = frm->len;
	while (len - (int) frm->len < n && (int) frm->len > 0)
		print_de(level, frm, split, psm, channel);
}

static inline void print_de(int level, struct frame *frm, int *split, uint16_t *psm, uint8_t *channel)
{
	int n = 0;
	uint8_t de_type = parse_de_hdr(frm, &n);

	switch (de_type) {
	case SDP_DE_NULL:
		printf(" null");
		break;
	case SDP_DE_UINT:
	case SDP_DE_INT:
	case SDP_DE_BOOL:
		print_int(de_type, level, n, frm, psm, channel);
		break;
	case SDP_DE_UUID:
		if (split) {
			/* Split output by uuids.
			 * Used for printing Protocol Desc List */
			if (*split) {
				printf("\n");
				p_indent(level, NULL);
			}
			++*split;
		}
		print_uuid(n, frm, psm, channel);
		break;
	case SDP_DE_URL:
	case SDP_DE_STRING:
		print_string(n, frm, de_type == SDP_DE_URL? "url": "str");
		break;
	case SDP_DE_SEQ:
		printf(" <");
		print_des(de_type, level, n, frm, split, psm, channel);
		printf(" >");
		break;
	case SDP_DE_ALT:
		printf(" [");
		print_des(de_type, level, n, frm, split, psm, channel);
		printf(" ]");
		break;
	}
}

static inline void print_srv_srch_pat(int level, struct frame *frm)
{
	int len, n1 = 0, n2 = 0;

	p_indent(level, frm);
	printf("pat");

	if (parse_de_hdr(frm, &n1) == SDP_DE_SEQ) {
		len = frm->len;
		while (len - (int) frm->len < n1 && (int) frm->len > 0) {
			if (parse_de_hdr(frm, &n2) == SDP_DE_UUID) {
				print_uuid(n2, frm, NULL, NULL);
			} else {
				printf("\nERROR: Unexpected syntax (UUID)\n");
				raw_dump(level, frm);
			}
		}
		printf("\n");
	} else {
		printf("\nERROR: Unexpected syntax (SEQ)\n");
		raw_dump(level, frm);
	}
}

static inline void print_attr_id_list(int level, struct frame *frm)
{
	uint16_t attr_id;
	uint32_t attr_id_range;
	int len, n1 = 0, n2 = 0;

	p_indent(level, frm);
	printf("aid(s)");

	if (parse_de_hdr(frm, &n1) == SDP_DE_SEQ) {
		len = frm->len;
		while (len - (int) frm->len < n1 && (int) frm->len > 0) {
			/* Print AttributeID */
			if (parse_de_hdr(frm, &n2) == SDP_DE_UINT) {
				char *name;
				switch(n2) {
				case 2:
					attr_id = p_get_u16(frm);
					name = get_attr_id_name(attr_id);
					if (!name)
						name = "unknown";
					printf(" 0x%04x (%s)", attr_id, name);
					break;
				case 4:
					attr_id_range = p_get_u32(frm);
					printf(" 0x%04x - 0x%04x",
							(attr_id_range >> 16),
							(attr_id_range & 0xFFFF));
					break;
				}
			} else {
				printf("\nERROR: Unexpected syntax\n");
				raw_dump(level, frm);
			}
		}
		printf("\n");
	} else {
		printf("\nERROR: Unexpected syntax\n");
		raw_dump(level, frm);
	}
}

static inline void print_attr_list(int level, struct frame *frm)
{
	uint16_t attr_id, psm;
	uint8_t channel;
	int len, split, n1 = 0, n2 = 0;

	if (parse_de_hdr(frm, &n1) == SDP_DE_SEQ) {
		len = frm->len;
		while (len - (int) frm->len < n1 && (int) frm->len > 0) {
			/* Print AttributeID */
			if (parse_de_hdr(frm, &n2) == SDP_DE_UINT && n2 == sizeof(attr_id)) {
				char *name;
				attr_id = p_get_u16(frm);
				p_indent(level, 0);
				name = get_attr_id_name(attr_id);
				if (!name)
					name = "unknown";
				printf("aid 0x%04x (%s)\n", attr_id, name);
				split = (attr_id != SDP_ATTR_ID_PROTOCOL_DESCRIPTOR_LIST);
				psm = 0;
				channel = 0;

				/* Print AttributeValue */
				p_indent(level + 1, 0);
				print_de(level + 1, frm, split ? NULL: &split,
					attr_id == SDP_ATTR_ID_PROTOCOL_DESCRIPTOR_LIST ? &psm : NULL,
					attr_id == SDP_ATTR_ID_PROTOCOL_DESCRIPTOR_LIST ? &channel : NULL);
				printf("\n");
			} else {
				printf("\nERROR: Unexpected syntax\n");
				raw_dump(level, frm);
				break;
			}
		}
	} else {
		printf("\nERROR: Unexpected syntax\n");
		raw_dump(level, frm);
	}
}

static inline void print_attr_lists(int level, struct frame *frm)
{
	int n = 0, cnt = 0;
	int count = frm->len;

	if (parse_de_hdr(frm, &n) == SDP_DE_SEQ) {
		while (count - (int) frm->len < n && (int) frm->len > 0) {
			p_indent(level, 0);
			printf("record #%d\n", cnt++);
			print_attr_list(level + 2, frm);
		}
	} else {
		printf("\nERROR: Unexpected syntax\n");
		raw_dump(level, frm);
	}
}

static inline void print_cont_state(int level, unsigned char *buf)
{
	uint8_t cont = buf[0];
	int i;

	p_indent(level, 0);
	printf("cont");
	for (i = 0; i < cont + 1; i++)
		printf(" %2.2X", buf[i]);
	printf("\n");
}

static char *pid2str(uint8_t pid)
{
	switch (pid) {
	case SDP_ERROR_RSP:
		return "Error Rsp";
	case SDP_SERVICE_SEARCH_REQ:
		return "SS Req";
	case SDP_SERVICE_SEARCH_RSP:
		return "SS Rsp";
	case SDP_SERVICE_ATTR_REQ:
		return "SA Req";
	case SDP_SERVICE_ATTR_RSP:
		return "SA Rsp";
	case SDP_SERVICE_SEARCH_ATTR_REQ:
		return "SSA Req";
	case SDP_SERVICE_SEARCH_ATTR_RSP:
		return "SSA Rsp";
	default:
		return "Unknown";
	}
}

#define FRAME_TABLE_SIZE 10

static struct frame frame_table[FRAME_TABLE_SIZE];

static int frame_add(struct frame *frm, int count)
{
	register struct frame *fr;
	register unsigned char *data;
	register int i, len = 0, pos = -1;

	for (i = 0; i < FRAME_TABLE_SIZE; i++) {
		if (frame_table[i].handle == frm->handle &&
				frame_table[i].cid == frm->cid) {
			pos = i;
			len = frame_table[i].data_len;
			break;
		}
		if (pos < 0 && !frame_table[i].handle)
			pos = i;
	}

	if (pos < 0 || count <= 0)
		return -EIO;

	data = malloc(len + count);
	if (!data)
		return -ENOMEM;

	fr = &frame_table[pos];

	if (len > 0) {
		memcpy(data, fr->data, len);
		memcpy(data + len, frm->ptr, count);
	} else
		memcpy(data, frm->ptr, count);

	if (fr->data)
		free(fr->data);

	fr->data       = data;
	fr->data_len   = len + count;
	fr->len        = fr->data_len;
	fr->ptr        = fr->data;
	fr->dev_id     = frm->dev_id;
	fr->in         = frm->in;
	fr->ts         = frm->ts;
	fr->handle     = frm->handle;
	fr->cid        = frm->cid;
	fr->num        = frm->num;
	fr->channel    = frm->channel;
	fr->pppdump_fd = frm->pppdump_fd;
	fr->audio_fd   = frm->audio_fd;

	return pos;
}

static struct frame *frame_get(struct frame *frm, int count)
{
	register int pos;

	pos = frame_add(frm, count);
	if (pos < 0)
		return frm;

	frame_table[pos].handle = 0;

	return &frame_table[pos];
}

void sdp_dump(int level, struct frame *frm)
{
	sdp_pdu_hdr *hdr = frm->ptr;
	uint16_t tid = ntohs(hdr->tid);
	uint16_t len = ntohs(hdr->len);
	uint16_t total, count;
	uint8_t cont;

	frm->ptr += SDP_PDU_HDR_SIZE;
	frm->len -= SDP_PDU_HDR_SIZE;

	p_indent(level, frm);
	printf("SDP %s: tid 0x%x len 0x%x\n", pid2str(hdr->pid), tid, len);

	switch (hdr->pid) {
	case SDP_ERROR_RSP:
		p_indent(level + 1, frm);
		printf("code 0x%x info ", p_get_u16(frm));
		if (frm->len > 0)
			hex_dump(0, frm, frm->len);
		else
			printf("none\n");
		break;

	case SDP_SERVICE_SEARCH_REQ:
		/* Parse ServiceSearchPattern */
		print_srv_srch_pat(level + 1, frm);

		/* Parse MaximumServiceRecordCount */
		p_indent(level + 1, frm);
		printf("max %d\n", p_get_u16(frm));

		/* Parse ContinuationState */
		print_cont_state(level + 1, frm->ptr);
		break;

	case SDP_SERVICE_SEARCH_RSP:
		/* Parse TotalServiceRecordCount */
		total = p_get_u16(frm);

		/* Parse CurrentServiceRecordCount */
		count = p_get_u16(frm);
		p_indent(level + 1, frm);
		if (count < total)
			printf("count %d of %d\n", count, total);
		else
			printf("count %d\n", count);

		/* Parse service record handle(s) */
		if (count > 0) {
			int i;
			p_indent(level + 1, frm);
			printf("handle%s", count > 1 ? "s" : "");
			for (i = 0; i < count; i++)
				printf(" 0x%x", p_get_u32(frm));
			printf("\n");
		}

		/* Parse ContinuationState */
		print_cont_state(level + 1, frm->ptr);
		break;

	case SDP_SERVICE_ATTR_REQ:
		/* Parse ServiceRecordHandle */
		p_indent(level + 1, frm);
		printf("handle 0x%x\n", p_get_u32(frm));

		/* Parse MaximumAttributeByteCount */
		p_indent(level + 1, frm);
		printf("max %d\n", p_get_u16(frm));

		/* Parse ServiceSearchPattern */
		print_attr_id_list(level + 1, frm);

		/* Parse ContinuationState */
		print_cont_state(level + 1, frm->ptr);
		break;

	case SDP_SERVICE_ATTR_RSP:
		/* Parse AttributeByteCount */
		count = p_get_u16(frm);
		p_indent(level + 1, frm);
		printf("count %d\n", count);

		/* Parse ContinuationState */
		cont = *(unsigned char *)(frm->ptr + count);

		if (cont == 0) {
			/* Parse AttributeList */
			print_attr_list(level + 1, frame_get(frm, count));
		} else
			frame_add(frm, count);

		print_cont_state(level + 1, frm->ptr + count);
		break;

	case SDP_SERVICE_SEARCH_ATTR_REQ:
		/* Parse ServiceSearchPattern */
		print_srv_srch_pat(level + 1, frm);

		/* Parse MaximumAttributeByteCount */
		p_indent(level + 1, frm);
		printf("max %d\n", p_get_u16(frm));

		/* Parse AttributeList */
		print_attr_id_list(level + 1, frm);

		/* Parse ContinuationState */
		print_cont_state(level + 1, frm->ptr);
		break;

	case SDP_SERVICE_SEARCH_ATTR_RSP:
		/* Parse AttributeByteCount */
		count = p_get_u16(frm);
		p_indent(level + 1, frm);
		printf("count %d\n", count);

		/* Parse ContinuationState */
		cont = *(unsigned char *)(frm->ptr + count);

		if (cont == 0) {
			/* Parse AttributeLists */
			print_attr_lists(level + 1, frame_get(frm, count));
		} else
			frame_add(frm, count);

		print_cont_state(level + 1, frm->ptr + count);
		break;

	default:
		raw_dump(level + 1, frm);
		break;
	}
}
