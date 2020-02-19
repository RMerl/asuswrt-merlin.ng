/*
 * Project: udptunnel
 * File: list.h
 *
 * Copyright (C) 2009 Daniel Meekins
 * Contact: dmeekins - gmail
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIST_H
#define LIST_H

//#include "common.h"

#ifdef WIN32
#define _inline_ __inline
#else
#define _inline_ inline
#endif

#include <pj/types.h>

#ifdef __cplusplus
extern "C" {
#endif


#define LIST_INIT_SIZE 10 /* Start off an array with 10 elements */

typedef struct list {
    void **obj_arr; /* Array of pointers to each object */
    size_t obj_sz;  /* Number of bytes each individual objects takes up */
    int num_objs;   /* Number of object pointers in the array */
    int length;     /* Actual length of the pointer array */
    int sort;       /* 0 - don't sort, 1 - keep sorted */
    /* Function pointers to use for specific type of data types */
    int (*obj_cmp)(const void *, const void *, size_t);
    void* (*obj_copy)(void *, const void *, size_t);
    void (*obj_free)(void **);
#ifdef USE_DISCONNECT_LOCK
	// +Roger - UDT disconnect mutex
	pj_mutex_t *disconn_lock;
#endif

} natnl_list_t;

#define LIST_LEN(l) ((l)->num_objs)

natnl_list_t *natnl_list_create(int inst_id, int call_id, 
					int obj_sz,
                    int (*obj_cmp)(const void *, const void *, size_t),
                    void* (*obj_copy)(void *, const void *, size_t),
					void (*obj_free)(void **), int sort);
natnl_list_t *natnl_list_create2(int inst_id, int call_id, 
					 int obj_sz,
					int (*obj_cmp)(const void *, const void *, size_t),
					void* (*obj_copy)(void *, const void *, size_t),
					void (*obj_free)(void **), int sort, int init_size);
void *natnl_list_add(natnl_list_t *list, void *obj, int copy);
void *natnl_list_add2(natnl_list_t *list, void *obj, int copy, int check_exists);
void *natnl_list_get(natnl_list_t *list, void *obj);
void *natnl_list_get_at(natnl_list_t *list, int i);
int natnl_list_get_index(natnl_list_t *list, void *obj);
natnl_list_t *natnl_list_copy(int inst_id, int call_id, natnl_list_t *src);
void natnl_list_action(natnl_list_t *list, void (*action)(void *));
void natnl_list_delete(natnl_list_t *list, void *obj);
void natnl_list_delete_at(natnl_list_t *list, int i);
void natnl_list_free(natnl_list_t **list);

static _inline_ int int_cmp(int *i, int *j, size_t sz)
{
    return *i - *j;
}

#define p_int_cmp ((int (*)(const void *, const void *, size_t))&int_cmp)

#ifdef __cplusplus
}
#endif

#endif /* LIST_H */

