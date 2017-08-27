/*
 * src/utils.c		Utilities
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2009 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @defgroup cli Command Line Interface API
 *
 * @{
 *
 * These modules provide an interface for text based applications. The
 * functions provided are wrappers for their libnl equivalent with
 * added error handling. The functions check for allocation failures,
 * invalid input, and unknown types and will print error messages
 * accordingly via nl_cli_fatal().
 */

#include <netlink/cli/utils.h>

/**
 * Parse a text based 32 bit unsigned integer argument
 * @arg arg		Integer in text form.
 *
 * Tries to convert the number provided in arg to a uint32_t. Will call
 * nl_cli_fatal() if the conversion fails.
 *
 * @return 32bit unsigned integer.
 */
uint32_t nl_cli_parse_u32(const char *arg)
{
	unsigned long lval;
	char *endptr;

	lval = strtoul(arg, &endptr, 0);
	if (endptr == arg || lval == ULONG_MAX)
		nl_cli_fatal(EINVAL, "Unable to parse \"%s\", not a number.",
			     arg);

	return (uint32_t) lval;
}

void nl_cli_print_version(void)
{
	printf("libnl tools version %s\n", LIBNL_VERSION);
	printf(
	"Copyright (C) 2003-2010 Thomas Graf <tgraf@redhat.com>\n"
	"\n"
	"This program comes with ABSOLUTELY NO WARRANTY. This is free \n"
	"software, and you are welcome to redistribute it under certain\n"
	"conditions. See the GNU General Public License for details.\n"
	);

	exit(0);
}

/**
 * Print error message and quit application
 * @arg err		Error code.
 * @arg fmt		Error message.
 *
 * Prints the formatted error message to stderr and quits the application
 * using the provided error code.
 */
void nl_cli_fatal(int err, const char *fmt, ...)
{
	va_list ap;

	fprintf(stderr, "Error: ");

	if (fmt) {
		va_start(ap, fmt);
		vfprintf(stderr, fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	} else
		fprintf(stderr, "%s\n", strerror(err));

	exit(abs(err));
}

int nl_cli_connect(struct nl_sock *sk, int protocol)
{
	int err;

	if ((err = nl_connect(sk, protocol)) < 0)
		nl_cli_fatal(err, "Unable to connect netlink socket: %s",
			     nl_geterror(err));

	return err;
}

struct nl_sock *nl_cli_alloc_socket(void)
{
	struct nl_sock *sock;

	if (!(sock = nl_socket_alloc()))
		nl_cli_fatal(ENOBUFS, "Unable to allocate netlink socket");

	return sock;
}

struct nl_addr *nl_cli_addr_parse(const char *str, int family)
{
	struct nl_addr *addr;
	int err;

	if ((err = nl_addr_parse(str, family, &addr)) < 0)
		nl_cli_fatal(err, "Unable to parse address \"%s\": %s",
			     str, nl_geterror(err));

	return addr;
}

int nl_cli_parse_dumptype(const char *str)
{
	if (!strcasecmp(str, "brief"))
		return NL_DUMP_LINE;
	else if (!strcasecmp(str, "details") || !strcasecmp(str, "detailed"))
		return NL_DUMP_DETAILS;
	else if (!strcasecmp(str, "stats"))
		return NL_DUMP_STATS;
	else
		nl_cli_fatal(EINVAL, "Invalid dump type \"%s\".\n", str);

	return 0;
}

int nl_cli_confirm(struct nl_object *obj, struct nl_dump_params *params,
		   int default_yes)
{
	nl_object_dump(obj, params);

	for (;;) {
		char buf[32] = { 0 };
		int answer;

		printf("Delete? (%c/%c) ",
			default_yes ? 'Y' : 'y',
			default_yes ? 'n' : 'N');

		if (!fgets(buf, sizeof(buf), stdin)) {
			fprintf(stderr, "Error while reading\n.");
			continue;
		}

		switch ((answer = tolower(buf[0]))) {
		case '\n':
			answer = default_yes ? 'y' : 'n';
		case 'y':
		case 'n':
			return answer == 'y';
		}

		fprintf(stderr, "Invalid input, try again.\n");
	}

	return 0;

}

struct nl_cache *nl_cli_alloc_cache(struct nl_sock *sock, const char *name,
			    int (*ac)(struct nl_sock *, struct nl_cache **))
{
	struct nl_cache *cache;
	int err;

	if ((err = ac(sock, &cache)) < 0)
		nl_cli_fatal(err, "Unable to allocate %s cache: %s",
			     name, nl_geterror(err));

	nl_cache_mngt_provide(cache);

	return cache;
}

void nl_cli_load_module(const char *prefix, const char *name)
{
	char path[FILENAME_MAX+1];
	void *handle;

	snprintf(path, sizeof(path), "%s/%s/%s.so",
		 PKGLIBDIR, prefix, name);

	if (!(handle = dlopen(path, RTLD_NOW)))
		nl_cli_fatal(ENOENT, "Unable to load module \"%s\": %s\n",
			path, dlerror());
}

/** @} */
