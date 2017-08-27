/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */
#ifndef FR_CONNECTION_H
#define FR_CONNECTION_H
/*
 * $Id$
 *
 * @file connection.h
 * @brief Structures, prototypes and global variables for server connection pools.
 *
 * @copyright 2012  The FreeRADIUS server project
 * @copyright 2012  Alan DeKok <aland@deployingradius.com>
 */

RCSIDH(connection_h, "$Id$")

#include <freeradius-devel/conffile.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct fr_connection_pool_t fr_connection_pool_t;

/** Create a new connection handle
 *
 * This function will be called whenever the connection pool manager needs
 * to spawn a new connection, and on reconnect.
 *
 * @note A function pointer matching this prototype must be passed
 * to fr_connection_pool.
 * @param[in] ctx pointer passed to fr_connection_pool_init.
 * @return NULL on error, else a connection handle.
 */
typedef void *(*fr_connection_create_t)(void *ctx);

/** Check a connection handle is still viable
 *
 * Should check the state  of a connection handle.
 *
 * @note NULL may be passed to fr_connection_init, if there is no way to check
 * the state of a connection handle.
 * @note Not currently use by connection pool manager.
 * @param[in] ctx pointer passed to fr_connection_pool_init.
 * @param[in] connection handle returned by fr_connection_create_t.
 * @return < 0 on error or if the connection is unusable, else 0.
 */
typedef int (*fr_connection_alive_t)(void *ctx, void *connection);

/** Delete a connection and free allocated memory
 *
 * Should close any sockets associated with the passed connection handle,
 * and free any memory allocated to it.
 *
 * @param[in] ctx pointer passed to fr_connection_pool_init.
 * @param[in,out] connection handle returned by fr_connection_create_t.
 * @return < 0 on error else 0 if connection was closed successfully.
 */
typedef int (*fr_connection_delete_t)(void *ctx, void *connection);

fr_connection_pool_t *fr_connection_pool_init(CONF_SECTION *cs,
					      void *ctx,
					      fr_connection_create_t c,
					      fr_connection_alive_t a,
					      fr_connection_delete_t d,
					      char *prefix);
void fr_connection_pool_delete(fr_connection_pool_t *pool);

int fr_connection_check(fr_connection_pool_t *pool, void *conn);
void *fr_connection_get(fr_connection_pool_t *pool);
void fr_connection_release(fr_connection_pool_t *pool, void *conn);
void *fr_connection_reconnect(fr_connection_pool_t *pool, void *conn);
int fr_connection_add(fr_connection_pool_t *pool, void *conn);
int fr_connection_del(fr_connection_pool_t *pool, void *conn);

#ifdef __cplusplus
}
#endif

#endif /* FR_CONNECTION_H*/
