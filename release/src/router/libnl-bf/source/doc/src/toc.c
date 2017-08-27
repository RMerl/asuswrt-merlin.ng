/**
 * \cond skip
 * vim:syntax=doxygen
 * \endcond

\mainpage

\section main_intro Introduction

libnl is a set of libraries to deal with the netlink protocol and some
of the high level protocols implemented on top of it. The goal is to
provide APIs on different levels of abstraction. The core library libnl.so
provides a fundamental set of functions to deal with sockets, construct
messages, and send/receive those messages. Additional high level interfaces
for several individual netlink protocols are provided in separate
libraries (e.g. "nl-route.so", "nl-genl.so", ...).

The library is designed to ensure that all components are optional, i.e.
even though the core library provides a caching system which allows to
easly manage objects of any kind, no application is required to use this
caching system if it has no need for it.

The library was developed and tested on 2.6.x kernel releases. It may
or may not work with older kernel series. Also, although all netlink
protocols are required to maintain backwards compatibility, this has not
always achieved and undesired side effects can occur if a recent libnl
version is used with a considerably older kernel.

\section main_toc Table of Contents

\section main_trees GIT Trees

\subsection tree_dev Development Tree

@code
git://git.kernel.org/pub/scm/libs/netlink/libnl.git
@endcode
- Web: http://www.kernel.org/pub/scm/libs/netlink/libnl.git

\subsection tree_stable Stable Tree

@code
git://git.kernel.org/pub/scm/libs/netlink/libnl-stable.git
@endcode
- Web: http://www.kernel.org/pub/scm/libs/netlink/libnl-stable.git

\section main_website Website

- http://www.infradead.org/~tgr/libnl/

\section main_mailinglist Mailinglist

Please post question and patches to the libnl mailinglist:

@code
libnl@lists.infradead.org
@endcode

- Archives: http://canuck.infradead.org/pipermail/libnl/

*/
