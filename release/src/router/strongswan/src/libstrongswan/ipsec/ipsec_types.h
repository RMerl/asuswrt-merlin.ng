/*
 * Copyright (C) 2012-2013 Tobias Brunner
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

/**
 * @defgroup ipsec_types ipsec_types
 * @{ @ingroup ipsec
 */

#ifndef IPSEC_TYPES_H_
#define IPSEC_TYPES_H_

typedef enum ipsec_mode_t ipsec_mode_t;
typedef enum policy_dir_t policy_dir_t;
typedef enum policy_type_t policy_type_t;
typedef enum policy_priority_t policy_priority_t;
typedef enum ipcomp_transform_t ipcomp_transform_t;
typedef enum hw_offload_t hw_offload_t;
typedef enum dscp_copy_t dscp_copy_t;
typedef enum mark_op_t mark_op_t;
typedef struct ipsec_sa_cfg_t ipsec_sa_cfg_t;
typedef struct lifetime_cfg_t lifetime_cfg_t;
typedef struct mark_t mark_t;

#include <library.h>

/**
 * Mode of an IPsec SA.
 */
enum ipsec_mode_t {
	/** not using any encapsulation */
	MODE_NONE = 0,
	/** transport mode, no inner address */
	MODE_TRANSPORT = 1,
	/** tunnel mode, inner and outer addresses */
	MODE_TUNNEL,
	/** BEET mode, tunnel mode but fixed, bound inner addresses */
	MODE_BEET,
	/** passthrough policy for traffic without an IPsec SA */
	MODE_PASS,
	/** drop policy discarding traffic */
	MODE_DROP
};

/**
 * enum names for ipsec_mode_t.
 */
extern enum_name_t *ipsec_mode_names;

/**
 * Direction of a policy. These are equal to those
 * defined in xfrm.h, but we want to stay implementation
 * neutral here.
 */
enum policy_dir_t {
	/** Policy for inbound traffic */
	POLICY_IN = 0,
	/** Policy for outbound traffic */
	POLICY_OUT = 1,
	/** Policy for forwarded traffic */
	POLICY_FWD = 2,
};

/**
 * enum names for policy_dir_t.
 */
extern enum_name_t *policy_dir_names;

/**
 * Type of a policy.
 */
enum policy_type_t {
	/** Normal IPsec policy */
	POLICY_IPSEC = 1,
	/** Passthrough policy (traffic is ignored by IPsec) */
	POLICY_PASS,
	/** Drop policy (traffic is discarded) */
	POLICY_DROP,
};

/**
 * High-level priority of a policy.
 */
enum policy_priority_t {
	/** Priority for passthrough policies */
	POLICY_PRIORITY_PASS,
	/** Priority for regular IPsec policies */
	POLICY_PRIORITY_DEFAULT,
	/** Priority for trap policies */
	POLICY_PRIORITY_ROUTED,
	/** Priority for fallback drop policies */
	POLICY_PRIORITY_FALLBACK,
};

/**
 * IPComp transform IDs, as in RFC 4306
 */
enum ipcomp_transform_t {
	IPCOMP_NONE = 0,
	IPCOMP_OUI = 1,
	IPCOMP_DEFLATE = 2,
	IPCOMP_LZS = 3,
	IPCOMP_LZJH = 4,
};

/**
 * enum strings for ipcomp_transform_t.
 */
extern enum_name_t *ipcomp_transform_names;

/**
 * HW offload mode options
 */
enum hw_offload_t {
	HW_OFFLOAD_NO = 0,
	HW_OFFLOAD_YES = 1,
	HW_OFFLOAD_AUTO = 2,
};

/**
 * enum names for hw_offload_t.
 */
extern enum_name_t *hw_offload_names;

/**
 * DSCP header field copy behavior (the default is not to copy from outer
 * to inner header)
 */
enum dscp_copy_t {
	DSCP_COPY_OUT_ONLY,
	DSCP_COPY_IN_ONLY,
	DSCP_COPY_YES,
	DSCP_COPY_NO,
};

/**
 * enum strings for dscp_copy_t.
 */
extern enum_name_t *dscp_copy_names;

/**
 * This struct contains details about IPsec SA(s) tied to a policy.
 */
struct ipsec_sa_cfg_t {
	/** mode of SA (tunnel, transport) */
	ipsec_mode_t mode;
	/** unique ID */
	uint32_t reqid;
	/** number of policies of the same kind (in/out/fwd) attached to SA */
	uint32_t policy_count;
	/** details about ESP/AH */
	struct {
		/** TRUE if this protocol is used */
		bool use;
		/** SPI for ESP/AH */
		uint32_t spi;
	} esp, ah;
	/** details about IPComp */
	struct {
		/** the IPComp transform used */
		uint16_t transform;
		/** CPI for IPComp */
		uint16_t cpi;
	} ipcomp;
};

/**
 * Compare two ipsec_sa_cfg_t objects for equality.
 *
 * @param a			first object
 * @param b			second object
 * @return			TRUE if both objects are equal
 */
bool ipsec_sa_cfg_equals(ipsec_sa_cfg_t *a, ipsec_sa_cfg_t *b);

/**
 * A lifetime_cfg_t defines the lifetime limits of an SA.
 *
 * Set any of these values to 0 to ignore.
 */
struct lifetime_cfg_t {
	struct {
		/** Limit before the SA gets invalid. */
		uint64_t	life;
		/** Limit before the SA gets rekeyed. */
		uint64_t	rekey;
		/** The range of a random value subtracted from rekey. */
		uint64_t	jitter;
	} time, bytes, packets;
};

/**
 * A mark_t defines an optional mark in an IPsec SA.
 */
struct mark_t {
	/** Mark value */
	uint32_t value;
	/** Mark mask */
	uint32_t mask;
};

/**
 * Special mark value that uses a unique mark for each CHILD_SA (and direction)
 */
#define MARK_UNIQUE (0xFFFFFFFF)
#define MARK_UNIQUE_DIR (0xFFFFFFFE)
#define MARK_SAME (0xFFFFFFFF)
#define MARK_IS_UNIQUE(m) ((m) == MARK_UNIQUE || (m) == MARK_UNIQUE_DIR)

/**
 * Special mark operations to accept when parsing marks.
 */
enum mark_op_t {
	/** none of the following */
	MARK_OP_NONE = 0,
	/** %unique and %unique-dir */
	MARK_OP_UNIQUE = (1<<0),
	/** %same */
	MARK_OP_SAME = (1<<1),
};

/**
 * Try to parse a mark_t from the given string of the form mark[/mask].
 *
 * @param value		string to parse
 * @param ops		operations to accept
 * @param mark		mark to fill
 * @return			TRUE if parsing was successful
 */
bool mark_from_string(const char *value, mark_op_t ops, mark_t *mark);

#endif /** IPSEC_TYPES_H_ @}*/
