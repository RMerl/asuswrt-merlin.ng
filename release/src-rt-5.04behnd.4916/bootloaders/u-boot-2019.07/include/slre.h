/*
 * Copyright (c) 2004-2005 Sergey Lyubka <valenok@gmail.com>
 * All rights reserved
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Sergey Lyubka wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */

/*
 * Downloaded Sat Nov  5 17:42:08 CET 2011 at
 * http://slre.sourceforge.net/1.0/slre.h
 */

/*
 * This is a regular expression library that implements a subset of Perl RE.
 * Please refer to http://slre.sourceforge.net for detailed description.
 *
 * Usage example (parsing HTTP request):
 *
 * struct slre	slre;
 * struct cap	captures[4 + 1];  // Number of braket pairs + 1
 * ...
 *
 * slre_compile(&slre,"^(GET|POST) (\S+) HTTP/(\S+?)\r\n");
 *
 * if (slre_match(&slre, buf, len, captures)) {
 *	printf("Request line length: %d\n", captures[0].len);
 *	printf("Method: %.*s\n", captures[1].len, captures[1].ptr);
 *	printf("URI: %.*s\n", captures[2].len, captures[2].ptr);
 * }
 *
 * Supported syntax:
 *	^		Match beginning of a buffer
 *	$		Match end of a buffer
 *	()		Grouping and substring capturing
 *	[...]		Match any character from set
 *	[^...]		Match any character but ones from set
 *	\s		Match whitespace
 *	\S		Match non-whitespace
 *	\d		Match decimal digit
 *	\r		Match carriage return
 *	\n		Match newline
 *	+		Match one or more times (greedy)
 *	+?		Match one or more times (non-greedy)
 *	*		Match zero or more times (greedy)
 *	*?		Match zero or more times (non-greedy)
 *	?		Match zero or once
 *	\xDD		Match byte with hex value 0xDD
 *	\meta		Match one of the meta character: ^$().[*+?\
 */

#ifndef SLRE_HEADER_DEFINED
#define	SLRE_HEADER_DEFINED

/*
 * Compiled regular expression
 */
struct slre {
	unsigned char	code[256];
	unsigned char	data[256];
	int		code_size;
	int		data_size;
	int		num_caps;	/* Number of bracket pairs	*/
	int		anchored;	/* Must match from string start	*/
	const char	*err_str;	/* Error string			*/
};

/*
 * Captured substring
 */
struct cap {
	const char	*ptr;		/* Pointer to the substring	*/
	int		len;		/* Substring length		*/
};

/*
 * Compile regular expression. If success, 1 is returned.
 * If error, 0 is returned and slre.err_str points to the error message.
 */
int slre_compile(struct slre *, const char *re);

/*
 * Return 1 if match, 0 if no match.
 * If `captured_substrings' array is not NULL, then it is filled with the
 * values of captured substrings. captured_substrings[0] element is always
 * a full matched substring. The round bracket captures start from
 * captured_substrings[1].
 * It is assumed that the size of captured_substrings array is enough to
 * hold all captures. The caller function must make sure it is! So, the
 * array_size = number_of_round_bracket_pairs + 1
 */
int slre_match(const struct slre *, const char *buf, int buf_len,
	struct cap *captured_substrings);

#ifdef SLRE_TEST
void slre_dump(const struct slre *r, FILE *fp);
#endif /* SLRE_TEST */
#endif /* SLRE_HEADER_DEFINED */
