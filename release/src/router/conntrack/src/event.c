/*
 * (C) 2006-2007 by Pablo Neira Ayuso <pablo@netfilter.org>
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
#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "event.h"

struct evfd {
	int read;
	int fds[2];
};

struct evfd *create_evfd(void)
{
	struct evfd *e;

	e = calloc(1, sizeof(struct evfd));
	if (e == NULL)
		return NULL;

	if (pipe(e->fds) == -1) {
		free(e);
		return NULL;
	}
	fcntl(e->fds[0], F_SETFL, O_NONBLOCK);

	return e;
}

void destroy_evfd(struct evfd *e)
{
	close(e->fds[0]);
	close(e->fds[1]);
	free(e);
}

int get_read_evfd(struct evfd *evfd)
{
	return evfd->fds[0];
}

int write_evfd(struct evfd *evfd)
{
	int data = 0, ret = 0;

	if (evfd->read == 0)
		ret = write(evfd->fds[1], &data, sizeof(data));
	evfd->read++;

	return ret;
}

int read_evfd(struct evfd *evfd)
{
	int data, ret = 0;

	if (--evfd->read == 0)
		ret = read(evfd->fds[0], &data, sizeof(data));
	return ret;
}
