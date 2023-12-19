/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
/*
 * Tiny Embedded JavaScript parser
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#include <httpd.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <shared.h>

#ifdef TRANSLATE_ON_FLY
#include <json_object.h>

extern char *get_cgi_json(char *name, json_object *root);
extern int do_json_decode(struct json_object *root);
extern void parsing_payload(char *url, FILE *stream, int cl, char *method, struct json_object *payload_obj);
#endif

struct REPLACE_PRODUCTID_S replace_productid_t[] =
{
	{"LYRA_VOICE", "LYRA VOICE", "global"},
	{"RT-AC57U_V2", "RT-AC57U V2", "global"},
	{"RT-AC58U_V2", "RT-AC58U V2", "global"},
	{"RT-AC1300G_PLUS_V2", "RT-AC1300G PLUS V2", "global"},
	{"RT-AC1500G_PLUS", "RT-AC1500G PLUS", "global"},
	{"ZenWiFi_CT8", "ZenWiFi AC", "global"},
	{"ZenWiFi_CT8", "灵耀AC3000", "CN"},
	{"ZenWiFi_XT8", "ZenWiFi AX", "global"},
	{"ZenWiFi_XT8", "灵耀AX6600", "CN"},
	{"ZenWiFi_XD4", "ZenWiFi AX Mini", "global"},
	{"ZenWiFi_XD4", "灵耀AX魔方", "CN"},
	{"ZenWiFi_CD6R", "ZenWiFi AC Mini", "global"},
	{"ZenWiFi_CD6N", "ZenWiFi AC Mini", "global"},
	{"ZenWiFi_XP4", "ZenWiFi AX Hybrid", "global"},
	{"ZenWiFi_XP4", "灵耀AX XP4", "CN"},
	{"ZenWiFi_CV4", "ZenWiFi Voice", "global"},
	{"ZenWiFi_Pro_XT12", "灵耀Pro AX11000", "CN"},
	{"ZenWiFi_XD4_Pro", "灵耀AX魔方Pro", "CN"},
	{"ZenWiFi_XT9", "灵耀AX7800", "CN"},
	{"ZenWiFi_XD6", "灵耀AX5400", "CN"},
	{"TUF-AX3000_V2", "TUF GAMING 小旋风", "CN"},
	{"GT6", "ROG魔方 • 幻", "CN"},
	{"TUF-AX4200Q", "TUF GAMING 小旋风 Pro", "CN"},
	{"TUF-AX4200", 	"TUF GAMING AX4200", "global"},
	{"TX-AX6000", "天选游戏路由", "CN"},
	{"TUF-AX6000",  "TUF GAMING AX6000", "global"},
	{NULL, NULL, NULL}
};

static char * get_arg(char *args, char **next);
static void call(char *func, FILE *stream);

/* Look for unquoted character within a string */
static char *
unqstrstr(const char *haystack, const char *needle)
{
	char *cur;
	int q;

	for (cur = (char*) haystack, q = 0;
	     cur < (haystack + strlen(haystack)) && !(!q && !strncmp(needle, cur, strlen(needle)));
	     cur++) {
		if (*cur == '"')
			q ? q-- : q++;
	}
	return (cur < (haystack + strlen(haystack))) ? cur : NULL;
}

static char *
get_arg(char *args, char **next)
{
	char *arg, *end;

	/* Parse out arg, ... */
	if (!(end = unqstrstr(args, ","))) {
		end = args + strlen(args);
		*next = NULL;
	} else
		*next = end + 1;

	/* Skip whitespace and quotation marks on either end of arg */
	for (arg = args; isspace((int)*arg) || *arg == '"'; arg++);
	for (*end-- = '\0'; isspace((int)*end) || *end == '"'; end--)
		*end = '\0';

	return arg;
}

static void
call(char *func, FILE *stream)
{
	char *args, *end, *next;
	int argc;
	char * argv[16];
	struct ej_handler *handler;

	/* Parse out ( args ) */
	if (!(args = strchr(func, '(')))
		return;
	if (!(end = unqstrstr(func, ")")))
		return;
	*args++ = *end = '\0';

	/* Set up argv list */
	for (argc = 0; argc < 16 && args && *args; argc++, args = next) {
		if (!(argv[argc] = get_arg(args, &next)))
			break;
	}

	/* Call handler */
	for (handler = &ej_handlers[0]; handler->pattern; handler++) {
//		if (strncmp(handler->pattern, func, strlen(handler->pattern)) == 0)
		if (strcmp(handler->pattern, func) == 0)
			handler->output(0, stream, argc, argv);
	}
}
#ifdef TRANSLATE_ON_FLY
static const char *asp_mark1 = "<%", *asp_mark2 = "%>", *kw_mark1 = "<#", *kw_mark2 = "#>";

// Call this function if and only if we can read whole <%....%> pattern.
static char *
process_asp (char *s, char *e, FILE *f)
{
	char *func = NULL, *end = NULL;

	if (s == NULL || e == NULL || f == NULL || s >= e) {
		return NULL;
	}

	for (func = s; func < e; func = end) {
		/* Skip initial whitespace */
		for (; isspace((int)*func); func++);
		if (!(end = unqstrstr(func, ";")))
			break;
		*end++ = '\0';

		/* Call function */
		call(func, f);

		// skip asp_mark2
		end = e + strlen (asp_mark2);
		break;
	}

	return end;
}

extern void replace_productid(char *GET_PID_STR, char *RP_PID_STR, int len){

	struct REPLACE_PRODUCTID_S *p;
	char *p_temp;

	for(p = &replace_productid_t[0]; p->org_name; p++){
		if(!strcmp(GET_PID_STR, p->org_name)){
			if(!strncmp(nvram_safe_get("preferred_lang"), p->p_lang, 2))
				strlcpy(RP_PID_STR, p->replace_name, len);

			if(!strcmp("global", p->p_lang) && !strlen(RP_PID_STR))
				strlcpy(RP_PID_STR, p->replace_name, len);
		}
	}

	if(strlen(RP_PID_STR))
		return;

	if ((p_temp = strstr(GET_PID_STR, "ZenWiFi_")) && !strncmp(nvram_safe_get("preferred_lang"), "CN", 2)) {
		p_temp += strlen("ZenWiFi_");
		snprintf(RP_PID_STR, len, "灵耀%s", p_temp);
	}
	else{
		strlcpy(RP_PID_STR, GET_PID_STR, len);
	}

	/* general  replace underscore with space */
	for (; *RP_PID_STR; ++RP_PID_STR)
	{
		if (*RP_PID_STR == '_')
			*RP_PID_STR = ' ';
	}
}

struct REPLACE_TAG_S replace_tag_string_t[] =
{
	{"BASIC_MODEL_NAME", ""}, //special tag for dynamic model replace
	{"ZVDOMAIN_NAMEVZ", DUT_DOMAIN_NAME},
	{NULL, NULL}
};

static char *replace_tag_string(char *desc, char *pattern, int pattern_len){

	int pid_len = 0, get_pid_len = 0;
	char target_string[64] = {0}, replace_string[64] = {0}, pattern_tmp[2048] = {0};
	char *p_PID_STR = NULL;
	struct REPLACE_TAG_S *p;

	for(p = &replace_tag_string_t[0]; p->org_name; p++){

		if(!strcmp("BASIC_MODEL_NAME", p->org_name)){
			char PID_STR[32] = {0}, GET_PID_STR[32]={0}, RP_PID_STR[32] = {0};
			strlcpy(PID_STR, nvram_safe_get("productid"), sizeof(PID_STR));
			strlcpy(GET_PID_STR, get_productid(), sizeof(GET_PID_STR));
			replace_productid(GET_PID_STR, RP_PID_STR, sizeof(RP_PID_STR));
			if(strcmp(PID_STR, RP_PID_STR) != 0){
				strlcpy(target_string, PID_STR, sizeof(target_string));
				strlcpy(replace_string, RP_PID_STR, sizeof(replace_string));
			}else
				continue;
		}else{
			strlcpy(target_string, p->org_name, sizeof(target_string));
			strlcpy(replace_string, p->replace_name, sizeof(replace_string));
		}

		pid_len = strlen(target_string);
		get_pid_len = strlen(replace_string);
		memset(pattern_tmp, 0, sizeof(pattern_tmp));

		char *pSrc  = desc;
		char *pDest = &pattern_tmp[0];

		while((p_PID_STR = strstr(pSrc, target_string)))
		{
			if((p_PID_STR - pSrc) > 0){
				memcpy(pDest, pSrc, p_PID_STR - pSrc);
				pDest[p_PID_STR - pSrc] = '\0';
			}
			pDest += (p_PID_STR - pSrc);
			pSrc   =  p_PID_STR + pid_len;

			memcpy(pDest, replace_string, get_pid_len);
			pDest[get_pid_len] = '\0';
			pDest += get_pid_len;
		}
		if(pDest != pattern_tmp)
		{
			strlcpy(pDest, pSrc, sizeof(pattern_tmp));
			strlcpy(pattern, pattern_tmp, pattern_len);
			desc = pattern;
		}
	}
	return desc;
}

// Call this function if and only if we can read whole <#....#> pattern.
static char *
translate_lang (char *s, char *e, FILE *f, kw_t *pkw)
{
	char *end = NULL, *name = NULL, *desc = NULL;
	if (s == NULL || e == NULL || f == NULL || pkw == NULL || s >= e) {
		return NULL;
	}

	for (name = s; name < e; name = end) {
		/* Skip initial whitespace */
		for (; isspace((int)*name); name++);
		if (!(end = strstr(name, kw_mark2)))
			break;
		*end++ = '=';	// '#' --> '=', search_desc() need '='
		*end++ = '\0';	// '>' --> '\0'

		desc = search_desc (pkw, name);
		if (desc != NULL) {
			static char pattern[2048] = {0};
			desc = replace_tag_string(desc, pattern, sizeof(pattern));
			fprintf (f, "%s", desc);
		}

		// skip kw_mark2
		end = e + strlen (kw_mark2);
		break;
	}

	return end;
}

#ifdef TRANSLATE_ON_FLY
extern char Accept_Language[];
extern int is_firsttime(void);
#endif

// This translation engine can not process <%...%> interlace with <#...#>
void
do_ej(char *path, FILE *stream)
{
#define PATTERN_LENGTH	1024
#define FRAG_SIZE	128
#define RESERVE_SIZE	4
	int frag_size = FRAG_SIZE;
	int pattern_size = PATTERN_LENGTH - RESERVE_SIZE;
	char pat_buf[PATTERN_LENGTH];
	char *pattern = pat_buf, *asp = NULL, *asp_end = NULL, *key = NULL, *key_end = NULL;
	char *start_pat, *end_pat, *lang;
	FILE *fp = NULL;
	int conn_break = 0;
	size_t ret, read_len, len;
	int no_translate = 1;
	static kw_t kw = {0, 0, NULL, NULL};

	if (!(fp = fopen(path, "r")))
		return;

#ifdef TRANSLATE_ON_FLY
	char *current_lang = NULL;
	struct json_object *root = json_object_new_object();
	struct json_object *payload_obj = json_object_new_object();

	// Load dictionary file
	lang = nvram_safe_get("preferred_lang");
	if(!check_lang_support(lang)){
		lang = nvram_default_get("preferred_lang");
		nvram_set("preferred_lang", lang);
	}

	if(check_user_agent(user_agent) == FROM_WebView)
		parsing_payload(referer_url, NULL, 0, "get", payload_obj);

	do_json_decode(root);

	if ((current_lang = get_cgi_json("current_lang", payload_obj)) != NULL || (current_lang = get_cgi_json("current_lang", root)) != NULL){
		if (load_dictionary (current_lang, &kw)){
			no_translate = 0;
		}
	}
	else{
		if (load_dictionary (lang, &kw)){
			no_translate = 0;
		}
	}
	if (root) json_object_put(root);
	if (payload_obj) json_object_put(payload_obj);
#endif  //defined TRANSLATE_ON_FLY

	start_pat = end_pat = pattern;
	memset (pattern + pattern_size, 0, 4);
	while (conn_break == 0)
	{
		int special;

		// Arrange pattern[] if available buffer length (end_pat~pattern[pattern_size]) is smaller than frag_size
		if (((pattern + pattern_size) - end_pat) < frag_size)
		{
			len = end_pat - start_pat;
			if(len < pattern_size){
				memcpy (pattern, start_pat, len);
				pattern[len] = '\0';
				start_pat = pattern;
				end_pat = start_pat + len;
				*end_pat = '\0';
			}
		}

		read_len = (pattern + pattern_size) - end_pat;
		len = fread (end_pat, 1, read_len, fp);
		if (len == 0)   {
			if (start_pat < end_pat)	{
				fwrite (start_pat, 1, (size_t) (end_pat - start_pat), stream);
			}
			break;
		}
		end_pat += len;
		*end_pat = '\0';

		asp = strstr (start_pat, asp_mark1);
		key = NULL;
		if (no_translate == 0)  {
			key = strstr (start_pat, kw_mark1);
		}
		special = 0;
		while ((start_pat < end_pat) && special == 0)
		{
			int postproc = 0;	/* 0: need more data; 1: translate; 2: execute asp; 3: write only; */
			char *s, *e, *p;

			/*				 asp      asp_end
			 *				 ^	^
			 *      +------------------------------<%.......%>-------------------------------+
			 *  |	 XXXXXXXXXXXXXXXXXXXXX<#.......#>YYYYYYYYYYYYYYYYYY0	    |0000
			 *  +------------------------------------------------------------------------+
			 *  ^	 ^		    ^	^ ^		 ^	     ^
			 *  |	 |		    |	| p		 |	     |
			 *  pattern   start_pat,s	  key,e(2) key_end	     end_pat,e(1)  pattern + pattern_size
			 *				     ^				|
			 *				     +--------------------------------+
			 *
			 */

			// If <%...%> and <#...#> do not exist in pattern[], write whole pattern[].
			s = start_pat;
			e = end_pat;

			if (key != NULL && asp == NULL) {
				e = key;						// Write start_pat ~ (key - 1)
				key_end = strstr (key, kw_mark2);
				if (key_end != NULL) {	// We do have <#...#> in pattern[].
					postproc = 1;
				}
			} else if (key != NULL && asp != NULL)  {
				// We have <%...%> and <#...#> in pattern[], process first occurrence
				if (asp < key)  {
					e = asp;					// Write start_pat ~ (asp - 1)
					asp_end = strstr (asp, asp_mark2);
					if (asp_end != NULL) {	// We do have whole <%...%>.
						postproc = 2;
					}
				} else {
					e = key;					// Write start_pat ~ (key - 1)
					key_end = strstr (key, kw_mark2);
					if (key_end != NULL) {	// We do have whole <#...#>.
						postproc = 1;
					}
				}
			} else if (key == NULL && asp != NULL)  {
				e = asp;						// Write start_pat ~ (asp - 1)
				asp_end = strstr (asp, asp_mark2);
				if (asp_end != NULL) {	// We do have whole <%...%>.
					postproc = 2;
				}
			} else {
				// Special case. If last character is '<'
				// DO NOT write this character due to next one may be % or #.
				if (*(e-1) == *asp_mark1 || *(e-1) == *kw_mark1)	{
					special = 1;
					e--;
				}

				postproc = 3;
			}

			// process text preceeding <# or <%
			if (e > s) {
				ret = fwrite (s, 1, (size_t) (e - s), stream);
				if (ret == 0 || ret < (e - s))  {
					/* the connection had been damaged. DO NOT process another data. */
					/* (reduce response time of httpd) */
//					cprintf ("fwrite() ret %d, s %p e %p len %d, break do_ej()'s while loop\n", ret, s, e, e-s);
					conn_break = 1;
					break;
				} else {
					start_pat = e;
				}
			}
			// post process
			p = NULL;
			if (postproc == 1) {				// translate
				p = translate_lang (key + strlen (kw_mark1), key_end, stream, &kw);
				if (no_translate == 0 && p != NULL) {
					key = strstr (p, kw_mark1);
				}
			} else if (postproc == 2) {			// execute asp
				p = process_asp (asp + strlen (asp_mark1), asp_end, stream);
				if (p != NULL)  {
					asp = strstr (p, asp_mark1);
				}
			} else if (postproc == 3) {			// no <%...%> or <#...#>
				p = e;
			} else if (postproc == 0) {			// read more data
				break;
			}

			if (p != NULL)  {
				start_pat = p;
			}

		}	/* while ((start_pat < end_pat) && special == 0) */
	}		/* while (conn_break == 0) */

	fflush (stream);

	fclose(fp);
}
#endif  // defined TRANSLATE_ON_FLY

int
ejArgs(int argc, char **argv, char *fmt, ...)
{
	va_list	ap;
	int arg;
	char *c;

	if (!argv)
		return 0;

	va_start(ap, fmt);
	for (arg = 0, c = fmt; c && *c && arg < argc;) {
		if (*c++ != '%')
			continue;
		switch (*c) {
		case 'd':
			*(va_arg(ap, int *)) = atoi(argv[arg]);
			break;
		case 's':
			*(va_arg(ap, char **)) = argv[arg];
			break;
		}
		arg++;
	}
	va_end(ap);

	return arg;
}
