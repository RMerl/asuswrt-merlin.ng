/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2003
 * Gerry Hamel, geh@ti.com, Texas Instruments
 */

#ifndef __CIRCBUF_H__
#define __CIRCBUF_H__

typedef struct circbuf {
	unsigned int size;	/* current number of bytes held */
	unsigned int totalsize; /* number of bytes allocated */

	char *top;		/* pointer to current buffer start */
	char *tail;		/* pointer to space for next element */

	char *data;		/* all data */
	char *end;		/* end of data buffer */
} circbuf_t;

int buf_init (circbuf_t * buf, unsigned int size);
int buf_free (circbuf_t * buf);
int buf_pop (circbuf_t * buf, char *dest, unsigned int len);
int buf_push (circbuf_t * buf, const char *src, unsigned int len);

#endif
