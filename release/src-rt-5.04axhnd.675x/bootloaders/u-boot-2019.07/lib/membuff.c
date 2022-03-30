// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * Copyright (c) 1992 Simon Glass
 */

#include <common.h>
#include <errno.h>
#include <malloc.h>
#include "membuff.h"

void membuff_purge(struct membuff *mb)
{
	/* set mb->head and mb->tail so the buffers look empty */
	mb->head = mb->start;
	mb->tail = mb->start;
}

static int membuff_putrawflex(struct membuff *mb, int maxlen, bool update,
			      char ***data, int *offsetp)
{
	int len;

	/* always write to 'mb->head' */
	assert(data && offsetp);
	*data = &mb->start;
	*offsetp = mb->head - mb->start;

	/* if there is no buffer, we can do nothing */
	if (!mb->start)
		return 0;

	/*
	 * if head is ahead of tail, we can write from head until the end of
	 * the buffer
	 */
	if (mb->head >= mb->tail) {
		/* work out how many bytes can fit here */
		len = mb->end - mb->head - 1;
		if (maxlen >= 0 && len > maxlen)
			len = maxlen;

		/* update the head pointer to mark these bytes as written */
		if (update)
			mb->head += len;

		/*
		 * if the tail isn't at start of the buffer, then we can
		 * write one more byte right at the end
		 */
		if ((maxlen < 0 || len < maxlen) && mb->tail != mb->start) {
			len++;
			if (update)
				mb->head = mb->start;
		}

	/* otherwise now we can write until head almost reaches tail */
	} else {
		/* work out how many bytes can fit here */
		len = mb->tail - mb->head - 1;
		if (maxlen >= 0 && len > maxlen)
			len = maxlen;

		/* update the head pointer to mark these bytes as written */
		if (update)
			mb->head += len;
	}

	/* return the number of bytes which can be/must be written */
	return len;
}

int membuff_putraw(struct membuff *mb, int maxlen, bool update, char **data)
{
	char **datap;
	int offset;
	int size;

	size = membuff_putrawflex(mb, maxlen, update, &datap, &offset);
	*data = *datap + offset;

	return size;
}

bool membuff_putbyte(struct membuff *mb, int ch)
{
	char *data;

	if (membuff_putraw(mb, 1, true, &data) != 1)
		return false;
	*data = ch;

	return true;
}

int membuff_getraw(struct membuff *mb, int maxlen, bool update, char **data)
{
	int len;

	/* assume for now there is no data to get */
	len = 0;

	/*
	 * in this case head is ahead of tail, so we must return data between
	 *'tail' and 'head'
	 */
	if (mb->head > mb->tail) {
		/* work out the amount of data */
		*data = mb->tail;
		len = mb->head - mb->tail;

		/* check it isn't too much */
		if (maxlen >= 0 && len > maxlen)
			len = maxlen;

		/* & mark it as read from the buffer */
		if (update)
			mb->tail += len;
	}

	/*
	 * if head is before tail, then we have data between 'tail' and 'end'
	 * and some more data between 'start' and 'head'(which we can't
	 * return this time
	 */
	else if (mb->head < mb->tail) {
		/* work out the amount of data */
		*data = mb->tail;
		len = mb->end - mb->tail;
		if (maxlen >= 0 && len > maxlen)
			len = maxlen;
		if (update) {
			mb->tail += len;
			if (mb->tail == mb->end)
				mb->tail = mb->start;
		}
	}

	debug("getraw: maxlen=%d, update=%d, head=%d, tail=%d, data=%d, len=%d",
	      maxlen, update, (int)(mb->head - mb->start),
	      (int)(mb->tail - mb->start), (int)(*data - mb->start), len);

	/* return the number of bytes we found */
	return len;
}

int membuff_getbyte(struct membuff *mb)
{
	char *data = 0;

	return membuff_getraw(mb, 1, true, &data) != 1 ? -1 : *(uint8_t *)data;
}

int membuff_peekbyte(struct membuff *mb)
{
	char *data = 0;

	return membuff_getraw(mb, 1, false, &data) != 1 ? -1 : *(uint8_t *)data;
}

int membuff_get(struct membuff *mb, char *buff, int maxlen)
{
	char *data = 0, *buffptr = buff;
	int len = 1, i;

	/*
	 * do this in up to two lots(see GetRaw for why) stopping when there
	 * is no more data
	 */
	for (i = 0; len && i < 2; i++) {
		/* get a pointer to the data available */
		len = membuff_getraw(mb, maxlen, true, &data);

		/* copy it into the buffer */
		memcpy(buffptr, data, len);
		buffptr += len;
		maxlen -= len;
	}

	/* return the number of bytes read */
	return buffptr - buff;
}

int membuff_put(struct membuff *mb, const char *buff, int length)
{
	char *data;
	int towrite, i, written;

	for (i = written = 0; i < 2; i++) {
		/* ask where some data can be written */
		towrite = membuff_putraw(mb, length, true, &data);

		/* and write it, updating the bytes length */
		memcpy(data, buff, towrite);
		written += towrite;
		buff += towrite;
		length -= towrite;
	}

	/* return the number of bytes written */
	return written;
}

bool membuff_isempty(struct membuff *mb)
{
	return mb->head == mb->tail;
}

int membuff_avail(struct membuff *mb)
{
	struct membuff copy;
	int i, avail;
	char *data = 0;

	/* make a copy of this buffer's control data */
	copy = *mb;

	/* now read everything out of the copied buffer */
	for (i = avail = 0; i < 2; i++)
		avail += membuff_getraw(&copy, -1, true, &data);

	/* and return how much we read */
	return avail;
}

int membuff_size(struct membuff *mb)
{
	return mb->end - mb->start;
}

bool membuff_makecontig(struct membuff *mb)
{
	int topsize, botsize;

	debug("makecontig: head=%d, tail=%d, size=%d",
	      (int)(mb->head - mb->start), (int)(mb->tail - mb->start),
	      (int)(mb->end - mb->start));

	/*
	 * first we move anything at the start of the buffer into the correct
	 * place some way along
	 */
	if (mb->tail > mb->head) {
		/*
		 * the data is split into two parts, from 0 to ->head and
		 * from ->tail to ->end. We move the stuff from 0 to ->head
		 * up to make space for the other data before it
		 */
		topsize = mb->end - mb->tail;
		botsize = mb->head - mb->start;

		/*
		 * must move data at bottom up by 'topsize' bytes - check if
		 * there's room
		 */
		if (mb->head + topsize >= mb->tail)
			return false;
		memmove(mb->start + topsize, mb->start, botsize);
		debug("	- memmove(%d, %d, %d)", topsize, 0, botsize);

	/* nothing at the start, so skip that step */
	} else {
		topsize = mb->head - mb->tail;
		botsize = 0;
	}

	/* now move data at top down to the bottom */
	memcpy(mb->start, mb->tail, topsize);
	debug("	- memcpy(%d, %d, %d)", 0, (int)(mb->tail - mb->start), topsize);

	/* adjust pointers */
	mb->tail = mb->start;
	mb->head = mb->start + topsize + botsize;

	debug("	- head=%d, tail=%d", (int)(mb->head - mb->start),
	      (int)(mb->tail - mb->start));

	/* all ok */
	return true;
}

int membuff_free(struct membuff *mb)
{
	return mb->end == mb->start ? 0 :
			(mb->end - mb->start) - 1 - membuff_avail(mb);
}

int membuff_readline(struct membuff *mb, char *str, int maxlen, int minch)
{
	int len;  /* number of bytes read (!= string length) */
	char *s, *end;
	bool ok = false;
	char *orig = str;

	end = mb->head >= mb->tail ? mb->head : mb->end;
	for (len = 0, s = mb->tail; s < end && len < maxlen - 1; str++) {
		*str = *s++;
		len++;
		if (*str == '\n' || *str < minch) {
			ok = true;
			break;
		}
		if (s == end && mb->tail > mb->head) {
			s = mb->start;
			end = mb->head;
		}
	}

	/* couldn't get the whole string */
	if (!ok) {
		if (maxlen)
			*orig = '\0';
		return 0;
	}

	/* terminate the string, update the membuff and return success */
	*str = '\0';
	mb->tail = s == mb->end ? mb->start : s;

	return len;
}

int membuff_extend_by(struct membuff *mb, int by, int max)
{
	int oldhead, oldtail;
	int size, orig;
	char *ptr;

	/* double the buffer size until it is big enough */
	assert(by >= 0);
	for (orig = mb->end - mb->start, size = orig; size < orig + by;)
		size *= 2;
	if (max != -1)
		size = min(size, max);
	by = size - orig;

	/* if we're already at maximum, give up */
	if (by <= 0)
		return -E2BIG;

	oldhead = mb->head - mb->start;
	oldtail = mb->tail - mb->start;
	ptr = realloc(mb->start, size);
	if (!ptr)
		return -ENOMEM;
	mb->start = ptr;
	mb->head = mb->start + oldhead;
	mb->tail = mb->start + oldtail;

	if (mb->head < mb->tail) {
		memmove(mb->tail + by, mb->tail, orig - oldtail);
		mb->tail += by;
	}
	mb->end = mb->start + size;

	return 0;
}

void membuff_init(struct membuff *mb, char *buff, int size)
{
	mb->start = buff;
	mb->end = mb->start + size;
	membuff_purge(mb);
}

int membuff_new(struct membuff *mb, int size)
{
	mb->start = malloc(size);
	if (!mb->start)
		return -ENOMEM;

	membuff_init(mb, mb->start, size);
	return 0;
}

void membuff_uninit(struct membuff *mb)
{
	mb->end = NULL;
	mb->start = NULL;
	membuff_purge(mb);
}

void membuff_dispose(struct membuff *mb)
{
	free(&mb->start);
	membuff_uninit(mb);
}
