// SPDX-License-Identifier: GPL-2.0+
/*
 *  EFI utils
 *
 *  Copyright (c) 2017 Rob Clark
 */

#include <malloc.h>
#include <charset.h>
#include <efi_loader.h>
#include <hexdump.h>
#include <environment.h>
#include <search.h>
#include <uuid.h>

#define READ_ONLY BIT(31)

/*
 * Mapping between EFI variables and u-boot variables:
 *
 *   efi_$guid_$varname = {attributes}(type)value
 *
 * For example:
 *
 *   efi_8be4df61-93ca-11d2-aa0d-00e098032b8c_OsIndicationsSupported=
 *      "{ro,boot,run}(blob)0000000000000000"
 *   efi_8be4df61-93ca-11d2-aa0d-00e098032b8c_BootOrder=
 *      "(blob)00010000"
 *
 * The attributes are a comma separated list of these possible
 * attributes:
 *
 *   + ro   - read-only
 *   + boot - boot-services access
 *   + run  - runtime access
 *
 * NOTE: with current implementation, no variables are available after
 * ExitBootServices, and all are persisted (if possible).
 *
 * If not specified, the attributes default to "{boot}".
 *
 * The required type is one of:
 *
 *   + utf8 - raw utf8 string
 *   + blob - arbitrary length hex string
 *
 * Maybe a utf16 type would be useful to for a string value to be auto
 * converted to utf16?
 */

#define PREFIX_LEN (strlen("efi_xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx_"))

/**
 * efi_to_native() - convert the UEFI variable name and vendor GUID to U-Boot
 *		     variable name
 *
 * The U-Boot variable name is a concatenation of prefix 'efi', the hexstring
 * encoded vendor GUID, and the UTF-8 encoded UEFI variable name separated by
 * underscores, e.g. 'efi_8be4df61-93ca-11d2-aa0d-00e098032b8c_BootOrder'.
 *
 * @native:		pointer to pointer to U-Boot variable name
 * @variable_name:	UEFI variable name
 * @vendor:		vendor GUID
 * Return:		status code
 */
static efi_status_t efi_to_native(char **native, const u16 *variable_name,
				  const efi_guid_t *vendor)
{
	size_t len;
	char *pos;

	len = PREFIX_LEN + utf16_utf8_strlen(variable_name) + 1;
	*native = malloc(len);
	if (!*native)
		return EFI_OUT_OF_RESOURCES;

	pos = *native;
	pos += sprintf(pos, "efi_%pUl_", vendor);
	utf16_utf8_strcpy(&pos, variable_name);

	return EFI_SUCCESS;
}

/**
 * prefix() - skip over prefix
 *
 * Skip over a prefix string.
 *
 * @str:	string with prefix
 * @prefix:	prefix string
 * Return:	string without prefix, or NULL if prefix not found
 */
static const char *prefix(const char *str, const char *prefix)
{
	size_t n = strlen(prefix);
	if (!strncmp(prefix, str, n))
		return str + n;
	return NULL;
}

/**
 * parse_attr() - decode attributes part of variable value
 *
 * Convert the string encoded attributes of a UEFI variable to a bit mask.
 * TODO: Several attributes are not supported.
 *
 * @str:	value of U-Boot variable
 * @attrp:	pointer to UEFI attributes
 * Return:	pointer to remainder of U-Boot variable value
 */
static const char *parse_attr(const char *str, u32 *attrp)
{
	u32 attr = 0;
	char sep = '{';

	if (*str != '{') {
		*attrp = EFI_VARIABLE_BOOTSERVICE_ACCESS;
		return str;
	}

	while (*str == sep) {
		const char *s;

		str++;

		if ((s = prefix(str, "ro"))) {
			attr |= READ_ONLY;
		} else if ((s = prefix(str, "nv"))) {
			attr |= EFI_VARIABLE_NON_VOLATILE;
		} else if ((s = prefix(str, "boot"))) {
			attr |= EFI_VARIABLE_BOOTSERVICE_ACCESS;
		} else if ((s = prefix(str, "run"))) {
			attr |= EFI_VARIABLE_RUNTIME_ACCESS;
		} else {
			printf("invalid attribute: %s\n", str);
			break;
		}

		str = s;
		sep = ',';
	}

	str++;

	*attrp = attr;

	return str;
}

/**
 * efi_get_variable() - retrieve value of a UEFI variable
 *
 * This function implements the GetVariable runtime service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * @variable_name:	name of the variable
 * @vendor:		vendor GUID
 * @attributes:		attributes of the variable
 * @data_size:		size of the buffer to which the variable value is copied
 * @data:		buffer to which the variable value is copied
 * Return:		status code
 */
efi_status_t EFIAPI efi_get_variable(u16 *variable_name,
				     const efi_guid_t *vendor, u32 *attributes,
				     efi_uintn_t *data_size, void *data)
{
	char *native_name;
	efi_status_t ret;
	unsigned long in_size;
	const char *val, *s;
	u32 attr;

	EFI_ENTRY("\"%ls\" %pUl %p %p %p", variable_name, vendor, attributes,
		  data_size, data);

	if (!variable_name || !vendor || !data_size)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	ret = efi_to_native(&native_name, variable_name, vendor);
	if (ret)
		return EFI_EXIT(ret);

	EFI_PRINT("get '%s'\n", native_name);

	val = env_get(native_name);
	free(native_name);
	if (!val)
		return EFI_EXIT(EFI_NOT_FOUND);

	val = parse_attr(val, &attr);

	in_size = *data_size;

	if ((s = prefix(val, "(blob)"))) {
		size_t len = strlen(s);

		/* number of hexadecimal digits must be even */
		if (len & 1)
			return EFI_EXIT(EFI_DEVICE_ERROR);

		/* two characters per byte: */
		len /= 2;
		*data_size = len;

		if (in_size < len) {
			ret = EFI_BUFFER_TOO_SMALL;
			goto out;
		}

		if (!data)
			return EFI_EXIT(EFI_INVALID_PARAMETER);

		if (hex2bin(data, s, len))
			return EFI_EXIT(EFI_DEVICE_ERROR);

		EFI_PRINT("got value: \"%s\"\n", s);
	} else if ((s = prefix(val, "(utf8)"))) {
		unsigned len = strlen(s) + 1;

		*data_size = len;

		if (in_size < len) {
			ret = EFI_BUFFER_TOO_SMALL;
			goto out;
		}

		if (!data)
			return EFI_EXIT(EFI_INVALID_PARAMETER);

		memcpy(data, s, len);
		((char *)data)[len] = '\0';

		EFI_PRINT("got value: \"%s\"\n", (char *)data);
	} else {
		EFI_PRINT("invalid value: '%s'\n", val);
		return EFI_EXIT(EFI_DEVICE_ERROR);
	}

out:
	if (attributes)
		*attributes = attr & EFI_VARIABLE_MASK;

	return EFI_EXIT(ret);
}

static char *efi_variables_list;
static char *efi_cur_variable;

/**
 * parse_uboot_variable() - parse a u-boot variable and get uefi-related
 *			    information
 * @variable:		whole data of u-boot variable (ie. name=value)
 * @variable_name_size: size of variable_name buffer in byte
 * @variable_name:	name of uefi variable in u16, null-terminated
 * @vendor:		vendor's guid
 * @attributes:		attributes
 *
 * A uefi variable is encoded into a u-boot variable as described above.
 * This function parses such a u-boot variable and retrieve uefi-related
 * information into respective parameters. In return, variable_name_size
 * is the size of variable name including NULL.
 *
 * Return:		EFI_SUCCESS if parsing is OK, EFI_NOT_FOUND when
			the entire variable list has been returned,
			otherwise non-zero status code
 */
static efi_status_t parse_uboot_variable(char *variable,
					 efi_uintn_t *variable_name_size,
					 u16 *variable_name,
					 const efi_guid_t *vendor,
					 u32 *attributes)
{
	char *guid, *name, *end, c;
	unsigned long name_len;
	u16 *p;

	guid = strchr(variable, '_');
	if (!guid)
		return EFI_INVALID_PARAMETER;
	guid++;
	name = strchr(guid, '_');
	if (!name)
		return EFI_INVALID_PARAMETER;
	name++;
	end = strchr(name, '=');
	if (!end)
		return EFI_INVALID_PARAMETER;

	name_len = end - name;
	if (*variable_name_size < (name_len + 1)) {
		*variable_name_size = name_len + 1;
		return EFI_BUFFER_TOO_SMALL;
	}
	end++; /* point to value */

	/* variable name */
	p = variable_name;
	utf8_utf16_strncpy(&p, name, name_len);
	variable_name[name_len] = 0;
	*variable_name_size = name_len + 1;

	/* guid */
	c = *(name - 1);
	*(name - 1) = '\0'; /* guid need be null-terminated here */
	uuid_str_to_bin(guid, (unsigned char *)vendor, UUID_STR_FORMAT_GUID);
	*(name - 1) = c;

	/* attributes */
	parse_attr(end, attributes);

	return EFI_SUCCESS;
}

/**
 * efi_get_next_variable_name() - enumerate the current variable names
 * @variable_name_size:	size of variable_name buffer in byte
 * @variable_name:	name of uefi variable's name in u16
 * @vendor:		vendor's guid
 *
 * This function implements the GetNextVariableName service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details: http://wiki.phoenix.com/wiki/index.php/
 *		EFI_RUNTIME_SERVICES#GetNextVariableName.28.29
 *
 * Return: status code
 */
efi_status_t EFIAPI efi_get_next_variable_name(efi_uintn_t *variable_name_size,
					       u16 *variable_name,
					       const efi_guid_t *vendor)
{
	char *native_name, *variable;
	ssize_t name_len, list_len;
	char regex[256];
	char * const regexlist[] = {regex};
	u32 attributes;
	int i;
	efi_status_t ret;

	EFI_ENTRY("%p \"%ls\" %pUl", variable_name_size, variable_name, vendor);

	if (!variable_name_size || !variable_name || !vendor)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	if (variable_name[0]) {
		/* check null-terminated string */
		for (i = 0; i < *variable_name_size; i++)
			if (!variable_name[i])
				break;
		if (i >= *variable_name_size)
			return EFI_EXIT(EFI_INVALID_PARAMETER);

		/* search for the last-returned variable */
		ret = efi_to_native(&native_name, variable_name, vendor);
		if (ret)
			return EFI_EXIT(ret);

		name_len = strlen(native_name);
		for (variable = efi_variables_list; variable && *variable;) {
			if (!strncmp(variable, native_name, name_len) &&
			    variable[name_len] == '=')
				break;

			variable = strchr(variable, '\n');
			if (variable)
				variable++;
		}

		free(native_name);
		if (!(variable && *variable))
			return EFI_EXIT(EFI_INVALID_PARAMETER);

		/* next variable */
		variable = strchr(variable, '\n');
		if (variable)
			variable++;
		if (!(variable && *variable))
			return EFI_EXIT(EFI_NOT_FOUND);
	} else {
		/*
		 *new search: free a list used in the previous search
		 */
		free(efi_variables_list);
		efi_variables_list = NULL;
		efi_cur_variable = NULL;

		snprintf(regex, 256, "efi_.*-.*-.*-.*-.*_.*");
		list_len = hexport_r(&env_htab, '\n',
				     H_MATCH_REGEX | H_MATCH_KEY,
				     &efi_variables_list, 0, 1, regexlist);
		/* 1 indicates that no match was found */
		if (list_len <= 1)
			return EFI_EXIT(EFI_NOT_FOUND);

		variable = efi_variables_list;
	}

	ret = parse_uboot_variable(variable, variable_name_size, variable_name,
				   vendor, &attributes);

	return EFI_EXIT(ret);
}

/**
 * efi_set_variable() - set value of a UEFI variable
 *
 * This function implements the SetVariable runtime service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * @variable_name:	name of the variable
 * @vendor:		vendor GUID
 * @attributes:		attributes of the variable
 * @data_size:		size of the buffer with the variable value
 * @data:		buffer with the variable value
 * Return:		status code
 */
efi_status_t EFIAPI efi_set_variable(u16 *variable_name,
				     const efi_guid_t *vendor, u32 attributes,
				     efi_uintn_t data_size, const void *data)
{
	char *native_name = NULL, *val = NULL, *s;
	efi_status_t ret = EFI_SUCCESS;
	u32 attr;

	EFI_ENTRY("\"%ls\" %pUl %x %zu %p", variable_name, vendor, attributes,
		  data_size, data);

	/* TODO: implement APPEND_WRITE */
	if (!variable_name || !*variable_name || !vendor ||
	    ((attributes & EFI_VARIABLE_RUNTIME_ACCESS) &&
	     !(attributes & EFI_VARIABLE_BOOTSERVICE_ACCESS)) ||
	    (attributes & EFI_VARIABLE_APPEND_WRITE)) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	ret = efi_to_native(&native_name, variable_name, vendor);
	if (ret)
		goto out;

#define ACCESS_ATTR (EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS)

	if ((data_size == 0) || !(attributes & ACCESS_ATTR)) {
		/* delete the variable: */
		env_set(native_name, NULL);
		ret = EFI_SUCCESS;
		goto out;
	}

	val = env_get(native_name);
	if (val) {
		parse_attr(val, &attr);

		/* We should not free val */
		val = NULL;
		if (attr & READ_ONLY) {
			ret = EFI_WRITE_PROTECTED;
			goto out;
		}

		/*
		 * attributes won't be changed
		 * TODO: take care of APPEND_WRITE once supported
		 */
		if (attr != attributes) {
			ret = EFI_INVALID_PARAMETER;
			goto out;
		}
	}

	val = malloc(2 * data_size + strlen("{ro,run,boot,nv}(blob)") + 1);
	if (!val) {
		ret = EFI_OUT_OF_RESOURCES;
		goto out;
	}

	s = val;

	/*
	 * store attributes
	 * TODO: several attributes are not supported
	 */
	attributes &= (EFI_VARIABLE_NON_VOLATILE |
		       EFI_VARIABLE_BOOTSERVICE_ACCESS |
		       EFI_VARIABLE_RUNTIME_ACCESS);
	s += sprintf(s, "{");
	while (attributes) {
		u32 attr = 1 << (ffs(attributes) - 1);

		if (attr == EFI_VARIABLE_NON_VOLATILE)
			s += sprintf(s, "nv");
		else if (attr == EFI_VARIABLE_BOOTSERVICE_ACCESS)
			s += sprintf(s, "boot");
		else if (attr == EFI_VARIABLE_RUNTIME_ACCESS)
			s += sprintf(s, "run");

		attributes &= ~attr;
		if (attributes)
			s += sprintf(s, ",");
	}
	s += sprintf(s, "}");

	/* store payload: */
	s += sprintf(s, "(blob)");
	s = bin2hex(s, data, data_size);
	*s = '\0';

	EFI_PRINT("setting: %s=%s\n", native_name, val);

	if (env_set(native_name, val))
		ret = EFI_DEVICE_ERROR;

out:
	free(native_name);
	free(val);

	return EFI_EXIT(ret);
}
