/*
 * Copyright (C) 2021 Tobias Brunner
 *
 * Copyright (C) secunet Security Networks AG
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
 * @defgroup sec_label sec_label
 * @{ @ingroup ipsec
 */

#ifndef SEC_LABEL_H_
#define SEC_LABEL_H_

typedef enum sec_label_mode_t sec_label_mode_t;
typedef struct sec_label_t sec_label_t;

#include <library.h>

/**
 * Mode in which security labels are used.
 */
enum sec_label_mode_t {

	/**
	 * System default.  Simple mode if SELinux is not supported or disabled
	 * on the system.
	 */
	SEC_LABEL_MODE_SYSTEM,

	/**
	 * Simple mode that does establish regular CHILD_SAs, matches labels exactly
	 * and does not install them in the kernel.
	 */
	SEC_LABEL_MODE_SIMPLE,

	/**
	 * SELinux mode where configured labels are installed on (trap) policies,
	 * labels from acquires/peer on SAs, child-less IKE_SAs are initiated
	 * if there is no acquire, labels are also matched via polmatch.
	 */
	SEC_LABEL_MODE_SELINUX,
};

/**
 * Names for security label modes.
 */
extern enum_name_t *sec_label_mode_names;

/**
 * Representation of a security label used on policies/SAs.
 *
 * For example, with SELinux this could be a value like
 * system_u:object_r:ipsec_spd_t:s0.
 */
struct sec_label_t {

	/**
	 * Return a binary encoding of the security label as used for IKE.
	 *
	 * @return			binary encoding (internal data)
	 */
	chunk_t (*get_encoding)(sec_label_t *this);

	/**
	 * Return a string representation of this security label.
	 *
	 * @return			string representation (internal data)
	 */
	char *(*get_string)(sec_label_t *this);

	/**
	 * Clone this security label.
	 *
	 * @return			clone of it
	 */
	sec_label_t *(*clone)(sec_label_t *this);

	/**
	 * Match two security labels.
	 *
	 * For SELinux this checks if this security label permits other in terms
	 * of association { polmatch }.
	 *
	 * @param other		security label to match against this
	 * @return			TRUE if matching, FALSE otherwise
	 */
	bool (*matches)(sec_label_t *this, sec_label_t *other);

	/**
	 * Compare two security labels for equality.
	 *
	 * @param other		security label to compare with this
	 * @return			TRUE if equal, FALSE otherwise
	 */
	bool (*equals)(sec_label_t *this, sec_label_t *other);

	/**
	 * Create a hash value for the security label.
	 *
	 * @param inc		optional value for incremental hashing
	 * @return			calculated hash value for the security label
	 */
	u_int (*hash)(sec_label_t *this, u_int inc);

	/**
	 * Destroys the object.
	 */
	void (*destroy)(sec_label_t *this);
};

/**
 * Try to parse a sec_label_t from the given binary encoding.
 *
 * @param value			encoding to parse
 * @return				security label instance, NULL if invalid
 */
sec_label_t *sec_label_from_encoding(const chunk_t value);

/**
 * Try to parse a sec_label_t from the given string.
 *
 * @param value			string to parse
 * @return				security label instance, NULL if invalid
 */
sec_label_t *sec_label_from_string(const char *value);

/**
 * Compare two security labels for equality, accept if both are NULL.
 *
 * @param a				first label
 * @param b				second label
 * @return				TRUE if labels are equal or both NULL
 */
static inline bool sec_labels_equal(sec_label_t *a, sec_label_t *b)
{
	return (!a && !b) || (a && a->equals(a, b));
}

/**
 * Try to parse a security label mode from the given string.
 *
 * @param value			string to parse
 * @param mode			parsed mode
 * @return				TRUE if mode is valid (and usable on system)
 */
bool sec_label_mode_from_string(const char *value, sec_label_mode_t *mode);

/**
 * Get the system default security label mode.
 *
 * @return				default mode
 */
sec_label_mode_t sec_label_mode_default();

#endif /** SEC_LABEL_H_ @}*/
