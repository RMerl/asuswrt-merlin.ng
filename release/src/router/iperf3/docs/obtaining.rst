.. _obtaining:

Obtaining iperf3
================

Binary Distributions
--------------------

Binary packages are available for several supported operating systems:

* FreeBSD:  `benchmarks/iperf3
  <http://freshports.org/benchmarks/iperf3>`_ in the FreeBSD Ports Collection
* Fedora / CentOS: `iperf3
  <https://apps.fedoraproject.org/packages/iperf3/>`_ and
  `iperf3-devel
  <https://apps.fedoraproject.org/packages/iperf3-devel>`_ in Fedora
  19 and 20 and in Fedora EPEL 5, 6, and 7.

Source Distributions
--------------------

Source distributions of iperf are available as compressed (gzip)
tarballs at:

http://downloads.es.net/pub/iperf/

**Note:**  Due to a software packaging error, the 3.0.2 release
tarball was not compressed, even though its filename had a ``.tar.gz``
suffix.

**Note:**  GitHub, which currently hosts the iperf3 project, supports
a "Releases" feature, which can automatically generate ``.zip`` or ``.tar.gz``
archives, on demand, from tags in the iperf3 source tree.  These tags are
created during the release engineering process to mark the exact
version of files making up a release.

In theory, the ``.tar.gz`` files produced by GitHub contain the same
contents as what are in the official tarballs, note that the tarballs
themselves will be different due to internal timestamps or other
metadata.  Therefore these files will *not* match the published SHA256
checksums and no guarantees can be made about the integrity of the
files.  The authors of iperf3 always recommend downloading source
distributions from the the directory above (or a mirror site), and
verifying the SHA256 checksums before using them for any purpose, to
ensure the files have not been tampered with.

Source Code Repository
----------------------

The iperf3 project is hosted on GitHub at:

https://github.com/esnet/iperf

The iperf3 source code repository can be checked out directly from
GitHub using:

``git clone https://github.com/esnet/iperf.git``

Primary development for iperf3 takes place on CentOS 6 Linux, FreeBSD 10,
and MacOS X 10.8. At this time, these are the only officially supported
platforms, however there have been some reports of success with
OpenBSD, Android, and other Linux distributions.
