/*
 * lib/version.c	Run-time version information
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2012 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup core
 * @defgroup utils Utilities
 *
 * Run-time version information
 *
 * @{
 */


/**
 * @name Run-time version information
 * @{
 */

#include <netlink/version.h>

const int      nl_ver_num = LIBNL_VER_NUM;
const int      nl_ver_maj = LIBNL_VER_MAJ;
const int      nl_ver_min = LIBNL_VER_MIN;
const int      nl_ver_mic = LIBNL_VER_MIC;

/** @} */

/** @} */
