/*
 * Copyright (C) 2005-2006 Kay Sievers <kay.sievers@vrfy.org>
 *	              2008 Jaroslav Kysela <perex@perex.cz>
 *
 *	This program is free software; you can redistribute it and/or modify it
 *	under the terms of the GNU General Public License as published by the
 *	Free Software Foundation version 2 of the License.
 * 
 *	This program is distributed in the hope that it will be useful, but
 *	WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *	General Public License for more details.
 * 
 *	You should have received a copy of the GNU General Public License along
 *	with this program; if not, write to the Free Software Foundation, Inc.,
 *	51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */


static char sysfs_path[PATH_SIZE];

/* attribute value cache */
static LIST_HEAD(attr_list);
struct sysfs_attr {
	struct list_head node;
	char path[PATH_SIZE];
	char *value;			/* points to value_local if value is cached */
	char value_local[NAME_SIZE];
};

static int sysfs_init(void)
{
	const char *env;
	char sysfs_test[PATH_SIZE];

	env = getenv("SYSFS_PATH");
	if (env) {
		strlcpy(sysfs_path, env, sizeof(sysfs_path));
		remove_trailing_chars(sysfs_path, '/');
	} else
		strlcpy(sysfs_path, "/sys", sizeof(sysfs_path));
	dbg("sysfs_path='%s'", sysfs_path);

	strlcpy(sysfs_test, sysfs_path, sizeof(sysfs_test));
	strlcat(sysfs_test, "/kernel/uevent_helper", sizeof(sysfs_test));
	if (access(sysfs_test, F_OK)) {
		error("sysfs path '%s' is invalid\n", sysfs_path);
		return -errno;
	}

	INIT_LIST_HEAD(&attr_list);
	return 0;
}

static void sysfs_cleanup(void)
{
	struct sysfs_attr *attr_loop;
	struct sysfs_attr *attr_temp;

	list_for_each_entry_safe(attr_loop, attr_temp, &attr_list, node) {
		list_del(&attr_loop->node);
		free(attr_loop);
	}
}

static char *sysfs_attr_get_value(const char *devpath, const char *attr_name)
{
	char path_full[PATH_SIZE];
	const char *path;
	char value[NAME_SIZE];
	struct sysfs_attr *attr_loop;
	struct sysfs_attr *attr;
	struct stat statbuf;
	int fd;
	ssize_t size;
	size_t sysfs_len;

	dbg("open '%s'/'%s'", devpath, attr_name);
	sysfs_len = strlcpy(path_full, sysfs_path, sizeof(path_full));
	path = &path_full[sysfs_len];
	strlcat(path_full, devpath, sizeof(path_full));
	strlcat(path_full, "/", sizeof(path_full));
	strlcat(path_full, attr_name, sizeof(path_full));

	/* look for attribute in cache */
	list_for_each_entry(attr_loop, &attr_list, node) {
		if (strcmp(attr_loop->path, path) == 0) {
			dbg("found in cache '%s'", attr_loop->path);
			return attr_loop->value;
		}
	}

	/* store attribute in cache (also negatives are kept in cache) */
	dbg("new uncached attribute '%s'", path_full);
	attr = malloc(sizeof(struct sysfs_attr));
	if (attr == NULL)
		return NULL;
	memset(attr, 0x00, sizeof(struct sysfs_attr));
	strlcpy(attr->path, path, sizeof(attr->path));
	dbg("add to cache '%s'", path_full);
	list_add(&attr->node, &attr_list);

	if (lstat(path_full, &statbuf) != 0) {
		dbg("stat '%s' failed: %s", path_full, strerror(errno));
		goto out;
	}

	if (S_ISLNK(statbuf.st_mode)) {
		/* links return the last element of the target path */
		char link_target[PATH_SIZE];
		int len;
		const char *pos;

		len = readlink(path_full, link_target, sizeof(link_target));
		if (len > 0) {
			link_target[len] = '\0';
			pos = strrchr(link_target, '/');
			if (pos != NULL) {
				dbg("cache '%s' with link value '%s'", path_full, pos+1);
				strlcpy(attr->value_local, &pos[1], sizeof(attr->value_local));
				attr->value = attr->value_local;
			}
		}
		goto out;
	}

	/* skip directories */
	if (S_ISDIR(statbuf.st_mode))
		goto out;

	/* skip non-readable files */
	if ((statbuf.st_mode & S_IRUSR) == 0)
		goto out;

	/* read attribute value */
	fd = open(path_full, O_RDONLY);
	if (fd < 0) {
		dbg("attribute '%s' does not exist", path_full);
		goto out;
	}
	size = read(fd, value, sizeof(value));
	close(fd);
	if (size < 0)
		goto out;
	if (size == sizeof(value))
		goto out;

	/* got a valid value, store and return it */
	value[size] = '\0';
	remove_trailing_chars(value, '\n');
	dbg("cache '%s' with attribute value '%s'", path_full, value);
	strlcpy(attr->value_local, value, sizeof(attr->value_local));
	attr->value = attr->value_local;

out:
	return attr->value;
}
