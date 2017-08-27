/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */
#ifndef MAP_H
#define MAP_H
/*
 * $Id$
 *
 * @file map.h
 * @brief Structures and prototypes for templates / maps
 *
 * @copyright 2013  The FreeRADIUS server project
 */

RCSIDH(map_h, "$Id$")

#include <freeradius-devel/conffile.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum pair_lists {
	PAIR_LIST_UNKNOWN = 0,
	PAIR_LIST_REQUEST,
	PAIR_LIST_REPLY,
	PAIR_LIST_CONTROL,
#ifdef WITH_PROXY
	PAIR_LIST_PROXY_REQUEST,
	PAIR_LIST_PROXY_REPLY,
#endif
#ifdef WITH_COA
	PAIR_LIST_COA,
	PAIR_LIST_COA_REPLY,
	PAIR_LIST_DM,
	PAIR_LIST_DM_REPLY
#endif
} pair_lists_t;

extern const FR_NAME_NUMBER pair_lists[];

typedef enum requests {
	REQUEST_UNKNOWN = 0,
	REQUEST_OUTER,
	REQUEST_CURRENT,
	REQUEST_PARENT	/* For future use */
} request_refs_t;

extern const FR_NAME_NUMBER request_refs[];

typedef struct pair_list {
	char const		*name;
	VALUE_PAIR		*check;
	VALUE_PAIR		*reply;
	int			lineno;
	int			order;
	struct pair_list	*next;
	struct pair_list	*lastdefault;
} PAIR_LIST;

typedef enum vpt_type {
	VPT_TYPE_UNKNOWN = 0,
	VPT_TYPE_LITERAL,		//!< Is a literal string.
	VPT_TYPE_XLAT,			//!< Needs to be expanded.
	VPT_TYPE_ATTR,			//!< Is a dictionary attribute.
	VPT_TYPE_LIST,			//!< Is a list.
	VPT_TYPE_REGEX,			//!< Is a regex.
	VPT_TYPE_EXEC,			//!< Needs to be executed.
	VPT_TYPE_DATA			//!< is a value_data_t
} vpt_type_t;

extern const FR_NAME_NUMBER vpt_types[];

/** A pre-parsed template attribute
 *
 *  Value pair template, used when processing various mappings sections
 *  to create a real valuepair later.
 *
 * @see value_pair_map_t
 */
typedef struct value_pair_tmpl_t {
	vpt_type_t		type;	 //!< What type of value tmpl refers to.
	char const		*name;   //!< Original attribute ref string, or
					 //!< where this refers to a none FR
					 //!< attribute, just the string id for
					 //!< the attribute.

	request_refs_t		request; //!< Request to search or insert in.
	pair_lists_t		list;	 //!< List to search or insert in.

	DICT_ATTR const		*da;	 //!< Resolved dictionary attribute.
	value_data_t const	*vpd;	 //!< actual data
	size_t			length;  //!< of the vpd data
} value_pair_tmpl_t;

/** Value pair map
 *
 * Value pair maps contain a pair of templates, that describe a src attribute
 * or value, and a destination attribute.
 *
 * Neither src or dst need to be an FR attribute, and their type can be inferred
 * from whether map->da is NULL (not FR).
 *
 * @see value_pair_tmpl_t
 */
typedef struct value_pair_map {
	value_pair_tmpl_t	*dst;	//!< Typically describes the attribute
					//!< to add or modify.
	value_pair_tmpl_t	*src;   //!< Typically describes a value or a
					//!< src attribute to copy.

	FR_TOKEN		op; 	//!< The operator that controls
					//!< insertion of the dst attribute.

	CONF_ITEM		*ci;	//!< Config item that the map was
					//!< created from. Mainly used for
					//!< logging validation errors.

	struct value_pair_map	*next;	//!< The next valuepair map.
} value_pair_map_t;

void radius_tmplfree(value_pair_tmpl_t **tmpl);
int radius_parse_attr(char const *name, value_pair_tmpl_t *vpt,
		      request_refs_t request_def,
		      pair_lists_t list_def);
value_pair_tmpl_t *radius_attr2tmpl(TALLOC_CTX *ctx, char const *name,
				    request_refs_t request_def,
				    pair_lists_t list_def);

value_pair_tmpl_t *radius_str2tmpl(TALLOC_CTX *ctx, char const *name, FR_TOKEN type);
size_t radius_tmpl2str(char *buffer, size_t bufsize, value_pair_tmpl_t const *vpt);
int radius_attrmap(CONF_SECTION *cs, value_pair_map_t **head,
		   pair_lists_t dst_list_def, pair_lists_t src_list_def,
		   unsigned int max);
value_pair_map_t *radius_str2map(TALLOC_CTX *ctx, char const *lhs, FR_TOKEN lhs_type,
				 FR_TOKEN op, char const *rhs, FR_TOKEN rhs_type,
				 request_refs_t dst_request_def,
				 pair_lists_t dst_list_def,
				 request_refs_t src_request_def,
				 pair_lists_t src_list_def);
size_t radius_map2str(char *buffer, size_t bufsize, value_pair_map_t const *map);
value_pair_map_t *radius_cp2map(TALLOC_CTX *ctx, CONF_PAIR *cp,
				request_refs_t dst_request_def,
				pair_lists_t dst_list_def,
				request_refs_t src_request_def,
				pair_lists_t src_list_def);

#ifdef __cplusplus
}
#endif

#endif	/* MAP_H */
