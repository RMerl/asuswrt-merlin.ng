/*
 *
 *  Embedded Linux library
 *
 *  Copyright (C) 2011-2014  Intel Corporation. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"
#include "queue.h"
#include "string.h"
#include "hashmap.h"
#include "dbus.h"
#include "dbus-service.h"
#include "dbus-private.h"
#include "private.h"
#include "idle.h"

#define XML_ID "-//freedesktop//DTD D-BUS Object Introspection 1.0//EN"
#define XML_DTD "http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd"
#define XML_HEAD "<!DOCTYPE node PUBLIC \""XML_ID"\"\n\""XML_DTD"\">\n"

static const char *static_introspectable =
		"\t<interface name=\"org.freedesktop.DBus.Introspectable\">\n"
		"\t\t<method name=\"Introspect\">\n"
		"\t\t\t<arg name=\"xml\" type=\"s\" direction=\"out\"/>\n"
		"\t\t</method>\n\t</interface>\n";

struct _dbus_method {
	l_dbus_interface_method_cb_t cb;
	uint32_t flags;
	unsigned char name_len;
	char metainfo[];
};

struct _dbus_signal {
	uint32_t flags;
	unsigned char name_len;
	char metainfo[];
};

struct _dbus_property {
	l_dbus_property_get_cb_t getter;
	l_dbus_property_set_cb_t setter;
	uint32_t flags;
	unsigned char name_len;
	char metainfo[];
};

struct l_dbus_interface {
	struct l_queue *methods;
	struct l_queue *signals;
	struct l_queue *properties;
	bool handle_old_style_properties;
	void (*instance_destroy)(void *);
	char name[];
};

struct child_node {
	struct object_node *node;
	struct child_node *next;
	char subpath[];
};

struct interface_instance {
	struct l_dbus_interface *interface;
	void *user_data;
};

struct object_node {
	struct object_node *parent;
	struct l_queue *instances;
	struct child_node *children;
	void *user_data;
	void (*destroy) (void *);
};

struct object_manager {
	char *path;
	struct l_dbus *dbus;
	struct l_queue *announce_added;
	struct l_queue *announce_removed;
};

struct interface_add_record {
	char *path;
	struct object_node *object;
	struct l_queue *instances;
};

struct interface_remove_record {
	char *path;
	struct object_node *object;
	struct l_queue *interface_names;
};

struct property_change_record {
	char *path;
	struct object_node *object;
	struct interface_instance *instance;
	struct l_queue *properties;
};

struct _dbus_object_tree {
	struct l_hashmap *interfaces;
	struct l_hashmap *objects;
	struct object_node *root;
	struct l_queue *object_managers;
	struct l_queue *property_changes;
	struct l_idle *emit_signals_work;
	bool flushing;
};

void _dbus_method_introspection(struct _dbus_method *info,
					struct l_string *buf)
{
	const char *sig;
	const char *end;
	const char *pname;
	unsigned int offset = info->name_len + 1;

	l_string_append_printf(buf, "\t\t<method name=\"%s\">\n",
				info->metainfo);

	sig = info->metainfo + offset;
	offset += strlen(sig) + 1;

	for (; *sig; sig++) {
		end = _dbus_signature_end(sig);
		pname = info->metainfo + offset;

		l_string_append_printf(buf, "\t\t\t<arg name=\"%s\" "
					"type=\"%.*s\" direction=\"in\"/>\n",
					pname, (int) (end - sig + 1), sig);
		sig = end;
		offset += strlen(pname) + 1;
	}

	sig = info->metainfo + offset;
	offset += strlen(sig) + 1;

	for (; *sig; sig++) {
		end = _dbus_signature_end(sig);
		pname = info->metainfo + offset;

		l_string_append_printf(buf, "\t\t\t<arg name=\"%s\" "
					"type=\"%.*s\" direction=\"out\"/>\n",
					pname, (int) (end - sig + 1), sig);
		sig = end;
		offset += strlen(pname) + 1;
	}

	if (info->flags & L_DBUS_METHOD_FLAG_DEPRECATED)
		l_string_append(buf, "\t\t\t<annotation name=\""
				"org.freedesktop.DBus.Deprecated\" "
				"value=\"true\"/>\n");

	if (info->flags & L_DBUS_METHOD_FLAG_NOREPLY)
		l_string_append(buf, "\t\t\t<annotation name=\""
				"org.freedesktop.DBus.Method.NoReply\" "
				"value=\"true\"/>\n");

	l_string_append(buf, "\t\t</method>\n");
}

void _dbus_signal_introspection(struct _dbus_signal *info,
					struct l_string *buf)
{
	const char *sig;
	const char *end;
	const char *pname;
	unsigned int offset = info->name_len + 1;

	l_string_append_printf(buf, "\t\t<signal name=\"%s\">\n",
				info->metainfo);

	sig = info->metainfo + offset;
	offset += strlen(sig) + 1;

	for (; *sig; sig++) {
		end = _dbus_signature_end(sig);
		pname = info->metainfo + offset;

		l_string_append_printf(buf, "\t\t\t<arg name=\"%s\" "
					"type=\"%.*s\"/>\n",
					pname, (int) (end - sig + 1), sig);
		sig = end;
		offset += strlen(pname) + 1;
	}

	if (info->flags & L_DBUS_SIGNAL_FLAG_DEPRECATED)
		l_string_append(buf, "\t\t\t<annotation name=\""
				"org.freedesktop.DBus.Deprecated\" "
				"value=\"true\"/>\n");

	l_string_append(buf, "\t\t</signal>\n");
}

void _dbus_property_introspection(struct _dbus_property *info,
						struct l_string *buf)
{
	unsigned int offset = info->name_len + 1;
	const char *signature = info->metainfo + offset;

	l_string_append_printf(buf, "\t\t<property name=\"%s\" type=\"%s\" ",
				info->metainfo, signature);

	if (info->setter)
		l_string_append(buf, "access=\"readwrite\"");
	else
		l_string_append(buf, "access=\"read\"");

	if (info->flags & L_DBUS_METHOD_FLAG_DEPRECATED) {
		l_string_append(buf, ">\n");
		l_string_append(buf, "\t\t\t<annotation name=\""
				"org.freedesktop.DBus.Deprecated\" "
				"value=\"true\"/>\n");
		l_string_append(buf, "\t\t</property>\n");
	} else
		l_string_append(buf, "/>\n");
}

void _dbus_interface_introspection(struct l_dbus_interface *interface,
						struct l_string *buf)
{
	l_string_append_printf(buf, "\t<interface name=\"%s\">\n",
				interface->name);

	l_queue_foreach(interface->methods,
		(l_queue_foreach_func_t) _dbus_method_introspection, buf);
	l_queue_foreach(interface->signals,
		(l_queue_foreach_func_t) _dbus_signal_introspection, buf);
	l_queue_foreach(interface->properties,
		(l_queue_foreach_func_t) _dbus_property_introspection, buf);

	l_string_append(buf, "\t</interface>\n");
}

#define COPY_PARAMS(dest, signature, args)	\
	do {	\
		const char *pname;	\
		const char *sig;	\
		dest = stpcpy(dest, signature) + 1;	\
		for (sig = signature; *sig; sig++) {	\
			sig = _dbus_signature_end(sig);	\
			pname = va_arg(args, const char *);	\
			dest = stpcpy(dest, pname) + 1;	\
		}	\
	} while(0)

#define SIZE_PARAMS(signature, args)	\
	({	\
		unsigned int len = strlen(signature) + 1;	\
		const char *pname;	\
		const char *sig;	\
		for (sig = signature; *sig; sig++) {	\
			sig = _dbus_signature_end(sig);	\
			if (!sig) {	\
				len = 0;	\
				break;	\
			}	\
			pname = va_arg(args, const char *);	\
			len += strlen(pname) + 1;	\
		}	\
		len;	\
	})

LIB_EXPORT bool l_dbus_interface_method(struct l_dbus_interface *interface,
					const char *name, uint32_t flags,
					l_dbus_interface_method_cb_t cb,
					const char *return_sig,
					const char *param_sig, ...)
{
	va_list args;
	unsigned int return_info_len;
	unsigned int param_info_len;
	struct _dbus_method *info;
	char *p;

	if (!_dbus_valid_method(name))
		return false;

	if (unlikely(!return_sig || !param_sig))
		return false;

	if (return_sig[0] && !_dbus_valid_signature(return_sig))
		return false;

	if (param_sig[0] && !_dbus_valid_signature(param_sig))
		return false;

	/* Pre-calculate the needed meta-info length */
	va_start(args, param_sig);

	return_info_len = SIZE_PARAMS(return_sig, args);
	param_info_len = SIZE_PARAMS(param_sig, args);

	va_end(args);

	if (!return_info_len || !param_info_len)
		return false;

	info = l_malloc(sizeof(*info) + return_info_len +
					param_info_len + strlen(name) + 1);
	info->cb = cb;
	info->flags = flags;
	info->name_len = strlen(name);
	strcpy(info->metainfo, name);

	va_start(args, param_sig);

	/*
	 * We store param signature + parameter names first, to speed up
	 * lookups during the message dispatch procedures.
	 */
	p = info->metainfo + info->name_len + param_info_len + 1;
	COPY_PARAMS(p, return_sig, args);

	p = info->metainfo + info->name_len + 1;
	COPY_PARAMS(p, param_sig, args);

	va_end(args);

	l_queue_push_tail(interface->methods, info);

	return true;
}

LIB_EXPORT bool l_dbus_interface_signal(struct l_dbus_interface *interface,
					const char *name, uint32_t flags,
					const char *signature, ...)
{
	va_list args;
	unsigned int metainfo_len;
	struct _dbus_signal *info;
	char *p;

	if (!_dbus_valid_method(name))
		return false;

	if (unlikely(!signature))
		return false;

	if (signature[0] && !_dbus_valid_signature(signature))
		return false;

	/* Pre-calculate the needed meta-info length */
	va_start(args, signature);
	metainfo_len = SIZE_PARAMS(signature, args);
	va_end(args);

	if (!metainfo_len)
		return false;

	metainfo_len += strlen(name) + 1;

	info = l_malloc(sizeof(*info) + metainfo_len);
	info->flags = flags;
	info->name_len = strlen(name);

	p = stpcpy(info->metainfo, name) + 1;

	va_start(args, signature);
	COPY_PARAMS(p, signature, args);
	va_end(args);

	l_queue_push_tail(interface->signals, info);

	return true;
}

LIB_EXPORT bool l_dbus_interface_property(struct l_dbus_interface *interface,
					const char *name, uint32_t flags,
					const char *signature,
					l_dbus_property_get_cb_t getter,
					l_dbus_property_set_cb_t setter)
{
	unsigned int metainfo_len;
	struct _dbus_property *info;
	char *p;

	if (!_dbus_valid_method(name))
		return false;

	if (unlikely(!signature || !getter))
		return false;

	if (_dbus_num_children(signature) != 1)
		return false;

	/* Pre-calculate the needed meta-info length */
	metainfo_len = strlen(name) + 1;
	metainfo_len += strlen(signature) + 1;

	info = l_malloc(sizeof(*info) + metainfo_len);
	info->flags = flags;
	info->name_len = strlen(name);
	info->getter = getter;
	info->setter = setter;

	p = stpcpy(info->metainfo, name) + 1;
	strcpy(p, signature);

	l_queue_push_tail(interface->properties, info);

	return true;
}

struct l_dbus_interface *_dbus_interface_new(const char *name)
{
	struct l_dbus_interface *interface;

	interface = l_malloc(sizeof(*interface) + strlen(name) + 1);

	interface->methods = l_queue_new();
	interface->signals = l_queue_new();
	interface->properties = l_queue_new();

	strcpy(interface->name, name);

	return interface;
}

void _dbus_interface_free(struct l_dbus_interface *interface)
{
	l_queue_destroy(interface->methods, l_free);
	l_queue_destroy(interface->signals, l_free);
	l_queue_destroy(interface->properties, l_free);

	l_free(interface);
}

static bool match_method(const void *a, const void *b)
{
	const struct _dbus_method *method = a;
	const char *name = b;

	if (!strcmp(method->metainfo, name))
		return true;

	return false;
}

struct _dbus_method *_dbus_interface_find_method(struct l_dbus_interface *i,
							const char *method)
{
	return l_queue_find(i->methods, match_method, (char *) method);
}

static bool match_signal(const void *a, const void *b)
{
	const struct _dbus_signal *signal = a;
	const char *name = b;

	if (!strcmp(signal->metainfo, name))
		return true;

	return false;
}

struct _dbus_signal *_dbus_interface_find_signal(struct l_dbus_interface *i,
							const char *signal)
{
	return l_queue_find(i->signals, match_signal, (char *) signal);
}

static bool match_property(const void *a, const void *b)
{
	const struct _dbus_property *property = a;
	const char *name = b;

	if (!strcmp(property->metainfo, name))
		return true;

	return false;
}

struct _dbus_property *_dbus_interface_find_property(struct l_dbus_interface *i,
							const char *property)
{
	return l_queue_find(i->properties, match_property, (char *) property);
}

static void interface_instance_free(struct interface_instance *instance)
{
	if (instance->interface->instance_destroy)
		instance->interface->instance_destroy(instance->user_data);

	l_free(instance);
}

static bool match_interface_instance(const void *a, const void *b)
{
	const struct interface_instance *instance = a;
	const char *name = b;

	if (!strcmp(instance->interface->name, name))
		return true;

	return false;
}

static bool match_interface_instance_ptr(const void *a, const void *b)
{
	const struct interface_instance *instance = a;

	return instance->interface == b;
}

static void interface_add_record_free(void *data)
{
	struct interface_add_record *rec = data;

	l_free(rec->path);
	l_queue_destroy(rec->instances, NULL);
	l_free(rec);
}

static void interface_removed_record_free(void *data)
{
	struct interface_remove_record *rec = data;

	l_free(rec->path);
	l_queue_destroy(rec->interface_names, l_free);
	l_free(rec);
}

static void property_change_record_free(void *data)
{
	struct property_change_record *rec = data;

	l_free(rec->path);
	l_queue_destroy(rec->properties, NULL);
	l_free(rec);
}

static void properties_setup_func(struct l_dbus_interface *);
static void object_manager_setup_func(struct l_dbus_interface *);

struct _dbus_object_tree *_dbus_object_tree_new()
{
	struct _dbus_object_tree *tree;

	tree = l_new(struct _dbus_object_tree, 1);

	tree->interfaces = l_hashmap_new();
	l_hashmap_set_hash_function(tree->interfaces, l_str_hash);
	l_hashmap_set_compare_function(tree->interfaces,
					(l_hashmap_compare_func_t)strcmp);

	tree->objects = l_hashmap_string_new();

	tree->root = l_new(struct object_node, 1);

	tree->property_changes = l_queue_new();

	_dbus_object_tree_register_interface(tree, L_DBUS_INTERFACE_PROPERTIES,
						properties_setup_func, NULL,
						false);

	tree->object_managers = l_queue_new();

	_dbus_object_tree_register_interface(tree,
						L_DBUS_INTERFACE_OBJECT_MANAGER,
						object_manager_setup_func, NULL,
						false);

	return tree;
}

static void subtree_free(struct object_node *node)
{
	struct child_node *child;

	while (node->children) {
		child = node->children;
		node->children = child->next;

		subtree_free(child->node);
		l_free(child);
	}

	l_queue_destroy(node->instances,
			(l_queue_destroy_func_t) interface_instance_free);

	if (node->destroy)
		node->destroy(node->user_data);

	l_free(node);
}

static void object_manager_free(void *data)
{
	struct object_manager *manager = data;

	l_free(manager->path);
	l_queue_destroy(manager->announce_added, interface_add_record_free);
	l_queue_destroy(manager->announce_removed,
						interface_removed_record_free);
	l_free(manager);
}

void _dbus_object_tree_free(struct _dbus_object_tree *tree)
{
	subtree_free(tree->root);

	l_hashmap_destroy(tree->interfaces,
			(l_hashmap_destroy_func_t) _dbus_interface_free);
	l_hashmap_destroy(tree->objects, NULL);

	l_queue_destroy(tree->object_managers, object_manager_free);

	l_queue_destroy(tree->property_changes, property_change_record_free);

	if (tree->emit_signals_work)
		l_idle_remove(tree->emit_signals_work);

	l_free(tree);
}

static struct object_node *makepath_recurse(struct object_node *node,
						const char *path)
{
	const char *end;
	struct child_node *child;

	if (*path == '\0')
		return node;

	path += 1;
	end = strchrnul(path, '/');
	child = node->children;

	while (child) {
		if (!strncmp(child->subpath, path, end - path) &&
				child->subpath[end - path] == '\0')
			goto done;

		child = child->next;
	}

	child = l_malloc(sizeof(*child) + end - path + 1);
	child->node = l_new(struct object_node, 1);
	child->node->parent = node;
	memcpy(child->subpath, path, end - path);
	child->subpath[end-path] = '\0';
	child->next = node->children;
	node->children = child;

done:
	return makepath_recurse(child->node, end);
}

struct object_node *_dbus_object_tree_makepath(struct _dbus_object_tree *tree,
						const char *path)
{
	if (path[0] == '/' && path[1] == '\0')
		return tree->root;

	return makepath_recurse(tree->root, path);
}

static struct object_node *lookup_recurse(struct object_node *node,
						const char *path)
{
	const char *end;
	struct child_node *child;

	if (*path == '\0')
		return node;

	path += 1;
	end = strchrnul(path, '/');
	child = node->children;

	while (child) {
		if (!strncmp(child->subpath, path, end - path) &&
				child->subpath[end - path] == '\0')
			return lookup_recurse(child->node, end);

		child = child->next;
	}

	return NULL;
}

struct object_node *_dbus_object_tree_lookup(struct _dbus_object_tree *tree,
						const char *path)
{
	if (path[0] == '/' && path[1] == '\0')
		return tree->root;

	return lookup_recurse(tree->root, path);
}

void _dbus_object_tree_prune_node(struct object_node *node)
{
	struct object_node *parent = node->parent;
	struct child_node *p = NULL, *c;

	while (parent) {
		for (c = parent->children, p = NULL; c; p = c, c = c->next) {
			if (c->node != node)
				continue;

			if (p)
				p->next = c->next;
			else
				parent->children = c->next;

			subtree_free(c->node);
			l_free(c);

			break;
		}

		if (parent->children != NULL)
			return;

		if (parent->instances)
			return;

		node = parent;
		parent = node->parent;
	}
}

struct object_node *_dbus_object_tree_new_object(struct _dbus_object_tree *tree,
						const char *path,
						void *user_data,
						void (*destroy) (void *))
{
	struct object_node *node;

	if (!_dbus_valid_object_path(path))
		return NULL;

	if (l_hashmap_lookup(tree->objects, path))
		return NULL;

	node = _dbus_object_tree_makepath(tree, path);
	node->user_data = user_data;
	node->destroy = destroy;

	/*
	 * Registered objects in the tree are marked by being present in the
	 * tree->objects hash and having non-null node->instances.  Remaining
	 * nodes are intermediate path elements added and removed
	 * automatically.
	 */
	node->instances = l_queue_new();

	l_hashmap_insert(tree->objects, path, node);

	return node;
}

bool _dbus_object_tree_object_destroy(struct _dbus_object_tree *tree,
					const char *path)
{
	struct object_node *node;
	const struct l_queue_entry *entry;
	const struct interface_instance *instance;

	node = l_hashmap_lookup(tree->objects, path);
	if (!node)
		return false;

	while ((entry = l_queue_get_entries(node->instances))) {
		instance = entry->data;

		if (!_dbus_object_tree_remove_interface(tree, path,
						instance->interface->name))
			return false;
	}

	l_hashmap_remove(tree->objects, path);

	l_queue_destroy(node->instances, NULL);
	node->instances = NULL;

	if (node->destroy) {
		node->destroy(node->user_data);
		node->destroy = NULL;
	}

	if (!node->children)
		_dbus_object_tree_prune_node(node);

	return true;
}

static bool get_properties_dict(struct l_dbus *dbus,
				struct l_dbus_message *message,
				struct l_dbus_message_builder *builder,
				const struct l_dbus_interface *interface,
				void *user_data)
{
	const struct l_queue_entry *entry;
	const struct _dbus_property *property;
	const char *signature;

	l_dbus_message_builder_enter_array(builder, "{sv}");
	_dbus_message_builder_mark(builder);

	for (entry = l_queue_get_entries(interface->properties); entry;
			entry = entry->next) {
		property = entry->data;
		signature = property->metainfo + strlen(property->metainfo) + 1;

		l_dbus_message_builder_enter_dict(builder, "sv");
		l_dbus_message_builder_append_basic(builder, 's',
							property->metainfo);
		l_dbus_message_builder_enter_variant(builder, signature);

		if (!property->getter(dbus, message, builder, user_data)) {
			if (!_dbus_message_builder_rewind(builder))
				return false;

			continue;
		}

		l_dbus_message_builder_leave_variant(builder);
		l_dbus_message_builder_leave_dict(builder);
		_dbus_message_builder_mark(builder);
	}

	l_dbus_message_builder_leave_array(builder);

	return true;
}

static struct l_dbus_message *build_interfaces_removed_signal(
				const struct object_manager *manager,
				const struct interface_remove_record *rec)
{
	struct l_dbus_message *signal;
	struct l_dbus_message_builder *builder;
	const struct l_queue_entry *entry;

	signal = l_dbus_message_new_signal(manager->dbus, manager->path,
						L_DBUS_INTERFACE_OBJECT_MANAGER,
						"InterfacesRemoved");

	builder = l_dbus_message_builder_new(signal);

	l_dbus_message_builder_append_basic(builder, 'o', rec->path);
	l_dbus_message_builder_enter_array(builder, "s");

	for (entry = l_queue_get_entries(rec->interface_names); entry;
			entry = entry->next)
		l_dbus_message_builder_append_basic(builder, 's', entry->data);

	l_dbus_message_builder_leave_array(builder);
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);

	return signal;
}

static struct l_dbus_message *build_interfaces_added_signal(
					const struct object_manager *manager,
					const struct interface_add_record *rec)
{
	struct l_dbus_message *signal;
	struct l_dbus_message_builder *builder;
	const struct l_queue_entry *entry;
	const struct interface_instance *instance;

	signal = l_dbus_message_new_signal(manager->dbus, manager->path,
						L_DBUS_INTERFACE_OBJECT_MANAGER,
						"InterfacesAdded");

	builder = l_dbus_message_builder_new(signal);

	l_dbus_message_builder_append_basic(builder, 'o', rec->path);
	l_dbus_message_builder_enter_array(builder, "{sa{sv}}");

	for (entry = l_queue_get_entries(rec->instances); entry;
			entry = entry->next) {
		instance = entry->data;

		l_dbus_message_builder_enter_dict(builder, "sa{sv}");
		l_dbus_message_builder_append_basic(builder, 's',
						instance->interface->name);

		if (!get_properties_dict(manager->dbus, signal, builder,
						instance->interface,
						instance->user_data)) {
			l_dbus_message_builder_destroy(builder);
			l_dbus_message_unref(signal);

			return NULL;
		}

		l_dbus_message_builder_leave_dict(builder);
	}

	l_dbus_message_builder_leave_array(builder);
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);

	return signal;
}

static struct l_dbus_message *build_old_property_changed_signal(
				struct l_dbus *dbus,
				const struct property_change_record *rec,
				const struct _dbus_property *property)
{
	struct l_dbus_message *signal;
	struct l_dbus_message_builder *builder;
	const char *signature;

	signature = property->metainfo + strlen(property->metainfo) + 1;

	signal = l_dbus_message_new_signal(dbus, rec->path,
						rec->instance->interface->name,
						"PropertyChanged");

	builder = l_dbus_message_builder_new(signal);

	l_dbus_message_builder_append_basic(builder, 's', property->metainfo);
	l_dbus_message_builder_enter_variant(builder, signature);

	if (!property->getter(dbus, signal, builder,
				rec->instance->user_data)) {
		l_dbus_message_builder_destroy(builder);
		l_dbus_message_unref(signal);

		return NULL;
	}

	l_dbus_message_builder_leave_variant(builder);

	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);

	return signal;
}

static struct l_dbus_message *build_properties_changed_signal(
				struct l_dbus *dbus,
				const struct property_change_record *rec)
{
	struct l_dbus_message *signal;
	struct l_dbus_message_builder *builder;
	const struct l_queue_entry *entry;
	const struct _dbus_property *property;
	const char *signature;
	struct l_queue *invalidated;

	signal = l_dbus_message_new_signal(dbus, rec->path,
						L_DBUS_INTERFACE_PROPERTIES,
						"PropertiesChanged");

	builder = l_dbus_message_builder_new(signal);

	invalidated = l_queue_new();

	l_dbus_message_builder_append_basic(builder, 's',
						rec->instance->interface->name);
	l_dbus_message_builder_enter_array(builder, "{sv}");

	for (entry = l_queue_get_entries(rec->properties); entry;
			entry = entry->next) {
		property = entry->data;
		signature = property->metainfo + strlen(property->metainfo) + 1;

		_dbus_message_builder_mark(builder);

		l_dbus_message_builder_enter_dict(builder, "sv");
		l_dbus_message_builder_append_basic(builder, 's',
							property->metainfo);
		l_dbus_message_builder_enter_variant(builder, signature);

		if (!property->getter(dbus, signal, builder,
					rec->instance->user_data)) {
			if (!_dbus_message_builder_rewind(builder)) {
				l_dbus_message_unref(signal);
				signal = NULL;

				goto done;
			}

			l_queue_push_tail(invalidated, (void *) property);

			continue;
		}

		l_dbus_message_builder_leave_variant(builder);
		l_dbus_message_builder_leave_dict(builder);
	}

	l_dbus_message_builder_leave_array(builder);
	l_dbus_message_builder_enter_array(builder, "s");

	while ((property = l_queue_pop_head(invalidated)))
		l_dbus_message_builder_append_basic(builder, 's',
							property->metainfo);

	l_dbus_message_builder_leave_array(builder);
	l_dbus_message_builder_finalize(builder);

done:
	l_dbus_message_builder_destroy(builder);

	l_queue_destroy(invalidated, NULL);

	return signal;
}

struct emit_signals_data {
	struct l_dbus *dbus;
	struct object_manager *manager;
	struct object_node *node;
};

static bool emit_interfaces_removed(void *data, void *user_data)
{
	struct interface_remove_record *rec = data;
	struct emit_signals_data *es = user_data;
	struct l_dbus_message *signal;

	if (es->node && rec->object != es->node)
		return false;

	signal = build_interfaces_removed_signal(es->manager, rec);
	interface_removed_record_free(rec);

	if (signal)
		l_dbus_send(es->manager->dbus, signal);

	return true;
}

static bool emit_interfaces_added(void *data, void *user_data)
{
	struct interface_add_record *rec = data;
	struct emit_signals_data *es = user_data;
	struct l_dbus_message *signal;

	if (es->node && rec->object != es->node)
		return false;

	signal = build_interfaces_added_signal(es->manager, rec);
	interface_add_record_free(rec);

	if (signal)
		l_dbus_send(es->manager->dbus, signal);

	return true;
}

static bool emit_properties_changed(void *data, void *user_data)
{
	struct property_change_record *rec = data;
	struct emit_signals_data *es = user_data;
	struct l_dbus_message *signal;
	const struct l_queue_entry *entry;

	if (es->node && rec->object != es->node)
		return false;

	if (rec->instance->interface->handle_old_style_properties)
		for (entry = l_queue_get_entries(rec->properties);
				entry; entry = entry->next) {
			signal = build_old_property_changed_signal(es->dbus,
							rec, entry->data);
			if (signal)
				l_dbus_send(es->dbus, signal);
		}

	if (l_queue_find(rec->object->instances, match_interface_instance,
				L_DBUS_INTERFACE_PROPERTIES)) {
		signal = build_properties_changed_signal(es->dbus, rec);
		if (signal)
			l_dbus_send(es->dbus, signal);
	}

	property_change_record_free(rec);

	return true;
}

void _dbus_object_tree_signals_flush(struct l_dbus *dbus, const char *path)
{
	struct _dbus_object_tree *tree = _dbus_get_tree(dbus);
	const struct l_queue_entry *entry;
	struct emit_signals_data data;
	bool all_done = true;

	if (!tree->emit_signals_work || tree->flushing)
		return;

	tree->flushing = true;

	data.dbus = dbus;
	data.node = path ? _dbus_object_tree_lookup(tree, path) : NULL;

	for (entry = l_queue_get_entries(tree->object_managers); entry;
			entry = entry->next) {
		data.manager = entry->data;

		l_queue_foreach_remove(data.manager->announce_removed,
					emit_interfaces_removed, &data);

		if (!l_queue_isempty(data.manager->announce_removed))
			all_done = false;

		l_queue_foreach_remove(data.manager->announce_added,
					emit_interfaces_added, &data);

		if (!l_queue_isempty(data.manager->announce_added))
			all_done = false;
	}

	l_queue_foreach_remove(tree->property_changes,
				emit_properties_changed, &data);

	if (!l_queue_isempty(tree->property_changes))
		all_done = false;

	if (all_done) {
		l_idle_remove(tree->emit_signals_work);
		tree->emit_signals_work = NULL;
	}

	tree->flushing = false;
}

static void emit_signals(struct l_idle *idle, void *user_data)
{
	struct l_dbus *dbus = user_data;

	_dbus_object_tree_signals_flush(dbus, NULL);
}

static void schedule_emit_signals(struct l_dbus *dbus)
{
	struct _dbus_object_tree *tree = _dbus_get_tree(dbus);

	if (tree->emit_signals_work)
		return;

	tree->emit_signals_work = l_idle_create(emit_signals, dbus, NULL);
}

static bool match_property_changes_instance(const void *a, const void *b)
{
	const struct property_change_record *rec = a;

	return rec->instance == b;
}

static bool match_pointer(const void *a, const void *b)
{
	return a == b;
}

bool _dbus_object_tree_property_changed(struct l_dbus *dbus,
					const char *path,
					const char *interface_name,
					const char *property_name)
{
	struct property_change_record *rec;
	struct object_node *object;
	struct interface_instance *instance;
	struct _dbus_property *property;
	struct _dbus_object_tree *tree = _dbus_get_tree(dbus);

	object = l_hashmap_lookup(tree->objects, path);
	if (!object)
		return false;

	instance = l_queue_find(object->instances, match_interface_instance,
				interface_name);
	if (!instance)
		return false;

	property = _dbus_interface_find_property(instance->interface,
							property_name);
	if (!property)
		return false;

	rec = l_queue_find(tree->property_changes,
				match_property_changes_instance, instance);

	if (rec) {
		if (l_queue_find(rec->properties, match_pointer, property))
			return true;
	} else {
		rec = l_new(struct property_change_record, 1);
		rec->path = l_strdup(path);
		rec->object = object;
		rec->instance = instance;
		rec->properties = l_queue_new();

		l_queue_push_tail(tree->property_changes, rec);
	}

	l_queue_push_tail(rec->properties, property);

	schedule_emit_signals(dbus);

	return true;
}

static void pending_property_set_done_common(struct l_dbus *dbus,
					struct l_dbus_message *message,
					struct l_dbus_message *reply,
					bool auto_emit)
{
	const char *member;
	const char *interface_name;
	const char *property_name;
	struct l_dbus_message_iter variant;

	if (!reply) {
		reply = l_dbus_message_new_method_return(message);
		l_dbus_message_set_arguments(reply, "");
	}

	l_dbus_send(dbus, l_dbus_message_ref(reply));

	member = l_dbus_message_get_member(message);
	if (!strcmp(member, "SetProperty")) {
		if (!l_dbus_message_get_arguments(message, "sv",
						&property_name, &variant))
			goto done;

		interface_name = l_dbus_message_get_interface(message);
	} else if (strcmp(member, "Set") ||
			!l_dbus_message_get_arguments(message, "ssv",
							&interface_name,
							&property_name,
							&variant))
		goto done;

	if (auto_emit)
		_dbus_object_tree_property_changed(dbus,
					l_dbus_message_get_path(message),
					interface_name, property_name);
done:
	l_dbus_message_unref(message);
	l_dbus_message_unref(reply);
}

static void pending_property_set_done_emit(struct l_dbus *dbus,
					struct l_dbus_message *message,
					struct l_dbus_message *reply)
{
	pending_property_set_done_common(dbus, message, reply, true);
}

static void pending_property_set_done(struct l_dbus *dbus,
					struct l_dbus_message *message,
					struct l_dbus_message *reply)
{
	pending_property_set_done_common(dbus, message, reply, false);
}

static struct l_dbus_message *old_set_property(struct l_dbus *dbus,
						struct l_dbus_message *message,
						void *user_data)
{
	struct l_dbus_interface *interface;
	const char *property_name;
	const struct _dbus_property *property;
	struct l_dbus_message_iter variant;
	struct _dbus_object_tree *tree = _dbus_get_tree(dbus);
	struct l_dbus_message *reply;
	l_dbus_property_complete_cb_t complete_cb;

	interface = l_hashmap_lookup(tree->interfaces,
					l_dbus_message_get_interface(message));
	/* If we got here the interface must exist */

	if (!l_dbus_message_get_arguments(message, "sv", &property_name,
						&variant))
		return l_dbus_message_new_error(message,
						"org.freedesktop.DBus.Error."
						"InvalidArgs",
						"Invalid arguments");

	property = _dbus_interface_find_property(interface, property_name);
	if (!property)
		return l_dbus_message_new_error(message,
						"org.freedesktop.DBus.Error."
						"InvalidArgs",
						"Unknown Property %s",
						property_name);

	if (!property->setter)
		return l_dbus_message_new_error(message,
						"org.freedesktop.DBus.Error."
						"InvalidArgs",
						"Property %s is read-only",
						property_name);

	if (property->flags & L_DBUS_PROPERTY_FLAG_AUTO_EMIT)
		complete_cb = pending_property_set_done_emit;
	else
		complete_cb = pending_property_set_done;

	reply = property->setter(dbus, l_dbus_message_ref(message), &variant,
					complete_cb, user_data);

	if (reply)
		complete_cb(dbus, message, reply);

	return NULL;
}

static struct l_dbus_message *old_get_properties(struct l_dbus *dbus,
						struct l_dbus_message *message,
						void *user_data)
{
	const struct l_dbus_interface *interface;
	struct l_dbus_message *reply;
	struct l_dbus_message_builder *builder;
	struct _dbus_object_tree *tree = _dbus_get_tree(dbus);

	interface = l_hashmap_lookup(tree->interfaces,
					l_dbus_message_get_interface(message));
	/* If we got here the interface must exist */

	reply = l_dbus_message_new_method_return(message);
	builder = l_dbus_message_builder_new(reply);

	if (!get_properties_dict(dbus, message, builder, interface,
					user_data)) {
		l_dbus_message_unref(reply);

		reply = l_dbus_message_new_error(message,
						"org.freedesktop.DBus.Error."
						"Failed",
						"Getting properties failed");
	} else
		l_dbus_message_builder_finalize(builder);

	l_dbus_message_builder_destroy(builder);

	return reply;
}

bool _dbus_object_tree_register_interface(struct _dbus_object_tree *tree,
				const char *interface,
				void (*setup_func)(struct l_dbus_interface *),
				void (*destroy) (void *),
				bool old_style_properties)
{
	struct l_dbus_interface *dbi;

	if (!_dbus_valid_interface(interface))
		return false;

	/*
	 * Check to make sure we do not have this interface already
	 * registered
	 */
	dbi = l_hashmap_lookup(tree->interfaces, interface);
	if (dbi)
		return false;

	dbi = _dbus_interface_new(interface);
	dbi->instance_destroy = destroy;
	dbi->handle_old_style_properties = old_style_properties;

	/* Add our methods first so we don't have to check for conflicts. */
	if (old_style_properties) {
		l_dbus_interface_method(dbi, "SetProperty", 0,
					old_set_property, "", "sv",
					"name", "value");
		l_dbus_interface_method(dbi, "GetProperties", 0,
					old_get_properties, "a{sv}", "",
					"properties");

		l_dbus_interface_signal(dbi, "PropertyChanged", 0, "sv",
					"name", "value");
	}

	setup_func(dbi);

	l_hashmap_insert(tree->interfaces, dbi->name, dbi);

	return true;
}

struct interface_check {
	struct _dbus_object_tree *tree;
	const char *interface;
};

static void check_interface_used(const void *key, void *value, void *user_data)
{
	const char *path = key;
	struct object_node *node = value;
	struct interface_check *state = user_data;

	if (!l_queue_find(node->instances, match_interface_instance,
				(char *) state->interface))
		return;

	_dbus_object_tree_remove_interface(state->tree, path, state->interface);
}

bool _dbus_object_tree_unregister_interface(struct _dbus_object_tree *tree,
						const char *interface_name)
{
	struct l_dbus_interface *interface;
	struct interface_check state = { tree, interface_name };

	interface = l_hashmap_lookup(tree->interfaces, interface_name);
	if (!interface)
		return false;

	/* Check that the interface is not in use */
	l_hashmap_foreach(tree->objects, check_interface_used, &state);

	l_hashmap_remove(tree->interfaces, interface_name);

	_dbus_interface_free(interface);

	return true;
}

static void collect_instances(struct object_node *node,
				const char *path,
				struct l_queue *announce)
{
	const struct l_queue_entry *entry;
	struct interface_add_record *change_rec;
	const struct child_node *child;

	if (!node->instances)
		goto recurse;

	change_rec = l_new(struct interface_add_record, 1);
	change_rec->path = l_strdup(path);
	change_rec->object = node;
	change_rec->instances = l_queue_new();

	for (entry = l_queue_get_entries(node->instances); entry;
			entry = entry->next)
		l_queue_push_tail(change_rec->instances, entry->data);

	l_queue_push_tail(announce, change_rec);

recurse:
	if (!strcmp(path, "/"))
		path = "";

	for (child = node->children; child; child = child->next) {
		char *child_path;

		child_path = l_strdup_printf("%s/%s", path, child->subpath);

		collect_instances(child->node, child_path, announce);

		l_free(child_path);
	}
}

static bool match_interfaces_added_object(const void *a, const void *b)
{
	const struct interface_add_record *rec = a;

	return rec->object == b;
}

static bool match_interfaces_removed_object(const void *a, const void *b)
{
	const struct interface_remove_record *rec = a;

	return rec->object == b;
}

bool _dbus_object_tree_add_interface(struct _dbus_object_tree *tree,
					const char *path, const char *interface,
					void *user_data)
{
	struct object_node *object;
	struct l_dbus_interface *dbi;
	struct interface_instance *instance;
	const struct l_queue_entry *entry;
	struct object_manager *manager;
	size_t path_len;
	struct interface_add_record *change_rec;

	dbi = l_hashmap_lookup(tree->interfaces, interface);
	if (!dbi)
		return false;

	object = l_hashmap_lookup(tree->objects, path);
	if (!object) {
		object = _dbus_object_tree_new_object(tree, path, NULL, NULL);

		if (!object)
			return false;
	}

	/*
	 * Check to make sure we do not have this interface already
	 * registered for this object
	 */
	if (l_queue_find(object->instances, match_interface_instance_ptr, dbi))
		return false;

	instance = l_new(struct interface_instance, 1);
	instance->interface = dbi;
	instance->user_data = user_data;

	l_queue_push_tail(object->instances, instance);

	for (entry = l_queue_get_entries(tree->object_managers); entry;
			entry = entry->next) {
		manager = entry->data;
		path_len = strlen(manager->path);

		if (strncmp(path, manager->path, path_len) ||
				(path[path_len] != '\0' &&
				 path[path_len] != '/' && path_len > 1))
			continue;

		change_rec = l_queue_find(manager->announce_added,
						match_interfaces_added_object,
						object);
		if (!change_rec) {
			change_rec = l_new(struct interface_add_record, 1);
			change_rec->path = l_strdup(path);
			change_rec->object = object;
			change_rec->instances = l_queue_new();

			l_queue_push_tail(manager->announce_added, change_rec);
		}

		/* No need to check for duplicates here */
		l_queue_push_tail(change_rec->instances, instance);

		schedule_emit_signals(manager->dbus);
	}

	if (!strcmp(interface, L_DBUS_INTERFACE_OBJECT_MANAGER)) {
		manager = l_new(struct object_manager, 1);
		manager->path = l_strdup(path);
		manager->dbus = instance->user_data;
		manager->announce_added = l_queue_new();
		manager->announce_removed = l_queue_new();

		l_queue_push_tail(tree->object_managers, manager);

		/* Emit InterfacesAdded for interfaces added before OM */
		collect_instances(object, path, manager->announce_added);

		if (manager->dbus && !l_queue_isempty(manager->announce_added))
			schedule_emit_signals(manager->dbus);
	}

	return true;
}

void *_dbus_object_tree_get_interface_data(struct _dbus_object_tree *tree,
						const char *path,
						const char *interface)
{
	struct object_node *object;
	struct interface_instance *instance;

	object = l_hashmap_lookup(tree->objects, path);
	if (!object)
		return NULL;

	instance = l_queue_find(object->instances, match_interface_instance,
				(char *) interface);
	if (!instance)
		return NULL;

	return instance->user_data;
}

static bool match_object_manager_path(const void *a, const void *b)
{
	const struct object_manager *manager = a;

	return !strcmp(manager->path, b);
}

bool _dbus_object_tree_remove_interface(struct _dbus_object_tree *tree,
					const char *path, const char *interface)
{
	struct object_node *node;
	struct interface_instance *instance;
	const struct l_queue_entry *entry;
	struct object_manager *manager;
	size_t path_len;
	struct interface_add_record *interfaces_added_rec;
	struct interface_remove_record *interfaces_removed_rec;
	struct property_change_record *property_change_rec;

	node = l_hashmap_lookup(tree->objects, path);
	if (!node)
		return false;

	instance = l_queue_remove_if(node->instances,
			match_interface_instance, (char *) interface);
	if (!instance)
		return false;

	if (!strcmp(interface, L_DBUS_INTERFACE_OBJECT_MANAGER)) {
		manager = l_queue_remove_if(tree->object_managers,
						match_object_manager_path,
						(char *) path);

		if (manager)
			object_manager_free(manager);
	}

	for (entry = l_queue_get_entries(tree->object_managers); entry;
			entry = entry->next) {
		manager = entry->data;
		path_len = strlen(manager->path);

		if (strncmp(path, manager->path, path_len) ||
				(path[path_len] != '\0' &&
				 path[path_len] != '/' && path_len > 1))
			continue;

		interfaces_added_rec = l_queue_find(manager->announce_added,
						match_interfaces_added_object,
						node);
		if (interfaces_added_rec && l_queue_remove(
						interfaces_added_rec->instances,
						instance)) {
			if (l_queue_isempty(interfaces_added_rec->instances))
				l_queue_remove(manager->announce_added,
						interfaces_added_rec);

			interface_add_record_free(interfaces_added_rec);

			continue;
		}

		interfaces_removed_rec = l_queue_find(manager->announce_removed,
						match_interfaces_removed_object,
						node);
		if (!interfaces_removed_rec) {
			interfaces_removed_rec =
				l_new(struct interface_remove_record, 1);
			interfaces_removed_rec->path = l_strdup(path);
			interfaces_removed_rec->object = node;
			interfaces_removed_rec->interface_names =
				l_queue_new();
			l_queue_push_tail(manager->announce_removed,
						interfaces_removed_rec);
		}

		/* No need to check for duplicates here */
		l_queue_push_tail(interfaces_removed_rec->interface_names,
					l_strdup(interface));

		schedule_emit_signals(manager->dbus);
	}

	property_change_rec = l_queue_remove_if(tree->property_changes,
						match_property_changes_instance,
						instance);
	if (property_change_rec)
		property_change_record_free(property_change_rec);

	interface_instance_free(instance);

	return true;
}

static void generate_interface_instance(void *data, void *user)
{
	struct interface_instance *instance = data;
	struct l_string *buf = user;

	_dbus_interface_introspection(instance->interface, buf);
}

void _dbus_object_tree_introspect(struct _dbus_object_tree *tree,
					const char *path, struct l_string *buf)
{
	struct object_node *node;
	struct child_node *child;

	node = l_hashmap_lookup(tree->objects, path);
	if (!node)
		node = _dbus_object_tree_lookup(tree, path);

	l_string_append(buf, XML_HEAD);
	l_string_append(buf, "<node>\n");

	if (node) {
		l_string_append(buf, static_introspectable);
		l_queue_foreach(node->instances,
					generate_interface_instance, buf);

		for (child = node->children; child; child = child->next)
			l_string_append_printf(buf, "\t<node name=\"%s\"/>\n",
						child->subpath);
	}

	l_string_append(buf, "</node>\n");
}

bool _dbus_object_tree_dispatch(struct _dbus_object_tree *tree,
					struct l_dbus *dbus,
					struct l_dbus_message *message)
{
	const char *path;
	const char *interface;
	const char *member;
	const char *msg_sig;
	const char *sig;
	struct object_node *node;
	struct interface_instance *instance;
	struct _dbus_method *method;
	struct l_dbus_message *reply;

	path = l_dbus_message_get_path(message);
	interface = l_dbus_message_get_interface(message);
	member = l_dbus_message_get_member(message);
	msg_sig = l_dbus_message_get_signature(message);

	/*
	 * Nothing in the spec explicitly forbids this, but handling of such
	 * messages is left up to the implementation.
	 *
	 * TODO: Another route is to go looking for a matching method under this
	 * object and call it.
	 */
	if (!interface)
		return false;

	if (!msg_sig)
		msg_sig = "";

	if (!strcmp(interface, "org.freedesktop.DBus.Introspectable") &&
			!strcmp(member, "Introspect") &&
			!strcmp(msg_sig, "")) {
		struct l_string *buf;
		char *xml;

		buf = l_string_new(0);
		_dbus_object_tree_introspect(tree, path, buf);
		xml = l_string_unwrap(buf);

		reply = l_dbus_message_new_method_return(message);
		l_dbus_message_set_arguments(reply, "s", xml);
		l_dbus_send(dbus, reply);

		l_free(xml);

		return true;
	}

	node = l_hashmap_lookup(tree->objects, path);
	if (!node)
		return false;

	instance = l_queue_find(node->instances,
				match_interface_instance, (char *) interface);
	if (!instance)
		return false;

	method = _dbus_interface_find_method(instance->interface, member);
	if (!method)
		return false;

	sig = method->metainfo + method->name_len + 1;

	if (strcmp(msg_sig, sig))
		return false;

	reply = method->cb(dbus, message, instance->user_data);
	if (reply)
		l_dbus_send(dbus, reply);

	return true;
}

LIB_EXPORT bool l_dbus_property_changed(struct l_dbus *dbus, const char *path,
					const char *interface,
					const char *property)
{
	return _dbus_object_tree_property_changed(dbus, path, interface,
							property);
}

static struct l_dbus_message *properties_get(struct l_dbus *dbus,
						struct l_dbus_message *message,
						void *user_data)
{
	const struct interface_instance *instance;
	const char *interface_name, *property_name, *signature;
	const struct _dbus_property *property;
	struct _dbus_object_tree *tree = _dbus_get_tree(dbus);
	const struct object_node *object;
	struct l_dbus_message *reply;
	struct l_dbus_message_builder *builder;

	if (!l_dbus_message_get_arguments(message, "ss", &interface_name,
						&property_name))
		return l_dbus_message_new_error(message,
						"org.freedesktop.DBus.Error."
						"InvalidArgs",
						"Invalid arguments");

	object = l_hashmap_lookup(tree->objects,
					l_dbus_message_get_path(message));
	/* If we got here the object must exist */

	instance = l_queue_find(object->instances,
				match_interface_instance,
				(char *) interface_name);
	if (!instance)
		return l_dbus_message_new_error(message,
						"org.freedesktop.DBus.Error."
						"InvalidArgs",
						"Object has no interface %s",
						interface_name);

	property = _dbus_interface_find_property(instance->interface,
							property_name);
	if (!property)
		return l_dbus_message_new_error(message,
						"org.freedesktop.DBus.Error."
						"InvalidArgs",
						"Unknown Property %s",
						property_name);


	reply = l_dbus_message_new_method_return(message);
	builder = l_dbus_message_builder_new(reply);

	signature = property->metainfo + strlen(property->metainfo) + 1;

	l_dbus_message_builder_enter_variant(builder, signature);

	if (property->getter(dbus, message, builder, instance->user_data)) {
		l_dbus_message_builder_leave_variant(builder);
		l_dbus_message_builder_finalize(builder);
	} else {
		l_dbus_message_unref(reply);

		reply = l_dbus_message_new_error(message,
						"org.freedesktop.DBus.Error."
						"Failed",
						"Getting property value "
						"failed");
	}

	l_dbus_message_builder_destroy(builder);

	return reply;
}

static struct l_dbus_message *properties_set(struct l_dbus *dbus,
						struct l_dbus_message *message,
						void *user_data)
{
	struct l_dbus_interface *interface;
	const struct interface_instance *instance;
	const char *interface_name, *property_name;
	const struct _dbus_property *property;
	struct l_dbus_message_iter variant;
	struct _dbus_object_tree *tree = _dbus_get_tree(dbus);
	const struct object_node *object;
	struct l_dbus_message *reply;
	l_dbus_property_complete_cb_t complete_cb;

	if (!l_dbus_message_get_arguments(message, "ssv", &interface_name,
						&property_name, &variant))
		return l_dbus_message_new_error(message,
						"org.freedesktop.DBus.Error."
						"InvalidArgs",
						"Invalid arguments");

	interface = l_hashmap_lookup(tree->interfaces, interface_name);
	if (!interface)
		return l_dbus_message_new_error(message,
						"org.freedesktop.DBus.Error."
						"InvalidArgs",
						"Unknown Interface %s",
						interface_name);

	property = _dbus_interface_find_property(interface, property_name);
	if (!property)
		return l_dbus_message_new_error(message,
						"org.freedesktop.DBus.Error."
						"InvalidArgs",
						"Unknown Property %s",
						property_name);

	if (!property->setter)
		return l_dbus_message_new_error(message,
						"org.freedesktop.DBus.Error."
						"InvalidArgs",
						"Property %s is read-only",
						property_name);

	object = l_hashmap_lookup(tree->objects,
					l_dbus_message_get_path(message));
	/* If we got here the object must exist */

	instance = l_queue_find(object->instances,
				match_interface_instance_ptr, interface);
	if (!instance)
		return l_dbus_message_new_error(message,
						"org.freedesktop.DBus.Error."
						"InvalidArgs",
						"Object has no interface %s",
						interface_name);

	if (property->flags & L_DBUS_PROPERTY_FLAG_AUTO_EMIT)
		complete_cb = pending_property_set_done_emit;
	else
		complete_cb = pending_property_set_done;

	reply = property->setter(dbus, l_dbus_message_ref(message), &variant,
					complete_cb, instance->user_data);

	if (reply)
		complete_cb(dbus, message, reply);

	return NULL;
}

static struct l_dbus_message *properties_get_all(struct l_dbus *dbus,
						struct l_dbus_message *message,
						void *user_data)
{
	const struct interface_instance *instance;
	const char *interface_name;
	struct _dbus_object_tree *tree = _dbus_get_tree(dbus);
	const struct object_node *object;
	struct l_dbus_message *reply;
	struct l_dbus_message_builder *builder;

	if (!l_dbus_message_get_arguments(message, "s", &interface_name))
		return l_dbus_message_new_error(message,
						"org.freedesktop.DBus.Error."
						"InvalidArgs",
						"Invalid arguments");

	object = l_hashmap_lookup(tree->objects,
					l_dbus_message_get_path(message));
	/* If we got here the object must exist */

	instance = l_queue_find(object->instances,
				match_interface_instance,
				(char *) interface_name);
	if (!instance)
		return l_dbus_message_new_error(message,
						"org.freedesktop.DBus.Error."
						"InvalidArgs",
						"Object has no interface %s",
						interface_name);

	reply = l_dbus_message_new_method_return(message);
	builder = l_dbus_message_builder_new(reply);

	if (!get_properties_dict(dbus, message, builder, instance->interface,
					instance->user_data)) {
		l_dbus_message_unref(reply);

		reply = l_dbus_message_new_error(message,
						"org.freedesktop.DBus.Error."
						"Failed",
						"Getting property values "
						"failed");
	} else
		l_dbus_message_builder_finalize(builder);

	l_dbus_message_builder_destroy(builder);

	return reply;
}

static void properties_setup_func(struct l_dbus_interface *interface)
{
	l_dbus_interface_method(interface, "Get", 0,
				properties_get, "v", "ss",
				"value", "interface_name", "property_name");
	l_dbus_interface_method(interface, "Set", 0,
				properties_set, "", "ssv",
				"interface_name", "property_name", "value");
	l_dbus_interface_method(interface, "GetAll", 0,
				properties_get_all, "a{sv}", "s",
				"props", "interface_name");

	l_dbus_interface_signal(interface, "PropertiesChanged", 0, "sa{sv}as",
				"interface_name", "changed_properties",
				"invalidated_properties");
}

static bool collect_objects(struct l_dbus *dbus, struct l_dbus_message *message,
				struct l_dbus_message_builder *builder,
				const struct object_node *node,
				const char *path)
{
	const struct l_queue_entry *entry;
	const struct child_node *child;
	char *child_path;
	const struct interface_instance *instance;
	bool r;

	if (!node->instances)
		goto recurse;

	l_dbus_message_builder_enter_dict(builder, "oa{sa{sv}}");
	l_dbus_message_builder_append_basic(builder, 'o', path);
	l_dbus_message_builder_enter_array(builder, "{sa{sv}}");

	for (entry = l_queue_get_entries(node->instances); entry;
			entry = entry->next) {
		instance = entry->data;

		l_dbus_message_builder_enter_dict(builder, "sa{sv}");
		l_dbus_message_builder_append_basic(builder, 's',
						instance->interface->name);

		if (!get_properties_dict(dbus, message, builder,
						instance->interface,
						instance->user_data))
			return false;

		l_dbus_message_builder_leave_dict(builder);
	}

	l_dbus_message_builder_leave_array(builder);
	l_dbus_message_builder_leave_dict(builder);

recurse:
	if (!strcmp(path, "/"))
		path = "";

	for (child = node->children; child; child = child->next) {
		child_path = l_strdup_printf("%s/%s", path, child->subpath);

		r = collect_objects(dbus, message, builder,
					child->node, child_path);

		l_free(child_path);

		if (!r)
			return false;
	}

	return true;
}

struct l_dbus_message *_dbus_object_tree_get_objects(
						struct _dbus_object_tree *tree,
						struct l_dbus *dbus,
						const char *path,
						struct l_dbus_message *message)
{
	const struct object_node *node;
	struct l_dbus_message *reply;
	struct l_dbus_message_builder *builder;

	node = l_hashmap_lookup(tree->objects, path);

	reply = l_dbus_message_new_method_return(message);
	builder = l_dbus_message_builder_new(reply);

	l_dbus_message_builder_enter_array(builder, "{oa{sa{sv}}}");

	if (!collect_objects(dbus, message, builder, node, path)) {
		l_dbus_message_builder_destroy(builder);
		l_dbus_message_unref(reply);

		return l_dbus_message_new_error(message,
						"org.freedesktop.DBus.Error."
						"Failed",
						"Getting property values "
						"failed");
	}

	l_dbus_message_builder_leave_array(builder);

	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);

	return reply;
}

static struct l_dbus_message *get_managed_objects(struct l_dbus *dbus,
						struct l_dbus_message *message,
						void *user_data)
{
	struct _dbus_object_tree *tree = _dbus_get_tree(dbus);
	const char *path = l_dbus_message_get_path(message);

	return _dbus_object_tree_get_objects(tree, dbus, path, message);
}

static void object_manager_setup_func(struct l_dbus_interface *interface)
{
	l_dbus_interface_method(interface, "GetManagedObjects", 0,
				get_managed_objects, "a{oa{sa{sv}}}", "",
				"objpath_interfaces_and_properties");

	l_dbus_interface_signal(interface, "InterfacesAdded", 0, "oa{sa{sv}}",
				"object_path", "interfaces_and_properties");
	l_dbus_interface_signal(interface, "InterfacesRemoved", 0, "oas",
				"object_path", "interfaces");
}
