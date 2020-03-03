/*
 * Copyright (C) 2009, 2010 Red Hat Inc, Steven Rostedt <srostedt@redhat.com>
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License (not later!)
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not,  see <http://www.gnu.org/licenses>
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include "event-parse.h"
#include "event-utils.h"

#define LOCAL_PLUGIN_DIR ".traceevent/plugins"

static struct registered_plugin_options {
	struct registered_plugin_options	*next;
	struct pevent_plugin_option		*options;
} *registered_options;

static struct trace_plugin_options {
	struct trace_plugin_options	*next;
	char				*plugin;
	char				*option;
	char				*value;
} *trace_plugin_options;

struct plugin_list {
	struct plugin_list	*next;
	char			*name;
	void			*handle;
};

static void lower_case(char *str)
{
	if (!str)
		return;
	for (; *str; str++)
		*str = tolower(*str);
}

static int update_option_value(struct pevent_plugin_option *op, const char *val)
{
	char *op_val;

	if (!val) {
		/* toggle, only if option is boolean */
		if (op->value)
			/* Warn? */
			return 0;
		op->set ^= 1;
		return 0;
	}

	/*
	 * If the option has a value then it takes a string
	 * otherwise the option is a boolean.
	 */
	if (op->value) {
		op->value = val;
		return 0;
	}

	/* Option is boolean, must be either "1", "0", "true" or "false" */

	op_val = strdup(val);
	if (!op_val)
		return -1;
	lower_case(op_val);

	if (strcmp(val, "1") == 0 || strcmp(val, "true") == 0)
		op->set = 1;
	else if (strcmp(val, "0") == 0 || strcmp(val, "false") == 0)
		op->set = 0;
	free(op_val);

	return 0;
}

/**
 * traceevent_plugin_list_options - get list of plugin options
 *
 * Returns an array of char strings that list the currently registered
 * plugin options in the format of <plugin>:<option>. This list can be
 * used by toggling the option.
 *
 * Returns NULL if there's no options registered. On error it returns
 * INVALID_PLUGIN_LIST_OPTION
 *
 * Must be freed with traceevent_plugin_free_options_list().
 */
char **traceevent_plugin_list_options(void)
{
	struct registered_plugin_options *reg;
	struct pevent_plugin_option *op;
	char **list = NULL;
	char *name;
	int count = 0;

	for (reg = registered_options; reg; reg = reg->next) {
		for (op = reg->options; op->name; op++) {
			char *alias = op->plugin_alias ? op->plugin_alias : op->file;
			char **temp = list;

			name = malloc(strlen(op->name) + strlen(alias) + 2);
			if (!name)
				goto err;

			sprintf(name, "%s:%s", alias, op->name);
			list = realloc(list, count + 2);
			if (!list) {
				list = temp;
				free(name);
				goto err;
			}
			list[count++] = name;
			list[count] = NULL;
		}
	}
	return list;

 err:
	while (--count >= 0)
		free(list[count]);
	free(list);

	return INVALID_PLUGIN_LIST_OPTION;
}

void traceevent_plugin_free_options_list(char **list)
{
	int i;

	if (!list)
		return;

	if (list == INVALID_PLUGIN_LIST_OPTION)
		return;

	for (i = 0; list[i]; i++)
		free(list[i]);

	free(list);
}

static int
update_option(const char *file, struct pevent_plugin_option *option)
{
	struct trace_plugin_options *op;
	char *plugin;
	int ret = 0;

	if (option->plugin_alias) {
		plugin = strdup(option->plugin_alias);
		if (!plugin)
			return -1;
	} else {
		char *p;
		plugin = strdup(file);
		if (!plugin)
			return -1;
		p = strstr(plugin, ".");
		if (p)
			*p = '\0';
	}

	/* first look for named options */
	for (op = trace_plugin_options; op; op = op->next) {
		if (!op->plugin)
			continue;
		if (strcmp(op->plugin, plugin) != 0)
			continue;
		if (strcmp(op->option, option->name) != 0)
			continue;

		ret = update_option_value(option, op->value);
		if (ret)
			goto out;
		break;
	}

	/* first look for unnamed options */
	for (op = trace_plugin_options; op; op = op->next) {
		if (op->plugin)
			continue;
		if (strcmp(op->option, option->name) != 0)
			continue;

		ret = update_option_value(option, op->value);
		break;
	}

 out:
	free(plugin);
	return ret;
}

/**
 * traceevent_plugin_add_options - Add a set of options by a plugin
 * @name: The name of the plugin adding the options
 * @options: The set of options being loaded
 *
 * Sets the options with the values that have been added by user.
 */
int traceevent_plugin_add_options(const char *name,
				  struct pevent_plugin_option *options)
{
	struct registered_plugin_options *reg;

	reg = malloc(sizeof(*reg));
	if (!reg)
		return -1;
	reg->next = registered_options;
	reg->options = options;
	registered_options = reg;

	while (options->name) {
		update_option(name, options);
		options++;
	}
	return 0;
}

/**
 * traceevent_plugin_remove_options - remove plugin options that were registered
 * @options: Options to removed that were registered with traceevent_plugin_add_options
 */
void traceevent_plugin_remove_options(struct pevent_plugin_option *options)
{
	struct registered_plugin_options **last;
	struct registered_plugin_options *reg;

	for (last = &registered_options; *last; last = &(*last)->next) {
		if ((*last)->options == options) {
			reg = *last;
			*last = reg->next;
			free(reg);
			return;
		}
	}
}

/**
 * traceevent_print_plugins - print out the list of plugins loaded
 * @s: the trace_seq descripter to write to
 * @prefix: The prefix string to add before listing the option name
 * @suffix: The suffix string ot append after the option name
 * @list: The list of plugins (usually returned by traceevent_load_plugins()
 *
 * Writes to the trace_seq @s the list of plugins (files) that is
 * returned by traceevent_load_plugins(). Use @prefix and @suffix for formating:
 * @prefix = "  ", @suffix = "\n".
 */
void traceevent_print_plugins(struct trace_seq *s,
			      const char *prefix, const char *suffix,
			      const struct plugin_list *list)
{
	while (list) {
		trace_seq_printf(s, "%s%s%s", prefix, list->name, suffix);
		list = list->next;
	}
}

static void
load_plugin(struct pevent *pevent, const char *path,
	    const char *file, void *data)
{
	struct plugin_list **plugin_list = data;
	pevent_plugin_load_func func;
	struct plugin_list *list;
	const char *alias;
	char *plugin;
	void *handle;

	plugin = malloc(strlen(path) + strlen(file) + 2);
	if (!plugin) {
		warning("could not allocate plugin memory\n");
		return;
	}

	strcpy(plugin, path);
	strcat(plugin, "/");
	strcat(plugin, file);

	handle = dlopen(plugin, RTLD_NOW | RTLD_GLOBAL);
	if (!handle) {
		warning("could not load plugin '%s'\n%s\n",
			plugin, dlerror());
		goto out_free;
	}

	alias = dlsym(handle, PEVENT_PLUGIN_ALIAS_NAME);
	if (!alias)
		alias = file;

	func = dlsym(handle, PEVENT_PLUGIN_LOADER_NAME);
	if (!func) {
		warning("could not find func '%s' in plugin '%s'\n%s\n",
			PEVENT_PLUGIN_LOADER_NAME, plugin, dlerror());
		goto out_free;
	}

	list = malloc(sizeof(*list));
	if (!list) {
		warning("could not allocate plugin memory\n");
		goto out_free;
	}

	list->next = *plugin_list;
	list->handle = handle;
	list->name = plugin;
	*plugin_list = list;

	pr_stat("registering plugin: %s", plugin);
	func(pevent);
	return;

 out_free:
	free(plugin);
}

static void
load_plugins_dir(struct pevent *pevent, const char *suffix,
		 const char *path,
		 void (*load_plugin)(struct pevent *pevent,
				     const char *path,
				     const char *name,
				     void *data),
		 void *data)
{
	struct dirent *dent;
	struct stat st;
	DIR *dir;
	int ret;

	ret = stat(path, &st);
	if (ret < 0)
		return;

	if (!S_ISDIR(st.st_mode))
		return;

	dir = opendir(path);
	if (!dir)
		return;

	while ((dent = readdir(dir))) {
		const char *name = dent->d_name;

		if (strcmp(name, ".") == 0 ||
		    strcmp(name, "..") == 0)
			continue;

		/* Only load plugins that end in suffix */
		if (strcmp(name + (strlen(name) - strlen(suffix)), suffix) != 0)
			continue;

		load_plugin(pevent, path, name, data);
	}

	closedir(dir);
}

static void
load_plugins(struct pevent *pevent, const char *suffix,
	     void (*load_plugin)(struct pevent *pevent,
				 const char *path,
				 const char *name,
				 void *data),
	     void *data)
{
	char *home;
	char *path;
	char *envdir;

	if (pevent->flags & PEVENT_DISABLE_PLUGINS)
		return;

	/*
	 * If a system plugin directory was defined,
	 * check that first.
	 */
#ifdef PLUGIN_DIR
	if (!(pevent->flags & PEVENT_DISABLE_SYS_PLUGINS))
		load_plugins_dir(pevent, suffix, PLUGIN_DIR,
				 load_plugin, data);
#endif

	/*
	 * Next let the environment-set plugin directory
	 * override the system defaults.
	 */
	envdir = getenv("TRACEEVENT_PLUGIN_DIR");
	if (envdir)
		load_plugins_dir(pevent, suffix, envdir, load_plugin, data);

	/*
	 * Now let the home directory override the environment
	 * or system defaults.
	 */
	home = getenv("HOME");
	if (!home)
		return;

	path = malloc(strlen(home) + strlen(LOCAL_PLUGIN_DIR) + 2);
	if (!path) {
		warning("could not allocate plugin memory\n");
		return;
	}

	strcpy(path, home);
	strcat(path, "/");
	strcat(path, LOCAL_PLUGIN_DIR);

	load_plugins_dir(pevent, suffix, path, load_plugin, data);

	free(path);
}

struct plugin_list*
traceevent_load_plugins(struct pevent *pevent)
{
	struct plugin_list *list = NULL;

	load_plugins(pevent, ".so", load_plugin, &list);
	return list;
}

void
traceevent_unload_plugins(struct plugin_list *plugin_list, struct pevent *pevent)
{
	pevent_plugin_unload_func func;
	struct plugin_list *list;

	while (plugin_list) {
		list = plugin_list;
		plugin_list = list->next;
		func = dlsym(list->handle, PEVENT_PLUGIN_UNLOADER_NAME);
		if (func)
			func(pevent);
		dlclose(list->handle);
		free(list->name);
		free(list);
	}
}
