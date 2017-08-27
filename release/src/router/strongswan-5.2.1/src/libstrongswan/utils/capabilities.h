/*
 * Copyright (C) 2013 Tobias Brunner
 * Hochschule fuer Technik Rapperswil
 * Copyright (C) 2012 Martin Willi
 * Copyright (C) 2012 revosec AG
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

/**
 * @defgroup capabilities capabilities
 * @{ @ingroup utils
 */

#ifndef CAPABILITIES_H_
#define CAPABILITIES_H_

typedef struct capabilities_t capabilities_t;

#include <library.h>
#ifdef HAVE_SYS_CAPABILITY_H
# include <sys/capability.h>
#elif defined(CAPABILITIES_NATIVE)
# include <linux/capability.h>
#endif

#ifndef CAP_CHOWN
# define CAP_CHOWN 0
#endif
#ifndef CAP_NET_BIND_SERVICE
# define CAP_NET_BIND_SERVICE 10
#endif
#ifndef CAP_NET_ADMIN
# define CAP_NET_ADMIN 12
#endif
#ifndef CAP_NET_RAW
# define CAP_NET_RAW 13
#endif
#ifndef CAP_DAC_OVERRIDE
# define CAP_DAC_OVERRIDE 1
#endif

/**
 * POSIX capability dropping abstraction layer.
 */
struct capabilities_t {

	/**
	 * Register a capability to keep while calling drop(). Verifies that the
	 * capability is currently held.
	 *
	 * @note CAP_CHOWN is handled specially as it might not be required.
	 *
	 * @param cap		capability to keep
	 * @return			FALSE if the capability is currently not held
	 */
	bool (*keep)(capabilities_t *this,
				 u_int cap) __attribute__((warn_unused_result));

	/**
	 * Check if the given capability is currently held.
	 *
	 * @note CAP_CHOWN is handled specially as it might not be required.
	 *
	 * @param cap		capability to check
	 * @return			TRUE if the capability is currently held
	 */
	bool (*check)(capabilities_t *this, u_int cap);

	/**
	 * Get the user ID set through set_uid/resolve_uid.
	 *
	 * @return			currently set user ID
	 */
	uid_t (*get_uid)(capabilities_t *this);

	/**
	 * Get the group ID set through set_gid/resolve_gid.
	 *
	 * @return			currently set group ID
	 */
	gid_t (*get_gid)(capabilities_t *this);

	/**
	 * Set the numerical user ID to use during rights dropping.
	 *
	 * @param uid		user ID to use
	 */
	void (*set_uid)(capabilities_t *this, uid_t uid);

	/**
	 * Set the numerical group ID to use during rights dropping.
	 *
	 * @param gid		group ID to use
	 */
	void (*set_gid)(capabilities_t *this, gid_t gid);

	/**
	 * Resolve a username and set the user ID accordingly.
	 *
	 * @param username	username get the uid for
	 * @return			TRUE if username resolved and uid set
	 */
	bool (*resolve_uid)(capabilities_t *this, char *username);

	/**
	 * Resolve a groupname and set the group ID accordingly.
	 *
	 * @param groupname	groupname to get the gid for
	 * @return			TRUE if groupname resolved and gid set
	 */
	bool (*resolve_gid)(capabilities_t *this, char *groupname);

	/**
	 * Drop all capabilities not previously passed to keep(), switch to UID/GID.
	 *
	 * @return			TRUE if capability drop successful
	 */
	bool (*drop)(capabilities_t *this);

	/**
	 * Destroy a capabilities_t.
	 */
	void (*destroy)(capabilities_t *this);
};

/**
 * Create a capabilities instance.
 */
capabilities_t *capabilities_create();

#endif /** CAPABILITIES_H_ @}*/
