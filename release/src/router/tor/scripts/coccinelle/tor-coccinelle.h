/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/*
 * This file looks like a C header, but its purpose is a bit different.
 *
 * We never include it from our real C files; we only tell Coccinelle
 * about it in apply.sh.
 *
 * It tells the Coccinelle semantic patching tool how to understand
 * things that would otherwise not be good C syntax, or which would
 * otherwise not make sense to it as C.  It doesn't need to produce
 * semantically equivalent C, or even correct C: it only has to produce
 * syntactically valid C.
 */

#define MOCK_DECL(a, b, c) a b c
#define MOCK_IMPL(a, b, c) a b c
#define CHECK_PRINTF(a, b)
#define CHECK_SCANF(a, b)
#define STATIC static
#define EXTERN(a,b) extern a b;

#define STMT_BEGIN do {
#define STMT_END } while (0)

#define BUG(x) (x)
#define IF_BUG_ONCE(x) if (x)

#define ATTR_NORETURN
#define ATTR_UNUSED
#define ATTR_CONST
#define ATTR_MALLOC
#define ATTR_WUR
#define DISABLE_GCC_WARNING(x)
#define ENABLE_GCC_WARNING(x)

#define HANDLE_DECL(a,b,c)
#define HANDLE_IMPL(a,b,c)
#define HT_ENTRY(x) void *
#define HT_HEAD(a,b) struct ht_head
#define HT_INITIALIZER() { }
#define X509 struct x509_st
#define STACK_OF(x) struct foo_stack_t
#define TOR_TAILQ_HEAD(a,b) struct tailq_head
#define TOR_TAILQ_ENTRY(a) struct tailq_entry
#define TOR_SIMPLEQ_HEAD(a,b) struct simpleq_entry
#define TOR_SIMPLEQ_ENTRY(a) struct simpleq_entry
#define TOR_LIST_HEAD(a,b) struct list_head
#define TOR_LIST_ENTRY(a) struct list_entry
#define TOR_SLIST_HEAD(a,b) struct slist_head
#define TOR_SLIST_ENTRY(a) struct slist_entry

#define NS_DECL(a, b, c) a b c
#define NS(a) a

#define CONF_TEST_MEMBERS(a,b,c)
#define DUMMY_CONF_TEST_MEMBERS

#define EAT_SEMICOLON extern int dummy__;
