/*======================================================================
 FILE: pvl.h
 CREATOR: eric November, 1995

 (C) COPYRIGHT 2000, Eric Busboom <eric@softwarestudio.org>
     http://www.softwarestudio.org

 This program is free software; you can redistribute it and/or modify
 it under the terms of either:

    The LGPL as published by the Free Software Foundation, version
    2.1, available at: http://www.gnu.org/licenses/lgpl-2.1.html

 Or:

    The Mozilla Public License Version 1.0. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/
======================================================================*/

#ifndef ICAL_PVL_H
#define ICAL_PVL_H

#include "libical_ical_export.h"

typedef struct pvl_list_t *pvl_list;
typedef struct pvl_elem_t *pvl_elem;

/**
 * This type is private. Always use pvl_elem instead. The struct would
 * not even appear in this header except to make code in the USE_MACROS
 * blocks work
 */

typedef struct pvl_elem_t
{
    int MAGIC;                          /**< Magic Identifier */
    void *d;                            /**< Pointer to data user is storing */
    struct pvl_elem_t *next;            /**< Next element */
    struct pvl_elem_t *prior;           /**< Prior element */
} pvl_elem_t;

/* Create new lists or elements */
LIBICAL_ICAL_EXPORT pvl_elem pvl_new_element(void *d, pvl_elem next, pvl_elem prior);

LIBICAL_ICAL_EXPORT pvl_list pvl_newlist(void);

LIBICAL_ICAL_EXPORT void pvl_free(pvl_list);

/* Add, remove, or get the head of the list */
LIBICAL_ICAL_EXPORT void pvl_unshift(pvl_list l, void *d);

LIBICAL_ICAL_EXPORT void *pvl_shift(pvl_list l);

LIBICAL_ICAL_EXPORT pvl_elem pvl_head(pvl_list);

/* Add, remove or get the tail of the list */
LIBICAL_ICAL_EXPORT void pvl_push(pvl_list l, void *d);

LIBICAL_ICAL_EXPORT void *pvl_pop(pvl_list l);

LIBICAL_ICAL_EXPORT pvl_elem pvl_tail(pvl_list);

/* Insert elements in random places */
typedef int (*pvl_comparef) (void *a, void *b); /* a, b are of the data type */

LIBICAL_ICAL_EXPORT void pvl_insert_ordered(pvl_list l, pvl_comparef f, void *d);

LIBICAL_ICAL_EXPORT void pvl_insert_after(pvl_list l, pvl_elem e, void *d);

LIBICAL_ICAL_EXPORT void pvl_insert_before(pvl_list l, pvl_elem e, void *d);

/* Remove an element, or clear the entire list */
LIBICAL_ICAL_EXPORT void *pvl_remove(pvl_list, pvl_elem);       /* Remove element, return data */

LIBICAL_ICAL_EXPORT void pvl_clear(pvl_list);   /* Remove all elements, de-allocate all data */

LIBICAL_ICAL_EXPORT int pvl_count(pvl_list);

/* Navagate the list */
LIBICAL_ICAL_EXPORT pvl_elem pvl_next(pvl_elem e);

LIBICAL_ICAL_EXPORT pvl_elem pvl_prior(pvl_elem e);

/* get the data in the list */
#if !defined(PVL_USE_MACROS)
LIBICAL_ICAL_EXPORT void *pvl_data(pvl_elem);
#else
#define pvl_data(x) x==0 ? 0 : ((struct pvl_elem_t *)x)->d;
#endif

/* Find an element for which a function returns true */
typedef int (*pvl_findf) (void *a, void *b);    /*a is list elem, b is other data */

LIBICAL_ICAL_EXPORT pvl_elem pvl_find(pvl_list l, pvl_findf f, void *v);

LIBICAL_ICAL_EXPORT pvl_elem pvl_find_next(pvl_list l, pvl_findf f, void *v);

/**
 * Pass each element in the list to a function
 * a is list elem, b is other data
 */
typedef void (*pvl_applyf) (void *a, void *b);

LIBICAL_ICAL_EXPORT void pvl_apply(pvl_list l, pvl_applyf f, void *v);

#endif /* ICAL_PVL_H */
