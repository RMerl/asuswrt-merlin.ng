/*
  Copyright (c) 2000 The Regents of the University of Michigan.
  All rights reserved.

  Copyright (c) 2002 Bruce Fields <bfields@UMICH.EDU>

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
  3. Neither the name of the University nor the names of its
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif	/* HAVE_CONFIG_H */

#include <sys/param.h>
#include <sys/stat.h>
#include <rpc/rpc.h>

#include <pwd.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <nfsidmap.h>
#include <nfslib.h>
#include <time.h>

#include "svcgssd.h"
#include "gss_util.h"
#include "err_util.h"
#include "context.h"
#include "misc.h"
#include "gss_oids.h"
#include "svcgssd_krb5.h"

static int
get_krb5_hostbased_name(gss_buffer_desc *name, char **hostbased_name)
{
	char *p, *sname = NULL;
	if (strchr(name->value, '@') && strchr(name->value, '/')) {
		if ((sname = calloc(name->length, 1)) == NULL) {
			printerr(0, "ERROR: get_krb5_hostbased_name failed "
				 "to allocate %d bytes\n", name->length);
			return -1;
		}
		/* read in name and instance and replace '/' with '@' */
		sscanf(name->value, "%[^@]", sname);
		p = strrchr(sname, '/');
		if (p == NULL) {    /* The '@' preceeded the '/' */
			free(sname);
			return -1;
		}
		*p = '@';
	}
	*hostbased_name = sname;
	return 0;
}

int
get_hostbased_client_name(gss_name_t client_name, gss_OID mech,
			  char **hostbased_name)
{
	u_int32_t	maj_stat, min_stat;
	gss_buffer_desc	name;
	gss_OID		name_type = GSS_C_NO_OID;
	char		*cname;
	int		res = -1;

	*hostbased_name = NULL;	    /* preset in case we fail */

	/* Get the client's gss authenticated name */
	maj_stat = gss_display_name(&min_stat, client_name, &name, &name_type);
	if (maj_stat != GSS_S_COMPLETE) {
		pgsserr("get_hostbased_client_name: gss_display_name",
			maj_stat, min_stat, mech);
		goto out_err;
	}
	if (name.length >= 0xffff) {	    /* don't overflow */
		printerr(0, "ERROR: get_hostbased_client_name: "
			 "received gss_name is too long (%d bytes)\n",
			 name.length);
		goto out_rel_buf;
	}

	/* For Kerberos, transform the NT_KRB5_PRINCIPAL name to
	 * an NT_HOSTBASED_SERVICE name */
	if (g_OID_equal(&krb5oid, mech)) {
		if (get_krb5_hostbased_name(&name, &cname) == 0)
			*hostbased_name = cname;
	} else {
		printerr(1, "WARNING: unknown/unsupport mech OID\n");
	}

	res = 0;
out_rel_buf:
	gss_release_buffer(&min_stat, &name);
out_err:
	return res;
}

void
get_hostbased_client_buffer(gss_name_t client_name, gss_OID mech,
			    gss_buffer_t buf)
{
	char *hname;

	if (!get_hostbased_client_name(client_name, mech, &hname)) {
		buf->length = strlen(hname) + 1;
		buf->value = hname;
	} else {
		buf->length = 0;
		buf->value = NULL;
	}
}
