/*
 * Copyright (C) 2007 Martin Willi
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

#include <string.h>

#include "xml.h"

#include <libxml/parser.h>
#include <libxml/tree.h>


typedef struct private_xml_t private_xml_t;

/**
 * private data of xml
 */
struct private_xml_t {

	/**
	 * public functions
	 */
	xml_t public;

	/**
	 * root node of this xml (part)
	 */
	xmlNode *node;

	/**
	 * document, only for root xml_t
	 */
	xmlDoc *doc;

	/**
	 * Root xml_t*
	 */
	private_xml_t *root;

	/**
	 * number of enumerator instances
	 */
	int enums;
};

/**
 * child element enumerator
 */
typedef struct {
	/** enumerator interface */
	enumerator_t e;
	/** current child context (returned to enumerate() caller) */
	private_xml_t child;
	/** currently processing node */
	xmlNode *node;
} child_enum_t;

METHOD(enumerator_t, child_enumerate, bool,
	child_enum_t *e, va_list args)
{
	private_xml_t **child;
	char **name, **value;

	VA_ARGS_VGET(args, child, name, value);

	while (e->node && e->node->type != XML_ELEMENT_NODE)
	{
		e->node = e->node->next;
	}
	if (e->node)
	{
		xmlNode *text;

		text = e->node->children;
		*value = NULL;

		while (text && text->type != XML_TEXT_NODE)
		{
			text = text->next;
		}
		if (text)
		{
			*value = text->content;
		}
		*name = (char*)e->node->name;
		*child = &e->child;
		e->child.node = e->node->children;
		e->node = e->node->next;
		return TRUE;
	}
	return FALSE;
}

METHOD(xml_t, get_attribute, char*,
	private_xml_t *this, char *name)
{
	return NULL;
}

METHOD(enumerator_t, child_destroy, void,
	child_enum_t *this)
{
	if (--this->child.root->enums == 0)
	{
		xmlFreeDoc(this->child.root->doc);
		free(this->child.root);
	}
	free(this);
}

METHOD(xml_t, children, enumerator_t*,
	private_xml_t *this)
{
	child_enum_t *ce;
	INIT(ce,
		.e = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _child_enumerate,
			.destroy = _child_destroy,
		},
		.child = {
			.public = {
				.get_attribute = _get_attribute,
				.children = _children,
			},
			.doc = this->doc,
			.root = this->root,
		},
		.node = this->node,
	);
	this->root->enums++;
	return &ce->e;
}

/*
 * see header file
 */
xml_t *xml_create(char *xml)
{
	private_xml_t *this;

	INIT(this,
		.public = {
			.get_attribute = _get_attribute,
			.children = _children,
		},
		.doc = xmlReadMemory(xml, strlen(xml), NULL, NULL, 0),
	);

	if (!this->doc)
	{
		free(this);
		return NULL;
	}

	this->node = xmlDocGetRootElement(this->doc);
	this->root = this;

	return &this->public;
}

