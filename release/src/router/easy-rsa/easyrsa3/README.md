[![CI](https://github.com/Openvpn/easy-rsa/actions/workflows/action.yml/badge.svg)](https://github.com/Openvpn/easy-rsa/actions/workflows/action.yml)
# Overview

easy-rsa is a CLI utility to build and manage a PKI CA. In layman's terms,
this means to create a root certificate authority, and request and sign
certificates, including intermediate CAs and certificate revocation lists (CRL).

# Downloads

If you are looking for release downloads, please see the releases section on
GitHub. Releases are also available as source checkouts using named tags.

# Documentation

For 3.x project documentation and usage, see the [README.quickstart.md](README.quickstart.md) file or
the more detailed docs under the [doc/](doc/) directory. The .md files are in Markdown
format and can be converted to html files as desired for release packages, or
read as-is in plaintext.

# Getting help using easy-rsa

Currently, Easy-RSA development co-exists with OpenVPN even though they are
separate projects. The following resources are good places as of this writing to
seek help using Easy-RSA:

The [openvpn-users mailing list](https://lists.sourceforge.net/lists/listinfo/openvpn-users)
is a good place to post usage or help questions.

Users of GitHub can report problems to the Easy-RSA `issues` list.

# Branch structure

The easy-rsa master branch is currently tracking development for the 3.x release
cycle. Please note that, at any given time, master may be broken. Feel free to
create issues against master, but have patience when using the master branch. It
is recommended to use a release, and priority will be given to bugs identified in
the most recent release.

The prior 2.x and 1.x versions are available as release branches for
tracking and possible back-porting of relevant fixes.

Branch layout is:

    master             <- Active: v3.2.x - Rolling.
    v3.<N>.<N>-<LABEL>    Active: Development branches.
    testing               Sandbox: Subject to change without notice.
    v3.1.8                Sunset: Bugfix only for v3.1.7

    The following are NOT compatible with OpenSSL version 3:

    v3.0.6                Inactive: Archived.
    v3.0.5                Inactive: Archived.
    v3.0.4                Inactive: Archived.
    release/3.0           Inactive: Archived.
    release/2.x           Inactive: Archived.
    release/1.x           Inactive: Unmaintained.

LICENSING info for 3.x is in the [COPYING.md](COPYING.md) file

## Contributing

Please refer to: [doc/EasyRSA-Contributing.md](doc/EasyRSA-Contributing.md)

# Code style, standards

We are attempting to adhere to the POSIX standard, which can be found here:

https://pubs.opengroup.org/onlinepubs/9699919799/
