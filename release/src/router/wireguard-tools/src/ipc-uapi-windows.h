// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2015-2020 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 */

#include <windows.h>
#include <tlhelp32.h>
#include <accctrl.h>
#include <aclapi.h>
#include <stdio.h>
#include <stdbool.h>
#include <fcntl.h>
#include <hashtable.h>

static FILE *userspace_interface_file(const char *iface)
{
	char fname[MAX_PATH];
	HANDLE pipe_handle;
	SID expected_sid;
	DWORD bytes = sizeof(expected_sid);
	PSID pipe_sid;
	PSECURITY_DESCRIPTOR pipe_sd;
	bool equal;
	int fd;

	if (!CreateWellKnownSid(WinLocalSystemSid, NULL, &expected_sid, &bytes))
		goto err;

	snprintf(fname, sizeof(fname), "\\\\.\\pipe\\ProtectedPrefix\\Administrators\\WireGuard\\%s", iface);
	pipe_handle = CreateFileA(fname, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (pipe_handle == INVALID_HANDLE_VALUE)
		goto err;
	if (GetSecurityInfo(pipe_handle, SE_FILE_OBJECT, OWNER_SECURITY_INFORMATION, &pipe_sid, NULL, NULL, NULL, &pipe_sd) != ERROR_SUCCESS)
		goto err_close;
	equal = EqualSid(&expected_sid, pipe_sid);
	LocalFree(pipe_sd);
	if (!equal)
		goto err_close;
	fd = _open_osfhandle((intptr_t)pipe_handle, _O_RDWR);
	if (fd == -1) {
		CloseHandle(pipe_handle);
		return NULL;
	}
	return _fdopen(fd, "r+");
err_close:
	CloseHandle(pipe_handle);
err:
	errno = EACCES;
	return NULL;
}

static bool have_cached_interfaces;
static struct hashtable cached_interfaces;

static bool userspace_has_wireguard_interface(const char *iface)
{
	char fname[MAX_PATH];
	WIN32_FIND_DATA find_data;
	HANDLE find_handle;
	bool ret = false;

	if (have_cached_interfaces)
		return hashtable_find_entry(&cached_interfaces, iface) != NULL;

	snprintf(fname, sizeof(fname), "ProtectedPrefix\\Administrators\\WireGuard\\%s", iface);
	find_handle = FindFirstFile("\\\\.\\pipe\\*", &find_data);
	if (find_handle == INVALID_HANDLE_VALUE)
		return -EIO;
	do {
		if (!strcmp(fname, find_data.cFileName)) {
			ret = true;
			break;
		}
	} while (FindNextFile(find_handle, &find_data));
	FindClose(find_handle);
	return ret;
}

static int userspace_get_wireguard_interfaces(struct string_list *list)
{
	static const char prefix[] = "ProtectedPrefix\\Administrators\\WireGuard\\";
	WIN32_FIND_DATA find_data;
	HANDLE find_handle;
	char *iface;
	int ret = 0;

	find_handle = FindFirstFile("\\\\.\\pipe\\*", &find_data);
	if (find_handle == INVALID_HANDLE_VALUE)
		return -EIO;
	do {
		if (strncmp(prefix, find_data.cFileName, strlen(prefix)))
			continue;
		iface = find_data.cFileName + strlen(prefix);
		ret = string_list_add(list, iface);
		if (ret < 0)
			goto out;
		if (!hashtable_find_or_insert_entry(&cached_interfaces, iface)) {
			ret = -errno;
			goto out;
		}
	} while (FindNextFile(find_handle, &find_data));
	have_cached_interfaces = true;

out:
	FindClose(find_handle);
	return ret;
}
