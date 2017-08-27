/**
 * \file dlmisc.c
 * \brief dynamic loader helpers
 * \author Jaroslav Kysela <perex@perex.cz>
 * \date 2001
 *
 * Dynamic loader helpers
 */
/*
 *  Dynamic loader helpers
 *  Copyright (c) 2000 by Jaroslav Kysela <perex@perex.cz>
 *
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as
 *   published by the Free Software Foundation; either version 2.1 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#include "list.h"
#include "local.h"

#ifndef DOC_HIDDEN
#ifndef PIC
struct snd_dlsym_link *snd_dlsym_start = NULL;
#endif
#endif

/**
 * \brief Opens a dynamic library - ALSA wrapper for \c dlopen.
 * \param name name of the library, similar to \c dlopen.
 * \param mode mode flags, similar to \c dlopen.
 * \return Library handle if successful, otherwise \c NULL.
 *
 * This function can emulate dynamic linking for the static build of
 * the alsa-lib library. In that case, \p name is set to \c NULL.
 */
void *snd_dlopen(const char *name, int mode)
{
#ifndef PIC
	if (name == NULL)
		return &snd_dlsym_start;
#else
#ifdef HAVE_LIBDL
	if (name == NULL) {
		static const char * self = NULL;
		if (self == NULL) {
			Dl_info dlinfo;
			if (dladdr(snd_dlopen, &dlinfo) > 0)
				self = dlinfo.dli_fname;
		}
		name = self;
	}
#endif
#endif
#ifdef HAVE_LIBDL
	return dlopen(name, mode);
#else
	return NULL;
#endif
}

/**
 * \brief Closes a dynamic library - ALSA wrapper for \c dlclose.
 * \param handle Library handle, similar to \c dlclose.
 * \return Zero if successful, otherwise an error code.
 *
 * This function can emulate dynamic linking for the static build of
 * the alsa-lib library.
 */
int snd_dlclose(void *handle)
{
#ifndef PIC
	if (handle == &snd_dlsym_start)
		return 0;
#endif
#ifdef HAVE_LIBDL
	return dlclose(handle);
#else
	return 0;
#endif
}

/**
 * \brief Verifies a dynamically loaded symbol.
 * \param handle Library handle, similar to \c dlsym.
 * \param name Symbol name.
 * \param version Version of the symbol.
 * \return Zero is successful, otherwise a negative error code.
 *
 * This function checks that the symbol with the version appended to its name
 * does exist in the library.
 */
static int snd_dlsym_verify(void *handle, const char *name, const char *version)
{
#ifdef HAVE_LIBDL
	int res;
	char *vname;
	
	if (handle == NULL)
		return -EINVAL;
	vname = alloca(1 + strlen(name) + strlen(version) + 1);
	if (vname == NULL)
		return -ENOMEM;
	vname[0] = '_';
	strcpy(vname + 1, name);
	strcat(vname, version);
	res = dlsym(handle, vname) == NULL ? -ENOENT : 0;
	// printf("dlsym verify: %i, vname = '%s'\n", res, vname);
	if (res < 0)
		SNDERR("unable to verify version for symbol %s", name);
	return res;
#else
	return 0;
#endif
}

/**
 * \brief Resolves a symbol from a dynamic library - ALSA wrapper for \c dlsym.
 * \param handle Library handle, similar to \c dlsym.
 * \param name Symbol name.
 * \param version Version of the symbol.
 *
 * This function can emulate dynamic linking for the static build of
 * the alsa-lib library.
 *
 * This special version of the \c dlsym function checks also the version
 * of the symbol. A versioned symbol should be defined using the
 * #SND_DLSYM_BUILD_VERSION macro.
 */
void *snd_dlsym(void *handle, const char *name, const char *version)
{
	int err;

#ifndef PIC
	if (handle == &snd_dlsym_start) {
		/* it's the funny part: */
		/* we are looking for a symbol in a static library */
		struct snd_dlsym_link *link = snd_dlsym_start;
		while (link) {
			if (!strcmp(name, link->dlsym_name))
				return (void *)link->dlsym_ptr;
			link = link->next;
		}
		return NULL;
	}
#endif
#ifdef HAVE_LIBDL
	if (version) {
		err = snd_dlsym_verify(handle, name, version);
		if (err < 0)
			return NULL;
	}
	return dlsym(handle, name);
#else
	return NULL;
#endif
}


#ifndef DOC_HIDDEN
struct dlobj_cache {
	const char *name;
	void *obj;
	void *func;
	struct list_head list;
};

static LIST_HEAD(pcm_dlobj_list);

void *snd_dlobj_cache_lookup(const char *name)
{
	struct list_head *p;
	struct dlobj_cache *c;

	list_for_each(p, &pcm_dlobj_list) {
		c = list_entry(p, struct dlobj_cache, list);
		if (strcmp(c->name, name) == 0)
			return c->func;
	}
	return NULL;
}

int snd_dlobj_cache_add(const char *name, void *dlobj, void *open_func)
{
	struct list_head *p;
	struct dlobj_cache *c;

	list_for_each(p, &pcm_dlobj_list) {
		c = list_entry(p, struct dlobj_cache, list);
		if (strcmp(c->name, name) == 0)
			return 0; /* already exists */
	}
	c = malloc(sizeof(*c));
	if (! c)
		return -ENOMEM;
	c->name = strdup(name);
	if (! c->name) {
		free(c);
		return -ENOMEM;
	}
	c->obj = dlobj;
	c->func = open_func;
	list_add_tail(&c->list, &pcm_dlobj_list);
	return 0;
}

void snd_dlobj_cache_cleanup(void)
{
	struct list_head *p;
	struct dlobj_cache *c;

	while (! list_empty(&pcm_dlobj_list)) {
		p = pcm_dlobj_list.next;
		c = list_entry(p, struct dlobj_cache, list);
		list_del(p);
		snd_dlclose(c->obj);
		free((void *)c->name); /* shut up gcc warning */
		free(c);
	}
}
#endif
