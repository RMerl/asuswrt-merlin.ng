/* SPDX-License-Identifier: MIT
 *
 * The sart code is copied from m1n1 (https://github.com/AsahiLinux/m1n1) and
 * licensed as MIT.
 *
 * (C) Copyright 2022 The Asahi Linux Contributors
 */

#ifndef SART_H
#define SART_H

#include <dm/ofnode.h>

struct apple_sart;

struct apple_sart *sart_init(ofnode node);
void sart_free(struct apple_sart *sart);

bool sart_add_allowed_region(struct apple_sart *sart, void *paddr, size_t sz);
bool sart_remove_allowed_region(struct apple_sart *sart, void *paddr, size_t sz);

#endif
