#include "config.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <libgen.h>

#include <pwd.h>
#include <grp.h>
#include <keyutils.h>
#include <nfsidmap.h>

#include <unistd.h>
#include "xlog.h"
#include "conffile.h"

int verbose = 0;
char *usage = "Usage: %s [-vh] [-c || [-u|-g|-r key] || -d || -l || [-t timeout] key desc]";

#define MAX_ID_LEN   11
#define IDMAP_NAMESZ 128
#define USER  1
#define GROUP 0

#define PROCKEYS "/proc/keys"
#ifndef DEFAULT_KEYRING
#define DEFAULT_KEYRING ".id_resolver"
#endif

#ifndef PATH_IDMAPDCONF
#define PATH_IDMAPDCONF "/etc/idmapd.conf"
#endif

#define UIDKEYS 0x1
#define GIDKEYS 0x2

#ifndef HAVE_FIND_KEY_BY_TYPE_AND_DESC
static key_serial_t find_key_by_type_and_desc(const char *type,
		const char *desc, key_serial_t destringid)
{
	char buf[BUFSIZ];
	key_serial_t key;
	FILE *fp;

	if ((fp = fopen(PROCKEYS, "r")) == NULL) {
		xlog_err("fopen(%s) failed: %m", PROCKEYS);
		return -1;
	}

	key = -1;
	while(fgets(buf, BUFSIZ, fp) != NULL) {
		unsigned int id;

		if (strstr(buf, type) == NULL)
			continue;
		if (strstr(buf, desc) == NULL)
			continue;
		if (sscanf(buf, "%x %*s", &id) != 1) {
			xlog_err("Unparsable keyring entry in %s", PROCKEYS);
			continue;
		}

		key = (key_serial_t)id;
		break;
	}

	fclose(fp);
	return key;
}
#endif

/*
 * Clear all the keys on the given keyring
 */
static int keyring_clear(const char *keyring)
{
	key_serial_t key;

	key = find_key_by_type_and_desc("keyring", keyring, 0);
	if (key == -1) {
		if (verbose)
			xlog_warn("'%s' keyring was not found.", keyring);
		return EXIT_SUCCESS;
	}

	if (keyctl_clear(key) < 0) {
		xlog_err("keyctl_clear(0x%x) failed: %m",
				(unsigned int)key);
		return EXIT_FAILURE;
	}

	if (verbose)
		xlog_warn("'%s' cleared", keyring);
	return EXIT_SUCCESS;
}

static int display_default_domain(void)
{
	char domain[NFS4_MAX_DOMAIN_LEN];
	int rc;

	rc = nfs4_get_default_domain(NULL, domain, NFS4_MAX_DOMAIN_LEN);
	if (rc) {
		xlog_errno(rc, "nfs4_get_default_domain failed: %m");
		return EXIT_FAILURE;
	}

	printf("%s\n", domain);
	return EXIT_SUCCESS;
}

static void list_key(key_serial_t key)
{
	char *buffer, *c;
	int rc;

	rc = keyctl_describe_alloc(key, &buffer);
	if (rc < 0) {
		switch (errno) {
		case EKEYEXPIRED:
			printf("Expired key not displayed\n");
			break;
		default:
			xlog_err("Failed to describe key: %m");
		}
		return;
	}

	c = strrchr(buffer, ';');
	if (!c) {
		xlog_err("Unparsable key not displayed\n");
		goto out_free;
	}
	printf("  %s\n", ++c);

out_free:
	free(buffer);
}

static void list_keys(const char *ring_name, key_serial_t ring_id)
{
	key_serial_t *key;
	void *keylist;
	int count;

	count = keyctl_read_alloc(ring_id, &keylist);
	if (count < 0) {
		xlog_err("Failed to read keyring %s: %m", ring_name);
		return;
	}
	count /= (int)sizeof(*key);

	switch (count) {
	case 0:
		printf("No %s keys found.\n", ring_name);
		break;
	case 1:
		printf("1 %s key found:\n", ring_name);
		break;
	default:
		printf("%u %s keys found:\n", count, ring_name);
	}

	for (key = keylist; count--; key++)
		list_key(*key);

	free(keylist);
}

/*
 * List all keys on a keyring
 */
static int list_keyring(const char *keyring)
{
	key_serial_t key;

	key = find_key_by_type_and_desc("keyring", keyring, 0);
	if (key == -1) {
		xlog_err("'%s' keyring was not found.", keyring);
		return EXIT_FAILURE;
	}

	list_keys(keyring, key);
	return EXIT_SUCCESS;
}

/*
 * Find either a user or group id based on the name@domain string
 */
static int id_lookup(char *name_at_domain, key_serial_t key, int type)
{
	char id[MAX_ID_LEN];
	uid_t uid = 0;
	gid_t gid = 0;
	int rc;

	if (type == USER) {
		rc = nfs4_owner_to_uid(name_at_domain, &uid);
		sprintf(id, "%u", uid);
	} else {
		rc = nfs4_group_owner_to_gid(name_at_domain, &gid);
		sprintf(id, "%u", gid);
	}
	if (rc < 0) {
		xlog_errno(rc, "id_lookup: %s: failed: %m",
			(type == USER ? "nfs4_owner_to_uid" : "nfs4_group_owner_to_gid"));
		return EXIT_FAILURE;
	}

	rc = EXIT_SUCCESS;
	if (keyctl_instantiate(key, id, strlen(id) + 1, 0)) {
		switch (errno) {
		case EDQUOT:
		case ENFILE:
		case ENOMEM:
			/*
			 * The keyring is full. Clear the keyring and try again
			 */
			rc = keyring_clear(DEFAULT_KEYRING);
			if (rc)
				break;
			if (keyctl_instantiate(key, id, strlen(id) + 1, 0)) {
				rc = EXIT_FAILURE;
				xlog_err("id_lookup: keyctl_instantiate failed: %m");
			}
			break;
		default:
			rc = EXIT_FAILURE;
			break;
		}
	}

	return rc;
}

/*
 * Find the name@domain string from either a user or group id
 */
static int name_lookup(char *id, key_serial_t key, int type)
{
	char name[IDMAP_NAMESZ];
	char domain[NFS4_MAX_DOMAIN_LEN];
	uid_t uid;
	gid_t gid;
	int rc;

	rc = nfs4_get_default_domain(NULL, domain, NFS4_MAX_DOMAIN_LEN);
	if (rc) {
		xlog_errno(rc,
			"name_lookup: nfs4_get_default_domain failed: %m");
		return EXIT_FAILURE;
	}

	if (type == USER) {
		uid = atoi(id);
		rc = nfs4_uid_to_name(uid, domain, name, IDMAP_NAMESZ);
	} else {
		gid = atoi(id);
		rc = nfs4_gid_to_name(gid, domain, name, IDMAP_NAMESZ);
	}
	if (rc) {
		xlog_errno(rc, "name_lookup: %s: failed: %m",
			(type == USER ? "nfs4_uid_to_name" : "nfs4_gid_to_name"));
		return EXIT_FAILURE;
	}

	rc = EXIT_SUCCESS;
	if (keyctl_instantiate(key, &name, strlen(name), 0)) {
		rc = EXIT_FAILURE;
		xlog_err("name_lookup: keyctl_instantiate failed: %m");
	}

	return rc;
}

/*
 * Revoke a key 
 */
static int key_invalidate(char *keystr, int keymask)
{
	FILE *fp;
	char buf[BUFSIZ], *ptr;
	key_serial_t key;
	int mask;

	xlog_syslog(0);

	if ((fp = fopen(PROCKEYS, "r")) == NULL) {
		xlog_err("fopen(%s) failed: %m", PROCKEYS);
		return EXIT_FAILURE;
	}

	while(fgets(buf, BUFSIZ, fp) != NULL) {
		if (strstr(buf, "keyring") != NULL)
			continue;

		mask = 0;
		if ((ptr = strstr(buf, "uid:")) != NULL)
			mask = UIDKEYS;
		else if ((ptr = strstr(buf, "gid:")) != NULL)
			mask = GIDKEYS;
		else 
			continue;

		if ((keymask & mask) == 0)
			continue;

		if (strncmp(ptr+4, keystr, strlen(keystr)) != 0)
			continue;

		if (verbose) {
			*(strchr(buf, '\n')) = '\0';
			xlog_warn("invalidating '%s'", buf);
		}
		/*
		 * The key is the first arugment in the string
		 */
		*(strchr(buf, ' ')) = '\0';
		sscanf(buf, "%x", &key);

/* older libkeyutils compatibility */
#ifndef KEYCTL_INVALIDATE
#define KEYCTL_INVALIDATE 21      /* invalidate a key */
#endif
		if (keyctl(KEYCTL_INVALIDATE, key) < 0) {
			if (errno != EOPNOTSUPP) {
				xlog_err("keyctl_invalidate(0x%x) failed: %m", key);
				fclose(fp);
				return EXIT_FAILURE;
			} else {
				/* older kernel compatibility attempt: */
				if (keyctl_revoke(key) < 0) {
					xlog_err("keyctl_revoke(0x%x) failed: %m", key);
					fclose(fp);
					return EXIT_FAILURE;
				}
			}
		}

		keymask &= ~mask;
		if (keymask == 0) {
			fclose(fp);
			return EXIT_SUCCESS;
		}
	}
	xlog_err("'%s' key was not found.", keystr);
	fclose(fp);
	return EXIT_FAILURE;
}

int main(int argc, char **argv)
{
	char *arg;
	char *value;
	char *type;
	int rc = 1, opt;
	int timeout = 600;
	key_serial_t key;
	char *progname, *keystr = NULL;
	int clearing = 0, keymask = 0, display = 0, list = 0;

	/* Set the basename */
	if ((progname = strrchr(argv[0], '/')) != NULL)
		progname++;
	else
		progname = argv[0];

	xlog_open(progname);

	while ((opt = getopt(argc, argv, "hdu:g:r:ct:vl")) != -1) {
		switch (opt) {
		case 'd':
			display++;
			break;
		case 'l':
			list++;
			break;
		case 'u':
			keymask = UIDKEYS;
			keystr = strdup(optarg);
			break;
		case 'g':
			keymask = GIDKEYS;
			keystr = strdup(optarg);
			break;
		case 'r':
			keymask = GIDKEYS|UIDKEYS;
			keystr = strdup(optarg);
			break;
		case 'c':
			clearing++;
			break;
		case 'v':
			verbose++;
			break;
		case 't':
			timeout = atoi(optarg);
			break;
		case 'h':
		default:
			xlog_warn(usage, progname);
			exit(opt == 'h' ? 0 : 1);
		}
	}

	if (geteuid() != 0) {
		xlog_err("Must be run as root.");
		return EXIT_FAILURE;
	}

	if ((rc = nfs4_init_name_mapping(PATH_IDMAPDCONF)))  {
		xlog_errno(rc, "Unable to create name to user id mappings.");
		return EXIT_FAILURE;
	}
	if (!verbose)
		verbose = conf_get_num("General", "Verbosity", 0);

	if (display)
		return display_default_domain();
	if (list)
		return list_keyring(DEFAULT_KEYRING);
	if (keystr) {
		return key_invalidate(keystr, keymask);
	}
	if (clearing) {
		xlog_syslog(0);
		return keyring_clear(DEFAULT_KEYRING);
	}

	xlog_stderr(verbose);
	if ((argc - optind) != 2) {
		xlog_warn("Bad arg count. Check /etc/request-key.conf");
		xlog_warn(usage, progname);
		return EXIT_FAILURE;
	}

	if (verbose)
		nfs4_set_debug(verbose, NULL);

	key = strtol(argv[optind++], NULL, 10);

	arg = strdup(argv[optind]);
	if (arg == NULL) {
		xlog_err("strdup failed: %m");
		return EXIT_FAILURE;
	}
	type = strtok(arg, ":");
	value = strtok(NULL, ":");
	if (value == NULL) {
		free(arg);
		xlog_err("Error: Null uid/gid value.");
		return EXIT_FAILURE;
	}
	if (verbose) {
		xlog_warn("key: 0x%lx type: %s value: %s timeout %ld",
			key, type, value, timeout);
	}

	/* Become a possesor of the to-be-instantiated key to set the key's timeout */
	request_key("keyring", DEFAULT_KEYRING, NULL, KEY_SPEC_THREAD_KEYRING);

	if (strcmp(type, "uid") == 0)
		rc = id_lookup(value, key, USER);
	else if (strcmp(type, "gid") == 0)
		rc = id_lookup(value, key, GROUP);
	else if (strcmp(type, "user") == 0)
		rc = name_lookup(value, key, USER);
	else if (strcmp(type, "group") == 0)
		rc = name_lookup(value, key, GROUP);

	/* Set timeout to 10 (600 seconds) minutes */
	if (rc == EXIT_SUCCESS)
		keyctl_set_timeout(key, timeout);

	free(arg);
	return rc;
}
