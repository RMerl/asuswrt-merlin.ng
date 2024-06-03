/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2018 Linaro Limited
 */

#ifndef __TEE_H
#define __TEE_H

#define TEE_UUID_LEN		16

#define TEE_GEN_CAP_GP          BIT(0)	/* GlobalPlatform compliant TEE */
#define TEE_GEN_CAP_REG_MEM     BIT(1)	/* Supports registering shared memory */

#define TEE_SHM_REGISTER	BIT(0)	/* In list of shared memory */
#define TEE_SHM_SEC_REGISTER	BIT(1)	/* TEE notified of this memory */
#define TEE_SHM_ALLOC		BIT(2)	/* The memory is malloced() and must */
					/* be freed() */

#define TEE_PARAM_ATTR_TYPE_NONE		0	/* parameter not used */
#define TEE_PARAM_ATTR_TYPE_VALUE_INPUT		1
#define TEE_PARAM_ATTR_TYPE_VALUE_OUTPUT	2
#define TEE_PARAM_ATTR_TYPE_VALUE_INOUT		3	/* input and output */
#define TEE_PARAM_ATTR_TYPE_MEMREF_INPUT	5
#define TEE_PARAM_ATTR_TYPE_MEMREF_OUTPUT	6
#define TEE_PARAM_ATTR_TYPE_MEMREF_INOUT	7	/* input and output */
#define TEE_PARAM_ATTR_TYPE_MASK		0xff
#define TEE_PARAM_ATTR_META			0x100
#define TEE_PARAM_ATTR_MASK			(TEE_PARAM_ATTR_TYPE_MASK | \
						 TEE_PARAM_ATTR_META)

/*
 * Some Global Platform error codes which has a meaning if the
 * TEE_GEN_CAP_GP bit is returned by the driver in
 * struct tee_version_data::gen_caps
 */
#define TEE_SUCCESS			0x00000000
#define TEE_ERROR_STORAGE_NOT_AVAILABLE	0xf0100003
#define TEE_ERROR_GENERIC		0xffff0000
#define TEE_ERROR_BAD_PARAMETERS	0xffff0006
#define TEE_ERROR_ITEM_NOT_FOUND	0xffff0008
#define TEE_ERROR_NOT_IMPLEMENTED	0xffff0009
#define TEE_ERROR_NOT_SUPPORTED		0xffff000a
#define TEE_ERROR_COMMUNICATION		0xffff000e
#define TEE_ERROR_SECURITY		0xffff000f
#define TEE_ERROR_OUT_OF_MEMORY		0xffff000c
#define TEE_ERROR_OVERFLOW              0xffff300f
#define TEE_ERROR_TARGET_DEAD		0xffff3024
#define TEE_ERROR_STORAGE_NO_SPACE      0xffff3041

#define TEE_ORIGIN_COMMS		0x00000002
#define TEE_ORIGIN_TEE			0x00000003
#define TEE_ORIGIN_TRUSTED_APP		0x00000004

struct udevice;

/**
 * struct tee_optee_ta_uuid - OP-TEE Trusted Application (TA) UUID format
 *
 * Used to identify an OP-TEE TA and define suitable to initialize structs
 * of this format is distributed with the interface of the TA. The
 * individual fields of this struct doesn't have any special meaning in
 * OP-TEE. See RFC4122 for details on the format.
 */
struct tee_optee_ta_uuid {
	u32 time_low;
	u16 time_mid;
	u16 time_hi_and_version;
	u8 clock_seq_and_node[8];
};

/**
 * struct tee_shm - memory shared with the TEE
 * @dev:	The TEE device
 * @link:	List node in the list in struct struct tee_uclass_priv
 * @addr:	Pointer to the shared memory
 * @size:	Size of the the shared memory
 * @flags:	TEE_SHM_* above
 */
struct tee_shm {
	struct udevice *dev;
	struct list_head link;
	void *addr;
	ulong size;
	u32 flags;
};

/**
 * struct tee_param_memref - memory reference for a Trusted Application
 * @shm_offs:	Offset in bytes into the shared memory object @shm
 * @size:	Size in bytes of the memory reference
 * @shm:	Pointer to a shared memory object for the buffer
 *
 * Used as a part of struct tee_param, see that for more information.
 */
struct tee_param_memref {
	ulong shm_offs;
	ulong size;
	struct tee_shm *shm;
};

/**
 * struct tee_param_value - value parameter for a Trusted Application
 * @a, @b, @c:	Parameters passed by value
 *
 * Used as a part of struct tee_param, see that for more information.
 */
struct tee_param_value {
	u64 a;
	u64 b;
	u64 c;
};

/**
 * struct tee_param - invoke parameter for a Trusted Application
 * @attr:	Attributes
 * @u.memref:	Memref parameter if (@attr & TEE_PARAM_ATTR_MASK) is one of
 *		TEE_PARAM_ATTR_TYPE_MEMREF_* above
 * @u.value:	Value parameter if (@attr & TEE_PARAM_ATTR_MASK) is one of
 *		TEE_PARAM_ATTR_TYPE_VALUE_* above
 *
 * Parameters to TA are passed using an array of this struct, for
 * flexibility both value parameters and memory refereces can be used.
 */
struct tee_param {
	u64 attr;
	union {
		struct tee_param_memref memref;
		struct tee_param_value value;
	} u;
};

/**
 * struct tee_open_session_arg - extra arguments for tee_open_session()
 * @uuid:	[in] UUID of the Trusted Application
 * @clnt_uuid:	[in] Normally zeroes
 * @clnt_login:	[in] Normally 0
 * @session:	[out] Session id
 * @ret:	[out] return value
 * @ret_origin:	[out] origin of the return value
 */
struct tee_open_session_arg {
	u8 uuid[TEE_UUID_LEN];
	u8 clnt_uuid[TEE_UUID_LEN];
	u32 clnt_login;
	u32 session;
	u32 ret;
	u32 ret_origin;
};

/**
 * struct tee_invoke_arg - extra arguments for tee_invoke_func()
 * @func:	[in] Trusted Application function, specific to the TA
 * @session:	[in] Session id, from open session
 * @ret:	[out] return value
 * @ret_origin:	[out] origin of the return value
 */
struct tee_invoke_arg {
	u32 func;
	u32 session;
	u32 ret;
	u32 ret_origin;
};

/**
 * struct tee_version_data - description of TEE
 * @gen_caps:	Generic capabilities, TEE_GEN_CAP_* above
 */
struct tee_version_data {
	u32 gen_caps;
};

/**
 * struct tee_driver_ops - TEE driver operations
 * @get_version:	Query capabilities of TEE device,
 * @open_session:	Opens a session to a Trusted Application in the TEE,
 * @close_session:	Closes a session to Trusted Application,
 * @invoke_func:	Invokes a function in a Trusted Application,
 * @shm_register:	Registers memory shared with the TEE
 * @shm_unregister:	Unregisters memory shared with the TEE
 */
struct tee_driver_ops {
	/**
	 * get_version() - Query capabilities of TEE device
	 * @dev:	The TEE device
	 * @vers:	Pointer to version data
	 */
	void (*get_version)(struct udevice *dev, struct tee_version_data *vers);
	/**
	 * open_session() - Open a session to a Trusted Application
	 * @dev:	The TEE device
	 * @arg:	Open session arguments
	 * @num_param:	Number of elements in @param
	 * @param:	Parameters for Trusted Application
	 *
	 * Returns < 0 on error else see @arg->ret for result. If @arg->ret is
	 * TEE_SUCCESS the session identifier is available in @arg->session.
	 */
	int (*open_session)(struct udevice *dev,
			    struct tee_open_session_arg *arg, uint num_param,
			    struct tee_param *param);
	/**
	 * close_session() - Close a session to a Trusted Application
	 * @dev:	The TEE device
	 * @session:	Session id
	 *
	 * Return < 0 on error else 0, regardless the session will not be valid
	 * after this function has returned.
	 */
	int (*close_session)(struct udevice *dev, u32 session);
	/**
	 * tee_invoke_func() - Invoke a function in a Trusted Application
	 * @dev:	The TEE device
	 * @arg:	Invoke arguments
	 * @num_param:	Number of elements in @param
	 * @param:	Parameters for Trusted Application
	 *
	 * Returns < 0 on error else see @arg->ret for result.
	 */
	int (*invoke_func)(struct udevice *dev, struct tee_invoke_arg *arg,
			   uint num_param, struct tee_param *param);
	/**
	 * shm_register() - Registers memory shared with the TEE
	 * @dev:	The TEE device
	 * @shm:	Pointer to a shared memory object
	 * Returns 0 on success or < 0 on failure.
	 */
	int (*shm_register)(struct udevice *dev, struct tee_shm *shm);
	/**
	 * shm_unregister() - Unregisters memory shared with the TEE
	 * @dev:	The TEE device
	 * @shm:	Pointer to a shared memory object
	 * Returns 0 on success or < 0 on failure.
	 */
	int (*shm_unregister)(struct udevice *dev, struct tee_shm *shm);
};

/**
 * __tee_shm_add() - Internal helper function to register shared memory
 * @dev:	The TEE device
 * @align:	Required alignment of allocated memory block if
 *		(@flags & TEE_SHM_ALLOC)
 * @addr:	Address of memory block, ignored if (@flags & TEE_SHM_ALLOC)
 * @size:	Size of memory block
 * @flags:	TEE_SHM_* above
 * @shmp:	If the function return 0, this holds the allocated
 *		struct tee_shm
 *
 * returns 0 on success or < 0 on failure.
 */
int __tee_shm_add(struct udevice *dev, ulong align, void *addr, ulong size,
		  u32 flags, struct tee_shm **shmp);

/**
 * tee_shm_alloc() - Allocate shared memory
 * @dev:	The TEE device
 * @size:	Size of memory block
 * @flags:	TEE_SHM_* above
 * @shmp:	If the function return 0, this holds the allocated
 *		struct tee_shm
 *
 * returns 0 on success or < 0 on failure.
 */
int tee_shm_alloc(struct udevice *dev, ulong size, u32 flags,
		  struct tee_shm **shmp);

/**
 * tee_shm_register() - Registers shared memory
 * @dev:	The TEE device
 * @addr:	Address of memory block
 * @size:	Size of memory block
 * @flags:	TEE_SHM_* above
 * @shmp:	If the function return 0, this holds the allocated
 *		struct tee_shm
 *
 * returns 0 on success or < 0 on failure.
 */
int tee_shm_register(struct udevice *dev, void *addr, ulong size, u32 flags,
		     struct tee_shm **shmp);

/**
 * tee_shm_free() - Frees shared memory
 * @shm:	Shared memory object
 */
void tee_shm_free(struct tee_shm *shm);

/**
 * tee_shm_is_registered() - Check register status of shared memory object
 * @shm:	Pointer to shared memory object
 * @dev:	The TEE device
 *
 * Returns true if the shared memory object is registered for the supplied
 * TEE device
 */
bool tee_shm_is_registered(struct tee_shm *shm, struct udevice *dev);

/**
 * tee_find_device() - Look up a TEE device
 * @start:	if not NULL, continue search after this device
 * @match:	function to check TEE device, returns != 0 if the device
 *		matches
 * @data:	data for match function
 * @vers:	if not NULL, version data of TEE device of the device returned
 *
 * Returns a probed TEE device of the first TEE device matched by the
 * match() callback or NULL.
 */
struct udevice *tee_find_device(struct udevice *start,
				int (*match)(struct tee_version_data *vers,
					     const void *data),
				const void *data,
				struct tee_version_data *vers);

/**
 * tee_get_version() - Query capabilities of TEE device
 * @dev:	The TEE device
 * @vers:	Pointer to version data
 */
void tee_get_version(struct udevice *dev, struct tee_version_data *vers);

/**
 * tee_open_session() - Open a session to a Trusted Application
 * @dev:	The TEE device
 * @arg:	Open session arguments
 * @num_param:	Number of elements in @param
 * @param:	Parameters for Trusted Application
 *
 * Returns < 0 on error else see @arg->ret for result. If @arg->ret is
 * TEE_SUCCESS the session identifier is available in @arg->session.
 */
int tee_open_session(struct udevice *dev, struct tee_open_session_arg *arg,
		     uint num_param, struct tee_param *param);

/**
 * tee_close_session() - Close a session to a Trusted Application
 * @dev:	The TEE device
 * @session:	Session id
 *
 * Return < 0 on error else 0, regardless the session will not be valid
 * after this function has returned.
 */
int tee_close_session(struct udevice *dev, u32 session);

/**
 * tee_invoke_func() - Invoke a function in a Trusted Application
 * @dev:	The TEE device
 * @arg:	Invoke arguments
 * @num_param:	Number of elements in @param
 * @param:	Parameters for Trusted Application
 *
 * Returns < 0 on error else see @arg->ret for result.
 */
int tee_invoke_func(struct udevice *dev, struct tee_invoke_arg *arg,
		    uint num_param, struct tee_param *param);

/**
 * tee_optee_ta_uuid_from_octets() - Converts to struct tee_optee_ta_uuid
 * @d:	Destination struct
 * @s:	Source UUID octets
 *
 * Conversion to a struct tee_optee_ta_uuid represantion from binary octet
 * representation.
 */
void tee_optee_ta_uuid_from_octets(struct tee_optee_ta_uuid *d,
				   const u8 s[TEE_UUID_LEN]);

/**
 * tee_optee_ta_uuid_to_octets() - Converts from struct tee_optee_ta_uuid
 * @d:	Destination UUID octets
 * @s:	Source struct
 *
 * Conversion from a struct tee_optee_ta_uuid represantion to binary octet
 * representation.
 */
void tee_optee_ta_uuid_to_octets(u8 d[TEE_UUID_LEN],
				 const struct tee_optee_ta_uuid *s);

#endif /* __TEE_H */
