/* modpriv.h: Stuff needed by both modules.c and modcall.c, but should not be
 * accessed from anywhere else.
 *
 * Version: $Id$ */
#ifndef FR_MODPRIV_H
#define FR_MODPRIV_H

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/modules.h>

#ifndef HAVE_DLFCN_H
#error FreeRADIUS needs either libltdl, or a working dlopen()
#else
#include <dlfcn.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef void *lt_dlhandle;

lt_dlhandle lt_dlopenext(char const *name);
void *lt_dlsym(lt_dlhandle handle, char const *symbol);
int lt_dlclose(lt_dlhandle handle);
char const *lt_dlerror(void);

/*
 *	Keep track of which modules we've loaded.
 */
typedef struct module_entry_t {
	char			name[MAX_STRING_LEN];
	module_t const		*module;
	lt_dlhandle		handle;
} module_entry_t;

typedef struct fr_module_hup_t fr_module_hup_t;

/*
 *	Per-instance data structure, to correlate the modules
 *	with the instance names (may NOT be the module names!),
 *	and the per-instance data structures.
 */
typedef struct module_instance_t {
	char			name[MAX_STRING_LEN];
	module_entry_t		*entry;
	void		    *insthandle;
#ifdef HAVE_PTHREAD_H
	pthread_mutex_t		*mutex;
#endif
	CONF_SECTION		*cs;
	int			dead;
	fr_module_hup_t	       	*mh;
} module_instance_t;

module_instance_t *find_module_instance(CONF_SECTION *, char const *instname,
					int do_link);
int module_hup_module(CONF_SECTION *cs, module_instance_t *node, time_t when);

#ifdef __cplusplus
}
#endif

#endif	/* FR_MODPRIV_H */
