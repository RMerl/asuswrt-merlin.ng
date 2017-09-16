/*
 * Copyright (C) 2011  Red Hat, Jeff Layton <jlayton@redhat.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _SQLITE_H_
#define _SQLITE_H_

int sqlite_prepare_dbh(const char *topdir);
int sqlite_insert_client(const unsigned char *clname, const size_t namelen,
				const bool has_session, const bool zerotime);
int sqlite_remove_client(const unsigned char *clname, const size_t namelen);
int sqlite_check_client(const unsigned char *clname, const size_t namelen,
				const bool has_session);
int sqlite_remove_unreclaimed(const time_t grace_start);
int sqlite_query_reclaiming(const time_t grace_start);

#endif /* _SQLITE_H */
