/* libcap-ng.h --
 * Copyright 2009,2013,2020,2022 Red Hat Inc.
 * All Rights Reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors:
 *      Steve Grubb <sgrubb@redhat.com>
 */

#ifndef LIBCAP_NG_HEADER
#define LIBCAP_NG_HEADER

#include <stdint.h>
#include <linux/capability.h>
#include <unistd.h>

#ifndef __attr_dealloc
# define __attr_dealloc(dealloc, argno)
# define __attr_dealloc_free
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {  CAPNG_DROP, CAPNG_ADD } capng_act_t;
typedef enum {  CAPNG_EFFECTIVE=1, CAPNG_PERMITTED=2,
		CAPNG_INHERITABLE=4, CAPNG_BOUNDING_SET=8,
		CAPNG_AMBIENT=16 } capng_type_t;
typedef enum {  CAPNG_SELECT_CAPS = 16, CAPNG_SELECT_BOUNDS = 32,
		CAPNG_SELECT_BOTH = 48, CAPNG_SELECT_AMBIENT = 64,
		CAPNG_SELECT_ALL = 112 } capng_select_t;
typedef enum {	CAPNG_FAIL=-1, CAPNG_NONE, CAPNG_PARTIAL,
		CAPNG_FULL } capng_results_t;
typedef enum {  CAPNG_PRINT_STDOUT, CAPNG_PRINT_BUFFER } capng_print_t;
typedef enum {  CAPNG_NO_FLAG=0, CAPNG_DROP_SUPP_GRP=1,
		CAPNG_CLEAR_BOUNDING=2, CAPNG_INIT_SUPP_GRP=4,
		CAPNG_CLEAR_AMBIENT=8 } capng_flags_t;

#define CAPNG_UNSET_ROOTID -1
#define CAPNG_SUPPORTS_AMBIENT 1

// These functions manipulate process capabilities
void capng_clear(capng_select_t set);
void capng_fill(capng_select_t set);
void capng_setpid(int pid);
int capng_get_caps_process(void);
int capng_update(capng_act_t action, capng_type_t type,unsigned int capability);
int capng_updatev(capng_act_t action, capng_type_t type,
		unsigned int capability, ...);

// These functions apply the capabilities previously setup to a process
int capng_apply(capng_select_t set);
int capng_lock(void);
int capng_change_id(int uid, int gid, capng_flags_t flag);

// These functions are used for file based capabilities
int capng_get_rootid(void);
int capng_set_rootid(int rootid);
int capng_get_caps_fd(int fd);
int capng_apply_caps_fd(int fd);

// These functions check capability bits
capng_results_t capng_have_capabilities(capng_select_t set);
capng_results_t capng_have_permitted_capabilities(void);
int capng_have_capability(capng_type_t which, unsigned int capability);

// These functions printout capabilities
char *capng_print_caps_numeric(capng_print_t where, capng_select_t set)
	__attr_dealloc_free;
char *capng_print_caps_text(capng_print_t where, capng_type_t which)
	__attr_dealloc_free;

// These functions convert between numeric and text string
int capng_name_to_capability(const char *name);
const char *capng_capability_to_name(unsigned int capability);

// These function should be used when you suspect a third party library
// may use libcap-ng also and want to make sure it doesn't alter something
// important. Otherwise you shouldn't need to call these.
void capng_restore_state(void **state);
void *capng_save_state(void);

#ifdef __cplusplus
}
#endif


#endif
