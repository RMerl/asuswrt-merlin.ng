/* dnsmasq is Copyright (c) 2000-2024 Simon Kelley

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991, or
   (at your option) version 3 dated 29 June, 2007.
 
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
     
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "dnsmasq.h"

#if defined (HAVE_NFTSET) && defined (HAVE_LINUX_NETWORK)

#include <nftables/libnftables.h>

#include <string.h>
#include <arpa/inet.h>

static struct nft_ctx *ctx = NULL;
static const char *cmd_add = "add element %s { %s }";
static const char *cmd_del = "delete element %s { %s }";

void nftset_init()
{
  ctx = nft_ctx_new(NFT_CTX_DEFAULT);
  if (ctx == NULL)
    die(_("failed to create nftset context"), NULL, EC_MISC);

  /* disable libnftables output */
  nft_ctx_buffer_error(ctx);
}

int add_to_nftset(const char *setname, const union all_addr *ipaddr, int flags, int remove)
{
  const char *cmd = remove ? cmd_del : cmd_add;
  int ret, af = (flags & F_IPV4) ? AF_INET : AF_INET6;
  size_t new_sz;
  char *err_str, *new, *nl;
  const char *err;
  static char *cmd_buf = NULL;
  static size_t cmd_buf_sz = 0;

  inet_ntop(af, ipaddr, daemon->addrbuff, ADDRSTRLEN);

  if (setname[1] == ' ' && (setname[0] == '4' || setname[0] == '6'))
    {
      if (setname[0] == '4' && !(flags & F_IPV4))
	return -1;

      if (setname[0] == '6' && !(flags & F_IPV6))
	return -1;

      setname += 2;
    }
  
  if (cmd_buf_sz == 0)
    new_sz = 150; /* initial allocation */
  else
    new_sz = snprintf(cmd_buf, cmd_buf_sz, cmd, setname, daemon->addrbuff);
  
  if (new_sz > cmd_buf_sz)
    {
      if (!(new = whine_malloc(new_sz + 10)))
	return 0;

      if (cmd_buf)
	free(cmd_buf);
      cmd_buf = new;
      cmd_buf_sz = new_sz + 10;
      snprintf(cmd_buf, cmd_buf_sz, cmd, setname, daemon->addrbuff);
    }

  ret = nft_run_cmd_from_buffer(ctx, cmd_buf);
  err = nft_ctx_get_error_buffer(ctx);

  if (ret != 0)
    {
      /* Log only first line of error return. */
      if ((err_str = whine_malloc(strlen(err) + 1)))
	{
	  strcpy(err_str, err);
	  if ((nl = strchr(err_str, '\n')))
	    *nl = 0;
	  my_syslog(LOG_ERR,  "nftset %s %s", setname, err_str);
	  free(err_str);
	}
    }
  
  return ret;
}

#endif
