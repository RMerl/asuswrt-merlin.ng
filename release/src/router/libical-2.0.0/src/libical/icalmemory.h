/*======================================================================
 FILE: icalmemory.h
 CREATOR: eric 30 June 1999

 (C) COPYRIGHT 2000, Eric Busboom <eric@softwarestudio.org>
     http://www.softwarestudio.org

 This program is free software; you can redistribute it and/or modify
 it under the terms of either:

    The LGPL as published by the Free Software Foundation, version
    2.1, available at: http://www.gnu.org/licenses/lgpl-2.1.html

 Or:

    The Mozilla Public License Version 1.0. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/

 The Initial Developer of the Original Code is Eric Busboom
======================================================================*/

#ifndef ICALMEMORY_H
#define ICALMEMORY_H

/* Tmp buffers are managed by ical. References can be returned to the
   caller, although the caller will not own the memory. */

#include "libical_ical_export.h"

LIBICAL_ICAL_EXPORT void *icalmemory_tmp_buffer(size_t size);
LIBICAL_ICAL_EXPORT char *icalmemory_tmp_copy(const char *str);

/** Add an externally allocated buffer to the ring. */
LIBICAL_ICAL_EXPORT void icalmemory_add_tmp_buffer(void *);

/** Free all memory used in the ring */
LIBICAL_ICAL_EXPORT void icalmemory_free_ring(void);

/* Non-tmp buffers must be freed. These are mostly wrappers around
 * malloc, etc, but are used so the caller can change the memory
 * allocators in a future version of the library */

LIBICAL_ICAL_EXPORT void *icalmemory_new_buffer(size_t size);
LIBICAL_ICAL_EXPORT void *icalmemory_resize_buffer(void *buf, size_t size);
LIBICAL_ICAL_EXPORT void icalmemory_free_buffer(void *buf);

/**
   icalmemory_append_string will copy the string 'string' to the
   buffer 'buf' starting at position 'pos', reallocing 'buf' if it is
   too small. 'buf_size' is the size of 'buf' and will be changed if
   'buf' is reallocated. 'pos' will point to the last byte of the new
   string in 'buf', usually a '\0' */

/* THESE ROUTINES CAN NOT BE USED ON TMP BUFFERS. Only use them on
   normally allocated memory, or on buffers created from
   icalmemory_new_buffer, never with buffers created by
   icalmemory_tmp_buffer. If icalmemory_append_string has to resize a
   buffer on the ring, the ring will loose track of it an you will
   have memory problems. */

LIBICAL_ICAL_EXPORT void icalmemory_append_string(char **buf, char **pos, size_t *buf_size,
                                                  const char *string);

/**  icalmemory_append_char is similar, but is appends a character instead of a string */
LIBICAL_ICAL_EXPORT void icalmemory_append_char(char **buf, char **pos, size_t *buf_size, char ch);

/** A wrapper around strdup. Partly to trap calls to strdup, partly
    because in -ansi, gcc on Red Hat claims that strdup is undeclared */
LIBICAL_ICAL_EXPORT char *icalmemory_strdup(const char *s);

#endif /* !ICALMEMORY_H */
