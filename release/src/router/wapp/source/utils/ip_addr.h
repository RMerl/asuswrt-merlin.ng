/*
 * IP address processing
 * Copyright (c) 2003-2006, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef IP_ADDR_H
#define IP_ADDR_H

const char * wapp_ip_txt(const struct wapp_ip_addr *addr, char *buf,
			    size_t buflen);
int wapp_parse_ip_addr(const char *txt, struct wapp_ip_addr *addr);

#endif /* IP_ADDR_H */
