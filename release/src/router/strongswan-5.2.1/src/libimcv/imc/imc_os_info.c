/*
 * Copyright (C) 2012-2014 Andreas Steffen
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

/* for GetTickCount64, Windows 7 */
#ifdef WIN32
# define _WIN32_WINNT 0x0601
#endif

#include "imc_os_info.h"

#include <stdio.h>
#include <stdarg.h>

#include <collections/linked_list.h>
#include <utils/debug.h>

typedef struct private_imc_os_info_t private_imc_os_info_t;

/**
 * Private data of an imc_os_info_t object.
 *
 */
struct private_imc_os_info_t {

	/**
	 * Public imc_os_info_t interface.
	 */
	imc_os_info_t public;

	/**
	 * OS type
	 */
	os_type_t type;

	/**
	 * OS name
	 */
	chunk_t name;

	/**
	 * OS version
	 */
	chunk_t version;

};

METHOD(imc_os_info_t, get_type, os_type_t,
	private_imc_os_info_t *this)
{
	return this->type;
}

METHOD(imc_os_info_t, get_name, chunk_t,
	private_imc_os_info_t *this)
{
	return this->name;
}

METHOD(imc_os_info_t, get_numeric_version, void,
	private_imc_os_info_t *this, u_int32_t *major, u_int32_t *minor)
{
	u_char *pos;

	if (major)
	{
		*major = atol(this->version.ptr);
	}
	pos = memchr(this->version.ptr, '.', this->version.len);
	if (minor)
	{
		*minor = pos ? atol(pos + 1) : 0;
	}
}

METHOD(imc_os_info_t, get_version, chunk_t,
	private_imc_os_info_t *this)
{
	return this->version;
}

#ifdef WIN32

METHOD(imc_os_info_t, get_fwd_status, os_fwd_status_t,
	private_imc_os_info_t *this)
{
	return OS_FWD_UNKNOWN;
}

METHOD(imc_os_info_t, get_uptime, time_t,
	private_imc_os_info_t *this)
{
	return GetTickCount64() / 1000;
}

METHOD(imc_os_info_t, get_setting, chunk_t,
	private_imc_os_info_t *this, char *name)
{
	return chunk_empty;
}

METHOD(imc_os_info_t, create_package_enumerator, enumerator_t*,
	private_imc_os_info_t *this)
{
	return NULL;
}

/**
 * Determine Windows release
 */
static bool extract_platform_info(os_type_t *type, chunk_t *name,
								  chunk_t *version)
{
	OSVERSIONINFOEX osvie;
	char buf[64];

	memset(&osvie, 0, sizeof(osvie));
	osvie.dwOSVersionInfoSize = sizeof(osvie);

	if (!GetVersionEx((LPOSVERSIONINFO)&osvie))
	{
		return FALSE;
	}
	*type = OS_TYPE_WINDOWS;
	snprintf(buf, sizeof(buf), "Windows %s %s",
			 osvie.wProductType == VER_NT_WORKSTATION ? "Client" : "Server",
#ifdef WIN64
			 "x86_64"
#else
			 "x86"
#endif
	);
	*name = chunk_clone(chunk_from_str(buf));

	snprintf(buf, sizeof(buf), "%d.%d.%d (SP %d.%d)",
			 osvie.dwMajorVersion, osvie.dwMinorVersion, osvie.dwBuildNumber,
			 osvie.wServicePackMajor, osvie.wServicePackMinor);
	*version = chunk_clone(chunk_from_str(buf));

	return TRUE;
}

#else /* !WIN32 */

#include <sys/utsname.h>

METHOD(imc_os_info_t, get_fwd_status, os_fwd_status_t,
	private_imc_os_info_t *this)
{
	const char ip_forward[] = "/proc/sys/net/ipv4/ip_forward";
	char buf[2];
	FILE *file;

	os_fwd_status_t fwd_status = OS_FWD_UNKNOWN;

	file = fopen(ip_forward, "r");
	if (file)
	{
		if (fread(buf, 1, 1, file) == 1)
		{
			switch (buf[0])
			{
				case '0':
					fwd_status = OS_FWD_DISABLED;
					break;
				case '1':
					fwd_status = OS_FWD_ENABLED;
					break;
				default:
					DBG1(DBG_IMC, "\"%s\" returns invalid value ", ip_forward);
					break;
			}
		}
		else
		{
			DBG1(DBG_IMC, "could not read from \"%s\"", ip_forward);
		}
		fclose(file);
	}
	else
	{
		DBG1(DBG_IMC, "failed to open \"%s\"", ip_forward);
	}

	return fwd_status;
}

METHOD(imc_os_info_t, get_uptime, time_t,
	private_imc_os_info_t *this)
{
	const char proc_uptime[] = "/proc/uptime";
	FILE *file;
	u_int uptime;

	file = fopen(proc_uptime, "r");
	if (!file)
	{
		DBG1(DBG_IMC, "failed to open \"%s\"", proc_uptime);
		return 0;
	}
	if (fscanf(file, "%u", &uptime) != 1)
	{
		DBG1(DBG_IMC, "failed to read file \"%s\"", proc_uptime);
		uptime = 0;
	}
	fclose(file);

	return uptime;
}

METHOD(imc_os_info_t, get_setting, chunk_t,
	private_imc_os_info_t *this, char *name)
{
	FILE *file;
	u_char buf[2048];
	size_t i = 0;
	chunk_t value;

	if (!strpfx(name, "/etc/") && !strpfx(name, "/proc/") &&
		!strpfx(name, "/sys/") && !strpfx(name, "/var/"))
	{
		/**
		 * In order to guarantee privacy, only settings from the
		 * /etc/, /proc/ and /sys/ directories can be retrieved
		 */
		DBG1(DBG_IMC, "not allowed to access '%s'", name);

		return chunk_empty;
	}

	file = fopen(name, "r");
	if (!file)
	{
		DBG1(DBG_IMC, "failed to open '%s'", name);

		return chunk_empty;
	}
	while (i < sizeof(buf) && fread(buf + i, 1, 1, file) == 1)
	{
		i++;
	}
	fclose(file);

	value = chunk_create(buf, i);

	return chunk_clone(value);
}

typedef struct {
	/**
	 * implements enumerator_t
	 */
	enumerator_t public;

	/**
	 * package info pipe stream
	 */
	FILE* file;

	/**
	 * line buffer
	 */
	u_char line[512];

} package_enumerator_t;

/**
 * Implementation of package_enumerator.destroy.
 */
static void package_enumerator_destroy(package_enumerator_t *this)
{
	pclose(this->file);
	free(this);
}

/**
 * Implementation of package_enumerator.enumerate
 */
static bool package_enumerator_enumerate(package_enumerator_t *this, ...)
{
	chunk_t *name, *version;
	u_char *pos;
	va_list args;

	while (TRUE)
	{
		if (!fgets(this->line, sizeof(this->line), this->file))
		{
			return FALSE;
		}

		pos = strchr(this->line, '\t');
		if (!pos)
		{
			return FALSE;
		}
		*pos++ = '\0';

		if (!streq(this->line, "install ok installed"))
		{
			continue;
		}
		va_start(args, this);

		name = va_arg(args, chunk_t*);
		name->ptr = pos;
		pos = strchr(pos, '\t');
		if (!pos)
		{
			va_end(args);
			return FALSE;
		}
		name->len = pos++ - name->ptr;

		version = va_arg(args, chunk_t*);
		version->ptr = pos;
		version->len = strlen(pos) - 1;

		va_end(args);
		return TRUE;
	}
}

METHOD(imc_os_info_t, create_package_enumerator, enumerator_t*,
	private_imc_os_info_t *this)
{
	FILE *file;
	const char command[] = "dpkg-query --show --showformat="
								"'${Status}\t${Package}\t${Version}\n'";
	package_enumerator_t *enumerator;

	/* Only Debian and Ubuntu package enumeration is currently supported */
	if (this->type != OS_TYPE_DEBIAN && this->type != OS_TYPE_UBUNTU)
	{
		return NULL;
	}

	/* Open a pipe stream for reading the output of the dpkg-query commmand */
	file = popen(command, "r");
	if (!file)
	{
		DBG1(DBG_IMC, "failed to run dpkg command");
		return NULL;
	}

	/* Create a package enumerator instance */
	enumerator = malloc_thing(package_enumerator_t);
	enumerator->public.enumerate = (void*)package_enumerator_enumerate;
	enumerator->public.destroy = (void*)package_enumerator_destroy;
	enumerator->file = file;

	return (enumerator_t*)enumerator;
}

#define RELEASE_LSB		0
#define RELEASE_DEBIAN	1

/**
 * Determine Linux distribution version and hardware platform
 */
static bool extract_platform_info(os_type_t *type, chunk_t *name,
								  chunk_t *version)
{
	FILE *file;
	u_char buf[BUF_LEN], *pos = buf;
	int len = BUF_LEN - 1;
	os_type_t os_type = OS_TYPE_UNKNOWN;
	chunk_t os_name = chunk_empty;
	chunk_t os_version = chunk_empty;
	char *os_str;
	struct utsname uninfo;
	int i;

	/* Linux/Unix distribution release info (from http://linuxmafia.com) */
	const char* releases[] = {
		"/etc/lsb-release",           "/etc/debian_version",
		"/etc/SuSE-release",          "/etc/novell-release",
		"/etc/sles-release",          "/etc/redhat-release",
		"/etc/fedora-release",        "/etc/gentoo-release",
		"/etc/slackware-version",     "/etc/annvix-release",
		"/etc/arch-release",          "/etc/arklinux-release",
		"/etc/aurox-release",         "/etc/blackcat-release",
		"/etc/cobalt-release",        "/etc/conectiva-release",
		"/etc/debian_release",        "/etc/immunix-release",
		"/etc/lfs-release",           "/etc/linuxppc-release",
		"/etc/mandrake-release",      "/etc/mandriva-release",
		"/etc/mandrakelinux-release", "/etc/mklinux-release",
		"/etc/pld-release",           "/etc/redhat_version",
		"/etc/slackware-release",     "/etc/e-smith-release",
		"/etc/release",               "/etc/sun-release",
		"/etc/tinysofa-release",      "/etc/turbolinux-release",
		"/etc/ultrapenguin-release",  "/etc/UnitedLinux-release",
		"/etc/va-release",            "/etc/yellowdog-release"
	};

	const char lsb_distrib_id[]      = "DISTRIB_ID=";
	const char lsb_distrib_release[] = "DISTRIB_RELEASE=";

	for (i = 0; i < countof(releases); i++)
	{
		file = fopen(releases[i], "r");
		if (!file)
		{
			continue;
		}

		/* read release file into buffer */
		fseek(file, 0, SEEK_END);
		len = min(ftell(file), len);
		rewind(file);
		buf[len] = '\0';
		if (fread(buf, 1, len, file) != len)
		{
			DBG1(DBG_IMC, "failed to read file \"%s\"", releases[i]);
			fclose(file);
			return FALSE;
		}
		fclose(file);

		DBG1(DBG_IMC, "processing \"%s\" file", releases[i]);

		switch (i)
		{
			case RELEASE_LSB:
			{
				/* Determine Distribution ID */
				pos = strstr(buf, lsb_distrib_id);
				if (!pos)
				{
					DBG1(DBG_IMC, "failed to find begin of DISTRIB_ID field");
					return FALSE;
				}
				pos += strlen(lsb_distrib_id);

				os_name.ptr = pos;

				pos = strchr(pos, '\n');
				if (!pos)
				{
					DBG1(DBG_IMC, "failed to find end of DISTRIB_ID field");
					return FALSE;
			 	}
				os_name.len = pos - os_name.ptr;

				/* Determine Distribution Release */
				pos = strstr(buf, lsb_distrib_release);
				if (!pos)
				{
					DBG1(DBG_IMC, "failed to find begin of DISTRIB_RELEASE field");
					return FALSE;
				}
				pos += strlen(lsb_distrib_release);

				os_version.ptr = pos;

				pos = strchr(pos, '\n');
				if (!pos)
				{
					DBG1(DBG_IMC, "failed to find end of DISTRIB_RELEASE field");
					return FALSE;
			 	}
				os_version.len = pos - os_version.ptr;

				break;
			}
			case RELEASE_DEBIAN:
			{
				os_type = OS_TYPE_DEBIAN;

				os_version.ptr = buf;
				pos = strchr(buf, '\n');
				if (!pos)
				{
					DBG1(DBG_PTS, "failed to find end of release string");
					return FALSE;
				}

				os_version.len = pos - os_version.ptr;

				break;
			}
			default:
			{
				const char str_release[] = " release ";

				os_name.ptr = buf;

				pos = strstr(buf, str_release);
				if (!pos)
				{
					DBG1(DBG_IMC, "failed to find release keyword");
					return FALSE;
				}

				os_name.len = pos - os_name.ptr;

				pos += strlen(str_release);
				os_version.ptr = pos;

				pos = strchr(pos, '\n');
				if (!pos)
				{
					DBG1(DBG_IMC, "failed to find end of release string");
					return FALSE;
			 	}

				os_version.len = pos - os_version.ptr;

				break;
			}
		}
		break;
	}

	if (!os_version.ptr)
	{
		DBG1(DBG_IMC, "no distribution release file found");
		return FALSE;
	}

	if (uname(&uninfo) < 0)
	{
		DBG1(DBG_IMC, "could not retrieve machine architecture");
		return FALSE;
	}

	/* Try to find a matching OS type based on the OS name */
	if (os_type == OS_TYPE_UNKNOWN)
	{
		os_type = os_type_from_name(os_name);
	}

	/* If known use the official OS name */
	if (os_type != OS_TYPE_UNKNOWN)
	{
		os_str = enum_to_name(os_type_names, os_type);
		os_name = chunk_create(os_str, strlen(os_str));
	}

	/* copy OS type */
	*type = os_type;

	/* copy OS name */
	*name = chunk_clone(os_name);

	/* copy OS version and machine architecture */
	*version = chunk_alloc(os_version.len + 1 + strlen(uninfo.machine));
	pos = version->ptr;
	memcpy(pos, os_version.ptr, os_version.len);
	pos += os_version.len;
	*pos++ = ' ';
	memcpy(pos, uninfo.machine, strlen(uninfo.machine));

	return TRUE;
}

#endif /* !WIN32 */

METHOD(imc_os_info_t, destroy, void,
	private_imc_os_info_t *this)
{
	free(this->name.ptr);
	free(this->version.ptr);
	free(this);
}

/**
 * See header
 */
imc_os_info_t *imc_os_info_create(void)
{
	private_imc_os_info_t *this;
	chunk_t name, version;
	os_type_t type;

	/* As an option OS name and OS version can be configured manually */
	name.ptr = lib->settings->get_str(lib->settings,
									  "%s.imcv.imc_os_info.name", NULL, lib->ns);
	version.ptr = lib->settings->get_str(lib->settings,
									  "%s.imcv.imc_os_info.version", NULL, lib->ns);
	if (name.ptr && version.ptr)
	{
		name.len = strlen(name.ptr);
		name = chunk_clone(name);

		version.len = strlen(version.ptr);
		version = chunk_clone(version);

		type = os_type_from_name(name);
	}
	else
	{
		if (!extract_platform_info(&type, &name, &version))
		{
			return NULL;
		}
	}
	DBG1(DBG_IMC, "operating system name is '%.*s'",
				   name.len, name.ptr);
	DBG1(DBG_IMC, "operating system version is '%.*s'",
				   version.len, version.ptr);

	INIT(this,
		.public = {
			.get_type = _get_type,
			.get_name = _get_name,
			.get_numeric_version = _get_numeric_version,
			.get_version = _get_version,
			.get_fwd_status = _get_fwd_status,
			.get_uptime = _get_uptime,
			.get_setting = _get_setting,
			.create_package_enumerator = _create_package_enumerator,
			.destroy = _destroy,
		},
		.type = type,
		.name = name,
		.version = version,
	);

	return &this->public;
}
