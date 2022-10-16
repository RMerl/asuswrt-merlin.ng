/* $Id: testupnpreplyparse.c,v 1.5 2017/12/12 11:18:46 nanard Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2006-2017 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "upnpreplyparse.h"

int
test_parsing(const char * buf, int len, FILE * f)
{
	char line[1024];
	struct NameValueParserData pdata;
	int ok = 1;
	ParseNameValue(buf, len, &pdata);
	/* check result */
	if(f != NULL)
	{
		while(fgets(line, sizeof(line), f))
		{
			char * value;
			char * equal;
			char * parsedvalue;
			int l;
			l = strlen(line);
			while((l > 0) && ((line[l-1] == '\r') || (line[l-1] == '\n')))
				line[--l] = '\0';
			/* skip empty lines */
			if(l == 0)
				continue;
			equal = strchr(line, '=');
			if(equal == NULL)
			{
				fprintf(stderr, "Warning, line does not contain '=' : %s\n", line);
				continue;
			}
			*equal = '\0';
			value = equal + 1;
			parsedvalue = GetValueFromNameValueList(&pdata, line);
			if((parsedvalue == NULL) || (strcmp(parsedvalue, value) != 0))
			{
				fprintf(stderr, "Element <%s> : expecting value '%s', got '%s'\n",
				        line, value, parsedvalue ? parsedvalue : "<null string>");
				ok = 0;
			}
		}
	}
	ClearNameValueList(&pdata);
	return ok;
}

int main(int argc, char * * argv)
{
	FILE * f;
	char * buffer;
	long l;
	int ok;

	if(argc<2)
	{
		fprintf(stderr, "Usage: %s file.xml [file.namevalues]\n", argv[0]);
		return 1;
	}
	f = fopen(argv[1], "r");
	if(!f)
	{
		fprintf(stderr, "Error : can not open file %s\n", argv[1]);
		return 2;
	}
	if(fseek(f, 0, SEEK_END) < 0) {
		perror("fseek");
		return 1;
	}
	l = (int)ftell(f);
	if(l < 0) {
		perror("ftell");
		return 1;
	}
	if(fseek(f, 0, SEEK_SET) < 0) {
		perror("fseek");
		return 1;
	}
	buffer = malloc(l + 1);
	if(buffer == NULL) {
		fprintf(stderr, "Error: failed to allocate %ld bytes\n", l+1);
		return 1;
	}
	l = fread(buffer, 1, l, f);
	fclose(f);
	f = NULL;
	buffer[l] = '\0';
	if(argc > 2)
	{
		f = fopen(argv[2], "r");
		if(!f)
		{
			fprintf(stderr, "Error : can not open file %s\n", argv[2]);
			return 2;
		}
	}
#ifdef DEBUG
	DisplayNameValueList(buffer, l);
#endif
	ok = test_parsing(buffer, l, f);
	if(f)
	{
		fclose(f);
	}
	free(buffer);
	return ok ? 0 : 3;
}

