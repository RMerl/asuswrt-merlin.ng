/* Helpers for initial module or kernel cmdline parsing
   Copyright (C) 2001 Rusty Russell.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/ctype.h>

/* Protects all parameters, and incidentally kmalloced_param list. */
static DEFINE_MUTEX(param_lock);

/* This just allows us to keep track of which parameters are kmalloced. */
struct kmalloced_param {
	struct list_head list;
	char val[];
};
static LIST_HEAD(kmalloced_params);

static void *kmalloc_parameter(unsigned int size)
{
	struct kmalloced_param *p;

	p = kmalloc(sizeof(*p) + size, GFP_KERNEL);
	if (!p)
		return NULL;

	list_add(&p->list, &kmalloced_params);
	return p->val;
}

/* Does nothing if parameter wasn't kmalloced above. */
static void maybe_kfree_parameter(void *param)
{
	struct kmalloced_param *p;

	list_for_each_entry(p, &kmalloced_params, list) {
		if (p->val == param) {
			list_del(&p->list);
			kfree(p);
			break;
		}
	}
}

static char dash2underscore(char c)
{
	if (c == '-')
		return '_';
	return c;
}

bool parameqn(const char *a, const char *b, size_t n)
{
	size_t i;

	for (i = 0; i < n; i++) {
		if (dash2underscore(a[i]) != dash2underscore(b[i]))
			return false;
	}
	return true;
}

bool parameq(const char *a, const char *b)
{
	return parameqn(a, b, strlen(a)+1);
}

static void param_check_unsafe(const struct kernel_param *kp)
{
	if (kp->flags & KERNEL_PARAM_FL_UNSAFE) {
		pr_warn("Setting dangerous option %s - tainting kernel\n",
			kp->name);
		add_taint(TAINT_USER, LOCKDEP_STILL_OK);
	}
}

static int parse_one(char *param,
		     char *val,
		     const char *doing,
		     const struct kernel_param *params,
		     unsigned num_params,
		     s16 min_level,
		     s16 max_level,
		     int (*handle_unknown)(char *param, char *val,
				     const char *doing))
{
	unsigned int i;
	int err;

	/* Find parameter */
	for (i = 0; i < num_params; i++) {
		if (parameq(param, params[i].name)) {
			if (params[i].level < min_level
			    || params[i].level > max_level)
				return 0;
			/* No one handled NULL, so do it here. */
			if (!val &&
			    !(params[i].ops->flags & KERNEL_PARAM_OPS_FL_NOARG))
				return -EINVAL;
			pr_debug("handling %s with %p\n", param,
				params[i].ops->set);
			mutex_lock(&param_lock);
			param_check_unsafe(&params[i]);
			err = params[i].ops->set(val, &params[i]);
			mutex_unlock(&param_lock);
			return err;
		}
	}

	if (handle_unknown) {
		pr_debug("doing %s: %s='%s'\n", doing, param, val);
		return handle_unknown(param, val, doing);
	}

	pr_debug("Unknown argument '%s'\n", param);
	return -ENOENT;
}

/* You can use " around spaces, but can't escape ". */
/* Hyphens and underscores equivalent in parameter names. */
static char *next_arg(char *args, char **param, char **val)
{
	unsigned int i, equals = 0;
	int in_quote = 0, quoted = 0;
	char *next;

	if (*args == '"') {
		args++;
		in_quote = 1;
		quoted = 1;
	}

	for (i = 0; args[i]; i++) {
		if (isspace(args[i]) && !in_quote)
			break;
		if (equals == 0) {
			if (args[i] == '=')
				equals = i;
		}
		if (args[i] == '"')
			in_quote = !in_quote;
	}

	*param = args;
	if (!equals)
		*val = NULL;
	else {
		args[equals] = '\0';
		*val = args + equals + 1;

		/* Don't include quotes in value. */
		if (**val == '"') {
			(*val)++;
			if (args[i-1] == '"')
				args[i-1] = '\0';
		}
	}
	if (quoted && args[i-1] == '"')
		args[i-1] = '\0';

	if (args[i]) {
		args[i] = '\0';
		next = args + i + 1;
	} else
		next = args + i;

	/* Chew up trailing spaces. */
	return skip_spaces(next);
}

/* Args looks like "foo=bar,bar2 baz=fuz wiz". */
char *parse_args(const char *doing,
		 char *args,
		 const struct kernel_param *params,
		 unsigned num,
		 s16 min_level,
		 s16 max_level,
		 int (*unknown)(char *param, char *val, const char *doing))
{
	char *param, *val;

	/* Chew leading spaces */
	args = skip_spaces(args);

	if (*args)
		pr_debug("doing %s, parsing ARGS: '%s'\n", doing, args);

	while (*args) {
		int ret;
		int irq_was_disabled;

		args = next_arg(args, &param, &val);
		/* Stop at -- */
		if (!val && strcmp(param, "--") == 0)
			return args;
		irq_was_disabled = irqs_disabled();
		ret = parse_one(param, val, doing, params, num,
				min_level, max_level, unknown);
		if (irq_was_disabled && !irqs_disabled())
			pr_warn("%s: option '%s' enabled irq's!\n",
				doing, param);

		switch (ret) {
		case -ENOENT:
			pr_err("%s: Unknown parameter `%s'\n", doing, param);
			return ERR_PTR(ret);
		case -ENOSPC:
			pr_err("%s: `%s' too large for parameter `%s'\n",
			       doing, val ?: "", param);
			return ERR_PTR(ret);
		case 0:
			break;
		default:
			pr_err("%s: `%s' invalid for parameter `%s'\n",
			       doing, val ?: "", param);
			return ERR_PTR(ret);
		}
	}

	/* All parsed OK. */
	return NULL;
}

/* Lazy bastard, eh? */
#define STANDARD_PARAM_DEF(name, type, format, strtolfn)      		\
	int param_set_##name(const char *val, const struct kernel_param *kp) \
	{								\
		return strtolfn(val, 0, (type *)kp->arg);		\
	}								\
	int param_get_##name(char *buffer, const struct kernel_param *kp) \
	{								\
		return scnprintf(buffer, PAGE_SIZE, format,		\
				*((type *)kp->arg));			\
	}								\
	struct kernel_param_ops param_ops_##name = {			\
		.set = param_set_##name,				\
		.get = param_get_##name,				\
	};								\
	EXPORT_SYMBOL(param_set_##name);				\
	EXPORT_SYMBOL(param_get_##name);				\
	EXPORT_SYMBOL(param_ops_##name)


STANDARD_PARAM_DEF(byte, unsigned char, "%hhu", kstrtou8);
STANDARD_PARAM_DEF(short, short, "%hi", kstrtos16);
STANDARD_PARAM_DEF(ushort, unsigned short, "%hu", kstrtou16);
STANDARD_PARAM_DEF(int, int, "%i", kstrtoint);
STANDARD_PARAM_DEF(uint, unsigned int, "%u", kstrtouint);
STANDARD_PARAM_DEF(long, long, "%li", kstrtol);
STANDARD_PARAM_DEF(ulong, unsigned long, "%lu", kstrtoul);
STANDARD_PARAM_DEF(ullong, unsigned long long, "%llu", kstrtoull);

int param_set_charp(const char *val, const struct kernel_param *kp)
{
	if (strlen(val) > 1024) {
		pr_err("%s: string parameter too long\n", kp->name);
		return -ENOSPC;
	}

	maybe_kfree_parameter(*(char **)kp->arg);

	/* This is a hack.  We can't kmalloc in early boot, and we
	 * don't need to; this mangled commandline is preserved. */
	if (slab_is_available()) {
		*(char **)kp->arg = kmalloc_parameter(strlen(val)+1);
		if (!*(char **)kp->arg)
			return -ENOMEM;
		strcpy(*(char **)kp->arg, val);
	} else
		*(const char **)kp->arg = val;

	return 0;
}
EXPORT_SYMBOL(param_set_charp);

int param_get_charp(char *buffer, const struct kernel_param *kp)
{
	return scnprintf(buffer, PAGE_SIZE, "%s", *((char **)kp->arg));
}
EXPORT_SYMBOL(param_get_charp);

static void param_free_charp(void *arg)
{
	maybe_kfree_parameter(*((char **)arg));
}

struct kernel_param_ops param_ops_charp = {
	.set = param_set_charp,
	.get = param_get_charp,
	.free = param_free_charp,
};
EXPORT_SYMBOL(param_ops_charp);

/* Actually could be a bool or an int, for historical reasons. */
int param_set_bool(const char *val, const struct kernel_param *kp)
{
	/* No equals means "set"... */
	if (!val) val = "1";

	/* One of =[yYnN01] */
	return strtobool(val, kp->arg);
}
EXPORT_SYMBOL(param_set_bool);

int param_get_bool(char *buffer, const struct kernel_param *kp)
{
	/* Y and N chosen as being relatively non-coder friendly */
	return sprintf(buffer, "%c", *(bool *)kp->arg ? 'Y' : 'N');
}
EXPORT_SYMBOL(param_get_bool);

struct kernel_param_ops param_ops_bool = {
	.flags = KERNEL_PARAM_OPS_FL_NOARG,
	.set = param_set_bool,
	.get = param_get_bool,
};
EXPORT_SYMBOL(param_ops_bool);

/* This one must be bool. */
int param_set_invbool(const char *val, const struct kernel_param *kp)
{
	int ret;
	bool boolval;
	struct kernel_param dummy;

	dummy.arg = &boolval;
	ret = param_set_bool(val, &dummy);
	if (ret == 0)
		*(bool *)kp->arg = !boolval;
	return ret;
}
EXPORT_SYMBOL(param_set_invbool);

int param_get_invbool(char *buffer, const struct kernel_param *kp)
{
	return sprintf(buffer, "%c", (*(bool *)kp->arg) ? 'N' : 'Y');
}
EXPORT_SYMBOL(param_get_invbool);

struct kernel_param_ops param_ops_invbool = {
	.set = param_set_invbool,
	.get = param_get_invbool,
};
EXPORT_SYMBOL(param_ops_invbool);

int param_set_bint(const char *val, const struct kernel_param *kp)
{
	struct kernel_param boolkp;
	bool v;
	int ret;

	/* Match bool exactly, by re-using it. */
	boolkp = *kp;
	boolkp.arg = &v;

	ret = param_set_bool(val, &boolkp);
	if (ret == 0)
		*(int *)kp->arg = v;
	return ret;
}
EXPORT_SYMBOL(param_set_bint);

struct kernel_param_ops param_ops_bint = {
	.flags = KERNEL_PARAM_OPS_FL_NOARG,
	.set = param_set_bint,
	.get = param_get_int,
};
EXPORT_SYMBOL(param_ops_bint);

/* We break the rule and mangle the string. */
static int param_array(const char *name,
		       const char *val,
		       unsigned int min, unsigned int max,
		       void *elem, int elemsize,
		       int (*set)(const char *, const struct kernel_param *kp),
		       s16 level,
		       unsigned int *num)
{
	int ret;
	struct kernel_param kp;
	char save;

	/* Get the name right for errors. */
	kp.name = name;
	kp.arg = elem;
	kp.level = level;

	*num = 0;
	/* We expect a comma-separated list of values. */
	do {
		int len;

		if (*num == max) {
			pr_err("%s: can only take %i arguments\n", name, max);
			return -EINVAL;
		}
		len = strcspn(val, ",");

		/* nul-terminate and parse */
		save = val[len];
		((char *)val)[len] = '\0';
		BUG_ON(!mutex_is_locked(&param_lock));
		ret = set(val, &kp);

		if (ret != 0)
			return ret;
		kp.arg += elemsize;
		val += len+1;
		(*num)++;
	} while (save == ',');

	if (*num < min) {
		pr_err("%s: needs at least %i arguments\n", name, min);
		return -EINVAL;
	}
	return 0;
}

static int param_array_set(const char *val, const struct kernel_param *kp)
{
	const struct kparam_array *arr = kp->arr;
	unsigned int temp_num;

	return param_array(kp->name, val, 1, arr->max, arr->elem,
			   arr->elemsize, arr->ops->set, kp->level,
			   arr->num ?: &temp_num);
}

static int param_array_get(char *buffer, const struct kernel_param *kp)
{
	int i, off, ret;
	const struct kparam_array *arr = kp->arr;
	struct kernel_param p;

	p = *kp;
	for (i = off = 0; i < (arr->num ? *arr->num : arr->max); i++) {
		if (i)
			buffer[off++] = ',';
		p.arg = arr->elem + arr->elemsize * i;
		BUG_ON(!mutex_is_locked(&param_lock));
		ret = arr->ops->get(buffer + off, &p);
		if (ret < 0)
			return ret;
		off += ret;
	}
	buffer[off] = '\0';
	return off;
}

static void param_array_free(void *arg)
{
	unsigned int i;
	const struct kparam_array *arr = arg;

	if (arr->ops->free)
		for (i = 0; i < (arr->num ? *arr->num : arr->max); i++)
			arr->ops->free(arr->elem + arr->elemsize * i);
}

struct kernel_param_ops param_array_ops = {
	.set = param_array_set,
	.get = param_array_get,
	.free = param_array_free,
};
EXPORT_SYMBOL(param_array_ops);

int param_set_copystring(const char *val, const struct kernel_param *kp)
{
	const struct kparam_string *kps = kp->str;

	if (strlen(val)+1 > kps->maxlen) {
		pr_err("%s: string doesn't fit in %u chars.\n",
		       kp->name, kps->maxlen-1);
		return -ENOSPC;
	}
	strcpy(kps->string, val);
	return 0;
}
EXPORT_SYMBOL(param_set_copystring);

int param_get_string(char *buffer, const struct kernel_param *kp)
{
	const struct kparam_string *kps = kp->str;
	return strlcpy(buffer, kps->string, kps->maxlen);
}
EXPORT_SYMBOL(param_get_string);

struct kernel_param_ops param_ops_string = {
	.set = param_set_copystring,
	.get = param_get_string,
};
EXPORT_SYMBOL(param_ops_string);

/* sysfs output in /sys/modules/XYZ/parameters/ */
#define to_module_attr(n) container_of(n, struct module_attribute, attr)
#define to_module_kobject(n) container_of(n, struct module_kobject, kobj)

struct param_attribute
{
	struct module_attribute mattr;
	const struct kernel_param *param;
};

struct module_param_attrs
{
	unsigned int num;
	struct attribute_group grp;
	struct param_attribute attrs[0];
};

#ifdef CONFIG_SYSFS
#define to_param_attr(n) container_of(n, struct param_attribute, mattr)

static ssize_t param_attr_show(struct module_attribute *mattr,
			       struct module_kobject *mk, char *buf)
{
	int count;
	struct param_attribute *attribute = to_param_attr(mattr);

	if (!attribute->param->ops->get)
		return -EPERM;

	mutex_lock(&param_lock);
	count = attribute->param->ops->get(buf, attribute->param);
	mutex_unlock(&param_lock);
	if (count > 0) {
		strcat(buf, "\n");
		++count;
	}
	return count;
}

/* sysfs always hands a nul-terminated string in buf.  We rely on that. */
static ssize_t param_attr_store(struct module_attribute *mattr,
				struct module_kobject *km,
				const char *buf, size_t len)
{
 	int err;
	struct param_attribute *attribute = to_param_attr(mattr);

	if (!attribute->param->ops->set)
		return -EPERM;

	mutex_lock(&param_lock);
	param_check_unsafe(attribute->param);
	err = attribute->param->ops->set(buf, attribute->param);
	mutex_unlock(&param_lock);
	if (!err)
		return len;
	return err;
}
#endif

#ifdef CONFIG_MODULES
#define __modinit
#else
#define __modinit __init
#endif

#ifdef CONFIG_SYSFS
void __kernel_param_lock(void)
{
	mutex_lock(&param_lock);
}
EXPORT_SYMBOL(__kernel_param_lock);

void __kernel_param_unlock(void)
{
	mutex_unlock(&param_lock);
}
EXPORT_SYMBOL(__kernel_param_unlock);

/*
 * add_sysfs_param - add a parameter to sysfs
 * @mk: struct module_kobject
 * @kparam: the actual parameter definition to add to sysfs
 * @name: name of parameter
 *
 * Create a kobject if for a (per-module) parameter if mp NULL, and
 * create file in sysfs.  Returns an error on out of memory.  Always cleans up
 * if there's an error.
 */
static __modinit int add_sysfs_param(struct module_kobject *mk,
				     const struct kernel_param *kp,
				     const char *name)
{
	struct module_param_attrs *new_mp;
	struct attribute **new_attrs;
	unsigned int i;

	/* We don't bother calling this with invisible parameters. */
	BUG_ON(!kp->perm);

	if (!mk->mp) {
		/* First allocation. */
		mk->mp = kzalloc(sizeof(*mk->mp), GFP_KERNEL);
		if (!mk->mp)
			return -ENOMEM;
		mk->mp->grp.name = "parameters";
		/* NULL-terminated attribute array. */
		mk->mp->grp.attrs = kzalloc(sizeof(mk->mp->grp.attrs[0]),
					    GFP_KERNEL);
		/* Caller will cleanup via free_module_param_attrs */
		if (!mk->mp->grp.attrs)
			return -ENOMEM;
	}

	/* Enlarge allocations. */
	new_mp = krealloc(mk->mp,
			  sizeof(*mk->mp) +
			  sizeof(mk->mp->attrs[0]) * (mk->mp->num + 1),
			  GFP_KERNEL);
	if (!new_mp)
		return -ENOMEM;
	mk->mp = new_mp;

	/* Extra pointer for NULL terminator */
	new_attrs = krealloc(mk->mp->grp.attrs,
			     sizeof(mk->mp->grp.attrs[0]) * (mk->mp->num + 2),
			     GFP_KERNEL);
	if (!new_attrs)
		return -ENOMEM;
	mk->mp->grp.attrs = new_attrs;

	/* Tack new one on the end. */
	memset(&mk->mp->attrs[mk->mp->num], 0, sizeof(mk->mp->attrs[0]));
	sysfs_attr_init(&mk->mp->attrs[mk->mp->num].mattr.attr);
	mk->mp->attrs[mk->mp->num].param = kp;
	mk->mp->attrs[mk->mp->num].mattr.show = param_attr_show;
	/* Do not allow runtime DAC changes to make param writable. */
	if ((kp->perm & (S_IWUSR | S_IWGRP | S_IWOTH)) != 0)
		mk->mp->attrs[mk->mp->num].mattr.store = param_attr_store;
	else
		mk->mp->attrs[mk->mp->num].mattr.store = NULL;
	mk->mp->attrs[mk->mp->num].mattr.attr.name = (char *)name;
	mk->mp->attrs[mk->mp->num].mattr.attr.mode = kp->perm;
	mk->mp->num++;

	/* Fix up all the pointers, since krealloc can move us */
	for (i = 0; i < mk->mp->num; i++)
		mk->mp->grp.attrs[i] = &mk->mp->attrs[i].mattr.attr;
	mk->mp->grp.attrs[mk->mp->num] = NULL;
	return 0;
}

#ifdef CONFIG_MODULES
static void free_module_param_attrs(struct module_kobject *mk)
{
	if (mk->mp)
		kfree(mk->mp->grp.attrs);
	kfree(mk->mp);
	mk->mp = NULL;
}

/*
 * module_param_sysfs_setup - setup sysfs support for one module
 * @mod: module
 * @kparam: module parameters (array)
 * @num_params: number of module parameters
 *
 * Adds sysfs entries for module parameters under
 * /sys/module/[mod->name]/parameters/
 */
int module_param_sysfs_setup(struct module *mod,
			     const struct kernel_param *kparam,
			     unsigned int num_params)
{
	int i, err;
	bool params = false;

	for (i = 0; i < num_params; i++) {
		if (kparam[i].perm == 0)
			continue;
		err = add_sysfs_param(&mod->mkobj, &kparam[i], kparam[i].name);
		if (err) {
			free_module_param_attrs(&mod->mkobj);
			return err;
		}
		params = true;
	}

	if (!params)
		return 0;

	/* Create the param group. */
	err = sysfs_create_group(&mod->mkobj.kobj, &mod->mkobj.mp->grp);
	if (err)
		free_module_param_attrs(&mod->mkobj);
	return err;
}

/*
 * module_param_sysfs_remove - remove sysfs support for one module
 * @mod: module
 *
 * Remove sysfs entries for module parameters and the corresponding
 * kobject.
 */
void module_param_sysfs_remove(struct module *mod)
{
	if (mod->mkobj.mp) {
		sysfs_remove_group(&mod->mkobj.kobj, &mod->mkobj.mp->grp);
		/* We are positive that no one is using any param
		 * attrs at this point.  Deallocate immediately. */
		free_module_param_attrs(&mod->mkobj);
	}
}
#endif

void destroy_params(const struct kernel_param *params, unsigned num)
{
	unsigned int i;

	for (i = 0; i < num; i++)
		if (params[i].ops->free)
			params[i].ops->free(params[i].arg);
}

static struct module_kobject * __init locate_module_kobject(const char *name)
{
	struct module_kobject *mk;
	struct kobject *kobj;
	int err;

	kobj = kset_find_obj(module_kset, name);
	if (kobj) {
		mk = to_module_kobject(kobj);
	} else {
		mk = kzalloc(sizeof(struct module_kobject), GFP_KERNEL);
		BUG_ON(!mk);

		mk->mod = THIS_MODULE;
		mk->kobj.kset = module_kset;
		err = kobject_init_and_add(&mk->kobj, &module_ktype, NULL,
					   "%s", name);
#ifdef CONFIG_MODULES
		if (!err)
			err = sysfs_create_file(&mk->kobj, &module_uevent.attr);
#endif
		if (err) {
			kobject_put(&mk->kobj);
			pr_crit("Adding module '%s' to sysfs failed (%d), the system may be unstable.\n",
				name, err);
			return NULL;
		}

		/* So that we hold reference in both cases. */
		kobject_get(&mk->kobj);
	}

	return mk;
}

static void __init kernel_add_sysfs_param(const char *name,
					  const struct kernel_param *kparam,
					  unsigned int name_skip)
{
	struct module_kobject *mk;
	int err;

	mk = locate_module_kobject(name);
	if (!mk)
		return;

	/* We need to remove old parameters before adding more. */
	if (mk->mp)
		sysfs_remove_group(&mk->kobj, &mk->mp->grp);

	/* These should not fail at boot. */
	err = add_sysfs_param(mk, kparam, kparam->name + name_skip);
	BUG_ON(err);
	err = sysfs_create_group(&mk->kobj, &mk->mp->grp);
	BUG_ON(err);
	kobject_uevent(&mk->kobj, KOBJ_ADD);
	kobject_put(&mk->kobj);
}

/*
 * param_sysfs_builtin - add sysfs parameters for built-in modules
 *
 * Add module_parameters to sysfs for "modules" built into the kernel.
 *
 * The "module" name (KBUILD_MODNAME) is stored before a dot, the
 * "parameter" name is stored behind a dot in kernel_param->name. So,
 * extract the "module" name for all built-in kernel_param-eters,
 * and for all who have the same, call kernel_add_sysfs_param.
 */
static void __init param_sysfs_builtin(void)
{
	const struct kernel_param *kp;
	unsigned int name_len;
	char modname[MODULE_NAME_LEN];

	for (kp = __start___param; kp < __stop___param; kp++) {
		char *dot;

		if (kp->perm == 0)
			continue;

		dot = strchr(kp->name, '.');
		if (!dot) {
			/* This happens for core_param() */
			strcpy(modname, "kernel");
			name_len = 0;
		} else {
			name_len = dot - kp->name + 1;
			strlcpy(modname, kp->name, name_len);
		}
		kernel_add_sysfs_param(modname, kp, name_len);
	}
}

ssize_t __modver_version_show(struct module_attribute *mattr,
			      struct module_kobject *mk, char *buf)
{
	struct module_version_attribute *vattr =
		container_of(mattr, struct module_version_attribute, mattr);

	return scnprintf(buf, PAGE_SIZE, "%s\n", vattr->version);
}

extern const struct module_version_attribute *__start___modver[];
extern const struct module_version_attribute *__stop___modver[];

static void __init version_sysfs_builtin(void)
{
	const struct module_version_attribute **p;
	struct module_kobject *mk;
	int err;

	for (p = __start___modver; p < __stop___modver; p++) {
		const struct module_version_attribute *vattr = *p;

		mk = locate_module_kobject(vattr->module_name);
		if (mk) {
			err = sysfs_create_file(&mk->kobj, &vattr->mattr.attr);
			kobject_uevent(&mk->kobj, KOBJ_ADD);
			kobject_put(&mk->kobj);
		}
	}
}

/* module-related sysfs stuff */

static ssize_t module_attr_show(struct kobject *kobj,
				struct attribute *attr,
				char *buf)
{
	struct module_attribute *attribute;
	struct module_kobject *mk;
	int ret;

	attribute = to_module_attr(attr);
	mk = to_module_kobject(kobj);

	if (!attribute->show)
		return -EIO;

	ret = attribute->show(attribute, mk, buf);

	return ret;
}

static ssize_t module_attr_store(struct kobject *kobj,
				struct attribute *attr,
				const char *buf, size_t len)
{
	struct module_attribute *attribute;
	struct module_kobject *mk;
	int ret;

	attribute = to_module_attr(attr);
	mk = to_module_kobject(kobj);

	if (!attribute->store)
		return -EIO;

	ret = attribute->store(attribute, mk, buf, len);

	return ret;
}

static const struct sysfs_ops module_sysfs_ops = {
	.show = module_attr_show,
	.store = module_attr_store,
};

static int uevent_filter(struct kset *kset, struct kobject *kobj)
{
	struct kobj_type *ktype = get_ktype(kobj);

	if (ktype == &module_ktype)
		return 1;
	return 0;
}

static const struct kset_uevent_ops module_uevent_ops = {
	.filter = uevent_filter,
};

struct kset *module_kset;
int module_sysfs_initialized;

static void module_kobj_release(struct kobject *kobj)
{
	struct module_kobject *mk = to_module_kobject(kobj);
	complete(mk->kobj_completion);
}

struct kobj_type module_ktype = {
	.release   =	module_kobj_release,
	.sysfs_ops =	&module_sysfs_ops,
};

/*
 * param_sysfs_init - wrapper for built-in params support
 */
static int __init param_sysfs_init(void)
{
	module_kset = kset_create_and_add("module", &module_uevent_ops, NULL);
	if (!module_kset) {
		printk(KERN_WARNING "%s (%d): error creating kset\n",
			__FILE__, __LINE__);
		return -ENOMEM;
	}
	module_sysfs_initialized = 1;

	version_sysfs_builtin();
	param_sysfs_builtin();

	return 0;
}
subsys_initcall(param_sysfs_init);

#endif /* CONFIG_SYSFS */
