/* SPDX-License-Identifier: Intel */
/*
 * Access to binman information at runtime
 *
 * Copyright 2019 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef _BINMAN_H_
#define _BINMAN_H_

#include <dm/ofnode.h>

/**
 *struct binman_entry - information about a binman entry
 *
 * @image_pos: Position of entry in the image
 * @size: Size of entry
 */
struct binman_entry {
	u32 image_pos;
	u32 size;
};

/**
 * binman_entry_map() - Look up the address of an entry in memory
 *
 * @parent: Parent binman node
 * @name: Name of entry
 * @bufp: Returns a pointer to the entry
 * @sizep: Returns the size of the entry
 * Return: 0 on success, -EPERM if the ROM offset is not set, -ENOENT if the
 *	entry cannot be found, other error code other error
 */
int binman_entry_map(ofnode parent, const char *name, void **bufp, int *sizep);

/**
 * binman_set_rom_offset() - Set the ROM memory-map offset
 *
 * @rom_offset: Offset from an image_pos to the memory-mapped address. This
 *	tells binman that ROM image_pos x can be addressed at rom_offset + x
 */
void binman_set_rom_offset(int rom_offset);

/**
 * binman_get_rom_offset() - Get the ROM memory-map offset
 *
 * @returns offset from an image_pos to the memory-mapped address
 */
int binman_get_rom_offset(void);

/**
 * binman_entry_find() - Find a binman symbol
 *
 * This searches the binman information in the device tree for a symbol of the
 * given name
 *
 * @name: Path to entry to examine (e.g. "/read-only/u-boot")
 * @entry: Returns information about the entry
 * Return: 0 if OK, -ENOENT if the path is not found, other -ve value if the
 *	binman information is invalid (missing image-pos or size)
 */
int binman_entry_find(const char *name, struct binman_entry *entry);

/**
 * binman_section_find_node() - Find a binman node
 *
 * @name: Name of node to look for
 * Return: Node that was found, ofnode_null() if not found
 */
ofnode binman_section_find_node(const char *name);

/**
 * binman_select_subnode() - Select a subnode to use to find entries
 *
 * Normally binman selects the top-level node for future entry requests, such as
 * binman_entry_find(). This function allows a subnode to be chosen instead.
 *
 * @name: Name of subnode, typically a section. This must be in the top-level
 *	binman node
 * Return: 0 if OK, -EINVAL if there is no /binman node, -ECHILD if multiple
 *	images are being used but the first image is not available, -ENOENT if
 *	the requested subnode cannot be found
 */
int binman_select_subnode(const char *name);

/**
 * binman_init() - Set up the binman symbol information
 *
 * This locates the binary symbol information in the device tree ready for use
 *
 * Return: 0 if OK, -ENOMEM if out of memory, -EINVAL if there is no binman node
 */
int binman_init(void);

#endif
