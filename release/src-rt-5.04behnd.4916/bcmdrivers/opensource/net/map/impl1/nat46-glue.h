/*
 * Glue headers, not much here.
 *
 * Copyright (c) 2013-2014 Andrew Yourtchenko <ayourtch@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <net/ip.h>
#include <net/ipv6.h>
#include <linux/icmp.h>
#include <linux/skbuff.h>
#include <net/ip6_route.h>
#include <linux/inet.h>
#include <net/ip6_checksum.h>
#include "nat46-netdev.h"


#ifndef IP6_OFFSET
#define IP6_OFFSET      0xFFF8
#endif

#define assert(x) printk("Assertion failed: %s", #x)

