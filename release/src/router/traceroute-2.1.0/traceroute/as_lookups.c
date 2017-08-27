/*
    Copyright (c)  2006, 2007		Dmitry Butskoy
					<buc@citadel.stu.neva.ru>
    License:  GPL v2 or any later

    See COPYING for the status of this software.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "traceroute.h"


#define DEF_RADB_SERVER		"whois.radb.net"
#define DEF_RADB_SERVICE	"nicname"


static sockaddr_any ra_addr = {{ 0, }, };
static char ra_buf[512] = { 0, };


const char *get_as_path (const char *query) {
	int sk, n;
	FILE *fp;
	char buf[1024];
	int prefix = 0, best_prefix = 0;
	char *rb, *re = &ra_buf[sizeof (ra_buf) / sizeof (*ra_buf) - 1];


	if (!ra_addr.sa.sa_family) {
	    const char *server, *service;
	    struct addrinfo *res;
	    int ret;

	    server = getenv ("RA_SERVER");
	    if (!server)  server = DEF_RADB_SERVER;

	    service = getenv ("RA_SERVICE");
	    if (!service)  service = DEF_RADB_SERVICE;


	    ret = getaddrinfo (server, service, NULL, &res);
	    if (ret) {
		fprintf (stderr, "%s/%s: %s\n", server, service,
						    gai_strerror(ret));
		exit (2);
	    }	

	    memcpy (&ra_addr, res->ai_addr, res->ai_addrlen);

	    freeaddrinfo (res);
	}


	sk = socket (ra_addr.sa.sa_family, SOCK_STREAM, 0);
	if (sk < 0)  error ("socket");

	if (connect (sk, &ra_addr.sa, sizeof (ra_addr)) < 0)
		goto  err_sk;

	n = snprintf (buf, sizeof (buf), "%s\r\n", query);
	if (n >= sizeof (buf))  goto err_sk;

	if (write (sk, buf, n) < n)
		goto err_sk;

	fp = fdopen (sk, "r");
	if (!fp)  goto err_sk;


	strcpy (ra_buf, "*");
	rb = ra_buf;

	while (fgets (buf, sizeof (buf), fp) != NULL) {

	    if (!strncmp (buf, "route:", sizeof ("route:") - 1) ||
		!strncmp (buf, "route6:", sizeof ("route6:") - 1)
	    ) {
		char *p = strchr (buf, '/');

		if (p)  prefix = strtoul (++p, NULL, 10);
		else  prefix = 0;	/*  Hmmm...  */

	    }
	    else if (!strncmp (buf, "origin:", sizeof ("origin:") -1)) {
		char *p, *as;

		p = buf + (sizeof ("origin:") - 1);

		while (isspace (*p))  p++;
		as = p;
		while (*p && !isspace (*p))  p++;
		*p = '\0';

		if (prefix > best_prefix) {
		    best_prefix = prefix;

		    rb = ra_buf;
		    while (rb < re && (*rb++ = *as++)) ;
		}
		else if (prefix == best_prefix) {
		    char *q = strstr (ra_buf, as);

		    if (!q || (*(q += strlen (as)) != '\0' && *q != '/')) {
			if (rb > ra_buf)  rb[-1] = '/';
			while (rb < re && (*rb++ = *as++)) ;
		    }
		}
		/*  else just ignore it   */
	    }
	}

	fclose (fp);

	return ra_buf;


err_sk:
	close (sk);
	return "!!";
}
