#ifndef _CONFFILE_H
#define _CONFFILE_H

/*
 * conffile.h	Defines for the conffile parsing routines.
 *
 * Version:	$Id$
 *
 */

RCSIDH(conffile_h, "$Id$")

#include <stddef.h>
#include <freeradius-devel/token.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Export the minimum amount of information about these structs
 */
typedef struct conf_item CONF_ITEM;
typedef struct conf_pair CONF_PAIR;
typedef struct conf_part CONF_SECTION;
typedef struct conf_data CONF_DATA;

/*
 *  Instead of putting the information into a configuration structure,
 *  the configuration file routines MAY just parse it directly into
 *  user-supplied variables.
 */
#define PW_TYPE_STRING_PTR	100
#define PW_TYPE_BOOLEAN		101
#define PW_TYPE_SUBSECTION	102
#define PW_TYPE_FILE_INPUT	103
#define PW_TYPE_FILE_OUTPUT	104
#define PW_TYPE_DEPRECATED	(1 << 10)
#define PW_TYPE_REQUIRED	(1 << 11)
#define PW_TYPE_ATTRIBUTE	(1 << 12)

typedef struct CONF_PARSER {
  char const *name;
  int type;			/* PW_TYPE_STRING, etc. */
  size_t offset;		/* relative pointer within "base" */
  void *data;			/* absolute pointer if base is NULL */
  const void *dflt;		/* default as it would appear in radiusd.conf */
} CONF_PARSER;

CONF_SECTION	*cf_section_alloc(CONF_SECTION *parent, char const *name1,
				  char const *name2);
int		cf_pair_replace(CONF_SECTION *cs, CONF_PAIR *cp,
				char const *value);
int		cf_item_parse(CONF_SECTION *cs, char const *name,
			      int type, void *data, char const *dflt);
int		cf_section_parse(CONF_SECTION *, void *base,
				 CONF_PARSER const *variables);
const CONF_PARSER *cf_section_parse_table(CONF_SECTION *cs);
CONF_SECTION	*cf_file_read(char const *file);
void		cf_file_free(CONF_SECTION *cs);
int		cf_file_include(CONF_SECTION *cs, char const *file);

CONF_PAIR	*cf_pair_find(CONF_SECTION const *, char const *name);
CONF_PAIR	*cf_pair_find_next(CONF_SECTION const *, CONF_PAIR const *, char const *name);
CONF_SECTION	*cf_section_find(char const *name);
CONF_SECTION	*cf_section_find_name2(CONF_SECTION const *section,
				       char const *name1, char const *name2);
CONF_SECTION	*cf_section_sub_find(CONF_SECTION const *, char const *name);
CONF_SECTION	*cf_section_sub_find_name2(CONF_SECTION const *, char const *name1, char const *name2);
char const 	*cf_section_value_find(CONF_SECTION const *, char const *attr);
CONF_SECTION	*cf_top_section(CONF_SECTION *cs);

void *cf_data_find(CONF_SECTION const *, char const *);
int cf_data_add(CONF_SECTION *, char const *, void *, void (*)(void *));

char const *cf_pair_attr(CONF_PAIR const *pair);
char const *cf_pair_value(CONF_PAIR const *pair);
FR_TOKEN cf_pair_operator(CONF_PAIR const *pair);
FR_TOKEN cf_pair_value_type(CONF_PAIR const *pair);
VALUE_PAIR *cf_pairtovp(CONF_PAIR *pair);
char const *cf_section_name1(CONF_SECTION const *);
char const *cf_section_name2(CONF_SECTION const *);
int dump_config(CONF_SECTION const *cs);
CONF_SECTION *cf_subsection_find_next(CONF_SECTION const *section,
				      CONF_SECTION const *subsection,
				      char const *name1);
CONF_SECTION *cf_section_find_next(CONF_SECTION const *section,
				   CONF_SECTION const *subsection,
				   char const *name1);
int cf_section_lineno(CONF_SECTION const *section);
int cf_pair_lineno(CONF_PAIR const *pair);
char const *cf_pair_filename(CONF_PAIR const *pair);
char const *cf_section_filename(CONF_SECTION const *section);
CONF_ITEM *cf_item_find_next(CONF_SECTION const *section, CONF_ITEM const *item);
CONF_SECTION *cf_item_parent(CONF_ITEM const *ci);
int cf_item_is_section(CONF_ITEM const *item);
int cf_item_is_pair(CONF_ITEM const *item);
CONF_PAIR *cf_itemtopair(CONF_ITEM const *item);
CONF_SECTION *cf_itemtosection(CONF_ITEM const *item);
CONF_ITEM *cf_pairtoitem(CONF_PAIR const *cp);
CONF_ITEM *cf_sectiontoitem(CONF_SECTION const *cs);
int cf_section_template(CONF_SECTION *cs, CONF_SECTION *template);
void cf_log_err(CONF_ITEM const *ci, char const *fmt, ...)
#ifdef __GNUC__
		__attribute__ ((format (printf, 2, 3)))
#endif
;
void cf_log_err_cs(CONF_SECTION const *cs, char const *fmt, ...)
#ifdef __GNUC__
		__attribute__ ((format (printf, 2, 3)))
#endif
;
void cf_log_err_cp(CONF_PAIR const *cp, char const *fmt, ...)
#ifdef __GNUC__
		__attribute__ ((format (printf, 2, 3)))
#endif
;

void cf_log_info(CONF_SECTION const *cs, char const *fmt, ...)
#ifdef __GNUC__
		__attribute__ ((format (printf, 2, 3)))
#endif
;
void cf_log_module(CONF_SECTION const *cs, char const *fmt, ...)
#ifdef __GNUC__
		__attribute__ ((format (printf, 2, 3)))
#endif
;
CONF_ITEM *cf_reference_item(CONF_SECTION const *parentcs,
			     CONF_SECTION *outercs,
			     char const *ptr);

extern int cf_pair2xml(FILE *fp, CONF_PAIR const *cp);
extern int cf_section2xml(FILE *fp, CONF_SECTION const *cs);
extern int cf_pair2file(FILE *fp, CONF_PAIR const *cp);
extern int cf_section2file(FILE *fp, CONF_SECTION const *cs);
extern CONF_SECTION *root_config;

/*
 *	Big magic.
 */
int cf_section_migrate(CONF_SECTION *dst, CONF_SECTION *src);

#ifdef __cplusplus
}
#endif

#endif /* _CONFFILE_H */
