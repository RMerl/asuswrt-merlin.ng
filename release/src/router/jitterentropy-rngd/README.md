Jitter RNG Daemon
=================

Using the Jitter RNG core, the rngd provides an entropy source that feeds
into the Linux /dev/random device if its entropy runs low. It updates the
/dev/random entropy estimator such that the newly provided entropy
unblocks /dev/random.

The seeding of /dev/random also ensures that /dev/urandom benefits from
entropy. Especially during boot time, when the entropy of Linux is low,
the Jitter RNGd provides a source of sufficient entropy.

By using the SP800-90B-compliant Jitter RNG core library, the RNGd itself
is now fully SP800-90B compliant.

Build Instructions
==================

To generate the shared library `make` followed by `make install`.

Usage
=====

See jitterentropy --help or see the man page jitterentropy-rngd.1.

Systemd Unit File
=================

A systemd unit file is provided with jitterentropy.service which can be
copied to /etc/systemd/system and enabled with the command
`systemctl enable jitterentropy`.

The unit file ensures that the Jitter RNGd is started as one of the first
daemons during the user space start process. This shall guarantee that
any cryptographic daemons, like sshd or a web server, benefits from a seeded
/dev/random and /dev/urandom device at the time they start up.

Docker
======

Run using docker-compose:

```
docker-compose up -d
```

Build and run manually:

```
docker build -t jitterentropy-rngd .
docker run --cap-add=SYS_ADMIN --cap-drop=ALL --name=rngd --network=none \
    --restart=always -d jitterentropy-rngd
```

Version Numbers
===============
The version numbers for this library have the following schema:
MAJOR.MINOR.PATCHLEVEL

Changes in the major number implies API and ABI incompatible changes, or
functional changes that require consumer to be updated (as long as this
number is zero, the API is not considered stable and can change without a
bump of the major version).

Changes in the minor version are API compatible, but the ABI may change.
Functional enhancements only are added. Thus, a consumer can be left
unchanged if enhancements are not considered. The consumer only needs to
be recompiled.

Patchlevel changes are API / ABI compatible. No functional changes, no
enhancements are made. This release is a bug fixe release only. The
consumer can be left unchanged and does not need to be recompiled.

Author
======
Stephan Mueller <smueller@chronox.de>
