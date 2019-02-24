/*
 * Copyright (C) 2009 Martin Willi
 * HSR Hochschule fuer Technik Rapperswil
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
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>

#include <library.h>
#include <daemon.h>
#include <collections/enumerator.h>

/**
 * Integrity checker
 */
integrity_checker_t *integrity;

/**
 * Create the checksum of a binary, using name and a symbol name
 */
static void build_checksum(char *path, char *name, char *sname)
{
	void *handle, *symbol;
	uint32_t fsum, ssum;
	size_t fsize = 0;
	size_t ssize = 0;

	fsum = integrity->build_file(integrity, path, &fsize);
	ssum = 0;
	if (sname)
	{
		handle = dlopen(path, RTLD_LAZY);
		if (handle)
		{
			symbol = dlsym(handle, sname);
			if (symbol)
			{
				ssum = integrity->build_segment(integrity, symbol, &ssize);
			}
			else
			{
				fprintf(stderr, "symbol lookup failed: %s\n", dlerror());
			}
			dlclose(handle);
		}
		else
		{
			fprintf(stderr, "dlopen failed: %s\n", dlerror());
		}
	}
	printf("\t{\"%-25s%7u, 0x%08x, %6u, 0x%08x},\n",
		   name, fsize, fsum, ssize, ssum);
	fprintf(stderr, "\"%-25s%7u / 0x%08x       %6u / 0x%08x\n",
			name, fsize, fsum, ssize, ssum);
}

/**
 * Build checksums for a set of plugins
 */
static void build_plugin_checksums(char *plugins)
{
	enumerator_t *enumerator;
	char *plugin, path[256], under[128], sname[128], name[128];

	enumerator = enumerator_create_token(plugins, " ", " ");
	while (enumerator->enumerate(enumerator, &plugin))
	{
		snprintf(under, sizeof(under), "%s", plugin);
		translate(under, "-", "_");
		snprintf(path, sizeof(path), "%s/libstrongswan-%s.so",
				 PLUGINDIR, plugin);
		snprintf(sname, sizeof(sname), "%s_plugin_create", under);
		snprintf(name, sizeof(name), "%s\",", plugin);
		build_checksum(path, name, sname);
	}
	enumerator->destroy(enumerator);
}

/**
 * Build checksums for a binary/library found at path
 */
static void build_binary_checksum(char *path)
{
	char *binary, *pos, name[128], sname[128];

	binary = strrchr(path, '/');
	if (binary)
	{
		binary++;
		pos = strrchr(binary, '.');
		if (pos && streq(pos, ".so"))
		{
			snprintf(name, sizeof(name), "%.*s\",", (int)(pos - binary),
					 binary);
			if (streq(name, "libstrongswan\","))
			{
				snprintf(sname, sizeof(sname), "%s", "library_init");
			}
			else
			{
				snprintf(sname, sizeof(sname), "%.*s_init", (int)(pos - binary),
						 binary);
			}
			build_checksum(path, name, sname);
		}
		else
		{
			snprintf(name, sizeof(name), "%s\",", binary);
			build_checksum(path, name, NULL);
		}
	}
}

int main(int argc, char* argv[])
{
	int i;

	/* forces link against libcharon, imports symbols needed to
	 * dlopen plugins */
	charon = NULL;

	/* avoid confusing leak reports in build process */
	setenv("LEAK_DETECTIVE_DISABLE", "1", 0);
	/* don't use a strongswan.conf, forces integrity check to disabled */
	library_init("", "checksum_builder");
	atexit(library_deinit);

	integrity = integrity_checker_create(NULL);

	printf("/**\n");
	printf(" * checksums of files and loaded code segments.\n");
	printf(" * created by %s\n", argv[0]);
	printf(" */\n");
	printf("\n");
	printf("#include <library.h>\n");
	printf("\n");
	printf("integrity_checksum_t checksums[] = {\n");
	fprintf(stderr, "integrity test data:\n");
	fprintf(stderr, "module name,            file size / checksum   "
					"segment size / checksum\n");
	for (i = 1; i < argc; i++)
	{
		build_binary_checksum(argv[i]);
	}
#ifdef S_PLUGINS
	build_plugin_checksums(S_PLUGINS);
#endif
#ifdef P_PLUGINS
	build_plugin_checksums(P_PLUGINS);
#endif
#ifdef T_PLUGINS
	build_plugin_checksums(T_PLUGINS);
#endif
#ifdef C_PLUGINS
	build_plugin_checksums(C_PLUGINS);
#endif

	printf("};\n");
	printf("\n");
	printf("int checksum_count = countof(checksums);\n");
	printf("\n");
	integrity->destroy(integrity);

	exit(0);
}

