#ifndef fooutilhfoo
#define fooutilhfoo

/*
  This file is part of nss-mdns.

  nss-mdns is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2 of the
  License, or (at your option) any later version.

  nss-mdns is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with nss-mdns; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <sys/time.h>
#include <time.h>
#include <inttypes.h>
#include <netdb.h>
#include <nss.h>
#include <stdio.h>
#ifdef __FreeBSD__
#include <netinet/in.h>
#endif
#include <resolv.h>

#include "avahi.h"

// Simple buffer allocator.
typedef struct {
    char* next;
    char* end;
} buffer_t;

// Sets up a buffer.
void buffer_init(buffer_t* buf, char* buffer, size_t buflen);

// Allocates a zeroed, aligned chunk of memory of a given size from the buffer
// manager.
// If there is insufficient space, returns NULL.
void* buffer_alloc(buffer_t* buf, size_t size);

// Duplicates a string into a newly allocated chunk of memory.
// If there is insufficient space, returns NULL.
char* buffer_strdup(buffer_t* buf, const char* str);

// Macro to help with checking buffer allocation results.
#define RETURN_IF_FAILED_ALLOC(ptr)                                            \
    if (ptr == NULL) {                                                         \
        *errnop = ERANGE;                                                      \
        *h_errnop = NO_RECOVERY;                                               \
        return NSS_STATUS_TRYAGAIN;                                            \
    }

int set_cloexec(int fd);
int ends_with(const char* name, const char* suffix);

typedef enum {
    USE_NAME_RESULT_SKIP,
    USE_NAME_RESULT_AUTHORITATIVE,
    USE_NAME_RESULT_OPTIONAL,
} use_name_result_t;

typedef enum {
    TEST_LOCAL_SOA_NO,
    TEST_LOCAL_SOA_YES,
    TEST_LOCAL_SOA_AUTO,
} test_local_soa_t;

// Returns true if we should try to resolve the name with mDNS.
//
// If mdns_allow_file is NULL, then this implements the "local" SOA
// check and two-label name checks similarly to the algorithm
// described at https://support.apple.com/en-us/HT201275. This means
// that if a unicast DNS server claims authority on "local", or if the
// user tries to resolve a >2-label name, we will not do mDNS resolution.
//
// The two heuristics described above are disabled if mdns_allow_file
// is not NULL.
use_name_result_t verify_name_allowed_with_soa(const char* name,
                                               FILE* mdns_allow_file,
                                               test_local_soa_t test);

typedef enum {
    VERIFY_NAME_RESULT_NOT_ALLOWED,
    VERIFY_NAME_RESULT_ALLOWED_IF_NO_LOCAL_SOA,
    VERIFY_NAME_RESULT_ALLOWED
} verify_name_result_t;

// Tells us if the name is not allowed unconditionally, allowed only
// if local_soa() returns false, or unconditionally allowed.
verify_name_result_t verify_name_allowed(const char* name,
                                         FILE* mdns_allow_file);

// Returns true if a DNS server claims authority over "local".
int local_soa(void);

// Returns the number of labels in a name.
int label_count(const char* name);

// Converts from a name and addr into the hostent format, used by
// gethostbyaddr_r.
enum nss_status convert_name_and_addr_to_hostent(const char* name,
                                                 const void* addr, int len,
                                                 int af, struct hostent* result,
                                                 buffer_t* buf, int* errnop,
                                                 int* h_errnop);

// Converts from the userdata struct into the hostent format, used by
// gethostbyaddr3_r.
enum nss_status convert_userdata_for_name_to_hostent(const userdata_t* u,
                                                     const char* name, int af,
                                                     struct hostent* result,
                                                     buffer_t* buf, int* errnop,
                                                     int* h_errnop);

// Converts from the userdata struct into the gaih_addrtuple format, used by
// gethostbyaddr4_r.
#ifndef __FreeBSD__
enum nss_status convert_userdata_to_addrtuple(const userdata_t* u,
                                              const char* name,
                                              struct gaih_addrtuple** pat,
                                              buffer_t* buf, int* errnop,
                                              int* h_errnop);
#endif

// Appends a query_address_result to userdata.
void append_address_to_userdata(const query_address_result_t* result,
                                userdata_t* u);

#endif
