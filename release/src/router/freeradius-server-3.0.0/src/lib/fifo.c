/*
 * fifo.c	Non-thread-safe fifo (FIFO) implementation, based
 *		on hash tables.
 *
 * Version:	$Id$
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 *  Copyright 2005,2006  The FreeRADIUS server project
 *  Copyright 2005  Alan DeKok <aland@ox.org>
 */

RCSID("$Id$")

#include <freeradius-devel/libradius.h>

struct fr_fifo_t {
	int num;
	int first, last;
	int max;
	fr_fifo_free_t freeNode;

	void *data[1];
};


fr_fifo_t *fr_fifo_create(int max, fr_fifo_free_t freeNode)
{
	fr_fifo_t *fi;

	if ((max < 2) || (max > (1024 * 1024))) return NULL;

	fi = malloc(sizeof(*fi) + (sizeof(fi->data[0])*max));
	if (!fi) return NULL;

	memset(fi, 0, sizeof(*fi));

	fi->max = max;
	fi->first = 0;
	fi->last = 0;
	fi->num = 0;
	fi->freeNode = freeNode;

	return fi;
}

void fr_fifo_free(fr_fifo_t *fi)
{
	int i;

	if (!fi) return;

	if (fi->freeNode) {
		for (i = 0 ; i < fi->num; i++) {
			int element;

			element = i + fi->first;
			if (element > fi->max) {
				element -= fi->max;
			}

			fi->freeNode(fi->data[element]);
			fi->data[element] = NULL;
		}
	}

	memset(fi, 0, sizeof(*fi));
	free(fi);
}

int fr_fifo_push(fr_fifo_t *fi, void *data)
{
	if (!fi || !data) return 0;

	if (fi->num >= fi->max) return 0;

	fi->data[fi->last++] = data;
	if (fi->last >= fi->max) fi->last = 0;
	fi->num++;

	return 1;
}

void *fr_fifo_pop(fr_fifo_t *fi)
{
	void *data;

	if (!fi || (fi->num == 0)) return NULL;

	data = fi->data[fi->first++];

	if (fi->first >= fi->max) {
		fi->first = 0;
	}
	fi->num--;

	return data;
}

void *fr_fifo_peek(fr_fifo_t *fi)
{
	if (!fi || (fi->num == 0)) return NULL;

	return fi->data[fi->first];
}

int fr_fifo_num_elements(fr_fifo_t *fi)
{
	if (!fi) return 0;

	return fi->num;
}

#ifdef TESTING

/*
 *  cc -DTESTING -I .. fifo.c -o fifo
 *
 *  ./fifo
 */

#define MAX 1024
int main(int argc, char **argv)
{
	int i, j, array[MAX];
	fr_fifo_t *fi;

	fi = fr_fifo_create(MAX, NULL);
	if (!fi) fr_exit(1);

	for (j = 0; j < 5; j++) {
#define SPLIT (MAX/3)
#define COUNT ((j * SPLIT) + i)
		for (i = 0; i < SPLIT; i++) {
			array[COUNT % MAX] = COUNT;

			if (!fr_fifo_push(fi, &array[COUNT % MAX])) {
				fprintf(stderr, "%d %d\tfailed pushing %d\n",
					j, i, COUNT);
				fr_exit(2);
			}

			if (fr_fifo_num_elements(fi) != (i + 1)) {
				fprintf(stderr, "%d %d\tgot size %d expected %d\n",
					j, i, i + 1, fr_fifo_num_elements(fi));
				fr_exit(1);
			}
		}

		if (fr_fifo_num_elements(fi) != SPLIT) {
			fprintf(stderr, "HALF %d %d\n",
				fr_fifo_num_elements(fi), SPLIT);
			fr_exit(1);
		}

		for (i = 0; i < SPLIT; i++) {
			int *p;

			p = fr_fifo_pop(fi);
			if (!p) {
				fprintf(stderr, "No pop at %d\n", i);
				fr_exit(3);
			}

			if (*p != COUNT) {
				fprintf(stderr, "%d %d\tgot %d expected %d\n",
					j, i, *p, COUNT);
				fr_exit(4);
			}

			if (fr_fifo_num_elements(fi) != SPLIT - (i + 1)) {
				fprintf(stderr, "%d %d\tgot size %d expected %d\n",
					j, i, SPLIT - (i + 1), fr_fifo_num_elements(fi));
				fr_exit(1);
			}
		}

		if (fr_fifo_num_elements(fi) != 0) {
			fprintf(stderr, "ZERO %d %d\n",
				fr_fifo_num_elements(fi), 0);
			fr_exit(1);
		}
	}

	fr_fifo_free(fi);

	fr_exit(0);
}
#endif
