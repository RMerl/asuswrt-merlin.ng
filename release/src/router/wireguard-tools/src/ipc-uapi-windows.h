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

static FILE *userspace_interface_file(const char *iface)
{
	char fname[MAX_PATH], error_message[1024 * 128] = { 0 };
	HANDLE thread_token, process_snapshot, winlogon_process, winlogon_token, duplicated_token, pipe_handle = INVALID_HANDLE_VALUE;
	PROCESSENTRY32 entry = { .dwSize = sizeof(PROCESSENTRY32) };
	PSECURITY_DESCRIPTOR pipe_sd;
	PSID pipe_sid;
	SID expected_sid;
	BOOL ret;
	int fd;
	DWORD last_error = ERROR_SUCCESS, bytes = sizeof(expected_sid);
	TOKEN_PRIVILEGES privileges = {
		.PrivilegeCount = 1,
		.Privileges = {{ .Attributes = SE_PRIVILEGE_ENABLED }}
	};

	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &privileges.Privileges[0].Luid))
		goto err;
	if (!CreateWellKnownSid(WinLocalSystemSid, NULL, &expected_sid, &bytes))
		goto err;

	process_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (process_snapshot == INVALID_HANDLE_VALUE)
		goto err;
	for (ret = Process32First(process_snapshot, &entry); ret; last_error = GetLastError(), ret = Process32Next(process_snapshot, &entry)) {
		if (strcasecmp(entry.szExeFile, "winlogon.exe"))
			continue;

		RevertToSelf();
		if (!ImpersonateSelf(SecurityImpersonation))
			continue;
		if (!OpenThreadToken(GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES, FALSE, &thread_token))
			continue;
		if (!AdjustTokenPrivileges(thread_token, FALSE, &privileges, sizeof(privileges), NULL, NULL)) {
			last_error = GetLastError();
			CloseHandle(thread_token);
			continue;
		}
		CloseHandle(thread_token);

		winlogon_process = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, entry.th32ProcessID);
		if (!winlogon_process)
			continue;
		if (!OpenProcessToken(winlogon_process, TOKEN_IMPERSONATE | TOKEN_DUPLICATE, &winlogon_token))
			continue;
		CloseHandle(winlogon_process);
		if (!DuplicateToken(winlogon_token, SecurityImpersonation, &duplicated_token)) {
			last_error = GetLastError();
			RevertToSelf();
			continue;
		}
		CloseHandle(winlogon_token);
		if (!SetThreadToken(NULL, duplicated_token)) {
			last_error = GetLastError();
			CloseHandle(duplicated_token);
			continue;
		}
		CloseHandle(duplicated_token);

		snprintf(fname, sizeof(fname), "\\\\.\\pipe\\ProtectedPrefix\\Administrators\\WireGuard\\%s", iface);
		pipe_handle = CreateFile(fname, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
		last_error = GetLastError();
		if (pipe_handle == INVALID_HANDLE_VALUE)
			continue;
		last_error = GetSecurityInfo(pipe_handle, SE_FILE_OBJECT, OWNER_SECURITY_INFORMATION, &pipe_sid, NULL, NULL, NULL, &pipe_sd);
		if (last_error != ERROR_SUCCESS) {
			CloseHandle(pipe_handle);
			continue;
		}
		last_error = EqualSid(&expected_sid, pipe_sid) ? ERROR_SUCCESS : ERROR_ACCESS_DENIED;
		LocalFree(pipe_sd);
		if (last_error != ERROR_SUCCESS) {
			CloseHandle(pipe_handle);
			continue;
		}
		last_error = ERROR_SUCCESS;
		break;
	}
	RevertToSelf();
	CloseHandle(process_snapshot);

	if (last_error != ERROR_SUCCESS || pipe_handle == INVALID_HANDLE_VALUE)
		goto err;
	fd = _open_osfhandle((intptr_t)pipe_handle, _O_RDWR);
	if (fd == -1) {
		last_error = GetLastError();
		CloseHandle(pipe_handle);
		goto err;
	}
	return _fdopen(fd, "r+");

err:
	if (last_error == ERROR_SUCCESS)
		last_error = GetLastError();
	if (last_error == ERROR_SUCCESS)
		last_error = ERROR_ACCESS_DENIED;
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, last_error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), error_message, sizeof(error_message) - 1, NULL);
	fprintf(stderr, "Error: Unable to open IPC handle via SYSTEM impersonation: %ld: %s\n", last_error, error_message);
	errno = EACCES;
	return NULL;
}

static int userspace_get_wireguard_interfaces(struct string_list *list)
{
	static const char prefix[] = "ProtectedPrefix\\Administrators\\WireGuard\\";
	WIN32_FIND_DATA find_data;
	HANDLE find_handle;
	int ret = 0;

	find_handle = FindFirstFile("\\\\.\\pipe\\*", &find_data);
	if (find_handle == INVALID_HANDLE_VALUE)
		return -GetLastError();
	do {
		if (strncmp(prefix, find_data.cFileName, strlen(prefix)))
			continue;
		ret = string_list_add(list, find_data.cFileName + strlen(prefix));
		if (ret < 0)
			goto out;
	} while (FindNextFile(find_handle, &find_data));

out:
	FindClose(find_handle);
	return ret;
}
