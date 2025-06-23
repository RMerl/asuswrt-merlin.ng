![LibJWT Logo](https://user-images.githubusercontent.com/320303/33439880-82406da4-d5bc-11e7-8959-6d53553c1984.png)

# JWT C Library

[![Build Status](https://app.travis-ci.com/benmcollins/libjwt.svg?branch=master)](https://app.travis-ci.com/github/benmcollins/libjwt) [![codecov](https://codecov.io/gh/benmcollins/libjwt/graph/badge.svg?token=MhCaZ8cpwQ)](https://codecov.io/gh/benmcollins/libjwt)

[![View on JWT.IO](http://jwt.io/img/badge.svg)](https://jwt.io)

## Build Requirements

- https://github.com/akheron/jansson
- OpenSSL or GnuTLS

## Documentation

[GitHub Pages](https://benmcollins.github.io/libjwt/)

## Pre-built Packages

LibJWT is available in most Linux distributions as well as through [Homebrew](https://brew.sh/)
for macOS and Windows.

## Build Instructions

**With GNU Make:** Use ``autoreconf -i`` to create project files and run ``./configure``.
- ``make all``: build library.
- ``make check``: build and run test suite.
- See INSTALL file for more details on GNU Auto tools and GNU Make.
- Use the ``--without-openssl`` with ``./configure`` to use GnuTLS.
