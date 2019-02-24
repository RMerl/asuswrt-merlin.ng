/*
 * Copyright (C) 2013 Martin Willi
 * Copyright (C) 2013 revosec AG
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

/* Windows 7, for some fwpmu.h functionality */
#define _WIN32_WINNT 0x0601

#include "kernel_wfp_compat.h"

#include <library.h>

ENUM(auth_type_names, IPSEC_AUTH_MD5, IPSEC_AUTH_AES_256,
	"MD5",
	"SHA1",
	"SHA256",
	"AES128",
	"AES192",
	"AES256",
);

ENUM(auth_config_names, 0, 5,
	"HMAC96",
	"HMAC96",
	"HMAC128",
	"GMAC",
	"GMAC",
	"GMAC",
);

ENUM(cipher_type_names, IPSEC_CIPHER_TYPE_DES, IPSEC_CIPHER_TYPE_AES_256,
	"DES",
	"3DES",
	"AES128",
	"AES192",
	"AES256",
);

ENUM(cipher_config_names, 1, 8,
	"CBC",
	"CBC",
	"CBC",
	"CBC",
	"CBC",
	"GCM",
	"GCM",
	"GCM",
);

ENUM(match_type_names, FWP_MATCH_EQUAL, FWP_MATCH_NOT_EQUAL,
	"equals",
	"greater",
	"less than",
	"greater or equal than",
	"less or equal than",
	"in range",
	"has all flags set",
	"has any flags set",
	"has none flags set",
	"equals case insensitive",
	"not equal",
);

ENUM(traffic_type_names, IPSEC_TRAFFIC_TYPE_TRANSPORT, IPSEC_TRAFFIC_TYPE_TUNNEL,
	"Transport",
	"Tunnel",
);

/**
 * Print a GUID to a static buffer
 */
static char *guid2string(GUID *guid)
{
	static char buf[64];

	snprintf(buf, sizeof(buf),
		"%08x,%04x,%04x%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x",
		guid->Data1, guid->Data2, guid->Data3,
		guid->Data4[0], guid->Data4[1], guid->Data4[2], guid->Data4[3],
		guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]);

	return buf;
}

/**
 * Convert filter condition key GUID to some known strings
 */
static char* cond2name(GUID *guid, bool *address)
{
	struct {
		GUID guid;
		char *name;
		bool address;
	} map[] = {
		{ FWPM_CONDITION_IP_LOCAL_ADDRESS, "local address", TRUE},
		{ FWPM_CONDITION_IP_REMOTE_ADDRESS, "remote address", TRUE},
		{ FWPM_CONDITION_IP_SOURCE_ADDRESS, "source address", TRUE},
		{ FWPM_CONDITION_IP_DESTINATION_ADDRESS, "destination address", TRUE},
		{ FWPM_CONDITION_IP_LOCAL_PORT, "local port", FALSE},
		{ FWPM_CONDITION_IP_REMOTE_PORT, "remote port", FALSE},
		{ FWPM_CONDITION_IP_PROTOCOL, "protocol", FALSE},
		{ FWPM_CONDITION_ICMP_CODE, "icmp code", FALSE},
		{ FWPM_CONDITION_ICMP_TYPE, "icmp type", FALSE},
	};
	int i;

	for (i = 0; i < countof(map); i++)
	{
		if (memeq(&map[i].guid, guid, sizeof(GUID)))
		{
			*address = map[i].address;
			return map[i].name;
		}
	}
	*address = FALSE;
	return guid2string(guid);
}

/**
 * Print a host from raw data and IP version
 */
static void print_host(FWP_IP_VERSION version, void *data)
{
	host_t *host = NULL;
	UINT32 ints[4];

	switch (version)
	{
		case FWP_IP_VERSION_V4:
			ints[0] = untoh32(data);
			host = host_create_from_chunk(AF_INET, chunk_from_thing(ints[0]), 0);
			break;
		case FWP_IP_VERSION_V6:
			ints[3] = untoh32(data);
			ints[2] = untoh32(data + 4);
			ints[1] = untoh32(data + 8);
			ints[0] = untoh32(data + 12);
			host = host_create_from_chunk(AF_INET6, chunk_from_thing(ints), 0);
			break;
		default:
			break;
	}
	if (host)
	{
		printf("%H", host);
		host->destroy(host);
	}
}

/**
 * Print IPSEC_SA_AUTH_INFORMATION0
 */
static void print_auth(IPSEC_SA_AUTH_INFORMATION0 *a)
{
	printf("%N-%N",
		auth_type_names, a->authTransform.authTransformId.authType,
		auth_config_names, a->authTransform.authTransformId.authConfig);
}

/**
 * Print IPSEC_SA_CIPHER_INFORMATION0
 */
static void print_cipher(IPSEC_SA_CIPHER_INFORMATION0 *c)
{
	printf("%N-%N",
		cipher_type_names, c->cipherTransform.cipherTransformId.cipherType,
		cipher_config_names, c->cipherTransform.cipherTransformId.cipherConfig);
}

/**
 * Print IPsec SA transform
 */
static void list_sa(HANDLE engine, IPSEC_SA0 *sa)
{
	printf("    SPI 0x%08x\n", sa->spi);
	switch (sa->saTransformType)
	{
		case IPSEC_TRANSFORM_AH:
			printf("      AH: ");
			print_auth(sa->ahInformation);
			break;
		case IPSEC_TRANSFORM_ESP_AUTH:
			printf("      ESP: ");
			print_auth(sa->espAuthInformation);
			break;
		case IPSEC_TRANSFORM_ESP_CIPHER:
			printf("      ESP: ");
			print_cipher(sa->espCipherInformation);
			break;
		case IPSEC_TRANSFORM_ESP_AUTH_AND_CIPHER:
			printf("      ESP: ");
			print_auth(&sa->espAuthAndCipherInformation->saAuthInformation);
			printf(", ");
			print_cipher(&sa->espAuthAndCipherInformation->saCipherInformation);
			break;
		default:
			printf("      (Transform %d)", sa->saTransformType);
			break;
	}
	printf("\n");
}

/**
 * List a filter condition value, optionally as IP address
 */
static void print_value(FWP_CONDITION_VALUE0 *value, bool address)
{
	chunk_t chunk;

	switch (value->type)
	{
		case FWP_EMPTY:
			printf("empty");
			break;
		case FWP_UINT8:
			printf("%u", value->uint8);
			break;
		case FWP_UINT16:
			printf("%u", value->uint16);
			break;
		case FWP_UINT32:
			if (address)
			{
				print_host(FWP_IP_VERSION_V4, &value->uint32);
			}
			else
			{
				printf("%u", value->uint32);
			}
			break;
		case FWP_UINT64:
			printf("%llu", value->uint64);
			break;
		case FWP_INT8:
			printf("%d", value->int8);
			break;
		case FWP_INT16:
			printf("%d", value->int16);
			break;
		case FWP_INT32:
			printf("%d", value->int32);
			break;
		case FWP_INT64:
			printf("%lld", value->int64);
			break;
		case FWP_FLOAT:
			printf("%f", value->float32);
			break;
		case FWP_DOUBLE:
			printf("%lf", value->double64);
			break;
		case FWP_BYTE_ARRAY16_TYPE:
			if (address)
			{
				print_host(FWP_IP_VERSION_V6, value->byteArray16);
			}
			else
			{
				chunk = chunk_create((u_char*)value->byteArray16, 16);
				printf("%#B", &chunk);
			}
			break;
		case FWP_BYTE_BLOB_TYPE:
			chunk = chunk_create(value->byteBlob->data, value->byteBlob->size);
			printf("%#B", &chunk);
			break;
		case FWP_V4_ADDR_MASK:
			print_host(FWP_IP_VERSION_V4, &value->v4AddrMask->addr);
			printf("/");
			print_host(FWP_IP_VERSION_V4, &value->v4AddrMask->mask);
			break;
		case FWP_V6_ADDR_MASK:
			print_host(FWP_IP_VERSION_V6, &value->v6AddrMask->addr);
			printf("/%u", &value->v6AddrMask->prefixLength);
			break;
		case FWP_RANGE_TYPE:
			print_value((FWP_CONDITION_VALUE0*)&value->rangeValue->valueLow,
						address);
			printf(" - ");
			print_value((FWP_CONDITION_VALUE0*)&value->rangeValue->valueHigh,
						address);
			break;
		default:
			printf("(unsupported)");
			break;
	}
}

/**
 * List a filter condition
 */
static void list_cond(HANDLE engine, FWPM_FILTER_CONDITION0 *cond)
{
	bool address;

	printf("      '%s' %N '", cond2name(&cond->fieldKey, &address),
		match_type_names, cond->matchType);
	print_value(&cond->conditionValue, address);
	printf("'\n");
}

/**
 * Print IPsec SA details
 */
static void list_details(HANDLE engine, IPSEC_SA_DETAILS1 *details)
{
	int i;

	printf("  %sbound SA: ",
		details->saDirection == FWP_DIRECTION_INBOUND ? "In" : "Out");
	print_host(details->traffic.ipVersion, &details->traffic.localV4Address);
	printf(" %s ", details->saDirection == FWP_DIRECTION_INBOUND ? "<-" : "->");
	print_host(details->traffic.ipVersion, &details->traffic.remoteV4Address);
	printf("\n    %N, flags: 0x%06x, lifetime: %us\n",
		  traffic_type_names, details->traffic.trafficType,
		  details->saBundle.flags, details->saBundle.lifetime.lifetimeSeconds);
	if (details->udpEncapsulation)
	{
		printf("    UDP encap ports %u - %u\n",
			details->udpEncapsulation->localUdpEncapPort,
			details->udpEncapsulation->remoteUdpEncapPort);
	}
	for (i = 0; i < details->saBundle.numSAs; i++)
	{
		list_sa(engine, &details->saBundle.saList[i]);
	}
	printf("    Filter ID %llu\n", details->transportFilter->filterId);
	for (i = 0; i < details->transportFilter->numFilterConditions; i++)
	{
		list_cond(engine, &details->transportFilter->filterCondition[i]);
	}
}

/**
 * List installed SA contexts
 */
static bool list_contexts(HANDLE engine)
{
	HANDLE handle;
	UINT32 returned;
	DWORD res;
	IPSEC_SA_CONTEXT1 **entries;

	res = IPsecSaContextCreateEnumHandle0(engine, NULL, &handle);
	if (res != ERROR_SUCCESS)
	{
		fprintf(stderr, "IPsecSaContextCreateEnumHandle0(): 0x%08x\n", res);
		return FALSE;
	}

	while (TRUE)
	{
		res = IPsecSaContextEnum1(engine, handle, 1, &entries, &returned);
		if (res != ERROR_SUCCESS)
		{
			fprintf(stderr, "IPsecSaContextEnum1(): 0x%08x\n", res);
			IPsecSaContextDestroyEnumHandle0(engine, handle);
			return FALSE;
		}
		if (returned == 0)
		{
			break;
		}

		printf("SA context %llu:\n", entries[0]->saContextId);
		list_details(engine, entries[0]->inboundSa);
		list_details(engine, entries[0]->outboundSa);

		FwpmFreeMemory0((void**)&entries);
	}
	IPsecSaContextDestroyEnumHandle0(engine, handle);
	return TRUE;
}

const GUID FWPM_LAYER_IPSEC_KM_DEMUX_V4 = {
	0xf02b1526, 0xa459, 0x4a51, { 0xb9, 0xe3, 0x75, 0x9d, 0xe5, 0x2b, 0x9d, 0x2c }
};
const GUID FWPM_LAYER_IPSEC_KM_DEMUX_V6 = {
	0x2f755cf6, 0x2fd4, 0x4e88, { 0xb3, 0xe4, 0xa9, 0x1b, 0xca, 0x49, 0x52, 0x35 }
};
const GUID FWPM_LAYER_IPSEC_V4 = {
	0xeda65c74, 0x610d, 0x4bc5, { 0x94, 0x8f, 0x3c, 0x4f, 0x89, 0x55, 0x68, 0x67 }
};
const GUID FWPM_LAYER_IPSEC_V6 = {
	0x13c48442, 0x8d87, 0x4261, { 0x9a, 0x29, 0x59, 0xd2, 0xab, 0xc3, 0x48, 0xb4 }
};
const GUID FWPM_LAYER_IKEEXT_V4 = {
	0xb14b7bdb, 0xdbbd, 0x473e, { 0xbe, 0xd4, 0x8b, 0x47, 0x08, 0xd4, 0xf2, 0x70 }
};
const GUID FWPM_LAYER_IKEEXT_V6 = {
	0xb64786b3, 0xf687, 0x4eb9, { 0x89, 0xd2, 0x8e, 0xf3, 0x2a, 0xcd, 0xab, 0xe2 }
};
const GUID FWPM_LAYER_INBOUND_IPPACKET_V4 = {
	0xc86fd1bf, 0x21cd, 0x497e, { 0xa0, 0xbb, 0x17, 0x42, 0x5c, 0x88, 0x5c, 0x58 }
};
const GUID FWPM_LAYER_INBOUND_IPPACKET_V4_DISCARD = {
	0xb5a230d0, 0xa8c0, 0x44f2, { 0x91, 0x6e, 0x99, 0x1b, 0x53, 0xde, 0xd1, 0xf7 }
};
const GUID FWPM_LAYER_INBOUND_IPPACKET_V6 = {
	0xf52032cb, 0x991c, 0x46e7, { 0x97, 0x1d, 0x26, 0x01, 0x45, 0x9a, 0x91, 0xca }
};
const GUID FWPM_LAYER_INBOUND_IPPACKET_V6_DISCARD = {
	0xbb24c279, 0x93b4, 0x47a2, { 0x83, 0xad, 0xae, 0x16, 0x98, 0xb5, 0x08, 0x85 }
};
const GUID FWPM_LAYER_OUTBOUND_IPPACKET_V4 = {
	0x1e5c9fae, 0x8a84, 0x4135, { 0xa3, 0x31, 0x95, 0x0b, 0x54, 0x22, 0x9e, 0xcd }
};
const GUID FWPM_LAYER_OUTBOUND_IPPACKET_V4_DISCARD = {
	0x08e4bcb5, 0xb647, 0x48f3, { 0x95, 0x3c, 0xe5, 0xdd, 0xbd, 0x03, 0x93, 0x7e }
};
const GUID FWPM_LAYER_OUTBOUND_IPPACKET_V6 = {
	0xa3b3ab6b, 0x3564, 0x488c, { 0x91, 0x17, 0xf3, 0x4e, 0x82, 0x14, 0x27, 0x63 }
};
const GUID FWPM_LAYER_OUTBOUND_IPPACKET_V6_DISCARD = {
	0x9513d7c4, 0xa934, 0x49dc, { 0x91, 0xa7, 0x6c, 0xcb, 0x80, 0xcc, 0x02, 0xe3 }
};
const GUID FWPM_LAYER_IPFORWARD_V4_DISCARD = {
	0x9e9ea773, 0x2fae, 0x4210, { 0x8f, 0x17, 0x34, 0x12, 0x9e, 0xf3, 0x69, 0xeb }
};
const GUID FWPM_LAYER_IPFORWARD_V6_DISCARD = {
	0x31524a5d, 0x1dfe, 0x472f, { 0xbb, 0x93, 0x51, 0x8e, 0xe9, 0x45, 0xd8, 0xa2 }
};
const GUID FWPM_LAYER_INBOUND_TRANSPORT_V4_DISCARD = {
	0xac4a9833, 0xf69d, 0x4648, { 0xb2, 0x61, 0x6d, 0xc8, 0x48, 0x35, 0xef, 0x39 }
};
const GUID FWPM_LAYER_INBOUND_TRANSPORT_V6_DISCARD = {
	0x2a6ff955, 0x3b2b, 0x49d2, { 0x98, 0x48, 0xad, 0x9d, 0x72, 0xdc, 0xaa, 0xb7 }
};
const GUID FWPM_LAYER_OUTBOUND_TRANSPORT_V4_DISCARD = {
	0xc5f10551, 0xbdb0, 0x43d7, { 0xa3, 0x13, 0x50, 0xe2, 0x11, 0xf4, 0xd6, 0x8a }
};
const GUID FWPM_LAYER_OUTBOUND_TRANSPORT_V6_DISCARD = {
	0xf433df69, 0xccbd, 0x482e, { 0xb9, 0xb2, 0x57, 0x16, 0x56, 0x58, 0xc3, 0xb3 }
};

/**
 * Convert filter layer GUID to name
 */
static char* layer2name(GUID *guid)
{
	struct {
		GUID guid;
		char *name;
	} map[] = {
		{ FWPM_LAYER_IPSEC_KM_DEMUX_V4, "IPsec KM demux v4" },
		{ FWPM_LAYER_IPSEC_KM_DEMUX_V6, "IPsec KM demux v6" },
		{ FWPM_LAYER_IPSEC_V4, "IPsec v4" },
		{ FWPM_LAYER_IPSEC_V6, "IPsec v6" },
		{ FWPM_LAYER_IKEEXT_V4, "IKE ext v4" },
		{ FWPM_LAYER_IKEEXT_V6, "IKE ext v6" },
		{ FWPM_LAYER_INBOUND_IPPACKET_V4, "inbound v4" },
		{ FWPM_LAYER_INBOUND_IPPACKET_V4_DISCARD, "inbound v4 dsc" },
		{ FWPM_LAYER_INBOUND_IPPACKET_V6, "inbound v6" },
		{ FWPM_LAYER_INBOUND_IPPACKET_V6_DISCARD, "inbound v6 dsc" },
		{ FWPM_LAYER_OUTBOUND_IPPACKET_V4, "outbound v4" },
		{ FWPM_LAYER_OUTBOUND_IPPACKET_V4_DISCARD, "outbound v4 dsc" },
		{ FWPM_LAYER_OUTBOUND_IPPACKET_V6, "outbound v6" },
		{ FWPM_LAYER_OUTBOUND_IPPACKET_V6_DISCARD, "outbound v6 dsc" },
		{ FWPM_LAYER_IPFORWARD_V4, "forward v4" },
		{ FWPM_LAYER_IPFORWARD_V4_DISCARD, "forward v4 dsc" },
		{ FWPM_LAYER_IPFORWARD_V6, "forward v6" },
		{ FWPM_LAYER_IPFORWARD_V6_DISCARD, "forward v6 discard" },
		{ FWPM_LAYER_INBOUND_TRANSPORT_V4, "inbound transport v4" },
		{ FWPM_LAYER_INBOUND_TRANSPORT_V4_DISCARD, "inbound transport v4 dsc" },
		{ FWPM_LAYER_INBOUND_TRANSPORT_V6, "inbound transport v6" },
		{ FWPM_LAYER_INBOUND_TRANSPORT_V6_DISCARD, "inbound v6 transport dsc" },
		{ FWPM_LAYER_OUTBOUND_TRANSPORT_V4, "outbound transport v4" },
		{ FWPM_LAYER_OUTBOUND_TRANSPORT_V4_DISCARD, "outbound transport v4 dsc" },
		{ FWPM_LAYER_OUTBOUND_TRANSPORT_V6, "outbound transport v6" },
		{ FWPM_LAYER_OUTBOUND_TRANSPORT_V6_DISCARD, "outbound transport v6 dsc" },
	};
	int i;

	for (i = 0; i < countof(map); i++)
	{
		if (memeq(&map[i].guid, guid, sizeof(GUID)))
		{
			return map[i].name;
		}
	}
	return NULL;
}

/**
 * Convert filter callout GUID to name
 */
static char* callout2name(GUID *guid)
{
	struct {
		GUID guid;
		char *name;
	} map[] = {
		{ FWPM_CALLOUT_IPSEC_INBOUND_TRANSPORT_V4, "inbound transport v4" },
		{ FWPM_CALLOUT_IPSEC_INBOUND_TRANSPORT_V6, "inbound transport v6" },
		{ FWPM_CALLOUT_IPSEC_OUTBOUND_TRANSPORT_V4, "outbound transport v4" },
		{ FWPM_CALLOUT_IPSEC_OUTBOUND_TRANSPORT_V6, "outbound transport v6" },
		{ FWPM_CALLOUT_IPSEC_INBOUND_TUNNEL_V4, "inbound tunnel v4" },
		{ FWPM_CALLOUT_IPSEC_INBOUND_TUNNEL_V6, "inbound tunnel v6" },
		{ FWPM_CALLOUT_IPSEC_OUTBOUND_TUNNEL_V4, "outbound tunnel v4" },
		{ FWPM_CALLOUT_IPSEC_OUTBOUND_TUNNEL_V6, "outbound tunnel v6" },
		{ FWPM_CALLOUT_IPSEC_FORWARD_INBOUND_TUNNEL_V4, "forward in tunnel v4" },
		{ FWPM_CALLOUT_IPSEC_FORWARD_INBOUND_TUNNEL_V6, "forward in tunnel v6" },
		{ FWPM_CALLOUT_IPSEC_FORWARD_OUTBOUND_TUNNEL_V4, "forward out tunnel v4" },
		{ FWPM_CALLOUT_IPSEC_FORWARD_OUTBOUND_TUNNEL_V6, "forward out tunnel v6" },
	};
	int i;

	for (i = 0; i < countof(map); i++)
	{
		if (memeq(&map[i].guid, guid, sizeof(GUID)))
		{
			return map[i].name;
		}
	}
	return guid2string(guid);
}

/**
 * Print display data with description
 */
static void print_display_data(FWPM_DISPLAY_DATA0 *data)
{
	char buf[128];

	buf[0] = '\0';
	if (data->name)
	{
		wcstombs(buf, data->name, sizeof(buf));
	}
	printf("%s", buf);
	if (data->description)
	{
		buf[0] = '\0';
		wcstombs(buf, data->description, sizeof(buf));
		if (strlen(buf))
		{
			printf(" (%s)", buf);
		}
	}
}

/**
 * List installed firewall filters
 */
static bool list_filters(HANDLE engine)
{
	HANDLE handle;
	UINT32 returned;
	DWORD res;
	FWPM_FILTER0 **entries;
	char *layer;
	int i;

	res = FwpmFilterCreateEnumHandle0(engine, NULL, &handle);
	if (res != ERROR_SUCCESS)
	{
		fprintf(stderr, "FwpmFilterCreateEnumHandle0(): 0x%08x\n", res);
		return FALSE;
	}

	while (TRUE)
	{
		res = FwpmFilterEnum0(engine, handle, 1, &entries, &returned);
		if (res != ERROR_SUCCESS)
		{
			fprintf(stderr, "FwpmFilterEnum0(): 0x%08x\n", res);
			FwpmFilterDestroyEnumHandle0(engine, handle);
			return FALSE;
		}
		if (returned == 0)
		{
			break;
		}

		layer = layer2name(&entries[0]->layerKey);
		if (layer)
		{
			printf("Filter ID %llu, '", entries[0]->filterId);
			print_display_data(&entries[0]->displayData);
			printf("'\n");
			printf("  %s, ", layer);
			if (entries[0]->effectiveWeight.type == FWP_UINT64)
			{
				printf("weight %016llx, ", *entries[0]->effectiveWeight.uint64);
			}

			switch (entries[0]->action.type)
			{
				case FWP_ACTION_BLOCK:
					printf("block\n");
					break;
				case FWP_ACTION_PERMIT:
					printf("permit\n");
					break;
				case FWP_ACTION_CALLOUT_TERMINATING:
					printf("callout terminating: %s\n",
						callout2name(&entries[0]->action.calloutKey));
					break;
				case FWP_ACTION_CALLOUT_INSPECTION:
					printf("callout inspection: %s\n",
						callout2name(&entries[0]->action.calloutKey));
					break;
				case FWP_ACTION_CALLOUT_UNKNOWN:
					printf("callout unknown: %s\n",
						callout2name(&entries[0]->action.calloutKey));
					break;
				default:
					printf("(unknown action)\n");
					break;
			}
			for (i = 0; i < entries[0]->numFilterConditions; i++)
			{
				list_cond(engine, &entries[0]->filterCondition[i]);
			}
		}
		FwpmFreeMemory0((void**)&entries);
	}
	FwpmFilterDestroyEnumHandle0(engine, handle);
	return TRUE;
}

/**
 * ipsecdump main()
 */
int main(int argc, char *argv[])
{
	FWPM_SESSION0 session = {
		.displayData = {
			.name = L"ipsecdump",
			.description = L"strongSwan SAD/SPD dumper",
		},
	};
	HANDLE engine;
	DWORD res;
	int code;

	library_init(NULL, "ipsecdump");
	atexit(library_deinit);

	res = FwpmEngineOpen0(NULL, RPC_C_AUTHN_WINNT, NULL, &session, &engine);
	if (res != ERROR_SUCCESS)
	{
		fprintf(stderr, "FwpmEngineOpen(): 0x%08x\n", res);
		return 2;
	}
	if (argc > 1 && streq(argv[1], "filters"))
	{
		code = list_filters(engine) ? 0 : 1;
	}
	else
	{
		code = list_contexts(engine) ? 0 : 1;
	}
	FwpmEngineClose0(engine);
	return code;
}
