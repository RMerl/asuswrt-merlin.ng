/*
 * Copyright (C) 2011-2014 Andreas Steffen
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

#include "pts_ima_event_list.h"

#include <utils/debug.h>
#include <crypto/hashers/hasher.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

typedef struct private_pts_ima_event_list_t private_pts_ima_event_list_t;
typedef struct event_entry_t event_entry_t;

#define IMA_TYPE_LEN				3
#define IMA_NG_TYPE_LEN				6
#define IMA_TYPE_LEN_MAX			10
#define IMA_ALGO_DIGEST_LEN_MAX		IMA_ALGO_LEN_MAX + HASH_SIZE_SHA512

/**
 * Private data of a pts_ima_event_list_t object.
 *
 */
struct private_pts_ima_event_list_t {

	/**
	 * Public pts_ima_event_list_t interface.
	 */
	pts_ima_event_list_t public;

	/**
	 * List of BIOS measurement entries
	 */
	linked_list_t *list;

	/**
	 * Time when IMA runtime file measurements were taken
	 */
	time_t creation_time;

};

/**
 * Linux IMA runtime file measurement entry
 */
struct event_entry_t {

	/**
	 * SHA1 measurement hash
	 */
	chunk_t measurement;

	/**
	 * IMA-NG hash algorithm name or NULL
	 */
	char *algo;

	/**
	 * IMA-NG eventname or IMA filename
	 */
	char *name;
};

/**
 * Free an ima_event_t object
 */
static void free_event_entry(event_entry_t *this)
{
	free(this->measurement.ptr);
	free(this->algo);
	free(this->name);
	free(this);
}

METHOD(pts_ima_event_list_t, get_time, time_t,
	private_pts_ima_event_list_t *this)
{
	return this->creation_time;
}

METHOD(pts_ima_event_list_t, get_count, int,
	private_pts_ima_event_list_t *this)
{
	return this->list->get_count(this->list);
}

METHOD(pts_ima_event_list_t, get_next, status_t,
	private_pts_ima_event_list_t *this, chunk_t *measurement, char **algo,
	char **name)
{
	event_entry_t *entry;
	status_t status;

	status = this->list->remove_first(this->list, (void**)&entry);
	*measurement = entry->measurement;
	*algo = entry->algo;
	*name = entry->name;
	free(entry);

	return status;
}

METHOD(pts_ima_event_list_t, destroy, void,
	private_pts_ima_event_list_t *this)
{
	this->list->destroy_function(this->list, (void *)free_event_entry);
	free(this);
}

/**
 * See header
 */
pts_ima_event_list_t* pts_ima_event_list_create(char *file)
{
	private_pts_ima_event_list_t *this;
	event_entry_t *entry;
	uint32_t pcr, type_len, name_len, eventdata_len, algo_digest_len, algo_len;
	char type[IMA_TYPE_LEN_MAX];
	char algo_digest[IMA_ALGO_DIGEST_LEN_MAX];
	char *pos, *error = "";
	struct stat st;
	ssize_t res;
	bool ima_ng;
	int fd;

	fd = open(file, O_RDONLY);
	if (fd == -1)
	{
		DBG1(DBG_PTS, "opening '%s' failed: %s", file, strerror(errno));
		return NULL;
	}

	if (fstat(fd, &st) == -1)
	{
		DBG1(DBG_PTS, "getting statistics of '%s' failed: %s", file,
			 strerror(errno));
		close(fd);
		return NULL;
	}

	INIT(this,
		.public = {
			.get_time = _get_time,
			.get_count = _get_count,
			.get_next = _get_next,
			.destroy = _destroy,
		},
		.creation_time = st.st_ctime,
		.list = linked_list_create(),
	);

	while (TRUE)
	{
		/* read 32 bit PCR number in host order */
		res = read(fd, &pcr, 4);

		/* exit if no more measurement data is available */
		if (res == 0)
		{
			DBG2(DBG_PTS, "loaded ima measurements '%s' (%d entries)",
				 file, this->list->get_count(this->list));
			close(fd);
			return &this->public;
		}

		/* create and initialize new IMA entry */
		entry = malloc_thing(event_entry_t);
		entry->measurement = chunk_alloc(HASH_SIZE_SHA1);
		entry->algo = NULL;
		entry->name = NULL;

		if (res != 4 || pcr != IMA_PCR)
		{
			error = "invalid IMA PCR field";
			break;
		}

		/* read 20 byte SHA-1 measurement digest */
		if (read(fd, entry->measurement.ptr, HASH_SIZE_SHA1) != HASH_SIZE_SHA1)
		{
			error = "invalid SHA-1 digest field";
			break;
		}

		/* read 32 bit length of IMA type string in host order */
		if (read(fd, &type_len, 4) != 4 || type_len > IMA_TYPE_LEN_MAX)
		{
			error = "invalid IMA type field length";
			break;
		}

		/* read and interpret IMA type string */
		if (read(fd, type, type_len) != type_len)
		{
			error = "invalid IMA type field";
			break;
		}
		if (type_len == IMA_NG_TYPE_LEN &&
			memeq(type, "ima-ng", IMA_NG_TYPE_LEN))
		{
			ima_ng = TRUE;
		}
		else if (type_len == IMA_TYPE_LEN &&
				 memeq(type, "ima", IMA_TYPE_LEN))
		{
			ima_ng = FALSE;
		}
		else
		{
			error = "unknown IMA type";
			break;
		}

		if (ima_ng)
		{
			/* read the 32 bit length of the event data in host order */
			if (read(fd, &eventdata_len, 4) != 4 || eventdata_len < 4)
			{
				error = "invalid event data field length";
				break;
			}

			/* read the 32 bit length of the algo_digest string in host order */
			if (read(fd, &algo_digest_len, 4) != 4 ||
				algo_digest_len > IMA_ALGO_DIGEST_LEN_MAX ||
				eventdata_len < 4 + algo_digest_len + 4)
			{
				error = "invalid digest_with_algo field length";
				break;
			}

			/* read the IMA algo_digest string */
			if (read(fd, algo_digest, algo_digest_len) != algo_digest_len)
			{
				error = "invalid digest_with_algo field";
				break;
			}

			/* extract the hash algorithm name */
			pos = memchr(algo_digest, '\0', algo_digest_len);
			if (!pos)
			{
				error = "no algo field";
				break;
			}
			algo_len = pos - algo_digest + 1;

			if (algo_len > IMA_ALGO_LEN_MAX ||
				algo_len < IMA_ALGO_LEN_MIN || *(pos - 1) != ':')
			{
				error = "invalid algo field";
				break;
			}

			/* copy and store the hash algorithm name */
			entry->algo = malloc(algo_len);
			memcpy(entry->algo, algo_digest, algo_len);

			/* read the 32 bit length of the event name in host order */
			if (read(fd, &name_len, 4) != 4 ||
				eventdata_len != 4 + algo_digest_len + 4 + name_len)
			{
				error = "invalid filename field length";
				break;
			}

			/* allocate memory for the file name */
			entry->name = malloc(name_len);

			/* read file name */
			if (read(fd, entry->name, name_len) != name_len)
			{
				error = "invalid filename field";
				break;
			}
		}
		else
		{
			/* skip SHA-1 digest of the file content */
			if (lseek(fd, HASH_SIZE_SHA1, SEEK_CUR) == -1)
			{
				break;
			}

			/* read the 32 bit length of the file name in host order */
			if (read(fd, &name_len, 4) != 4 || name_len == UINT32_MAX)
			{
				error = "invalid filename field length";
				break;
			}

			/* allocate memory for the file name */
			entry->name = malloc(name_len + 1);

			/* read file name */
			if (read(fd, entry->name, name_len) != name_len)
			{
				error = "invalid eventname field";
				break;
			}

			/* terminate the file name with a nul character */
			entry->name[name_len] = '\0';
		}

		this->list->insert_last(this->list, entry);
	}

	DBG1(DBG_PTS, "loading ima measurements '%s' failed: %s", file, error);
	free_event_entry(entry);
	close(fd);
	destroy(this);

	return NULL;
}
