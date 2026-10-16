static const struct xt_icmp_names {
	const char *name;
	uint8_t type;
	uint8_t code_min, code_max;
} icmp_codes[] = {
	{ "any", 0xFF, 0, 0xFF },
	{ "echo-reply", 0, 0, 0xFF },
	/* Alias */ { "pong", 0, 0, 0xFF },

	{ "destination-unreachable", 3, 0, 0xFF },
	{   "network-unreachable", 3, 0, 0 },
	{   "host-unreachable", 3, 1, 1 },
	{   "protocol-unreachable", 3, 2, 2 },
	{   "port-unreachable", 3, 3, 3 },
	{   "fragmentation-needed", 3, 4, 4 },
	{   "source-route-failed", 3, 5, 5 },
	{   "network-unknown", 3, 6, 6 },
	{   "host-unknown", 3, 7, 7 },
	{   "network-prohibited", 3, 9, 9 },
	{   "host-prohibited", 3, 10, 10 },
	{   "TOS-network-unreachable", 3, 11, 11 },
	{   "TOS-host-unreachable", 3, 12, 12 },
	{   "communication-prohibited", 3, 13, 13 },
	{   "host-precedence-violation", 3, 14, 14 },
	{   "precedence-cutoff", 3, 15, 15 },

	{ "source-quench", 4, 0, 0xFF },

	{ "redirect", 5, 0, 0xFF },
	{   "network-redirect", 5, 0, 0 },
	{   "host-redirect", 5, 1, 1 },
	{   "TOS-network-redirect", 5, 2, 2 },
	{   "TOS-host-redirect", 5, 3, 3 },

	{ "echo-request", 8, 0, 0xFF },
	/* Alias */ { "ping", 8, 0, 0xFF },

	{ "router-advertisement", 9, 0, 0xFF },

	{ "router-solicitation", 10, 0, 0xFF },

	{ "time-exceeded", 11, 0, 0xFF },
	/* Alias */ { "ttl-exceeded", 11, 0, 0xFF },
	{   "ttl-zero-during-transit", 11, 0, 0 },
	{   "ttl-zero-during-reassembly", 11, 1, 1 },

	{ "parameter-problem", 12, 0, 0xFF },
	{   "ip-header-bad", 12, 0, 0 },
	{   "required-option-missing", 12, 1, 1 },

	{ "timestamp-request", 13, 0, 0xFF },

	{ "timestamp-reply", 14, 0, 0xFF },

	{ "address-mask-request", 17, 0, 0xFF },

	{ "address-mask-reply", 18, 0, 0xFF }
}, icmpv6_codes[] = {
	{ "destination-unreachable", 1, 0, 0xFF },
	{   "no-route", 1, 0, 0 },
	{   "communication-prohibited", 1, 1, 1 },
	{   "beyond-scope", 1, 2, 2 },
	{   "address-unreachable", 1, 3, 3 },
	{   "port-unreachable", 1, 4, 4 },
	{   "failed-policy", 1, 5, 5 },
	{   "reject-route", 1, 6, 6 },

	{ "packet-too-big", 2, 0, 0xFF },

	{ "time-exceeded", 3, 0, 0xFF },
	/* Alias */ { "ttl-exceeded", 3, 0, 0xFF },
	{   "ttl-zero-during-transit", 3, 0, 0 },
	{   "ttl-zero-during-reassembly", 3, 1, 1 },

	{ "parameter-problem", 4, 0, 0xFF },
	{   "bad-header", 4, 0, 0 },
	{   "unknown-header-type", 4, 1, 1 },
	{   "unknown-option", 4, 2, 2 },

	{ "echo-request", 128, 0, 0xFF },
	/* Alias */ { "ping", 128, 0, 0xFF },

	{ "echo-reply", 129, 0, 0xFF },
	/* Alias */ { "pong", 129, 0, 0xFF },

	{ "mld-listener-query", 130, 0, 0xFF },

	{ "mld-listener-report", 131, 0, 0xFF },

	{ "mld-listener-done", 132, 0, 0xFF },
	/* Alias */ { "mld-listener-reduction", 132, 0, 0xFF },

	{ "router-solicitation", 133, 0, 0xFF },

	{ "router-advertisement", 134, 0, 0xFF },

	{ "neighbour-solicitation", 135, 0, 0xFF },
	/* Alias */ { "neighbor-solicitation", 135, 0, 0xFF },

	{ "neighbour-advertisement", 136, 0, 0xFF },
	/* Alias */ { "neighbor-advertisement", 136, 0, 0xFF },

	{ "redirect", 137, 0, 0xFF },
}, igmp_types[] = {
	{ "membership-query", 0x11 },
	{ "membership-report-v1", 0x12 },
	{ "membership-report-v2", 0x16 },
	{ "leave-group", 0x17 },
	{ "membership-report-v3", 0x22 },
};

static inline char *parse_range(const char *str, unsigned int res[])
{
	char *next;

	if (!xtables_strtoui(str, &next, &res[0], 0, 255))
		return NULL;

	res[1] = res[0];
	if (*next == ':') {
		str = next + 1;
		if (!xtables_strtoui(str, &next, &res[1], 0, 255))
			return NULL;
	}

	return next;
}

static void
__parse_icmp(const struct xt_icmp_names codes[], size_t n_codes,
	     const char *codes_name, const char *fmtstring,
	     uint8_t type[], uint8_t code[])
{
	unsigned int match = n_codes;
	unsigned int i, number[2];

	for (i = 0; i < n_codes; i++) {
		if (strncasecmp(codes[i].name, fmtstring, strlen(fmtstring)))
			continue;
		if (match != n_codes)
			xtables_error(PARAMETER_PROBLEM,
				      "Ambiguous %s type `%s': `%s' or `%s'?",
				      codes_name, fmtstring, codes[match].name,
				      codes[i].name);
		match = i;
	}

	if (match < n_codes) {
		type[0] = type[1] = codes[match].type;
		if (code) {
			code[0] = codes[match].code_min;
			code[1] = codes[match].code_max;
		}
	} else {
		char *next = parse_range(fmtstring, number);
		if (!next)
			xtables_error(PARAMETER_PROBLEM, "Unknown %s type `%s'",
				      codes_name, fmtstring);
		type[0] = (uint8_t) number[0];
		type[1] = (uint8_t) number[1];
		switch (*next) {
		case 0:
			if (code) {
				code[0] = 0;
				code[1] = 255;
			}
			return;
		case '/':
			if (!code)
				break;

			next = parse_range(next + 1, number);
			if (!next)
				xtables_error(PARAMETER_PROBLEM,
					      "Unknown %s code `%s'",
					      codes_name, fmtstring);
			code[0] = (uint8_t) number[0];
			code[1] = (uint8_t) number[1];
			if (!*next)
				break;
		/* fallthrough */
		default:
			xtables_error(PARAMETER_PROBLEM,
				      "unknown character %c", *next);
		}
	}
}

static inline void
__ipt_parse_icmp(const struct xt_icmp_names *codes, size_t n_codes,
		 const char *codes_name, const char *fmtstr,
		 uint8_t *type, uint8_t code[])
{
	uint8_t types[2];

	__parse_icmp(codes, n_codes, codes_name, fmtstr, types, code);
	if (types[1] != types[0])
		xtables_error(PARAMETER_PROBLEM,
			      "%s type range not supported", codes_name);
	*type = types[0];
}

static inline void
ipt_parse_icmp(const char *str, uint8_t *type, uint8_t code[])
{
	__ipt_parse_icmp(icmp_codes, ARRAY_SIZE(icmp_codes),
			 "ICMP", str, type, code);
}

static inline void
ipt_parse_icmpv6(const char *str, uint8_t *type, uint8_t code[])
{
	__ipt_parse_icmp(icmpv6_codes, ARRAY_SIZE(icmpv6_codes),
			 "ICMPv6", str, type, code);
}

static inline void
ebt_parse_icmp(const char *str, uint8_t type[], uint8_t code[])
{
	__parse_icmp(icmp_codes, ARRAY_SIZE(icmp_codes),
		     "ICMP", str, type, code);
}

static inline void
ebt_parse_icmpv6(const char *str, uint8_t type[], uint8_t code[])
{
	__parse_icmp(icmpv6_codes, ARRAY_SIZE(icmpv6_codes),
		     "ICMPv6", str, type, code);
}

static inline void
ebt_parse_igmp(const char *str, uint8_t type[])
{
	__parse_icmp(igmp_types, ARRAY_SIZE(igmp_types),
		     "IGMP", str, type, NULL);
}

static void xt_print_icmp_types(const struct xt_icmp_names *_icmp_codes,
				unsigned int n_codes)
{
	unsigned int i;

	for (i = 0; i < n_codes; ++i) {
		if (i && _icmp_codes[i].type == _icmp_codes[i-1].type) {
			if (_icmp_codes[i].code_min == _icmp_codes[i-1].code_min
			    && (_icmp_codes[i].code_max
				== _icmp_codes[i-1].code_max))
				printf(" (%s)", _icmp_codes[i].name);
			else
				printf("\n   %s", _icmp_codes[i].name);
		}
		else
			printf("\n%s", _icmp_codes[i].name);
	}
	printf("\n");
}
