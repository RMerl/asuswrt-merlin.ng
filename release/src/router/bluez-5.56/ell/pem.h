/*
 *
 *  Embedded Linux library
 *
 *  Copyright (C) 2015  Intel Corporation. All rights reserved.
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

#ifndef __ELL_PEM_H
#define __ELL_PEM_H

#ifdef __cplusplus
extern "C" {
#endif

struct l_queue;
struct l_key;
struct l_cert;
struct l_certchain;

uint8_t *l_pem_load_buffer(const void *buf, size_t buf_len, char **type_label,
				size_t *out_len);
uint8_t *l_pem_load_file(const char *filename, char **type_label, size_t *len);

struct l_certchain *l_pem_load_certificate_chain(const char *filename);
struct l_certchain *l_pem_load_certificate_chain_from_data(const void *buf,
								size_t len);
struct l_queue *l_pem_load_certificate_list(const char *filename);
struct l_queue *l_pem_load_certificate_list_from_data(const void *buf,
							size_t len);

struct l_key *l_pem_load_private_key(const char *filename,
					const char *passphrase,
					bool *encrypted);
struct l_key *l_pem_load_private_key_from_data(const void *buf, size_t len,
						const char *passphrase,
						bool *encrypted);

#ifdef __cplusplus
}
#endif

#endif /* __ELL_PEM_H */
