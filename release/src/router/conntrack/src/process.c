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

#include <signal.h>
#include "conntrackd.h"
#include "process.h"

static LIST_HEAD(process_list);

int fork_process_new(int type, int flags, void (*cb)(void *data), void *data)
{
	struct child_process *c, *this;
	int pid;

	/* We only want one process of this type at the same time. This is
	 * useful if you want to prevent two child processes from accessing
	 * a shared descriptor at the same time. */
	if (flags & CTD_PROC_F_EXCL) {
		list_for_each_entry(this, &process_list, head) {
			if (this->type == type) {
				return -1;
			}
		}
	}
	c = calloc(sizeof(struct child_process), 1);
	if (c == NULL)
		return -1;

	c->type = type;
	c->cb = cb;
	c->data = data;
	c->pid = pid = fork();

	if (c->pid > 0)
		list_add(&c->head, &process_list);
	else
		free(c);

	return pid;
}

int fork_process_delete(int pid)
{
	struct child_process *this, *tmp;

	list_for_each_entry_safe(this, tmp, &process_list, head) {
		if (this->pid == pid) {
			list_del(&this->head);
			if (this->cb) {
				this->cb(this->data);
			}
			free(this);
			return 1;
		}
	}
	return 0;
}

static const char *process_type_to_name[CTD_PROC_MAX] = {
	[CTD_PROC_ANY]		= "any",
	[CTD_PROC_FLUSH]	= "flush",
	[CTD_PROC_COMMIT]	= "commit",
};

void fork_process_dump(int fd)
{
	struct child_process *this;
	char buf[4096];
	int size = 0;

	list_for_each_entry(this, &process_list, head) {
		size += snprintf(buf+size, sizeof(buf),
				 "PID=%u type=%s\n",
				 this->pid,
				 this->type < CTD_PROC_MAX ?
				 process_type_to_name[this->type] : "unknown");
	}

	send(fd, buf, size, 0);
}
