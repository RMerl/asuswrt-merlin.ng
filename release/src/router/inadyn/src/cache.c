/* Reads and updates cache files
 *
 * Copyright (C) 2003-2004  Narcis Ilisei <inarcis2002@hotpop.com>
 * Copyright (C) 2010-2020  Joachim Nilsson <troglobit@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, visit the Free Software Foundation
 * website at http://www.gnu.org/licenses/gpl-2.0.html or write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

/*
 * A user may have several DNS records to update.  Earlier versions of
 * inadyn supports this, but only recorded changes in one cache file.
 * This made keeping track of update times per record impossible.  Now
 * inadyn records each DNS entry to be updated in its own cache file,
 * enabling individual updates and tracking the file MTIME better.
 *
 * At startup inadyn will fall back to the old cache file and remove it
 * once it has read the IP and the modification time.
 */

#include <time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <resolv.h>
#include <sys/stat.h>

#include "ddns.h"
#include "cache.h"

extern ddns_info_t *conf_info_iterator(int first);

static int nslookup(ddns_alias_t *alias)
{
	int error;
	struct addrinfo hints;
	struct addrinfo *result;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;	/* IPv4 */
	hints.ai_socktype = SOCK_DGRAM;	/* Datagram socket */
	hints.ai_flags = 0;
	hints.ai_protocol = 0;          /* Any protocol */

	error = getaddrinfo(alias->name, NULL, &hints, &result);
	if (!error) {
		char address[MAX_ADDRESS_LEN];

		/* DNS reply for alias found, convert to IP# */
		if (!getnameinfo(result->ai_addr, result->ai_addrlen, address, sizeof(address), NULL, 0, NI_NUMERICHOST)) {
			/* Update local record for next checkip call. */
			alias->last_update = 0;
			strlcpy(alias->address, address, sizeof(alias->address));
			logit(LOG_INFO, "Resolving hostname %s => IP# %s", alias->name, address);
		}

		freeaddrinfo(result);
		return 0;
	}

	logit(LOG_WARNING, "Failed resolving hostname %s: %s", alias->name, gai_strerror(error));

	return 1;
}

static void read_one(ddns_alias_t *alias, int nonslookup)
{
	FILE *fp;
	char path[256];

	alias->last_update = 0;
	memset(alias->address, 0, sizeof(alias->address));

	cache_file(alias->name, path, sizeof(path));
	fp = fopen(path, "r");
	if (!fp) {
		/* Exception for dnsomatic's special global hostname */
		if (nonslookup || !strncmp(alias->name, "all.dnsomatic.com", sizeof(alias->name)))
			return;

		/* Try a DNS lookup of our last known IP#. */
		nslookup(alias);
	} else {
		struct stat st;
		char address[MAX_ADDRESS_LEN];

		if (fgets(address, sizeof(address), fp)) {
			logit(LOG_INFO, "Cached IP# %s for %s from previous invocation.", address, alias->name);
			strlcpy(alias->address, address, sizeof(alias->address));
		}

		/* Initialize time since last update from modification time of cache file. */
		if (!fstat(fileno(fp), &st)) {
			alias->last_update = st.st_mtime;
			logit(LOG_INFO, "Last update of %s on %s", alias->name, ctime(&st.st_mtime));
		}

		fclose(fp);
	}
}

char *cache_file(char *name, char *buf, size_t len)
{
	if (!buf || !name)
		return NULL;

	if (snprintf(buf, len, "%s/%s.cache", cache_dir, name) >= (int)len)
		logit(LOG_WARNING, "Too long name for buffer: '%s/' + '%s' + '.cache'", cache_dir, name);

	return buf;
}

/*
 * At boot, or when restarting inadyn at runtime, the memory struct holding our
 * current IP# is empty.  We want to avoid unnecessary updates of our DDNS server
 * record, since we might get locked out for abuse, so we "seed" each of the DDNS
 * records of our struct with the cached IP# from our cache file, or from a regular
 * DNS query.
 */
int read_cache_file(ddns_t *ctx)
{
	ddns_info_t *info;

	/*
	 * Clear DNS cache before querying for the IP below, this to
	 * prevent any artefacts from, e.g., nscd, which is a known
	 * problem with DDNS clients.
	 */
	res_init();

	if (!ctx)
		return RC_INVALID_POINTER;

	info = conf_info_iterator(1);
	while (info) {
		/* XXX: Possibly move this exception to each plugin */
		const char *except[] = {
			"ipv6tb@he.net",
			"default@tunnelbroker.net"
		};
		const char *name = info->system->name;
		size_t i, j;
		int nonslookup = 0;

		/* Exceptions -- no name to lookup */
		for (i = 0; i < NELEMS(except); i++) {
			if (!strncmp(name, except[i], strlen(name))) {
				nonslookup = 1;
				break;
			}
		}

// XXX: TODO better plugin identifiction here
		for (j = 0; j < info->alias_count; j++)
			read_one(&info->alias[j], nonslookup);

		info = conf_info_iterator(0);
	}

	return 0;
}

/*
 * Update cache with new IP
 * /var/cache/inadyn/my.server.name.cache { LAST-IPADDR } MTIME
 */
int write_cache_file(ddns_alias_t *alias)
{
	FILE *fp;
	char path[256];

	cache_file(alias->name, path, sizeof(path));
	fp = fopen(path, "w");
	if (fp) {
		logit(LOG_NOTICE, "Updating cache for %s", alias->name);
		fprintf(fp, "%s", alias->address);
		fclose(fp);

		return 0;
	}

	return 1;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
