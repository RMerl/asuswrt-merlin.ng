/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2013 The Chromium OS Authors.
 * Coypright (c) 2013 Guntermann & Drunck GmbH
 */

#ifndef __TPM_V1_H
#define __TPM_V1_H

#include <tpm-common.h>

/* Useful constants */
enum {
	TPM_REQUEST_HEADER_LENGTH	= 10,
	TPM_RESPONSE_HEADER_LENGTH	= 10,
	PCR_DIGEST_LENGTH		= 20,
	DIGEST_LENGTH			= 20,
	TPM_REQUEST_AUTH_LENGTH		= 45,
	TPM_RESPONSE_AUTH_LENGTH	= 41,
	/* some max lengths, valid for RSA keys <= 2048 bits */
	TPM_KEY12_MAX_LENGTH		= 618,
	TPM_PUBKEY_MAX_LENGTH		= 288,
};

enum tpm_startup_type {
	TPM_ST_CLEAR		= 0x0001,
	TPM_ST_STATE		= 0x0002,
	TPM_ST_DEACTIVATED	= 0x0003,
};

enum tpm_physical_presence {
	TPM_PHYSICAL_PRESENCE_HW_DISABLE	= 0x0200,
	TPM_PHYSICAL_PRESENCE_CMD_DISABLE	= 0x0100,
	TPM_PHYSICAL_PRESENCE_LIFETIME_LOCK	= 0x0080,
	TPM_PHYSICAL_PRESENCE_HW_ENABLE		= 0x0040,
	TPM_PHYSICAL_PRESENCE_CMD_ENABLE	= 0x0020,
	TPM_PHYSICAL_PRESENCE_NOTPRESENT	= 0x0010,
	TPM_PHYSICAL_PRESENCE_PRESENT		= 0x0008,
	TPM_PHYSICAL_PRESENCE_LOCK		= 0x0004,
};

enum tpm_nv_index {
	TPM_NV_INDEX_LOCK	= 0xffffffff,
	TPM_NV_INDEX_0		= 0x00000000,
	TPM_NV_INDEX_DIR	= 0x10000001,
};

enum tpm_resource_type {
	TPM_RT_KEY	= 0x00000001,
	TPM_RT_AUTH	= 0x00000002,
	TPM_RT_HASH	= 0x00000003,
	TPM_RT_TRANS	= 0x00000004,
	TPM_RT_CONTEXT	= 0x00000005,
	TPM_RT_COUNTER	= 0x00000006,
	TPM_RT_DELEGATE	= 0x00000007,
	TPM_RT_DAA_TPM	= 0x00000008,
	TPM_RT_DAA_V0	= 0x00000009,
	TPM_RT_DAA_V1	= 0x0000000A,
};

enum tpm_capability_areas {
	TPM_CAP_ORD		= 0x00000001,
	TPM_CAP_ALG		= 0x00000002,
	TPM_CAP_PID		= 0x00000003,
	TPM_CAP_FLAG		= 0x00000004,
	TPM_CAP_PROPERTY	= 0x00000005,
	TPM_CAP_VERSION		= 0x00000006,
	TPM_CAP_KEY_HANDLE	= 0x00000007,
	TPM_CAP_CHECK_LOADED	= 0x00000008,
	TPM_CAP_SYM_MODE	= 0x00000009,
	TPM_CAP_KEY_STATUS	= 0x0000000C,
	TPM_CAP_NV_LIST		= 0x0000000D,
	TPM_CAP_MFR		= 0x00000010,
	TPM_CAP_NV_INDEX	= 0x00000011,
	TPM_CAP_TRANS_ALG	= 0x00000012,
	TPM_CAP_HANDLE		= 0x00000014,
	TPM_CAP_TRANS_ES	= 0x00000015,
	TPM_CAP_AUTH_ENCRYPT	= 0x00000017,
	TPM_CAP_SELECT_SIZE	= 0x00000018,
	TPM_CAP_DA_LOGIC	= 0x00000019,
	TPM_CAP_VERSION_VAL	= 0x0000001A,
};

enum tmp_cap_flag {
	TPM_CAP_FLAG_PERMANENT	= 0x108,
};

#define TPM_TAG_PERMANENT_FLAGS		0x001f

#define TPM_NV_PER_GLOBALLOCK		BIT(15)
#define TPM_NV_PER_PPREAD		BIT(16)
#define TPM_NV_PER_PPWRITE		BIT(0)
#define TPM_NV_PER_READ_STCLEAR		BIT(31)
#define TPM_NV_PER_WRITE_STCLEAR	BIT(14)
#define TPM_NV_PER_WRITEDEFINE		BIT(13)
#define TPM_NV_PER_WRITEALL		BIT(12)

enum {
	TPM_PUBEK_SIZE			= 256,
};

enum {
	TPM_CMD_EXTEND			= 0x14,
	TPM_CMD_GET_CAPABILITY		= 0x65,
	TPM_CMD_NV_DEFINE_SPACE		= 0xcc,
	TPM_CMD_NV_WRITE_VALUE		= 0xcd,
	TPM_CMD_NV_READ_VALUE		= 0xcf,
};

/**
 * TPM return codes as defined in the TCG Main specification
 * (TPM Main Part 2 Structures; Specification version 1.2)
 */
enum tpm_return_code {
	TPM_BASE	= 0x00000000,
	TPM_NON_FATAL	= 0x00000800,
	TPM_SUCCESS	= TPM_BASE,
	/* TPM-defined fatal error codes */
	TPM_AUTHFAIL			= TPM_BASE +  1,
	TPM_BADINDEX			= TPM_BASE +  2,
	TPM_BAD_PARAMETER		= TPM_BASE +  3,
	TPM_AUDITFAILURE		= TPM_BASE +  4,
	TPM_CLEAR_DISABLED		= TPM_BASE +  5,
	TPM_DEACTIVATED			= TPM_BASE +  6,
	TPM_DISABLED			= TPM_BASE +  7,
	TPM_DISABLED_CMD		= TPM_BASE +  8,
	TPM_FAIL			= TPM_BASE +  9,
	TPM_BAD_ORDINAL			= TPM_BASE + 10,
	TPM_INSTALL_DISABLED		= TPM_BASE + 11,
	TPM_INVALID_KEYHANDLE		= TPM_BASE + 12,
	TPM_KEYNOTFOUND			= TPM_BASE + 13,
	TPM_INAPPROPRIATE_ENC		= TPM_BASE + 14,
	TPM_MIGRATE_FAIL		= TPM_BASE + 15,
	TPM_INVALID_PCR_INFO		= TPM_BASE + 16,
	TPM_NOSPACE			= TPM_BASE + 17,
	TPM_NOSRK			= TPM_BASE + 18,
	TPM_NOTSEALED_BLOB		= TPM_BASE + 19,
	TPM_OWNER_SET			= TPM_BASE + 20,
	TPM_RESOURCES			= TPM_BASE + 21,
	TPM_SHORTRANDOM			= TPM_BASE + 22,
	TPM_SIZE			= TPM_BASE + 23,
	TPM_WRONGPCRVAL			= TPM_BASE + 24,
	TPM_BAD_PARAM_SIZE		= TPM_BASE + 25,
	TPM_SHA_THREAD			= TPM_BASE + 26,
	TPM_SHA_ERROR			= TPM_BASE + 27,
	TPM_FAILEDSELFTEST		= TPM_BASE + 28,
	TPM_AUTH2FAIL			= TPM_BASE + 29,
	TPM_BADTAG			= TPM_BASE + 30,
	TPM_IOERROR			= TPM_BASE + 31,
	TPM_ENCRYPT_ERROR		= TPM_BASE + 32,
	TPM_DECRYPT_ERROR		= TPM_BASE + 33,
	TPM_INVALID_AUTHHANDLE		= TPM_BASE + 34,
	TPM_NO_ENDORSEMENT		= TPM_BASE + 35,
	TPM_INVALID_KEYUSAGE		= TPM_BASE + 36,
	TPM_WRONG_ENTITYTYPE		= TPM_BASE + 37,
	TPM_INVALID_POSTINIT		= TPM_BASE + 38,
	TPM_INAPPROPRIATE_SIG		= TPM_BASE + 39,
	TPM_BAD_KEY_PROPERTY		= TPM_BASE + 40,
	TPM_BAD_MIGRATION		= TPM_BASE + 41,
	TPM_BAD_SCHEME			= TPM_BASE + 42,
	TPM_BAD_DATASIZE		= TPM_BASE + 43,
	TPM_BAD_MODE			= TPM_BASE + 44,
	TPM_BAD_PRESENCE		= TPM_BASE + 45,
	TPM_BAD_VERSION			= TPM_BASE + 46,
	TPM_NO_WRAP_TRANSPORT		= TPM_BASE + 47,
	TPM_AUDITFAIL_UNSUCCESSFUL	= TPM_BASE + 48,
	TPM_AUDITFAIL_SUCCESSFUL	= TPM_BASE + 49,
	TPM_NOTRESETABLE		= TPM_BASE + 50,
	TPM_NOTLOCAL			= TPM_BASE + 51,
	TPM_BAD_TYPE			= TPM_BASE + 52,
	TPM_INVALID_RESOURCE		= TPM_BASE + 53,
	TPM_NOTFIPS			= TPM_BASE + 54,
	TPM_INVALID_FAMILY		= TPM_BASE + 55,
	TPM_NO_NV_PERMISSION		= TPM_BASE + 56,
	TPM_REQUIRES_SIGN		= TPM_BASE + 57,
	TPM_KEY_NOTSUPPORTED		= TPM_BASE + 58,
	TPM_AUTH_CONFLICT		= TPM_BASE + 59,
	TPM_AREA_LOCKED			= TPM_BASE + 60,
	TPM_BAD_LOCALITY		= TPM_BASE + 61,
	TPM_READ_ONLY			= TPM_BASE + 62,
	TPM_PER_NOWRITE			= TPM_BASE + 63,
	TPM_FAMILY_COUNT		= TPM_BASE + 64,
	TPM_WRITE_LOCKED		= TPM_BASE + 65,
	TPM_BAD_ATTRIBUTES		= TPM_BASE + 66,
	TPM_INVALID_STRUCTURE		= TPM_BASE + 67,
	TPM_KEY_OWNER_CONTROL		= TPM_BASE + 68,
	TPM_BAD_COUNTER			= TPM_BASE + 69,
	TPM_NOT_FULLWRITE		= TPM_BASE + 70,
	TPM_CONTEXT_GAP			= TPM_BASE + 71,
	TPM_MAXNVWRITES			= TPM_BASE + 72,
	TPM_NOOPERATOR			= TPM_BASE + 73,
	TPM_RESOURCEMISSING		= TPM_BASE + 74,
	TPM_DELEGATE_LOCK		= TPM_BASE + 75,
	TPM_DELEGATE_FAMILY		= TPM_BASE + 76,
	TPM_DELEGATE_ADMIN		= TPM_BASE + 77,
	TPM_TRANSPORT_NOTEXCLUSIVE	= TPM_BASE + 78,
	TPM_OWNER_CONTROL		= TPM_BASE + 79,
	TPM_DAA_RESOURCES		= TPM_BASE + 80,
	TPM_DAA_INPUT_DATA0		= TPM_BASE + 81,
	TPM_DAA_INPUT_DATA1		= TPM_BASE + 82,
	TPM_DAA_ISSUER_SETTINGS		= TPM_BASE + 83,
	TPM_DAA_TPM_SETTINGS		= TPM_BASE + 84,
	TPM_DAA_STAGE			= TPM_BASE + 85,
	TPM_DAA_ISSUER_VALIDITY		= TPM_BASE + 86,
	TPM_DAA_WRONG_W			= TPM_BASE + 87,
	TPM_BAD_HANDLE			= TPM_BASE + 88,
	TPM_BAD_DELEGATE		= TPM_BASE + 89,
	TPM_BADCONTEXT			= TPM_BASE + 90,
	TPM_TOOMANYCONTEXTS		= TPM_BASE + 91,
	TPM_MA_TICKET_SIGNATURE		= TPM_BASE + 92,
	TPM_MA_DESTINATION		= TPM_BASE + 93,
	TPM_MA_SOURCE			= TPM_BASE + 94,
	TPM_MA_AUTHORITY		= TPM_BASE + 95,
	TPM_PERMANENTEK			= TPM_BASE + 97,
	TPM_BAD_SIGNATURE		= TPM_BASE + 98,
	TPM_NOCONTEXTSPACE		= TPM_BASE + 99,
	/* TPM-defined non-fatal errors */
	TPM_RETRY		= TPM_BASE + TPM_NON_FATAL,
	TPM_NEEDS_SELFTEST	= TPM_BASE + TPM_NON_FATAL + 1,
	TPM_DOING_SELFTEST	= TPM_BASE + TPM_NON_FATAL + 2,
	TPM_DEFEND_LOCK_RUNNING	= TPM_BASE + TPM_NON_FATAL + 3,
};

struct tpm_permanent_flags {
	__be16	tag;
	u8	disable;
	u8	ownership;
	u8	deactivated;
	u8	read_pubek;
	u8	disable_owner_clear;
	u8	allow_maintenance;
	u8	physical_presence_lifetime_lock;
	u8	physical_presence_hw_enable;
	u8	physical_presence_cmd_enable;
	u8	cekp_used;
	u8	tpm_post;
	u8	tpm_post_lock;
	u8	fips;
	u8	operator;
	u8	enable_revoke_ek;
	u8	nv_locked;
	u8	read_srk_pub;
	u8	tpm_established;
	u8	maintenance_done;
	u8	disable_full_da_logic_info;
} __packed;

#define TPM_SHA1_160_HASH_LEN	0x14

struct __packed tpm_composite_hash {
	u8	digest[TPM_SHA1_160_HASH_LEN];
};

struct __packed tpm_pcr_selection {
	__be16	size_of_select;
	u8	pcr_select[3];	/* matches vboot's struct */
};

struct __packed tpm_pcr_info_short {
	struct tpm_pcr_selection pcr_selection;
	u8	locality_at_release;
	struct tpm_composite_hash digest_at_release;
};

struct __packed tpm_nv_attributes {
	__be16	tag;
	__be32	attributes;
};

struct __packed tpm_nv_data_public {
	__be16	tag;
	__be32	nv_index;
	struct tpm_pcr_info_short pcr_info_read;
	struct tpm_pcr_info_short pcr_info_write;
	struct tpm_nv_attributes permission;
	u8	read_st_clear;
	u8	write_st_clear;
	u8	write_define;
	__be32	data_size;
};

/**
 * Issue a TPM_Startup command.
 *
 * @param dev		TPM device
 * @param mode		TPM startup mode
 * @return return code of the operation
 */
u32 tpm_startup(struct udevice *dev, enum tpm_startup_type mode);

/**
 * Issue a TPM_SelfTestFull command.
 *
 * @param dev		TPM device
 * @return return code of the operation
 */
u32 tpm_self_test_full(struct udevice *dev);

/**
 * Issue a TPM_ContinueSelfTest command.
 *
 * @param dev		TPM device
 * @return return code of the operation
 */
u32 tpm_continue_self_test(struct udevice *dev);

/**
 * Issue a TPM_NV_DefineSpace command.  The implementation is limited
 * to specify TPM_NV_ATTRIBUTES and size of the area.  The area index
 * could be one of the special value listed in enum tpm_nv_index.
 *
 * @param dev		TPM device
 * @param index		index of the area
 * @param perm		TPM_NV_ATTRIBUTES of the area
 * @param size		size of the area
 * @return return code of the operation
 */
u32 tpm_nv_define_space(struct udevice *dev, u32 index, u32 perm, u32 size);

/**
 * Issue a TPM_NV_ReadValue command.  This implementation is limited
 * to read the area from offset 0.  The area index could be one of
 * the special value listed in enum tpm_nv_index.
 *
 * @param dev		TPM device
 * @param index		index of the area
 * @param data		output buffer of the area contents
 * @param count		size of output buffer
 * @return return code of the operation
 */
u32 tpm_nv_read_value(struct udevice *dev, u32 index, void *data, u32 count);

/**
 * Issue a TPM_NV_WriteValue command.  This implementation is limited
 * to write the area from offset 0.  The area index could be one of
 * the special value listed in enum tpm_nv_index.
 *
 * @param dev		TPM device
 * @param index		index of the area
 * @param data		input buffer to be wrote to the area
 * @param length	length of data bytes of input buffer
 * @return return code of the operation
 */
u32 tpm_nv_write_value(struct udevice *dev, u32 index, const void *data,
		       u32 length);

/**
 * Issue a TPM_Extend command.
 *
 * @param dev		TPM device
 * @param index		index of the PCR
 * @param in_digest	160-bit value representing the event to be
 *			recorded
 * @param out_digest	160-bit PCR value after execution of the
 *			command
 * @return return code of the operation
 */
u32 tpm_extend(struct udevice *dev, u32 index, const void *in_digest,
	       void *out_digest);

/**
 * Issue a TPM_PCRRead command.
 *
 * @param dev		TPM device
 * @param index		index of the PCR
 * @param data		output buffer for contents of the named PCR
 * @param count		size of output buffer
 * @return return code of the operation
 */
u32 tpm_pcr_read(struct udevice *dev, u32 index, void *data, size_t count);

/**
 * Issue a TSC_PhysicalPresence command.  TPM physical presence flag
 * is bit-wise OR'ed of flags listed in enum tpm_physical_presence.
 *
 * @param dev		TPM device
 * @param presence	TPM physical presence flag
 * @return return code of the operation
 */
u32 tpm_tsc_physical_presence(struct udevice *dev, u16 presence);

/**
 * Issue a TPM_ReadPubek command.
 *
 * @param dev		TPM device
 * @param data		output buffer for the public endorsement key
 * @param count		size of output buffer
 * @return return code of the operation
 */
u32 tpm_read_pubek(struct udevice *dev, void *data, size_t count);

/**
 * Issue a TPM_ForceClear command.
 *
 * @param dev		TPM device
 * @return return code of the operation
 */
u32 tpm_force_clear(struct udevice *dev);

/**
 * Issue a TPM_PhysicalEnable command.
 *
 * @param dev		TPM device
 * @return return code of the operation
 */
u32 tpm_physical_enable(struct udevice *dev);

/**
 * Issue a TPM_PhysicalDisable command.
 *
 * @param dev		TPM device
 * @return return code of the operation
 */
u32 tpm_physical_disable(struct udevice *dev);

/**
 * Issue a TPM_PhysicalSetDeactivated command.
 *
 * @param dev		TPM device
 * @param state		boolean state of the deactivated flag
 * @return return code of the operation
 */
u32 tpm_physical_set_deactivated(struct udevice *dev, u8 state);

/**
 * Issue a TPM_GetCapability command.  This implementation is limited
 * to query sub_cap index that is 4-byte wide.
 *
 * @param dev		TPM device
 * @param cap_area	partition of capabilities
 * @param sub_cap	further definition of capability, which is
 *			limited to be 4-byte wide
 * @param cap		output buffer for capability information
 * @param count		size of output buffer
 * @return return code of the operation
 */
u32 tpm_get_capability(struct udevice *dev, u32 cap_area, u32 sub_cap,
		       void *cap, size_t count);

/**
 * Issue a TPM_FlushSpecific command for a AUTH resource.
 *
 * @param dev		TPM device
 * @param auth_handle	handle of the auth session
 * @return return code of the operation
 */
u32 tpm_terminate_auth_session(struct udevice *dev, u32 auth_handle);

/**
 * Issue a TPM_OIAP command to setup an object independent authorization
 * session.
 * Information about the session is stored internally.
 * If there was already an OIAP session active it is terminated and a new
 * session is set up.
 *
 * @param dev		TPM device
 * @param auth_handle	pointer to the (new) auth handle or NULL.
 * @return return code of the operation
 */
u32 tpm_oiap(struct udevice *dev, u32 *auth_handle);

/**
 * Ends an active OIAP session.
 *
 * @param dev		TPM device
 * @return return code of the operation
 */
u32 tpm_end_oiap(struct udevice *dev);

/**
 * Issue a TPM_LoadKey2 (Auth1) command using an OIAP session for authenticating
 * the usage of the parent key.
 *
 * @param dev		TPM device
 * @param parent_handle	handle of the parent key.
 * @param key		pointer to the key structure (TPM_KEY or TPM_KEY12).
 * @param key_length	size of the key structure
 * @param parent_key_usage_auth	usage auth for the parent key
 * @param key_handle	pointer to the key handle
 * @return return code of the operation
 */
u32 tpm_load_key2_oiap(struct udevice *dev, u32 parent_handle, const void *key,
		       size_t key_length, const void *parent_key_usage_auth,
		       u32 *key_handle);

/**
 * Issue a TPM_GetPubKey (Auth1) command using an OIAP session for
 * authenticating the usage of the key.
 *
 * @param dev		TPM device
 * @param key_handle	handle of the key
 * @param usage_auth	usage auth for the key
 * @param pubkey	pointer to the pub key buffer; may be NULL if the pubkey
 *			should not be stored.
 * @param pubkey_len	pointer to the pub key buffer len. On entry: the size of
 *			the provided pubkey buffer. On successful exit: the size
 *			of the stored TPM_PUBKEY structure (iff pubkey != NULL).
 * @return return code of the operation
 */
u32 tpm_get_pub_key_oiap(struct udevice *dev, u32 key_handle,
			 const void *usage_auth, void *pubkey,
			 size_t *pubkey_len);

/**
 * Get the TPM permanent flags value
 *
 * @param dev		TPM device
 * @param pflags	Place to put permanent flags
 * @return return code of the operation
 */
u32 tpm_get_permanent_flags(struct udevice *dev,
			    struct tpm_permanent_flags *pflags);

/**
 * Get the TPM permissions
 *
 * @param dev		TPM device
 * @param perm		Returns permissions value
 * @return return code of the operation
 */
u32 tpm_get_permissions(struct udevice *dev, u32 index, u32 *perm);

/**
 * Flush a resource with a given handle and type from the TPM
 *
 * @param dev		TPM device
 * @param key_handle           handle of the resource
 * @param resource_type                type of the resource
 * @return return code of the operation
 */
u32 tpm_flush_specific(struct udevice *dev, u32 key_handle, u32 resource_type);

#ifdef CONFIG_TPM_LOAD_KEY_BY_SHA1
/**
 * Search for a key by usage AuthData and the hash of the parent's pub key.
 *
 * @param dev		TPM device
 * @param auth	        Usage auth of the key to search for
 * @param pubkey_digest	SHA1 hash of the pub key structure of the key
 * @param[out] handle	The handle of the key (Non-null iff found)
 * @return 0 if key was found in TPM; != 0 if not.
 */
u32 tpm_find_key_sha1(struct udevice *dev, const u8 auth[20],
		      const u8 pubkey_digest[20], u32 *handle);
#endif /* CONFIG_TPM_LOAD_KEY_BY_SHA1 */

/**
 * Read random bytes from the TPM RNG. The implementation deals with the fact
 * that the TPM may legally return fewer bytes than requested by retrying
 * until @p count bytes have been received.
 *
 * @param dev		TPM device
 * @param data		output buffer for the random bytes
 * @param count		size of output buffer
 * @return return code of the operation
 */
u32 tpm_get_random(struct udevice *dev, void *data, u32 count);

/**
 * tpm_finalise_physical_presence() - Finalise physical presence
 *
 * @param dev		TPM device
 * @return return code of the operation (0 = success)
 */
u32 tpm_finalise_physical_presence(struct udevice *dev);

/**
 * tpm_nv_set_locked() - lock the non-volatile space
 *
 * @param dev		TPM device
 * @return return code of the operation (0 = success)
 */
u32 tpm_nv_set_locked(struct udevice *dev);

/**
 * tpm_set_global_lock() - set the global lock
 *
 * @param dev		TPM device
 * @return return code of the operation (0 = success)
 */
u32 tpm_set_global_lock(struct udevice *dev);

/**
 * tpm_resume() - start up the TPM from resume (after suspend)
 *
 * @param dev		TPM device
 * @return return code of the operation (0 = success)
 */
u32 tpm_resume(struct udevice *dev);

#endif /* __TPM_V1_H */
