/*
 * (C) 2006 by Pablo Neira Ayuso <pablo@netfilter.org>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "traffic_stats.h"
#include "conntrackd.h"

void update_traffic_stats(struct nf_conntrack *ct)
{
	STATE(stats).bytes_orig +=
		nfct_get_attr_u32(ct, ATTR_ORIG_COUNTER_BYTES);
	STATE(stats).bytes_repl +=
		nfct_get_attr_u32(ct, ATTR_REPL_COUNTER_BYTES);
	STATE(stats).packets_orig += 
		nfct_get_attr_u32(ct, ATTR_ORIG_COUNTER_PACKETS);
	STATE(stats).packets_repl +=
		nfct_get_attr_u32(ct, ATTR_REPL_COUNTER_PACKETS);
}

void dump_traffic_stats(int fd)
{
	char buf[512];
	int size;
	uint64_t bytes = STATE(stats).bytes_orig + STATE(stats).bytes_repl;
	uint64_t packets = STATE(stats).packets_orig +
			   STATE(stats).packets_repl;

	size = sprintf(buf, "traffic processed:\n");
	size += sprintf(buf+size, "%20llu Bytes      ", (unsigned long long)bytes);
	size += sprintf(buf+size, "%20llu Pckts\n\n", (unsigned long long)packets);

	send(fd, buf, size, 0);
}
