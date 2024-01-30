/* Simplistic plugin support for DDNS service providers
 *
 * Copyright (c) 2012-2021  Joachim Wiberg <troglobit@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "config.h"
#include <assert.h>
#include <errno.h>
#include <dirent.h>		/* readdir() et al */
#include <string.h>

#include "ddns.h"

static char *plugpath = NULL;   /* Set by first load. */
static TAILQ_HEAD(, ddns_system) plugins = TAILQ_HEAD_INITIALIZER(plugins);

int plugin_register(ddns_system_t *plugin, const char *req)
{
	if (!plugin) {
		errno = EINVAL;
		return 1;
	}

	assert(plugin->name);

	/* Already registered? */
	if (plugin_find(plugin->name, 0)) {
		logit(LOG_DEBUG, "... %s already loaded.", plugin->name);
		return 0;
	}

	plugin->server_req = req;
	TAILQ_INSERT_TAIL(&plugins, plugin, link);

	return 0;
}

/* clones v4 plugin to create from default@foo.org -> ipv6@foo.org */
int plugin_register_v6(ddns_system_t *plugin, const char *req)
{
	ddns_system_t *p;

	p = malloc(sizeof(*p));
	if (!p)
		return 1;

	memcpy(p, plugin, sizeof(*p));
	/* default@foo.org -> ipv6@foo.org */
	p->name = strdup(plugin->name);
	p->cloned = 1;
	sprintf(p->name, "ipv6%s", plugin->name + 7);

	return plugin_register(p, req);
}

int plugin_unregister(ddns_system_t *plugin)
{
	ddns_system_t *plugin_v4, *plugin_v6;
	char *name;

	plugin_v4 = plugin_find(plugin->name, 0);
	if (!plugin_v4)		/* already unregistered */
		return 0;

	name = strdup(plugin->name);
	if (strstr(name, "default@"))
		sprintf(name, "ipv6%s", plugin->name + 7);

	TAILQ_REMOVE(&plugins, plugin, link);
	plugin_v6 = plugin_find(name, 0);
	if (plugin_v6 && plugin_v6->cloned) {
		TAILQ_REMOVE(&plugins, plugin_v6, link);
		free(plugin_v6->name);
		free(plugin_v6);
	}
	free(name);
	/* XXX: Unfinished, add cleanup code here! */

	return 0;
}

static ddns_system_t *search_plugin(const char *name, int loose)
{
	ddns_system_t *p, *tmp;

	if (loose) {
		PLUGIN_ITERATOR(p, tmp) {
			if (strcasestr(p->name, name))
				return p;
			if (p->alias && strcasestr(p->alias, name))
				return p;
		}

		return NULL;
	}

	PLUGIN_ITERATOR(p, tmp) {
		if (!strcasecmp(p->name, name))
			return p;
		if (p->alias && !strcasecmp(p->alias, name))
			return p;
	}

	return NULL;
}

/**
 * plugin_list - List all plugins
 *
 * Returns:
 * Always successful, returning POSIX OK(0).
 */
int plugin_list(int json)
{
	ddns_system_t *p, *tmp;

	if (json) {
		int prev = 0;

		printf("[");
		PLUGIN_ITERATOR(p, tmp) {
			printf("%s{\"name\":\"%s\",", prev ? "," : "", p->name);
			if (p->alias)
				printf("\"alias\":\"%s\",", p->alias);
			printf("\"checkip-server\":\"%s\","
			       "\"checkip-url\":\"%s\","
			       "\"update-server\":\"%s\","
			       "\"update-url\":\"%s\"}",
			       p->checkip_name, p->checkip_url,
			       p->server_name, p->server_url);
			prev++;
		}
		puts("]");
	} else {
		printf("\e[7m%-28s %-22s %-26s %-20s %-30s %-15s\e[0m\n", "PROVIDER", "ALIAS", "CHECKIP SERVER", "URL", "UPDATE SERVER", "URL");
		PLUGIN_ITERATOR(p, tmp) {
			printf("%-28s %-22s %-26s %-20s %-30s %s\n", p->name, p->alias ?: "",
			       p->checkip_name, p->checkip_url,
			       p->server_name, p->server_url);
		}
	}

	return 0;
}

/**
 * plugin_show - Show a specific plugin
 * @name: full name of plugin, including default@ or ipv6@
 *
 * Returns:
 * POSIX OK(0) when found, otherise 1.
 */
int plugin_show(char *name)
{
	ddns_system_t *p;
	char *req;

	p = plugin_find(name, 0);
	if (!p)
		p = plugin_find(name, 1);
	if (!p) {
		fprintf(stderr, "No such plugin '%s', even tried substring match\n", name);
		return 1;
	}

	if (p->server_req) {
		size_t i, pos = 0, len = strlen(p->server_req);

		req = malloc(len * 2);
		for (i = 0; i < len; i++) {
			char c = p->server_req[i];

			if (c == '\\')
				req[pos++] = '\\';
			req[pos++] = c;
		}
		req[pos++] = 0;
	} else
		req = strdup("");

	printf("Name           : %s\n"
	       "nousername     : %s\n"
	       "checkip server : %s\n"
	       "checkip URL    : %s\n"
	       "update server  : %s\n"
	       "update URL     : %s\n"
	       "update REQ     : ",
	       p->name,
	       p->nousername ? "true" : "false",
	       p->checkip_name, p->checkip_url,
	       p->server_name, p->server_url);
	puts(req);
	free(req);

	return 0;
}

/**
 * plugin_find - Find a plugin by name
 * @name: With or without path, or .so extension
 * @loose: Use substring match when looking for plugin
 *
 * This function uses an opporunistic search for a suitable plugin and
 * returns the first match.  Albeit with at least some measure of
 * heuristics.
 *
 * First it checks for an exact match.  If no match is found and @name
 * starts with a slash the search ends.  Otherwise a new search with the
 * plugin path prepended to @name is made.  Also, if @name does not end
 * with .so it too is added to @name before searching.
 *
 * Returns:
 * On success the pointer to the matching &ddns_system_t is returned,
 * otherwise %NULL is returned.
 */
ddns_system_t *plugin_find(const char *name, int loose)
{
	char *tmp, *ptr;
	ddns_system_t *p;

	if (!name) {
		errno = EINVAL;
		return NULL;
	}

	/* Check for multiple instances of plugin */
	tmp = strdup(name);
	if (!tmp)
		return NULL;

	ptr = strchr(tmp, ':');
	if (ptr)
		*ptr = 0;
	name = tmp;

	p = search_plugin(name, loose);
	if (p) {
		free(tmp);
		return p;
	}

	if (plugpath && name[0] != '/') {
		int noext;
		char *path;
		size_t len = strlen(plugpath) + strlen(name) + 5;

		path = malloc(len);
		if (!path) {
			free(tmp);
			return NULL;
		}

		noext = strcmp(name + strlen(name) - 3, ".so");
		snprintf(path, len, "%s%s%s%s", plugpath,
			 plugpath[strlen(plugpath) - 1] == '/' ? "" : "/",
			 name, noext ? ".so" : "");

		p = search_plugin(path, loose);
		free(path);
		if (p) {
			free(tmp);
			return p;
		}
	}

	free(tmp);
	errno = ENOENT;

	return NULL;
}

/* Private daemon API *******************************************************/
#if o
/**
 * load_one - Load one plugin
 * @path: Path to finit plugins, usually %PLUGIN_PATH
 * @name: Name of plugin, optionally ending in ".so"
 *
 * Loads a plugin from @path/@name[.so].  Note, if ".so" is missing from
 * the plugin @name it is added before attempting to load.
 *
 * It is up to the plugin itself ot register itself as a "ctor" with the
 * %PLUGIN_INIT macro so that plugin_register() is called automatically.
 *
 * Returns:
 * POSIX OK(0) on success, non-zero otherwise.
 */
static int load_one(char *path, char *name)
{
	int noext;
	char plugin[256];
	void *handle;

	if (!path || !name) {
		errno = EINVAL;
		return 1;
	}

	/* Compose full path, with optional .so extension, to plugin */
	noext = strcmp(name + strlen(name) - 3, ".so");
	snprintf(plugin, sizeof(plugin), "%s/%s%s", path, name, noext ? ".so" : "");

	logit(LOG_DEBUG, "Loading plugin %s ...", basename(plugin));
	handle = dlopen(plugin, RTLD_LAZY | RTLD_GLOBAL);
	if (!handle) {
		logit(LOG_ERR, "Failed loading plugin %s: %s", plugin, dlerror());
		return 1;
	}

	return 0;
}

int plugin_load_all(char *path)
{
	int fail = 0;
	DIR *dp = opendir(path);
	struct dirent *entry;

	if (!dp) {
		logit(LOG_ERR, "Failed, cannot open plugin directory %s: %s", path, strerror(errno));
		return 1;
	}
	plugpath = path;

	while ((entry = readdir(dp))) {
		if (entry->d_name[0] == '.')
			continue; /* Skip . and .. directories */

		if (load_one(path, entry->d_name))
			fail++;
	}

	closedir(dp);

	return fail;
}
#endif

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
