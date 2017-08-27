/*
 * Copyright (C) 2009 Martin Willi
 * Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#define _GNU_SOURCE

#include "integrity_checker.h"

#include <dlfcn.h>
#include <link.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "debug.h"
#include "library.h"

typedef struct private_integrity_checker_t private_integrity_checker_t;

/**
 * Private data of an integrity_checker_t object.
 */
struct private_integrity_checker_t {

	/**
	 * Public integrity_checker_t interface.
	 */
	integrity_checker_t public;

	/**
	 * dlopen handle to checksum library
	 */
	void *handle;

	/**
	 * checksum array
	 */
	integrity_checksum_t *checksums;

	/**
	 * number of checksums in array
	 */
	int checksum_count;
};

METHOD(integrity_checker_t, build_file, u_int32_t,
	private_integrity_checker_t *this, char *file, size_t *len)
{
	u_int32_t checksum;
	chunk_t *contents;

	contents = chunk_map(file, FALSE);
	if (!contents)
	{
		DBG1(DBG_LIB, "  opening '%s' failed: %s", file, strerror(errno));
		return 0;
	}
	*len = contents->len;
	checksum = chunk_hash_static(*contents);
	chunk_unmap(contents);

	return checksum;
}

/**
 * dl_iterate_phdr callback function
 */
static int callback(struct dl_phdr_info *dlpi, size_t size, Dl_info *dli)
{
	/* We are looking for the dlpi_addr matching the address of our dladdr().
	 * dl_iterate_phdr() returns such an address for other (unknown) objects
	 * in very rare cases (e.g. in a chrooted gentoo, but only if
	 * the checksum_builder is invoked by 'make'). As a workaround, we filter
	 * objects by dlpi_name; valid objects have a library name.
	 */
	if (dli->dli_fbase == (void*)dlpi->dlpi_addr &&
		dlpi->dlpi_name && *dlpi->dlpi_name)
	{
		int i;

		for (i = 0; i < dlpi->dlpi_phnum; i++)
		{
			const ElfW(Phdr) *sgmt = &dlpi->dlpi_phdr[i];

			/* we are interested in the executable LOAD segment */
			if (sgmt->p_type == PT_LOAD && (sgmt->p_flags & PF_X))
			{
				/* safe begin of segment in dli_fbase */
				dli->dli_fbase = (void*)sgmt->p_vaddr + dlpi->dlpi_addr;
				/* safe end of segment in dli_saddr */
				dli->dli_saddr = dli->dli_fbase + sgmt->p_memsz;
				return 1;
			}
		}
	}
	return 0;
}

METHOD(integrity_checker_t, build_segment, u_int32_t,
	private_integrity_checker_t *this, void *sym, size_t *len)
{
	chunk_t segment;
	Dl_info dli;

	if (dladdr(sym, &dli) == 0)
	{
		DBG1(DBG_LIB, "  unable to locate symbol: %s", dlerror());
		return 0;
	}
	/* we reuse the Dl_info struct as in/out parameter */
	if (!dl_iterate_phdr((void*)callback, &dli))
	{
		DBG1(DBG_LIB, "  executable section not found");
		return 0;
	}

	segment = chunk_create(dli.dli_fbase, dli.dli_saddr - dli.dli_fbase);
	*len = segment.len;
	return chunk_hash_static(segment);
}

/**
 * Find a checksum by its name
 */
static integrity_checksum_t *find_checksum(private_integrity_checker_t *this,
										   char *name)
{
	int i;

	for (i = 0; i < this->checksum_count; i++)
	{
		if (streq(this->checksums[i].name, name))
		{
			return &this->checksums[i];
		}
	}
	return NULL;
}

METHOD(integrity_checker_t, check_file, bool,
	private_integrity_checker_t *this, char *name, char *file)
{
	integrity_checksum_t *cs;
	u_int32_t sum;
	size_t len = 0;

	cs = find_checksum(this, name);
	if (!cs)
	{
		DBG1(DBG_LIB, "  '%s' file checksum not found", name);
		return FALSE;
	}
	sum = build_file(this, file, &len);
	if (!sum)
	{
		return FALSE;
	}
	if (cs->file_len != len)
	{
		DBG1(DBG_LIB, "  invalid '%s' file size: %u bytes, expected %u bytes",
			 name, len, cs->file_len);
		return FALSE;
	}
	if (cs->file != sum)
	{
		DBG1(DBG_LIB, "  invalid '%s' file checksum: %08x, expected %08x",
			 name, sum, cs->file);
		return FALSE;
	}
	DBG2(DBG_LIB, "  valid '%s' file checksum: %08x", name, sum);
	return TRUE;
}

METHOD(integrity_checker_t, check_segment, bool,
	private_integrity_checker_t *this, char *name, void *sym)
{
	integrity_checksum_t *cs;
	u_int32_t sum;
	size_t len = 0;

	cs = find_checksum(this, name);
	if (!cs)
	{
		DBG1(DBG_LIB, "  '%s' segment checksum not found", name);
		return FALSE;
	}
	sum = build_segment(this, sym, &len);
	if (!sum)
	{
		return FALSE;
	}
	if (cs->segment_len != len)
	{
		DBG1(DBG_LIB, "  invalid '%s' segment size: %u bytes,"
			 " expected %u bytes", name, len, cs->segment_len);
		return FALSE;
	}
	if (cs->segment != sum)
	{
		DBG1(DBG_LIB, "  invalid '%s' segment checksum: %08x, expected %08x",
			 name, sum, cs->segment);
		return FALSE;
	}
	DBG2(DBG_LIB, "  valid '%s' segment checksum: %08x", name, sum);
	return TRUE;
}

METHOD(integrity_checker_t, check, bool,
	private_integrity_checker_t *this, char *name, void *sym)
{
	Dl_info dli;

	if (dladdr(sym, &dli) == 0)
	{
		DBG1(DBG_LIB, "unable to locate symbol: %s", dlerror());
		return FALSE;
	}
	if (!check_file(this, name, (char*)dli.dli_fname))
	{
		return FALSE;
	}
	if (!check_segment(this, name, sym))
	{
		return FALSE;
	}
	return TRUE;
}

METHOD(integrity_checker_t, destroy, void,
	private_integrity_checker_t *this)
{
	if (this->handle)
	{
		dlclose(this->handle);
	}
	free(this);
}

/**
 * See header
 */
integrity_checker_t *integrity_checker_create(char *checksum_library)
{
	private_integrity_checker_t *this;

	INIT(this,
		.public = {
			.check_file = _check_file,
			.build_file = _build_file,
			.check_segment = _check_segment,
			.build_segment = _build_segment,
			.check = _check,
			.destroy = _destroy,
		},
	);

	if (checksum_library)
	{
		this->handle = dlopen(checksum_library, RTLD_LAZY);
		if (this->handle)
		{
			int *checksum_count;

			this->checksums = dlsym(this->handle, "checksums");
			checksum_count = dlsym(this->handle, "checksum_count");
			if (this->checksums && checksum_count)
			{
				this->checksum_count = *checksum_count;
			}
			else
			{
				DBG1(DBG_LIB, "checksum library '%s' invalid",
					 checksum_library);
			}
		}
		else
		{
			DBG1(DBG_LIB, "loading checksum library '%s' failed",
				 checksum_library);
		}
	}
	return &this->public;
}
