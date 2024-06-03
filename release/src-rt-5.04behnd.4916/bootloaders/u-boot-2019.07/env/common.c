// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2001 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Andreas Heppel <aheppel@sysgo.de>
 */

#include <common.h>
#include <command.h>
#include <environment.h>
#include <linux/stddef.h>
#include <search.h>
#include <errno.h>
#include <malloc.h>

DECLARE_GLOBAL_DATA_PTR;

/************************************************************************
 * Default settings to be used when no valid environment is found
 */
#include <env_default.h>

struct hsearch_data env_htab = {
#if CONFIG_IS_ENABLED(ENV_SUPPORT)
	/* defined in flags.c, only compile with ENV_SUPPORT */
	.change_ok = env_flags_validate,
#endif
};

/*
 * Read an environment variable as a boolean
 * Return -1 if variable does not exist (default to true)
 */
int env_get_yesno(const char *var)
{
	char *s = env_get(var);

	if (s == NULL)
		return -1;
	return (*s == '1' || *s == 'y' || *s == 'Y' || *s == 't' || *s == 'T') ?
		1 : 0;
}

/*
 * Look up the variable from the default environment
 */
char *env_get_default(const char *name)
{
	char *ret_val;
	unsigned long really_valid = gd->env_valid;
	unsigned long real_gd_flags = gd->flags;

	/* Pretend that the image is bad. */
	gd->flags &= ~GD_FLG_ENV_READY;
	gd->env_valid = ENV_INVALID;
	ret_val = env_get(name);
	gd->env_valid = really_valid;
	gd->flags = real_gd_flags;
	return ret_val;
}

void set_default_env(const char *s, int flags)
{
	if (sizeof(default_environment) > ENV_SIZE) {
		puts("*** Error - default environment is too large\n\n");
		return;
	}

	if (s) {
		if ((flags & H_INTERACTIVE) == 0) {
			printf("*** Warning - %s, "
				"using default environment\n\n", s);
		} else {
			puts(s);
		}
	} else {
		debug("Using default environment\n");
	}

	if (himport_r(&env_htab, (char *)default_environment,
			sizeof(default_environment), '\0', flags, 0,
			0, NULL) == 0)
		pr_err("## Error: Environment import failed: errno = %d\n",
		       errno);

	gd->flags |= GD_FLG_ENV_READY;
	gd->flags |= GD_FLG_ENV_DEFAULT;
}


/* [re]set individual variables to their value in the default environment */
int set_default_vars(int nvars, char * const vars[], int flags)
{
	/*
	 * Special use-case: import from default environment
	 * (and use \0 as a separator)
	 */
	flags |= H_NOCLEAR;
	return himport_r(&env_htab, (const char *)default_environment,
				sizeof(default_environment), '\0',
				flags, 0, nvars, vars);
}

/*
 * Check if CRC is valid and (if yes) import the environment.
 * Note that "buf" may or may not be aligned.
 */
int env_import(const char *buf, int check)
{
	env_t *ep = (env_t *)buf;

	if (check) {
		uint32_t crc;

		memcpy(&crc, &ep->crc, sizeof(crc));

		if (crc32(0, ep->data, ENV_SIZE) != crc) {
			set_default_env("bad CRC", 0);
			return -ENOMSG; /* needed for env_load() */
		}
	}

	if (himport_r(&env_htab, (char *)ep->data, ENV_SIZE, '\0', 0, 0,
			0, NULL)) {
		gd->flags |= GD_FLG_ENV_READY;
		return 0;
	}

	pr_err("Cannot import environment: errno = %d\n", errno);

	set_default_env("import failed", 0);

	return -EIO;
}

#ifdef CONFIG_SYS_REDUNDAND_ENVIRONMENT
static unsigned char env_flags;

int env_import_redund(const char *buf1, int buf1_read_fail,
		      const char *buf2, int buf2_read_fail)
{
	int crc1_ok, crc2_ok;
	env_t *ep, *tmp_env1, *tmp_env2;

	tmp_env1 = (env_t *)buf1;
	tmp_env2 = (env_t *)buf2;

	if (buf1_read_fail && buf2_read_fail) {
		puts("*** Error - No Valid Environment Area found\n");
	} else if (buf1_read_fail || buf2_read_fail) {
		puts("*** Warning - some problems detected ");
		puts("reading environment; recovered successfully\n");
	}

	if (buf1_read_fail && buf2_read_fail) {
		set_default_env("bad env area", 0);
		return -EIO;
	} else if (!buf1_read_fail && buf2_read_fail) {
		gd->env_valid = ENV_VALID;
		return env_import((char *)tmp_env1, 1);
	} else if (buf1_read_fail && !buf2_read_fail) {
		gd->env_valid = ENV_REDUND;
		return env_import((char *)tmp_env2, 1);
	}

	crc1_ok = crc32(0, tmp_env1->data, ENV_SIZE) ==
			tmp_env1->crc;
	crc2_ok = crc32(0, tmp_env2->data, ENV_SIZE) ==
			tmp_env2->crc;

	if (!crc1_ok && !crc2_ok) {
		set_default_env("bad CRC", 0);
		return -ENOMSG; /* needed for env_load() */
	} else if (crc1_ok && !crc2_ok) {
		gd->env_valid = ENV_VALID;
	} else if (!crc1_ok && crc2_ok) {
		gd->env_valid = ENV_REDUND;
	} else {
		/* both ok - check serial */
		if (tmp_env1->flags == 255 && tmp_env2->flags == 0)
			gd->env_valid = ENV_REDUND;
		else if (tmp_env2->flags == 255 && tmp_env1->flags == 0)
			gd->env_valid = ENV_VALID;
		else if (tmp_env1->flags > tmp_env2->flags)
			gd->env_valid = ENV_VALID;
		else if (tmp_env2->flags > tmp_env1->flags)
			gd->env_valid = ENV_REDUND;
		else /* flags are equal - almost impossible */
			gd->env_valid = ENV_VALID;
	}

	if (gd->env_valid == ENV_VALID)
		ep = tmp_env1;
	else
		ep = tmp_env2;

	env_flags = ep->flags;
	return env_import((char *)ep, 0);
}
#endif /* CONFIG_SYS_REDUNDAND_ENVIRONMENT */

/* Export the environment and generate CRC for it. */
int env_export(env_t *env_out)
{
	char *res;
	ssize_t	len;

	res = (char *)env_out->data;
	len = hexport_r(&env_htab, '\0', 0, &res, ENV_SIZE, 0, NULL);
	if (len < 0) {
		pr_err("Cannot export environment: errno = %d\n", errno);
		return 1;
	}

	env_out->crc = crc32(0, env_out->data, ENV_SIZE);

#ifdef CONFIG_SYS_REDUNDAND_ENVIRONMENT
	env_out->flags = ++env_flags; /* increase the serial */
#endif

	return 0;
}

void env_relocate(void)
{
#if defined(CONFIG_NEEDS_MANUAL_RELOC)
	env_reloc();
	env_fix_drivers();

	if (env_htab.change_ok)
		env_htab.change_ok += gd->reloc_off;
#endif
	if (gd->env_valid == ENV_INVALID) {
#if defined(CONFIG_ENV_IS_NOWHERE) || defined(CONFIG_SPL_BUILD)
		/* Environment not changable */
		set_default_env(NULL, 0);
#else
		bootstage_error(BOOTSTAGE_ID_NET_CHECKSUM);
		set_default_env("bad CRC", 0);
#endif
	} else {
		env_load();
	}
}

#ifdef CONFIG_AUTO_COMPLETE
int env_complete(char *var, int maxv, char *cmdv[], int bufsz, char *buf,
		 bool dollar_comp)
{
	ENTRY *match;
	int found, idx;

	if (dollar_comp) {
		/*
		 * When doing $ completion, the first character should
		 * obviously be a '$'.
		 */
		if (var[0] != '$')
			return 0;

		var++;

		/*
		 * The second one, if present, should be a '{', as some
		 * configuration of the u-boot shell expand ${var} but not
		 * $var.
		 */
		if (var[0] == '{')
			var++;
		else if (var[0] != '\0')
			return 0;
	}

	idx = 0;
	found = 0;
	cmdv[0] = NULL;


	while ((idx = hmatch_r(var, idx, &match, &env_htab))) {
		int vallen = strlen(match->key) + 1;

		if (found >= maxv - 2 ||
		    bufsz < vallen + (dollar_comp ? 3 : 0))
			break;

		cmdv[found++] = buf;

		/* Add the '${' prefix to each var when doing $ completion. */
		if (dollar_comp) {
			strcpy(buf, "${");
			buf += 2;
			bufsz -= 3;
		}

		memcpy(buf, match->key, vallen);
		buf += vallen;
		bufsz -= vallen;

		if (dollar_comp) {
			/*
			 * This one is a bit odd: vallen already contains the
			 * '\0' character but we need to add the '}' suffix,
			 * hence the buf - 1 here. strcpy() will add the '\0'
			 * character just after '}'. buf is then incremented
			 * to account for the extra '}' we just added.
			 */
			strcpy(buf - 1, "}");
			buf++;
		}
	}

	qsort(cmdv, found, sizeof(cmdv[0]), strcmp_compar);

	if (idx)
		cmdv[found++] = dollar_comp ? "${...}" : "...";

	cmdv[found] = NULL;
	return found;
}
#endif
