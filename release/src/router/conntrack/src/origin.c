/*
 * (C) 2009 by Pablo Neira Ayuso <pablo@netfilter.org>
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
#include "conntrackd.h"
#include "origin.h"

static LIST_HEAD(origin_list);

struct origin {
	struct list_head	head;
	unsigned int		nl_portid;
	int			type;
};

/* register a Netlink socket as origin of possible events */
int origin_register(struct nfct_handle *h, int origin_type)
{
	struct origin *nlp;

	nlp = calloc(sizeof(struct origin), 1);
	if (nlp == NULL)
		return -1;

	nlp->nl_portid = nfnl_portid(nfct_nfnlh(h));
	nlp->type = origin_type;

	list_add(&nlp->head, &origin_list);
	return 0;
}

/* look up for the origin of this Netlink event */
int origin_find(const struct nlmsghdr *nlh)
{
	struct origin *this;

	list_for_each_entry(this, &origin_list, head) {
		if (this->nl_portid == nlh->nlmsg_pid) {
			return this->type;
		}
	}
	return CTD_ORIGIN_NOT_ME;
}

int origin_unregister(struct nfct_handle *h)
{
	struct origin *this, *tmp;

	list_for_each_entry_safe(this, tmp, &origin_list, head) {
		if (this->nl_portid == nfnl_portid(nfct_nfnlh(h))) {
			list_del(&this->head);
			free(this);
			return 1;
		}
	}
	return 0;
}
