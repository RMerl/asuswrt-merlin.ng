/*
 * parser.h - netlink command line parser
 *
 * Interface for command line parser used by netlink code.
 */

#ifndef ETHTOOL_NETLINK_PARSER_H__
#define ETHTOOL_NETLINK_PARSER_H__

#include <stddef.h>

#include "netlink.h"

/* Some commands need to generate multiple netlink messages or multiple nested
 * attributes from one set of command line parameters. Argument @group_style
 * of nl_parser() takes one of three values:
 *
 * PARSER_GROUP_NONE - no grouping, flat series of attributes (default)
 * PARSER_GROUP_NEST - one nest for each @group value (@group is nest type)
 * PARSER_GROUP_MSG  - one message for each @group value (@group is command)
 */
enum parser_group_style {
	PARSER_GROUP_NONE = 0,
	PARSER_GROUP_NEST,
	PARSER_GROUP_MSG,
};

typedef int (*param_parser_cb_t)(struct nl_context *, uint16_t, const void *,
				 struct nl_msg_buff *, void *);

struct param_parser {
	/* command line parameter handled by this entry */
	const char		*arg;
	/* group id (see enum parser_group_style above) */
	unsigned int		group;
	/* netlink attribute type */
	uint16_t		type;		/* netlink attribute type */
	/* function called to parse parameter and its value */
	param_parser_cb_t	handler;
	/* pointer passed as @data argument of the handler */
	const void		*handler_data;
	/* minimum number of extra arguments after this parameter */
	unsigned int		min_argc;
	/* if @dest is passed to nl_parser(), offset to store value */
	unsigned int		dest_offset;
	/* parameter alternative group - only one parameter from a group
	 * can be specified, 0 means no group
	 */
	unsigned int		alt_group;
};

/* data structures used for handler data */

/* used by nl_parse_lookup_u32() */
struct lookup_entry_u32 {
	const char	*arg;
	uint32_t	val;
};

/* used by nl_parse_lookup_u8() */
struct lookup_entry_u8 {
	const char	*arg;
	uint8_t		val;
};

/* used by nl_parse_byte_str() */
struct byte_str_parser_data {
	unsigned int	min_len;
	unsigned int	max_len;
	char		delim;
};

/* used for value stored by nl_parse_byte_str() */
struct byte_str_value {
	unsigned int	len;
	u8		*data;
};

/* used by nl_parse_error() */
struct error_parser_data {
	const char	*err_msg;
	int		ret_val;
	unsigned int	extra_args;
};

/* used by nl_parse_bitset() */
struct bitset_parser_data {
	bool		force_hex;
	bool		no_mask;
};

/* used by nl_parse_char_bitset() */
struct char_bitset_parser_data {
	const char	*bit_chars;
	unsigned int	nbits;
	char		reset_char;
	bool		no_mask;
};

int parse_u32(const char *arg, uint32_t *result);

/* parser handlers to use as param_parser::handler */

/* NLA_FLAG represented by on | off */
int nl_parse_flag(struct nl_context *nlctx, uint16_t type, const void *data,
		  struct nl_msg_buff *msgbuff, void *dest);
/* NLA_NUL_STRING represented by a string argument */
int nl_parse_string(struct nl_context *nlctx, uint16_t type, const void *data,
		    struct nl_msg_buff *msgbuff, void *dest);
/* NLA_U32 represented as numeric argument */
int nl_parse_direct_u32(struct nl_context *nlctx, uint16_t type,
			const void *data, struct nl_msg_buff *msgbuff,
			void *dest);
/* NLA_U8 represented as numeric argument */
int nl_parse_direct_u8(struct nl_context *nlctx, uint16_t type,
		       const void *data, struct nl_msg_buff *msgbuff,
		       void *dest);
/* NLA_U32 represented as float number of meters, converted to cm. */
int nl_parse_direct_m2cm(struct nl_context *nlctx, uint16_t type,
			 const void *data, struct nl_msg_buff *msgbuff,
			 void *dest);
/* NLA_U8 represented as on | off */
int nl_parse_u8bool(struct nl_context *nlctx, uint16_t type, const void *data,
		    struct nl_msg_buff *msgbuff, void *dest);
/* NLA_U32 represented by a string argument (lookup table) */
int nl_parse_lookup_u32(struct nl_context *nlctx, uint16_t type,
			const void *data, struct nl_msg_buff *msgbuff,
			void *dest);
/* NLA_U8 represented by a string argument (lookup table) */
int nl_parse_lookup_u8(struct nl_context *nlctx, uint16_t type,
		       const void *data, struct nl_msg_buff *msgbuff,
		       void *dest);
/* always fail (for deprecated parameters) */
int nl_parse_error(struct nl_context *nlctx, uint16_t type, const void *data,
		   struct nl_msg_buff *msgbuff, void *dest);
/* NLA_BINARY represented by %x:%x:...%x (e.g. a MAC address) */
int nl_parse_byte_str(struct nl_context *nlctx, uint16_t type,
		      const void *data, struct nl_msg_buff *msgbuff,
		      void *dest);
/* bitset represented by %x[/%x] or name on|off ... [--] */
int nl_parse_bitset(struct nl_context *nlctx, uint16_t type, const void *data,
		    struct nl_msg_buff *msgbuff, void *dest);
/* bitset represented by %u[/%u] or string (characters for bits) */
int nl_parse_char_bitset(struct nl_context *nlctx, uint16_t type,
			 const void *data, struct nl_msg_buff *msgbuff,
			 void *dest);

/* main entry point called to parse the command line */
int nl_parser(struct nl_context *nlctx, const struct param_parser *params,
	      void *dest, enum parser_group_style group_style,
	      struct nl_msg_buff **msgbuffs);

#endif /* ETHTOOL_NETLINK_PARSER_H__ */
