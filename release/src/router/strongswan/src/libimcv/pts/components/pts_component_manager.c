/*
 * Copyright (C) 2011-2012 Andreas Steffen
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

#include "pts/components/pts_component_manager.h"

#include <collections/linked_list.h>
#include <utils/debug.h>

typedef struct private_pts_component_manager_t private_pts_component_manager_t;
typedef struct vendor_entry_t vendor_entry_t;
typedef struct component_entry_t component_entry_t;

#define PTS_QUALIFIER_SIZE		6

/**
 * Vendor-specific namespace information and list of registered components
 */
struct vendor_entry_t {

	/**
	 * Vendor ID
     */
	pen_t vendor_id;

	/**
 	 * Vendor-specific Component Functional names
	 */
	enum_name_t *comp_func_names;

	/**
	 * Vendor-specific Qualifier Type names
	 */
	enum_name_t *qualifier_type_names;

	/**
	 * Vendor-specific Qualifier Flag names
	 */
	char *qualifier_flag_names;

	/**
	 * Vendor-specific size of Qualfiier Type field
	 */
	int qualifier_type_size;

	/**
	 * List of vendor-specific registered Functional Components
	 */
	linked_list_t *components;
};

/**
 * Destroy a vendor_entry_t object
 */
static void vendor_entry_destroy(vendor_entry_t *entry)
{
	entry->components->destroy_function(entry->components, free);
	free(entry);
}

/**
 * Creation method for a vendor-specific Functional Component
 */
struct component_entry_t {

	/**
	 * Vendor-Specific Component Functional Name
	 */
	uint32_t name;

	/**
	 * Functional Component creation method
	 */
	pts_component_create_t create;
};

/**
 * Private data of a pts_component_manager_t object.
 *
 */
struct private_pts_component_manager_t {

	/**
	 * Public pts_component_manager_t interface.
	 */
	pts_component_manager_t public;

	/**
	 * List of vendor-specific namespaces and registered components
	 */
	linked_list_t *list;
};

METHOD(pts_component_manager_t, add_vendor, void,
	private_pts_component_manager_t *this, pen_t vendor_id,
	enum_name_t *comp_func_names, int qualifier_type_size,
	char *qualifier_flag_names, enum_name_t *qualifier_type_names)
{
	vendor_entry_t *entry;

	entry = malloc_thing(vendor_entry_t);
	entry->vendor_id = vendor_id;
	entry->comp_func_names = comp_func_names;
	entry->qualifier_type_size = qualifier_type_size;
	entry->qualifier_flag_names = qualifier_flag_names;
	entry->qualifier_type_names = qualifier_type_names;
	entry->components = linked_list_create();

	this->list->insert_last(this->list, entry);
	DBG2(DBG_PTS, "added %N functional component namespace",
		 pen_names, vendor_id);
}

METHOD(pts_component_manager_t, get_comp_func_names, enum_name_t*,
	private_pts_component_manager_t *this, pen_t vendor_id)
{
	enumerator_t *enumerator;
	vendor_entry_t *entry;
	enum_name_t *names = NULL;

	enumerator = this->list->create_enumerator(this->list);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->vendor_id == vendor_id)
		{
			names = entry->comp_func_names;
			break;
		}
	}
	enumerator->destroy(enumerator);

	return names;
}

METHOD(pts_component_manager_t, get_qualifier_type_names, enum_name_t*,
	private_pts_component_manager_t *this, pen_t vendor_id)
{
	enumerator_t *enumerator;
	vendor_entry_t *entry;
	enum_name_t *names = NULL;

	enumerator = this->list->create_enumerator(this->list);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->vendor_id == vendor_id)
		{
			names = entry->qualifier_type_names;
			break;
		}
	}
	enumerator->destroy(enumerator);

	return names;
}

METHOD(pts_component_manager_t, add_component, void,
	private_pts_component_manager_t *this, pen_t vendor_id, uint32_t name,
	pts_component_create_t create)
{
	enumerator_t *enumerator;
	vendor_entry_t *entry;
	component_entry_t *component;

	enumerator = this->list->create_enumerator(this->list);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->vendor_id == vendor_id)
		{
			component = malloc_thing(component_entry_t);
			component->name = name;
			component->create = create;

			entry->components->insert_last(entry->components, component);
			DBG2(DBG_PTS, "added %N functional component '%N'",
				 pen_names, vendor_id,
				 get_comp_func_names(this, vendor_id), name);
		}
	}
	enumerator->destroy(enumerator);
}

METHOD(pts_component_manager_t, remove_vendor, void,
	private_pts_component_manager_t *this, pen_t vendor_id)
{
	enumerator_t *enumerator;
	vendor_entry_t *entry;

	enumerator = this->list->create_enumerator(this->list);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->vendor_id == vendor_id)
		{
			this->list->remove_at(this->list, enumerator);
			vendor_entry_destroy(entry);
			DBG2(DBG_PTS, "removed %N functional component namespace",
				 pen_names, vendor_id);
		}
	}
	enumerator->destroy(enumerator);
}

METHOD(pts_component_manager_t, get_qualifier, uint8_t,
	private_pts_component_manager_t *this, pts_comp_func_name_t *name,
	char *flags)
{
	enumerator_t *enumerator;
	vendor_entry_t *entry;
	uint8_t qualifier, size, flag, type = 0;
	int i;

	enumerator = this->list->create_enumerator(this->list);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->vendor_id == name->get_vendor_id(name))
		{
			qualifier = name->get_qualifier(name);
			size = entry->qualifier_type_size;

			/* mask qualifier type field */
			type = qualifier & ((1 << size) - 1);

			/* determine flags */
			size = PTS_QUALIFIER_SIZE - size;
			flag = (1 << (PTS_QUALIFIER_SIZE - 1));
			if (flags)
			{
				for (i = 0 ; i < size; i++)
				{
					flags[i] = (qualifier & flag) ?
								entry->qualifier_flag_names[i] : '.';
					flag >>= 1;
				}
				flags[size] = '\0';
			}
		}
	}
	enumerator->destroy(enumerator);

	return type;
}

METHOD(pts_component_manager_t, create, pts_component_t*,
	private_pts_component_manager_t *this,
	pts_comp_func_name_t *name, uint32_t depth, pts_database_t *pts_db)
{
	enumerator_t *enumerator, *e2;
	vendor_entry_t *entry;
	component_entry_t *entry2;
	pts_component_t *component = NULL;

	enumerator = this->list->create_enumerator(this->list);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->vendor_id == name->get_vendor_id(name))
		{
			e2 = entry->components->create_enumerator(entry->components);
			while (e2->enumerate(e2, &entry2))
			{
				if (entry2->name == name->get_name(name) && entry2->create)
				{
					component = entry2->create(depth, pts_db);
					break;
				}
			}
			e2->destroy(e2);
			break;
		}
	}
	enumerator->destroy(enumerator);

	return component;
}

METHOD(pts_component_manager_t, destroy, void,
	private_pts_component_manager_t *this)
{
	this->list->destroy_function(this->list, (void *)vendor_entry_destroy);
	free(this);
}

/**
 * See header
 */
pts_component_manager_t *pts_component_manager_create(void)
{
	private_pts_component_manager_t *this;

	INIT(this,
		.public = {
			.add_vendor = _add_vendor,
			.add_component = _add_component,
			.remove_vendor = _remove_vendor,
			.get_comp_func_names = _get_comp_func_names,
			.get_qualifier_type_names = _get_qualifier_type_names,
			.get_qualifier = _get_qualifier,
			.create = _create,
			.destroy = _destroy,
		},
		.list = linked_list_create(),
	);

	return &this->public;
}

