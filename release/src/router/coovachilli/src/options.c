/* -*- mode: c; c-basic-offset: 2 -*- */
/* 
 * Copyright (C) 2003, 2004, 2005 Mondru AB.
 * Copyright (C) 2007-2012 David Bird (Coova Technologies) <support@coova.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#include "chilli.h"
#ifdef ENABLE_MODULES
#include "chilli_module.h"
#endif

void options_init() {
  memset(&_options, 0, sizeof(_options));
}

static inline void options_md5(struct options_t *o, uint8_t *cksum) {
  MD5_CTX context;
  MD5Init(&context);
  MD5Update(&context, (uint8_t *)o, sizeof(struct options_t));
  MD5Final(cksum, &context);
}

/* Get IP address and mask */
int option_aton(struct in_addr *addr, struct in_addr *mask,
		char *pool, int number) {

  /* Parse only first instance of network for now */
  /* Eventually "number" will indicate the token which we want to parse */

  unsigned int a1, a2, a3, a4;
  unsigned int m1, m2, m3, m4;
  unsigned int m;
  int masklog;
  int c;

  c = sscanf(pool, "%u.%u.%u.%u/%u.%u.%u.%u",
	     &a1, &a2, &a3, &a4,
	     &m1, &m2, &m3, &m4);
  
  switch (c) {
  case 4:
    mask->s_addr = htonl(0xffffff00);
    break;
  case 5:
    if (m1 > 32) {
      log_err(0, "Invalid mask");
      return -1; /* Invalid mask */
    }
    mask->s_addr = m1 > 0 ? htonl(0xffffffff << (32 - m1)) : 0;
    break;
  case 8:
    if (m1 >= 256 ||  m2 >= 256 || m3 >= 256 || m4 >= 256) {
      log_err(0, "Invalid mask");
      return -1; /* Wrong mask format */
    }
    m = m1 * 0x1000000 + m2 * 0x10000 + m3 * 0x100 + m4;
    for (masklog = 0; ((1 << masklog) < ((~m)+1)); masklog++);
    if (((~m)+1) != (1 << masklog)) {
      log_err(0, "Invalid mask");
      return -1; /* Wrong mask format (not all ones followed by all zeros)*/
    }
    mask->s_addr = htonl(m);
    break;
  default:
    log_err(0, "Invalid mask");
    return -1; /* Invalid mask */
  }

  if (a1 >= 256 ||  a2 >= 256 || a3 >= 256 || a4 >= 256) {
    log_err(0, "Wrong IP address format");
    return -1;
  }
  else
    addr->s_addr = htonl(a1 * 0x1000000 + a2 * 0x10000 + a3 * 0x100 + a4);

  return 0;
}

static int option_s_s(bstring str, char **sp) {
  char *s = *sp ? *sp : "";
  size_t len = strlen(s) + 1;
  *sp = (char *)(size_t)str->slen;
  if (bcatblk(str, s, len) != BSTR_OK) return 0;
  return 1;
}

static int option_s_l(bstring str, char **s) {
  size_t offset = (size_t) *s;
  *s = ((char *)str->data) + offset;
  if (!**s) *s = 0;
  return 1;
}

static int opt_run(int argc, char **argv, int reload) {
  char **newargs;
  char file[128];
  int status;
  int i;

  chilli_binconfig(file, sizeof(file), 0);

  log_dbg("(Re)processing options [%s]", file);

  if ((status = safe_fork()) < 0) {
    log_err(errno, "fork() returned -1!");
    return -1;
  }
  
  if (status > 0) { /* Parent */
    return status;
  }

  if (!(newargs = calloc(1, sizeof(char *) * (argc + 4)))) {
    return -1;
  }

  for (i=1; i < argc; i++) {
    newargs[i] = argv[i];
  }

  newargs[0] = "chilli_opt";
  newargs[i++] = "-b";
  newargs[i++] = file;
  newargs[i++] = reload ? "-r" : NULL;

  log_dbg("running chilli_opt on %s", file);

  if (execv(SBINDIR "/chilli_opt", newargs) != 0) {
    log_err(errno, "execl() did not return 0!");
    exit(0);
  }

  exit(0);
}

int options_load(int argc, char **argv, bstring bt) {
  static char done_before = 0;
  char file[128];
  int fd;

  chilli_binconfig(file, sizeof(file), 0);

  fd = open(file, O_RDONLY);

  while (fd <= 0) {
    int status = 0;
    int pid = opt_run(argc, argv, 0);
    waitpid(pid, &status, 0);
    if (WIFEXITED(status) && WEXITSTATUS(status) == 2) exit(0);
    fd = open(file, O_RDONLY);
    if (fd <= 0) {
      if (done_before) break;
      else {
	char *offline = getenv("CHILLI_OFFLINE");
	if (offline) {
	  execl(
#ifdef ENABLE_CHILLISCRIPT
		SBINDIR "/chilli_script", SBINDIR "/chilli_script", _options.binconfig, 
#else
		offline,
#endif
		offline, (char *) 0);

	  break;
	} 

	log_warn(0, "could not generate configuration (%s), sleeping one second", file);
	sleep(1);
      }
    }
  }

  if (fd <= 0) return 0;
  done_before = 1;

  log_dbg("PID %d rereading binary file %s", getpid(), file);
  return options_fromfd(fd, bt);
}

int options_mkdir(char *path) {

  if (mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO)) {
    switch (errno) {
    case EEXIST:
      /* not necessarily a directory */
      unlink(path);
      if (mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO)) {
	log_err(errno, "mkdir %s", path);
	return -1;
      }
      break;
    default:
      log_err(errno, "mkdir %s", path);
      return -1;
    }
  }

  if (_options.uid && geteuid() == 0) {
    if (chown(path, _options.uid, _options.gid)) {
      log_err(errno, "could not chown() %s", path);
    }
  }
  return 0;
}

int options_fromfd(int fd, bstring bt) {
  uint8_t cksum[16], cksum_check[16];
  struct options_t o;
  char has_error = 1;
  size_t len;
  int i;

#ifdef ENABLE_MODULES
  char isReload[MAX_MODULES];
#endif
  
  int rd = safe_read(fd, &o, sizeof(o));

  if (rd == sizeof(o)) {
    rd = safe_read(fd, &len, sizeof(len));
    if (rd == sizeof(len)) {
      ballocmin(bt, len);
      rd = safe_read(fd, bt->data, len);
      if (rd == len) {
	has_error = 0;
      }
      rd = safe_read(fd, cksum_check, sizeof(cksum_check));
      if (rd != sizeof(cksum_check)) {
	has_error = 1;
      } else {
	options_md5(&o, cksum);
	if (memcmp(cksum, cksum_check, sizeof(cksum))) {
	  has_error = 1;
	}
      }
    }
  }
  
  close(fd);

  if (has_error) {
    log_err(errno, "could not read configuration, some kind of mismatch fd=%d %s",
	    fd, SBINDIR);
    return 0;
  }
  
  if (!option_s_l(bt, &o.binconfig)) return 0;
  if (!option_s_l(bt, &o.pidfile)) return 0;
  if (!option_s_l(bt, &o.statedir)) return 0;
  if (!option_s_l(bt, &o.usestatusfile)) return 0;
  if (!option_s_l(bt, &o.tundev)) return 0;
  if (!option_s_l(bt, &o.dynip)) return 0;
  if (!option_s_l(bt, &o.statip)) return 0;
  if (!option_s_l(bt, &o.ethers)) return 0;

  if (!option_s_l(bt, &o.domain)) return 0;
  if (!option_s_l(bt, &o.ipup)) return 0;
  if (!option_s_l(bt, &o.ipdown)) return 0;
  if (!option_s_l(bt, &o.conup)) return 0;
  if (!option_s_l(bt, &o.condown)) return 0;
  if (!option_s_l(bt, &o.macup)) return 0;
  if (!option_s_l(bt, &o.macdown)) return 0;
#ifdef ENABLE_IEEE8021Q
  if (!option_s_l(bt, &o.vlanupdate)) return 0;
#endif
#ifdef ENABLE_PROXYVSA
  if (!option_s_l(bt, &o.locationupdate)) return 0;
#endif

  if (!option_s_l(bt, &o.radiussecret)) return 0;
#ifdef ENABLE_LARGELIMITS
  if (!option_s_l(bt, &o.radiusacctsecret)) return 0;
  if (!option_s_l(bt, &o.radiusadmsecret)) return 0;
#endif
  if (!option_s_l(bt, &o.radiusnasid)) return 0;
  if (!option_s_l(bt, &o.radiuslocationid)) return 0;
  if (!option_s_l(bt, &o.radiuslocationname)) return 0;
  if (!option_s_l(bt, &o.locationname)) return 0;
  if (!option_s_l(bt, &o.proxysecret)) return 0;
  
  if (!option_s_l(bt, &o.dhcpif)) return 0;
#ifdef ENABLE_MULTILAN
  for (i=0; i < MAX_MOREIF; i++) {
    if (!option_s_l(bt, &o.moreif[i].dhcpif)) return 0;
    if (!option_s_l(bt, &o.moreif[i].vlan)) return 0;
  }
#endif
  if (!option_s_l(bt, &o.routeif)) return 0;
  if (!option_s_l(bt, &o.peerkey)) return 0;

  if (!option_s_l(bt, &o.macsuffix)) return 0;
  if (!option_s_l(bt, &o.macpasswd)) return 0;

  if (!option_s_l(bt, &o.uamsecret)) return 0;
  if (!option_s_l(bt, &o.uamurl)) return 0;
  if (!option_s_l(bt, &o.uamaaaurl)) return 0;
  if (!option_s_l(bt, &o.uamhomepage)) return 0;
  if (!option_s_l(bt, &o.wisprlogin)) return 0;

  if (!option_s_l(bt, &o.wwwdir)) return 0;
  if (!option_s_l(bt, &o.wwwbin)) return 0;
  if (!option_s_l(bt, &o.uamui)) return 0;
  if (!option_s_l(bt, &o.localusers)) return 0;
#ifdef HAVE_SSL
  if (!option_s_l(bt, &o.sslkeyfile)) return 0;
  if (!option_s_l(bt, &o.sslkeypass)) return 0;
  if (!option_s_l(bt, &o.sslcertfile)) return 0;
  if (!option_s_l(bt, &o.sslcafile)) return 0;
#endif
#ifdef USING_IPC_UNIX
  if (!option_s_l(bt, &o.unixipc)) return 0;
#endif
#ifdef HAVE_NETFILTER_COOVA
  if (!option_s_l(bt, &o.kname)) return 0;
#endif
#ifdef ENABLE_DNSLOG
  if (!option_s_l(bt, &o.dnslog)) return 0;
#endif
#ifdef ENABLE_IPWHITELIST
  if (!option_s_l(bt, &o.ipwhitelist)) return 0;
#endif
#ifdef ENABLE_UAMDOMAINFILE
  if (!option_s_l(bt, &o.uamdomainfile)) return 0;
#endif
#ifdef ENABLE_MODULES
  if (!option_s_l(bt, &o.moddir)) return 0;
#endif

  if (!option_s_l(bt, &o.adminuser)) return 0;
  if (!option_s_l(bt, &o.adminpasswd)) return 0;
  if (!option_s_l(bt, &o.adminupdatefile)) return 0;
  if (!option_s_l(bt, &o.rtmonfile)) return 0;

  if (!option_s_l(bt, &o.ssid)) return 0;
  if (!option_s_l(bt, &o.vlan)) return 0;
  if (!option_s_l(bt, &o.nasmac)) return 0;
  if (!option_s_l(bt, &o.nasip)) return 0;
  if (!option_s_l(bt, &o.cmdsocket)) return 0;

  if (!option_s_l(bt, &o.uamaliasname)) return 0;
  if (!option_s_l(bt, &o.uamhostname)) return 0;

#ifdef ENABLE_REDIRINJECT
  if (!option_s_l(bt, &o.inject)) return 0;
  if (!option_s_l(bt, &o.inject_ext)) return 0;
#endif

  for (i=0; i < MAX_UAM_DOMAINS; i++) {
    if (!option_s_l(bt, &o.uamdomains[i])) 
      return 0;
  }

#ifdef EX_OPTIONS_LOAD
#include EX_OPTIONS_LOAD
#endif

#ifdef ENABLE_CHILLIREDIR
  for (i = 0; i < MAX_REGEX_PASS_THROUGHS; i++) {
#if defined (__FreeBSD__) || defined (__APPLE__) || defined (__OpenBSD__) || defined (__NetBSD__)
    regfree(&_options.regex_pass_throughs[i].re_host);
    regfree(&_options.regex_pass_throughs[i].re_path);
    regfree(&_options.regex_pass_throughs[i].re_qs);
#else
    if (_options.regex_pass_throughs[i].re_host.allocated)
      regfree(&_options.regex_pass_throughs[i].re_host);
    if (_options.regex_pass_throughs[i].re_path.allocated)
      regfree(&_options.regex_pass_throughs[i].re_path);
    if (_options.regex_pass_throughs[i].re_qs.allocated)
      regfree(&_options.regex_pass_throughs[i].re_qs);
#endif
  }
#endif

#ifdef ENABLE_MODULES
  for (i=0; i < MAX_MODULES; i++) {
    isReload[i]=0;
    if (!_options.modules[i].name[0]) break;
    if (!_options.modules[i].ctx) continue;
    else {
      struct chilli_module *m = 
	(struct chilli_module *)_options.modules[i].ctx;
      if (!strcmp(_options.modules[i].name, o.modules[i].name))
	isReload[i]=1;
      if (m->destroy)
	m->destroy(isReload[i]);
    }
    log_dbg("Unloading module %s",_options.modules[i].name);
    chilli_module_unload(_options.modules[i].ctx);
  }
#endif

  if (_options._data) free(_options._data);
  memcpy(&_options, &o, sizeof(o));
  _options._data = (char *)bt->data;

#ifdef ENABLE_MODULES
  log_dbg("Loading modules");
  for (i=0; i < MAX_MODULES; i++) {
    if (!_options.modules[i].name[0]) break;
    log_dbg("Loading module %s",_options.modules[i].name);
    chilli_module_load(&_options.modules[i].ctx, 
		       _options.modules[i].name);
    if (_options.modules[i].ctx) {
      struct chilli_module *m = 
	(struct chilli_module *)_options.modules[i].ctx;
      if (m->initialize)
	m->initialize(_options.modules[i].conf, isReload[i]); 
    }
  }
#endif

  /* 
   *  We took the buffer and this bt will be destroyed.
   *  Give the bstring a bogus buffer so that bdestroy() works.
   */
  bt->data = (u_char *)strdup("");
  bt->mlen = 1;
  bt->slen = 0;

  return 1;
}

int options_save(char *file, bstring bt) {
  uint8_t cksum[16];
  struct options_t o;
  mode_t oldmask;
  int fd, i;

  log_dbg("PID %d saving options to %s", getpid(), file);

  memcpy(&o, &_options, sizeof(o));

#ifdef ENABLE_CHILLIREDIR
  for (i = 0; i < MAX_REGEX_PASS_THROUGHS; i++) {
    memset(&o.regex_pass_throughs[i].re_host, 0, sizeof(regex_t));
    memset(&o.regex_pass_throughs[i].re_path, 0, sizeof(regex_t));
    memset(&o.regex_pass_throughs[i].re_qs, 0, sizeof(regex_t));
  }
#endif

  if (!option_s_s(bt, &o.binconfig)) return 0;
  if (!option_s_s(bt, &o.pidfile)) return 0;
  if (!option_s_s(bt, &o.statedir)) return 0;
  if (!option_s_s(bt, &o.usestatusfile)) return 0;
  if (!option_s_s(bt, &o.tundev)) return 0;
  if (!option_s_s(bt, &o.dynip)) return 0;
  if (!option_s_s(bt, &o.statip)) return 0;
  if (!option_s_s(bt, &o.ethers)) return 0;

  if (!option_s_s(bt, &o.domain)) return 0;
  if (!option_s_s(bt, &o.ipup)) return 0;
  if (!option_s_s(bt, &o.ipdown)) return 0;
  if (!option_s_s(bt, &o.conup)) return 0;
  if (!option_s_s(bt, &o.condown)) return 0;
  if (!option_s_s(bt, &o.macup)) return 0;
  if (!option_s_s(bt, &o.macdown)) return 0;
#ifdef ENABLE_IEEE8021Q
  if (!option_s_s(bt, &o.vlanupdate)) return 0;
#endif
#ifdef ENABLE_PROXYVSA
  if (!option_s_s(bt, &o.locationupdate)) return 0;
#endif

  if (!option_s_s(bt, &o.radiussecret)) return 0;
#ifdef ENABLE_LARGELIMITS
  if (!option_s_s(bt, &o.radiusacctsecret)) return 0;
  if (!option_s_s(bt, &o.radiusadmsecret)) return 0;
#endif

  if (!option_s_s(bt, &o.radiusnasid)) return 0;
  if (!option_s_s(bt, &o.radiuslocationid)) return 0;
  if (!option_s_s(bt, &o.radiuslocationname)) return 0;
  if (!option_s_s(bt, &o.locationname)) return 0;
  if (!option_s_s(bt, &o.proxysecret)) return 0;

  if (!option_s_s(bt, &o.dhcpif)) return 0;
#ifdef ENABLE_MULTILAN
  for (i=0; i < MAX_MOREIF; i++) {
    if (!option_s_s(bt, &o.moreif[i].dhcpif)) return 0;
    if (!option_s_s(bt, &o.moreif[i].vlan)) return 0;
  }
#endif
  if (!option_s_s(bt, &o.routeif)) return 0;
  if (!option_s_s(bt, &o.peerkey)) return 0;

  if (!option_s_s(bt, &o.macsuffix)) return 0;
  if (!option_s_s(bt, &o.macpasswd)) return 0;

  if (!option_s_s(bt, &o.uamsecret)) return 0;
  if (!option_s_s(bt, &o.uamurl)) return 0;
  if (!option_s_s(bt, &o.uamaaaurl)) return 0;
  if (!option_s_s(bt, &o.uamhomepage)) return 0;
  if (!option_s_s(bt, &o.wisprlogin)) return 0;

  if (!option_s_s(bt, &o.wwwdir)) return 0;
  if (!option_s_s(bt, &o.wwwbin)) return 0;
  if (!option_s_s(bt, &o.uamui)) return 0;
  if (!option_s_s(bt, &o.localusers)) return 0;
#ifdef HAVE_SSL
  if (!option_s_s(bt, &o.sslkeyfile)) return 0;
  if (!option_s_s(bt, &o.sslkeypass)) return 0;
  if (!option_s_s(bt, &o.sslcertfile)) return 0;
  if (!option_s_s(bt, &o.sslcafile)) return 0;
#endif
#ifdef USING_IPC_UNIX
  if (!option_s_s(bt, &o.unixipc)) return 0;
#endif
#ifdef HAVE_NETFILTER_COOVA
  if (!option_s_s(bt, &o.kname)) return 0;
#endif
#ifdef ENABLE_DNSLOG
  if (!option_s_s(bt, &o.dnslog)) return 0;
#endif
#ifdef ENABLE_IPWHITELIST
  if (!option_s_s(bt, &o.ipwhitelist)) return 0;
#endif
#ifdef ENABLE_UAMDOMAINFILE
  if (!option_s_s(bt, &o.uamdomainfile)) return 0;
#endif
#ifdef ENABLE_MODULES
  if (!option_s_s(bt, &o.moddir)) return 0;
#endif

  if (!option_s_s(bt, &o.adminuser)) return 0;
  if (!option_s_s(bt, &o.adminpasswd)) return 0;
  if (!option_s_s(bt, &o.adminupdatefile)) return 0;
  if (!option_s_s(bt, &o.rtmonfile)) return 0;

  if (!option_s_s(bt, &o.ssid)) return 0;
  if (!option_s_s(bt, &o.vlan)) return 0;
  if (!option_s_s(bt, &o.nasmac)) return 0;
  if (!option_s_s(bt, &o.nasip)) return 0;
  if (!option_s_s(bt, &o.cmdsocket)) return 0;

  if (!option_s_s(bt, &o.uamaliasname)) return 0;
  if (!option_s_s(bt, &o.uamhostname)) return 0;

#ifdef ENABLE_REDIRINJECT
  if (!option_s_s(bt, &o.inject)) return 0;
  if (!option_s_s(bt, &o.inject_ext)) return 0;
#endif

  for (i = 0; i < MAX_UAM_DOMAINS; i++) {
    if (!option_s_s(bt, &o.uamdomains[i])) 
      return 0;
  }

#ifdef EX_OPTIONS_SAVE
#include EX_OPTIONS_SAVE
#endif

  oldmask = umask(022);

  fd = open(file, O_RDWR | O_TRUNC | O_CREAT, 0666);

  umask(oldmask);

  if (fd <= 0) {

    log_err(errno, "could not save to %s", file);

    return 0;

  } else {
    if (safe_write(fd, &o, sizeof(o)) < 0)
      log_err(errno, "write()");

    size_t len = bt->slen;

    if (safe_write(fd, &len, sizeof(len)) < 0)
      log_err(errno, "write()");

    if (safe_write(fd, bt->data, len) < 0)
      log_err(errno, "write()");

    options_md5(&o, cksum);

    if (safe_write(fd, cksum, sizeof(cksum)) < 0)
      log_err(errno, "write()");

    close(fd);

    if (_options.uid) {
      if (chown(file, _options.uid, _options.gid)) {
	log_err(errno, "could not chown() %s", 
		_options.binconfig);
      }
    }
  }

  return 1;
}

int options_binload(char *file) {
  int fd = open(file, O_RDONLY);
  int ok = 0;
  if (fd > 0) {
    bstring bt = bfromcstr("");
    log_dbg("PID %d loading binary options file %s", getpid(), file);
    ok = options_fromfd(fd, bt);
    bdestroy(bt);
    return ok;
  }
  return ok;
}

int process_options(int argc, char **argv, int minimal) {

  /*
   *  If ran with arguments besides the load file, then pass
   *  off the arguments to chilli_opt for processing. If chilli_opt
   *  returns true, then we'll also start the server. 
   *
   */

  mode_t process_mask = umask(0077);
  int i;

  for (i=0; i < argc; i++) {
    if (!strcmp(argv[i],"-b")) {
      if (i+1 < argc) {
	return options_binload(argv[i+1]);
      }
    }
  }
  
  umask(process_mask);
  return !reload_options(argc, argv);
}

void reprocess_options(int argc, char **argv) {
  opt_run(argc, argv, 1);
}

int reload_options(int argc, char **argv) {
  bstring bt = bfromcstr("");
  int ok = options_load(argc, argv, bt);
  log_dbg("PID %d reloaded binary options file", getpid());
  bdestroy(bt);
  return ok;
}

void options_destroy() {
  if (_options._data) 
    free(_options._data);
}

void options_cleanup() {
  char file[128];

#ifdef ENABLE_MODULES
  int i;
  for (i=0; i < MAX_MODULES; i++) {
    if (!_options.modules[i].name[0]) break;
    if (!_options.modules[i].ctx) continue;
    else {
      struct chilli_module *m = 
	(struct chilli_module *)_options.modules[i].ctx;
      if (m->destroy)
	m->destroy(0);
    }
    log_dbg("Unloading module %s",_options.modules[i].name);
    chilli_module_unload(_options.modules[i].ctx);
  }
#endif

  chilli_binconfig(file, sizeof(file), getpid());
  log_dbg("Removing %s", file);
  if (remove(file)) log_dbg("remove(%s) failed", file);
  options_destroy();
}

