/******************************************************************************

   Copyright 2023-2024 MaxLinear, Inc.

   For licensing information, see the file 'LICENSE' in the root folder of
   this software module.

******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <host_adapt.h>
#include <gsw_types.h>
#include <gpy2xx.h>

#include <gsw_cli_common.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#define ULONG_MAX_X    4294967295UL

uint32_t convert_pmac_adr_str( char *pmac_adr_str, unsigned char *pmac_adr_ptr )
{
	char *str_ptr=pmac_adr_str;
	char *endptr;
	int i;
	unsigned long int val;

	if (strlen(pmac_adr_str) != (16+7))
	{
		printf("ERROR: Invalid length of pmac (xx:xx:xx:xx:xx:xx:xx:xx)!\n");
		return 0;
	}

	for (i=0; i<8; i++)
	{
		val = strtoul(str_ptr, &endptr, 16);
		if ((*endptr != 0) && (*endptr != ':') && (*endptr != '-'))
			return 0;
		*(pmac_adr_ptr+i)= (unsigned char) (val & 0xFF);
		str_ptr = endptr+1;
	}
	return 1;
}

static int convert_ipv4_str(const char *ip_adr_str, uint32_t *ip_adr_ptr)
{
	struct in_addr dst;

	if (!ip_adr_str || !ip_adr_ptr)
		return 0;

	if (inet_pton(AF_INET, ip_adr_str, &dst) <= 0)
		return 0;

	*ip_adr_ptr = ntohl(dst.s_addr);

	return 1;
}

static int convert_ipv6_str(const char *ip_adr_str, unsigned short ip_adr_ptr[8])
{
	struct in6_addr dst;
	size_t i;

	if (!ip_adr_str || !ip_adr_ptr)
		return 0;

	if (inet_pton(AF_INET6, ip_adr_str, &dst) <= 0)
		return 0;

	for (i = 0; i < 8; i++)
		ip_adr_ptr[i] = ntohs(dst.s6_addr16[i]);

	return 1;
}

static int convert_pmapper_adr_str(char *pmap_adr_str, unsigned char *pmap_adr_ptr)
{
	char *str_ptr = pmap_adr_str;
	char *endptr;
	int i;
	unsigned long int val;

	for (i = 0; i < 73; i++) {
		val = strtoul(str_ptr, &endptr, 10);

		if ((*endptr != 0) && (*endptr != ',') && (*endptr != '-'))
			return 0;

		*(pmap_adr_ptr + i) = (unsigned char)(val & 0xFF);
		str_ptr = endptr + 1;
	}

	return 1;
}

char *findArgParam(int argc, char *argv[], char *name)
{
	int i;
	size_t len;

	len = strlen(name);

	for (i = 0; i < argc; i++) {
		if (strncasecmp(name, argv[i], len) == 0) {
			if (strlen(argv[i]) > (len + 1)) {
				if ('=' == argv[i][len]) {
					return argv[i] + len + 1;
				}
			}
		}
	}

	return NULL;
}

int scanStrParamArg(int argc, char *argv[], char *name, size_t size, char *param)
{
	char *ptr = findArgParam(argc, argv, name);

	if (ptr == NULL)
		return 0;

	// Copy the string value to the provided buffer
	strncpy(param, ptr, size - 1);
	param[size - 1] = '\0'; // Ensure null-terminated

	return 1;
}

int scanParamArg(int argc, char *argv[], char *name, size_t size, void *param)
{
	uint64_t tmp = 0;
	char *endptr;
	char *ptr = findArgParam(argc, argv, name);

	if (ptr == NULL)
		return 0;

	/* check if the given parameter value are the boolean "TRUE" / "FALSE" */
	if (0 == strncasecmp(ptr, "TRUE", strnlen(ptr, 6))) {
		tmp = 1;
	} else if (0 == strncasecmp(ptr, "FALSE", strnlen(ptr, 6))) {
		tmp = 0;
	} else {
		/* scan for a number */
		tmp = strtoull(ptr, &endptr, 0);

		/* parameter detection does not work in case there are more character after the provided number */
		if (*endptr != '\0')
			return 0;
	}

	if (size == sizeof(uint16_t) || size == sizeof(uint16_t) * 8)
		*((uint16_t *)param) = (uint16_t)tmp;
	else if (size == sizeof(uint32_t) || size == sizeof(uint32_t) * 8)
		*((uint32_t *)param) = (uint32_t)tmp;
	else if (size == sizeof(uint64_t) || size == sizeof(uint64_t) * 8)
		*((uint64_t *)param) = (uint64_t)tmp;
	else
		*((uint8_t *)param) = (uint8_t)tmp;

	return 1;
}

static int convert_mac_adr_str(char *mac_adr_str, u8 *mac_adr_ptr)
{
	char *str_ptr = mac_adr_str;
	char *endptr;
	int i;
	unsigned long int val;

	if (strlen(mac_adr_str) != (12 + 5)) {
		printf("ERROR: Invalid length of address string!\n");
		return 0;
	}

	for (i = 0; i < 6; i++) {
		val = strtoul(str_ptr, &endptr, 16);

		if ((*endptr != 0) && (*endptr != ':') && (*endptr != '-'))
			return 0;

		*(mac_adr_ptr + i) = (u8)(val & 0xFF);
		str_ptr = endptr + 1;
	}

	return 1;
}

static int copy_key_to_dst(char *key_adr_str, u8 size, char *key_adr_ptr)
{
	int i;
	char buf[20];
	char *pEnd = NULL;
	char *in_key = (char *)key_adr_str;
	uint32_t *out_key = (uint32_t *)key_adr_ptr;

	if (strlen(key_adr_str) != (size_t)(size * 2)) {
		printf("WARN: Len mismatch %ld != %ld!\n", strlen(in_key), (size_t)(size * 2));
	}

	for (i = 0; i < (size / 4); i++) {
		pEnd = NULL;
		snprintf(buf, sizeof(buf) / sizeof(buf[0]), "%c%c%c%c%c%c%c%c",
			 in_key[6], in_key[7], in_key[4], in_key[5],
			 in_key[2], in_key[3], in_key[0], in_key[1]);

		out_key[i] = (strtoul(buf, &pEnd, 16) & 0xFFFFFFFFu);
		in_key = in_key + 8;
		//printf("\nConverted int %s, %x", buf, out_key[i]);
	}

	return 0;
}

void printHex32Value(char *name, uint32_t value, uint32_t bitmapIndicator)
{
	if (bitmapIndicator == 0) {
		printf("\t%40s:\t%u", name, (uint32_t)value);

		if (value > 9) {
			/* Make an additional hex printout for larger values */
			printf(" (0x%0x)", (uint32_t)value);
		}
	} else {
		int i;
		int bitset = 0;

		if ((1 << (bitmapIndicator - 1)) & value) {

			/* Make an additional hex printout for larger values */
			uint32_t tmp = (1 << bitmapIndicator) - 1;

			if (tmp == 0) tmp -= 1;

			value = value & tmp;
			printf("\t%40s:\t0x%0x (Bits: ", name, (uint32_t)value);

			/* The highest data bit is set and is used as bitmap indicator, therefore
			   represent the data as bitmap as well. */
			for (i = bitmapIndicator - 2; i >= 0; i--) {
				if ((1 << i) & value) {
					if (bitset) printf(",");

					printf("%d", i);
					bitset = 1;
				}
			}
		}

		printf(")");
	}

	printf("\n");
}

int findStringParam(int argc, char *argv[], char *name)
{
	int i;

	/* search for all programm parameter for a command name */
	for (i = 1; i < argc; i++) {
		if (strncasecmp(argv[i], name, strlen(name)) == 0) {
			return 1;
		}
	}

	return 0;
}

int scanMAC_Arg(int argc, char *argv[], char *name, unsigned char *param)
{
	char *ptr = findArgParam(argc, argv, name);

	if (ptr == NULL) return 0;

	return convert_mac_adr_str(ptr, param);
}

int scanIPv4_Arg(int argc, char *argv[], char *name, uint32_t *param)
{
	char *ptr = findArgParam(argc, argv, name);

	if (ptr == NULL) return 0;

	return convert_ipv4_str(ptr, param);
}

int scanIPv6_Arg(int argc, char *argv[], char *name, unsigned short *param)
{
	char *ptr = findArgParam(argc, argv, name);

	if (ptr == NULL) return 0;

	return convert_ipv6_str(ptr, param);
}

int scanPMAP_Arg(int argc, char *argv[], char *name, unsigned char *param)
{
	char *ptr = findArgParam(argc, argv, name);

	if (ptr == NULL) return 0;

	return convert_pmapper_adr_str(ptr, param);
}

int scanKey_Arg(int argc, char *argv[], char *name, unsigned char size, char *param)
{
	char *ptr = findArgParam(argc, argv, name);

	if (ptr == NULL) return 0;

	return copy_key_to_dst(ptr, size, param);
}

int scanPMAC_Arg(int argc, char *argv[], char *name, unsigned char *param)
{
	char *ptr = findArgParam(argc, argv, name);

	if (ptr == NULL) return 0;

	return convert_pmac_adr_str(ptr, param);
}

void printMAC_Address(unsigned char *pMAC)
{
	printf("%02x:%02x:%02x:%02x:%02x:%02x",
	       pMAC[0],
	       pMAC[1],
	       pMAC[2],
	       pMAC[3],
	       pMAC[4],
	       pMAC[5]);
}

int checkValidMAC_Address(unsigned char *pMAC)
{
	if ((pMAC[0] == 0) &&
	    (pMAC[1] == 0) &&
	    (pMAC[2] == 0) &&
	    (pMAC[3] == 0) &&
	    (pMAC[4] == 0) &&
	    (pMAC[5] == 0))
		return (-1);

	return 0;
}

static struct {
	const char *name;
	enum link_mode_bit_indices bit;
} phy_advert[] = {
	{"10baseT_Half", LINK_MODE_10baseT_Half_BIT},
	{"10baseT_Full", LINK_MODE_10baseT_Full_BIT},
	{"100baseT_Half", LINK_MODE_100baseT_Half_BIT},
	{"100baseT_Full", LINK_MODE_100baseT_Full_BIT},
	{"1000baseT_Half", LINK_MODE_1000baseT_Half_BIT},
	{"1000baseT_Full", LINK_MODE_1000baseT_Full_BIT},
	{"2500baseT_Full", LINK_MODE_2500baseT_Full_BIT},
	{"2500baseT_FR", LINK_MODE_2500baseT_FR_BIT},
	{"5000baseT_Full", LINK_MODE_5000baseT_Full_BIT},
	{"5000baseT_FR", LINK_MODE_5000baseT_FR_BIT},
	{"Autoneg", LINK_MODE_Autoneg_BIT},
	{"Pause", LINK_MODE_Pause_BIT},
	{"Asym_Pause", LINK_MODE_Asym_Pause_BIT},
};

int scan_advert(int argc, char *argv[], char *name, uint64_t *param)
{
	char *p, *p1, *p2;
	uint8_t i;

	p = findArgParam(argc, argv, name);

	if (p == NULL)
		return 0;

	*param = 0;

	for (p1 = p; *p1 != 0; p1 = p2) {
		for (p2 = p1; *p2 != 0 && *p2 != ',' && *p2 != '|' && *p2 != ';'; p2++);

		if (*p2 != 0) {
			*p2 = 0;
			p2++;
		}

		if (*p1 == 0)
			continue;

		for (i = 0; i < ARRAY_SIZE(phy_advert); i++) {
			if (strncasecmp(p1, phy_advert[i].name, strlen(p)) == 0) {
				*param |= (uint64_t)1 << phy_advert[i].bit;
				break;
			}
		}
	}

	if (*param == 0)
		*param = strtoull(p, NULL, 0);

	return *param == 0 ? 0 : 1;
}

int print_advert(char *buf, uint32_t size, uint64_t param)
{
	char *p = buf;
	uint32_t flag = 0;
	uint32_t total_len = 0;
	uint32_t len;
	uint32_t i;

	strncpy(buf, "none", size);
	buf[size - 1] = 0;

	for (i = 0; i < ARRAY_SIZE(phy_advert); i++) {
		if ((param & ((uint64_t)1 << phy_advert[i].bit))) {
			len = (uint32_t)strlen(phy_advert[i].name);

			if (flag == 0) {
				if (total_len + len + 1 >= size)
					break;

				flag++;
			} else {
				if (total_len + len + 4 >= size)
					break;

				p[0] = ' ';
				p[1] = '|';
				p[2] = ' ';
				p += 3;
				total_len += 3;
			}

			strcpy(p, phy_advert[i].name);
			p += len;
			total_len += len;
		}
	}

	return (int)total_len;
}


