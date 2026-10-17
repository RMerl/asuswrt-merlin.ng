/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
 */

#ifndef _EFI_VARIABLE_H
#define _EFI_VARIABLE_H

#include <linux/bitops.h>

#define EFI_VARIABLE_READ_ONLY BIT(31)

enum efi_auth_var_type {
	EFI_AUTH_VAR_NONE = 0,
	EFI_AUTH_MODE,
	EFI_AUTH_VAR_PK,
	EFI_AUTH_VAR_KEK,
	EFI_AUTH_VAR_DB,
	EFI_AUTH_VAR_DBX,
	EFI_AUTH_VAR_DBT,
	EFI_AUTH_VAR_DBR,
};

/**
 * efi_get_variable() - retrieve value of a UEFI variable
 *
 * @variable_name:	name of the variable
 * @vendor:		vendor GUID
 * @attributes:		attributes of the variable
 * @data_size:		size of the buffer to which the variable value is copied
 * @data:		buffer to which the variable value is copied
 * @timep:		authentication time (seconds since start of epoch)
 * Return:		status code
 */
efi_status_t efi_get_variable_int(const u16 *variable_name,
				  const efi_guid_t *vendor,
				  u32 *attributes, efi_uintn_t *data_size,
				  void *data, u64 *timep);

/**
 * efi_set_variable() - set value of a UEFI variable
 *
 * @variable_name:	name of the variable
 * @vendor:		vendor GUID
 * @attributes:		attributes of the variable
 * @data_size:		size of the buffer with the variable value
 * @data:		buffer with the variable value
 * @ro_check:		check the read only read only bit in attributes
 * Return:		status code
 */
efi_status_t efi_set_variable_int(const u16 *variable_name,
				  const efi_guid_t *vendor,
				  u32 attributes, efi_uintn_t data_size,
				  const void *data, bool ro_check);

/**
 * efi_get_next_variable_name_int() - enumerate the current variable names
 *
 * @variable_name_size:	size of variable_name buffer in byte
 * @variable_name:	name of uefi variable's name in u16
 * @vendor:		vendor's guid
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * Return: status code
 */
efi_status_t efi_get_next_variable_name_int(efi_uintn_t *variable_name_size,
					    u16 *variable_name,
					    efi_guid_t *vendor);

/**
 * efi_query_variable_info_int() - get information about EFI variables
 *
 * This function implements the QueryVariableInfo() runtime service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * @attributes:				bitmask to select variables to be
 *					queried
 * @maximum_variable_storage_size:	maximum size of storage area for the
 *					selected variable types
 * @remaining_variable_storage_size:	remaining size of storage are for the
 *					selected variable types
 * @maximum_variable_size:		maximum size of a variable of the
 *					selected type
 * Returns:				status code
 */
efi_status_t efi_query_variable_info_int(u32 attributes,
					 u64 *maximum_variable_storage_size,
					 u64 *remaining_variable_storage_size,
					 u64 *maximum_variable_size);

#define EFI_VAR_FILE_NAME "ubootefi.var"

#define EFI_VAR_BUF_SIZE CONFIG_EFI_VAR_BUF_SIZE

/*
 * This constant identifies the file format for storing UEFI variables in
 * struct efi_var_file.
 */
#define EFI_VAR_FILE_MAGIC 0x0161566966456255 /* UbEfiVa, version 1 */

/**
 * struct efi_var_entry - UEFI variable file entry
 *
 * @length:	length of enty, multiple of 8
 * @attr:	variable attributes
 * @time:	authentication time (seconds since start of epoch)
 * @guid:	vendor GUID
 * @name:	UTF16 variable name
 */
struct efi_var_entry {
	u32 length;
	u32 attr;
	u64 time;
	efi_guid_t guid;
	u16 name[];
};

/**
 * struct efi_var_file - file for storing UEFI variables
 *
 * @reserved:	unused, may be overwritten by memory probing
 * @magic:	identifies file format, takes value %EFI_VAR_FILE_MAGIC
 * @length:	length including header
 * @crc32:	CRC32 without header
 * @var:	variables
 */
struct efi_var_file {
	u64 reserved;
	u64 magic;
	u32 length;
	u32 crc32;
	struct efi_var_entry var[];
};

/**
 * efi_var_to_file() - save non-volatile variables as file
 *
 * File ubootefi.var is created on the EFI system partion.
 *
 * Return:	status code
 */
efi_status_t efi_var_to_file(void);

/**
 * efi_var_collect() - collect variables in buffer
 *
 * A buffer is allocated and filled with variables in a format ready to be
 * written to disk.
 *
 * @bufp:		pointer to pointer of buffer with collected variables
 * @lenp:		pointer to length of buffer
 * @check_attr_mask:	bitmask with required attributes of variables to be collected.
 *                      variables are only collected if all of the required
 *                      attributes are set.
 * Return:		status code
 */
efi_status_t __maybe_unused efi_var_collect(struct efi_var_file **bufp, loff_t *lenp,
					    u32 check_attr_mask);

/**
 * efi_var_restore() - restore EFI variables from buffer
 *
 * Only if @safe is set secure boot related variables will be restored.
 *
 * @buf:	buffer
 * @safe:	restoring from tamper-resistant storage
 * Return:	status code
 */
efi_status_t efi_var_restore(struct efi_var_file *buf, bool safe);

/**
 * efi_var_from_file() - read variables from file
 *
 * File ubootefi.var is read from the EFI system partitions and the variables
 * stored in the file are created.
 *
 * In case the file does not exist yet or a variable cannot be set EFI_SUCCESS
 * is returned.
 *
 * Return:	status code
 */
efi_status_t efi_var_from_file(void);

/**
 * efi_var_mem_init() - set-up variable list
 *
 * Return:	status code
 */
efi_status_t efi_var_mem_init(void);

/**
 * efi_var_mem_find() - find a variable in the list
 *
 * @guid:	GUID of the variable
 * @name:	name of the variable
 * @next:	on exit pointer to the next variable after the found one
 * Return:	found variable
 */
struct efi_var_entry *efi_var_mem_find(const efi_guid_t *guid, const u16 *name,
				       struct efi_var_entry **next);

/**
 * efi_var_mem_del() - delete a variable from the list of variables
 *
 * @var:	variable to delete
 */
void efi_var_mem_del(struct efi_var_entry *var);

/**
 * efi_var_mem_ins() - append a variable to the list of variables
 *
 * The variable is appended without checking if a variable of the same name
 * already exists. The two data buffers are concatenated.
 *
 * @variable_name:	variable name
 * @vendor:		GUID
 * @attributes:		variable attributes
 * @size1:		size of the first data buffer
 * @data1:		first data buffer
 * @size2:		size of the second data field
 * @data2:		second data buffer
 * @time:		time of authentication (as seconds since start of epoch)
 * Result:		status code
 */
efi_status_t efi_var_mem_ins(const u16 *variable_name,
			     const efi_guid_t *vendor, u32 attributes,
			     const efi_uintn_t size1, const void *data1,
			     const efi_uintn_t size2, const void *data2,
			     const u64 time);

/**
 * efi_var_mem_free() - determine free memory for variables
 *
 * Return:	maximum data size plus variable name size
 */
u64 efi_var_mem_free(void);

/**
 * efi_init_secure_state - initialize secure boot state
 *
 * Return:	status code
 */
efi_status_t efi_init_secure_state(void);

/**
 * efi_auth_var_get_type() - convert variable name and guid to enum
 *
 * @name:	name of UEFI variable
 * @guid:	guid of UEFI variable
 * Return:	identifier for authentication related variables
 */
enum efi_auth_var_type efi_auth_var_get_type(const u16 *name,
					     const efi_guid_t *guid);

/**
 * efi_auth_var_get_guid() - get the predefined GUID for a variable name
 *
 * @name:	name of UEFI variable
 * Return:	guid of UEFI variable
 */
const efi_guid_t *efi_auth_var_get_guid(const u16 *name);

/**
 * efi_get_next_variable_name_mem() - Runtime common code across efi variable
 *                                    implementations for GetNextVariable()
 *                                    from the cached memory copy
 *
 * @variable_name_size:	size of variable_name buffer in bytes
 * @variable_name:	name of uefi variable's name in u16
 * @vendor:		vendor's guid
 *
 * Return: status code
 */
efi_status_t __efi_runtime
efi_get_next_variable_name_mem(efi_uintn_t *variable_name_size, u16 *variable_name,
			       efi_guid_t *vendor);
/**
 * efi_get_variable_mem() - Runtime common code across efi variable
 *                          implementations for GetVariable() from
 *                          the cached memory copy
 *
 * @variable_name:	name of the variable
 * @vendor:		vendor GUID
 * @attributes:		attributes of the variable
 * @data_size:		size of the buffer to which the variable value is copied
 * @data:		buffer to which the variable value is copied
 * @timep:		authentication time (seconds since start of epoch)
 * Return:		status code
 */
efi_status_t __efi_runtime
efi_get_variable_mem(const u16 *variable_name, const efi_guid_t *vendor,
		     u32 *attributes, efi_uintn_t *data_size, void *data,
		     u64 *timep);

/**
 * efi_get_variable_runtime() - runtime implementation of GetVariable()
 *
 * @variable_name:	name of the variable
 * @guid:		vendor GUID
 * @attributes:		attributes of the variable
 * @data_size:		size of the buffer to which the variable value is copied
 * @data:		buffer to which the variable value is copied
 * Return:		status code
 */
efi_status_t __efi_runtime EFIAPI
efi_get_variable_runtime(u16 *variable_name, const efi_guid_t *guid,
			 u32 *attributes, efi_uintn_t *data_size, void *data);

/**
 * efi_get_next_variable_name_runtime() - runtime implementation of
 *					  GetNextVariable()
 *
 * @variable_name_size:	size of variable_name buffer in byte
 * @variable_name:	name of uefi variable's name in u16
 * @guid:		vendor's guid
 * Return:              status code
 */
efi_status_t __efi_runtime EFIAPI
efi_get_next_variable_name_runtime(efi_uintn_t *variable_name_size,
				   u16 *variable_name, efi_guid_t *guid);

/**
 * efi_var_buf_update() - udpate memory buffer for variables
 *
 * @var_buf:	source buffer
 *
 * This function copies to the memory buffer for UEFI variables. Call this
 * function in ExitBootServices() if memory backed variables are only used
 * at runtime to fill the buffer.
 */
void efi_var_buf_update(struct efi_var_file *var_buf);

#endif
