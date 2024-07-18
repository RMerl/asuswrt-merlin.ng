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

#define _DEFAULT_SOURCE

#include <check.h>
#include <errno.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include "../src/util.h"

// Tests that verify_name_allowed works in MINIMAL mode, or with no config file.
// Only names with TLD "local" are allowed.
// Only 2-label names are allowed.
// SOA check is required.
START_TEST(test_verify_name_allowed_minimal) {
    ck_assert_int_eq(verify_name_allowed("example.local", NULL),
                     VERIFY_NAME_RESULT_ALLOWED_IF_NO_LOCAL_SOA);
    ck_assert_int_eq(verify_name_allowed("example.local.", NULL),
                     VERIFY_NAME_RESULT_ALLOWED_IF_NO_LOCAL_SOA);
    ck_assert_int_eq(verify_name_allowed("com.example.local", NULL),
                     VERIFY_NAME_RESULT_NOT_ALLOWED);
    ck_assert_int_eq(verify_name_allowed("com.example.local.", NULL),
                     VERIFY_NAME_RESULT_NOT_ALLOWED);
    ck_assert_int_eq(verify_name_allowed("example.com", NULL),
                     VERIFY_NAME_RESULT_NOT_ALLOWED);
    ck_assert_int_eq(verify_name_allowed("example.com.", NULL),
                     VERIFY_NAME_RESULT_NOT_ALLOWED);
    ck_assert_int_eq(verify_name_allowed("example.local.com", NULL),
                     VERIFY_NAME_RESULT_NOT_ALLOWED);
    ck_assert_int_eq(verify_name_allowed("example.local.com.", NULL),
                     VERIFY_NAME_RESULT_NOT_ALLOWED);
    ck_assert_int_eq(verify_name_allowed("", NULL),
                     VERIFY_NAME_RESULT_NOT_ALLOWED);
    ck_assert_int_eq(verify_name_allowed(".", NULL),
                     VERIFY_NAME_RESULT_NOT_ALLOWED);

    ck_assert_int_eq(verify_name_allowed_with_soa(".", NULL, TEST_LOCAL_SOA_YES),
                     USE_NAME_RESULT_SKIP);
    ck_assert_int_eq(verify_name_allowed_with_soa(".", NULL, TEST_LOCAL_SOA_NO),
                     USE_NAME_RESULT_SKIP);
    ck_assert_int_eq(verify_name_allowed_with_soa(".", NULL, TEST_LOCAL_SOA_AUTO),
                     USE_NAME_RESULT_SKIP);
    ck_assert_int_eq(verify_name_allowed_with_soa("example3.sub.local",
                         NULL, TEST_LOCAL_SOA_YES), USE_NAME_RESULT_SKIP);
    ck_assert_int_eq(verify_name_allowed_with_soa("example4.sub.local",
                         NULL, TEST_LOCAL_SOA_NO), USE_NAME_RESULT_SKIP);
    ck_assert_int_eq(verify_name_allowed_with_soa("example4.sub.local",
                         NULL, TEST_LOCAL_SOA_AUTO), USE_NAME_RESULT_SKIP);
    ck_assert_int_eq(verify_name_allowed_with_soa("example1.local",
                         NULL, TEST_LOCAL_SOA_YES), USE_NAME_RESULT_OPTIONAL);
    ck_assert_int_eq(verify_name_allowed_with_soa("example2.local",
                         NULL, TEST_LOCAL_SOA_NO), USE_NAME_RESULT_AUTHORITATIVE);
    /* TEST_LOCAL_SOA_AUTO would test actual DNS on host, skip that. */
}
END_TEST

// Calls verify_name_allowed by first creating a memfile to read from.
static int verify_name_allowed_from_string(const char* name,
                                           const char* file_contents) {
    FILE* f = fmemopen((void*)file_contents, strlen(file_contents), "r");
    int result = verify_name_allowed(name, f);
    fclose(f);
    return result;
}

// Tests verify_name_allowed with empty config.
// Nothing is permitted.
START_TEST(test_verify_name_allowed_empty) {
    const char allow_file[] = "";

    ck_assert_int_eq(
        verify_name_allowed_from_string("example.local", allow_file),
        VERIFY_NAME_RESULT_NOT_ALLOWED);
    ck_assert_int_eq(
        verify_name_allowed_from_string("example.local.", allow_file),
        VERIFY_NAME_RESULT_NOT_ALLOWED);
    ck_assert_int_eq(
        verify_name_allowed_from_string("com.example.local", allow_file),
        VERIFY_NAME_RESULT_NOT_ALLOWED);
    ck_assert_int_eq(
        verify_name_allowed_from_string("com.example.local.", allow_file),
        VERIFY_NAME_RESULT_NOT_ALLOWED);
    ck_assert_int_eq(verify_name_allowed_from_string("example.com", allow_file),
                     VERIFY_NAME_RESULT_NOT_ALLOWED);
    ck_assert_int_eq(
        verify_name_allowed_from_string("example.com.", allow_file),
        VERIFY_NAME_RESULT_NOT_ALLOWED);
    ck_assert_int_eq(
        verify_name_allowed_from_string("example.local.com", allow_file),
        VERIFY_NAME_RESULT_NOT_ALLOWED);
    ck_assert_int_eq(
        verify_name_allowed_from_string("example.local.com.", allow_file),
        VERIFY_NAME_RESULT_NOT_ALLOWED);
    ck_assert_int_eq(verify_name_allowed_from_string("", allow_file),
                     VERIFY_NAME_RESULT_NOT_ALLOWED);
    ck_assert_int_eq(verify_name_allowed_from_string(".", allow_file),
                     VERIFY_NAME_RESULT_NOT_ALLOWED);
}
END_TEST

// Tests verify_name_allowed with the standard config.
// .local is unconditionally permitted, without SOA check.
// Multi-label names are allowed.
START_TEST(test_verify_name_allowed_default) {
    const char allow_file[] = "# /etc/mdns.allow\n"
                              ".local.\n"
                              ".local\n";

    ck_assert_int_eq(
        verify_name_allowed_from_string("example.local", allow_file),
        VERIFY_NAME_RESULT_ALLOWED);
    ck_assert_int_eq(
        verify_name_allowed_from_string("example.local.", allow_file),
        VERIFY_NAME_RESULT_ALLOWED);
    ck_assert_int_eq(
        verify_name_allowed_from_string("com.example.local", allow_file),
        VERIFY_NAME_RESULT_ALLOWED);
    ck_assert_int_eq(
        verify_name_allowed_from_string("com.example.local.", allow_file),
        VERIFY_NAME_RESULT_ALLOWED);
    ck_assert_int_eq(verify_name_allowed_from_string("example.com", allow_file),
                     VERIFY_NAME_RESULT_NOT_ALLOWED);
    ck_assert_int_eq(
        verify_name_allowed_from_string("example.com.", allow_file),
        VERIFY_NAME_RESULT_NOT_ALLOWED);
    ck_assert_int_eq(
        verify_name_allowed_from_string("example.local.com", allow_file),
        VERIFY_NAME_RESULT_NOT_ALLOWED);
    ck_assert_int_eq(
        verify_name_allowed_from_string("example.local.com.", allow_file),
        VERIFY_NAME_RESULT_NOT_ALLOWED);
    ck_assert_int_eq(verify_name_allowed_from_string("", allow_file),
                     VERIFY_NAME_RESULT_NOT_ALLOWED);
    ck_assert_int_eq(verify_name_allowed_from_string(".", allow_file),
                     VERIFY_NAME_RESULT_NOT_ALLOWED);
}
END_TEST

// Tests verify_name_allowed with wildcard.
// Everything is permitted, with no SOA check.
// Multi-label names are allowed.
START_TEST(test_verify_name_allowed_wildcard) {
    const char allow_file[] = "*\n";

    ck_assert_int_eq(
        verify_name_allowed_from_string("example.local", allow_file),
        VERIFY_NAME_RESULT_ALLOWED);
    ck_assert_int_eq(
        verify_name_allowed_from_string("example.local.", allow_file),
        VERIFY_NAME_RESULT_ALLOWED);
    ck_assert_int_eq(
        verify_name_allowed_from_string("com.example.local", allow_file),
        VERIFY_NAME_RESULT_ALLOWED);
    ck_assert_int_eq(
        verify_name_allowed_from_string("com.example.local.", allow_file),
        VERIFY_NAME_RESULT_ALLOWED);
    ck_assert_int_eq(verify_name_allowed_from_string("example.com", allow_file),
                     VERIFY_NAME_RESULT_ALLOWED);
    ck_assert_int_eq(
        verify_name_allowed_from_string("example.com.", allow_file),
        VERIFY_NAME_RESULT_ALLOWED);
    ck_assert_int_eq(
        verify_name_allowed_from_string("example.local.com", allow_file),
        VERIFY_NAME_RESULT_ALLOWED);
    ck_assert_int_eq(
        verify_name_allowed_from_string("example.local.com.", allow_file),
        VERIFY_NAME_RESULT_ALLOWED);
    ck_assert_int_eq(verify_name_allowed_from_string("", allow_file),
                     VERIFY_NAME_RESULT_ALLOWED);
    ck_assert_int_eq(verify_name_allowed_from_string(".", allow_file),
                     VERIFY_NAME_RESULT_ALLOWED);
}
END_TEST

// Tests verify_name_allowed with too-long lines.
START_TEST(test_verify_name_allowed_too_long) {
    const char allow_file[] =
        "# /etc/mdns.allow\n"
        ".local."
        "                                                  " // 50 spaces
        "                                                  " // 50 spaces
        "                                                  " // 50 spaces
        "\n"
        ".local\n";

    ck_assert_int_eq(
        verify_name_allowed_from_string("example.local", allow_file),
        VERIFY_NAME_RESULT_ALLOWED);
    ck_assert_int_eq(
        verify_name_allowed_from_string("example.local.", allow_file),
        VERIFY_NAME_RESULT_ALLOWED);
    ck_assert_int_eq(
        verify_name_allowed_from_string("com.example.local", allow_file),
        VERIFY_NAME_RESULT_ALLOWED);
    ck_assert_int_eq(
        verify_name_allowed_from_string("com.example.local.", allow_file),
        VERIFY_NAME_RESULT_ALLOWED);
    ck_assert_int_eq(verify_name_allowed_from_string("example.com", allow_file),
                     VERIFY_NAME_RESULT_NOT_ALLOWED);
    ck_assert_int_eq(
        verify_name_allowed_from_string("example.com.", allow_file),
        VERIFY_NAME_RESULT_NOT_ALLOWED);
    ck_assert_int_eq(
        verify_name_allowed_from_string("example.local.com", allow_file),
        VERIFY_NAME_RESULT_NOT_ALLOWED);
    ck_assert_int_eq(
        verify_name_allowed_from_string("example.local.com.", allow_file),
        VERIFY_NAME_RESULT_NOT_ALLOWED);
    ck_assert_int_eq(verify_name_allowed_from_string("", allow_file),
                     VERIFY_NAME_RESULT_NOT_ALLOWED);
    ck_assert_int_eq(verify_name_allowed_from_string(".", allow_file),
                     VERIFY_NAME_RESULT_NOT_ALLOWED);
}
END_TEST

// Tests verify_name_allowed with too-long non-empty lines.
START_TEST(test_verify_name_allowed_too_long2) {
    const char allow_file[] =
        "# /etc/mdns.allow\n"
        ".aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" // 50 characters
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" // 50 characters
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" // 50 characters
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" // 50 characters
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" // 50 characters
        "\n"
        ".local.\n"
        ".local\n";

    // The input is truncated at 127 bytes, so we allow this string.
    ck_assert_int_eq(
        verify_name_allowed_from_string(
            "example"
            ".aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" // 50
                                                                 // characters
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" // 50
                                                                 // characters
            "aaaaaaaaaaaaaaaaaaaaaaaaaaa", // 27 characters
            allow_file),
        VERIFY_NAME_RESULT_ALLOWED);

    // Even though this exactly matches the item in the allow file,
    // it is too long.
    ck_assert_int_eq(
        verify_name_allowed_from_string(
            "example"
            ".aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"  // 50
                                                                  // characters
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"  // 50
                                                                  // characters
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"  // 50
                                                                  // characters
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"  // 50
                                                                  // characters
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", // 50
                                                                  // characters
            allow_file),
        VERIFY_NAME_RESULT_NOT_ALLOWED);

    ck_assert_int_eq(
        verify_name_allowed_from_string("example.local", allow_file),
        VERIFY_NAME_RESULT_ALLOWED);
    ck_assert_int_eq(
        verify_name_allowed_from_string("example.local.", allow_file),
        VERIFY_NAME_RESULT_ALLOWED);
    ck_assert_int_eq(
        verify_name_allowed_from_string("com.example.local", allow_file),
        VERIFY_NAME_RESULT_ALLOWED);
    ck_assert_int_eq(
        verify_name_allowed_from_string("com.example.local.", allow_file),
        VERIFY_NAME_RESULT_ALLOWED);
    ck_assert_int_eq(verify_name_allowed_from_string("example.com", allow_file),
                     VERIFY_NAME_RESULT_NOT_ALLOWED);
    ck_assert_int_eq(
        verify_name_allowed_from_string("example.com.", allow_file),
        VERIFY_NAME_RESULT_NOT_ALLOWED);
    ck_assert_int_eq(
        verify_name_allowed_from_string("example.local.com", allow_file),
        VERIFY_NAME_RESULT_NOT_ALLOWED);
    ck_assert_int_eq(
        verify_name_allowed_from_string("example.local.com.", allow_file),
        VERIFY_NAME_RESULT_NOT_ALLOWED);
    ck_assert_int_eq(verify_name_allowed_from_string("", allow_file),
                     VERIFY_NAME_RESULT_NOT_ALLOWED);
    ck_assert_int_eq(verify_name_allowed_from_string(".", allow_file),
                     VERIFY_NAME_RESULT_NOT_ALLOWED);
}
END_TEST

// Tests verify_name_allowed with a custom config.
START_TEST(test_verify_name_allowed_com_and_local) {
    const char allow_file[] = "# /etc/mdns.allow\n"
                              ".com.\n"
                              ".com\n"
                              ".local.\n"
                              ".local\n";

    ck_assert_int_eq(
        verify_name_allowed_from_string("example.local", allow_file),
        VERIFY_NAME_RESULT_ALLOWED);
    ck_assert_int_eq(
        verify_name_allowed_from_string("example.local.", allow_file),
        VERIFY_NAME_RESULT_ALLOWED);
    ck_assert_int_eq(
        verify_name_allowed_from_string("com.example.local", allow_file),
        VERIFY_NAME_RESULT_ALLOWED);
    ck_assert_int_eq(
        verify_name_allowed_from_string("com.example.local.", allow_file),
        VERIFY_NAME_RESULT_ALLOWED);
    ck_assert_int_eq(verify_name_allowed_from_string("example.com", allow_file),
                     VERIFY_NAME_RESULT_ALLOWED);
    ck_assert_int_eq(
        verify_name_allowed_from_string("example.com.", allow_file),
        VERIFY_NAME_RESULT_ALLOWED);
    ck_assert_int_eq(
        verify_name_allowed_from_string("example.local.com", allow_file),
        VERIFY_NAME_RESULT_ALLOWED);
    ck_assert_int_eq(
        verify_name_allowed_from_string("example.local.com.", allow_file),
        VERIFY_NAME_RESULT_ALLOWED);
    ck_assert_int_eq(verify_name_allowed_from_string("example.net", allow_file),
                     VERIFY_NAME_RESULT_NOT_ALLOWED);
    ck_assert_int_eq(
        verify_name_allowed_from_string("example.net.", allow_file),
        VERIFY_NAME_RESULT_NOT_ALLOWED);
    ck_assert_int_eq(verify_name_allowed_from_string("", allow_file),
                     VERIFY_NAME_RESULT_NOT_ALLOWED);
    ck_assert_int_eq(verify_name_allowed_from_string(".", allow_file),
                     VERIFY_NAME_RESULT_NOT_ALLOWED);
}
END_TEST

// Tests ends_with.
START_TEST(test_ends_with) {
    ck_assert(ends_with("", ""));
    ck_assert(!ends_with("", " "));
    ck_assert(!ends_with("", "z"));
    ck_assert(ends_with("z", ""));
    ck_assert(ends_with("z", "z"));
    ck_assert(!ends_with("z", "zz"));
    ck_assert(ends_with("example.local", ".local"));
    ck_assert(ends_with("example.local.", ".local."));
    ck_assert(!ends_with("example.local.", ".local"));
    ck_assert(!ends_with("example.local.", ".local"));
}
END_TEST

// Tests label_count.
START_TEST(test_label_count) {
    ck_assert_int_eq(label_count(""), 1);
    ck_assert_int_eq(label_count("."), 1);
    ck_assert_int_eq(label_count("local"), 1);
    ck_assert_int_eq(label_count("local."), 1);
    ck_assert_int_eq(label_count("foo.local"), 2);
    ck_assert_int_eq(label_count("foo.local."), 2);
    ck_assert_int_eq(label_count("bar.foo.local"), 3);
    ck_assert_int_eq(label_count("bar.foo.local."), 3);
    ck_assert_int_eq(label_count("my-foo.local"), 2);
    ck_assert_int_eq(label_count("my-foo.local."), 2);
}
END_TEST

// Tests for buffer_t functions.

START_TEST(test_buffer_alloc_too_large_returns_null) {
    char *buffer = malloc(100);
    buffer_t buf;
    buffer_init(&buf, buffer, 100);

    ck_assert_ptr_null(buffer_alloc(&buf, 101));
    free(buffer);
}
END_TEST

START_TEST(test_buffer_alloc_just_right_returns_nonnull) {
    char *buffer = malloc(100);
    buffer_t buf;
    buffer_init(&buf, buffer, 100);

    ck_assert_ptr_nonnull(buffer_alloc(&buf, 100));
    free(buffer);
}
END_TEST

START_TEST(test_unaligned_buffer_alloc_returns_aligned) {
    char *buffer = malloc(1000);
    buffer_t buf;

    for (size_t i = 0; i < 32; i++) {
        buffer_init(&buf, buffer + i, 1000 - i);
        char* ptr = buffer_alloc(&buf, 10);
        ck_assert_uint_eq((uintptr_t)ptr % sizeof(void*), 0);
    }
    free(buffer);
}
END_TEST

START_TEST(test_buffer_alloc_returns_aligned) {
    char *buffer = malloc(1000);
    buffer_t buf;
    buffer_init(&buf, buffer, 1000);

    for (size_t i = 0; i < 32; i++) {
        char* ptr = buffer_alloc(&buf, i);
        ck_assert_ptr_nonnull(ptr);
        ck_assert_uint_eq((uintptr_t)ptr % sizeof(void*), 0);
    }
    free(buffer);
}
END_TEST

START_TEST(test_null_buffer_zero_alloc_returns_nonnull) {
    buffer_t buf;
    buffer_init(&buf, NULL, 0);

    ck_assert_ptr_nonnull(buffer_alloc(&buf, 0));
}
END_TEST

START_TEST(test_zero_buffer_zero_alloc_returns_nonnull) {
    char *buffer = malloc(1);
    buffer_t buf;
    buffer_init(&buf, buffer, 0);

    ck_assert_ptr_nonnull(buffer_alloc(&buf, 0));
    free(buffer);
}
END_TEST

START_TEST(test_nonzero_buffer_zero_alloc_returns_nonnull) {
    char *buffer = malloc(100);
    buffer_t buf;
    buffer_init(&buf, buffer, 100);

    ck_assert_ptr_nonnull(buffer_alloc(&buf, 0));
    free(buffer);
}
END_TEST

START_TEST(test_null_buffer_nonzero_alloc_returns_null) {
    buffer_t buf;
    buffer_init(&buf, NULL, 0);

    ck_assert_ptr_null(buffer_alloc(&buf, 1));
}
END_TEST

START_TEST(test_zero_buffer_nonzero_alloc_returns_null) {
    char *buffer = malloc(1);
    buffer_t buf;
    buffer_init(&buf, buffer, 0);

    ck_assert_ptr_null(buffer_alloc(&buf, 1));
    free(buffer);
}
END_TEST

START_TEST(test_buffer_tiny_alloc_returns_nonnull) {
    char *buffer = malloc(100);
    buffer_t buf;
    buffer_init(&buf, buffer, 100);

    ck_assert_ptr_nonnull(buffer_alloc(&buf, 1));
    free(buffer);
}
END_TEST

START_TEST(test_tiny_buffer_tiny_alloc_returns_nonnull) {
    char* buffer = malloc(1);
    buffer_t buf;
    buffer_init(&buf, buffer, 1);

    ck_assert_ptr_nonnull(buffer_alloc(&buf, 1));
    free(buffer);
}
END_TEST

START_TEST(test_tiny_unaligned_buffer_tiny_alloc_returns_null) {
    char* buffer = malloc(2);
    buffer_t buf;
    buffer_init(&buf, buffer + 1, 1);

    ck_assert_ptr_null(buffer_alloc(&buf, 1));
    free(buffer);
}
END_TEST

START_TEST(test_tiny_buffer_second_alloc_returns_null) {
    char* buffer = malloc(2);
    buffer_t buf;
    buffer_init(&buf, buffer, 2);

    ck_assert_ptr_nonnull(buffer_alloc(&buf, 1));
    ck_assert_ptr_null(buffer_alloc(&buf, 1)); // With alignment, out of room.
    free(buffer);
}
END_TEST

START_TEST(test_tiny_buffer_one_too_big_alloc_returns_null) {
    char* buffer = malloc(1); // Need to malloc to get pre-aligned buffer.
    buffer_t buf;
    buffer_init(&buf, buffer, 1);

    ck_assert_ptr_null(buffer_alloc(&buf, 2));
    free(buffer);
}
END_TEST

START_TEST(test_buffer_alloc_returns_zeroed_memory) {
    char *buffer = malloc(100);
    memset(buffer, 0xFF, 100);

    char zero[100];
    memset(zero, 0, sizeof(zero));

    buffer_t buf;
    buffer_init(&buf, buffer, 100);
    ck_assert_mem_eq(buffer_alloc(&buf, 50), zero, 50);
    free(buffer);
}
END_TEST

static const uint8_t ipv6_doc_prefix[16] = {
    0x0, 0x0, 0x0, 0x0, 0x0,  0x0,  0x0,  0x0,
    0x0, 0x0, 0x0, 0x0, 0xb8, 0x0d, 0x01, 0x20};   // Documentation prefix
static const uint32_t ipv4_test_addr = 0xc6336401; // 198.51.100.1 (TEST-NET-2)

static query_address_result_t create_address_result(int offset, int af) {
    query_address_result_t result;

    if (af == AF_UNSPEC) {
        // Alternate between IPv4 and IPv6.
        af = offset % 2 ? AF_INET : AF_INET6;
        result.af = af;
    }

    switch (af) {
    case AF_INET:
        result.address.ipv4.address = htonl(ipv4_test_addr + offset);
        break;
    case AF_INET6:
        memcpy(result.address.ipv6.address, ipv6_doc_prefix,
               sizeof ipv6_doc_prefix);
        result.address.ipv6.address[0] = offset;
        result.scopeid = (offset / 2) % 3;
        break;
    }
    return result;
}

#ifndef __FreeBSD__
static void validate_addrtuples(struct gaih_addrtuple* pat,
                                const char* expected_name, int expected_count) {
    int i = 0;
    while (pat != NULL) {
        ck_assert_str_eq(pat->name, expected_name);
        int expected_af = i % 2 ? AF_INET : AF_INET6;
        ck_assert_int_eq(pat->family, expected_af);

        uint32_t expected_ipv4 = htonl(ipv4_test_addr + i);
        uint8_t expected_ipv6[16];
        memcpy(expected_ipv6, ipv6_doc_prefix, sizeof ipv6_doc_prefix);
        expected_ipv6[0] = i;
        switch (expected_af) {
        case AF_INET:
            ck_assert_mem_eq(pat->addr, &expected_ipv4, sizeof expected_ipv4);
            break;
        case AF_INET6:
            ck_assert_mem_eq(pat->addr, expected_ipv6, sizeof expected_ipv6);
            ck_assert_int_eq(pat->scopeid, (i / 2) % 3);
            break;
        }

        i++;
        pat = pat->next;
    }
    ck_assert_int_eq(i, expected_count);
}
#endif

static userdata_t create_address_userdata(int num_addresses, int af) {
    ck_assert_int_le(num_addresses, MAX_ENTRIES);

    userdata_t u;
    u.count = 0;
    for (int i = 0; i < num_addresses; i++) {
        query_address_result_t result = create_address_result(i, af);
        append_address_to_userdata(&result, &u);
    }
    return u;
}

static void poison(char* buf, size_t buflen) { memset(buf, 0x55, buflen); }

static void validate_poison(char* buf, size_t buflen, size_t full_buflen) {
    size_t excess = full_buflen - buflen;
    char poison[excess];
    memset(poison, 0x55, sizeof(poison));
    ck_assert_mem_eq(buf + buflen, poison, excess);
}

static void validate_hostent(struct hostent* hostent, const char* name, int af,
                             int expected_count) {
    ck_assert_str_eq(name, hostent->h_name);
    ck_assert_ptr_nonnull(hostent->h_aliases);
    ck_assert_ptr_null(hostent->h_aliases[0]);
    ck_assert_int_eq(af, hostent->h_addrtype);
    ck_assert_int_eq(af == AF_INET ? sizeof(ipv4_address_t)
                                   : sizeof(ipv6_address_t),
                     hostent->h_length);
    ck_assert_ptr_nonnull(hostent->h_addr_list);

    int i = 0;
    char** addr = hostent->h_addr_list;
    while (*addr != NULL) {
        uint32_t expected_ipv4 = htonl(ipv4_test_addr + i);
        uint8_t expected_ipv6[16];
        memcpy(expected_ipv6, ipv6_doc_prefix, sizeof ipv6_doc_prefix);
        expected_ipv6[0] = i;
        switch (af) {
        case AF_INET:
            ck_assert_mem_eq(*addr, &expected_ipv4, sizeof expected_ipv4);
            break;
        case AF_INET6:
            ck_assert_mem_eq(*addr, expected_ipv6, sizeof expected_ipv6);
            break;
        }
        addr++;
        i++;
    }
    ck_assert_int_eq(expected_count, i);
}

// Tests for convert_userdata_to_addrtuple.

#ifndef __FreeBSD__
START_TEST(test_userdata_to_addrtuple_returns_tuples) {
    userdata_t u = create_address_userdata(16, AF_UNSPEC);
    struct gaih_addrtuple* pat = NULL;
    char buffer[2048];
    int errnop;
    int h_errnop;

    buffer_t buf;
    buffer_init(&buf, buffer, sizeof(buffer));
    enum nss_status status = convert_userdata_to_addrtuple(
        &u, "example.local", &pat, &buf, &errnop, &h_errnop);
    ck_assert_int_eq(status, NSS_STATUS_SUCCESS);
    validate_addrtuples(pat, "example.local", 16);
}
END_TEST

START_TEST(test_userdata_to_addrtuple_buffer_too_small_returns_erange) {
    userdata_t u = create_address_userdata(8, AF_UNSPEC);
    struct gaih_addrtuple* pat = NULL;
    char buffer[10];
    int errnop;
    int h_errnop;

    buffer_t buf;
    buffer_init(&buf, buffer, 0);
    enum nss_status status = convert_userdata_to_addrtuple(
        &u, "example.local", &pat, &buf, &errnop, &h_errnop);
    ck_assert_int_eq(errnop, ERANGE);
    ck_assert_int_eq(h_errnop, NO_RECOVERY);
    ck_assert_int_eq(status, NSS_STATUS_TRYAGAIN);
}
END_TEST

START_TEST(test_userdata_to_addrtuple_smallest_buffer_eventually_works) {
    userdata_t u = create_address_userdata(16, AF_UNSPEC);
    struct gaih_addrtuple* pat;
    char buffer[2048];
    int errnop;
    int h_errnop;

    enum nss_status status = 0;
    size_t buflen;
    for (buflen = 0; buflen < sizeof(buffer); buflen++) {
        poison(buffer, sizeof(buffer));
        errnop = h_errnop = 0;
        buffer_t buf;
        pat = NULL;
        buffer_init(&buf, buffer, buflen);
        status = convert_userdata_to_addrtuple(&u, "example.local", &pat, &buf,
                                               &errnop, &h_errnop);
        validate_poison(buffer, buflen, sizeof(buffer));
        if (errnop != ERANGE)
            break;
        if (h_errnop != NO_RECOVERY)
            break;
        if (status != NSS_STATUS_TRYAGAIN)
            break;
    }
    ck_assert_int_eq(status, NSS_STATUS_SUCCESS);
    validate_addrtuples(pat, "example.local", 16);
}
END_TEST

START_TEST(test_userdata_to_addrtuple_nonnull_pat_is_used) {
    userdata_t u = create_address_userdata(16, AF_UNSPEC);
    struct gaih_addrtuple tuple;
    struct gaih_addrtuple* pat = &tuple;
    char buffer[2048];
    int errnop;
    int h_errnop;

    memset(&tuple, 0, sizeof tuple);
    buffer_t buf;
    buffer_init(&buf, buffer, sizeof(buffer));
    enum nss_status status = convert_userdata_to_addrtuple(
        &u, "example.local", &pat, &buf, &errnop, &h_errnop);
    ck_assert_int_eq(status, NSS_STATUS_SUCCESS);
    validate_addrtuples(&tuple, "example.local", 16);
}
END_TEST
#endif

// Tests for convert_userdata_for_name_to_hostent.

START_TEST(test_userdata_for_name_to_hostent_returns_hostent_4) {
    userdata_t u = create_address_userdata(16, AF_INET);
    struct hostent result;
    char buffer[2048];
    int errnop;
    int h_errnop;

    buffer_t buf;
    buffer_init(&buf, buffer, sizeof(buffer));
    enum nss_status status = convert_userdata_for_name_to_hostent(
        &u, "example.local", AF_INET, &result, &buf, &errnop, &h_errnop);
    ck_assert_int_eq(status, NSS_STATUS_SUCCESS);
    validate_hostent(&result, "example.local", AF_INET, 16);
}
END_TEST

START_TEST(test_userdata_for_name_to_hostent_returns_hostent_6) {
    userdata_t u = create_address_userdata(16, AF_INET6);
    struct hostent result;
    char buffer[2048];
    int errnop;
    int h_errnop;

    buffer_t buf;
    buffer_init(&buf, buffer, sizeof(buffer));
    enum nss_status status = convert_userdata_for_name_to_hostent(
        &u, "example.local", AF_INET6, &result, &buf, &errnop, &h_errnop);
    ck_assert_int_eq(status, NSS_STATUS_SUCCESS);
    validate_hostent(&result, "example.local", AF_INET6, 16);
}
END_TEST

START_TEST(test_userdata_for_name_to_hostent_buffer_too_small_returns_erange) {
    userdata_t u = create_address_userdata(16, AF_INET);
    struct hostent result;
    char buffer[10];
    int errnop;
    int h_errnop;

    buffer_t buf;
    buffer_init(&buf, buffer, 0);
    enum nss_status status = convert_userdata_for_name_to_hostent(
        &u, "example.local", AF_INET, &result, &buf, &errnop, &h_errnop);
    ck_assert_int_eq(errnop, ERANGE);
    ck_assert_int_eq(h_errnop, NO_RECOVERY);
    ck_assert_int_eq(status, NSS_STATUS_TRYAGAIN);
}
END_TEST

START_TEST(
    test_userdata_for_name_to_hostent_smallest_buffer_eventually_works_4) {
    userdata_t u = create_address_userdata(16, AF_INET);
    struct hostent result;
    char buffer[2048];
    int errnop;
    int h_errnop;

    enum nss_status status = 0;
    size_t buflen;
    for (buflen = 0; buflen < sizeof(buffer); buflen++) {
        poison(buffer, sizeof(buffer));
        errnop = h_errnop = 0;

        buffer_t buf;
        buffer_init(&buf, buffer, buflen);
        status = convert_userdata_for_name_to_hostent(
            &u, "example.local", AF_INET, &result, &buf, &errnop, &h_errnop);
        validate_poison(buffer, buflen, sizeof(buffer));
        if (errnop != ERANGE)
            break;
        if (h_errnop != NO_RECOVERY)
            break;
        if (status != NSS_STATUS_TRYAGAIN)
            break;
    }
    ck_assert_int_eq(status, NSS_STATUS_SUCCESS);
    validate_hostent(&result, "example.local", AF_INET, 16);
}
END_TEST

START_TEST(
    test_userdata_for_name_to_hostent_smallest_buffer_eventually_works_6) {
    userdata_t u = create_address_userdata(16, AF_INET6);
    struct hostent result;
    char buffer[2048];
    int errnop;
    int h_errnop;

    enum nss_status status = 0;
    size_t buflen;
    for (buflen = 0; buflen < sizeof(buffer); buflen++) {
        poison(buffer, sizeof(buffer));
        errnop = h_errnop = 0;
        buffer_t buf;
        buffer_init(&buf, buffer, buflen);
        status = convert_userdata_for_name_to_hostent(
            &u, "example.local", AF_INET6, &result, &buf, &errnop, &h_errnop);
        validate_poison(buffer, buflen, sizeof(buffer));
        if (errnop != ERANGE)
            break;
        if (h_errnop != NO_RECOVERY)
            break;
        if (status != NSS_STATUS_TRYAGAIN)
            break;
    }
    ck_assert_int_eq(status, NSS_STATUS_SUCCESS);
    validate_hostent(&result, "example.local", AF_INET6, 16);
}
END_TEST

// Tests for convert_name_and_addr_to_hostent.

START_TEST(test_name_and_addr_to_hostent_returns_hostent_4) {
    struct hostent result;
    char buffer[2048];
    int errnop;
    int h_errnop;
    uint32_t ipv4 = htonl(ipv4_test_addr);

    buffer_t buf;
    buffer_init(&buf, buffer, sizeof(buffer));
    enum nss_status status = convert_name_and_addr_to_hostent(
        "example.local", &ipv4, sizeof ipv4, AF_INET, &result, &buf, &errnop,
        &h_errnop);
    ck_assert_int_eq(status, NSS_STATUS_SUCCESS);
    validate_hostent(&result, "example.local", AF_INET, 1);
}
END_TEST

START_TEST(test_name_and_addr_to_hostent_returns_hostent_6) {
    struct hostent result;
    char buffer[2048];
    int errnop;
    int h_errnop;

    buffer_t buf;
    buffer_init(&buf, buffer, sizeof(buffer));
    enum nss_status status = convert_name_and_addr_to_hostent(
        "example.local", &ipv6_doc_prefix, sizeof ipv6_doc_prefix, AF_INET6,
        &result, &buf, &errnop, &h_errnop);
    ck_assert_int_eq(status, NSS_STATUS_SUCCESS);
    validate_hostent(&result, "example.local", AF_INET6, 1);
}
END_TEST

START_TEST(test_name_and_addr_to_hostent_buffer_too_small_returns_erange) {
    struct hostent result;
    char buffer[10];
    int errnop;
    int h_errnop;
    uint32_t ipv4 = htonl(ipv4_test_addr);

    buffer_t buf;
    buffer_init(&buf, buffer, sizeof(buffer));
    enum nss_status status = convert_name_and_addr_to_hostent(
        "example.local", &ipv4, sizeof ipv4, AF_INET, &result, &buf, &errnop,
        &h_errnop);
    ck_assert_int_eq(errnop, ERANGE);
    ck_assert_int_eq(h_errnop, NO_RECOVERY);
    ck_assert_int_eq(status, NSS_STATUS_TRYAGAIN);
}
END_TEST

START_TEST(test_name_and_addr_to_hostent_smallest_buffer_eventually_works_4) {
    struct hostent result;
    char buffer[2048];
    int errnop;
    int h_errnop;
    uint32_t ipv4 = htonl(ipv4_test_addr);
    enum nss_status status = 0;
    size_t buflen;
    for (buflen = 0; buflen < sizeof(buffer); buflen++) {
        poison(buffer, sizeof(buffer));
        errnop = h_errnop = 0;
        buffer_t buf;
        buffer_init(&buf, buffer, buflen);
        status = convert_name_and_addr_to_hostent("example.local", &ipv4,
                                                  sizeof ipv4, AF_INET, &result,
                                                  &buf, &errnop, &h_errnop);
        validate_poison(buffer, buflen, sizeof(buffer));
        if (errnop != ERANGE)
            break;
        if (h_errnop != NO_RECOVERY)
            break;
        if (status != NSS_STATUS_TRYAGAIN)
            break;
    }
    ck_assert_int_eq(status, NSS_STATUS_SUCCESS);
    validate_hostent(&result, "example.local", AF_INET, 1);
}
END_TEST

START_TEST(test_name_and_addr_to_hostent_smallest_buffer_eventually_works_6) {
    struct hostent result;
    char buffer[2048];
    int errnop;
    int h_errnop;
    enum nss_status status = 0;
    size_t buflen;
    for (buflen = 0; buflen < sizeof(buffer); buflen++) {
        poison(buffer, sizeof(buffer));
        errnop = h_errnop = 0;
        buffer_t buf;
        buffer_init(&buf, buffer, buflen);
        status = convert_name_and_addr_to_hostent(
            "example.local", &ipv6_doc_prefix, sizeof ipv6_doc_prefix, AF_INET6,
            &result, &buf, &errnop, &h_errnop);
        validate_poison(buffer, buflen, sizeof(buffer));
        if (errnop != ERANGE)
            break;
        if (h_errnop != NO_RECOVERY)
            break;
        if (status != NSS_STATUS_TRYAGAIN)
            break;
    }
    ck_assert_int_eq(status, NSS_STATUS_SUCCESS);
    validate_hostent(&result, "example.local", AF_INET6, 1);
}
END_TEST

// Boilerplate from https://libcheck.github.io/check/doc/check_html/check_3.html
static Suite* util_suite(void) {
    Suite* s = suite_create("util");

    TCase* tc_verify_name = tcase_create("verify_name");
    tcase_add_test(tc_verify_name, test_verify_name_allowed_minimal);
    tcase_add_test(tc_verify_name, test_verify_name_allowed_default);
    tcase_add_test(tc_verify_name, test_verify_name_allowed_empty);
    tcase_add_test(tc_verify_name, test_verify_name_allowed_wildcard);
    tcase_add_test(tc_verify_name, test_verify_name_allowed_too_long);
    tcase_add_test(tc_verify_name, test_verify_name_allowed_too_long2);
    tcase_add_test(tc_verify_name, test_verify_name_allowed_com_and_local);
    suite_add_tcase(s, tc_verify_name);

    TCase* tc_ends_with = tcase_create("ends_with");
    tcase_add_test(tc_ends_with, test_ends_with);
    suite_add_tcase(s, tc_ends_with);

    TCase* tc_label_count = tcase_create("label_count");
    tcase_add_test(tc_label_count, test_label_count);
    suite_add_tcase(s, tc_label_count);

    TCase* tc_buffer = tcase_create("buffer");
    tcase_add_test(tc_buffer, test_buffer_alloc_too_large_returns_null);
    tcase_add_test(tc_buffer, test_buffer_alloc_just_right_returns_nonnull);
    tcase_add_test(tc_buffer, test_unaligned_buffer_alloc_returns_aligned);
    tcase_add_test(tc_buffer, test_buffer_alloc_returns_aligned);
    tcase_add_test(tc_buffer, test_null_buffer_zero_alloc_returns_nonnull);
    tcase_add_test(tc_buffer, test_zero_buffer_zero_alloc_returns_nonnull);
    tcase_add_test(tc_buffer, test_nonzero_buffer_zero_alloc_returns_nonnull);
    tcase_add_test(tc_buffer, test_null_buffer_nonzero_alloc_returns_null);
    tcase_add_test(tc_buffer, test_zero_buffer_nonzero_alloc_returns_null);
    tcase_add_test(tc_buffer, test_buffer_tiny_alloc_returns_nonnull);
    tcase_add_test(tc_buffer, test_tiny_buffer_tiny_alloc_returns_nonnull);
    tcase_add_test(tc_buffer,
                   test_tiny_unaligned_buffer_tiny_alloc_returns_null);
    tcase_add_test(tc_buffer, test_tiny_buffer_second_alloc_returns_null);
    tcase_add_test(tc_buffer, test_tiny_buffer_one_too_big_alloc_returns_null);
    tcase_add_test(tc_buffer, test_buffer_alloc_returns_zeroed_memory);
    suite_add_tcase(s, tc_buffer);

#ifndef __FreeBSD__
    TCase* tc_userdata_to_addrtuple = tcase_create("userdata_to_addrtuple");
    tcase_add_test(tc_userdata_to_addrtuple,
                   test_userdata_to_addrtuple_returns_tuples);
    tcase_add_test(tc_userdata_to_addrtuple,
                   test_userdata_to_addrtuple_buffer_too_small_returns_erange);
    tcase_add_test(tc_userdata_to_addrtuple,
                   test_userdata_to_addrtuple_smallest_buffer_eventually_works);
    tcase_add_test(tc_userdata_to_addrtuple,
                   test_userdata_to_addrtuple_nonnull_pat_is_used);
    suite_add_tcase(s, tc_userdata_to_addrtuple);
#endif

    TCase* tc_userdata_for_name_to_hostent =
        tcase_create("userdata_for_name_to_hostent");
    tcase_add_test(tc_userdata_for_name_to_hostent,
                   test_userdata_for_name_to_hostent_returns_hostent_4);
    tcase_add_test(tc_userdata_for_name_to_hostent,
                   test_userdata_for_name_to_hostent_returns_hostent_6);
    tcase_add_test(
        tc_userdata_for_name_to_hostent,
        test_userdata_for_name_to_hostent_buffer_too_small_returns_erange);
    tcase_add_test(
        tc_userdata_for_name_to_hostent,
        test_userdata_for_name_to_hostent_smallest_buffer_eventually_works_4);
    tcase_add_test(
        tc_userdata_for_name_to_hostent,
        test_userdata_for_name_to_hostent_smallest_buffer_eventually_works_6);
    suite_add_tcase(s, tc_userdata_for_name_to_hostent);

    TCase* tc_name_and_addr_to_hostent =
        tcase_create("name_and_addr_to_hostent");
    tcase_add_test(tc_name_and_addr_to_hostent,
                   test_name_and_addr_to_hostent_returns_hostent_4);
    tcase_add_test(tc_name_and_addr_to_hostent,
                   test_name_and_addr_to_hostent_returns_hostent_6);
    tcase_add_test(
        tc_name_and_addr_to_hostent,
        test_name_and_addr_to_hostent_buffer_too_small_returns_erange);
    tcase_add_test(
        tc_name_and_addr_to_hostent,
        test_name_and_addr_to_hostent_smallest_buffer_eventually_works_4);
    tcase_add_test(
        tc_name_and_addr_to_hostent,
        test_name_and_addr_to_hostent_smallest_buffer_eventually_works_6);
    suite_add_tcase(s, tc_name_and_addr_to_hostent);

    return s;
}

int main(void) {
    int number_failed;
    Suite* s;
    SRunner* sr;

    s = util_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_VERBOSE);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
