/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2013 The Chromium OS Authors.
 * Coypright (c) 2013 Guntermann & Drunck GmbH
 */

#ifndef __TPM_API_H
#define __TPM_API_H

#include <tpm-common.h>
#include <tpm-v1.h>
#include <tpm-v2.h>

/**
 * Issue a TPM_Startup command.
 *
 * @param dev		TPM device
 * @param mode		TPM startup mode
 * Return: return code of the operation
 */
u32 tpm_startup(struct udevice *dev, enum tpm_startup_type mode);

/**
 * Issue a TPM_SelfTestFull command.
 *
 * @param dev		TPM device
 * Return: return code of the operation
 */
u32 tpm_self_test_full(struct udevice *dev);

/**
 * Issue a TPM_ContinueSelfTest command.
 *
 * @param dev		TPM device
 * Return: return code of the operation
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
 * Return: return code of the operation
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
 * Return: return code of the operation
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
 * Return: return code of the operation
 */
u32 tpm_nv_write_value(struct udevice *dev, u32 index, const void *data,
		       u32 length);

/**
 * Issue a TPM_Extend command.
 *
 * @param dev		TPM device
 * @param index		index of the PCR
 * @param in_digest	160/256-bit value representing the event to be
 *			recorded
 * @param size		size of digest in bytes
 * @param out_digest	160/256-bit PCR value after execution of the
 *			command
 * @param name		digest source, used for log output
 * Return: return code of the operation
 */
u32 tpm_pcr_extend(struct udevice *dev, u32 index, const void *in_digest,
		   uint size, void *out_digest, const char *name);

/**
 * Issue a TPM_PCRRead command.
 *
 * @param dev		TPM device
 * @param index		index of the PCR
 * @param data		output buffer for contents of the named PCR
 * @param count		size of output buffer
 * Return: return code of the operation
 */
u32 tpm_pcr_read(struct udevice *dev, u32 index, void *data, size_t count);

/**
 * Issue a TSC_PhysicalPresence command.  TPM physical presence flag
 * is bit-wise OR'ed of flags listed in enum tpm_physical_presence.
 *
 * @param dev		TPM device
 * @param presence	TPM physical presence flag
 * Return: return code of the operation
 */
u32 tpm_tsc_physical_presence(struct udevice *dev, u16 presence);

/**
 * Issue a TPM_ReadPubek command.
 *
 * @param dev		TPM device
 * @param data		output buffer for the public endorsement key
 * @param count		size of output buffer
 * Return: return code of the operation
 */
u32 tpm_read_pubek(struct udevice *dev, void *data, size_t count);

/**
 * Issue a TPM_ForceClear command.
 *
 * @param dev		TPM device
 * Return: return code of the operation
 */
u32 tpm_force_clear(struct udevice *dev);

/**
 * Issue a TPM_PhysicalEnable command.
 *
 * @param dev		TPM device
 * Return: return code of the operation
 */
u32 tpm_physical_enable(struct udevice *dev);

/**
 * Issue a TPM_PhysicalDisable command.
 *
 * @param dev		TPM device
 * Return: return code of the operation
 */
u32 tpm_physical_disable(struct udevice *dev);

/**
 * Issue a TPM_PhysicalSetDeactivated command.
 *
 * @param dev		TPM device
 * @param state		boolean state of the deactivated flag
 * Return: return code of the operation
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
 * Return: return code of the operation
 */
u32 tpm_get_capability(struct udevice *dev, u32 cap_area, u32 sub_cap,
		       void *cap, size_t count);

/**
 * Issue a TPM_FlushSpecific command for a AUTH resource.
 *
 * @param dev		TPM device
 * @param auth_handle	handle of the auth session
 * Return: return code of the operation
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
 * Return: return code of the operation
 */
u32 tpm_oiap(struct udevice *dev, u32 *auth_handle);

/**
 * Ends an active OIAP session.
 *
 * @param dev		TPM device
 * Return: return code of the operation
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
 * Return: return code of the operation
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
 * Return: return code of the operation
 */
u32 tpm_get_pub_key_oiap(struct udevice *dev, u32 key_handle,
			 const void *usage_auth, void *pubkey,
			 size_t *pubkey_len);

/**
 * Get the TPM permissions
 *
 * @param dev		TPM device
 * @param perm		Returns permissions value
 * Return: return code of the operation
 */
u32 tpm_get_permissions(struct udevice *dev, u32 index, u32 *perm);

/**
 * Flush a resource with a given handle and type from the TPM
 *
 * @param dev		TPM device
 * @param key_handle           handle of the resource
 * @param resource_type                type of the resource
 * Return: return code of the operation
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
 * Return: 0 if key was found in TPM; != 0 if not.
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
 * Return: return code of the operation
 */
u32 tpm_get_random(struct udevice *dev, void *data, u32 count);

/**
 * tpm_finalise_physical_presence() - Finalise physical presence
 *
 * @param dev		TPM device
 * Return: return code of the operation (0 = success)
 */
u32 tpm_finalise_physical_presence(struct udevice *dev);

/**
 * tpm_nv_enable_locking() - lock the non-volatile space
 *
 * @param dev		TPM device
 * Return: return code of the operation (0 = success)
 */
u32 tpm_nv_enable_locking(struct udevice *dev);

/**
 * tpm_set_global_lock() - set the global lock
 *
 * @param dev		TPM device
 * Return: return code of the operation (0 = success)
 */
u32 tpm_set_global_lock(struct udevice *dev);

/**
 * tpm_write_lock() - lock the non-volatile space
 *
 * @param dev		TPM device
 * @param index		Index of space to lock
 * Return: return code of the operation (0 = success)
 */
u32 tpm_write_lock(struct udevice *dev, u32 index);

/**
 * tpm_resume() - start up the TPM from resume (after suspend)
 *
 * @param dev		TPM device
 * Return: return code of the operation (0 = success)
 */
u32 tpm_resume(struct udevice *dev);

static inline bool tpm_is_v1(struct udevice *dev)
{
	return IS_ENABLED(CONFIG_TPM_V1) && tpm_get_version(dev) == TPM_V1;
}

static inline bool tpm_is_v2(struct udevice *dev)
{
	return IS_ENABLED(CONFIG_TPM_V2) && tpm_get_version(dev) == TPM_V2;
}

/**
 * tpm_auto_start() - start up the TPM and perform selftests
 *
 * @param dev		TPM device
 * Return: return code of the operation (0 = success)
 */
u32 tpm_auto_start(struct udevice *dev);

#endif /* __TPM_API_H */
