/* Shared library add-on to iptables for DCCP matching
 *
 * (C) 2005 by Harald Welte <laforge@netfilter.org>
 *
 * This program is distributed under the terms of GNU GPL v2, 1991
 *
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <xtables.h>
#include <linux/dccp.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter/xt_dccp.h>

#if 0
#define DEBUGP(format, first...) printf(format, ##first)
#define static
#else
#define DEBUGP(format, fist...) 
#endif

enum {
	O_SOURCE_PORT = 0,
	O_DEST_PORT,
	O_DCCP_TYPES,
	O_DCCP_OPTION,
};

static void dccp_help(void)
{
	printf(
"dccp match options\n"
"[!] --source-port port[:port]                          match source port(s)\n"
" --sport ...\n"
"[!] --destination-port port[:port]                     match destination port(s)\n"
" --dport ...\n"
"[!] --dccp-types type[,...]                            match when packet is one of the given types\n"
"[!] --dccp-option option                               match if option (by number!) is set\n"
);
}

#define s struct xt_dccp_info
static const struct xt_option_entry dccp_opts[] = {
	{.name = "source-port", .id = O_SOURCE_PORT, .type = XTTYPE_PORTRC,
	 .flags = XTOPT_INVERT | XTOPT_PUT, XTOPT_POINTER(s, spts)},
	{.name = "sport", .id = O_SOURCE_PORT, .type = XTTYPE_PORTRC,
	 .flags = XTOPT_INVERT | XTOPT_PUT, XTOPT_POINTER(s, spts)},
	{.name = "destination-port", .id = O_DEST_PORT, .type = XTTYPE_PORTRC,
	 .flags = XTOPT_INVERT | XTOPT_PUT, XTOPT_POINTER(s, dpts)},
	{.name = "dport", .id = O_DEST_PORT, .type = XTTYPE_PORTRC,
	 .flags = XTOPT_INVERT | XTOPT_PUT, XTOPT_POINTER(s, dpts)},
	{.name = "dccp-types", .id = O_DCCP_TYPES, .type = XTTYPE_STRING,
	 .flags = XTOPT_INVERT},
	{.name = "dccp-option", .id = O_DCCP_OPTION, .type = XTTYPE_UINT8,
	 .min = 1, .max = UINT8_MAX, .flags = XTOPT_INVERT | XTOPT_PUT,
	 XTOPT_POINTER(s, option)},
	XTOPT_TABLEEND,
};
#undef s

static const char *const dccp_pkt_types[] = {
	[DCCP_PKT_REQUEST] 	= "REQUEST",
	[DCCP_PKT_RESPONSE]	= "RESPONSE",
	[DCCP_PKT_DATA]		= "DATA",
	[DCCP_PKT_ACK]		= "ACK",
	[DCCP_PKT_DATAACK]	= "DATAACK",
	[DCCP_PKT_CLOSEREQ]	= "CLOSEREQ",
	[DCCP_PKT_CLOSE]	= "CLOSE",
	[DCCP_PKT_RESET]	= "RESET",
	[DCCP_PKT_SYNC]		= "SYNC",
	[DCCP_PKT_SYNCACK]	= "SYNCACK",
	[DCCP_PKT_INVALID]	= "INVALID",
};

/* Bits for type values 11-15 */
#define INVALID_OTHER_TYPE_MASK		0xf800

static uint16_t
parse_dccp_types(const char *typestring)
{
	uint16_t typemask = 0;
	char *ptr, *buffer;

	buffer = xtables_strdup(typestring);

	for (ptr = strtok(buffer, ","); ptr; ptr = strtok(NULL, ",")) {
		unsigned int i;
		for (i = 0; i < ARRAY_SIZE(dccp_pkt_types); ++i)
			if (!strcasecmp(dccp_pkt_types[i], ptr)) {
				typemask |= (1 << i);
				break;
			}
		if (i == ARRAY_SIZE(dccp_pkt_types))
			xtables_error(PARAMETER_PROBLEM,
				   "Unknown DCCP type `%s'", ptr);
	}
	if (typemask & (1 << DCCP_PKT_INVALID))
		typemask |= INVALID_OTHER_TYPE_MASK;


	free(buffer);
	return typemask;
}

static void dccp_parse(struct xt_option_call *cb)
{
	struct xt_dccp_info *einfo = cb->data;

	xtables_option_parse(cb);
	switch (cb->entry->id) {
	case O_SOURCE_PORT:
		einfo->flags |= XT_DCCP_SRC_PORTS;
		if (cb->invert)
			einfo->invflags |= XT_DCCP_SRC_PORTS;
		break;
	case O_DEST_PORT:
		einfo->flags |= XT_DCCP_DEST_PORTS;
		if (cb->invert)
			einfo->invflags |= XT_DCCP_DEST_PORTS;
		break;
	case O_DCCP_TYPES:
		einfo->flags |= XT_DCCP_TYPE;
		einfo->typemask = parse_dccp_types(cb->arg);
		if (cb->invert)
			einfo->invflags |= XT_DCCP_TYPE;
		break;
	case O_DCCP_OPTION:
		einfo->flags |= XT_DCCP_OPTION;
		if (cb->invert)
			einfo->invflags |= XT_DCCP_OPTION;
		break;
	}
}

static const char *
port_to_service(int port)
{
	const struct servent *service;

	if ((service = getservbyport(htons(port), "dccp")))
		return service->s_name;

	return NULL;
}

static void
print_port(uint16_t port, int numeric)
{
	const char *service;

	if (numeric || (service = port_to_service(port)) == NULL)
		printf("%u", port);
	else
		printf("%s", service);
}

static void
print_ports(const char *name, uint16_t min, uint16_t max,
	    int invert, int numeric)
{
	const char *inv = invert ? "!" : "";

	if (min != 0 || max != 0xFFFF || invert) {
		printf(" %s", name);
		if (min == max) {
			printf(":%s", inv);
			print_port(min, numeric);
		} else {
			printf("s:%s", inv);
			print_port(min, numeric);
			printf(":");
			print_port(max, numeric);
		}
	}
}

static void
print_types(uint16_t types, int inverted, int numeric)
{
	int have_type = 0;

	if (inverted)
		printf(" !");

	printf(" ");
	while (types) {
		unsigned int i;

		for (i = 0; !(types & (1 << i)); i++);

		if (have_type)
			printf(",");
		else
			have_type = 1;

		if (numeric)
			printf("%u", i);
		else {
			printf("%s", dccp_pkt_types[i]);

			if (i == DCCP_PKT_INVALID)
				break;
		}

		types &= ~(1 << i);
	}
}

static void
print_option(uint8_t option, int invert, int numeric)
{
	if (option || invert)
		printf(" option=%s%u", invert ? "!" : "", option);
}

static void
dccp_print(const void *ip, const struct xt_entry_match *match, int numeric)
{
	const struct xt_dccp_info *einfo =
		(const struct xt_dccp_info *)match->data;

	printf(" dccp");

	if (einfo->flags & XT_DCCP_SRC_PORTS) {
		print_ports("spt", einfo->spts[0], einfo->spts[1],
			einfo->invflags & XT_DCCP_SRC_PORTS,
			numeric);
	}

	if (einfo->flags & XT_DCCP_DEST_PORTS) {
		print_ports("dpt", einfo->dpts[0], einfo->dpts[1],
			einfo->invflags & XT_DCCP_DEST_PORTS,
			numeric);
	}

	if (einfo->flags & XT_DCCP_TYPE) {
		print_types(einfo->typemask,
			   einfo->invflags & XT_DCCP_TYPE,
			   numeric);
	}

	if (einfo->flags & XT_DCCP_OPTION) {
		print_option(einfo->option,
			     einfo->invflags & XT_DCCP_OPTION, numeric);
	}
}

static void dccp_save(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_dccp_info *einfo =
		(const struct xt_dccp_info *)match->data;

	if (einfo->flags & XT_DCCP_SRC_PORTS) {
		if (einfo->invflags & XT_DCCP_SRC_PORTS)
			printf(" !");
		if (einfo->spts[0] != einfo->spts[1])
			printf(" --sport %u:%u",
			       einfo->spts[0], einfo->spts[1]);
		else
			printf(" --sport %u", einfo->spts[0]);
	}

	if (einfo->flags & XT_DCCP_DEST_PORTS) {
		if (einfo->invflags & XT_DCCP_DEST_PORTS)
			printf(" !");
		if (einfo->dpts[0] != einfo->dpts[1])
			printf(" --dport %u:%u",
			       einfo->dpts[0], einfo->dpts[1]);
		else
			printf(" --dport %u", einfo->dpts[0]);
	}

	if (einfo->flags & XT_DCCP_TYPE) {
		printf("%s --dccp-types",
		       einfo->invflags & XT_DCCP_TYPE ? " !" : "");
		print_types(einfo->typemask, false, 0);
	}

	if (einfo->flags & XT_DCCP_OPTION) {
		printf("%s --dccp-option %u",
			einfo->invflags & XT_DCCP_OPTION ? " !" : "",
			einfo->option);
	}
}

static const char *const dccp_pkt_types_xlate[] = {
	[DCCP_PKT_REQUEST]      = "request",
	[DCCP_PKT_RESPONSE]     = "response",
	[DCCP_PKT_DATA]         = "data",
	[DCCP_PKT_ACK]          = "ack",
	[DCCP_PKT_DATAACK]      = "dataack",
	[DCCP_PKT_CLOSEREQ]     = "closereq",
	[DCCP_PKT_CLOSE]        = "close",
	[DCCP_PKT_RESET]        = "reset",
	[DCCP_PKT_SYNC]         = "sync",
	[DCCP_PKT_SYNCACK]      = "syncack",
	[DCCP_PKT_INVALID]	= "10-15",
};

static int dccp_type_xlate(const struct xt_dccp_info *einfo,
			   struct xt_xlate *xl)
{
	bool have_type = false, set_need = false;
	uint16_t types = einfo->typemask;

	if (types & INVALID_OTHER_TYPE_MASK) {
		types &= ~INVALID_OTHER_TYPE_MASK;
		types |= 1 << DCCP_PKT_INVALID;
	}

	if ((types != 0) && !(types == (types & -types))) {
		xt_xlate_add(xl, "{");
		set_need = true;
	}

	while (types) {
		unsigned int i;

		for (i = 0; !(types & (1 << i)); i++);

		if (have_type)
			xt_xlate_add(xl, ", ");
		else
			have_type = true;

		xt_xlate_add(xl, "%s", dccp_pkt_types_xlate[i]);

		types &= ~(1 << i);
	}

	if (set_need)
		xt_xlate_add(xl, "}");

	return 1;
}

static int dccp_xlate(struct xt_xlate *xl,
		      const struct xt_xlate_mt_params *params)
{
	const struct xt_dccp_info *einfo =
		(const struct xt_dccp_info *)params->match->data;
	int ret = 1;

	if (einfo->flags & XT_DCCP_SRC_PORTS) {
		xt_xlate_add(xl, "dccp sport%s %u",
			     einfo->invflags & XT_DCCP_SRC_PORTS ? " !=" : "",
			     einfo->spts[0]);

		if (einfo->spts[0] != einfo->spts[1])
			xt_xlate_add(xl, "-%u", einfo->spts[1]);
	}

	if (einfo->flags & XT_DCCP_DEST_PORTS) {
		xt_xlate_add(xl, "dccp dport%s %u",
			     einfo->invflags & XT_DCCP_DEST_PORTS ? " !=" : "",
			     einfo->dpts[0]);

		if (einfo->dpts[0] != einfo->dpts[1])
			xt_xlate_add(xl, "-%u", einfo->dpts[1]);
	}

	if (einfo->flags & XT_DCCP_TYPE && einfo->typemask) {
		xt_xlate_add(xl, "dccp type%s ",
			     einfo->invflags & XT_DCCP_TYPE ? " !=" : "");
		ret = dccp_type_xlate(einfo, xl);
	}

	/* FIXME: no dccp option support in nftables yet */
	if (einfo->flags & XT_DCCP_OPTION)
		ret = 0;

	return ret;
}
static struct xtables_match dccp_match = {
	.name		= "dccp",
	.family		= NFPROTO_UNSPEC,
	.version	= XTABLES_VERSION,
	.size		= XT_ALIGN(sizeof(struct xt_dccp_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct xt_dccp_info)),
	.help		= dccp_help,
	.print		= dccp_print,
	.save		= dccp_save,
	.x6_parse	= dccp_parse,
	.x6_options	= dccp_opts,
	.xlate		= dccp_xlate,
};

void _init(void)
{
	xtables_register_match(&dccp_match);
}
