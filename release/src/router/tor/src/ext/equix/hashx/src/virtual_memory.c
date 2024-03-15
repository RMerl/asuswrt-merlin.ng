/* Copyright (c) 2020 tevador <tevador@gmail.com> */
/* See LICENSE for licensing information */

#include "virtual_memory.h"

#ifdef HASHX_WIN
#include <windows.h>
#else
#ifdef __APPLE__
#include <mach/vm_statistics.h>
#endif
#include <sys/types.h>
#include <sys/mman.h>
#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif
#define PAGE_READONLY PROT_READ
#define PAGE_READWRITE (PROT_READ | PROT_WRITE)
#define PAGE_EXECUTE_READ (PROT_READ | PROT_EXEC)
#define PAGE_EXECUTE_READWRITE (PROT_READ | PROT_WRITE | PROT_EXEC)
#if defined(__NetBSD__) && defined(PROT_MPROTECT)
#define PAGE_MMAP_PROT (PAGE_READWRITE | PROT_MPROTECT(PROT_EXEC))
#else
#define PAGE_MMAP_PROT PAGE_READWRITE
#endif
#endif

#ifdef HASHX_WIN

static bool set_privilege(const char* pszPrivilege, BOOL bEnable) {
	HANDLE           hToken;
	TOKEN_PRIVILEGES tp;
	BOOL             status;
	DWORD            error;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES
		| TOKEN_QUERY, &hToken))
		return false;

	if (!LookupPrivilegeValue(NULL, pszPrivilege, &tp.Privileges[0].Luid))
		return false;

	tp.PrivilegeCount = 1;

	if (bEnable)
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	else
		tp.Privileges[0].Attributes = 0;

	status = AdjustTokenPrivileges(hToken, FALSE, &tp, 0,
		(PTOKEN_PRIVILEGES)NULL, 0);
	error = GetLastError();

	CloseHandle(hToken);

	return status && (error == ERROR_SUCCESS);
}
#endif

void* hashx_vm_alloc(size_t bytes) {
	void* mem;
#ifdef HASHX_WIN
	mem = VirtualAlloc(NULL, bytes, MEM_COMMIT, PAGE_READWRITE);
#else
	mem = mmap(NULL, bytes, PAGE_MMAP_PROT, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	if (mem == MAP_FAILED)
		return NULL;
#endif
	return mem;
}

static inline bool page_protect(void* ptr, size_t bytes, int rules) {
#ifdef HASHX_WIN
	DWORD oldp;
	if (!VirtualProtect(ptr, bytes, (DWORD)rules, &oldp)) {
		return false;
	}
#else
	if (mprotect(ptr, bytes, rules) != 0)
		return false;
#endif
	return true;
}

bool hashx_vm_rw(void* ptr, size_t bytes) {
	return page_protect(ptr, bytes, PAGE_READWRITE);
}

bool hashx_vm_rx(void* ptr, size_t bytes) {
	return page_protect(ptr, bytes, PAGE_EXECUTE_READ);
}

#ifdef EQUIX_SUPPORT_HUGEPAGES
void* hashx_vm_alloc_huge(size_t bytes) {
	void* mem;
#ifdef HASHX_WIN
	if (!set_privilege("SeLockMemoryPrivilege", 1)) {
		/* Failed, but try the VirtualAlloc anyway */
	}
	SIZE_T page_min = GetLargePageMinimum();
	if (page_min > 0) {
		mem = VirtualAlloc(NULL, ALIGN_SIZE(bytes, page_min), MEM_COMMIT
			| MEM_RESERVE | MEM_LARGE_PAGES, PAGE_READWRITE);
	}
	else {
		mem = NULL;
	}
#else
#ifdef __APPLE__
	mem = mmap(NULL, bytes, PAGE_READWRITE, MAP_PRIVATE | MAP_ANONYMOUS,
		VM_FLAGS_SUPERPAGE_SIZE_2MB, 0);
#elif defined(__FreeBSD__)
	mem = mmap(NULL, bytes, PAGE_READWRITE, MAP_PRIVATE | MAP_ANONYMOUS
		| MAP_ALIGNED_SUPER, -1, 0);
#elif defined(__OpenBSD__) || defined(__NetBSD__)
	(void)bytes;
	mem = MAP_FAILED; // OpenBSD and NetBSD do not support huge pages
#else
	mem = mmap(NULL, bytes, PAGE_READWRITE, MAP_PRIVATE | MAP_ANONYMOUS
		| MAP_HUGETLB | MAP_POPULATE, -1, 0);
#endif
	if (mem == MAP_FAILED) {
		mem = NULL;
	}
#endif
	return mem;
}
#endif /* EQUIX_SUPPORT_HUGEPAGES */

void hashx_vm_free(void* ptr, size_t bytes) {
	if (!ptr) {
		return;
	}
#ifdef HASHX_WIN
	(void)bytes;
	VirtualFree(ptr, 0, MEM_RELEASE);
#else
	munmap(ptr, bytes);
#endif
}
