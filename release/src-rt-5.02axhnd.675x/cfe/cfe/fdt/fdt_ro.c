/*
 * libfdt - Flat Device Tree manipulation
 * Copyright (C) 2006 David Gibson, IBM Corporation.
 *
 * libfdt is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 *
 *  a) This library is free software; you can redistribute it and/or
 *     modify it under the terms of the GNU General Public License as
 *     published by the Free Software Foundation; either version 2 of the
 *     License, or (at your option) any later version.
 *
 *     This library is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public
 *     License along with this library; if not, write to the Free
 *     Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 *     MA 02110-1301 USA
 *
 * Alternatively,
 *
 *  b) Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *     1. Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *     2. Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 *     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 *     CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 *     INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *     MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *     DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *     CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *     SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *     NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *     LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *     HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *     CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *     OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 *     EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "libfdt_env.h"

#include <fdt.h>
#include <libfdt.h>

#include "libfdt_internal.h"

static int _fdt_nodename_eq(const void *fdt, int offset,
			    const char *s, int len)
{
	const char *p = fdt_offset_ptr(fdt, offset + FDT_TAGSIZE, len+1);

	if (! p)
		/* short match */
		return 0;

	if (memcmp(p, s, len) != 0)
		return 0;

	if (p[len] == '\0')
		return 1;
	else if (!memchr(s, '@', len) && (p[len] == '@'))
		return 1;
	else
		return 0;
}

const char *fdt_string(const void *fdt, int stroffset)
{
	return (const char *)fdt + fdt_off_dt_strings(fdt) + stroffset;
}

static int _fdt_string_eq(const void *fdt, int stroffset,
			  const char *s, int len)
{
	const char *p = fdt_string(fdt, stroffset);

	return (strlen(p) == len) && (memcmp(p, s, len) == 0);
}

int fdt_get_mem_rsv(const void *fdt, int n, uint64_t *address, uint64_t *size)
{
	FDT_CHECK_HEADER(fdt);
	*address = fdt64_to_cpu(_fdt_mem_rsv(fdt, n)->address);
	*size = fdt64_to_cpu(_fdt_mem_rsv(fdt, n)->size);
	return 0;
}

int fdt_num_mem_rsv(const void *fdt)
{
	int i = 0;

	while (fdt64_to_cpu(_fdt_mem_rsv(fdt, i)->size) != 0)
		i++;
	return i;
}

static int _nextprop(const void *fdt, int offset)
{
	uint32_t tag;
	int nextoffset;

	do {
		tag = fdt_next_tag(fdt, offset, &nextoffset);

		switch (tag) {
		case FDT_END:
			if (nextoffset >= 0)
				return -FDT_ERR_BADSTRUCTURE;
			else
				return nextoffset;

		case FDT_PROP:
			return offset;
		}
		offset = nextoffset;
	} while (tag == FDT_NOP);

	return -FDT_ERR_NOTFOUND;
}

int fdt_subnode_offset_namelen(const void *fdt, int offset,
			       const char *name, int namelen)
{
	int depth;

	FDT_CHECK_HEADER(fdt);

	for (depth = 0;
	     (offset >= 0) && (depth >= 0);
	     offset = fdt_next_node(fdt, offset, &depth))
		if ((depth == 1)
		    && _fdt_nodename_eq(fdt, offset, name, namelen))
			return offset;

	if (depth < 0)
		return -FDT_ERR_NOTFOUND;
	return offset; /* error */
}

int fdt_subnode_offset(const void *fdt, int parentoffset,
		       const char *name)
{
	return fdt_subnode_offset_namelen(fdt, parentoffset, name, strlen(name));
}

int fdt_path_offset(const void *fdt, const char *path)
{
	const char *end = path + strlen(path);
	const char *p = path;
	int offset = 0;

	FDT_CHECK_HEADER(fdt);

	/* see if we have an alias */
	if (*path != '/') {
		const char *q = strchr(path, '/');

		if (!q)
			q = end;

		p = fdt_get_alias_namelen(fdt, p, q - p);
		if (!p)
			return -FDT_ERR_BADPATH;
		offset = fdt_path_offset(fdt, p);

		p = q;
	}

	while (*p) {
		const char *q;

		while (*p == '/')
			p++;
		if (! *p)
			return offset;
		q = strchr(p, '/');
		if (! q)
			q = end;

		offset = fdt_subnode_offset_namelen(fdt, offset, p, q-p);
		if (offset < 0)
			return offset;

		p = q;
	}

	return offset;
}

const char *fdt_get_name(const void *fdt, int nodeoffset, int *len)
{
	const struct fdt_node_header *nh = _fdt_offset_ptr(fdt, nodeoffset);
	int err;

	if (((err = fdt_check_header(fdt)) != 0)
	    || ((err = _fdt_check_node_offset(fdt, nodeoffset)) < 0))
			goto fail;

	if (len)
		*len = strlen(nh->name);

	return nh->name;

 fail:
	if (len)
		*len = err;
	return NULL;
}

int fdt_first_property_offset(const void *fdt, int nodeoffset)
{
	int offset;

	if ((offset = _fdt_check_node_offset(fdt, nodeoffset)) < 0)
		return offset;

	return _nextprop(fdt, offset);
}

int fdt_next_property_offset(const void *fdt, int offset)
{
	if ((offset = _fdt_check_prop_offset(fdt, offset)) < 0)
		return offset;

	return _nextprop(fdt, offset);
}

const struct fdt_property *fdt_get_property_by_offset(const void *fdt,
						      int offset,
						      int *lenp)
{
	int err;
	const struct fdt_property *prop;

	if ((err = _fdt_check_prop_offset(fdt, offset)) < 0) {
		if (lenp)
			*lenp = err;
		return NULL;
	}

	prop = _fdt_offset_ptr(fdt, offset);

	if (lenp)
		*lenp = fdt32_to_cpu(prop->len);

	return prop;
}

const struct fdt_property *fdt_get_property_namelen(const void *fdt,
						    int offset,
						    const char *name,
						    int namelen, int *lenp)
{
	for (offset = fdt_first_property_offset(fdt, offset);
	     (offset >= 0);
	     (offset = fdt_next_property_offset(fdt, offset))) {
		const struct fdt_property *prop;

		if (!(prop = fdt_get_property_by_offset(fdt, offset, lenp))) {
			offset = -FDT_ERR_INTERNAL;
			break;
		}
		if (_fdt_string_eq(fdt, fdt32_to_cpu(prop->nameoff),
				   name, namelen))
			return prop;
	}

	if (lenp)
		*lenp = offset;
	return NULL;
}

const struct fdt_property *fdt_get_property(const void *fdt,
					    int nodeoffset,
					    const char *name, int *lenp)
{
	return fdt_get_property_namelen(fdt, nodeoffset, name,
					strlen(name), lenp);
}

const void *fdt_getprop_namelen(const void *fdt, int nodeoffset,
				const char *name, int namelen, int *lenp)
{
	const struct fdt_property *prop;

	prop = fdt_get_property_namelen(fdt, nodeoffset, name, namelen, lenp);
	if (! prop)
		return NULL;

	return prop->data;
}

const void *fdt_getprop_by_offset(const void *fdt, int offset,
				  const char **namep, int *lenp)
{
	const struct fdt_property *prop;

	prop = fdt_get_property_by_offset(fdt, offset, lenp);
	if (!prop)
		return NULL;
	if (namep)
		*namep = fdt_string(fdt, fdt32_to_cpu(prop->nameoff));
	return prop->data;
}

const void *fdt_getprop(const void *fdt, int nodeoffset,
			const char *name, int *lenp)
{
	return fdt_getprop_namelen(fdt, nodeoffset, name, strlen(name), lenp);
}

uint32_t fdt_get_phandle(const void *fdt, int nodeoffset)
{
	const uint32_t *php;
	int len;

	/* FIXME: This is a bit sub-optimal, since we potentially scan
	 * over all the properties twice. */
	php = fdt_getprop(fdt, nodeoffset, "phandle", &len);
	if (!php || (len != sizeof(*php))) {
		php = fdt_getprop(fdt, nodeoffset, "linux,phandle", &len);
		if (!php || (len != sizeof(*php)))
			return 0;
	}

	return fdt32_to_cpu(*php);
}

const char *fdt_get_alias_namelen(const void *fdt,
				  const char *name, int namelen)
{
	int aliasoffset;

	aliasoffset = fdt_path_offset(fdt, "/aliases");
	if (aliasoffset < 0)
		return NULL;

	return fdt_getprop_namelen(fdt, aliasoffset, name, namelen, NULL);
}

const char *fdt_get_alias(const void *fdt, const char *name)
{
	return fdt_get_alias_namelen(fdt, name, strlen(name));
}

int fdt_get_path(const void *fdt, int nodeoffset, char *buf, int buflen)
{
	int pdepth = 0, p = 0;
	int offset, depth, namelen;
	const char *name;

	FDT_CHECK_HEADER(fdt);

	if (buflen < 2)
		return -FDT_ERR_NOSPACE;

	for (offset = 0, depth = 0;
	     (offset >= 0) && (offset <= nodeoffset);
	     offset = fdt_next_node(fdt, offset, &depth)) {
		while (pdepth > depth) {
			do {
				p--;
			} while (buf[p-1] != '/');
			pdepth--;
		}

		if (pdepth >= depth) {
			name = fdt_get_name(fdt, offset, &namelen);
			if (!name)
				return namelen;
			if ((p + namelen + 1) <= buflen) {
				memcpy(buf + p, name, namelen);
				p += namelen;
				buf[p++] = '/';
				pdepth++;
			}
		}

		if (offset == nodeoffset) {
			if (pdepth < (depth + 1))
				return -FDT_ERR_NOSPACE;

			if (p > 1) /* special case so that root path is "/", not "" */
				p--;
			buf[p] = '\0';
			return 0;
		}
	}

	if ((offset == -FDT_ERR_NOTFOUND) || (offset >= 0))
		return -FDT_ERR_BADOFFSET;
	else if (offset == -FDT_ERR_BADOFFSET)
		return -FDT_ERR_BADSTRUCTURE;

	return offset; /* error from fdt_next_node() */
}

int fdt_supernode_atdepth_offset(const void *fdt, int nodeoffset,
				 int supernodedepth, int *nodedepth)
{
	int offset, depth;
	int supernodeoffset = -FDT_ERR_INTERNAL;

	FDT_CHECK_HEADER(fdt);

	if (supernodedepth < 0)
		return -FDT_ERR_NOTFOUND;

	for (offset = 0, depth = 0;
	     (offset >= 0) && (offset <= nodeoffset);
	     offset = fdt_next_node(fdt, offset, &depth)) {
		if (depth == supernodedepth)
			supernodeoffset = offset;

		if (offset == nodeoffset) {
			if (nodedepth)
				*nodedepth = depth;

			if (supernodedepth > depth)
				return -FDT_ERR_NOTFOUND;
			else
				return supernodeoffset;
		}
	}

	if ((offset == -FDT_ERR_NOTFOUND) || (offset >= 0))
		return -FDT_ERR_BADOFFSET;
	else if (offset == -FDT_ERR_BADOFFSET)
		return -FDT_ERR_BADSTRUCTURE;

	return offset; /* error from fdt_next_node() */
}

int fdt_node_depth(const void *fdt, int nodeoffset)
{
	int nodedepth;
	int err;

	err = fdt_supernode_atdepth_offset(fdt, nodeoffset, 0, &nodedepth);
	if (err)
		return (err < 0) ? err : -FDT_ERR_INTERNAL;
	return nodedepth;
}

int fdt_parent_offset(const void *fdt, int nodeoffset)
{
	int nodedepth = fdt_node_depth(fdt, nodeoffset);

	if (nodedepth < 0)
		return nodedepth;
	return fdt_supernode_atdepth_offset(fdt, nodeoffset,
					    nodedepth - 1, NULL);
}

int fdt_node_offset_by_prop_value(const void *fdt, int startoffset,
				  const char *propname,
				  const void *propval, int proplen)
{
	int offset;
	const void *val;
	int len;

	FDT_CHECK_HEADER(fdt);

	/* FIXME: The algorithm here is pretty horrible: we scan each
	 * property of a node in fdt_getprop(), then if that didn't
	 * find what we want, we scan over them again making our way
	 * to the next node.  Still it's the easiest to implement
	 * approach; performance can come later. */
	for (offset = fdt_next_node(fdt, startoffset, NULL);
	     offset >= 0;
	     offset = fdt_next_node(fdt, offset, NULL)) {
		val = fdt_getprop(fdt, offset, propname, &len);
		if (val && (len == proplen)
		    && (memcmp(val, propval, len) == 0))
			return offset;
	}

	return offset; /* error from fdt_next_node() */
}

int fdt_node_offset_by_phandle(const void *fdt, uint32_t phandle)
{
	int offset;

	if ((phandle == 0) || (phandle == -1))
		return -FDT_ERR_BADPHANDLE;

	FDT_CHECK_HEADER(fdt);

	/* FIXME: The algorithm here is pretty horrible: we
	 * potentially scan each property of a node in
	 * fdt_get_phandle(), then if that didn't find what
	 * we want, we scan over them again making our way to the next
	 * node.  Still it's the easiest to implement approach;
	 * performance can come later. */
	for (offset = fdt_next_node(fdt, -1, NULL);
	     offset >= 0;
	     offset = fdt_next_node(fdt, offset, NULL)) {
		if (fdt_get_phandle(fdt, offset) == phandle)
			return offset;
	}

	return offset; /* error from fdt_next_node() */
}

static int _fdt_stringlist_contains(const char *strlist, int listlen,
				    const char *str)
{
	int len = strlen(str);
	const char *p;

	while (listlen >= len) {
		if (memcmp(str, strlist, len+1) == 0)
			return 1;
		p = memchr(strlist, '\0', listlen);
		if (!p)
			return 0; /* malformed strlist.. */
		listlen -= (p-strlist) + 1;
		strlist = p + 1;
	}
	return 0;
}

int fdt_node_check_compatible(const void *fdt, int nodeoffset,
			      const char *compatible)
{
	const void *prop;
	int len;

	prop = fdt_getprop(fdt, nodeoffset, "compatible", &len);
	if (!prop)
		return len;
	if (_fdt_stringlist_contains(prop, len, compatible))
		return 0;
	else
		return 1;
}

int fdt_node_offset_by_compatible(const void *fdt, int startoffset,
				  const char *compatible)
{
	int offset, err;

	FDT_CHECK_HEADER(fdt);

	/* FIXME: The algorithm here is pretty horrible: we scan each
	 * property of a node in fdt_node_check_compatible(), then if
	 * that didn't find what we want, we scan over them again
	 * making our way to the next node.  Still it's the easiest to
	 * implement approach; performance can come later. */
	for (offset = fdt_next_node(fdt, startoffset, NULL);
	     offset >= 0;
	     offset = fdt_next_node(fdt, offset, NULL)) {
		err = fdt_node_check_compatible(fdt, offset, compatible);
		if ((err < 0) && (err != -FDT_ERR_NOTFOUND))
			return err;
		else if (err == 0)
			return offset;
	}

	return offset; /* error from fdt_next_node() */
}
