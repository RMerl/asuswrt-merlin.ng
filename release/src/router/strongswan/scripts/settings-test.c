/*
 * Copyright (C) 2014-2018 Tobias Brunner
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>

#include <library.h>
#include <settings/settings_types.h>

/**
 * Defined in libstrongswan but not part of the public API
 */
bool settings_parser_parse_file(void *this, char *name);

/**
 * Produce indentation for the given level
 */
static void get_indent(char indent[BUF_LEN], int level)
{
	int i;

	for (i = 0; i < level * 2 && i < BUF_LEN - 2; i += 2)
	{
		indent[i  ] = ' ';
		indent[i+1] = ' ';
	}
	indent[i] = '\0';
}

/**
 * Recursively print the section and all subsections/settings
 */
static void print_section(section_t *section, int level)
{
	section_t *sub;
	section_ref_t *ref;
	kv_t *kv;
	char indent[BUF_LEN];
	int i, j;

	get_indent(indent, level);

	for (i = 0; i < array_count(section->kv_order); i++)
	{
		array_get(section->kv_order, i, &kv);
		printf("%s%s = %s\n", indent, kv->key, kv->value);
	}
	for (i = 0; i < array_count(section->sections_order); i++)
	{
		array_get(section->sections_order, i, &sub);
		printf("%s%s", indent, sub->name);
		if (array_count(sub->references))
		{
			for (j = 0; j < array_count(sub->references); j++)
			{
				array_get(sub->references, j, &ref);
				printf("%s%s", j == 0 ? " : " : ", ", ref->name);
			}
		}
		printf(" {\n");
		print_section(sub, level + 1);
		printf("%s}\n", indent);
	}
}

/**
 * Recursively print a given section and all subsections/settings
 */
static void print_settings_section(settings_t *settings, char *section,
								   int level)
{
	enumerator_t *enumerator;
	char indent[BUF_LEN], buf[BUF_LEN], *key, *value;

	get_indent(indent, level);

	enumerator = settings->create_key_value_enumerator(settings, section);
	while (enumerator->enumerate(enumerator, &key, &value))
	{
		printf("%s%s = %s\n", indent, key, value);

	}
	enumerator->destroy(enumerator);

	enumerator = settings->create_section_enumerator(settings, section);
	while (enumerator->enumerate(enumerator, &key))
	{
		printf("%s%s {\n", indent, key);
		snprintf(buf, sizeof(buf), "%s%s%s", section,
				 strlen(section) ? "." : "", key);
		print_settings_section(settings, buf, level + 1);
		printf("%s}\n", indent);
	}
	enumerator->destroy(enumerator);
}

static void usage(FILE *out, char *name)
{
	fprintf(out, "Test strongswan.conf parser\n\n");
	fprintf(out, "%s [OPTIONS]\n\n", name);
	fprintf(out, "Options:\n");
	fprintf(out, "  -h, --help          print this help.\n");
	fprintf(out, "  -d, --debug         enables debugging of the parser.\n");
	fprintf(out, "  -r, --resolve       displays the settings with references/redefines resolved.\n");
	fprintf(out, "  -f, --file=FILE     config file to load (default STDIN).\n");
	fprintf(out, "\n");
}

int main(int argc, char *argv[])
{
	char *file = NULL;
	bool resolve = FALSE;

	while (true)
	{
		struct option long_opts[] = {
			{"help",		no_argument,		NULL,	'h' },
			{"debug",		no_argument,		NULL,	'd' },
			{"file",		required_argument,	NULL,	'f' },
			{"resolve",		no_argument,		NULL,	'r' },
			{0,0,0,0 },
		};
		switch (getopt_long(argc, argv, "hdf:r", long_opts, NULL))
		{
			case EOF:
				break;
			case 'h':
				usage(stdout, argv[0]);
				return 0;
			case 'd':
				setenv("DEBUG_SETTINGS_PARSER", "1", TRUE);
				continue;
			case 'f':
				file = optarg;
				continue;
			case 'r':
				resolve = TRUE;
				continue;
			default:
				usage(stderr, argv[0]);
				return 1;
		}
		break;
	}

	/* don't load strongswan.conf */
	library_init("", "settings-test");
	atexit(library_deinit);

	dbg_default_set_level(3);

	if (file)
	{
		if (resolve)
		{
			settings_t *settings = settings_create(file);

			print_settings_section(settings, "", 0);

			settings->destroy(settings);
		}
		else
		{
			section_t *root = settings_section_create(strdup("root"));

			settings_parser_parse_file(root, file);

			print_section(root, 0);

			settings_section_destroy(root, NULL);
		}
	}
	else
	{
		usage(stderr, argv[0]);
	}
	return 0;
}
