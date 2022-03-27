/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2001-2002  Nokia Corporation
 *  Copyright (C) 2002-2003  Maxim Krasnyansky <maxk@qualcomm.com>
 *  Copyright (C) 2002-2010  Marcel Holtmann <marcel@holtmann.org>
 *  Copyright (C) 2002-2003  Stephen Crane <steve.crane@rococosoft.com>
 *  Copyright (C) 2002-2003  Jean Tourrilhes <jt@hpl.hp.com>
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
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"
#include "lib/hci_lib.h"
#include "lib/sdp.h"
#include "lib/sdp_lib.h"

#include "src/sdp-xml.h"

#ifndef APPLE_AGENT_SVCLASS_ID
#define APPLE_AGENT_SVCLASS_ID 0x2112
#endif

#define for_each_opt(opt, long, short) while ((opt=getopt_long(argc, argv, short ? short:"+", long, 0)) != -1)
#define N_ELEMENTS(x) (sizeof(x) / sizeof((x)[0]))

/*
 * Convert a string to a BDADDR, with a few "enhancements" - Jean II
 */
static int estr2ba(char *str, bdaddr_t *ba)
{
	/* Only trap "local", "any" is already dealt with */
	if(!strcmp(str, "local")) {
		bacpy(ba, BDADDR_LOCAL);
		return 0;
	}
	return str2ba(str, ba);
}

#define DEFAULT_VIEW	0	/* Display only known attribute */
#define TREE_VIEW	1	/* Display full attribute tree */
#define RAW_VIEW	2	/* Display raw tree */
#define XML_VIEW	3	/* Display xml tree */

/* Pass args to the inquiry/search handler */
struct search_context {
	char		*svc;		/* Service */
	uuid_t		group;		/* Browse group */
	int		view;		/* View mode */
	uint32_t	handle;		/* Service record handle */
};

typedef int (*handler_t)(bdaddr_t *bdaddr, struct search_context *arg);

static char UUID_str[MAX_LEN_UUID_STR];
static bdaddr_t interface;

/* Definition of attribute members */
struct member_def {
	char *name;
};

/* Definition of an attribute */
struct attrib_def {
	int			num;		/* Numeric ID - 16 bits */
	char			*name;		/* User readable name */
	struct member_def	*members;	/* Definition of attribute args */
	int			member_max;	/* Max of attribute arg definitions */
};

/* Definition of a service or protocol */
struct uuid_def {
	int			num;		/* Numeric ID - 16 bits */
	char			*name;		/* User readable name */
	struct attrib_def	*attribs;	/* Specific attribute definitions */
	int			attrib_max;	/* Max of attribute definitions */
};

/* Context information about current attribute */
struct attrib_context {
	struct uuid_def		*service;	/* Service UUID, if known */
	struct attrib_def	*attrib;	/* Description of the attribute */
	int			member_index;	/* Index of current attribute member */
};

/* Context information about the whole service */
struct service_context {
	struct uuid_def		*service;	/* Service UUID, if known */
};

/* Allow us to do nice formatting of the lists */
static char *indent_spaces = "                                         ";

/* ID of the service attribute.
 * Most attributes after 0x200 are defined based on the service, so
 * we need to find what is the service (which is messy) - Jean II */
#define SERVICE_ATTR	0x1

/* Definition of the optional arguments in protocol list */
static struct member_def protocol_members[] = {
	{ "Protocol"		},
	{ "Channel/Port"	},
	{ "Version"		},
};

/* Definition of the optional arguments in profile list */
static struct member_def profile_members[] = {
	{ "Profile"	},
	{ "Version"	},
};

/* Definition of the optional arguments in Language list */
static struct member_def language_members[] = {
	{ "Code ISO639"		},
	{ "Encoding"		},
	{ "Base Offset"		},
};

/* Name of the various common attributes. See BT assigned numbers */
static struct attrib_def attrib_names[] = {
	{ 0x0, "ServiceRecordHandle", NULL, 0 },
	{ 0x1, "ServiceClassIDList", NULL, 0 },
	{ 0x2, "ServiceRecordState", NULL, 0 },
	{ 0x3, "ServiceID", NULL, 0 },
	{ 0x4, "ProtocolDescriptorList",
		protocol_members, N_ELEMENTS(protocol_members) },
	{ 0x5, "BrowseGroupList", NULL, 0 },
	{ 0x6, "LanguageBaseAttributeIDList",
		language_members, N_ELEMENTS(language_members) },
	{ 0x7, "ServiceInfoTimeToLive", NULL, 0 },
	{ 0x8, "ServiceAvailability", NULL, 0 },
	{ 0x9, "BluetoothProfileDescriptorList",
		profile_members, N_ELEMENTS(profile_members) },
	{ 0xA, "DocumentationURL", NULL, 0 },
	{ 0xB, "ClientExecutableURL", NULL, 0 },
	{ 0xC, "IconURL", NULL, 0 },
	{ 0xD, "AdditionalProtocolDescriptorLists", NULL, 0 },
	/* Definitions after that are tricky (per profile or offset) */
};

const int attrib_max = N_ELEMENTS(attrib_names);

/* Name of the various SPD attributes. See BT assigned numbers */
static struct attrib_def sdp_attrib_names[] = {
	{ 0x200, "VersionNumberList", NULL, 0 },
	{ 0x201, "ServiceDatabaseState", NULL, 0 },
};

/* Name of the various SPD attributes. See BT assigned numbers */
static struct attrib_def browse_attrib_names[] = {
	{ 0x200, "GroupID", NULL, 0 },
};

/* Name of the various Device ID attributes. See Device Id spec. */
static struct attrib_def did_attrib_names[] = {
	{ 0x200, "SpecificationID", NULL, 0 },
	{ 0x201, "VendorID", NULL, 0 },
	{ 0x202, "ProductID", NULL, 0 },
	{ 0x203, "Version", NULL, 0 },
	{ 0x204, "PrimaryRecord", NULL, 0 },
	{ 0x205, "VendorIDSource", NULL, 0 },
};

/* Name of the various HID attributes. See HID spec. */
static struct attrib_def hid_attrib_names[] = {
	{ 0x200, "DeviceReleaseNum", NULL, 0 },
	{ 0x201, "ParserVersion", NULL, 0 },
	{ 0x202, "DeviceSubclass", NULL, 0 },
	{ 0x203, "CountryCode", NULL, 0 },
	{ 0x204, "VirtualCable", NULL, 0 },
	{ 0x205, "ReconnectInitiate", NULL, 0 },
	{ 0x206, "DescriptorList", NULL, 0 },
	{ 0x207, "LangIDBaseList", NULL, 0 },
	{ 0x208, "SDPDisable", NULL, 0 },
	{ 0x209, "BatteryPower", NULL, 0 },
	{ 0x20a, "RemoteWakeup", NULL, 0 },
	{ 0x20b, "ProfileVersion", NULL, 0 },
	{ 0x20c, "SupervisionTimeout", NULL, 0 },
	{ 0x20d, "NormallyConnectable", NULL, 0 },
	{ 0x20e, "BootDevice", NULL, 0 },
};

/* Name of the various PAN attributes. See BT assigned numbers */
/* Note : those need to be double checked - Jean II */
static struct attrib_def pan_attrib_names[] = {
	{ 0x200, "IpSubnet", NULL, 0 },		/* Obsolete ??? */
	{ 0x30A, "SecurityDescription", NULL, 0 },
	{ 0x30B, "NetAccessType", NULL, 0 },
	{ 0x30C, "MaxNetAccessrate", NULL, 0 },
	{ 0x30D, "IPv4Subnet", NULL, 0 },
	{ 0x30E, "IPv6Subnet", NULL, 0 },
};

/* Name of the various Generic-Audio attributes. See BT assigned numbers */
/* Note : totally untested - Jean II */
static struct attrib_def audio_attrib_names[] = {
	{ 0x302, "Remote audio volume control", NULL, 0 },
};

/* Name of the various IrMCSync attributes. See BT assigned numbers */
static struct attrib_def irmc_attrib_names[] = {
	{ 0x0301, "SupportedDataStoresList", NULL, 0 },
};

/* Name of the various GOEP attributes. See BT assigned numbers */
static struct attrib_def goep_attrib_names[] = {
	{ 0x200, "GoepL2capPsm", NULL, 0 },
};

/* Name of the various PBAP attributes. See BT assigned numbers */
static struct attrib_def pbap_attrib_names[] = {
	{ 0x0314, "SupportedRepositories", NULL, 0 },
	{ 0x0317, "PbapSupportedFeatures", NULL, 0 },
};

/* Name of the various MAS attributes. See BT assigned numbers */
static struct attrib_def mas_attrib_names[] = {
	{ 0x0315, "MASInstanceID", NULL, 0 },
	{ 0x0316, "SupportedMessageTypes", NULL, 0 },
	{ 0x0317, "MapSupportedFeatures", NULL, 0 },
};

/* Name of the various MNS attributes. See BT assigned numbers */
static struct attrib_def mns_attrib_names[] = {
	{ 0x0317, "MapSupportedFeatures", NULL, 0 },
};

/* Same for the UUIDs. See BT assigned numbers */
static struct uuid_def uuid16_names[] = {
	/* -- Protocols -- */
	{ 0x0001, "SDP", NULL, 0 },
	{ 0x0002, "UDP", NULL, 0 },
	{ 0x0003, "RFCOMM", NULL, 0 },
	{ 0x0004, "TCP", NULL, 0 },
	{ 0x0005, "TCS-BIN", NULL, 0 },
	{ 0x0006, "TCS-AT", NULL, 0 },
	{ 0x0008, "OBEX", NULL, 0 },
	{ 0x0009, "IP", NULL, 0 },
	{ 0x000a, "FTP", NULL, 0 },
	{ 0x000c, "HTTP", NULL, 0 },
	{ 0x000e, "WSP", NULL, 0 },
	{ 0x000f, "BNEP", NULL, 0 },
	{ 0x0010, "UPnP/ESDP", NULL, 0 },
	{ 0x0011, "HIDP", NULL, 0 },
	{ 0x0012, "HardcopyControlChannel", NULL, 0 },
	{ 0x0014, "HardcopyDataChannel", NULL, 0 },
	{ 0x0016, "HardcopyNotification", NULL, 0 },
	{ 0x0017, "AVCTP", NULL, 0 },
	{ 0x0019, "AVDTP", NULL, 0 },
	{ 0x001b, "CMTP", NULL, 0 },
	{ 0x001d, "UDI_C-Plane", NULL, 0 },
	{ 0x0100, "L2CAP", NULL, 0 },
	/* -- Services -- */
	{ 0x1000, "ServiceDiscoveryServerServiceClassID",
		sdp_attrib_names, N_ELEMENTS(sdp_attrib_names) },
	{ 0x1001, "BrowseGroupDescriptorServiceClassID",
		browse_attrib_names, N_ELEMENTS(browse_attrib_names) },
	{ 0x1002, "PublicBrowseGroup", NULL, 0 },
	{ 0x1101, "SerialPort", NULL, 0 },
	{ 0x1102, "LANAccessUsingPPP", NULL, 0 },
	{ 0x1103, "DialupNetworking (DUN)", NULL, 0 },
	{ 0x1104, "IrMCSync",
		irmc_attrib_names, N_ELEMENTS(irmc_attrib_names) },
	{ 0x1105, "OBEXObjectPush",
		goep_attrib_names, N_ELEMENTS(goep_attrib_names) },
	{ 0x1106, "OBEXFileTransfer",
		goep_attrib_names, N_ELEMENTS(goep_attrib_names) },
	{ 0x1107, "IrMCSyncCommand", NULL, 0 },
	{ 0x1108, "Headset",
		audio_attrib_names, N_ELEMENTS(audio_attrib_names) },
	{ 0x1109, "CordlessTelephony", NULL, 0 },
	{ 0x110a, "AudioSource", NULL, 0 },
	{ 0x110b, "AudioSink", NULL, 0 },
	{ 0x110c, "RemoteControlTarget", NULL, 0 },
	{ 0x110d, "AdvancedAudio", NULL, 0 },
	{ 0x110e, "RemoteControl", NULL, 0 },
	{ 0x110f, "RemoteControlController", NULL, 0 },
	{ 0x1110, "Intercom", NULL, 0 },
	{ 0x1111, "Fax", NULL, 0 },
	{ 0x1112, "HeadsetAudioGateway", NULL, 0 },
	{ 0x1113, "WAP", NULL, 0 },
	{ 0x1114, "WAP Client", NULL, 0 },
	{ 0x1115, "PANU (PAN/BNEP)",
		pan_attrib_names, N_ELEMENTS(pan_attrib_names) },
	{ 0x1116, "NAP (PAN/BNEP)",
		pan_attrib_names, N_ELEMENTS(pan_attrib_names) },
	{ 0x1117, "GN (PAN/BNEP)",
		pan_attrib_names, N_ELEMENTS(pan_attrib_names) },
	{ 0x1118, "DirectPrinting (BPP)", NULL, 0 },
	{ 0x1119, "ReferencePrinting (BPP)", NULL, 0 },
	{ 0x111a, "Imaging (BIP)", NULL, 0 },
	{ 0x111b, "ImagingResponder (BIP)", NULL, 0 },
	{ 0x111c, "ImagingAutomaticArchive (BIP)", NULL, 0 },
	{ 0x111d, "ImagingReferencedObjects (BIP)", NULL, 0 },
	{ 0x111e, "Handsfree", NULL, 0 },
	{ 0x111f, "HandsfreeAudioGateway", NULL, 0 },
	{ 0x1120, "DirectPrintingReferenceObjectsService (BPP)", NULL, 0 },
	{ 0x1121, "ReflectedUI (BPP)", NULL, 0 },
	{ 0x1122, "BasicPrinting (BPP)", NULL, 0 },
	{ 0x1123, "PrintingStatus (BPP)", NULL, 0 },
	{ 0x1124, "HumanInterfaceDeviceService (HID)",
		hid_attrib_names, N_ELEMENTS(hid_attrib_names) },
	{ 0x1125, "HardcopyCableReplacement (HCR)", NULL, 0 },
	{ 0x1126, "HCR_Print (HCR)", NULL, 0 },
	{ 0x1127, "HCR_Scan (HCR)", NULL, 0 },
	{ 0x1128, "Common ISDN Access (CIP)", NULL, 0 },
	{ 0x112a, "UDI-MT", NULL, 0 },
	{ 0x112b, "UDI-TA", NULL, 0 },
	{ 0x112c, "Audio/Video", NULL, 0 },
	{ 0x112d, "SIM Access (SAP)", NULL, 0 },
	{ 0x112e, "Phonebook Access (PBAP) - PCE", NULL, 0 },
	{ 0x112f, "Phonebook Access (PBAP) - PSE",
		pbap_attrib_names, N_ELEMENTS(pbap_attrib_names) },
	{ 0x1130, "Phonebook Access (PBAP)", NULL, 0 },
	{ 0x1131, "Headset (HSP)", NULL, 0 },
	{ 0x1132, "Message Access (MAP) - MAS",
		mas_attrib_names, N_ELEMENTS(mas_attrib_names) },
	{ 0x1133, "Message Access (MAP) - MNS",
		mns_attrib_names, N_ELEMENTS(mns_attrib_names) },
	{ 0x1134, "Message Access (MAP)", NULL, 0 },
	/* ... */
	{ 0x1200, "PnPInformation",
		did_attrib_names, N_ELEMENTS(did_attrib_names) },
	{ 0x1201, "GenericNetworking", NULL, 0 },
	{ 0x1202, "GenericFileTransfer", NULL, 0 },
	{ 0x1203, "GenericAudio",
		audio_attrib_names, N_ELEMENTS(audio_attrib_names) },
	{ 0x1204, "GenericTelephony", NULL, 0 },
	/* ... */
	{ 0x1303, "VideoSource", NULL, 0 },
	{ 0x1304, "VideoSink", NULL, 0 },
	{ 0x1305, "VideoDistribution", NULL, 0 },
	{ 0x1400, "HDP", NULL, 0 },
	{ 0x1401, "HDPSource", NULL, 0 },
	{ 0x1402, "HDPSink", NULL, 0 },
	{ 0x2112, "AppleAgent", NULL, 0 },
};

static const int uuid16_max = N_ELEMENTS(uuid16_names);

static void sdp_data_printf(sdp_data_t *, struct attrib_context *, int);

/*
 * Parse a UUID.
 * The BT assigned numbers only list UUID16, so I'm not sure the
 * other types will ever get used...
 */
static void sdp_uuid_printf(uuid_t *uuid, struct attrib_context *context, int indent)
{
	if (uuid) {
		if (uuid->type == SDP_UUID16) {
			uint16_t uuidNum = uuid->value.uuid16;
			struct uuid_def *uuidDef = NULL;
			int i;

			for (i = 0; i < uuid16_max; i++)
				if (uuid16_names[i].num == uuidNum) {
					uuidDef = &uuid16_names[i];
					break;
				}

			/* Check if it's the service attribute */
			if (context->attrib && context->attrib->num == SERVICE_ATTR) {
				/* We got the service ID !!! */
				context->service = uuidDef;
			}

			if (uuidDef)
				printf("%.*sUUID16 : 0x%.4x - %s\n",
					indent, indent_spaces, uuidNum, uuidDef->name);
			else
				printf("%.*sUUID16 : 0x%.4x\n",
					indent, indent_spaces, uuidNum);
		} else if (uuid->type == SDP_UUID32) {
			struct uuid_def *uuidDef = NULL;
			int i;

			if (!(uuid->value.uuid32 & 0xffff0000)) {
				uint16_t uuidNum = uuid->value.uuid32;
				for (i = 0; i < uuid16_max; i++)
					if (uuid16_names[i].num == uuidNum) {
						uuidDef = &uuid16_names[i];
						break;
					}
			}

			if (uuidDef)
				printf("%.*sUUID32 : 0x%.8x - %s\n",
					indent, indent_spaces, uuid->value.uuid32, uuidDef->name);
			else
				printf("%.*sUUID32 : 0x%.8x\n",
					indent, indent_spaces, uuid->value.uuid32);
		} else if (uuid->type == SDP_UUID128) {
			unsigned int data0;
			unsigned short data1;
			unsigned short data2;
			unsigned short data3;
			unsigned int data4;
			unsigned short data5;

			memcpy(&data0, &uuid->value.uuid128.data[0], 4);
			memcpy(&data1, &uuid->value.uuid128.data[4], 2);
			memcpy(&data2, &uuid->value.uuid128.data[6], 2);
			memcpy(&data3, &uuid->value.uuid128.data[8], 2);
			memcpy(&data4, &uuid->value.uuid128.data[10], 4);
			memcpy(&data5, &uuid->value.uuid128.data[14], 2);

			printf("%.*sUUID128 : 0x%.8x-%.4x-%.4x-%.4x-%.8x-%.4x\n",
				indent, indent_spaces,
				ntohl(data0), ntohs(data1), ntohs(data2),
				ntohs(data3), ntohl(data4), ntohs(data5));
		} else
			printf("%.*sEnum type of UUID not set\n",
				indent, indent_spaces);
	} else
		printf("%.*sNull passed to print UUID\n",
				indent, indent_spaces);
}

/*
 * Parse a sequence of data elements (i.e. a list)
 */
static void printf_dataseq(sdp_data_t * pData, struct attrib_context *context, int indent)
{
	sdp_data_t *sdpdata = NULL;

	sdpdata = pData;
	if (sdpdata) {
		context->member_index = 0;
		do {
			sdp_data_printf(sdpdata, context, indent + 2);
			sdpdata = sdpdata->next;
			context->member_index++;
		} while (sdpdata);
	} else {
		printf("%.*sBroken dataseq link\n", indent, indent_spaces);
	}
}

/*
 * Parse a single data element (either in the attribute or in a data
 * sequence).
 */
static void sdp_data_printf(sdp_data_t *sdpdata, struct attrib_context *context, int indent)
{
	char *member_name = NULL;

	/* Find member name. Almost black magic ;-) */
	if (context && context->attrib && context->attrib->members &&
			context->member_index < context->attrib->member_max) {
		member_name = context->attrib->members[context->member_index].name;
	}

	switch (sdpdata->dtd) {
	case SDP_DATA_NIL:
		printf("%.*sNil\n", indent, indent_spaces);
		break;
	case SDP_BOOL:
	case SDP_UINT8:
	case SDP_UINT16:
	case SDP_UINT32:
	case SDP_UINT64:
	case SDP_UINT128:
	case SDP_INT8:
	case SDP_INT16:
	case SDP_INT32:
	case SDP_INT64:
	case SDP_INT128:
		if (member_name) {
			printf("%.*s%s (Integer) : 0x%x\n",
				indent, indent_spaces, member_name, sdpdata->val.uint32);
		} else {
			printf("%.*sInteger : 0x%x\n", indent, indent_spaces,
				sdpdata->val.uint32);
		}
		break;

	case SDP_UUID16:
	case SDP_UUID32:
	case SDP_UUID128:
		//printf("%.*sUUID\n", indent, indent_spaces);
		sdp_uuid_printf(&sdpdata->val.uuid, context, indent);
		break;

	case SDP_TEXT_STR8:
	case SDP_TEXT_STR16:
	case SDP_TEXT_STR32:
		if (sdpdata->unitSize > (int) strlen(sdpdata->val.str)) {
			int i;
			printf("%.*sData :", indent, indent_spaces);
			for (i = 0; i < sdpdata->unitSize; i++)
				printf(" %02x", (unsigned char) sdpdata->val.str[i]);
			printf("\n");
		} else
			printf("%.*sText : \"%s\"\n", indent, indent_spaces, sdpdata->val.str);
		break;
	case SDP_URL_STR8:
	case SDP_URL_STR16:
	case SDP_URL_STR32:
		printf("%.*sURL : %s\n", indent, indent_spaces, sdpdata->val.str);
		break;

	case SDP_SEQ8:
	case SDP_SEQ16:
	case SDP_SEQ32:
		printf("%.*sData Sequence\n", indent, indent_spaces);
		printf_dataseq(sdpdata->val.dataseq, context, indent);
		break;

	case SDP_ALT8:
	case SDP_ALT16:
	case SDP_ALT32:
		printf("%.*sData Sequence Alternates\n", indent, indent_spaces);
		printf_dataseq(sdpdata->val.dataseq, context, indent);
		break;
	}
}

/*
 * Parse a single attribute.
 */
static void print_tree_attr_func(void *value, void *userData)
{
	sdp_data_t *sdpdata = value;
	uint16_t attrId;
	struct service_context *service = (struct service_context *) userData;
	struct attrib_context context;
	struct attrib_def *attrDef = NULL;
	int i;

	if (!sdpdata)
		return;

	attrId = sdpdata->attrId;
	/* Search amongst the generic attributes */
	for (i = 0; i < attrib_max; i++)
		if (attrib_names[i].num == attrId) {
			attrDef = &attrib_names[i];
			break;
		}
	/* Search amongst the specific attributes of this service */
	if ((attrDef == NULL) && (service->service != NULL) &&
				(service->service->attribs != NULL)) {
		struct attrib_def *svc_attribs = service->service->attribs;
		int		svc_attrib_max = service->service->attrib_max;
		for (i = 0; i < svc_attrib_max; i++)
			if (svc_attribs[i].num == attrId) {
				attrDef = &svc_attribs[i];
				break;
			}
	}

	if (attrDef)
		printf("Attribute Identifier : 0x%x - %s\n", attrId, attrDef->name);
	else
		printf("Attribute Identifier : 0x%x\n", attrId);
	/* Build context */
	context.service = service->service;
	context.attrib = attrDef;
	context.member_index = 0;
	/* Parse attribute members */
	sdp_data_printf(sdpdata, &context, 2);
	/* Update service */
	service->service = context.service;
}

/*
 * Main entry point of this library. Parse a SDP record.
 * We assume the record has already been read, parsed and cached
 * locally. Jean II
 */
static void print_tree_attr(sdp_record_t *rec)
{
	if (rec && rec->attrlist) {
		struct service_context service = { NULL };
		sdp_list_foreach(rec->attrlist, print_tree_attr_func, &service);
	}
}

static void print_raw_data(sdp_data_t *data, int indent)
{
	struct uuid_def *def;
	int i, hex;

	if (!data)
		return;

	for (i = 0; i < indent; i++)
		printf("\t");

	switch (data->dtd) {
	case SDP_DATA_NIL:
		printf("NIL\n");
		break;
	case SDP_BOOL:
		printf("Bool %s\n", data->val.uint8 ? "True" : "False");
		break;
	case SDP_UINT8:
		printf("UINT8 0x%02x\n", data->val.uint8);
		break;
	case SDP_UINT16:
		printf("UINT16 0x%04x\n", data->val.uint16);
		break;
	case SDP_UINT32:
		printf("UINT32 0x%08x\n", data->val.uint32);
		break;
	case SDP_UINT64:
		printf("UINT64 0x%016jx\n", data->val.uint64);
		break;
	case SDP_UINT128:
		printf("UINT128 ...\n");
		break;
	case SDP_INT8:
		printf("INT8 %d\n", data->val.int8);
		break;
	case SDP_INT16:
		printf("INT16 %d\n", data->val.int16);
		break;
	case SDP_INT32:
		printf("INT32 %d\n", data->val.int32);
		break;
	case SDP_INT64:
		printf("INT64 %jd\n", data->val.int64);
		break;
	case SDP_INT128:
		printf("INT128 ...\n");
		break;
	case SDP_UUID16:
	case SDP_UUID32:
	case SDP_UUID128:
		switch (data->val.uuid.type) {
		case SDP_UUID16:
			def = NULL;
			for (i = 0; i < uuid16_max; i++)
				if (uuid16_names[i].num == data->val.uuid.value.uuid16) {
					def = &uuid16_names[i];
					break;
				}
			if (def)
				printf("UUID16 0x%04x - %s\n", data->val.uuid.value.uuid16, def->name);
			else
				printf("UUID16 0x%04x\n", data->val.uuid.value.uuid16);
			break;
		case SDP_UUID32:
			def = NULL;
			if (!(data->val.uuid.value.uuid32 & 0xffff0000)) {
				uint16_t value = data->val.uuid.value.uuid32;
				for (i = 0; i < uuid16_max; i++)
					if (uuid16_names[i].num == value) {
						def = &uuid16_names[i];
						break;
					}
			}
			if (def)
				printf("UUID32 0x%08x - %s\n", data->val.uuid.value.uuid32, def->name);
			else
				printf("UUID32 0x%08x\n", data->val.uuid.value.uuid32);
			break;
		case SDP_UUID128:
			printf("UUID128 ");
			for (i = 0; i < 16; i++) {
				switch (i) {
				case 4:
				case 6:
				case 8:
				case 10:
					printf("-");
					break;
				}
				printf("%02x", (unsigned char ) data->val.uuid.value.uuid128.data[i]);
			}
			printf("\n");
			break;
		default:
			printf("UUID type 0x%02x\n", data->val.uuid.type);
			break;
		}
		break;
	case SDP_TEXT_STR8:
	case SDP_TEXT_STR16:
	case SDP_TEXT_STR32:
		hex = 0;
		for (i = 0; i < data->unitSize; i++) {
			if (i == (data->unitSize - 1) && data->val.str[i] == '\0')
				break;
			if (!isprint(data->val.str[i])) {
				hex = 1;
				break;
			}
		}
		if (hex) {
			printf("Data");
			for (i = 0; i < data->unitSize; i++)
				printf(" %02x", (unsigned char) data->val.str[i]);
		} else {
			printf("String ");
			for (i = 0; i < data->unitSize; i++)
				printf("%c", data->val.str[i]);
		}
		printf("\n");
		break;
	case SDP_URL_STR8:
	case SDP_URL_STR16:
	case SDP_URL_STR32:
		printf("URL %s\n", data->val.str);
		break;
	case SDP_SEQ8:
	case SDP_SEQ16:
	case SDP_SEQ32:
		printf("Sequence\n");
		print_raw_data(data->val.dataseq, indent + 1);
		break;
	case SDP_ALT8:
	case SDP_ALT16:
	case SDP_ALT32:
		printf("Alternate\n");
		print_raw_data(data->val.dataseq, indent + 1);
		break;
	default:
		printf("Unknown type 0x%02x\n", data->dtd);
		break;
	}

	print_raw_data(data->next, indent);
}

static void print_raw_attr_func(void *value, void *userData)
{
	sdp_data_t *data = (sdp_data_t *) value;
	struct attrib_def *def = NULL;
	int i;

	if (!data)
		return;

	/* Search amongst the generic attributes */
	for (i = 0; i < attrib_max; i++)
		if (attrib_names[i].num == data->attrId) {
			def = &attrib_names[i];
			break;
		}

	if (def)
		printf("\tAttribute 0x%04x - %s\n", data->attrId, def->name);
	else
		printf("\tAttribute 0x%04x\n", data->attrId);

	print_raw_data(data, 2);
}

static void print_raw_attr(sdp_record_t *rec)
{
	if (rec && rec->attrlist) {
		printf("Sequence\n");
		sdp_list_foreach(rec->attrlist, print_raw_attr_func, 0);
	}
}

/*
 * Set attributes with single values in SDP record
 * Jean II
 */
static int set_attrib(sdp_session_t *sess, uint32_t handle, uint16_t attrib, char *value)
{
	sdp_list_t *attrid_list;
	uint32_t range = 0x0000ffff;
	sdp_record_t *rec;
	int ret;

	/* Get the old SDP record */
	attrid_list = sdp_list_append(NULL, &range);
	rec = sdp_service_attr_req(sess, handle, SDP_ATTR_REQ_RANGE, attrid_list);
	sdp_list_free(attrid_list, NULL);

	if (!rec) {
		printf("Service get request failed.\n");
		return -1;
	}

	/* Check the type of attribute */
	if (!strncasecmp(value, "u0x", 3)) {
		/* UUID16 */
		uint16_t value_int = 0;
		uuid_t value_uuid;
		value_int = strtoul(value + 3, NULL, 16);
		sdp_uuid16_create(&value_uuid, value_int);
		printf("Adding attrib 0x%X uuid16 0x%X to record 0x%X\n",
			attrib, value_int, handle);

		sdp_attr_add_new(rec, attrib, SDP_UUID16, &value_uuid.value.uuid16);
	} else if (!strncasecmp(value, "0x", 2)) {
		/* Int */
		uint32_t value_int;
		value_int = strtoul(value + 2, NULL, 16);
		printf("Adding attrib 0x%X int 0x%X to record 0x%X\n",
			attrib, value_int, handle);

		sdp_attr_add_new(rec, attrib, SDP_UINT32, &value_int);
	} else {
		/* String */
		printf("Adding attrib 0x%X string \"%s\" to record 0x%X\n",
			attrib, value, handle);

		/* Add/Update our attribute to the record */
		sdp_attr_add_new(rec, attrib, SDP_TEXT_STR8, value);
	}

	/* Update on the server */
	ret = sdp_device_record_update(sess, &interface, rec);
	if (ret < 0)
		printf("Service Record update failed (%d).\n", errno);
	sdp_record_free(rec);
	return ret;
}

static struct option set_options[] = {
	{ "help",	0, 0, 'h' },
	{ 0, 0, 0, 0 }
};

static const char *set_help =
	"Usage:\n"
	"\tget record_handle attrib_id attrib_value\n";

/*
 * Add an attribute to an existing SDP record on the local SDP server
 */
static int cmd_setattr(int argc, char **argv)
{
	int opt, status;
	uint32_t handle;
	uint16_t attrib;
	sdp_session_t *sess;

	for_each_opt(opt, set_options, NULL) {
		switch(opt) {
		default:
			printf("%s", set_help);
			return -1;
		}
	}

	argc -= optind;
	argv += optind;

	if (argc < 3) {
		printf("%s", set_help);
		return -1;
	}

	/* Convert command line args */
	handle = strtoul(argv[0], NULL, 16);
	attrib = strtoul(argv[1], NULL, 16);

	/* Do it */
	sess = sdp_connect(BDADDR_ANY, BDADDR_LOCAL, 0);
	if (!sess)
		return -1;

	status = set_attrib(sess, handle, attrib, argv[2]);
	sdp_close(sess);

	return status;
}

/*
 * We do only simple data sequences. Sequence of sequences is a pain ;-)
 * Jean II
 */
static int set_attribseq(sdp_session_t *session, uint32_t handle, uint16_t attrib, int argc, char **argv)
{
	sdp_list_t *attrid_list;
	uint32_t range = 0x0000ffff;
	sdp_record_t *rec;
	sdp_data_t *pSequenceHolder = NULL;
	void **dtdArray;
	void **valueArray;
	void **allocArray;
	uint8_t uuid16 = SDP_UUID16;
	uint8_t uint32 = SDP_UINT32;
	uint8_t str8 = SDP_TEXT_STR8;
	int i, ret = 0;

	/* Get the old SDP record */
	attrid_list = sdp_list_append(NULL, &range);
	rec = sdp_service_attr_req(session, handle, SDP_ATTR_REQ_RANGE, attrid_list);
	sdp_list_free(attrid_list, NULL);

	if (!rec) {
		printf("Service get request failed.\n");
		return -1;
	}

	/* Create arrays */
	dtdArray = malloc(argc * sizeof(void *));
	valueArray = malloc(argc * sizeof(void *));
	allocArray = malloc(argc * sizeof(void *));

	if (!dtdArray || !valueArray || !allocArray) {
		ret = -ENOMEM;
		goto cleanup;
	}

	/* Loop on all args, add them in arrays */
	for (i = 0; i < argc; i++) {
		/* Check the type of attribute */
		if (!strncasecmp(argv[i], "u0x", 3)) {
			/* UUID16 */
			uint16_t value_int = strtoul((argv[i]) + 3, NULL, 16);
			uuid_t *value_uuid = malloc(sizeof(uuid_t));
			if (!value_uuid) {
				ret = -ENOMEM;
				goto cleanup;
			}

			allocArray[i] = value_uuid;
			sdp_uuid16_create(value_uuid, value_int);

			printf("Adding uuid16 0x%X to record 0x%X\n", value_int, handle);
			dtdArray[i] = &uuid16;
			valueArray[i] = &value_uuid->value.uuid16;
		} else if (!strncasecmp(argv[i], "0x", 2)) {
			/* Int */
			uint32_t *value_int = malloc(sizeof(int));
			if (!value_int) {
				ret = -ENOMEM;
				goto cleanup;
			}

			allocArray[i] = value_int;
			*value_int = strtoul((argv[i]) + 2, NULL, 16);

			printf("Adding int 0x%X to record 0x%X\n", *value_int, handle);
			dtdArray[i] = &uint32;
			valueArray[i] = value_int;
		} else {
			/* String */
			printf("Adding string \"%s\" to record 0x%X\n", argv[i], handle);
			dtdArray[i] = &str8;
			valueArray[i] = argv[i];
		}
	}

	/* Add this sequence to the attrib list */
	pSequenceHolder = sdp_seq_alloc(dtdArray, valueArray, argc);
	if (pSequenceHolder) {
		sdp_attr_replace(rec, attrib, pSequenceHolder);

		/* Update on the server */
		ret = sdp_device_record_update(session, &interface, rec);
		if (ret < 0)
			printf("Service Record update failed (%d).\n", errno);
	} else
		printf("Failed to create pSequenceHolder\n");

cleanup:
	if (ret == -ENOMEM)
		printf("Memory allocation failed\n");

	/* Cleanup */
	for (i = 0; i < argc; i++)
		if (allocArray)
			free(allocArray[i]);

	free(dtdArray);
	free(valueArray);
	free(allocArray);

	sdp_record_free(rec);

	return ret;
}

static struct option seq_options[] = {
	{ "help",	0, 0, 'h' },
	{ 0, 0, 0, 0 }
};

static const char *seq_help =
	"Usage:\n"
	"\tget record_handle attrib_id attrib_values\n";

/*
 * Add an attribute sequence to an existing SDP record
 * on the local SDP server
 */
static int cmd_setseq(int argc, char **argv)
{
	int opt, status;
	uint32_t handle;
	uint16_t attrib;
	sdp_session_t *sess;

	for_each_opt(opt, seq_options, NULL) {
		switch(opt) {
		default:
			printf("%s", seq_help);
			return -1;
		}
	}

	argc -= optind;
	argv += optind;

	if (argc < 3) {
		printf("%s", seq_help);
		return -1;
	}

	/* Convert command line args */
	handle = strtoul(argv[0], NULL, 16);
	attrib = strtoul(argv[1], NULL, 16);

	argc -= 2;
	argv += 2;

	/* Do it */
	sess = sdp_connect(BDADDR_ANY, BDADDR_LOCAL, 0);
	if (!sess)
		return -1;

	status = set_attribseq(sess, handle, attrib, argc, argv);
	sdp_close(sess);

	return status;
}

static void print_service_class(void *value, void *userData)
{
	char ServiceClassUUID_str[MAX_LEN_SERVICECLASS_UUID_STR];
	uuid_t *uuid = (uuid_t *)value;

	sdp_uuid2strn(uuid, UUID_str, MAX_LEN_UUID_STR);
	sdp_svclass_uuid2strn(uuid, ServiceClassUUID_str, MAX_LEN_SERVICECLASS_UUID_STR);
	if (uuid->type != SDP_UUID128)
		printf("  \"%s\" (0x%s)\n", ServiceClassUUID_str, UUID_str);
	else
		printf("  UUID 128: %s\n", UUID_str);
}

static void print_service_desc(void *value, void *user)
{
	char str[MAX_LEN_PROTOCOL_UUID_STR];
	sdp_data_t *p = (sdp_data_t *)value, *s;
	int i = 0, proto = 0;

	for (; p; p = p->next, i++) {
		switch (p->dtd) {
		case SDP_UUID16:
		case SDP_UUID32:
		case SDP_UUID128:
			sdp_uuid2strn(&p->val.uuid, UUID_str, MAX_LEN_UUID_STR);
			sdp_proto_uuid2strn(&p->val.uuid, str, sizeof(str));
			proto = sdp_uuid_to_proto(&p->val.uuid);
			printf("  \"%s\" (0x%s)\n", str, UUID_str);
			break;
		case SDP_UINT8:
			if (proto == RFCOMM_UUID)
				printf("    Channel: %d\n", p->val.uint8);
			else
				printf("    uint8: 0x%02x\n", p->val.uint8);
			break;
		case SDP_UINT16:
			if (proto == L2CAP_UUID) {
				if (i == 1)
					printf("    PSM: %d\n", p->val.uint16);
				else
					printf("    Version: 0x%04x\n", p->val.uint16);
			} else if (proto == BNEP_UUID)
				if (i == 1)
					printf("    Version: 0x%04x\n", p->val.uint16);
				else
					printf("    uint16: 0x%04x\n", p->val.uint16);
			else
				printf("    uint16: 0x%04x\n", p->val.uint16);
			break;
		case SDP_SEQ16:
			printf("    SEQ16:");
			for (s = p->val.dataseq; s; s = s->next)
				printf(" %x", s->val.uint16);
			printf("\n");
			break;
		case SDP_SEQ8:
			printf("    SEQ8:");
			for (s = p->val.dataseq; s; s = s->next)
				printf(" %x", s->val.uint8);
			printf("\n");
			break;
		default:
			printf("    FIXME: dtd=0%x\n", p->dtd);
			break;
		}
	}
}

static void print_lang_attr(void *value, void *user)
{
	sdp_lang_attr_t *lang = (sdp_lang_attr_t *)value;
	printf("  code_ISO639: 0x%02x\n", lang->code_ISO639);
	printf("  encoding:    0x%02x\n", lang->encoding);
	printf("  base_offset: 0x%02x\n", lang->base_offset);
}

static void print_access_protos(void *value, void *userData)
{
	sdp_list_t *protDescSeq = (sdp_list_t *)value;
	sdp_list_foreach(protDescSeq, print_service_desc, 0);
}

static void print_profile_desc(void *value, void *userData)
{
	sdp_profile_desc_t *desc = (sdp_profile_desc_t *)value;
	char str[MAX_LEN_PROFILEDESCRIPTOR_UUID_STR];

	sdp_uuid2strn(&desc->uuid, UUID_str, MAX_LEN_UUID_STR);
	sdp_profile_uuid2strn(&desc->uuid, str, MAX_LEN_PROFILEDESCRIPTOR_UUID_STR);

	printf("  \"%s\" (0x%s)\n", str, UUID_str);
	if (desc->version)
		printf("    Version: 0x%04x\n", desc->version);
}

/*
 * Parse a SDP record in user friendly form.
 */
static void print_service_attr(sdp_record_t *rec)
{
	sdp_list_t *list = 0, *proto = 0;

	sdp_record_print(rec);

	printf("Service RecHandle: 0x%x\n", rec->handle);

	if (sdp_get_service_classes(rec, &list) == 0) {
		printf("Service Class ID List:\n");
		sdp_list_foreach(list, print_service_class, 0);
		sdp_list_free(list, free);
	}
	if (sdp_get_access_protos(rec, &proto) == 0) {
		printf("Protocol Descriptor List:\n");
		sdp_list_foreach(proto, print_access_protos, 0);
		sdp_list_foreach(proto, (sdp_list_func_t)sdp_list_free, 0);
		sdp_list_free(proto, 0);
	}
	if (sdp_get_lang_attr(rec, &list) == 0) {
		printf("Language Base Attr List:\n");
		sdp_list_foreach(list, print_lang_attr, 0);
		sdp_list_free(list, free);
	}
	if (sdp_get_profile_descs(rec, &list) == 0) {
		printf("Profile Descriptor List:\n");
		sdp_list_foreach(list, print_profile_desc, 0);
		sdp_list_free(list, free);
	}
}

/*
 * Support for Service (de)registration
 */
typedef struct {
	uint32_t handle;
	char *name;
	char *provider;
	char *desc;
	unsigned int class;
	unsigned int profile;
	uint16_t psm;
	uint8_t channel;
	uint8_t network;
} svc_info_t;

static int add_sp(sdp_session_t *session, svc_info_t *si)
{
	sdp_list_t *svclass_id, *apseq, *proto[2], *profiles, *root, *aproto;
	uuid_t root_uuid, sp_uuid, l2cap, rfcomm;
	sdp_profile_desc_t profile;
	sdp_record_t record;
	uint8_t u8 = si->channel ? si->channel : 1;
	sdp_data_t *channel;
	int ret = 0;

	memset(&record, 0, sizeof(sdp_record_t));
	record.handle = si->handle;
	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(0, &root_uuid);
	sdp_set_browse_groups(&record, root);

	sdp_uuid16_create(&sp_uuid, SERIAL_PORT_SVCLASS_ID);
	svclass_id = sdp_list_append(0, &sp_uuid);
	sdp_set_service_classes(&record, svclass_id);

	sdp_uuid16_create(&profile.uuid, SERIAL_PORT_PROFILE_ID);
	profile.version = 0x0100;
	profiles = sdp_list_append(0, &profile);
	sdp_set_profile_descs(&record, profiles);

	sdp_uuid16_create(&l2cap, L2CAP_UUID);
	proto[0] = sdp_list_append(0, &l2cap);
	apseq = sdp_list_append(0, proto[0]);

	sdp_uuid16_create(&rfcomm, RFCOMM_UUID);
	proto[1] = sdp_list_append(0, &rfcomm);
	channel = sdp_data_alloc(SDP_UINT8, &u8);
	proto[1] = sdp_list_append(proto[1], channel);
	apseq = sdp_list_append(apseq, proto[1]);

	aproto = sdp_list_append(0, apseq);
	sdp_set_access_protos(&record, aproto);

	sdp_add_lang_attr(&record);

	sdp_set_info_attr(&record, "Serial Port", "BlueZ", "COM Port");

	sdp_set_url_attr(&record, "http://www.bluez.org/",
			"http://www.bluez.org/", "http://www.bluez.org/");

	sdp_set_service_id(&record, sp_uuid);
	sdp_set_service_ttl(&record, 0xffff);
	sdp_set_service_avail(&record, 0xff);
	sdp_set_record_state(&record, 0x00001234);

	if (sdp_device_record_register(session, &interface, &record, SDP_RECORD_PERSIST) < 0) {
		printf("Service Record registration failed\n");
		ret = -1;
		goto end;
	}

	printf("Serial Port service registered\n");

end:
	sdp_data_free(channel);
	sdp_list_free(proto[0], 0);
	sdp_list_free(proto[1], 0);
	sdp_list_free(apseq, 0);
	sdp_list_free(aproto, 0);
	sdp_list_free(root, 0);
	sdp_list_free(svclass_id, 0);
	sdp_list_free(profiles, 0);

	return ret;
}

static int add_dun(sdp_session_t *session, svc_info_t *si)
{
	sdp_list_t *svclass_id, *pfseq, *apseq, *root, *aproto;
	uuid_t rootu, dun, gn, l2cap, rfcomm;
	sdp_profile_desc_t profile;
	sdp_list_t *proto[2];
	sdp_record_t record;
	uint8_t u8 = si->channel ? si->channel : 2;
	sdp_data_t *channel;
	int ret = 0;

	memset(&record, 0, sizeof(sdp_record_t));
	record.handle = si->handle;

	sdp_uuid16_create(&rootu, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(0, &rootu);
	sdp_set_browse_groups(&record, root);

	sdp_uuid16_create(&dun, DIALUP_NET_SVCLASS_ID);
	svclass_id = sdp_list_append(0, &dun);
	sdp_uuid16_create(&gn,  GENERIC_NETWORKING_SVCLASS_ID);
	svclass_id = sdp_list_append(svclass_id, &gn);
	sdp_set_service_classes(&record, svclass_id);

	sdp_uuid16_create(&profile.uuid, DIALUP_NET_PROFILE_ID);
	profile.version = 0x0100;
	pfseq = sdp_list_append(0, &profile);
	sdp_set_profile_descs(&record, pfseq);

	sdp_uuid16_create(&l2cap, L2CAP_UUID);
	proto[0] = sdp_list_append(0, &l2cap);
	apseq = sdp_list_append(0, proto[0]);

	sdp_uuid16_create(&rfcomm, RFCOMM_UUID);
	proto[1] = sdp_list_append(0, &rfcomm);
	channel = sdp_data_alloc(SDP_UINT8, &u8);
	proto[1] = sdp_list_append(proto[1], channel);
	apseq = sdp_list_append(apseq, proto[1]);

	aproto = sdp_list_append(0, apseq);
	sdp_set_access_protos(&record, aproto);

	sdp_set_info_attr(&record, "Dial-Up Networking", 0, 0);

	if (sdp_device_record_register(session, &interface, &record, SDP_RECORD_PERSIST) < 0) {
		printf("Service Record registration failed\n");
		ret = -1;
		goto end;
	}

	printf("Dial-Up Networking service registered\n");

end:
	sdp_data_free(channel);
	sdp_list_free(proto[0], 0);
	sdp_list_free(proto[1], 0);
	sdp_list_free(apseq, 0);
	sdp_list_free(aproto, 0);

	return ret;
}

static int add_fax(sdp_session_t *session, svc_info_t *si)
{
	sdp_list_t *svclass_id, *pfseq, *apseq, *root;
	uuid_t root_uuid, fax_uuid, tel_uuid, l2cap_uuid, rfcomm_uuid;
	sdp_profile_desc_t profile;
	sdp_list_t *aproto, *proto[2];
	sdp_record_t record;
	uint8_t u8 = si->channel? si->channel : 3;
	sdp_data_t *channel;
	int ret = 0;

	memset(&record, 0, sizeof(sdp_record_t));
	record.handle = si->handle;

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(0, &root_uuid);
	sdp_set_browse_groups(&record, root);

	sdp_uuid16_create(&fax_uuid, FAX_SVCLASS_ID);
	svclass_id = sdp_list_append(0, &fax_uuid);
	sdp_uuid16_create(&tel_uuid, GENERIC_TELEPHONY_SVCLASS_ID);
	svclass_id = sdp_list_append(svclass_id, &tel_uuid);
	sdp_set_service_classes(&record, svclass_id);

	sdp_uuid16_create(&profile.uuid, FAX_PROFILE_ID);
	profile.version = 0x0100;
	pfseq = sdp_list_append(0, &profile);
	sdp_set_profile_descs(&record, pfseq);

	sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
	proto[0] = sdp_list_append(0, &l2cap_uuid);
	apseq = sdp_list_append(0, proto[0]);

	sdp_uuid16_create(&rfcomm_uuid, RFCOMM_UUID);
	proto[1] = sdp_list_append(0, &rfcomm_uuid);
	channel = sdp_data_alloc(SDP_UINT8, &u8);
	proto[1] = sdp_list_append(proto[1], channel);
	apseq  = sdp_list_append(apseq, proto[1]);

	aproto = sdp_list_append(0, apseq);
	sdp_set_access_protos(&record, aproto);

	sdp_set_info_attr(&record, "Fax", 0, 0);

	if (sdp_device_record_register(session, &interface, &record, SDP_RECORD_PERSIST) < 0) {
		printf("Service Record registration failed\n");
		ret = -1;
		goto end;
	}
	printf("Fax service registered\n");
end:
	sdp_data_free(channel);
	sdp_list_free(proto[0], 0);
	sdp_list_free(proto[1], 0);
	sdp_list_free(apseq, 0);
	sdp_list_free(aproto, 0);
	return ret;
}

static int add_lan(sdp_session_t *session, svc_info_t *si)
{
	sdp_list_t *svclass_id, *pfseq, *apseq, *root;
	uuid_t root_uuid, svclass_uuid, l2cap_uuid, rfcomm_uuid;
	sdp_profile_desc_t profile;
	sdp_list_t *aproto, *proto[2];
	sdp_record_t record;
	uint8_t u8 = si->channel ? si->channel : 4;
	sdp_data_t *channel;
	int ret = 0;

	memset(&record, 0, sizeof(sdp_record_t));
	record.handle = si->handle;

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(0, &root_uuid);
	sdp_set_browse_groups(&record, root);

	sdp_uuid16_create(&svclass_uuid, LAN_ACCESS_SVCLASS_ID);
	svclass_id = sdp_list_append(0, &svclass_uuid);
	sdp_set_service_classes(&record, svclass_id);

	sdp_uuid16_create(&profile.uuid, LAN_ACCESS_PROFILE_ID);
	profile.version = 0x0100;
	pfseq = sdp_list_append(0, &profile);
	sdp_set_profile_descs(&record, pfseq);

	sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
	proto[0] = sdp_list_append(0, &l2cap_uuid);
	apseq = sdp_list_append(0, proto[0]);

	sdp_uuid16_create(&rfcomm_uuid, RFCOMM_UUID);
	proto[1] = sdp_list_append(0, &rfcomm_uuid);
	channel = sdp_data_alloc(SDP_UINT8, &u8);
	proto[1] = sdp_list_append(proto[1], channel);
	apseq = sdp_list_append(apseq, proto[1]);

	aproto = sdp_list_append(0, apseq);
	sdp_set_access_protos(&record, aproto);

	sdp_set_info_attr(&record, "LAN Access over PPP", 0, 0);

	if (sdp_device_record_register(session, &interface, &record, SDP_RECORD_PERSIST) < 0) {
		printf("Service Record registration failed\n");
		ret = -1;
		goto end;
	}

	printf("LAN Access service registered\n");

end:
	sdp_data_free(channel);
	sdp_list_free(proto[0], 0);
	sdp_list_free(proto[1], 0);
	sdp_list_free(apseq, 0);
	sdp_list_free(aproto, 0);

	return ret;
}

static int add_headset(sdp_session_t *session, svc_info_t *si)
{
	sdp_list_t *svclass_id, *pfseq, *apseq, *root;
	uuid_t root_uuid, svclass_uuid, ga_svclass_uuid, l2cap_uuid, rfcomm_uuid;
	sdp_profile_desc_t profile;
	sdp_list_t *aproto, *proto[2];
	sdp_record_t record;
	uint8_t u8 = si->channel ? si->channel : 5;
	sdp_data_t *channel;
	int ret = 0;

	memset(&record, 0, sizeof(sdp_record_t));
	record.handle = si->handle;

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(0, &root_uuid);
	sdp_set_browse_groups(&record, root);

	sdp_uuid16_create(&svclass_uuid, HEADSET_SVCLASS_ID);
	svclass_id = sdp_list_append(0, &svclass_uuid);
	sdp_uuid16_create(&ga_svclass_uuid, GENERIC_AUDIO_SVCLASS_ID);
	svclass_id = sdp_list_append(svclass_id, &ga_svclass_uuid);
	sdp_set_service_classes(&record, svclass_id);

	sdp_uuid16_create(&profile.uuid, HEADSET_PROFILE_ID);
	profile.version = 0x0100;
	pfseq = sdp_list_append(0, &profile);
	sdp_set_profile_descs(&record, pfseq);

	sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
	proto[0] = sdp_list_append(0, &l2cap_uuid);
	apseq = sdp_list_append(0, proto[0]);

	sdp_uuid16_create(&rfcomm_uuid, RFCOMM_UUID);
	proto[1] = sdp_list_append(0, &rfcomm_uuid);
	channel = sdp_data_alloc(SDP_UINT8, &u8);
	proto[1] = sdp_list_append(proto[1], channel);
	apseq = sdp_list_append(apseq, proto[1]);

	aproto = sdp_list_append(0, apseq);
	sdp_set_access_protos(&record, aproto);

	sdp_set_info_attr(&record, "Headset", 0, 0);

	if (sdp_device_record_register(session, &interface, &record, SDP_RECORD_PERSIST) < 0) {
		printf("Service Record registration failed\n");
		ret = -1;
		goto end;
	}

	printf("Headset service registered\n");

end:
	sdp_data_free(channel);
	sdp_list_free(proto[0], 0);
	sdp_list_free(proto[1], 0);
	sdp_list_free(apseq, 0);
	sdp_list_free(aproto, 0);

	return ret;
}

static int add_headset_ag(sdp_session_t *session, svc_info_t *si)
{
	sdp_list_t *svclass_id, *pfseq, *apseq, *root;
	uuid_t root_uuid, svclass_uuid, ga_svclass_uuid, l2cap_uuid, rfcomm_uuid;
	sdp_profile_desc_t profile;
	sdp_list_t *aproto, *proto[2];
	sdp_record_t record;
	uint8_t u8 = si->channel ? si->channel : 7;
	sdp_data_t *channel;
	uint8_t netid = si->network ? si->network : 0x01; // ???? profile document
	sdp_data_t *network = sdp_data_alloc(SDP_UINT8, &netid);
	int ret = 0;

	memset(&record, 0, sizeof(sdp_record_t));
	record.handle = si->handle;

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(0, &root_uuid);
	sdp_set_browse_groups(&record, root);

	sdp_uuid16_create(&svclass_uuid, HEADSET_AGW_SVCLASS_ID);
	svclass_id = sdp_list_append(0, &svclass_uuid);
	sdp_uuid16_create(&ga_svclass_uuid, GENERIC_AUDIO_SVCLASS_ID);
	svclass_id = sdp_list_append(svclass_id, &ga_svclass_uuid);
	sdp_set_service_classes(&record, svclass_id);

	sdp_uuid16_create(&profile.uuid, HEADSET_PROFILE_ID);
	profile.version = 0x0100;
	pfseq = sdp_list_append(0, &profile);
	sdp_set_profile_descs(&record, pfseq);

	sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
	proto[0] = sdp_list_append(0, &l2cap_uuid);
	apseq = sdp_list_append(0, proto[0]);

	sdp_uuid16_create(&rfcomm_uuid, RFCOMM_UUID);
	proto[1] = sdp_list_append(0, &rfcomm_uuid);
	channel = sdp_data_alloc(SDP_UINT8, &u8);
	proto[1] = sdp_list_append(proto[1], channel);
	apseq = sdp_list_append(apseq, proto[1]);

	aproto = sdp_list_append(0, apseq);
	sdp_set_access_protos(&record, aproto);

	sdp_set_info_attr(&record, "Voice Gateway", 0, 0);

	sdp_attr_add(&record, SDP_ATTR_EXTERNAL_NETWORK, network);

	if (sdp_record_register(session, &record, SDP_RECORD_PERSIST) < 0) {
		printf("Service Record registration failed\n");
		ret = -1;
		goto end;
	}

	printf("Headset AG service registered\n");

end:
	sdp_data_free(channel);
	sdp_list_free(proto[0], 0);
	sdp_list_free(proto[1], 0);
	sdp_list_free(apseq, 0);
	sdp_list_free(aproto, 0);

	return ret;
}

static int add_handsfree(sdp_session_t *session, svc_info_t *si)
{
	sdp_list_t *svclass_id, *pfseq, *apseq, *root;
	uuid_t root_uuid, svclass_uuid, ga_svclass_uuid, l2cap_uuid, rfcomm_uuid;
	sdp_profile_desc_t profile;
	sdp_list_t *aproto, *proto[2];
	sdp_record_t record;
	uint8_t u8 = si->channel ? si->channel : 6;
	uint16_t u16 = 0x31;
	sdp_data_t *channel, *features;
	int ret = 0;

	memset(&record, 0, sizeof(sdp_record_t));
	record.handle = si->handle;

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(0, &root_uuid);
	sdp_set_browse_groups(&record, root);

	sdp_uuid16_create(&svclass_uuid, HANDSFREE_SVCLASS_ID);
	svclass_id = sdp_list_append(0, &svclass_uuid);
	sdp_uuid16_create(&ga_svclass_uuid, GENERIC_AUDIO_SVCLASS_ID);
	svclass_id = sdp_list_append(svclass_id, &ga_svclass_uuid);
	sdp_set_service_classes(&record, svclass_id);

	sdp_uuid16_create(&profile.uuid, HANDSFREE_PROFILE_ID);
	profile.version = 0x0101;
	pfseq = sdp_list_append(0, &profile);
	sdp_set_profile_descs(&record, pfseq);

	sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
	proto[0] = sdp_list_append(0, &l2cap_uuid);
	apseq = sdp_list_append(0, proto[0]);

	sdp_uuid16_create(&rfcomm_uuid, RFCOMM_UUID);
	proto[1] = sdp_list_append(0, &rfcomm_uuid);
	channel = sdp_data_alloc(SDP_UINT8, &u8);
	proto[1] = sdp_list_append(proto[1], channel);
	apseq = sdp_list_append(apseq, proto[1]);

	features = sdp_data_alloc(SDP_UINT16, &u16);
	sdp_attr_add(&record, SDP_ATTR_SUPPORTED_FEATURES, features);

	aproto = sdp_list_append(0, apseq);
	sdp_set_access_protos(&record, aproto);

	sdp_set_info_attr(&record, "Handsfree", 0, 0);

	if (sdp_device_record_register(session, &interface, &record, SDP_RECORD_PERSIST) < 0) {
		printf("Service Record registration failed\n");
		ret = -1;
		goto end;
	}

	printf("Handsfree service registered\n");

end:
	sdp_data_free(channel);
	sdp_list_free(proto[0], 0);
	sdp_list_free(proto[1], 0);
	sdp_list_free(apseq, 0);
	sdp_list_free(aproto, 0);

	return ret;
}

static int add_handsfree_ag(sdp_session_t *session, svc_info_t *si)
{
	sdp_list_t *svclass_id, *pfseq, *apseq, *root;
	uuid_t root_uuid, svclass_uuid, ga_svclass_uuid, l2cap_uuid, rfcomm_uuid;
	sdp_profile_desc_t profile;
	sdp_list_t *aproto, *proto[2];
	sdp_record_t record;
	uint8_t u8 = si->channel ? si->channel : 7;
	uint16_t u16 = 0x17;
	sdp_data_t *channel, *features;
	uint8_t netid = si->network ? si->network : 0x01; // ???? profile document
	sdp_data_t *network = sdp_data_alloc(SDP_UINT8, &netid);
	int ret = 0;

	memset(&record, 0, sizeof(sdp_record_t));
	record.handle = si->handle;

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(0, &root_uuid);
	sdp_set_browse_groups(&record, root);

	sdp_uuid16_create(&svclass_uuid, HANDSFREE_AGW_SVCLASS_ID);
	svclass_id = sdp_list_append(0, &svclass_uuid);
	sdp_uuid16_create(&ga_svclass_uuid, GENERIC_AUDIO_SVCLASS_ID);
	svclass_id = sdp_list_append(svclass_id, &ga_svclass_uuid);
	sdp_set_service_classes(&record, svclass_id);

	sdp_uuid16_create(&profile.uuid, HANDSFREE_PROFILE_ID);
	profile.version = 0x0105;
	pfseq = sdp_list_append(0, &profile);
	sdp_set_profile_descs(&record, pfseq);

	sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
	proto[0] = sdp_list_append(0, &l2cap_uuid);
	apseq = sdp_list_append(0, proto[0]);

	sdp_uuid16_create(&rfcomm_uuid, RFCOMM_UUID);
	proto[1] = sdp_list_append(0, &rfcomm_uuid);
	channel = sdp_data_alloc(SDP_UINT8, &u8);
	proto[1] = sdp_list_append(proto[1], channel);
	apseq = sdp_list_append(apseq, proto[1]);

	features = sdp_data_alloc(SDP_UINT16, &u16);
	sdp_attr_add(&record, SDP_ATTR_SUPPORTED_FEATURES, features);

	aproto = sdp_list_append(0, apseq);
	sdp_set_access_protos(&record, aproto);

	sdp_set_info_attr(&record, "Voice Gateway", 0, 0);

	sdp_attr_add(&record, SDP_ATTR_EXTERNAL_NETWORK, network);

	if (sdp_record_register(session, &record, SDP_RECORD_PERSIST) < 0) {
		printf("Service Record registration failed\n");
		ret = -1;
		goto end;
	}

	printf("Handsfree AG service registered\n");

end:
	sdp_data_free(channel);
	sdp_list_free(proto[0], 0);
	sdp_list_free(proto[1], 0);
	sdp_list_free(apseq, 0);
	sdp_list_free(aproto, 0);

	return ret;
}

static int add_simaccess(sdp_session_t *session, svc_info_t *si)
{
	sdp_list_t *svclass_id, *pfseq, *apseq, *root;
	uuid_t root_uuid, svclass_uuid, ga_svclass_uuid, l2cap_uuid, rfcomm_uuid;
	sdp_profile_desc_t profile;
	sdp_list_t *aproto, *proto[2];
	sdp_record_t record;
	uint8_t u8 = si->channel? si->channel : 8;
	uint16_t u16 = 0x31;
	sdp_data_t *channel, *features;
	int ret = 0;

	memset((void *)&record, 0, sizeof(sdp_record_t));
	record.handle = si->handle;

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(0, &root_uuid);
	sdp_set_browse_groups(&record, root);

	sdp_uuid16_create(&svclass_uuid, SAP_SVCLASS_ID);
	svclass_id = sdp_list_append(0, &svclass_uuid);
	sdp_uuid16_create(&ga_svclass_uuid, GENERIC_TELEPHONY_SVCLASS_ID);
	svclass_id = sdp_list_append(svclass_id, &ga_svclass_uuid);
	sdp_set_service_classes(&record, svclass_id);

	sdp_uuid16_create(&profile.uuid, SAP_PROFILE_ID);
	profile.version = 0x0101;
	pfseq = sdp_list_append(0, &profile);
	sdp_set_profile_descs(&record, pfseq);

	sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
	proto[0] = sdp_list_append(0, &l2cap_uuid);
	apseq = sdp_list_append(0, proto[0]);

	sdp_uuid16_create(&rfcomm_uuid, RFCOMM_UUID);
	proto[1] = sdp_list_append(0, &rfcomm_uuid);
	channel = sdp_data_alloc(SDP_UINT8, &u8);
	proto[1] = sdp_list_append(proto[1], channel);
	apseq = sdp_list_append(apseq, proto[1]);

	features = sdp_data_alloc(SDP_UINT16, &u16);
	sdp_attr_add(&record, SDP_ATTR_SUPPORTED_FEATURES, features);

	aproto = sdp_list_append(0, apseq);
	sdp_set_access_protos(&record, aproto);

	sdp_set_info_attr(&record, "SIM Access", 0, 0);

	if (sdp_device_record_register(session, &interface, &record, SDP_RECORD_PERSIST) < 0) {
		printf("Service Record registration failed\n");
		ret = -1;
		goto end;
	}

	printf("SIM Access service registered\n");

end:
	sdp_data_free(channel);
	sdp_list_free(proto[0], 0);
	sdp_list_free(proto[1], 0);
	sdp_list_free(apseq, 0);
	sdp_list_free(aproto, 0);

	return ret;
}

static int add_opush(sdp_session_t *session, svc_info_t *si)
{
	sdp_list_t *svclass_id, *pfseq, *apseq, *root;
	uuid_t root_uuid, opush_uuid, l2cap_uuid, rfcomm_uuid, obex_uuid;
	sdp_profile_desc_t profile[1];
	sdp_list_t *aproto, *proto[3];
	sdp_record_t record;
	uint8_t chan = si->channel ? si->channel : 9;
	sdp_data_t *channel;
	uint8_t formats[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0xff };
	void *dtds[sizeof(formats)], *values[sizeof(formats)];
	unsigned int i;
	uint8_t dtd = SDP_UINT8;
	sdp_data_t *sflist;
	int ret = 0;

	memset(&record, 0, sizeof(sdp_record_t));
	record.handle = si->handle;

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(0, &root_uuid);
	sdp_set_browse_groups(&record, root);

	sdp_uuid16_create(&opush_uuid, OBEX_OBJPUSH_SVCLASS_ID);
	svclass_id = sdp_list_append(0, &opush_uuid);
	sdp_set_service_classes(&record, svclass_id);

	sdp_uuid16_create(&profile[0].uuid, OBEX_OBJPUSH_PROFILE_ID);
	profile[0].version = 0x0100;
	pfseq = sdp_list_append(0, profile);
	sdp_set_profile_descs(&record, pfseq);

	sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
	proto[0] = sdp_list_append(0, &l2cap_uuid);
	apseq = sdp_list_append(0, proto[0]);

	sdp_uuid16_create(&rfcomm_uuid, RFCOMM_UUID);
	proto[1] = sdp_list_append(0, &rfcomm_uuid);
	channel = sdp_data_alloc(SDP_UINT8, &chan);
	proto[1] = sdp_list_append(proto[1], channel);
	apseq = sdp_list_append(apseq, proto[1]);

	sdp_uuid16_create(&obex_uuid, OBEX_UUID);
	proto[2] = sdp_list_append(0, &obex_uuid);
	apseq = sdp_list_append(apseq, proto[2]);

	aproto = sdp_list_append(0, apseq);
	sdp_set_access_protos(&record, aproto);

	for (i = 0; i < sizeof(formats); i++) {
		dtds[i] = &dtd;
		values[i] = &formats[i];
	}
	sflist = sdp_seq_alloc(dtds, values, sizeof(formats));
	sdp_attr_add(&record, SDP_ATTR_SUPPORTED_FORMATS_LIST, sflist);

	sdp_set_info_attr(&record, "OBEX Object Push", 0, 0);

	if (sdp_device_record_register(session, &interface, &record, SDP_RECORD_PERSIST) < 0) {
		printf("Service Record registration failed\n");
		ret = -1;
		goto end;
	}

	printf("OBEX Object Push service registered\n");

end:
	sdp_data_free(channel);
	sdp_list_free(proto[0], 0);
	sdp_list_free(proto[1], 0);
	sdp_list_free(proto[2], 0);
	sdp_list_free(apseq, 0);
	sdp_list_free(pfseq, 0);
	sdp_list_free(aproto, 0);
	sdp_list_free(root, 0);
	sdp_list_free(svclass_id, NULL);

	return ret;
}

static int add_pbap(sdp_session_t *session, svc_info_t *si)
{
	sdp_list_t *svclass_id, *pfseq, *apseq, *root;
	uuid_t root_uuid, pbap_uuid, l2cap_uuid, rfcomm_uuid, obex_uuid;
	sdp_profile_desc_t profile[1];
	sdp_list_t *aproto, *proto[3];
	sdp_record_t record;
	uint8_t chan = si->channel ? si->channel : 19;
	sdp_data_t *channel;
	uint8_t formats[] = {0x01};
	uint8_t dtd = SDP_UINT8;
	sdp_data_t *sflist;
	int ret = 0;

	memset(&record, 0, sizeof(sdp_record_t));
	record.handle = si->handle;

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(0, &root_uuid);
	sdp_set_browse_groups(&record, root);

	sdp_uuid16_create(&pbap_uuid, PBAP_PSE_SVCLASS_ID);
	svclass_id = sdp_list_append(0, &pbap_uuid);
	sdp_set_service_classes(&record, svclass_id);

	sdp_uuid16_create(&profile[0].uuid, PBAP_PROFILE_ID);
	profile[0].version = 0x0100;
	pfseq = sdp_list_append(0, profile);
	sdp_set_profile_descs(&record, pfseq);

	sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
	proto[0] = sdp_list_append(0, &l2cap_uuid);
	apseq = sdp_list_append(0, proto[0]);

	sdp_uuid16_create(&rfcomm_uuid, RFCOMM_UUID);
	proto[1] = sdp_list_append(0, &rfcomm_uuid);
	channel = sdp_data_alloc(SDP_UINT8, &chan);
	proto[1] = sdp_list_append(proto[1], channel);
	apseq = sdp_list_append(apseq, proto[1]);

	sdp_uuid16_create(&obex_uuid, OBEX_UUID);
	proto[2] = sdp_list_append(0, &obex_uuid);
	apseq = sdp_list_append(apseq, proto[2]);

	aproto = sdp_list_append(0, apseq);
	sdp_set_access_protos(&record, aproto);

	sflist = sdp_data_alloc(dtd,formats);
	sdp_attr_add(&record, SDP_ATTR_SUPPORTED_REPOSITORIES, sflist);

	sdp_set_info_attr(&record, "OBEX Phonebook Access Server", 0, 0);

	if (sdp_device_record_register(session, &interface, &record,
			SDP_RECORD_PERSIST) < 0) {
		printf("Service Record registration failed\n");
		ret = -1;
		goto end;
	}

	printf("PBAP service registered\n");

end:
	sdp_data_free(channel);
	sdp_list_free(proto[0], 0);
	sdp_list_free(proto[1], 0);
	sdp_list_free(proto[2], 0);
	sdp_list_free(apseq, 0);
	sdp_list_free(pfseq, 0);
	sdp_list_free(aproto, 0);
	sdp_list_free(root, 0);
	sdp_list_free(svclass_id, 0);

	return ret;
}

static int add_map(sdp_session_t *session, svc_info_t *si)
{
	sdp_list_t *svclass_id, *pfseq, *apseq, *root;
	uuid_t root_uuid, map_uuid, l2cap_uuid, rfcomm_uuid, obex_uuid;
	sdp_profile_desc_t profile[1];
	sdp_list_t *aproto, *proto[3];
	sdp_record_t record;
	uint8_t chan = si->channel ? si->channel : 17;
	sdp_data_t *channel;
	uint8_t msg_formats[] = {0x0f};
	uint32_t supp_features[] = {0x0000007f};
	uint8_t dtd_msg = SDP_UINT8, dtd_sf = SDP_UINT32;
	sdp_data_t *smlist;
	sdp_data_t *sflist;
	int ret = 0;

	memset(&record, 0, sizeof(sdp_record_t));
	record.handle = si->handle;

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(0, &root_uuid);
	sdp_set_browse_groups(&record, root);

	sdp_uuid16_create(&map_uuid, MAP_MSE_SVCLASS_ID);
	svclass_id = sdp_list_append(0, &map_uuid);
	sdp_set_service_classes(&record, svclass_id);

	sdp_uuid16_create(&profile[0].uuid, MAP_PROFILE_ID);
	profile[0].version = 0x0100;
	pfseq = sdp_list_append(0, profile);
	sdp_set_profile_descs(&record, pfseq);

	sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
	proto[0] = sdp_list_append(0, &l2cap_uuid);
	apseq = sdp_list_append(0, proto[0]);

	sdp_uuid16_create(&rfcomm_uuid, RFCOMM_UUID);
	proto[1] = sdp_list_append(0, &rfcomm_uuid);
	channel = sdp_data_alloc(SDP_UINT8, &chan);
	proto[1] = sdp_list_append(proto[1], channel);
	apseq = sdp_list_append(apseq, proto[1]);

	sdp_uuid16_create(&obex_uuid, OBEX_UUID);
	proto[2] = sdp_list_append(0, &obex_uuid);
	apseq = sdp_list_append(apseq, proto[2]);

	aproto = sdp_list_append(0, apseq);
	sdp_set_access_protos(&record, aproto);

	smlist = sdp_data_alloc(dtd_msg, msg_formats);
	sdp_attr_add(&record, SDP_ATTR_SUPPORTED_MESSAGE_TYPES, smlist);

	sflist = sdp_data_alloc(dtd_sf, supp_features);
	sdp_attr_add(&record, SDP_ATTR_MAP_SUPPORTED_FEATURES, sflist);

	sdp_set_info_attr(&record, "OBEX Message Access Server", 0, 0);

	if (sdp_device_record_register(session, &interface, &record,
			SDP_RECORD_PERSIST) < 0) {
		printf("Service Record registration failed\n");
		ret = -1;
		goto end;
	}

	printf("MAP service registered\n");

end:
	sdp_data_free(channel);
	sdp_list_free(proto[0], 0);
	sdp_list_free(proto[1], 0);
	sdp_list_free(proto[2], 0);
	sdp_list_free(apseq, 0);
	sdp_list_free(pfseq, 0);
	sdp_list_free(aproto, 0);
	sdp_list_free(root, 0);
	sdp_list_free(svclass_id, 0);

	return ret;
}

static int add_ftp(sdp_session_t *session, svc_info_t *si)
{
	sdp_list_t *svclass_id, *pfseq, *apseq, *root;
	uuid_t root_uuid, ftrn_uuid, l2cap_uuid, rfcomm_uuid, obex_uuid;
	sdp_profile_desc_t profile[1];
	sdp_list_t *aproto, *proto[3];
	sdp_record_t record;
	uint8_t u8 = si->channel ? si->channel: 10;
	sdp_data_t *channel;
	int ret = 0;

	memset(&record, 0, sizeof(sdp_record_t));
	record.handle = si->handle;

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(0, &root_uuid);
	sdp_set_browse_groups(&record, root);

	sdp_uuid16_create(&ftrn_uuid, OBEX_FILETRANS_SVCLASS_ID);
	svclass_id = sdp_list_append(0, &ftrn_uuid);
	sdp_set_service_classes(&record, svclass_id);

	sdp_uuid16_create(&profile[0].uuid, OBEX_FILETRANS_PROFILE_ID);
	profile[0].version = 0x0100;
	pfseq = sdp_list_append(0, &profile[0]);
	sdp_set_profile_descs(&record, pfseq);

	sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
	proto[0] = sdp_list_append(0, &l2cap_uuid);
	apseq = sdp_list_append(0, proto[0]);

	sdp_uuid16_create(&rfcomm_uuid, RFCOMM_UUID);
	proto[1] = sdp_list_append(0, &rfcomm_uuid);
	channel = sdp_data_alloc(SDP_UINT8, &u8);
	proto[1] = sdp_list_append(proto[1], channel);
	apseq = sdp_list_append(apseq, proto[1]);

	sdp_uuid16_create(&obex_uuid, OBEX_UUID);
	proto[2] = sdp_list_append(0, &obex_uuid);
	apseq = sdp_list_append(apseq, proto[2]);

	aproto = sdp_list_append(0, apseq);
	sdp_set_access_protos(&record, aproto);

	sdp_set_info_attr(&record, "OBEX File Transfer", 0, 0);

	if (sdp_device_record_register(session, &interface, &record, SDP_RECORD_PERSIST) < 0) {
		printf("Service Record registration failed\n");
		ret = -1;
		goto end;
	}

	printf("OBEX File Transfer service registered\n");

end:
	sdp_data_free(channel);
	sdp_list_free(proto[0], 0);
	sdp_list_free(proto[1], 0);
	sdp_list_free(proto[2], 0);
	sdp_list_free(apseq, 0);
	sdp_list_free(aproto, 0);

	return ret;
}

static int add_directprint(sdp_session_t *session, svc_info_t *si)
{
	sdp_list_t *svclass_id, *pfseq, *apseq, *root;
	uuid_t root_uuid, opush_uuid, l2cap_uuid, rfcomm_uuid, obex_uuid;
	sdp_profile_desc_t profile[1];
	sdp_list_t *aproto, *proto[3];
	sdp_record_t record;
	uint8_t chan = si->channel ? si->channel : 12;
	sdp_data_t *channel;
	int ret = 0;

	memset(&record, 0, sizeof(sdp_record_t));
	record.handle = si->handle;

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(0, &root_uuid);
	sdp_set_browse_groups(&record, root);

	sdp_uuid16_create(&opush_uuid, DIRECT_PRINTING_SVCLASS_ID);
	svclass_id = sdp_list_append(0, &opush_uuid);
	sdp_set_service_classes(&record, svclass_id);

	sdp_uuid16_create(&profile[0].uuid, BASIC_PRINTING_PROFILE_ID);
	profile[0].version = 0x0100;
	pfseq = sdp_list_append(0, profile);
	sdp_set_profile_descs(&record, pfseq);

	sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
	proto[0] = sdp_list_append(0, &l2cap_uuid);
	apseq = sdp_list_append(0, proto[0]);

	sdp_uuid16_create(&rfcomm_uuid, RFCOMM_UUID);
	proto[1] = sdp_list_append(0, &rfcomm_uuid);
	channel = sdp_data_alloc(SDP_UINT8, &chan);
	proto[1] = sdp_list_append(proto[1], channel);
	apseq = sdp_list_append(apseq, proto[1]);

	sdp_uuid16_create(&obex_uuid, OBEX_UUID);
	proto[2] = sdp_list_append(0, &obex_uuid);
	apseq = sdp_list_append(apseq, proto[2]);

	aproto = sdp_list_append(0, apseq);
	sdp_set_access_protos(&record, aproto);

	sdp_set_info_attr(&record, "Direct Printing", 0, 0);

	if (sdp_device_record_register(session, &interface, &record, SDP_RECORD_PERSIST) < 0) {
		printf("Service Record registration failed\n");
		ret = -1;
		goto end;
	}

	printf("Direct Printing service registered\n");

end:
	sdp_data_free(channel);
	sdp_list_free(proto[0], 0);
	sdp_list_free(proto[1], 0);
	sdp_list_free(proto[2], 0);
	sdp_list_free(apseq, 0);
	sdp_list_free(aproto, 0);

	return ret;
}

static int add_nap(sdp_session_t *session, svc_info_t *si)
{
	sdp_list_t *svclass_id, *pfseq, *apseq, *root;
	uuid_t root_uuid, ftrn_uuid, l2cap_uuid, bnep_uuid;
	sdp_profile_desc_t profile[1];
	sdp_list_t *aproto, *proto[2];
	sdp_record_t record;
	uint16_t lp = 0x000f, ver = 0x0100;
	sdp_data_t *psm, *version;
	int ret = 0;

	memset(&record, 0, sizeof(sdp_record_t));
	record.handle = si->handle;

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(0, &root_uuid);
	sdp_set_browse_groups(&record, root);

	sdp_uuid16_create(&ftrn_uuid, NAP_SVCLASS_ID);
	svclass_id = sdp_list_append(0, &ftrn_uuid);
	sdp_set_service_classes(&record, svclass_id);

	sdp_uuid16_create(&profile[0].uuid, NAP_PROFILE_ID);
	profile[0].version = 0x0100;
	pfseq = sdp_list_append(0, &profile[0]);
	sdp_set_profile_descs(&record, pfseq);

	sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
	proto[0] = sdp_list_append(0, &l2cap_uuid);
	psm = sdp_data_alloc(SDP_UINT16, &lp);
	proto[0] = sdp_list_append(proto[0], psm);
	apseq = sdp_list_append(0, proto[0]);

	sdp_uuid16_create(&bnep_uuid, BNEP_UUID);
	proto[1] = sdp_list_append(0, &bnep_uuid);
	version  = sdp_data_alloc(SDP_UINT16, &ver);
	proto[1] = sdp_list_append(proto[1], version);

	{
		uint16_t ptype[4] = { 0x0010, 0x0020, 0x0030, 0x0040 };
		sdp_data_t *head, *pseq;
		int p;

		for (p = 0, head = NULL; p < 4; p++) {
			sdp_data_t *data = sdp_data_alloc(SDP_UINT16, &ptype[p]);
			head = sdp_seq_append(head, data);
		}
		pseq = sdp_data_alloc(SDP_SEQ16, head);
		proto[1] = sdp_list_append(proto[1], pseq);
	}

	apseq = sdp_list_append(apseq, proto[1]);

	aproto = sdp_list_append(0, apseq);
	sdp_set_access_protos(&record, aproto);

	sdp_set_info_attr(&record, "Network Access Point Service", 0, 0);

	if (sdp_device_record_register(session, &interface, &record, SDP_RECORD_PERSIST) < 0) {
		printf("Service Record registration failed\n");
		ret = -1;
		goto end;
	}

	printf("NAP service registered\n");

end:
	sdp_data_free(version);
	sdp_data_free(psm);
	sdp_list_free(proto[0], 0);
	sdp_list_free(proto[1], 0);
	sdp_list_free(apseq, 0);
	sdp_list_free(aproto, 0);

	return ret;
}

static int add_gn(sdp_session_t *session, svc_info_t *si)
{
	sdp_list_t *svclass_id, *pfseq, *apseq, *root;
	uuid_t root_uuid, ftrn_uuid, l2cap_uuid, bnep_uuid;
	sdp_profile_desc_t profile[1];
	sdp_list_t *aproto, *proto[2];
	sdp_record_t record;
	uint16_t lp = 0x000f, ver = 0x0100;
	sdp_data_t *psm, *version;
	int ret = 0;

	memset(&record, 0, sizeof(sdp_record_t));
	record.handle = si->handle;

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(0, &root_uuid);
	sdp_set_browse_groups(&record, root);

	sdp_uuid16_create(&ftrn_uuid, GN_SVCLASS_ID);
	svclass_id = sdp_list_append(0, &ftrn_uuid);
	sdp_set_service_classes(&record, svclass_id);

	sdp_uuid16_create(&profile[0].uuid, GN_PROFILE_ID);
	profile[0].version = 0x0100;
	pfseq = sdp_list_append(0, &profile[0]);
	sdp_set_profile_descs(&record, pfseq);

	sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
	proto[0] = sdp_list_append(0, &l2cap_uuid);
	psm = sdp_data_alloc(SDP_UINT16, &lp);
	proto[0] = sdp_list_append(proto[0], psm);
	apseq = sdp_list_append(0, proto[0]);

	sdp_uuid16_create(&bnep_uuid, BNEP_UUID);
	proto[1] = sdp_list_append(0, &bnep_uuid);
	version = sdp_data_alloc(SDP_UINT16, &ver);
	proto[1] = sdp_list_append(proto[1], version);
	apseq = sdp_list_append(apseq, proto[1]);

	aproto = sdp_list_append(0, apseq);
	sdp_set_access_protos(&record, aproto);

	sdp_set_info_attr(&record, "Group Network Service", 0, 0);

	if (sdp_device_record_register(session, &interface, &record, SDP_RECORD_PERSIST) < 0) {
		printf("Service Record registration failed\n");
		ret = -1;
		goto end;
	}

	printf("GN service registered\n");

end:
	sdp_data_free(version);
	sdp_data_free(psm);
	sdp_list_free(proto[0], 0);
	sdp_list_free(proto[1], 0);
	sdp_list_free(apseq, 0);
	sdp_list_free(aproto, 0);

	return ret;
}

static int add_panu(sdp_session_t *session, svc_info_t *si)
{
	sdp_list_t *svclass_id, *pfseq, *apseq, *root;
	uuid_t root_uuid, ftrn_uuid, l2cap_uuid, bnep_uuid;
	sdp_profile_desc_t profile[1];
	sdp_list_t *aproto, *proto[2];
	sdp_record_t record;
	uint16_t lp = 0x000f, ver = 0x0100;
	sdp_data_t *psm, *version;
	int ret = 0;

	memset(&record, 0, sizeof(sdp_record_t));
	record.handle = si->handle;

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(NULL, &root_uuid);
	sdp_set_browse_groups(&record, root);
	sdp_list_free(root, NULL);

	sdp_uuid16_create(&ftrn_uuid, PANU_SVCLASS_ID);
	svclass_id = sdp_list_append(NULL, &ftrn_uuid);
	sdp_set_service_classes(&record, svclass_id);
	sdp_list_free(svclass_id, NULL);

	sdp_uuid16_create(&profile[0].uuid, PANU_PROFILE_ID);
	profile[0].version = 0x0100;
	pfseq = sdp_list_append(NULL, &profile[0]);
	sdp_set_profile_descs(&record, pfseq);
	sdp_list_free(pfseq, NULL);

	sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
	proto[0] = sdp_list_append(NULL, &l2cap_uuid);
	psm = sdp_data_alloc(SDP_UINT16, &lp);
	proto[0] = sdp_list_append(proto[0], psm);
	apseq = sdp_list_append(NULL, proto[0]);

	sdp_uuid16_create(&bnep_uuid, BNEP_UUID);
	proto[1] = sdp_list_append(NULL, &bnep_uuid);
	version = sdp_data_alloc(SDP_UINT16, &ver);
	proto[1] = sdp_list_append(proto[1], version);
	apseq = sdp_list_append(apseq, proto[1]);

	aproto = sdp_list_append(NULL, apseq);
	sdp_set_access_protos(&record, aproto);

	sdp_set_info_attr(&record, "PAN User", NULL, NULL);

	if (sdp_device_record_register(session, &interface, &record, SDP_RECORD_PERSIST) < 0) {
		printf("Service Record registration failed\n");
		ret = -1;
		goto end;
	}

	printf("PANU service registered\n");

end:
	sdp_data_free(version);
	sdp_data_free(psm);
	sdp_list_free(proto[0], 0);
	sdp_list_free(proto[1], 0);
	sdp_list_free(apseq, 0);
	sdp_list_free(aproto, 0);

	return ret;
}

static int add_hid_keyb(sdp_session_t *session, svc_info_t *si)
{
	sdp_record_t record;
	sdp_list_t *svclass_id, *pfseq, *apseq, *root;
	uuid_t root_uuid, hidkb_uuid, l2cap_uuid, hidp_uuid;
	sdp_profile_desc_t profile[1];
	sdp_list_t *aproto, *proto[3];
	sdp_data_t *psm, *lang_lst, *lang_lst2, *hid_spec_lst, *hid_spec_lst2;
	unsigned int i;
	uint8_t dtd = SDP_UINT16;
	uint8_t dtd2 = SDP_UINT8;
	uint8_t dtd_data = SDP_TEXT_STR8;
	void *dtds[2];
	void *values[2];
	void *dtds2[2];
	void *values2[2];
	int leng[2];
	uint8_t hid_spec_type = 0x22;
	uint16_t hid_attr_lang[] = { 0x409, 0x100 };
	static const uint16_t ctrl = 0x11;
	static const uint16_t intr = 0x13;
	static const uint16_t hid_attr[] = { 0x100, 0x111, 0x40, 0x0d, 0x01, 0x01 };
	static const uint16_t hid_attr2[] = { 0x0, 0x01, 0x100, 0x1f40, 0x01, 0x01 };
	const uint8_t hid_spec[] = {
		0x05, 0x01, // usage page
		0x09, 0x06, // keyboard
		0xa1, 0x01, // key codes
		0x85, 0x01, // minimum
		0x05, 0x07, // max
		0x19, 0xe0, // logical min
		0x29, 0xe7, // logical max
		0x15, 0x00, // report size
		0x25, 0x01, // report count
		0x75, 0x01, // input data variable absolute
		0x95, 0x08, // report count
		0x81, 0x02, // report size
		0x75, 0x08,
		0x95, 0x01,
		0x81, 0x01,
		0x75, 0x01,
		0x95, 0x05,
		0x05, 0x08,
		0x19, 0x01,
		0x29, 0x05,
		0x91, 0x02,
		0x75, 0x03,
		0x95, 0x01,
		0x91, 0x01,
		0x75, 0x08,
		0x95, 0x06,
		0x15, 0x00,
		0x26, 0xff,
		0x00, 0x05,
		0x07, 0x19,
		0x00, 0x2a,
		0xff, 0x00,
		0x81, 0x00,
		0x75, 0x01,
		0x95, 0x01,
		0x15, 0x00,
		0x25, 0x01,
		0x05, 0x0c,
		0x09, 0xb8,
		0x81, 0x06,
		0x09, 0xe2,
		0x81, 0x06,
		0x09, 0xe9,
		0x81, 0x02,
		0x09, 0xea,
		0x81, 0x02,
		0x75, 0x01,
		0x95, 0x04,
		0x81, 0x01,
		0xc0         // end tag
	};

	memset(&record, 0, sizeof(sdp_record_t));
	record.handle = si->handle;

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(0, &root_uuid);
	sdp_set_browse_groups(&record, root);

	sdp_add_lang_attr(&record);

	sdp_uuid16_create(&hidkb_uuid, HID_SVCLASS_ID);
	svclass_id = sdp_list_append(0, &hidkb_uuid);
	sdp_set_service_classes(&record, svclass_id);

	sdp_uuid16_create(&profile[0].uuid, HID_PROFILE_ID);
	profile[0].version = 0x0100;
	pfseq = sdp_list_append(0, profile);
	sdp_set_profile_descs(&record, pfseq);

	/* protocols */
	sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
	proto[1] = sdp_list_append(0, &l2cap_uuid);
	psm = sdp_data_alloc(SDP_UINT16, &ctrl);
	proto[1] = sdp_list_append(proto[1], psm);
	apseq = sdp_list_append(0, proto[1]);

	sdp_uuid16_create(&hidp_uuid, HIDP_UUID);
	proto[2] = sdp_list_append(0, &hidp_uuid);
	apseq = sdp_list_append(apseq, proto[2]);

	aproto = sdp_list_append(0, apseq);
	sdp_set_access_protos(&record, aproto);

	/* additional protocols */
	proto[1] = sdp_list_append(0, &l2cap_uuid);
	psm = sdp_data_alloc(SDP_UINT16, &intr);
	proto[1] = sdp_list_append(proto[1], psm);
	apseq = sdp_list_append(0, proto[1]);

	sdp_uuid16_create(&hidp_uuid, HIDP_UUID);
	proto[2] = sdp_list_append(0, &hidp_uuid);
	apseq = sdp_list_append(apseq, proto[2]);

	aproto = sdp_list_append(0, apseq);
	sdp_set_add_access_protos(&record, aproto);

	sdp_set_info_attr(&record, "HID Keyboard", NULL, NULL);

	for (i = 0; i < sizeof(hid_attr) / 2; i++)
		sdp_attr_add_new(&record,
					SDP_ATTR_HID_DEVICE_RELEASE_NUMBER + i,
					SDP_UINT16, &hid_attr[i]);

	dtds[0] = &dtd2;
	values[0] = &hid_spec_type;
	dtds[1] = &dtd_data;
	values[1] = (uint8_t *) hid_spec;
	leng[0] = 0;
	leng[1] = sizeof(hid_spec);
	hid_spec_lst = sdp_seq_alloc_with_length(dtds, values, leng, 2);
	hid_spec_lst2 = sdp_data_alloc(SDP_SEQ8, hid_spec_lst);
	sdp_attr_add(&record, SDP_ATTR_HID_DESCRIPTOR_LIST, hid_spec_lst2);

	for (i = 0; i < sizeof(hid_attr_lang) / 2; i++) {
		dtds2[i] = &dtd;
		values2[i] = &hid_attr_lang[i];
	}

	lang_lst = sdp_seq_alloc(dtds2, values2, sizeof(hid_attr_lang) / 2);
	lang_lst2 = sdp_data_alloc(SDP_SEQ8, lang_lst);
	sdp_attr_add(&record, SDP_ATTR_HID_LANG_ID_BASE_LIST, lang_lst2);

	sdp_attr_add_new(&record, SDP_ATTR_HID_SDP_DISABLE, SDP_UINT16, &hid_attr2[0]);

	for (i = 0; i < sizeof(hid_attr2) / 2 - 1; i++)
		sdp_attr_add_new(&record, SDP_ATTR_HID_REMOTE_WAKEUP + i,
						SDP_UINT16, &hid_attr2[i + 1]);

	if (sdp_record_register(session, &record, SDP_RECORD_PERSIST) < 0) {
		printf("Service Record registration failed\n");
		return -1;
	}

	printf("HID keyboard service registered\n");

	return 0;
}

static int add_hid_wiimote(sdp_session_t *session, svc_info_t *si)
{
	sdp_record_t record;
	sdp_list_t *svclass_id, *pfseq, *apseq, *root;
	uuid_t root_uuid, hid_uuid, l2cap_uuid, hidp_uuid;
	sdp_profile_desc_t profile[1];
	sdp_list_t *aproto, *proto[3];
	sdp_data_t *psm, *lang_lst, *lang_lst2, *hid_spec_lst, *hid_spec_lst2;
	unsigned int i;
	uint8_t dtd = SDP_UINT16;
	uint8_t dtd2 = SDP_UINT8;
	uint8_t dtd_data = SDP_TEXT_STR8;
	void *dtds[2];
	void *values[2];
	void *dtds2[2];
	void *values2[2];
	int leng[2];
	uint8_t hid_spec_type = 0x22;
	uint16_t hid_attr_lang[] = { 0x409, 0x100 };
	uint16_t ctrl = 0x11, intr = 0x13;
	uint16_t hid_release = 0x0100, parser_version = 0x0111;
	uint8_t subclass = 0x04, country = 0x33;
	uint8_t virtual_cable = 0, reconnect = 1, sdp_disable = 0;
	uint8_t battery = 1, remote_wakeup = 1;
	uint16_t profile_version = 0x0100, superv_timeout = 0x0c80;
	uint8_t norm_connect = 0, boot_device = 0;
	const uint8_t hid_spec[] = {
		0x05, 0x01, 0x09, 0x05, 0xa1, 0x01, 0x85, 0x10,
		0x15, 0x00, 0x26, 0xff, 0x00, 0x75, 0x08, 0x95,
		0x01, 0x06, 0x00, 0xff, 0x09, 0x01, 0x91, 0x00,
		0x85, 0x11, 0x95, 0x01, 0x09, 0x01, 0x91, 0x00,
		0x85, 0x12, 0x95, 0x02, 0x09, 0x01, 0x91, 0x00,
		0x85, 0x13, 0x95, 0x01, 0x09, 0x01, 0x91, 0x00,
		0x85, 0x14, 0x95, 0x01, 0x09, 0x01, 0x91, 0x00,
		0x85, 0x15, 0x95, 0x01, 0x09, 0x01, 0x91, 0x00,
		0x85, 0x16, 0x95, 0x15, 0x09, 0x01, 0x91, 0x00,
		0x85, 0x17, 0x95, 0x06, 0x09, 0x01, 0x91, 0x00,
		0x85, 0x18, 0x95, 0x15, 0x09, 0x01, 0x91, 0x00,
		0x85, 0x19, 0x95, 0x01, 0x09, 0x01, 0x91, 0x00,
		0x85, 0x1a, 0x95, 0x01, 0x09, 0x01, 0x91, 0x00,
		0x85, 0x20, 0x95, 0x06, 0x09, 0x01, 0x81, 0x00,
		0x85, 0x21, 0x95, 0x15, 0x09, 0x01, 0x81, 0x00,
		0x85, 0x22, 0x95, 0x04, 0x09, 0x01, 0x81, 0x00,
		0x85, 0x30, 0x95, 0x02, 0x09, 0x01, 0x81, 0x00,
		0x85, 0x31, 0x95, 0x05, 0x09, 0x01, 0x81, 0x00,
		0x85, 0x32, 0x95, 0x0a, 0x09, 0x01, 0x81, 0x00,
		0x85, 0x33, 0x95, 0x11, 0x09, 0x01, 0x81, 0x00,
		0x85, 0x34, 0x95, 0x15, 0x09, 0x01, 0x81, 0x00,
		0x85, 0x35, 0x95, 0x15, 0x09, 0x01, 0x81, 0x00,
		0x85, 0x36, 0x95, 0x15, 0x09, 0x01, 0x81, 0x00,
		0x85, 0x37, 0x95, 0x15, 0x09, 0x01, 0x81, 0x00,
		0x85, 0x3d, 0x95, 0x15, 0x09, 0x01, 0x81, 0x00,
		0x85, 0x3e, 0x95, 0x15, 0x09, 0x01, 0x81, 0x00,
		0x85, 0x3f, 0x95, 0x15, 0x09, 0x01, 0x81, 0x00,
		0xc0, 0x00
	};

	memset(&record, 0, sizeof(sdp_record_t));
	record.handle = si->handle;

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(NULL, &root_uuid);
	sdp_set_browse_groups(&record, root);

	sdp_uuid16_create(&hid_uuid, HID_SVCLASS_ID);
	svclass_id = sdp_list_append(NULL, &hid_uuid);
	sdp_set_service_classes(&record, svclass_id);

	sdp_uuid16_create(&profile[0].uuid, HID_PROFILE_ID);
	profile[0].version = 0x0100;
	pfseq = sdp_list_append(NULL, profile);
	sdp_set_profile_descs(&record, pfseq);

	sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
	proto[1] = sdp_list_append(0, &l2cap_uuid);
	psm = sdp_data_alloc(SDP_UINT16, &ctrl);
	proto[1] = sdp_list_append(proto[1], psm);
	apseq = sdp_list_append(0, proto[1]);

	sdp_uuid16_create(&hidp_uuid, HIDP_UUID);
	proto[2] = sdp_list_append(0, &hidp_uuid);
	apseq = sdp_list_append(apseq, proto[2]);

	aproto = sdp_list_append(0, apseq);
	sdp_set_access_protos(&record, aproto);

	proto[1] = sdp_list_append(0, &l2cap_uuid);
	psm = sdp_data_alloc(SDP_UINT16, &intr);
	proto[1] = sdp_list_append(proto[1], psm);
	apseq = sdp_list_append(0, proto[1]);

	sdp_uuid16_create(&hidp_uuid, HIDP_UUID);
	proto[2] = sdp_list_append(0, &hidp_uuid);
	apseq = sdp_list_append(apseq, proto[2]);

	aproto = sdp_list_append(0, apseq);
	sdp_set_add_access_protos(&record, aproto);

	sdp_add_lang_attr(&record);

	sdp_set_info_attr(&record, "Nintendo RVL-CNT-01",
					"Nintendo", "Nintendo RVL-CNT-01");

	sdp_attr_add_new(&record, SDP_ATTR_HID_DEVICE_RELEASE_NUMBER,
						SDP_UINT16, &hid_release);

	sdp_attr_add_new(&record, SDP_ATTR_HID_PARSER_VERSION,
						SDP_UINT16, &parser_version);

	sdp_attr_add_new(&record, SDP_ATTR_HID_DEVICE_SUBCLASS,
						SDP_UINT8, &subclass);

	sdp_attr_add_new(&record, SDP_ATTR_HID_COUNTRY_CODE,
						SDP_UINT8, &country);

	sdp_attr_add_new(&record, SDP_ATTR_HID_VIRTUAL_CABLE,
						SDP_BOOL, &virtual_cable);

	sdp_attr_add_new(&record, SDP_ATTR_HID_RECONNECT_INITIATE,
						SDP_BOOL, &reconnect);

	dtds[0] = &dtd2;
	values[0] = &hid_spec_type;
	dtds[1] = &dtd_data;
	values[1] = (uint8_t *) hid_spec;
	leng[0] = 0;
	leng[1] = sizeof(hid_spec);
	hid_spec_lst = sdp_seq_alloc_with_length(dtds, values, leng, 2);
	hid_spec_lst2 = sdp_data_alloc(SDP_SEQ8, hid_spec_lst);
	sdp_attr_add(&record, SDP_ATTR_HID_DESCRIPTOR_LIST, hid_spec_lst2);

	for (i = 0; i < sizeof(hid_attr_lang) / 2; i++) {
		dtds2[i] = &dtd;
		values2[i] = &hid_attr_lang[i];
	}

	lang_lst = sdp_seq_alloc(dtds2, values2, sizeof(hid_attr_lang) / 2);
	lang_lst2 = sdp_data_alloc(SDP_SEQ8, lang_lst);
	sdp_attr_add(&record, SDP_ATTR_HID_LANG_ID_BASE_LIST, lang_lst2);

	sdp_attr_add_new(&record, SDP_ATTR_HID_SDP_DISABLE,
						SDP_BOOL, &sdp_disable);

	sdp_attr_add_new(&record, SDP_ATTR_HID_BATTERY_POWER,
						SDP_BOOL, &battery);

	sdp_attr_add_new(&record, SDP_ATTR_HID_REMOTE_WAKEUP,
						SDP_BOOL, &remote_wakeup);

	sdp_attr_add_new(&record, SDP_ATTR_HID_PROFILE_VERSION,
						SDP_UINT16, &profile_version);

	sdp_attr_add_new(&record, SDP_ATTR_HID_SUPERVISION_TIMEOUT,
						SDP_UINT16, &superv_timeout);

	sdp_attr_add_new(&record, SDP_ATTR_HID_NORMALLY_CONNECTABLE,
						SDP_BOOL, &norm_connect);

	sdp_attr_add_new(&record, SDP_ATTR_HID_BOOT_DEVICE,
						SDP_BOOL, &boot_device);

	if (sdp_record_register(session, &record, SDP_RECORD_PERSIST) < 0) {
		printf("Service Record registration failed\n");
		return -1;
	}

	printf("Wii-Mote service registered\n");

	return 0;
}

static int add_cip(sdp_session_t *session, svc_info_t *si)
{
	sdp_list_t *svclass_id, *pfseq, *apseq, *root;
	uuid_t root_uuid, l2cap, cmtp, cip;
	sdp_profile_desc_t profile[1];
	sdp_list_t *aproto, *proto[2];
	sdp_record_t record;
	uint16_t psm = si->psm ? si->psm : 0x1001;
	uint8_t netid = si->network ? si->network : 0x02; // 0x02 = ISDN, 0x03 = GSM
	sdp_data_t *network = sdp_data_alloc(SDP_UINT8, &netid);
	int ret = 0;

	memset(&record, 0, sizeof(sdp_record_t));
	record.handle = si->handle;

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(0, &root_uuid);
	sdp_set_browse_groups(&record, root);

	sdp_uuid16_create(&cip, CIP_SVCLASS_ID);
	svclass_id = sdp_list_append(0, &cip);
	sdp_set_service_classes(&record, svclass_id);

	sdp_uuid16_create(&profile[0].uuid, CIP_PROFILE_ID);
	profile[0].version = 0x0100;
	pfseq = sdp_list_append(0, &profile[0]);
	sdp_set_profile_descs(&record, pfseq);

	sdp_uuid16_create(&l2cap, L2CAP_UUID);
	proto[0] = sdp_list_append(0, &l2cap);
	apseq = sdp_list_append(0, proto[0]);
	proto[0] = sdp_list_append(proto[0], sdp_data_alloc(SDP_UINT16, &psm));
	apseq = sdp_list_append(apseq, proto[0]);

	sdp_uuid16_create(&cmtp, CMTP_UUID);
	proto[1] = sdp_list_append(0, &cmtp);
	apseq = sdp_list_append(apseq, proto[1]);

	aproto = sdp_list_append(0, apseq);
	sdp_set_access_protos(&record, aproto);

	sdp_attr_add(&record, SDP_ATTR_EXTERNAL_NETWORK, network);

	sdp_set_info_attr(&record, "Common ISDN Access", 0, 0);

	if (sdp_device_record_register(session, &interface, &record, SDP_RECORD_PERSIST) < 0) {
		printf("Service Record registration failed\n");
		ret = -1;
		goto end;
	}

	printf("CIP service registered\n");

end:
	sdp_list_free(proto[0], 0);
	sdp_list_free(proto[1], 0);
	sdp_list_free(apseq, 0);
	sdp_list_free(aproto, 0);
	sdp_data_free(network);

	return ret;
}

static int add_ctp(sdp_session_t *session, svc_info_t *si)
{
	sdp_list_t *svclass_id, *pfseq, *apseq, *root;
	uuid_t root_uuid, l2cap, tcsbin, ctp;
	sdp_profile_desc_t profile[1];
	sdp_list_t *aproto, *proto[2];
	sdp_record_t record;
	uint8_t netid = si->network ? si->network : 0x02; // 0x01-0x07 cf. p120 profile document
	sdp_data_t *network = sdp_data_alloc(SDP_UINT8, &netid);
	int ret = 0;

	memset(&record, 0, sizeof(sdp_record_t));
	record.handle = si->handle;

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(0, &root_uuid);
	sdp_set_browse_groups(&record, root);

	sdp_uuid16_create(&ctp, CORDLESS_TELEPHONY_SVCLASS_ID);
	svclass_id = sdp_list_append(0, &ctp);
	sdp_set_service_classes(&record, svclass_id);

	sdp_uuid16_create(&profile[0].uuid, CORDLESS_TELEPHONY_PROFILE_ID);
	profile[0].version = 0x0100;
	pfseq = sdp_list_append(0, &profile[0]);
	sdp_set_profile_descs(&record, pfseq);

	sdp_uuid16_create(&l2cap, L2CAP_UUID);
	proto[0] = sdp_list_append(0, &l2cap);
	apseq = sdp_list_append(0, proto[0]);

	sdp_uuid16_create(&tcsbin, TCS_BIN_UUID);
	proto[1] = sdp_list_append(0, &tcsbin);
	apseq = sdp_list_append(apseq, proto[1]);

	aproto = sdp_list_append(0, apseq);
	sdp_set_access_protos(&record, aproto);

	sdp_attr_add(&record, SDP_ATTR_EXTERNAL_NETWORK, network);

	sdp_set_info_attr(&record, "Cordless Telephony", 0, 0);

	if (sdp_device_record_register(session, &interface, &record, SDP_RECORD_PERSIST) < 0) {
		printf("Service Record registration failed\n");
		ret = -1;
		goto end;
	}

	printf("CTP service registered\n");

end:
	sdp_list_free(proto[0], 0);
	sdp_list_free(proto[1], 0);
	sdp_list_free(apseq, 0);
	sdp_list_free(aproto, 0);
	sdp_data_free(network);

	return ret;
}

static int add_a2source(sdp_session_t *session, svc_info_t *si)
{
	sdp_list_t *svclass_id, *pfseq, *apseq, *root;
	uuid_t root_uuid, l2cap, avdtp, a2src;
	sdp_profile_desc_t profile[1];
	sdp_list_t *aproto, *proto[2];
	sdp_record_t record;
	sdp_data_t *psm, *version;
	uint16_t lp = 0x0019, ver = 0x0100;
	int ret = 0;

	memset(&record, 0, sizeof(sdp_record_t));
	record.handle = si->handle;

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(0, &root_uuid);
	sdp_set_browse_groups(&record, root);

	sdp_uuid16_create(&a2src, AUDIO_SOURCE_SVCLASS_ID);
	svclass_id = sdp_list_append(0, &a2src);
	sdp_set_service_classes(&record, svclass_id);

	sdp_uuid16_create(&profile[0].uuid, ADVANCED_AUDIO_PROFILE_ID);
	profile[0].version = 0x0100;
	pfseq = sdp_list_append(0, &profile[0]);
	sdp_set_profile_descs(&record, pfseq);

	sdp_uuid16_create(&l2cap, L2CAP_UUID);
	proto[0] = sdp_list_append(0, &l2cap);
	psm = sdp_data_alloc(SDP_UINT16, &lp);
	proto[0] = sdp_list_append(proto[0], psm);
	apseq = sdp_list_append(0, proto[0]);

	sdp_uuid16_create(&avdtp, AVDTP_UUID);
	proto[1] = sdp_list_append(0, &avdtp);
	version = sdp_data_alloc(SDP_UINT16, &ver);
	proto[1] = sdp_list_append(proto[1], version);
	apseq = sdp_list_append(apseq, proto[1]);

	aproto = sdp_list_append(0, apseq);
	sdp_set_access_protos(&record, aproto);

	sdp_set_info_attr(&record, "Audio Source", 0, 0);

	if (sdp_device_record_register(session, &interface, &record, SDP_RECORD_PERSIST) < 0) {
		printf("Service Record registration failed\n");
		ret = -1;
		goto done;
	}

	printf("Audio source service registered\n");

done:
	sdp_list_free(proto[0], 0);
	sdp_list_free(proto[1], 0);
	sdp_list_free(apseq, 0);
	sdp_list_free(aproto, 0);

	return ret;
}

static int add_a2sink(sdp_session_t *session, svc_info_t *si)
{
	sdp_list_t *svclass_id, *pfseq, *apseq, *root;
	uuid_t root_uuid, l2cap, avdtp, a2snk;
	sdp_profile_desc_t profile[1];
	sdp_list_t *aproto, *proto[2];
	sdp_record_t record;
	sdp_data_t *psm, *version;
	uint16_t lp = 0x0019, ver = 0x0100;
	int ret = 0;

	memset(&record, 0, sizeof(sdp_record_t));
	record.handle = si->handle;

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(0, &root_uuid);
	sdp_set_browse_groups(&record, root);

	sdp_uuid16_create(&a2snk, AUDIO_SINK_SVCLASS_ID);
	svclass_id = sdp_list_append(0, &a2snk);
	sdp_set_service_classes(&record, svclass_id);

	sdp_uuid16_create(&profile[0].uuid, ADVANCED_AUDIO_PROFILE_ID);
	profile[0].version = 0x0100;
	pfseq = sdp_list_append(0, &profile[0]);
	sdp_set_profile_descs(&record, pfseq);

	sdp_uuid16_create(&l2cap, L2CAP_UUID);
	proto[0] = sdp_list_append(0, &l2cap);
	psm = sdp_data_alloc(SDP_UINT16, &lp);
	proto[0] = sdp_list_append(proto[0], psm);
	apseq = sdp_list_append(0, proto[0]);

	sdp_uuid16_create(&avdtp, AVDTP_UUID);
	proto[1] = sdp_list_append(0, &avdtp);
	version = sdp_data_alloc(SDP_UINT16, &ver);
	proto[1] = sdp_list_append(proto[1], version);
	apseq = sdp_list_append(apseq, proto[1]);

	aproto = sdp_list_append(0, apseq);
	sdp_set_access_protos(&record, aproto);

	sdp_set_info_attr(&record, "Audio Sink", 0, 0);

	if (sdp_device_record_register(session, &interface, &record, SDP_RECORD_PERSIST) < 0) {
		printf("Service Record registration failed\n");
		ret = -1;
		goto done;
	}

	printf("Audio sink service registered\n");

done:
	sdp_list_free(proto[0], 0);
	sdp_list_free(proto[1], 0);
	sdp_list_free(apseq, 0);
	sdp_list_free(aproto, 0);

	return ret;
}

static int add_avrct(sdp_session_t *session, svc_info_t *si)
{
	sdp_list_t *svclass_id, *pfseq, *apseq, *root;
	uuid_t root_uuid, l2cap, avctp, avrct;
	sdp_profile_desc_t profile[1];
	sdp_list_t *aproto, *proto[2];
	sdp_record_t record;
	sdp_data_t *psm, *version, *features;
	uint16_t lp = 0x0017, ver = 0x0100, feat = 0x000f;
	int ret = 0;

	memset(&record, 0, sizeof(sdp_record_t));
	record.handle = si->handle;

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(0, &root_uuid);
	sdp_set_browse_groups(&record, root);

	sdp_uuid16_create(&avrct, AV_REMOTE_SVCLASS_ID);
	svclass_id = sdp_list_append(0, &avrct);
	sdp_set_service_classes(&record, svclass_id);

	sdp_uuid16_create(&profile[0].uuid, AV_REMOTE_PROFILE_ID);
	profile[0].version = 0x0100;
	pfseq = sdp_list_append(0, &profile[0]);
	sdp_set_profile_descs(&record, pfseq);

	sdp_uuid16_create(&l2cap, L2CAP_UUID);
	proto[0] = sdp_list_append(0, &l2cap);
	psm = sdp_data_alloc(SDP_UINT16, &lp);
	proto[0] = sdp_list_append(proto[0], psm);
	apseq = sdp_list_append(0, proto[0]);

	sdp_uuid16_create(&avctp, AVCTP_UUID);
	proto[1] = sdp_list_append(0, &avctp);
	version = sdp_data_alloc(SDP_UINT16, &ver);
	proto[1] = sdp_list_append(proto[1], version);
	apseq = sdp_list_append(apseq, proto[1]);

	aproto = sdp_list_append(0, apseq);
	sdp_set_access_protos(&record, aproto);

	features = sdp_data_alloc(SDP_UINT16, &feat);
	sdp_attr_add(&record, SDP_ATTR_SUPPORTED_FEATURES, features);

	sdp_set_info_attr(&record, "AVRCP CT", 0, 0);

	if (sdp_device_record_register(session, &interface, &record, SDP_RECORD_PERSIST) < 0) {
		printf("Service Record registration failed\n");
		ret = -1;
		goto done;
	}

	printf("Remote control service registered\n");

done:
	sdp_list_free(proto[0], 0);
	sdp_list_free(proto[1], 0);
	sdp_list_free(apseq, 0);
	sdp_list_free(aproto, 0);

	return ret;
}

static int add_avrtg(sdp_session_t *session, svc_info_t *si)
{
	sdp_list_t *svclass_id, *pfseq, *apseq, *root;
	uuid_t root_uuid, l2cap, avctp, avrtg;
	sdp_profile_desc_t profile[1];
	sdp_list_t *aproto, *proto[2];
	sdp_record_t record;
	sdp_data_t *psm, *version, *features;
	uint16_t lp = 0x0017, ver = 0x0100, feat = 0x000f;
	int ret = 0;

	memset(&record, 0, sizeof(sdp_record_t));
	record.handle = si->handle;

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(0, &root_uuid);
	sdp_set_browse_groups(&record, root);

	sdp_uuid16_create(&avrtg, AV_REMOTE_TARGET_SVCLASS_ID);
	svclass_id = sdp_list_append(0, &avrtg);
	sdp_set_service_classes(&record, svclass_id);

	sdp_uuid16_create(&profile[0].uuid, AV_REMOTE_PROFILE_ID);
	profile[0].version = 0x0100;
	pfseq = sdp_list_append(0, &profile[0]);
	sdp_set_profile_descs(&record, pfseq);

	sdp_uuid16_create(&l2cap, L2CAP_UUID);
	proto[0] = sdp_list_append(0, &l2cap);
	psm = sdp_data_alloc(SDP_UINT16, &lp);
	proto[0] = sdp_list_append(proto[0], psm);
	apseq = sdp_list_append(0, proto[0]);

	sdp_uuid16_create(&avctp, AVCTP_UUID);
	proto[1] = sdp_list_append(0, &avctp);
	version = sdp_data_alloc(SDP_UINT16, &ver);
	proto[1] = sdp_list_append(proto[1], version);
	apseq = sdp_list_append(apseq, proto[1]);

	aproto = sdp_list_append(0, apseq);
	sdp_set_access_protos(&record, aproto);

	features = sdp_data_alloc(SDP_UINT16, &feat);
	sdp_attr_add(&record, SDP_ATTR_SUPPORTED_FEATURES, features);

	sdp_set_info_attr(&record, "AVRCP TG", 0, 0);

	if (sdp_device_record_register(session, &interface, &record, SDP_RECORD_PERSIST) < 0) {
		printf("Service Record registration failed\n");
		ret = -1;
		goto done;
	}

	printf("Remote target service registered\n");

done:
	sdp_list_free(proto[0], 0);
	sdp_list_free(proto[1], 0);
	sdp_list_free(apseq, 0);
	sdp_list_free(aproto, 0);

	return ret;
}

static int add_udi_ue(sdp_session_t *session, svc_info_t *si)
{
	sdp_record_t record;
	sdp_list_t *root, *svclass, *proto;
	uuid_t root_uuid, svclass_uuid, l2cap_uuid, rfcomm_uuid;
	uint8_t channel = si->channel ? si->channel: 18;

	memset(&record, 0, sizeof(record));
	record.handle = si->handle;

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(NULL, &root_uuid);
	sdp_set_browse_groups(&record, root);
	sdp_list_free(root, NULL);

	sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
	proto = sdp_list_append(NULL, sdp_list_append(NULL, &l2cap_uuid));

	sdp_uuid16_create(&rfcomm_uuid, RFCOMM_UUID);
	proto = sdp_list_append(proto, sdp_list_append(
		sdp_list_append(NULL, &rfcomm_uuid), sdp_data_alloc(SDP_UINT8, &channel)));

	sdp_set_access_protos(&record, sdp_list_append(NULL, proto));

	sdp_uuid16_create(&svclass_uuid, UDI_MT_SVCLASS_ID);
	svclass = sdp_list_append(NULL, &svclass_uuid);
	sdp_set_service_classes(&record, svclass);
	sdp_list_free(svclass, NULL);

	sdp_set_info_attr(&record, "UDI UE", NULL, NULL);

	if (sdp_device_record_register(session, &interface, &record, SDP_RECORD_PERSIST) < 0) {
		printf("Service Record registration failed\n");
		return -1;
	}

	printf("UDI UE service registered\n");

	return 0;
}

static int add_udi_te(sdp_session_t *session, svc_info_t *si)
{
	sdp_record_t record;
	sdp_list_t *root, *svclass, *proto;
	uuid_t root_uuid, svclass_uuid, l2cap_uuid, rfcomm_uuid;
	uint8_t channel = si->channel ? si->channel: 19;

	memset(&record, 0, sizeof(record));
	record.handle = si->handle;

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(NULL, &root_uuid);
	sdp_set_browse_groups(&record, root);
	sdp_list_free(root, NULL);

	sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
	proto = sdp_list_append(NULL, sdp_list_append(NULL, &l2cap_uuid));

	sdp_uuid16_create(&rfcomm_uuid, RFCOMM_UUID);
	proto = sdp_list_append(proto, sdp_list_append(
		sdp_list_append(NULL, &rfcomm_uuid), sdp_data_alloc(SDP_UINT8, &channel)));

	sdp_set_access_protos(&record, sdp_list_append(NULL, proto));

	sdp_uuid16_create(&svclass_uuid, UDI_TA_SVCLASS_ID);
	svclass = sdp_list_append(NULL, &svclass_uuid);
	sdp_set_service_classes(&record, svclass);
	sdp_list_free(svclass, NULL);

	sdp_set_info_attr(&record, "UDI TE", NULL, NULL);

	if (sdp_device_record_register(session, &interface, &record, SDP_RECORD_PERSIST) < 0) {
		printf("Service Record registration failed\n");
		return -1;
	}

	printf("UDI TE service registered\n");

	return 0;
}

static unsigned char sr1_uuid[] = {	0xbc, 0x19, 0x9c, 0x24, 0x95, 0x8b, 0x4c, 0xc0,
					0xa2, 0xcb, 0xfd, 0x8a, 0x30, 0xbf, 0x32, 0x06 };

static int add_sr1(sdp_session_t *session, svc_info_t *si)
{
	sdp_record_t record;
	sdp_list_t *root, *svclass;
	uuid_t root_uuid, svclass_uuid;

	memset(&record, 0, sizeof(record));
	record.handle = si->handle;

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(NULL, &root_uuid);
	sdp_set_browse_groups(&record, root);

	sdp_uuid128_create(&svclass_uuid, (void *) sr1_uuid);
	svclass = sdp_list_append(NULL, &svclass_uuid);
	sdp_set_service_classes(&record, svclass);

	sdp_set_info_attr(&record, "TOSHIBA SR-1", NULL, NULL);

	if (sdp_device_record_register(session, &interface, &record, SDP_RECORD_PERSIST) < 0) {
		printf("Service Record registration failed\n");
		return -1;
	}

	printf("Toshiba Speech Recognition SR-1 service record registered\n");

	return 0;
}

static unsigned char syncmls_uuid[] = {	0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x10, 0x00,
					0x80, 0x00, 0x00, 0x02, 0xEE, 0x00, 0x00, 0x02 };

static unsigned char syncmlc_uuid[] = {	0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x10, 0x00,
					0x80, 0x00, 0x00, 0x02, 0xEE, 0x00, 0x00, 0x02 };

static int add_syncml(sdp_session_t *session, svc_info_t *si)
{
	sdp_record_t record;
	sdp_list_t *root, *svclass, *proto;
	uuid_t root_uuid, svclass_uuid, l2cap_uuid, rfcomm_uuid, obex_uuid;
	uint8_t channel = si->channel ? si->channel: 15;

	memset(&record, 0, sizeof(record));
	record.handle = si->handle;

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(NULL, &root_uuid);
	sdp_set_browse_groups(&record, root);

	sdp_uuid128_create(&svclass_uuid, (void *) syncmlc_uuid);
	svclass = sdp_list_append(NULL, &svclass_uuid);
	sdp_set_service_classes(&record, svclass);

	sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
	proto = sdp_list_append(NULL, sdp_list_append(NULL, &l2cap_uuid));

	sdp_uuid16_create(&rfcomm_uuid, RFCOMM_UUID);
	proto = sdp_list_append(proto, sdp_list_append(
		sdp_list_append(NULL, &rfcomm_uuid), sdp_data_alloc(SDP_UINT8, &channel)));

	sdp_uuid16_create(&obex_uuid, OBEX_UUID);
	proto = sdp_list_append(proto, sdp_list_append(NULL, &obex_uuid));

	sdp_set_access_protos(&record, sdp_list_append(NULL, proto));

	sdp_set_info_attr(&record, "SyncML Client", NULL, NULL);

	if (sdp_device_record_register(session, &interface, &record, SDP_RECORD_PERSIST) < 0) {
		printf("Service Record registration failed\n");
		return -1;
	}

	printf("SyncML Client service record registered\n");

	return 0;
}

static unsigned char async_uuid[] = {	0x03, 0x50, 0x27, 0x8F, 0x3D, 0xCA, 0x4E, 0x62,
					0x83, 0x1D, 0xA4, 0x11, 0x65, 0xFF, 0x90, 0x6C };

static int add_activesync(sdp_session_t *session, svc_info_t *si)
{
	sdp_record_t record;
	sdp_list_t *root, *svclass, *proto;
	uuid_t root_uuid, svclass_uuid, l2cap_uuid, rfcomm_uuid;
	uint8_t channel = si->channel ? si->channel: 21;

	memset(&record, 0, sizeof(record));
	record.handle = si->handle;

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(NULL, &root_uuid);
	sdp_set_browse_groups(&record, root);

	sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
	proto = sdp_list_append(NULL, sdp_list_append(NULL, &l2cap_uuid));

	sdp_uuid16_create(&rfcomm_uuid, RFCOMM_UUID);
	proto = sdp_list_append(proto, sdp_list_append(
	sdp_list_append(NULL, &rfcomm_uuid), sdp_data_alloc(SDP_UINT8, &channel)));

	sdp_set_access_protos(&record, sdp_list_append(NULL, proto));

	sdp_uuid128_create(&svclass_uuid, (void *) async_uuid);
	svclass = sdp_list_append(NULL, &svclass_uuid);
	sdp_set_service_classes(&record, svclass);

	sdp_set_info_attr(&record, "Microsoft ActiveSync", NULL, NULL);

	if (sdp_device_record_register(session, &interface, &record, SDP_RECORD_PERSIST) < 0) {
		printf("Service Record registration failed\n");
		return -1;
	}

	printf("ActiveSync service record registered\n");

	return 0;
}

static unsigned char hotsync_uuid[] = {	0xD8, 0x0C, 0xF9, 0xEA, 0x13, 0x4C, 0x11, 0xD5,
					0x83, 0xCE, 0x00, 0x30, 0x65, 0x7C, 0x54, 0x3C };

static int add_hotsync(sdp_session_t *session, svc_info_t *si)
{
	sdp_record_t record;
	sdp_list_t *root, *svclass, *proto;
	uuid_t root_uuid, svclass_uuid, l2cap_uuid, rfcomm_uuid;
	uint8_t channel = si->channel ? si->channel: 22;

	memset(&record, 0, sizeof(record));
	record.handle = si->handle;

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(NULL, &root_uuid);
	sdp_set_browse_groups(&record, root);

	sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
	proto = sdp_list_append(NULL, sdp_list_append(NULL, &l2cap_uuid));

	sdp_uuid16_create(&rfcomm_uuid, RFCOMM_UUID);
	proto = sdp_list_append(proto, sdp_list_append(
	sdp_list_append(NULL, &rfcomm_uuid), sdp_data_alloc(SDP_UINT8, &channel)));

	sdp_set_access_protos(&record, sdp_list_append(NULL, proto));

	sdp_uuid128_create(&svclass_uuid, (void *) hotsync_uuid);
	svclass = sdp_list_append(NULL, &svclass_uuid);
	sdp_set_service_classes(&record, svclass);

	sdp_set_info_attr(&record, "PalmOS HotSync", NULL, NULL);

	if (sdp_device_record_register(session, &interface, &record, SDP_RECORD_PERSIST) < 0) {
		printf("Service Record registration failed\n");
		return -1;
	}

	printf("HotSync service record registered\n");

	return 0;
}

static unsigned char palmos_uuid[] = {	0xF5, 0xBE, 0xB6, 0x51, 0x41, 0x71, 0x40, 0x51,
					0xAC, 0xF5, 0x6C, 0xA7, 0x20, 0x22, 0x42, 0xF0 };

static int add_palmos(sdp_session_t *session, svc_info_t *si)
{
	sdp_record_t record;
	sdp_list_t *root, *svclass;
	uuid_t root_uuid, svclass_uuid;
	int err;

	memset(&record, 0, sizeof(record));
	record.handle = si->handle;

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(NULL, &root_uuid);
	sdp_set_browse_groups(&record, root);

	sdp_uuid128_create(&svclass_uuid, (void *) palmos_uuid);
	svclass = sdp_list_append(NULL, &svclass_uuid);
	sdp_set_service_classes(&record, svclass);

	err = sdp_device_record_register(session, &interface, &record,
							SDP_RECORD_PERSIST);
	sdp_list_free(root, NULL);
	sdp_list_free(svclass, NULL);

	if (err < 0) {
		printf("Service Record registration failed\n");
		return -1;
	}

	printf("PalmOS service record registered\n");

	return 0;
}

static unsigned char nokid_uuid[] = {	0x00, 0x00, 0x55, 0x55, 0x00, 0x00, 0x10, 0x00,
					0x80, 0x00, 0x00, 0x02, 0xEE, 0x00, 0x00, 0x01 };

static int add_nokiaid(sdp_session_t *session, svc_info_t *si)
{
	sdp_record_t record;
	sdp_list_t *root, *svclass;
	uuid_t root_uuid, svclass_uuid;
	uint16_t verid = 0x005f;
	sdp_data_t *version = sdp_data_alloc(SDP_UINT16, &verid);

	memset(&record, 0, sizeof(record));
	record.handle = si->handle;

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(NULL, &root_uuid);
	sdp_set_browse_groups(&record, root);

	sdp_uuid128_create(&svclass_uuid, (void *) nokid_uuid);
	svclass = sdp_list_append(NULL, &svclass_uuid);
	sdp_set_service_classes(&record, svclass);

	sdp_attr_add(&record, SDP_ATTR_SERVICE_VERSION, version);

	if (sdp_device_record_register(session, &interface, &record, SDP_RECORD_PERSIST) < 0) {
		printf("Service Record registration failed\n");
		sdp_data_free(version);
		return -1;
	}

	printf("Nokia ID service record registered\n");

	return 0;
}

static unsigned char pcsuite_uuid[] = {	0x00, 0x00, 0x50, 0x02, 0x00, 0x00, 0x10, 0x00,
					0x80, 0x00, 0x00, 0x02, 0xEE, 0x00, 0x00, 0x01 };

static int add_pcsuite(sdp_session_t *session, svc_info_t *si)
{
	sdp_record_t record;
	sdp_list_t *root, *svclass, *proto;
	uuid_t root_uuid, svclass_uuid, l2cap_uuid, rfcomm_uuid;
	uint8_t channel = si->channel ? si->channel: 14;

	memset(&record, 0, sizeof(record));
	record.handle = si->handle;

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(NULL, &root_uuid);
	sdp_set_browse_groups(&record, root);

	sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
	proto = sdp_list_append(NULL, sdp_list_append(NULL, &l2cap_uuid));

	sdp_uuid16_create(&rfcomm_uuid, RFCOMM_UUID);
	proto = sdp_list_append(proto, sdp_list_append(
		sdp_list_append(NULL, &rfcomm_uuid), sdp_data_alloc(SDP_UINT8, &channel)));

	sdp_set_access_protos(&record, sdp_list_append(NULL, proto));

	sdp_uuid128_create(&svclass_uuid, (void *) pcsuite_uuid);
	svclass = sdp_list_append(NULL, &svclass_uuid);
	sdp_set_service_classes(&record, svclass);

	sdp_set_info_attr(&record, "Nokia PC Suite", NULL, NULL);

	if (sdp_device_record_register(session, &interface, &record, SDP_RECORD_PERSIST) < 0) {
		printf("Service Record registration failed\n");
		return -1;
	}

	printf("Nokia PC Suite service registered\n");

	return 0;
}

static unsigned char nftp_uuid[] = {	0x00, 0x00, 0x50, 0x05, 0x00, 0x00, 0x10, 0x00,
					0x80, 0x00, 0x00, 0x02, 0xEE, 0x00, 0x00, 0x01 };

static unsigned char nsyncml_uuid[] = {	0x00, 0x00, 0x56, 0x01, 0x00, 0x00, 0x10, 0x00,
					0x80, 0x00, 0x00, 0x02, 0xEE, 0x00, 0x00, 0x01 };

static unsigned char ngage_uuid[] = {	0x00, 0x00, 0x13, 0x01, 0x00, 0x00, 0x10, 0x00,
					0x80, 0x00, 0x00, 0x02, 0xEE, 0x00, 0x00, 0x01 };

static unsigned char apple_uuid[] = {	0xf0, 0x72, 0x2e, 0x20, 0x0f, 0x8b, 0x4e, 0x90,
					0x8c, 0xc2, 0x1b, 0x46, 0xf5, 0xf2, 0xef, 0xe2 };

static unsigned char iap_uuid[] = {	0x00, 0x00, 0x00, 0x00, 0xde, 0xca, 0xfa, 0xde,
					0xde, 0xca, 0xde, 0xaf, 0xde, 0xca, 0xca, 0xfe };

static int add_apple(sdp_session_t *session, svc_info_t *si)
{
	sdp_record_t record;
	sdp_list_t *root;
	uuid_t root_uuid;
	uint32_t attr783 = 0x00000000;
	uint32_t attr785 = 0x00000002;
	uint16_t attr786 = 0x1234;

	memset(&record, 0, sizeof(record));
	record.handle = si->handle;

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(NULL, &root_uuid);
	sdp_set_browse_groups(&record, root);

	sdp_attr_add_new(&record, 0x0780, SDP_UUID128, (void *) apple_uuid);
	sdp_attr_add_new(&record, 0x0781, SDP_TEXT_STR8, (void *) "Macmini");
	sdp_attr_add_new(&record, 0x0782, SDP_TEXT_STR8, (void *) "PowerMac10,1");
	sdp_attr_add_new(&record, 0x0783, SDP_UINT32, (void *) &attr783);
	sdp_attr_add_new(&record, 0x0784, SDP_TEXT_STR8, (void *) "1.6.6f22");
	sdp_attr_add_new(&record, 0x0785, SDP_UINT32, (void *) &attr785);
	sdp_attr_add_new(&record, 0x0786, SDP_UUID16, (void *) &attr786);

	sdp_set_info_attr(&record, "Apple Macintosh Attributes", NULL, NULL);

	if (sdp_device_record_register(session, &interface, &record, SDP_RECORD_PERSIST) < 0) {
		printf("Service Record registration failed\n");
		return -1;
	}

	printf("Apple attribute service registered\n");

	return 0;
}

static int add_isync(sdp_session_t *session, svc_info_t *si)
{
	sdp_record_t record;
	sdp_list_t *root, *svclass, *proto;
	uuid_t root_uuid, svclass_uuid, serial_uuid, l2cap_uuid, rfcomm_uuid;
	uint8_t channel = si->channel ? si->channel : 16;

	memset(&record, 0, sizeof(record));
	record.handle = si->handle;

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(NULL, &root_uuid);
	sdp_set_browse_groups(&record, root);

	sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
	proto = sdp_list_append(NULL, sdp_list_append(NULL, &l2cap_uuid));

	sdp_uuid16_create(&rfcomm_uuid, RFCOMM_UUID);
	proto = sdp_list_append(proto, sdp_list_append(
		sdp_list_append(NULL, &rfcomm_uuid), sdp_data_alloc(SDP_UINT8, &channel)));

	sdp_set_access_protos(&record, sdp_list_append(NULL, proto));

	sdp_uuid16_create(&serial_uuid, SERIAL_PORT_SVCLASS_ID);
	svclass = sdp_list_append(NULL, &serial_uuid);

	sdp_uuid16_create(&svclass_uuid, APPLE_AGENT_SVCLASS_ID);
	svclass = sdp_list_append(svclass, &svclass_uuid);

	sdp_set_service_classes(&record, svclass);

	sdp_set_info_attr(&record, "AppleAgent", "Bluetooth acceptor", "Apple Computer Ltd.");

	if (sdp_device_record_register(session, &interface, &record, SDP_RECORD_PERSIST) < 0) {
		printf("Service Record registration failed\n");
		return -1;
	}

	printf("Apple iSync service registered\n");

	return 0;
}

static int add_semchla(sdp_session_t *session, svc_info_t *si)
{
	sdp_record_t record;
	sdp_profile_desc_t profile;
	sdp_list_t *root, *svclass, *proto, *profiles;
	uuid_t root_uuid, service_uuid, l2cap_uuid, semchla_uuid;
	uint16_t psm = 0xf0f9;

	memset(&record, 0, sizeof(record));
	record.handle = si->handle;

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(NULL, &root_uuid);
	sdp_set_browse_groups(&record, root);

	sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
	proto = sdp_list_append(NULL, sdp_list_append(
		sdp_list_append(NULL, &l2cap_uuid), sdp_data_alloc(SDP_UINT16, &psm)));

	sdp_uuid32_create(&semchla_uuid, 0x8e770300);
	proto = sdp_list_append(proto, sdp_list_append(NULL, &semchla_uuid));

	sdp_set_access_protos(&record, sdp_list_append(NULL, proto));

	sdp_uuid32_create(&service_uuid, 0x8e771301);
	svclass = sdp_list_append(NULL, &service_uuid);

	sdp_set_service_classes(&record, svclass);

	sdp_uuid32_create(&profile.uuid, 0x8e771302);	// Headset
	//sdp_uuid32_create(&profile.uuid, 0x8e771303);	// Phone
	profile.version = 0x0100;
	profiles = sdp_list_append(NULL, &profile);
	sdp_set_profile_descs(&record, profiles);

	sdp_set_info_attr(&record, "SEMC HLA", NULL, NULL);

	if (sdp_device_record_register(session, &interface, &record, SDP_RECORD_PERSIST) < 0) {
		printf("Service Record registration failed\n");
		return -1;
	}

	/* SEMC High Level Authentication */
	printf("SEMC HLA service registered\n");

	return 0;
}

static int add_gatt(sdp_session_t *session, svc_info_t *si)
{
	sdp_list_t *svclass_id, *apseq, *proto[2], *profiles, *root, *aproto;
	uuid_t root_uuid, proto_uuid, gatt_uuid, l2cap;
	sdp_profile_desc_t profile;
	sdp_record_t record;
	sdp_data_t *psm, *sh, *eh;
	uint16_t att_psm = 27, start = 0x0001, end = 0x000f;
	int ret;

	memset(&record, 0, sizeof(sdp_record_t));
	record.handle = si->handle;
	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(NULL, &root_uuid);
	sdp_set_browse_groups(&record, root);
	sdp_list_free(root, NULL);

	sdp_uuid16_create(&gatt_uuid, GENERIC_ATTRIB_SVCLASS_ID);
	svclass_id = sdp_list_append(NULL, &gatt_uuid);
	sdp_set_service_classes(&record, svclass_id);
	sdp_list_free(svclass_id, NULL);

	sdp_uuid16_create(&profile.uuid, GENERIC_ATTRIB_PROFILE_ID);
	profile.version = 0x0100;
	profiles = sdp_list_append(NULL, &profile);
	sdp_set_profile_descs(&record, profiles);
	sdp_list_free(profiles, NULL);

	sdp_uuid16_create(&l2cap, L2CAP_UUID);
	proto[0] = sdp_list_append(NULL, &l2cap);
	psm = sdp_data_alloc(SDP_UINT16, &att_psm);
	proto[0] = sdp_list_append(proto[0], psm);
	apseq = sdp_list_append(NULL, proto[0]);

	sdp_uuid16_create(&proto_uuid, ATT_UUID);
	proto[1] = sdp_list_append(NULL, &proto_uuid);
	sh = sdp_data_alloc(SDP_UINT16, &start);
	proto[1] = sdp_list_append(proto[1], sh);
	eh = sdp_data_alloc(SDP_UINT16, &end);
	proto[1] = sdp_list_append(proto[1], eh);
	apseq = sdp_list_append(apseq, proto[1]);

	aproto = sdp_list_append(NULL, apseq);
	sdp_set_access_protos(&record, aproto);

	sdp_set_info_attr(&record, "Generic Attribute Profile", "BlueZ", NULL);

	sdp_set_url_attr(&record, "http://www.bluez.org/",
			"http://www.bluez.org/", "http://www.bluez.org/");

	sdp_set_service_id(&record, gatt_uuid);

	ret = sdp_device_record_register(session, &interface, &record,
							SDP_RECORD_PERSIST);
	if (ret < 0)
		printf("Service Record registration failed\n");
	else
		printf("Generic Attribute Profile Service registered\n");

	sdp_data_free(psm);
	sdp_data_free(sh);
	sdp_data_free(eh);
	sdp_list_free(proto[0], NULL);
	sdp_list_free(proto[1], NULL);
	sdp_list_free(apseq, NULL);
	sdp_list_free(aproto, NULL);

	return ret;
}

struct {
	char		*name;
	uint32_t	class;
	int		(*add)(sdp_session_t *sess, svc_info_t *si);
	unsigned char *uuid;
} service[] = {
	{ "DID",	PNP_INFO_SVCLASS_ID,		NULL,		},

	{ "SP",		SERIAL_PORT_SVCLASS_ID,		add_sp		},
	{ "DUN",	DIALUP_NET_SVCLASS_ID,		add_dun		},
	{ "LAN",	LAN_ACCESS_SVCLASS_ID,		add_lan		},
	{ "FAX",	FAX_SVCLASS_ID,			add_fax		},
	{ "OPUSH",	OBEX_OBJPUSH_SVCLASS_ID,	add_opush	},
	{ "FTP",	OBEX_FILETRANS_SVCLASS_ID,	add_ftp		},
	{ "PRINT",	DIRECT_PRINTING_SVCLASS_ID,	add_directprint	},

	{ "HS",		HEADSET_SVCLASS_ID,		add_headset	},
	{ "HSAG",	HEADSET_AGW_SVCLASS_ID,		add_headset_ag	},
	{ "HF",		HANDSFREE_SVCLASS_ID,		add_handsfree	},
	{ "HFAG",	HANDSFREE_AGW_SVCLASS_ID,	add_handsfree_ag},
	{ "SAP",	SAP_SVCLASS_ID,			add_simaccess	},
	{ "PBAP",	PBAP_SVCLASS_ID,		add_pbap,	},

	{ "MAP",	MAP_SVCLASS_ID,			add_map,	},
	{ "NAP",	NAP_SVCLASS_ID,			add_nap		},
	{ "GN",		GN_SVCLASS_ID,			add_gn		},
	{ "PANU",	PANU_SVCLASS_ID,		add_panu	},

	{ "HCRP",	HCR_SVCLASS_ID,			NULL		},
	{ "HID",	HID_SVCLASS_ID,			NULL		},
	{ "KEYB",	HID_SVCLASS_ID,			add_hid_keyb	},
	{ "WIIMOTE",	HID_SVCLASS_ID,			add_hid_wiimote	},
	{ "CIP",	CIP_SVCLASS_ID,			add_cip		},
	{ "CTP",	CORDLESS_TELEPHONY_SVCLASS_ID,	add_ctp		},

	{ "A2SRC",	AUDIO_SOURCE_SVCLASS_ID,	add_a2source	},
	{ "A2SNK",	AUDIO_SINK_SVCLASS_ID,		add_a2sink	},
	{ "AVRCT",	AV_REMOTE_SVCLASS_ID,		add_avrct	},
	{ "AVRTG",	AV_REMOTE_TARGET_SVCLASS_ID,	add_avrtg	},

	{ "UDIUE",	UDI_MT_SVCLASS_ID,		add_udi_ue	},
	{ "UDITE",	UDI_TA_SVCLASS_ID,		add_udi_te	},

	{ "SEMCHLA",	0x8e771301,			add_semchla	},

	{ "SR1",	0,				add_sr1,	sr1_uuid	},
	{ "SYNCML",	0,				add_syncml,	syncmlc_uuid	},
	{ "SYNCMLSERV",	0,				NULL,		syncmls_uuid	},
	{ "ACTIVESYNC",	0,				add_activesync,	async_uuid	},
	{ "HOTSYNC",	0,				add_hotsync,	hotsync_uuid	},
	{ "PALMOS",	0,				add_palmos,	palmos_uuid	},
	{ "NOKID",	0,				add_nokiaid,	nokid_uuid	},
	{ "PCSUITE",	0,				add_pcsuite,	pcsuite_uuid	},
	{ "NFTP",	0,				NULL,		nftp_uuid	},
	{ "NSYNCML",	0,				NULL,		nsyncml_uuid	},
	{ "NGAGE",	0,				NULL,		ngage_uuid	},
	{ "APPLE",	0,				add_apple,	apple_uuid	},
	{ "IAP",	0,				NULL,		iap_uuid	},

	{ "ISYNC",	APPLE_AGENT_SVCLASS_ID,		add_isync,	},
	{ "GATT",	GENERIC_ATTRIB_SVCLASS_ID,	add_gatt,	},

	{ 0 }
};

/* Add local service */
static int add_service(bdaddr_t *bdaddr, svc_info_t *si)
{
	sdp_session_t *sess;
	int i, ret = -1;

	if (!si->name)
		return -1;

	sess = sdp_connect(&interface, BDADDR_LOCAL, SDP_RETRY_IF_BUSY);
	if (!sess)
		return -1;

	for (i = 0; service[i].name; i++)
		if (!strcasecmp(service[i].name, si->name)) {
			if (service[i].add)
				ret = service[i].add(sess, si);
			goto done;
		}

	printf("Unknown service name: %s\n", si->name);

done:
	free(si->name);
	sdp_close(sess);

	return ret;
}

static struct option add_options[] = {
	{ "help",	0, 0, 'h' },
	{ "handle",	1, 0, 'r' },
	{ "psm",	1, 0, 'p' },
	{ "channel",	1, 0, 'c' },
	{ "network",	1, 0, 'n' },
	{ 0, 0, 0, 0 }
};

static const char *add_help =
	"Usage:\n"
	"\tadd [--handle=RECORD_HANDLE --channel=CHANNEL] service\n";

static int cmd_add(int argc, char **argv)
{
	svc_info_t si;
	int opt;

	memset(&si, 0, sizeof(si));
	si.handle = 0xffffffff;

	for_each_opt(opt, add_options, 0) {
		switch (opt) {
		case 'r':
			if (strncasecmp(optarg, "0x", 2))
				si.handle = atoi(optarg);
			else
				si.handle = strtol(optarg + 2, NULL, 16);
			break;
		case 'p':
			if (strncasecmp(optarg, "0x", 2))
				si.psm = atoi(optarg);
			else
				si.psm = strtol(optarg + 2, NULL, 16);
			break;
		case 'c':
			if (strncasecmp(optarg, "0x", 2))
				si.channel = atoi(optarg);
			else
				si.channel = strtol(optarg + 2, NULL, 16);
			break;
		case 'n':
			if (strncasecmp(optarg, "0x", 2))
				si.network = atoi(optarg);
			else
				si.network = strtol(optarg + 2, NULL, 16);
			break;
		default:
			printf("%s", add_help);
			return -1;
		}
	}

	argc -= optind;
	argv += optind;

	if (argc < 1) {
		printf("%s", add_help);
		return -1;
	}

	si.name = strdup(argv[0]);

	return add_service(0, &si);
}

/* Delete local service */
static int del_service(bdaddr_t *bdaddr, void *arg)
{
	uint32_t handle, range = 0x0000ffff;
	sdp_list_t *attr;
	sdp_session_t *sess;
	sdp_record_t *rec;

	if (!arg) {
		printf("Record handle was not specified.\n");
		return -1;
	}

	sess = sdp_connect(&interface, BDADDR_LOCAL, SDP_RETRY_IF_BUSY);
	if (!sess) {
		printf("No local SDP server!\n");
		return -1;
	}

	handle = strtoul((char *)arg, 0, 16);
	attr = sdp_list_append(0, &range);
	rec = sdp_service_attr_req(sess, handle, SDP_ATTR_REQ_RANGE, attr);
	sdp_list_free(attr, 0);

	if (!rec) {
		printf("Service Record not found.\n");
		sdp_close(sess);
		return -1;
	}

	if (sdp_device_record_unregister(sess, &interface, rec)) {
		printf("Failed to unregister service record: %s\n", strerror(errno));
		sdp_close(sess);
		return -1;
	}

	printf("Service Record deleted.\n");
	sdp_close(sess);

	return 0;
}

static struct option del_options[] = {
	{ "help",	0, 0, 'h' },
	{ 0, 0, 0, 0 }
};

static const char *del_help =
	"Usage:\n"
	"\tdel record_handle\n";

static int cmd_del(int argc, char **argv)
{
	int opt;

	for_each_opt(opt, del_options, 0) {
		switch (opt) {
		default:
			printf("%s", del_help);
			return -1;
		}
	}

	argc -= optind;
	argv += optind;

	if (argc < 1) {
		printf("%s", del_help);
		return -1;
	}

	return del_service(NULL, argv[0]);
}

/*
 * Perform an inquiry and search/browse all peer found.
 */
static void inquiry(handler_t handler, void *arg)
{
	inquiry_info ii[20];
	uint8_t count = 0;
	int i;

	printf("Inquiring ...\n");
	if (sdp_general_inquiry(ii, 20, 8, &count) < 0) {
		printf("Inquiry failed\n");
		return;
	}

	for (i = 0; i < count; i++)
		handler(&ii[i].bdaddr, arg);
}

static void doprintf(void *data, const char *str)
{
	printf("%s", str);
}

/*
 * Search for a specific SDP service
 */
static int do_search(bdaddr_t *bdaddr, struct search_context *context)
{
	sdp_list_t *attrid, *search, *seq, *next;
	uint32_t range = 0x0000ffff;
	char str[20];
	sdp_session_t *sess;

	if (!bdaddr) {
		inquiry(do_search, context);
		return 0;
	}

	sess = sdp_connect(&interface, bdaddr, SDP_RETRY_IF_BUSY);
	ba2str(bdaddr, str);
	if (!sess) {
		printf("Failed to connect to SDP server on %s: %s\n", str, strerror(errno));
		return -1;
	}

	if (context->view != RAW_VIEW) {
		if (context->svc)
			printf("Searching for %s on %s ...\n", context->svc, str);
		else
			printf("Browsing %s ...\n", str);
	}

	attrid = sdp_list_append(0, &range);
	search = sdp_list_append(0, &context->group);
	if (sdp_service_search_attr_req(sess, search, SDP_ATTR_REQ_RANGE, attrid, &seq)) {
		printf("Service Search failed: %s\n", strerror(errno));
		sdp_list_free(attrid, 0);
		sdp_list_free(search, 0);
		sdp_close(sess);
		return -1;
	}
	sdp_list_free(attrid, 0);
	sdp_list_free(search, 0);

	for (; seq; seq = next) {
		sdp_record_t *rec = (sdp_record_t *) seq->data;
		struct search_context sub_context;

		switch (context->view) {
		case DEFAULT_VIEW:
			/* Display user friendly form */
			print_service_attr(rec);
			printf("\n");
			break;
		case TREE_VIEW:
			/* Display full tree */
			print_tree_attr(rec);
			printf("\n");
			break;
		case XML_VIEW:
			/* Display raw XML tree */
			convert_sdp_record_to_xml(rec, 0, doprintf);
			break;
		default:
			/* Display raw tree */
			print_raw_attr(rec);
			break;
		}

		/* Set the subcontext for browsing the sub tree */
		memcpy(&sub_context, context, sizeof(struct search_context));

		if (sdp_get_group_id(rec, &sub_context.group) != -1) {
			/* Browse the next level down if not done */
			if (sub_context.group.value.uuid16 != context->group.value.uuid16)
				do_search(bdaddr, &sub_context);
		}
		next = seq->next;
		free(seq);
		sdp_record_free(rec);
	}

	sdp_close(sess);
	return 0;
}

static struct option browse_options[] = {
	{ "help",	0, 0, 'h' },
	{ "tree",	0, 0, 't' },
	{ "raw",	0, 0, 'r' },
	{ "xml",	0, 0, 'x' },
	{ "uuid",	1, 0, 'u' },
	{ "l2cap",	0, 0, 'l' },
	{ 0, 0, 0, 0 }
};

static const char *browse_help =
	"Usage:\n"
	"\tbrowse [--tree] [--raw] [--xml] [--uuid uuid] [--l2cap] [bdaddr]\n";

/*
 * Browse the full SDP database (i.e. list all services starting from the
 * root/top-level).
 */
static int cmd_browse(int argc, char **argv)
{
	struct search_context context;
	int opt, num;

	/* Initialise context */
	memset(&context, '\0', sizeof(struct search_context));
	/* We want to browse the top-level/root */
	sdp_uuid16_create(&context.group, PUBLIC_BROWSE_GROUP);

	for_each_opt(opt, browse_options, 0) {
		switch (opt) {
		case 't':
			context.view = TREE_VIEW;
			break;
		case 'r':
			context.view = RAW_VIEW;
			break;
		case 'x':
			context.view = XML_VIEW;
			break;
		case 'u':
			if (sscanf(optarg, "%i", &num) != 1 || num < 0 || num > 0xffff) {
				printf("Invalid uuid %s\n", optarg);
				return -1;
			}
			sdp_uuid16_create(&context.group, num);
			break;
		case 'l':
			sdp_uuid16_create(&context.group, L2CAP_UUID);
			break;
		default:
			printf("%s", browse_help);
			return -1;
		}
	}

	argc -= optind;
	argv += optind;

	if (argc >= 1) {
		bdaddr_t bdaddr;
		estr2ba(argv[0], &bdaddr);
		return do_search(&bdaddr, &context);
	}

	return do_search(NULL, &context);
}

static struct option search_options[] = {
	{ "help",	0, 0, 'h' },
	{ "bdaddr",	1, 0, 'b' },
	{ "tree",	0, 0, 't' },
	{ "raw",	0, 0, 'r' },
	{ "xml",	0, 0, 'x' },
	{ 0, 0, 0, 0}
};

static const char *search_help =
	"Usage:\n"
	"\tsearch [--bdaddr bdaddr] [--tree] [--raw] [--xml] SERVICE\n"
	"SERVICE is a name (string) or UUID (0x1002)\n";

/*
 * Search for a specific SDP service
 *
 * Note : we should support multiple services on the command line :
 *          sdptool search 0x0100 0x000f 0x1002
 * (this would search a service supporting both L2CAP and BNEP directly in
 * the top level browse group)
 */
static int cmd_search(int argc, char **argv)
{
	struct search_context context;
	unsigned char *uuid = NULL;
	uint32_t class = 0;
	bdaddr_t bdaddr;
	int has_addr = 0;
	int i;
	int opt;

	/* Initialise context */
	memset(&context, '\0', sizeof(struct search_context));

	for_each_opt(opt, search_options, 0) {
		switch (opt) {
		case 'b':
			estr2ba(optarg, &bdaddr);
			has_addr = 1;
			break;
		case 't':
			context.view = TREE_VIEW;
			break;
		case 'r':
			context.view = RAW_VIEW;
			break;
		case 'x':
			context.view = XML_VIEW;
			break;
		default:
			printf("%s", search_help);
			return -1;
		}
	}

	argc -= optind;
	argv += optind;

	if (argc < 1) {
		printf("%s", search_help);
		return -1;
	}

	/* Note : we need to find a way to support search combining
	 * multiple services */
	context.svc = strdup(argv[0]);
	if (!strncasecmp(context.svc, "0x", 2)) {
		int num;
		/* This is a UUID16, just convert to int */
		sscanf(context.svc + 2, "%X", &num);
		class = num;
		printf("Class 0x%X\n", class);
	} else {
		/* Convert class name to an UUID */

		for (i = 0; service[i].name; i++)
			if (strcasecmp(context.svc, service[i].name) == 0) {
				class = service[i].class;
				uuid = service[i].uuid;
				break;
			}
		if (!class && !uuid) {
			printf("Unknown service %s\n", context.svc);
			return -1;
		}
	}

	if (class) {
		if (class & 0xffff0000)
			sdp_uuid32_create(&context.group, class);
		else {
			uint16_t class16 = class & 0xffff;
			sdp_uuid16_create(&context.group, class16);
		}
	} else
		sdp_uuid128_create(&context.group, uuid);

	if (has_addr)
		return do_search(&bdaddr, &context);

	return do_search(NULL, &context);
}

/*
 * Show how to get a specific SDP record by its handle.
 * Not really useful to the user, just show how it can be done...
 */
static int get_service(bdaddr_t *bdaddr, struct search_context *context, int quite)
{
	sdp_list_t *attrid;
	uint32_t range = 0x0000ffff;
	sdp_record_t *rec;
	sdp_session_t *session = sdp_connect(&interface, bdaddr, SDP_RETRY_IF_BUSY);

	if (!session) {
		char str[20];
		ba2str(bdaddr, str);
		printf("Failed to connect to SDP server on %s: %s\n", str, strerror(errno));
		return -1;
	}

	attrid = sdp_list_append(0, &range);
	rec = sdp_service_attr_req(session, context->handle, SDP_ATTR_REQ_RANGE, attrid);
	sdp_list_free(attrid, 0);
	sdp_close(session);

	if (!rec) {
		if (!quite) {
			printf("Service get request failed.\n");
			return -1;
		} else
			return 0;
	}

	switch (context->view) {
	case DEFAULT_VIEW:
		/* Display user friendly form */
		print_service_attr(rec);
		printf("\n");
		break;
	case TREE_VIEW:
		/* Display full tree */
		print_tree_attr(rec);
		printf("\n");
		break;
	case XML_VIEW:
		/* Display raw XML tree */
		convert_sdp_record_to_xml(rec, 0, doprintf);
		break;
	default:
		/* Display raw tree */
		print_raw_attr(rec);
		break;
	}

	sdp_record_free(rec);
	return 0;
}

static struct option records_options[] = {
	{ "help",	0, 0, 'h' },
	{ "tree",	0, 0, 't' },
	{ "raw",	0, 0, 'r' },
	{ "xml",	0, 0, 'x' },
	{ 0, 0, 0, 0 }
};

static const char *records_help =
	"Usage:\n"
	"\trecords [--tree] [--raw] [--xml] bdaddr\n";

/*
 * Request possible SDP service records
 */
static int cmd_records(int argc, char **argv)
{
	struct search_context context;
	uint32_t base[] = { 0x10000, 0x10300, 0x10500,
				0x1002e, 0x110b, 0x90000, 0x2008000,
					0x4000000, 0x100000, 0x1000000,
						0x4f491100, 0x4f491200 };
	bdaddr_t bdaddr;
	unsigned int i, n, num = 32;
	int opt, err = 0;

	/* Initialise context */
	memset(&context, '\0', sizeof(struct search_context));

	for_each_opt(opt, records_options, 0) {
		switch (opt) {
		case 't':
			context.view = TREE_VIEW;
			break;
		case 'r':
			context.view = RAW_VIEW;
			break;
		case 'x':
			context.view = XML_VIEW;
			break;
		default:
			printf("%s", records_help);
			return -1;
		}
	}

	argc -= optind;
	argv += optind;

	if (argc < 1) {
		printf("%s", records_help);
		return -1;
	}

	/* Convert command line parameters */
	estr2ba(argv[0], &bdaddr);

	for (i = 0; i < sizeof(base) / sizeof(uint32_t); i++)
		for (n = 0; n < num; n++) {
			context.handle = base[i] + n;
			err = get_service(&bdaddr, &context, 1);
			if (err < 0)
				return 0;
		}

	return 0;
}

static struct option get_options[] = {
	{ "help",	0, 0, 'h' },
	{ "bdaddr",	1, 0, 'b' },
	{ "tree",	0, 0, 't' },
	{ "raw",	0, 0, 'r' },
	{ "xml",	0, 0, 'x' },
	{ 0, 0, 0, 0 }
};

static const char *get_help =
	"Usage:\n"
	"\tget [--tree] [--raw] [--xml] [--bdaddr bdaddr] record_handle\n";

/*
 * Get a specific SDP record on the local SDP server
 */
static int cmd_get(int argc, char **argv)
{
	struct search_context context;
	bdaddr_t bdaddr;
	int has_addr = 0;
	int opt;

	/* Initialise context */
	memset(&context, '\0', sizeof(struct search_context));

	for_each_opt(opt, get_options, 0) {
		switch (opt) {
		case 'b':
			estr2ba(optarg, &bdaddr);
			has_addr = 1;
			break;
		case 't':
			context.view = TREE_VIEW;
			break;
		case 'r':
			context.view = RAW_VIEW;
			break;
		case 'x':
			context.view = XML_VIEW;
			break;
		default:
			printf("%s", get_help);
			return -1;
		}
	}

	argc -= optind;
	argv += optind;

	if (argc < 1) {
		printf("%s", get_help);
		return -1;
	}

	/* Convert command line parameters */
	context.handle = strtoul(argv[0], 0, 16);

	return get_service(has_addr ? &bdaddr : BDADDR_LOCAL, &context, 0);
}

static struct {
	char *cmd;
	int (*func)(int argc, char **argv);
	char *doc;
} command[] = {
	{ "search",  cmd_search,      "Search for a service"          },
	{ "browse",  cmd_browse,      "Browse all available services" },
	{ "records", cmd_records,     "Request all records"           },
	{ "add",     cmd_add,         "Add local service"             },
	{ "del",     cmd_del,         "Delete local service"          },
	{ "get",     cmd_get,         "Get local service"             },
	{ "setattr", cmd_setattr,     "Set/Add attribute to a SDP record"          },
	{ "setseq",  cmd_setseq,      "Set/Add attribute sequence to a SDP record" },
	{ 0, 0, 0 }
};

static void usage(void)
{
	int i, pos = 0;

	printf("sdptool - SDP tool v%s\n", VERSION);
	printf("Usage:\n"
		"\tsdptool [options] <command> [command parameters]\n");
	printf("Options:\n"
		"\t-h\t\tDisplay help\n"
		"\t-i\t\tSpecify source interface\n");

	printf("Commands:\n");
	for (i = 0; command[i].cmd; i++)
		printf("\t%-4s\t\t%s\n", command[i].cmd, command[i].doc);

	printf("\nServices:\n\t");
	for (i = 0; service[i].name; i++) {
		printf("%s ", service[i].name);
		pos += strlen(service[i].name) + 1;
		if (pos > 60) {
			printf("\n\t");
			pos = 0;
		}
	}
	printf("\n");
}

static struct option main_options[] = {
	{ "help",	0, 0, 'h' },
	{ "device",	1, 0, 'i' },
	{ 0, 0, 0, 0 }
};

int main(int argc, char *argv[])
{
	int i, opt;

	bacpy(&interface, BDADDR_ANY);

	while ((opt=getopt_long(argc, argv, "+i:h", main_options, NULL)) != -1) {
		switch(opt) {
		case 'i':
			if (!strncmp(optarg, "hci", 3))
				hci_devba(atoi(optarg + 3), &interface);
			else
				str2ba(optarg, &interface);
			break;

		case 'h':
			usage();
			exit(0);

		default:
			exit(1);
		}
	}

	argc -= optind;
	argv += optind;
	optind = 0;

	if (argc < 1) {
		usage();
		exit(1);
	}

	for (i = 0; command[i].cmd; i++)
		if (strncmp(command[i].cmd, argv[0], 4) == 0)
			return command[i].func(argc, argv);

	return 1;
}
