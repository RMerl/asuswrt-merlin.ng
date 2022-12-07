/*
 *
 *  Embedded Linux library
 *
 *  Copyright (C) 2019  Intel Corporation. All rights reserved.
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
#include <sys/types.h>
#include <sys/stat.h>

struct l_certchain;

struct pem_file_info {
	int fd;
	struct stat st;
	uint8_t *data;
};

int pem_file_open(struct pem_file_info *info, const char *filename);
void pem_file_close(struct pem_file_info *info);

const char *pem_next(const void *buf, size_t buf_len, char **type_label,
				size_t *base64_len,
				const char **endp, bool strict);

uint8_t *pem_load_buffer(const void *buf, size_t buf_len,
				char **out_type_label, size_t *out_len,
				char **out_headers, const char **out_endp);

struct l_key *pem_load_private_key(uint8_t *content, size_t len, char *label,
					const char *passphrase, char *headers,
					bool *encrypted);

int pem_write_certificate_chain(const struct l_certchain *cert,
				const char *filename);
