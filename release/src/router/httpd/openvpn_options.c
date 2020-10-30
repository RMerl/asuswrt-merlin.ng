#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/stat.h>

#define TYPEDEF_BOOL
#include <shared.h>
#include <shutils.h>
#include <bcmnvram.h>
#include "httpd.h"
#include <openvpn_options.h>
#include <openvpn_config.h>

struct buffer
alloc_buf (size_t size)
{
	struct buffer buf;

	if (!buf_size_valid (size))
		buf_size_error (size);
	buf.capacity = (int)size;
	buf.offset = 0;
	buf.len = 0;
	buf.data = calloc (1, size);

	return buf;
}

void
buf_size_error (const size_t size)
{
	logmessage ("OVPN", "fatal buffer size error, size=%lu", (unsigned long)size);
}

bool
buf_printf (struct buffer *buf, const char *format, ...)
{
	int ret = false;
	if (buf_defined (buf))
	{
		va_list arglist;
		uint8_t *ptr = buf_bend (buf);
		int cap = buf_forward_capacity (buf);

		if (cap > 0)
		{
			int stat;
			va_start (arglist, format);
			stat = vsnprintf ((char *)ptr, cap, format, arglist);
			va_end (arglist);
			*(buf->data + buf->capacity - 1) = 0; /* windows vsnprintf needs this */
			buf->len += (int) strlen ((char *)ptr);
			if (stat >= 0 && stat < cap)
				ret = true;
		}
	}
	return ret;
}

bool
buf_parse (struct buffer *buf, const int delim, char *line, const int size)
{
	bool eol = false;
	int n = 0;
	int c;

	do
	{
		c = buf_read_u8 (buf);
		if (c < 0)
			eol = true;
		if (c <= 0 || c == delim)
			c = 0;
		if (n >= size)
			break;
		line[n++] = c;
	}
	while (c);

	line[size-1] = '\0';
	return !(eol && !strlen (line));
}

void
buf_clear (struct buffer *buf)
{
	if (buf->capacity > 0)
		memset (buf->data, 0, buf->capacity);
	buf->len = 0;
	buf->offset = 0;
}

void
free_buf (struct buffer *buf)
{
	if (buf->data)
		free (buf->data);
	CLEAR (*buf);
}

char *
string_alloc (const char *str)
{
	if (str)
	{
		const int n = strlen (str) + 1;
		char *ret;

		ret = calloc(n, 1);
		if(!ret)
			return NULL;

		memcpy (ret, str, n);
		return ret;
	}
	else
		return NULL;
}

static inline bool
space (unsigned char c)
{
	return c == '\0' || isspace (c);
}

int
parse_line (const char *line, char *p[], const int n, const int line_num)
{
	const int STATE_INITIAL = 0;
	const int STATE_READING_QUOTED_PARM = 1;
	const int STATE_READING_UNQUOTED_PARM = 2;
	const int STATE_DONE = 3;
	const int STATE_READING_SQUOTED_PARM = 4;

	int ret = 0;
	const char *c = line;
	int state = STATE_INITIAL;
	bool backslash = false;
	char in, out;

	char parm[OPTION_PARM_SIZE];
	unsigned int parm_len = 0;

	do
	{
		in = *c;
		out = 0;

		if (!backslash && in == '\\' && state != STATE_READING_SQUOTED_PARM)
		{
			backslash = true;
		}
		else
		{
			if (state == STATE_INITIAL)
			{
				if (!space (in))
				{
					if (in == ';' || in == '#') /* comment */
						break;
					if (!backslash && in == '\"')
						state = STATE_READING_QUOTED_PARM;
					else if (!backslash && in == '\'')
						state = STATE_READING_SQUOTED_PARM;
					else
					{
						out = in;
						state = STATE_READING_UNQUOTED_PARM;
					}
				}
			}
			else if (state == STATE_READING_UNQUOTED_PARM)
			{
				if (!backslash && space (in))
					state = STATE_DONE;
				else
					out = in;
			}
			else if (state == STATE_READING_QUOTED_PARM)
			{
				if (!backslash && in == '\"')
					state = STATE_DONE;
				else
					out = in;
			}
			else if (state == STATE_READING_SQUOTED_PARM)
			{
				if (in == '\'')
					state = STATE_DONE;
				else
					out = in;
			}

			if (state == STATE_DONE)
			{
				p[ret] = calloc (parm_len + 1, 1);
				memcpy (p[ret], parm, parm_len);
				p[ret][parm_len] = '\0';
				state = STATE_INITIAL;
				parm_len = 0;
				++ret;
			}

			if (backslash && out)
			{
				if (!(out == '\\' || out == '\"' || space (out)))
				{
					logmessage ("OVPN", "Options warning: Bad backslash ('\\') usage in %d", line_num);
					return 0;
				}
			}
			backslash = false;
		}

		/* store parameter character */
		if (out)
		{
			if (parm_len >= SIZE (parm))
			{
				parm[SIZE (parm) - 1] = 0;
				logmessage ("OVPN", "Options error: Parameter at %d is too long (%d chars max): %s",
					line_num, (int) SIZE (parm), parm);
				return 0;
			}
			parm[parm_len++] = out;
		}

		/* avoid overflow if too many parms in one config file line */
		if (ret >= n)
			break;

	} while (*c++ != '\0');


	if (state == STATE_READING_QUOTED_PARM)
	{
		logmessage ("OVPN", "Options error: No closing quotation (\") in %d", line_num);
		return 0;
	}
	if (state == STATE_READING_SQUOTED_PARM)
	{
		logmessage ("OVPN", "Options error: No closing single quotation (\') in %d", line_num);
		return 0;
	}
	if (state != STATE_INITIAL)
	{
		logmessage ("OVPN", "Options error: Residual parse state (%d) in %d", line_num);
		return 0;
	}

	return ret;
}

static void
bypass_doubledash (char **p)
{
	if (strlen (*p) >= 3 && !strncmp (*p, "--", 2))
		*p += 2;
}

static bool
in_src_get (const struct in_src *is, char *line, const int size)
{
	if (is->type == IS_TYPE_FP)
	{
		return BOOL_CAST (fgets (line, size, is->u.fp));
	}
	else if (is->type == IS_TYPE_BUF)
	{
		bool status = buf_parse (is->u.multiline, '\n', line, size);
		if ((int) strlen (line) + 1 < size)
			strcat (line, "\n");
		return status;
	}
	else
	{
		return false;
	}
}

static char *
read_inline_file (struct in_src *is, const char *close_tag)
{
	char line[OPTION_LINE_SIZE];
	struct buffer buf = alloc_buf (10000);
	char *ret;
	while (in_src_get (is, line, sizeof (line)))
	{
		if (!strncmp (line, close_tag, strlen (close_tag)))
			break;
		buf_printf (&buf, "%s", line);
	}
	ret = string_alloc (buf_str (&buf));
	buf_clear (&buf);
	free_buf (&buf);
	CLEAR (line);
	return ret;
}

static bool
check_inline_file (struct in_src *is, char *p[])
{
	bool ret = false;
	if (p[0] && !p[1])
	{
		char *arg = p[0];
		if (arg[0] == '<' && arg[strlen(arg)-1] == '>')
		{
			struct buffer close_tag;
			arg[strlen(arg)-1] = '\0';
			p[0] = string_alloc (arg+1);
			p[1] = string_alloc (INLINE_FILE_TAG);
			close_tag = alloc_buf (strlen(p[0]) + 4);
			buf_printf (&close_tag, "</%s>", p[0]);
			p[2] = read_inline_file (is, buf_str (&close_tag));
			p[3] = NULL;
			free_buf (&close_tag);
			ret = true;
		}
	}
	return ret;
}

static bool
check_inline_file_via_fp (FILE *fp, char *p[])
{
	struct in_src is;
	is.type = IS_TYPE_FP;
	is.u.fp = fp;
	return check_inline_file (&is, p);
}

void
add_custom(int unit, char *p[])
{
	char custom[2048];
	char *param = NULL;
	char *final_custom = NULL;
	int i = 0, size = 0, sizeParam = 0;

	if(!p[0])
		return;

	while(p[i]) {
		size += strlen(p[i]) + 1;
		if(strchr(p[i], ' ')) {
			size += 2;
		}
		i++;
	}

	param = (char*)calloc(size, sizeof(char));
	sizeParam = size * sizeof(char);

	if(!param)
		return;

	i = 0;
	while(p[i]) {
		//_dprintf("p[%d]: [%s]\n", i, p[i]);
		if(*param)
			strlcat(param, " ", sizeParam);

		if(strchr(p[i], ' ')) {
			strlcat(param, "\"", sizeParam);
			strlcat(param, p[i], sizeParam);
			strlcat(param, "\"", sizeParam);
		}
		else {
			strlcat(param, p[i], sizeParam);
		}

		i++;
	}
	_dprintf("add [%s]\n", param);

	get_ovpn_custom(OVPN_TYPE_CLIENT, unit, custom, sizeof (custom));

	sizeParam = (strlen(custom) + strlen(param) + 2)*sizeof(char);
	final_custom = calloc(strlen(custom) + strlen(param) + 2, sizeof(char));

	if(final_custom) {
		if(*custom) {
			strlcat(final_custom, custom, sizeParam);
			strlcat(final_custom, "\n", sizeParam);
		}
		strlcat(final_custom, param, sizeParam);

		set_ovpn_custom(OVPN_TYPE_CLIENT, unit, final_custom);
		free(final_custom);
	}

	free(param);
}

static int
add_option (char *p[], int line, int unit)
{
	char prefix[32] = {0};

	snprintf(prefix, sizeof(prefix), "vpn_client%d_", unit);

	if  (streq (p[0], "dev") && p[1])
	{
		if(!strncmp(p[1], "tun", 3))
			nvram_pf_set(prefix, "if", "tun");
		else if(!strncmp(p[1], "tap", 3))
			nvram_pf_set(prefix, "if", "tap");
	}
	else if  (streq (p[0], "proto") && p[1])
	{
		nvram_pf_set(prefix, "proto", p[1]);
	}
	else if  (streq (p[0], "remote") && p[1])
	{
		nvram_pf_set(prefix, "addr", p[1]);

		if(p[2])
			nvram_pf_set(prefix, "port", p[2]);
		if(p[3])
		{
			if(!strncmp(p[3], "tcp", 3))
				nvram_pf_set(prefix, "proto", "tcp-client");
			else if(!strncmp(p[1], "udp", 3))
				nvram_pf_set(prefix,  "proto", "udp");
		}
	}
	else if  (streq (p[0], "port") && p[1])
	{
		nvram_pf_set(prefix, "port", p[1]);
	}
	else if (streq (p[0], "connect-retry-max") && p[1])
	{
		nvram_pf_set(prefix, "connretry", p[1]);
	}
	else if (streq (p[0], "comp-lzo"))
	{
		if(p[1])
			nvram_pf_set(prefix, "comp", p[1]);
		else
			nvram_pf_set(prefix, "comp", "adaptive");
	}
	else if (streq (p[0], "compress"))
	{
		if (p[1]) {
			if (streq (p[1], "lzo"))
				nvram_pf_set(prefix, "comp", "yes");
			else if (streq (p[1], "lz4"))
				nvram_pf_set(prefix, "comp", "lz4");
			else if (streq (p[1], "lz4-v2"))
				nvram_pf_set(prefix, "comp", "lz4-v2");
			else if (streq (p[1], "stub"))
				nvram_pf_set(prefix, "comp", "stub");
                        else if (streq (p[1], "stub-v2"))
                                nvram_pf_set(prefix, "comp", "stub-v2");
		} else {
			nvram_pf_set(prefix, "comp", "no");
		}
	}
#if 0
	else if (streq (p[0], "cipher") && p[1])
	{
		nvram_pf_set(prefix, "cipher", p[1]);
	}
#endif
	else if (streq (p[0], "auth") && p[1])
	{
		nvram_pf_set(prefix, "digest", p[1]);
	}
	else if (streq (p[0], "verb") && p[1])
	{
		nvram_pf_set(prefix, "verb", p[1]);
	}
	else if  (streq (p[0], "ca") && p[1])
	{
		nvram_pf_set(prefix, "crypt", "tls");
		if (streq (p[1], INLINE_FILE_TAG) && p[2] && strstr(p[2], PEM_START_TAG))
		{
			set_ovpn_key(OVPN_TYPE_CLIENT, unit, OVPN_CLIENT_CA, p[2], NULL);
		}
		else
		{
			return VPN_UPLOAD_NEED_CA_CERT;
		}
	}
	else if  (streq (p[0], "cert") && p[1])
	{
		if (streq (p[1], INLINE_FILE_TAG) && p[2] && strstr(p[2], PEM_START_TAG))
		{
			set_ovpn_key(OVPN_TYPE_CLIENT, unit, OVPN_CLIENT_CERT, p[2], NULL);
		}
		else
		{
			return VPN_UPLOAD_NEED_CERT;
		}
	}
	else if (streq (p[0], "extra-certs") && p[1])
	{
		if (streq (p[1], INLINE_FILE_TAG) && p[2] && strstr(p[2], PEM_START_TAG))
		{
			set_ovpn_key(OVPN_TYPE_CLIENT, unit, OVPN_CLIENT_EXTRA, p[2], NULL);
		}
		else
		{
			return VPN_UPLOAD_NEED_EXTRA;
		}
	}
	else if  (streq (p[0], "key") && p[1])
	{
		if (streq (p[1], INLINE_FILE_TAG) && p[2] && strstr(p[2], PEM_START_TAG))
		{
			set_ovpn_key(OVPN_TYPE_CLIENT, unit, OVPN_CLIENT_KEY, p[2], NULL);
		}
		else
		{
			return VPN_UPLOAD_NEED_KEY;
		}
	}
	else if (streq (p[0], "tls-auth") && p[1])
	{
		if (streq (p[1], INLINE_FILE_TAG) && p[2] && strstr(p[2], PEM_START_TAG))
		{
			set_ovpn_key(OVPN_TYPE_CLIENT, unit, OVPN_CLIENT_STATIC, p[2], NULL);
			//key-direction
			if(nvram_pf_match(prefix, "hmac", "-1"))	//default, disable
				nvram_pf_set(prefix, "hmac", "2");	//openvpn default value: KEY_DIRECTION_BIDIRECTIONAL
		}
		else
		{
			if(p[2])
				nvram_pf_set(prefix, "hmac", p[2]);

			return VPN_UPLOAD_NEED_STATIC;
		}
	}
	else if (streq (p[0], "tls-crypt") && p[1])
	{
		if (streq (p[1], INLINE_FILE_TAG) && p[2] && strstr(p[2], PEM_START_TAG))
		{
			set_ovpn_key(OVPN_TYPE_CLIENT, unit, OVPN_CLIENT_STATIC, p[2], NULL);
			//key-direction
			if(nvram_pf_match(prefix, "hmac", "-1"))	//default, disable
				nvram_pf_set(prefix, "hmac", "3");	//Enable tls-crypt
		}
		else
		{
			if(p[2]) {
				nvram_pf_set(prefix, "hmac", p[2]);
			}
			return VPN_UPLOAD_NEED_STATIC;
		}
	}
	else if (streq (p[0], "tls-crypt-v2") && p[1])
	{
		if (streq (p[1], INLINE_FILE_TAG) && p[2] && strstr(p[2], PEM_START_TAG))
		{
			set_ovpn_key(OVPN_TYPE_CLIENT, unit, OVPN_CLIENT_STATIC, p[2], NULL);
			if(nvram_pf_match(prefix, "hmac", "-1"))	//default, disable
				nvram_pf_set(prefix, "hmac", "4");	//Enable tls-crypt-v2
		}
		else
		{
			if(p[2]) {
				nvram_pf_set(prefix, "hmac", 4);
			}
			return VPN_UPLOAD_NEED_STATIC;
		}
	}
	else if (streq (p[0], "secret") && p[1])
	{
		nvram_pf_set(prefix, "crypt", "secret");
		if (streq (p[1], INLINE_FILE_TAG) && p[2] && strstr(p[2], PEM_START_TAG))
		{
			set_ovpn_key(OVPN_TYPE_CLIENT, unit, OVPN_CLIENT_STATIC, p[2], NULL);
		}
		else
		{
			return VPN_UPLOAD_NEED_STATIC;
		}
	}
	else if (streq (p[0], "auth-user-pass"))
	{
		nvram_pf_set(prefix, "userauth", "1");
	}
	else if (streq (p[0], "tls-remote") && p[1])
	{
		nvram_pf_set(prefix, "tlsremote", "1");
		nvram_pf_set(prefix, "cn", p[1]);
	}
	else if (streq (p[0], "verify-x509-name") && p[1] && p[2])
	{
		if (streq(p[2], "name"))
			nvram_pf_set(prefix, "tlsremote", "1");
		else if (streq(p[2], "name-prefix"))
			nvram_pf_set(prefix, "tlsremote", "2");
		else if (streq(p[2], "subject"))
			nvram_pf_set(prefix, "tlsremote", "3");
		nvram_pf_set(prefix, "cn", p[1]);
	}
	else if (streq (p[0], "key-direction") && p[1])
	{
		nvram_pf_set(prefix, "hmac", p[1]);
	}
	else if (streq (p[0], "reneg-sec") && p[1])
	{
		nvram_pf_set(prefix, "reneg", p[1]);
	}
	// These are already added by us
	else if (streq (p[0], "client") ||
		 streq (p[0], "nobind") ||
		 streq (p[0], "persist-key") ||
		 streq (p[0], "persist-tun"))
	{
		return 0;	// Don't duplicate them
	}
	else if (streq (p[0], "crl-verify") && p[1])
	{
		if (streq (p[1], INLINE_FILE_TAG) && p[2] && strstr(p[2], PEM_START_TAG))
		{
			set_ovpn_key(OVPN_TYPE_CLIENT, unit, OVPN_CLIENT_CRL, p[2], NULL);
		}
		else
		{
			return VPN_UPLOAD_NEED_CRL;
		}
	}
	else if (streq (p[0], "ncp-ciphers") && p[1])
	{
		nvram_pf_set(prefix, "ncp_ciphers", p[1]);
	}
	else if (streq (p[0], "redirect-gateway") && (!p[1] || streq (p[1], "def1")))	// Only handle if default GW
	{
		nvram_pf_set(prefix, "rgw", "1");
		add_custom(unit, p);
	}
	else
	{
		if ( streq (p[0], "client")
			|| streq (p[0], "nobind")
			|| streq (p[0], "persist-key")
			|| streq (p[0], "persist-tun")
			|| streq (p[0], "user")
			|| streq (p[0], "group")
		) {
			;//ignore
		}
		else {
			add_custom(unit, p);
		}
	}
	return 0;
}

int
read_config_file (const char *file, int unit)
{
	FILE *fp;
	int line_num;
	char line[OPTION_LINE_SIZE];
	char *p[MAX_PARMS];
	int ret = 0;
	char prefix[32] = {0};

	fp = fopen (file, "r");
	if (fp)
	{
		line_num = 0;
		while (fgets(line, sizeof (line), fp))
		{
			int offset = 0;
			CLEAR (p);
			++line_num;
			/* Ignore UTF-8 BOM at start of stream */
			if (line_num == 1 && strncmp (line, "\xEF\xBB\xBF", 3) == 0)
				offset = 3;
			if (parse_line (line + offset, p, SIZE (p), line_num))
			{
				bypass_doubledash (&p[0]);
				check_inline_file_via_fp (fp, p);
				ret |= add_option (p, line_num, unit);
			}
		}
		fclose (fp);

		if( !(ret & VPN_UPLOAD_NEED_KEY)
			&& !ovpn_key_exists(OVPN_TYPE_CLIENT, unit, OVPN_CLIENT_KEY)
		) {
			snprintf(prefix, sizeof(prefix), "vpn_client%d_", unit);
			nvram_pf_set(prefix, "useronly", "1");
		}
	}
	else
	{
		logmessage ("OVPN", "Error opening configuration file");
	}

	CLEAR (line);
	CLEAR (p);

	return ret;
}


void parse_openvpn_status(int unit)
{
	FILE *fpi, *fpo;
	char buf[512];
	char *token;
	char nv_name[32] = "";
	char prefix_vpn[] = "vpn_serverXX_";

	snprintf(buf, sizeof(buf), "/etc/openvpn/server%d/status", unit);
	fpi = fopen(buf, "r");

	snprintf(buf, sizeof(buf), "/etc/openvpn/server%d/client_status", unit);
	fpo = fopen(buf, "w");

	snprintf(prefix_vpn, sizeof(prefix_vpn), "vpn_server%d_", unit);

	if(fpi && fpo) {
		while(!feof(fpi)){
			CLEAR(buf);
			if (!fgets(buf, sizeof(buf), fpi))
				break;
			if(!strncmp(buf, "CLIENT_LIST", 11)) {
				//printf("%s", buf);
				token = strtok(buf, ",");	//CLIENT_LIST
				token = strtok(NULL, ",");	//Common Name
				token = strtok(NULL, ",");	//Real Address
				if(token)
					fprintf(fpo, "%s ", token);
				else
					fprintf(fpo, "NoRealAddress ");
				snprintf(nv_name, sizeof(nv_name) -1, "vpn_server%d_if", unit);

				if(nvram_match(strcat_r(prefix_vpn, "if", nv_name), "tap")
					&& nvram_match(strcat_r(prefix_vpn, "dhcp", nv_name), "1")) {
					fprintf(fpo, "VirtualAddressAssignedByDhcp ");
				}
				else {
					token = strtok(NULL, ",");	//Virtual Address
					if(token)
						fprintf(fpo, "%s ", token);
					else
						fprintf(fpo, "NoVirtualAddress ");
				}
				token = strtok(NULL, ",");	//Bytes Received
				token = strtok(NULL, ",");	//Bytes Sent
				token = strtok(NULL, ",");	//Connected Since
				token = strtok(NULL, ",");	//Connected Since (time_t)
				token = strtok(NULL, ",");	//Username
				if(token)
					fprintf(fpo, "%s\n", token);
				else
					fprintf(fpo, "NoUsername\n");
				fprintf(fpo, "\n");
			}
#if 0
			else if(!strncmp(buf, "REMOTE", 6)) {
				token = strtok(buf, ",");       //REMOTE,
				token = strtok(NULL, ",");      //Real Address
				if(token)
					fprintf(fpo, "%s ", token);
				else
					fprintf(fpo, "NoRealAddress ");

				fprintf(fpo, "%s ", conf.remote);
				fprintf(fpo, "Static_Key");
				break;
			}
#endif
		}
	}
	if(fpi) fclose(fpi);
	if(fpo) fclose(fpo);
}
