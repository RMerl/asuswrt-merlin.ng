/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-tld-private.h:
 *
 * Copyright (C) 2012 Igalia, S.L.
 */

#ifndef SOUP_TLD_PRIVATE_H
#define SOUP_TLD_PRIVATE_H 1

G_BEGIN_DECLS

typedef enum {
  SOUP_TLD_RULE_NORMAL,
  SOUP_TLD_RULE_MATCH_ALL = 1 << 0,
  SOUP_TLD_RULE_EXCEPTION = 1 << 1,
} SoupTLDRuleFlags;

typedef struct {
	char *domain;
	guint flags;
} SoupTLDEntry;

G_END_DECLS

#endif /* SOUP_TLD_PRIVATE_H */
