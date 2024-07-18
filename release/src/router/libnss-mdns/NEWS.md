# News

## Sat Jun 12 2021:

[Version 0.15.1](https://github.com/lathiat/nss-mdns/releases/tag/v0.15.1)
released. Highlights:

* This fixes the broken previous release by restoring the missing
  `src/nss.h` file. If you are using 0.15, you must upgrade to this
  version, or downgrade to a previous one.

## Mon May 10 2021:

[Version 0.15](https://github.com/lathiat/nss-mdns/releases/tag/v0.15)
released. Highlights:

* Updated README.md for clarity
* The return of BSD support!
* Support for `AVAHI_SOCKET` in `/run` (instead of legacy `/var/run`)

## Sun Mar 18 2018:

[Version 0.14.1](https://github.com/lathiat/nss-mdns/releases/tag/v0.14.1)
released. Highlights:

* No code changes
* Fix unit tests to properly work on s390x

## Sun Mar 18 2018:

[Version 0.14](https://github.com/lathiat/nss-mdns/releases/tag/v0.14)
released. Highlights:

* Fix -Wformat-truncation problem during reading of the allow file

## Tue Feb 20 2018:

[Version 0.13.2](https://github.com/lathiat/nss-mdns/releases/tag/v0.13.2)
released. Highlights:

* No code changes
* Change how `./configure --enable/disable-tests` works:
  * `--enable-tests`: tests are enabled and will fail if dependencies are
    not found
  * `--disable-tests`: tests are not enabled and will not be built even
    if dependencies are found
  * no flag given: tests are conditionally enabled if dependencies are
    found

## Sun Feb 18 2018:

[Version 0.13.1](https://github.com/lathiat/nss-mdns/releases/tag/v0.13.1)
released. Highlights:

* Very minor code changes (should result in no binary changes)
* Reformat source to 80 columns
* Improve configure options to allow disabling tests even if
  the testing libraries are present
* Automake is now non-recursive
* Hardcoded paths are now exposed as configure variables

## Mon Feb 12 2018:

[Version 0.13](https://github.com/lathiat/nss-mdns/releases/tag/v0.13)
released. Highlights:

* Fix an old memory leak in reverse lookup
* Fix the broken workaround for nscd segfaults (not all clients
  would see all results)
* Simplify buffer management
* More unit tests, more cleanups, and fewer gotos

## Sat Feb 10 2018:

[Version 0.12](https://github.com/lathiat/nss-mdns/releases/tag/v0.12)
released. Highlights:

* Fix segfault when using nscd
* Remove untested, unmaintained BSD support (please help out if you
  would like BSD support to return!)

## Mon Jan 22 2018:

[Version 0.11](https://github.com/lathiat/nss-mdns/releases/tag/v0.11)
released. The first release in some time! Highlights:

* Moved to new GitHub location, docs migrated to markdown
* The long-deprecated `LEGACY` mode is removed
* The long-deprecated `HONOUR_SEARCH_DOMAINS` option is removed
* Unit tests are now included, with `make check`
* nss-mdns now implements [standard
  heuristics](https://support.apple.com/en-us/HT201275) for
  detecting `.local` unicast resolution and will automatically
  disable resolution when a local server responds to `.local` requests
* `_nss_mdns_gethostbyname3_r` and `_nss_mdns_gethostbyname4_r`
  are now implemented
* Full dual-stack IPv4/IPv6 support is implemented

## Sat May 12 2007:

[Version 0.10](https://github.com/lathiat/nss-mdns/releases/tag/v0.10)
released. Changes include: Ported to FreeBSD; alignment fixes for SPARC.

## Mon Jan 1 2007:

[Version 0.9](https://github.com/lathiat/nss-mdns/releases/tag/v0.9)
released. Changes include: Make most shared library symbols private to
not conflict with any symbols of the program we're loaded into. Fix a
potential endless loop in the mDNS packet parsing code.

**Please note that due to security reasons from this release on the
minimal mDNS stack included in `nss-mdns` (dubbed "legacy") is no
longer built by default. Thus, `nss-mdns` will not work unless
[Avahi](http://avahi.org/) is running! That makes Avahi essentially a
hard dependency of `nss-mdns`. Pass `--enable-legacy` to reenable the
mini mDNS stack again. Please note as well that this release does not
honour `/etc/resolv.conf` domain search lists by default anymore. It
created a lot of problems and was never recommended anyway. You may
reenable this functionality by passing `--enable-search-domains`.**

## Sat Apr 29 2006:

[Version 0.8](https://github.com/lathiat/nss-mdns/releases/tag/v0.8)
released. Changes include: Build time option to disable "legacy unicast" mDNS
requests, i.e. resolve exclusively with Avahi; build a special
`_minimal` flavour of the shared objects to minimize
unnecessary name lookup timeouts; fix IPv6 resolving when using
Avahi.

**Please note that starting with nss-mdns 0.8 we encourage you to use
a different `/etc/nsswitch.conf` configuration line. See below
for more information!**

## Sat Nov 19 2005:

[Version
0.7](https://github.com/lathiat/nss-mdns/releases/tag/v0.7)
released. Changes include: Portability patch for ARM from Philipp
Zabel; make sure not to print any messages to STDERR; deal with OOM
situations properly; if multiple addresses are assigned to the same
interface make sure to send a query packet only once; other cleanups

## Sun Aug 21 2005:

[Version 0.6](https://github.com/lathiat/nss-mdns/releases/tag/v0.6)
released. Changes include: honour search list in
`/etc/resolv.conf`; try to contact [Avahi](http://avahi.org/) for
resolving.

## Sat Jun 4 2005:

[Version 0.5](https://github.com/lathiat/nss-mdns/releases/tag/v0.5)
released. Changes include: only lookup hostnames ending in
`.local`; add support for a configuration file
(`/etc/mdns.allow`) to allow lookups for other names.

## Sun May 15 2005:

[Version 0.4](https://github.com/lathiat/nss-mdns/releases/tag/v0.4)
released. Changes include: small portability fix for big endian
architectures; send "legacy unicast" packets instead of normal mDNS
packets (this should reduce traffic and improve response time)

## Jan Sun 16 2005:

[Version
0.3](https://github.com/lathiat/nss-mdns/releases/tag/v0.3)
released. Changes include: add Debianization; use `ip6.arpa` instead
of `ip6.int` for reverse IPv6 lookups.

## Fri Dec 17 2004:

[Version 0.2](https://github.com/lathiat/nss-mdns/releases/tag/v0.2)
released. Changes include: send mDNS queries on every interface that
supports multicasts, instead of only the one with the default route,
making `nss-mdns` more robust on multi-homed hosts; gcc 2.95
compatiblity.

## Mon Dec 6 2004:

[Version 0.1](https://github.com/lathiat/nss-mdns/releases/tag/v0.1)
