// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2012
 * Joe Hershberger, National Instruments, joe.hershberger@ni.com
 */

#include <common.h>
#include <environment.h>

#if defined(CONFIG_NEEDS_MANUAL_RELOC)
DECLARE_GLOBAL_DATA_PTR;
#endif

/*
 * Look up a callback function pointer by name
 */
static struct env_clbk_tbl *find_env_callback(const char *name)
{
	struct env_clbk_tbl *clbkp;
	int i;
	int num_callbacks = ll_entry_count(struct env_clbk_tbl, env_clbk);

	if (name == NULL)
		return NULL;

	/* look up the callback in the linker-list */
	for (i = 0, clbkp = ll_entry_start(struct env_clbk_tbl, env_clbk);
	     i < num_callbacks;
	     i++, clbkp++) {
		if (strcmp(name, clbkp->name) == 0)
			return clbkp;
	}

	return NULL;
}

static int first_call = 1;
static const char *callback_list;

/*
 * Look for a possible callback for a newly added variable
 * This is called specifically when the variable did not exist in the hash
 * previously, so the blanket update did not find this variable.
 */
void env_callback_init(ENTRY *var_entry)
{
	const char *var_name = var_entry->key;
	char callback_name[256] = "";
	struct env_clbk_tbl *clbkp;
	int ret = 1;

	if (first_call) {
		callback_list = env_get(ENV_CALLBACK_VAR);
		first_call = 0;
	}

	/* look in the ".callbacks" var for a reference to this variable */
	if (callback_list != NULL)
		ret = env_attr_lookup(callback_list, var_name, callback_name);

	/* only if not found there, look in the static list */
	if (ret)
		ret = env_attr_lookup(ENV_CALLBACK_LIST_STATIC, var_name,
			callback_name);

	/* if an association was found, set the callback pointer */
	if (!ret && strlen(callback_name)) {
		clbkp = find_env_callback(callback_name);
		if (clbkp != NULL)
#if defined(CONFIG_NEEDS_MANUAL_RELOC)
			var_entry->callback = clbkp->callback + gd->reloc_off;
#else
			var_entry->callback = clbkp->callback;
#endif
	}
}

/*
 * Called on each existing env var prior to the blanket update since removing
 * a callback association should remove its callback.
 */
static int clear_callback(ENTRY *entry)
{
	entry->callback = NULL;

	return 0;
}

/*
 * Call for each element in the list that associates variables to callbacks
 */
static int set_callback(const char *name, const char *value, void *priv)
{
	ENTRY e, *ep;
	struct env_clbk_tbl *clbkp;

	e.key	= name;
	e.data	= NULL;
	e.callback = NULL;
	hsearch_r(e, FIND, &ep, &env_htab, 0);

	/* does the env variable actually exist? */
	if (ep != NULL) {
		/* the assocaition delares no callback, so remove the pointer */
		if (value == NULL || strlen(value) == 0)
			ep->callback = NULL;
		else {
			/* assign the requested callback */
			clbkp = find_env_callback(value);
			if (clbkp != NULL)
#if defined(CONFIG_NEEDS_MANUAL_RELOC)
				ep->callback = clbkp->callback + gd->reloc_off;
#else
				ep->callback = clbkp->callback;
#endif
		}
	}

	return 0;
}

static int on_callbacks(const char *name, const char *value, enum env_op op,
	int flags)
{
	/* remove all callbacks */
	hwalk_r(&env_htab, clear_callback);

	/* configure any static callback bindings */
	env_attr_walk(ENV_CALLBACK_LIST_STATIC, set_callback, NULL);
	/* configure any dynamic callback bindings */
	env_attr_walk(value, set_callback, NULL);

	return 0;
}
U_BOOT_ENV_CALLBACK(callbacks, on_callbacks);
