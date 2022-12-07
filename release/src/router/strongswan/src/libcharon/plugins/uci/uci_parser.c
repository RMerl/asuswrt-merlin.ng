/*
 * Copyright (C) 2008 Martin Willi
 * Copyright (C) 2008 Thomas Kallenberg
 *
 * Copyright (C) secunet Security Networks AG
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

#include "uci_parser.h"

#include <stdarg.h>

#include <library.h>
#include <uci.h>

typedef struct private_uci_parser_t private_uci_parser_t;

/**
 * Private data of an uci_parser_t object
 */
struct private_uci_parser_t {

	/**
	 * Public part
	 */
	uci_parser_t public;

	/**
	 * UCI package name this parser reads
	 */
	char *package;
};

/**
 * enumerator implementation create_section_enumerator
 */
typedef struct {
	/** implements enumerator */
	enumerator_t public;
	/** currently enumerated uci section */
	struct uci_element *current;
	/** all uci ipsec config sections */
	struct uci_list *list;
	/** uci conntext */
	struct uci_context *ctx;
	/** ipsec uci package */
	struct uci_package *package;
	/** NULL terminated list of keywords */
	char *keywords[];
} section_enumerator_t;

METHOD(enumerator_t, section_enumerator_enumerate, bool,
	section_enumerator_t *this, va_list args)
{
	struct uci_element *element;
	char **value;
	int i;

	if (&this->current->list == this->list)
	{
		return FALSE;
	}

	value = va_arg(args, char**);
	if (value)
	{
		if (uci_lookup(this->ctx, &element, this->package,
					   this->current->name, "name") == UCI_OK)
		{	/* use "name" attribute as config name if available ... */
			*value = uci_to_option(element)->value;
		}
		else
		{	/* ... or the section name becomes config name */
			*value = uci_to_section(this->current)->type;
		}
	}

	/* followed by keyword parameters */
	for (i = 0; this->keywords[i]; i++)
	{
		value = va_arg(args, char**);
		if (value && uci_lookup(this->ctx, &element, this->package,
						  this->current->name, this->keywords[i]) == UCI_OK)
		{
			*value = uci_to_option(element)->value;
		}
	}

	this->current = list_to_element(this->current->list.next);
	return TRUE;
}

METHOD(enumerator_t, section_enumerator_destroy, void,
	section_enumerator_t *this)
{
	uci_free_context(this->ctx);
	free(this);
}

METHOD(uci_parser_t, create_section_enumerator, enumerator_t*,
	private_uci_parser_t *this, ...)
{
	section_enumerator_t *e;
	va_list args;
	int i;

	/* allocate enumerator large enough to hold keyword pointers */
	i = 1;
	va_start(args, this);
	while (va_arg(args, char*))
	{
		i++;
	}
	va_end(args);
	INIT_EXTRA(e, sizeof(char*) * i,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _section_enumerator_enumerate,
			.destroy = _section_enumerator_destroy,
		},
	);
	i = 0;
	va_start(args, this);
	do
	{
		e->keywords[i] = va_arg(args, char*);
	}
	while (e->keywords[i++]);
	va_end(args);

	/* load uci context */
	e->ctx = uci_alloc_context();
	if (uci_load(e->ctx, this->package, &e->package) != UCI_OK)
	{
		section_enumerator_destroy(e);
		return NULL;
	}
	e->list = &e->package->sections;
	e->current = list_to_element(e->list->next);
	if (e->current->type != UCI_TYPE_SECTION)
	{
		section_enumerator_destroy(e);
		return NULL;
	}
	return &e->public;
}

METHOD(uci_parser_t, destroy, void,
	private_uci_parser_t *this)
{
	free(this->package);
	free(this);
}

/**
 * Described in header.
 */
uci_parser_t *uci_parser_create(char *package)
{
	private_uci_parser_t *this;

	INIT(this,
		.public = {
			.create_section_enumerator = _create_section_enumerator,
			.destroy = _destroy,
		},
		.package = strdup(package),
	);

	return &this->public;
}

