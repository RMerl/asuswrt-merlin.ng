/*
 * Copyright (C) 2004-2009 Kay Sievers <kay.sievers@vrfy.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <ctype.h>
#include <stdarg.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#if defined(RTCONFIG_HND_ROUTER_AX_6756) || defined(RTCONFIG_HND_ROUTER_BE_4916)
#include <sys/sysmacros.h>
#endif

#include "udev.h"

static bool skip_attribute(const char *name)
{
	static const char const *skip[] = {
		"uevent",
		"dev",
		"modalias",
		"resource",
		"driver",
		"subsystem",
		"module",
	};
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(skip); i++)
		if (strcmp(name, skip[i]) == 0)
			return true;
	return false;
}

static void print_all_attributes(struct udev_device *device, const char *key)
{
	struct udev_list_entry *sysattr;

	udev_list_entry_foreach(sysattr, udev_device_get_sysattr_list_entry(device)) {
		struct udev *udev = udev_device_get_udev(device);
		const char *name;
		const char *value;
		size_t len;

		name = udev_list_entry_get_name(sysattr);
		if (skip_attribute(name))
			continue;

		value = udev_device_get_sysattr_value(device, name);
		if (value == NULL)
			continue;
		dbg(udev, "attr '%s'='%s'\n", name, value);

		/* skip nonprintable attributes */
		len = strlen(value);
		while (len > 0 && isprint(value[len-1]))
			len--;
		if (len > 0) {
			dbg(udev, "attribute value of '%s' non-printable, skip\n", name);
			continue;
		}

		printf("    %s{%s}==\"%s\"\n", key, name, value);
	}
	printf("\n");
}

static int print_device_chain(struct udev_device *device)
{
	struct udev_device *device_parent;
	const char *str;

	printf("\n"
	       "Udevadm info starts with the device specified by the devpath and then\n"
	       "walks up the chain of parent devices. It prints for every device\n"
	       "found, all possible attributes in the udev rules key format.\n"
	       "A rule to match, can be composed by the attributes of the device\n"
	       "and the attributes from one single parent device.\n"
	       "\n");

	printf("  looking at device '%s':\n", udev_device_get_devpath(device));
	printf("    KERNEL==\"%s\"\n", udev_device_get_sysname(device));
	str = udev_device_get_subsystem(device);
	if (str == NULL)
		str = "";
	printf("    SUBSYSTEM==\"%s\"\n", str);
	str = udev_device_get_driver(device);
	if (str == NULL)
		str = "";
	printf("    DRIVER==\"%s\"\n", str);
	print_all_attributes(device, "ATTR");

	device_parent = device;
	do {
		device_parent = udev_device_get_parent(device_parent);
		if (device_parent == NULL)
			break;
		printf("  looking at parent device '%s':\n", udev_device_get_devpath(device_parent));
		printf("    KERNELS==\"%s\"\n", udev_device_get_sysname(device_parent));
		str = udev_device_get_subsystem(device_parent);
		if (str == NULL)
			str = "";
		printf("    SUBSYSTEMS==\"%s\"\n", str);
		str = udev_device_get_driver(device_parent);
		if (str == NULL)
			str = "";
		printf("    DRIVERS==\"%s\"\n", str);
		print_all_attributes(device_parent, "ATTRS");
	} while (device_parent != NULL);

	return 0;
}

static void print_record(struct udev_device *device)
{
	size_t len;
	const char *str;
	int i;
	struct udev_list_entry *list_entry;

	printf("P: %s\n", udev_device_get_devpath(device));

	len = strlen(udev_get_dev_path(udev_device_get_udev(device)));
	str = udev_device_get_devnode(device);
	if (str != NULL)
		printf("N: %s\n", &str[len+1]);

	i = udev_device_get_devlink_priority(device);
	if (i != 0)
		printf("L: %i\n", i);

	udev_list_entry_foreach(list_entry, udev_device_get_devlinks_list_entry(device)) {
		len = strlen(udev_get_dev_path(udev_device_get_udev(device)));
		printf("S: %s\n", &udev_list_entry_get_name(list_entry)[len+1]);
	}

	udev_list_entry_foreach(list_entry, udev_device_get_properties_list_entry(device))
		printf("E: %s=%s\n",
		       udev_list_entry_get_name(list_entry),
		       udev_list_entry_get_value(list_entry));
	printf("\n");
}

static int stat_device(const char *name, bool export, const char *prefix)
{
	struct stat statbuf;

	if (stat(name, &statbuf) != 0)
		return -1;

	if (export) {
		if (prefix == NULL)
			prefix = "INFO_";
		printf("%sMAJOR=%d\n"
		       "%sMINOR=%d\n",
		       prefix, major(statbuf.st_dev),
		       prefix, minor(statbuf.st_dev));
	} else
		printf("%d:%d\n", major(statbuf.st_dev), minor(statbuf.st_dev));
	return 0;
}

static int export_devices(struct udev *udev)
{
	struct udev_enumerate *udev_enumerate;
	struct udev_list_entry *list_entry;

	udev_enumerate = udev_enumerate_new(udev);
	if (udev_enumerate == NULL)
		return -1;
	udev_enumerate_scan_devices(udev_enumerate);
	udev_list_entry_foreach(list_entry, udev_enumerate_get_list_entry(udev_enumerate)) {
		struct udev_device *device;

		device = udev_device_new_from_syspath(udev, udev_list_entry_get_name(list_entry));
		if (device != NULL) {
			print_record(device);
			udev_device_unref(device);
		}
	}
	udev_enumerate_unref(udev_enumerate);
	return 0;
}

static void cleanup_dir(DIR *dir, mode_t mask, int depth)
{
	struct dirent *dent;

	if (depth <= 0)
		return;

	for (dent = readdir(dir); dent != NULL; dent = readdir(dir)) {
		struct stat stats;

		if (dent->d_name[0] == '.')
			continue;
		if (fstatat(dirfd(dir), dent->d_name, &stats, AT_SYMLINK_NOFOLLOW) != 0)
			continue;
		if ((stats.st_mode & mask) != 0)
			continue;
		if (S_ISDIR(stats.st_mode)) {
			DIR *dir2;

			dir2 = fdopendir(openat(dirfd(dir), dent->d_name, O_RDONLY|O_NONBLOCK|O_DIRECTORY|O_CLOEXEC));
			if (dir2 != NULL) {
				cleanup_dir(dir2, mask, depth-1);
				closedir(dir2);
			}
			unlinkat(dirfd(dir), dent->d_name, AT_REMOVEDIR);
		} else {
			unlinkat(dirfd(dir), dent->d_name, 0);
		}
	}
}

static void cleanup_db(struct udev *udev)
{
	char filename[UTIL_PATH_SIZE];
	DIR *dir;

	util_strscpyl(filename, sizeof(filename), udev_get_run_path(udev), "/queue.bin", NULL);
	unlink(filename);

	util_strscpyl(filename, sizeof(filename), udev_get_run_path(udev), "/data", NULL);
	dir = opendir(filename);
	if (dir != NULL) {
		cleanup_dir(dir, S_ISVTX, 1);
		closedir(dir);
	}

	util_strscpyl(filename, sizeof(filename), udev_get_run_path(udev), "/links", NULL);
	dir = opendir(filename);
	if (dir != NULL) {
		cleanup_dir(dir, 0, 2);
		closedir(dir);
	}

	util_strscpyl(filename, sizeof(filename), udev_get_run_path(udev), "/tags", NULL);
	dir = opendir(filename);
	if (dir != NULL) {
		cleanup_dir(dir, 0, 2);
		closedir(dir);
	}

	util_strscpyl(filename, sizeof(filename), udev_get_run_path(udev), "/watch", NULL);
	dir = opendir(filename);
	if (dir != NULL) {
		cleanup_dir(dir, 0, 1);
		closedir(dir);
	}

	util_strscpyl(filename, sizeof(filename), udev_get_run_path(udev), "/firmware-missing", NULL);
	dir = opendir(filename);
	if (dir != NULL) {
		cleanup_dir(dir, 0, 1);
		closedir(dir);
	}
}

static int uinfo(struct udev *udev, int argc, char *argv[])
{
	struct udev_device *device = NULL;
	bool root = 0;
	bool export = 0;
	const char *export_prefix = NULL;
	char path[UTIL_PATH_SIZE];
	char name[UTIL_PATH_SIZE];
	struct udev_list_entry *list_entry;
	int rc = 0;

	static const struct option options[] = {
		{ "name", required_argument, NULL, 'n' },
		{ "path", required_argument, NULL, 'p' },
		{ "query", required_argument, NULL, 'q' },
		{ "attribute-walk", no_argument, NULL, 'a' },
		{ "cleanup-db", no_argument, NULL, 'c' },
		{ "export-db", no_argument, NULL, 'e' },
		{ "root", no_argument, NULL, 'r' },
		{ "run", no_argument, NULL, 'R' },
		{ "device-id-of-file", required_argument, NULL, 'd' },
		{ "export", no_argument, NULL, 'x' },
		{ "export-prefix", required_argument, NULL, 'P' },
		{ "version", no_argument, NULL, 'V' },
		{ "help", no_argument, NULL, 'h' },
		{}
	};

	enum action_type {
		ACTION_NONE,
		ACTION_QUERY,
		ACTION_ATTRIBUTE_WALK,
		ACTION_ROOT,
		ACTION_DEVICE_ID_FILE,
	} action = ACTION_NONE;

	enum query_type {
		QUERY_NONE,
		QUERY_NAME,
		QUERY_PATH,
		QUERY_SYMLINK,
		QUERY_PROPERTY,
		QUERY_ALL,
	} query = QUERY_NONE;

	for (;;) {
		int option;
		struct stat statbuf;

		option = getopt_long(argc, argv, "aced:n:p:q:rxP:RVh", options, NULL);
		if (option == -1)
			break;

		dbg(udev, "option '%c'\n", option);
		switch (option) {
		case 'n':
			if (device != NULL) {
				fprintf(stderr, "device already specified\n");
				rc = 2;
				goto exit;
			}
			/* remove /dev if given */
			if (strncmp(optarg, udev_get_dev_path(udev), strlen(udev_get_dev_path(udev))) != 0)
				util_strscpyl(name, sizeof(name), udev_get_dev_path(udev), "/", optarg, NULL);
			else
				util_strscpy(name, sizeof(name), optarg);
			util_remove_trailing_chars(name, '/');
			if (stat(name, &statbuf) < 0) {
				fprintf(stderr, "device node not found\n");
				rc = 2;
				goto exit;
			} else {
				char type;

				if (S_ISBLK(statbuf.st_mode)) {
					type = 'b';
				} else if (S_ISCHR(statbuf.st_mode)) {
					type = 'c';
				} else {
					fprintf(stderr, "device node has wrong file type\n");
					rc = 2;
					goto exit;
				}
				device = udev_device_new_from_devnum(udev, type, statbuf.st_rdev);
				if (device == NULL) {
					fprintf(stderr, "device node not found\n");
					rc = 2;
					goto exit;
				}
			}
			break;
		case 'p':
			if (device != NULL) {
				fprintf(stderr, "device already specified\n");
				rc = 2;
				goto exit;
			}
			/* add sys dir if needed */
			if (strncmp(optarg, udev_get_sys_path(udev), strlen(udev_get_sys_path(udev))) != 0)
				util_strscpyl(path, sizeof(path), udev_get_sys_path(udev), optarg, NULL);
			else
				util_strscpy(path, sizeof(path), optarg);
			util_remove_trailing_chars(path, '/');
			device = udev_device_new_from_syspath(udev, path);
			if (device == NULL) {
				fprintf(stderr, "device path not found\n");
				rc = 2;
				goto exit;
			}
			break;
		case 'q':
			action = ACTION_QUERY;
			if (strcmp(optarg, "property") == 0 || strcmp(optarg, "env") == 0) {
				query = QUERY_PROPERTY;
			} else if (strcmp(optarg, "name") == 0) {
				query = QUERY_NAME;
			} else if (strcmp(optarg, "symlink") == 0) {
				query = QUERY_SYMLINK;
			} else if (strcmp(optarg, "path") == 0) {
				query = QUERY_PATH;
			} else if (strcmp(optarg, "all") == 0) {
				query = QUERY_ALL;
			} else {
				fprintf(stderr, "unknown query type\n");
				rc = 3;
				goto exit;
			}
			break;
		case 'r':
			if (action == ACTION_NONE)
				action = ACTION_ROOT;
			root = true;
			break;
		case 'R':
			printf("%s\n", udev_get_run_path(udev));
			goto exit;
		case 'd':
			action = ACTION_DEVICE_ID_FILE;
			util_strscpy(name, sizeof(name), optarg);
			break;
		case 'a':
			action = ACTION_ATTRIBUTE_WALK;
			break;
		case 'e':
			export_devices(udev);
			goto exit;
		case 'c':
			cleanup_db(udev);
			goto exit;
		case 'x':
			export = true;
			break;
		case 'P':
			export_prefix = optarg;
			break;
		case 'V':
			printf("%s\n", VERSION);
			goto exit;
		case 'h':
			printf("Usage: udevadm info OPTIONS\n"
			       "  --query=<type>             query device information:\n"
			       "      name                     name of device node\n"
			       "      symlink                  pointing to node\n"
			       "      path                     sys device path\n"
			       "      property                 the device properties\n"
			       "      all                      all values\n"
			       "  --path=<syspath>           sys device path used for query or attribute walk\n"
			       "  --name=<name>              node or symlink name used for query or attribute walk\n"
			       "  --root                     prepend dev directory to path names\n"
			       "  --attribute-walk           print all key matches while walking along the chain\n"
			       "                             of parent devices\n"
			       "  --device-id-of-file=<file> print major:minor of device containing this file\n"
			       "  --export                   export key/value pairs\n"
			       "  --export-prefix            export the key name with a prefix\n"
			       "  --export-db                export the content of the udev database\n"
			       "  --cleanup-db               cleanup the udev database\n"
			       "  --help\n\n");
			goto exit;
		default:
			rc = 1;
			goto exit;
		}
	}

	switch (action) {
	case ACTION_QUERY:
		if (device == NULL) {
			fprintf(stderr, "query needs a valid device specified by --path= or --name=\n");
			rc = 4;
			goto exit;
		}

		switch(query) {
		case QUERY_NAME: {
			const char *node = udev_device_get_devnode(device);

			if (node == NULL) {
				fprintf(stderr, "no device node found\n");
				rc = 5;
				goto exit;
			}

			if (root) {
				printf("%s\n", udev_device_get_devnode(device));
			} else {
				size_t len = strlen(udev_get_dev_path(udev));

				printf("%s\n", &udev_device_get_devnode(device)[len+1]);
			}
			break;
		}
		case QUERY_SYMLINK:
			list_entry = udev_device_get_devlinks_list_entry(device);
			while (list_entry != NULL) {
				if (root) {
					printf("%s", udev_list_entry_get_name(list_entry));
				} else {
					size_t len;

					len = strlen(udev_get_dev_path(udev_device_get_udev(device)));
					printf("%s", &udev_list_entry_get_name(list_entry)[len+1]);
				}
				list_entry = udev_list_entry_get_next(list_entry);
				if (list_entry != NULL)
					printf(" ");
			}
			printf("\n");
			break;
		case QUERY_PATH:
			printf("%s\n", udev_device_get_devpath(device));
			goto exit;
		case QUERY_PROPERTY:
			list_entry = udev_device_get_properties_list_entry(device);
			while (list_entry != NULL) {
				if (export) {
					const char *prefix = export_prefix;

					if (prefix == NULL)
						prefix = "";
					printf("%s%s='%s'\n", prefix,
					       udev_list_entry_get_name(list_entry),
					       udev_list_entry_get_value(list_entry));
				} else {
					printf("%s=%s\n", udev_list_entry_get_name(list_entry), udev_list_entry_get_value(list_entry));
				}
				list_entry = udev_list_entry_get_next(list_entry);
			}
			break;
		case QUERY_ALL:
			print_record(device);
			break;
		default:
			fprintf(stderr, "unknown query type\n");
			break;
		}
		break;
	case ACTION_ATTRIBUTE_WALK:
		if (device == NULL) {
			fprintf(stderr, "query needs a valid device specified by --path= or --name=\n");
			rc = 4;
			goto exit;
		}
		print_device_chain(device);
		break;
	case ACTION_DEVICE_ID_FILE:
		if (stat_device(name, export, export_prefix) != 0)
			rc = 1;
		break;
	case ACTION_ROOT:
		printf("%s\n", udev_get_dev_path(udev));
		break;
	default:
		fprintf(stderr, "missing option\n");
		rc = 1;
		break;
	}

exit:
	udev_device_unref(device);
	return rc;
}

const struct udevadm_cmd udevadm_info = {
	.name = "info",
	.cmd = uinfo,
	.help = "query sysfs or the udev database",
};
