Change Log
==========

All notable changes to the project are documented in this file.


[v2.7][] - 2020-03-22
---------------------

### Changes
- Issue #301: Add `broken-rtc = <true | false>` .conf file setting, by
  Vladislav Grishenko
- Issue #302: Add common authentication failure error code handling, by
  Vladislav Grishenko
- Issue #308: Improve Dockerfile by using the same commit of the current
  Dockerfile to to build inadyn.  Previously the Dockerfile always pulled
  the latest git master, by Dominik Courcelles

### Fixes
- Fix #300: `--force` option not being recognized, by Eric Sauvageau
- Fix #305: Fix hash generation regression in FreeDNS plugin, found and
  fixed by Eric Sauvageau and Vladislav Grishenko


[v2.6][] - 2020-02-22
---------------------

**NOTE:** The `-1, --once` mode has changed semantics, it no longer
    defaults to forced update, for that you now need `--force`

### Changes
- Add support for Cloudflare, by Simon Pilkington and Jo Rhett
- Add Yandex PDD (Yandex.Connect) plugin, by Timur Birsh
- Add Dockerfile for running In-a-Dyn from an Alpine container, by
  Jean-Ralph Aviles with fixes by Robin Bürkli and Richard Powell
- Drop support for TZO, acqured by Dyn in 2012 and no longer works
- Drop support for DtDNS, shut down August 1, 2018
- Drop support for DynSIP, dropped off the Internet in 2014
- Drop support for Zerigo, shut down in 2017

### Fixes
- Fix #222: Simplify `utimensat()` replacement for macOS Sierra
- Fix #223: Sync up with accepted Homebrew formula, by Jo Rhett
- Fix #231: Update build requirements and recommendations
- Fix #233: Fix building with OpenSSL >1.1, by Rosen Penev
- Fix #235: dynv6, don't signal error on unchanged address
- Fix #239: Boundary check of MAX supported hostname aliases,
  bumped max aliases from 20 -> 50 per provider
- Fix #241: OpenBSD header ordering, tested on OpenBSD 6.1
- Fix #242: Mention libtool requirement when building from GIT
- Fix #247: Check if PID is actually running if PID file exists
- Fix internal buffer size warnings, found by GCC-9 (freeDNS)
- Fix #256: Document how to use Google Domains plugin
- Fix #262: Use .com TLD instead of .org for DynDNS, because the latter
  cannot be resolved in China, by Eric Sauvageau
- Fix #268: Set SIGCHLD to SIG_IGN to reap children, by Markus Gothe
- Fixes to socket leaks, memory corrupions, buffer overflows and more,
  found by Coverity Scan
- Fix configure script to allow `--prefix=` for install to `/sbin`
- Fix #274: D.U.I.A. DNS basic authentication, by Markus Gothe
- Fix #276: Change `--once` behavior, now requires `--force` to update
- Fix default checkip server for PubYun/3322, bliao.com not working.
  This patch adds support for the official 3322 checkip server
- Fix #277: See #235 but also for IPv4 ...
- Fix #297: api.ipify.org default checkip server for custom providers


[v2.5][] - 2018-09-30
---------------------

### Changes
- macOS changes by Jo Rhett:
  - Add linking with `-lresolv`
  - Use Homebrew's CA trust store
  - Update REDAME with install help
- Add support for selfhost.de DDNS

### Fixes
- Fix #211: Only show DDNS server response on successful transaction
- Fix #211: Improved error handling in OpenSSL back-end
- Fix #214: Add `nochg` to list of good responses for custom providers
- Fixes by Eric Sauvageau:
  - Fix #216: Add DNS lookup exception for `all.dnsomatic.com`
  - Fix #219: Add DNS lookup exception for `default@tunnelbrooker.net`


[v2.4][] - 2018-08-18
---------------------

### Changes
- Add support for Dynu DDNS provider

### Fixes
- Add missing defines for `LLONG_MAX` and `LLONG_MIN` on some platforms
- Fix #209: Update FreeDNS plugin to use v2 of their API to fetch update key
- Fix #210: Use `~/.cache/inadyn` or `~/.inadyn` when running unprivileged


[v2.3.1][] - 2018-02-12
-----------------------

This minor bug fix release holds Debian packaging fixes by André Colomb.

### Changes
- Make .deb files an official part of releases

### Fixes
- Fix installation of `inadyn` in `/usr/sbin` and symlink in `/usr/bin`
- Rename debian/inadyn.links to be standards-compliant
- Update deprecated build dependency for dh-systemd
- Fix lintian warning about unsafe symlinks for build scripts
- Version numbers containing a dash are inappropriate for 'native'
  packages, bump revision instead


[v2.3][] - 2018-01-05
---------------------

### Changes
- Distribute `CONTRIBUTING.md` in release tarballs, by André Colomb
- Clean up debug messages for HTTPS connections, by André Colomb
- New build-depends, `libgnutls28-dev` for Debian/Ubuntu users and
  GnuTLS >= 3.0 for others, by André Colomb
- Issue #192: Add `examples/*.conf` to source distribution, by André Colomb

### Fixes
- TCP, not UDP, for `getaddrinfo()` hints + numeric lookups, by André Colomb
- Disable SSL for checkip connections to SPDYN service, by André Colomb
- Issue #186: Allow IPv6 for HTTP(S) connections, by André Colomb
- Issue #189: Ignore premature session termination in GnuTLS, by André Colomb
- Issue #193: Fix broken internal links in README.md, by André Colomb


[v2.2.1][] - 2017-10-06
-----------------------

### Fixes

- Issue #174: `gnutls.c` missing `stdint.h`, fix for ArchLinux
- Issue #179: Update easyDNS plugin to new API, by Nicholas Alipaz


[v2.2][] - 2017-08-09
---------------------

### Changes
- Use HTTP by default for DYN.com checkip server, used by many DDNS
  providers that do not have their own.  This change is far more user
  friendly since you no longer have to explicitly set `checkip-ssl =
  false` for the most common use-case.
- Some DDNS providers have multiple IP addresses registered for the same
  service, as of this release Inadyn immediately tries to connect to the
  next listed addresses on connection problems.
- Issue #153: Support for custom HTTP User Agent.  Useful with providers
  that require using a specific brower.  Set to, e.g. "Mozilla/4.0", or
  rely on the default "inadyn/VERSION" user agent.
- Support for the `%%` format specifier in custom server URL's, as
  mentioned in issue #152.
- Add support for a `.conf` syntax checker: `inadyn --check-config`
- Add support for logging to `stderr` when running in foreground or
  without syslog enabled
- Simplified provider name lookup in `.conf` file.  Now substring match
  is used, resulting in support for `provider Dyn { ... }`.
- Remove libite dependency by importing all its used files into inadyn.
  This should ease adoption by distributions and end users.  All code
  is under free licenses: BSD, ISC.
- Import Timur's Debian packaging, adding debconf support

### Fixes
- Issue #152: Do not attempt to create PID file in oneshot mode (`-1`)
- Issue #152: Must URL encode custom server URL's
- Issue #170: Use configured `--prefix` not hard coded `/etc/inadyn.conf`
- Issue #172: Use separate variable for `--iface` command line option and
  `.conf` file option


[v2.1][] - 2016-12-04
---------------------

### Changes
- Use HTTPS instead of HTTP by default
- Support for disabling HTTPS for `checkip-server`, per provider.
  Idea from Valery Frolov
- Add `-I,--ident=NAME` option for syslog+pidfile name
- Deprecate `--pidfile=NAME` option in favor of `--ident=NAME`

### Fixes
- Issue #150: Custom update URL parser fixes
- Issue #151: Support for detecting OpenSSL v1.1
- Issue #144: Clarify use of public vs private IP.  It is possible
  to register private IP addresses in a public DNS
- Clarify `--foreground` option in man page
- Document minimum required versions of libite and libConfuse
- Portability fixes, replace `__progname` with a small function,
  replace `%m` with `%s` and `strerror(errno)`.


[v2.0][] - 2016-09-12
---------------------

New configuration file format, changed command line options, improved
HTTPS support using GnuTLS and Open/LibreSSL.  Inadyn now comes with
certificate validation enabled by default.

### Changes
- New configuration file format using [libConfuse][]
- Radically simplified command line, a .conf file is now required
- Reorganized SSL code, split `ssl.c` into `openssl.c` and `gnutls.c`
- Strict HTTPS certificate validation is now default.  To disable this
  use `strict-ssl = false` in the .conf file.
- Certificate validation uses trusted CA certificates from the system
  with fall-backs to certain known locations.  To override this default
  handling a `ca-trust-file = FILE` setting in `inadyn.conf` can be used
  to provide the path to another CA cert bundle, in PEM format.
- Massive overhaul of `inadyn(8)` and `inadyn.conf(5)` man pages
- Support for reading address from interface, including IPv6 addresses
- Support for calling an external script to get the IP address
- Support for multiple users @ same provider, idea from Valery Frolov:

        provider default@no-ip.com:1 {
            username    = ian
            password    = secret
            alias       = flemming.no-ip.com
        }
    
        provider default@no-ip.com:2 {
            username    = james
            password    = bond
            alias       = spectre.no-ip.com
        }

- Support for ddnss.de and dynv6.com, contributed by Sven Hoefer
- Support for spdyn.de, on request from Frank Röhm
- Support for strato.com, contributed by Duncan Overbruck
- Support for disabling IP address validation: `verify-address = false`
- Refactored memory handling and privilige separation to simplify code
- Refactored logging and backgrounding to simplify code
- Removed old compatibility symlinks and other required GNU specific
  files, we now distribute and install README.md and ChangeLog.md

### Fixes
- Fix issue #61: Add HTTPS certificate validation for OpenSSL/LibreSSL
- Fix issue #67: Use GnuTLS native API for HTTPS
- Fix DuckDNS: now requires 'www.' prefix in server URL.  By Frank Aurich
- Fix issue #110: Poodle `SSL_MODE_SEND_FALLBACK_SCSV` not needed
- Fix issue #101: Remove support for custom pidfile
- Fix issue #102: Relocate cache files `/var/run/inadyn` to `/var/cache/inadyn`
- Fix issue #113: `--drop-privs` does not work
- Add actual permissions check to `os_check_perms()`
- Fix issue #121: Support for fully customizable update URL
- Fix issue #122: Only use HTTPS connection for DNS update, not checkip
- Fix issue #131: Use FreeDNS' own checkip server instead of DYN.com's
- Fix issue #134: Support wildcard cert with GnuTLS backend


[1.99.15][] - 2015-09-09
------------------------

Minor bugfix release.

### Changes
- Support for new API at https://tunnelbroker.net, fixes issue #83.
  Use `default@tunnelbroker.net` to use the DYN.com API to update
  the IPv4 address for your IPv6-in-IPv4 tunnel.  Thanks goes to
  Horst Venzke @hvenzke for reporting this problem!
- The old API for the IPv6-in-IPv4 system `ipv6tb@he.net` is now
  deprecated.  Users should migrate to `default@tunnelbroker.net`
- Files generated by the GNU Configure & Build System is now no longer
  stored in GIT.  Instead, users that rely on GIT must run the new
  `./autogen.sh` script to generate the necessary files (`configure`).


### Fixes
- Fix issue #100: regression from [1.99.13][] pidfile is no longer
  created.  Inadyn 1.x semantics incompatible with OpenBSD `pidfile()`
  that replaced local version in [1.99.14][].  Problem found by David
  Schury @daersc.
- Fix issue #107: If an IP address update fails, e.g. due to temporary
  connectivity, HTTP transmission problems, etc. then Inadyn now forces
  an update in the next IP check cycle.  (This is the configurable
  `period` interval in `inadyn.conf`.)  Reported by Oliver Graute
  @redbrain17 and audited by @BulldozerBSG, thanks!
- Fix issue #108: Update README with correct alias syntax for Namecheap,
  issue reported by @quazar0


[1.99.14][] - 2015-07-14
------------------------

Improved support for configuring custom DDNS providers and support for
running in Windows, using Cygwin!

### Changes
- New setting `append-myip` which, instead of appending your hostname
  alias, appends the current IP to the server GET update URL.  See
  [README][README.md] or the man pages for more details.
- Prevent Inadyn from bugging out if it cannot write a cache file when
  the `-o, --once` flag is given.
- Inadyn now defaults to a silent build, use V=1 (like Linux) to get a
  verbose build.  Useful for auto-builders etc.
- Migrate to [libite][] for functions like `pidfile()`, `strlcpy()` etc.
- Add support for <http://GiraDNS.com>, thanks to Thorsten Mühlfelder!
- Add support for <https://www.duiadns.net>, thanks to Ionut Slaveanu!
- Add Cygwin support for running Inadyn in Windows, thanks to Scott Mann!

### Fixes

- Issue #89: Fix Duck DNS support, thanks to Ismani Nieuweboer!
- Issue #82: No rule to build target CHANGELOG, regression introduced in
  [1.99.13][].
- Issue #84: Sanitized default logs by placing conditions for debug
  logs.  Big thanks to Frank Aurich for this work!


[1.99.13][] - 2015-02-08
------------------------

### Changes
- Add support for <https://domains.google.com> DDNS, by Justin McManus
- Add support for <https://www.ovh.com> DDNS service, by Andres Gomez
- Add support for <http://dtdns.com> DDNS, by Denton Gentry
- Rename `NEWS.md` to `CHANGELOG.md` and update formatting in an attempt
  to align with <http://keepachangelog.com> -- this also means using ISO
  date format, finally!

### Fixes
- Fix issue #78: Parsing checkip responses may fail, by Andy Padavan
- Fix issue #79: Don't treat inline '#' as comment if not preceeded by
  space, by Andy Padavan


[1.99.12][] - 2014-10-20
------------------------

### Changes
- Add custom support for <http://Namecheap.com>, by Terzeus S. Dominguez

### Fixes
- Fix cross compilation issues with OpenSSL (depends on libcrypto)
- Fix cross compilation issues with `malloc` vs. `rpl_malloc` (removed
  autoconf check completely)


[1.99.11][] - 2014-10-15
------------------------

### Changes
- Add support for <https://nsupdate.info>, thanks to Thomas Waldmann
- Add support for <https://www.loopia.com> DynDNS service extension
- Add support for <https://duckdns.org>, thanks to Andy Padavan
- Updated man pages, both `inadyn(8)` and `inadyn.conf(5)` with examples

### Fixes
- Fix building on FreeBSD by converting to use GNU Configure & Build system
- Fixes to add support for TLS 1.x with SNI, thanks to Thomas Waldmann
- SSL mitigation fixes for POODLE


[1.99.10][] - 2014-09-13
------------------------

### Changes
- Refactor string functions `strcat()` and `strcpy()` to use secure
  OpenBSD replacements `strlcat()` and `strlcpy()` instead.

### Fixes
- Fix issue #57: `snprintf()` causes loss of \= from password string
- Fix issue #58: Add support for GnuTLS as the default SSL library
- Fix bugs found by Coverity Scan
- Fix memory leaks found by GLIBC (!) on PowerPC
- Fix include order problem with `error.h`


[1.99.9][] - 2014-05-21
-----------------------

### Changes
- Support for <http://www.zerigo.com> DDNS provider
- Support for <http://www.dhis.org> DDNS provider

### Fixes
- Fix memory leak in new HTTPS support, found by Valgrind
- Other misc. Valgrind and Cppcheck fixes


[1.99.8][] - 2014-05-20
-----------------------

### Changes
- Support for HTTPS to secure login credentials at DNS update, issue #36
- Support for persistent cache files with new `--cache-dir PATH`
- Support for <http://twoDNS.de> in generic plugin, see
  [README][README.md] for details
- Man page updates


[1.99.7][] - 2014-05-14
-----------------------

### Changes
- Support for multiple cache files, one per DDNS provider, issue #35
- Refactor DDNS providers as plugins, issue #30


[1.99.6][] - 2013-12-25
-----------------------

### Changes
- Update documentation for custom servers and add missing compatibility
  entry for custom servers.

### Fixes
- Fix nasty socket leak.


[1.99.5][] - 2013-11-27
-----------------------

### Changes
- Support for `--fake-address` on new `SIGUSR1` (forced update)
- Support for `SIGUSR2` (check now),
- Support for `--startup-delay SEC`, for embedded systems
- Man page updates

### Fixes
- Many minor bug fixes


[1.99.4][] - 2013-08-08
-----------------------

This release fixes a base64 password encoding regression in [1.99.3][]


[1.99.3][] - 2013-07-15 [YANKED]
--------------------------------

This release adds the ability to specify the cache file and the ability
to check the IP of the interface (UNIX only).  If no interface is
specified, no external IP check is performed.  The old `--iface` option
has been renamed `--bind`. changeip.com support has been added.  Minor
bugfixes and code optimizations have been made.

### Changes
- Add ability to specify cache file
- Add ability to check IP of interface (UNIX only).  If interface is
  specified, no external IP check performed Old `--iface` option renamed
  to `--bind`
- Specify IP address in freedns.afraid.org update request (only
  autodetect was used)
- Add changeip.com support

### Fixes
- Minor bugfixes and code optimization


[1.99.2][] - 2012-09-07
-----------------------

### Changes
- Get HTTP status description

### Fixes
- Fix inability to change update period (broken in 1.99.0)
- Fix debug output description


[1.99.1][] - 2012-09-01
-----------------------

### Changes
- Make HTTP status code check server-specific
- Update maintainer e-mail address


[1.99.0][] - 2012-08-17
-----------------------

### Changes
- Merge wl500g patches from <http://code.google.com/p/wl500g>:
  - `120-heipv6tb.patch` adds support for tunnelbroker
  - `121-hedyndns.patch` adds support for HE dyndns
  - `210-wildcard.patch` makes wildcard option account specific
- For ddns services that have their own checkip service, use it instead of
  dyndns.org checkip service
- Add ability to handle non-fatal temporary errors ("update too often",
  system error etc.)
- Warn if initial DNS request failed
- Add dnsexit.com support
- Modify http client to parse response for http status code and response
  body
- Remove DynDNS ignored and deprecated parameters (wildcard, mx, backmx,
  system). Wildcard kept for easydns.com
- Report detected IP to sitelutions and dynsip servers (only autodetect
  was used)
- Update TZO support
- Check HTTP status code before validating response
- Remake zoneedit response validation
- Little code cleanup

### Fixes
- Fix malformed HTTP request


[1.98.1][] - 2011-07-18
-----------------------

### Changes
- Preserve time since last update (forced update counter) and num
  interations from being reset by `SIGHUP` restart command
- Extend `--drop-privs` to support hyphens
- Cleanup of inadyn log messages, reformat & clarification

### Fixes
- Bug fix segfault at initial DNS lookup
- Bug fix `--drop-privs` uid/gid was swapped and a possible buffer overflow
- Typo fixes and polish to man pages inadyn(8) and inadyn.conf(5)


[1.98.0][] - 2011-02-28
-----------------------

### Changes
- New config file, command line, syntax (still backwards compatible!)
- New option `--drop-privs USER[:GROUP]` to support privilege separation
- Drop privileges before creating any files
- Documentation updates


[1.97.4][] - 2010-11-02
-----------------------

### Changes
- Support for dynsip.org by milkfish, from DD-WRT
- Add support for sitelutions.com, from inadyn-mt (untested)

### Fixes
- Clear DNS cache before calling `getaddrinfo()`, fixes GitHub issue #3


[1.97.3][] - 2010-11-02
-----------------------

### Changes
- Merge wl500g patches from <http://code.google.com/p/wl500g>:
  - `101-http-request.patch`. This cleans up the DDNS server defintions
	and callbacks, evidently originating from ideas implemented by
	DD-WRT.
  - `102-zoneedit.patch`. This fixes issues in what appears to be both
	request and response formats in ZoneEdit DDNS. Originating from
	DD-WRT.
  - `103-tzo.patch`. This patch adds support for tzo.com DDNS serivices.
  - `110-dnsomatic.patch`.  This patch adds support for DNS-O-Matic
	<http://dnsomatic.com/>, an OpenDNS service which can act as a
	"meta" update to several other DDNS service providers.
  - `120-heipv6tb.patch`. This patch adds support for Hurricane Electric's
	http://tunnelbroker.net/ DDNS services <https://dns.he.net/>.
- When starting: always use cache file, if it exists, or seed with DNS
  lookup

### Fixes
- Fix Debian bug #575549: freedns.afraid.org example in `inadyn(8)` is
  incorrect.


[1.97.2][] - 2010-10-30
-----------------------

### Changes
- Replace `gethostbyname()` with `getaddrinfo()` and improve logging at
  `connect()`

### Fixes
- Fix missing man pages from install/uninstall targets
- Fix GitHub issue #2: `setsocktopt()` takes pointer to `struct
  timeval`, not `int` as argument


[1.97.1][] - 2010-10-19
-----------------------

### Changes
- Add support for properly restarting inadyn on `SIGHUP`
- Remove `INADYN:` prefix in `MODULE_TAG` entirely - messes up syslog
  output


[1.97.0][] - 2010-10-18
-----------------------

- Apply patches by Neufbox4 from <http://dev.efixo.net>:
  - `100-inadyn-1.96.2.patch`, cache file support
  - `100-inadyn-1.96.2.patch`, bind interface support
  - `200-inadyn-1.96.2-64bits-fix.patch`
  - `300-inadyn-1.96.2-pidfile-and-improve.patch`
- New [README][README.md], COPYING and LICENSE file, remove readme.html
- Refactor and cleanup Makefile (renamed from makefile)
- Add support for `SIGTERM` signal
- Relocate include files to include directory
- Apply patch for multiple accounts from Christian Eyrich
- Remove unused `uint typedef` causing trouble on ARM GCC 4.4.x
- Fix missing `strdup()` of input config file and free any preexisting
- Make sure `TYPE_TCP` enum relly is 0 on all compilers
- Improve error messages using `strerror()` and use `-1` as stale
  socket, not `0`
- Fix nasty socket leak
- Merge with inadyn-advanced, <http://sf.net/projects/inadyn-advanced>:
  - Add support for 3322.org and easydns.org
  - Add support for domain wildcarding, `--wildcard` option NOTE: Domain
    wildcarding is now *disabled* by default
  - Add support for running an external command hook on IP update, new
	`--exec` option
  - Add support for datetime in debug messages
- Refactor `DBG_PRINTF((..))` --> `logit(..)`
- Update man page `inadyn(8)` with info on `--bind_interface`,
  `--wildcard`, `--exec` options and support for easydns.org
  and 3322.org services
- Misc fixes and cleanups


1.96.2 - 2007-03-12
-------------------

### Fixes
- If the Dynamic DNS server responds with an error Inadyn will abort.
  This will prevent further retries with wrong dyndns credentials.
- Default port number included in the request, to support the requests
  via Proxy, to ports different than 80.
- Simplified main inadyn update loop function. (there was no bug there)


1.96 - 2005-09-09
-----------------

### Changes
- zoneedit.com supported.
- no-ip.com supported.
- support for generic DNS services that support HTTP updates

### Fixes
- Immediate exit in case of --iterations=1 (not after a sleep period)
- Add missing option for specifying the path in the DNS server


1.95 - 2005-07-20
-----------------

### Changes
- UNIX signals supported - inadyn will stop gracefully in case of ALRM,
  HUP, INT, ...
- New dynamic DNS service supported - www.zoneedit.com - Not tested!
- Makefile adjusted for Solaris - compilable under Solaris.
- Support for generic DYNDNS service that supports update via HTTP with
  basic authentication not yet fully complete. Not known where might be
  applicable.


1.90 - 2005-02-24
-----------------

### Changes
- New option `--change_persona uid:gid` - inadyn can switch the user
  after launch. Typical feature for daemons
- Addition to `--ip_server_name` feature, now it has another parameter:
  the URL to read from the given server name
- Reduced some error messages
- Manual pages updated. (thanks to Shaul Karl)

### Fixes
- Typo fixed (`--ip_server_name` option)


1.86 - 2005-01-30
-----------------

### Changes
- Updated UNIX man pages for inadyn.  Even a page for `inadyn.conf`,
  Thanks to Shaul Karl!
- Inadyn doesn't print anything (e.g. ver. number) anymore when goes to
  background
- New config file parser. Accepts ESCAPE character: `\`

### Fixes
- Corrected check of the return code from `socket()` call


1.85 - 2005-01-10
-----------------

### Changes
- Config file related enhancements:
  - Use default location for config file, when no arguments for inadyn
  - More *NIX like format for the config file.  Thanks to Jerome Benoit

### Fixes
- When 'iterations' option is specified as being '1', inadyn exits
  immediately after first update, not after the sleep period as before


1.81 - 2004-11-23
-----------------

### Changes
- No new features, just a better integration with Linux.  Reviewed usage
  of syslog and fork to background in daemon mode, thanks to Shaul Karl.

1.80 - 2004-10-16
-----------------

### Changes
- Optional output to syslog for Linux, should work for all *nix systems
- Run in background for Linux, should work for all *nix systems

### Fixes
- Minor compile warnings removed


1.70 - 2004-07-05
-----------------

### Changes
- New `--iterations` cmd line option.  Now one can run inadyn with only
  one iteration.  Untested.  It was a debug option now made accessible
  via cmd line.  It should work.

### Fixes
- Custom DNS from dyndns option was not accepted by the cmd line parser.
  "Copy-paste" error :-(


1.60 - 2004-06-05
-----------------

### Changes
- On users' request the inadyn can read the options from file.


1.51 - 2004-05-03
-----------------

### Changes
- Support for more aliases for DNS service offered by freedns.afraid.org


1.5 - 2004-05-01
----------------

### Changes
- Support for dynamic DNS service offered by freedns.afraid.org
- Support for http proxy
- GPL copyright notice added.


1.4 - 2004-03-01
----------------

### Changes
- Support for custom DNS and static DNS services offered by dyndns.org
- Support for forced IP update, so the account will not expire even
  though the IP never changes


1.35 - 2004-02-04
-----------------

### Fixes
- Multiple aliases are AGAIN supported
- In case of error in IP update the OS signal handler is not installed again.


1.34 - 2003-11-06
-----------------

First port to *NIX - Linux running OK as console app.

### TODO
- Run as a background daemon in UNIX
- Better interface

### Fixes
- Various bug fixes.


1.2 - Jun 2003
--------------
Port to pSOS

**Note:** no DNS support under pSOS -- hard coded IP addresses of the
  server.


1.0 - 20013-05-20
-----------------

First stable version.

### Features
- DYNDNS client
- free
- works fine behind a NAT router
- runs fine as a service
- has a nice log file

### TODO
- port to *NIX
- port to pSOS


[UNRELEASED]: https://github.com/troglobit/inadyn/compare/v2.3...HEAD
[v2.4]:   https://github.com/troglobit/inadyn/compare/v2.3.1...v2.4
[v2.3.1]: https://github.com/troglobit/inadyn/compare/v2.3...v2.3.1
[v2.3]:   https://github.com/troglobit/inadyn/compare/v2.2.1...v2.3
[v2.2.1]: https://github.com/troglobit/inadyn/compare/v2.2...v2.2.1
[v2.2]:   https://github.com/troglobit/inadyn/compare/v2.1...v2.2
[v2.1]:   https://github.com/troglobit/inadyn/compare/v2.0...v2.1
[v2.0]:   https://github.com/troglobit/inadyn/compare/1.99.15...v2.0
[1.99.15]: https://github.com/troglobit/inadyn/compare/1.99.14...1.99.15
[1.99.14]: https://github.com/troglobit/inadyn/compare/1.99.13...1.99.14
[1.99.13]: https://github.com/troglobit/inadyn/compare/1.99.12...1.99.13
[1.99.12]: https://github.com/troglobit/inadyn/compare/1.99.11...1.99.12
[1.99.11]: https://github.com/troglobit/inadyn/compare/1.99.10...1.99.11
[1.99.10]: https://github.com/troglobit/inadyn/compare/1.99.9...1.99.10
[1.99.9]: https://github.com/troglobit/inadyn/compare/1.99.8...1.99.9
[1.99.8]: https://github.com/troglobit/inadyn/compare/1.99.7...1.99.8
[1.99.7]: https://github.com/troglobit/inadyn/compare/1.99.6...1.99.7
[1.99.6]: https://github.com/troglobit/inadyn/compare/1.99.5...1.99.6
[1.99.5]: https://github.com/troglobit/inadyn/compare/1.99.4...1.99.5
[1.99.4]: https://github.com/troglobit/inadyn/compare/1.99.3...1.99.4
[1.99.3]: https://github.com/troglobit/inadyn/compare/1.99.2...1.99.3
[1.99.1]: https://github.com/troglobit/inadyn/compare/1.99.1...1.99.2
[1.99.1]: https://github.com/troglobit/inadyn/compare/1.99.0...1.99.1
[1.99.0]: https://github.com/troglobit/inadyn/compare/1.98.1...1.99.0
[1.98.1]: https://github.com/troglobit/inadyn/compare/1.98.0...1.98.1
[1.98.0]: https://github.com/troglobit/inadyn/compare/1.97.4...1.98.0
[1.97.4]: https://github.com/troglobit/inadyn/compare/1.97.3...1.97.4
[1.97.3]: https://github.com/troglobit/inadyn/compare/1.97.2...1.97.3
[1.97.2]: https://github.com/troglobit/inadyn/compare/1.97.1...1.97.2
[1.97.1]: https://github.com/troglobit/inadyn/compare/1.97.0...1.97.1
[1.97.0]: https://github.com/troglobit/inadyn/compare/1.96.2...1.97.0
[libite]: https://github.com/troglobit/libite
[README.md]: https://github.com/troglobit/inadyn/blob/master/README.md
[libConfuse]: https://github.com/martinh/libconfuse
