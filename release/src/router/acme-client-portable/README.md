## Synopsis

*acme-client-portable* is yet another
[ACME](https://letsencrypt.github.io/acme-spec/) client, specifically for
[Let's Encrypt](https://letsencrypt.org), but one with a strong focus on
security. 

It was named *letskencrypt-portable* until version 0.1.11.

Please see
[kristaps.bsd.lv/acme-client](https://kristaps.bsd.lv/acme-client) for
stable releases: this repository is for current development of the
portable branch, which tracks
[acme-client](https://github.com/kristapsdz/acme-client) with goop to
allow compilation and secure operation on Linux, Mac OS X, NetBSD, and
FreeBSD (hence "-portable").
You will need [libressl](http://www.libressl.org/) on all systems and
[libbsd](https://libbsd.freedesktop.org/wiki/) on Linux (except for
[musl](https://www.musl-libc.org) libc systems like
[Alpine](https://alpinelinux.org/)).

Linux has an experimental
[libseccomp](https://github.com/seccomp/libseccomp) sandbox, but you
must enable it yourself.  Details in
[Linux-seccomp.md](Linux-seccomp.md).

This repository mirrors the master CVS repository: any source changes
will occur on the master and be pushed periodically to GitHub.  If you
have bug reports or patches, either file them here or e-mail them to me.
Feature requests will be ignored unless joined by a patch.

What are the difference between this and the non-portable release?

* Conditional support for OpenBSD's sandbox, Mac OS X, or
  **experimentally** on Linux.
* Proper preprocessor flags for unlocking some Linux functions.
* Different library names on Linux.
* Uses GNU make instead of BSD make.

This version tries its best to be secure, but some of its supported
operating systems are hostile to security.

On both Linux and Mac OS X, for example, the DNS resolution process is
effectively run in the main file-system and un-sandboxed due to the
complexity of lookups (needing mDNSresponder in the latter case or a
slew of mystery files in the former).

Moreover, while the sandbox on Mac OS X (which is deprecated?) exists,
its behaviour is not well-documented and, morever, is weakened to
co-exist with the file-system jail.

**Feature requests will be ignored unless joined by a patch.**  If
there's something you need, I'm happy to work with you to make it
happen.  If you really need it, I'm available for contract (contact me
by e-mail).

## Configuration

Since your system might not be one of the tested ones (FreeBSD, Linux,
Linux with musl libc, etc.), you may need to tune some
of the values in the [GNUmakefile](GNUmakefile) or [config.h](config.h).
Please **tell me** if you do so, so I can accommodate in future
releases.

In the former, you can adjust system-specific compilation flags.

In the latter, you can set the `NOBODY_USER` value to be the name of an
unprivileged user for privilege dropping.
You can also set `DEFAULT_CA_FILE` for the location of the certificate
file loaded by libtls.
There's also `PATH_VAR_EMPTY`, which should be an empty directory into
which we can create our jail.

## License

Sources use the ISC (like OpenBSD) license.  See the
[LICENSE.md](LICENSE.md) file for details.

The [jsmn.c](jsmn.c) and [jsmn.h](jsmn.h) files use the MIT license.
See [https://github.com/zserge/jsmn](https://github.com/zserge/jsmn) for
details.
