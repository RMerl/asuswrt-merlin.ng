/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file recommend_pkg.h
 * \brief Header file for recommend_pkg.c
 **/

#ifndef TOR_RECOMMEND_PKG_H
#define TOR_RECOMMEND_PKG_H

int validate_recommended_package_line(const char *line);

#endif
