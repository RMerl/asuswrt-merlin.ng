
TODO: revise this to talk about how things are, rather than how things
have changed.

TODO: Make this into good markdown.



For quite a while now, the program "tor" has been built from source
code in just two directories: src/common and src/or.

This has become more-or-less untenable, for a few reasons -- most
notably of which is that it has led our code to become more
spaghetti-ish than I can endorse with a clean conscience.

So to fix that, we've gone and done a huge code movement in our git
master branch, which will land in a release once Tor 0.3.5.1-alpha is
out.

Here's what we did:

  * src/common has been turned into a set of static libraries.  These
all live in the "src/lib/*" directories.  The dependencies between
these libraries should have no cycles.  The libraries are:

    arch -- Headers to handle architectural differences
    cc -- headers to handle differences among compilers
    compress -- wraps zlib, zstd, lzma
    container -- high-level container types
    crypt_ops -- Cryptographic operations. Planning to split this into
a higher and lower level library
    ctime -- Operations that need to run in constant-time. (Properly,
data-invariant time)
    defs -- miscelaneous definitions needed throughout Tor.
    encoding -- transforming one data type into another, and various
data types into strings.
    err -- lowest-level error handling, in cases where we can't use
the logs because something that the logging system needs has broken.
    evloop -- Generic event-loop handling logic
    fdio -- Low-level IO wrapper functions for file descriptors.
    fs -- Operations on the filesystem
    intmath -- low-level integer math and misc bit-twiddling hacks
    lock -- low-level locking code
    log -- Tor's logging module.  This library sits roughly halfway up
the library dependency diagram, since everything it depends on has to
be carefully crafted to *not* log.
    malloc -- Low-level wrappers for the platform memory allocation functions.
    math -- Higher-level mathematical functions, and floating-point math
    memarea -- An arena allocator
    meminfo -- Functions for querying the current process's memory
status and resources
    net -- Networking compatibility and convenience code
    osinfo -- Querying information about the operating system
    process -- Launching and querying the status of other processes
    sandbox -- Backend for the linux seccomp2 sandbox
    smartlist_core -- The lowest-level of the smartlist_t data type.
Separated from the rest of the containers library because the logging
subsystem depends on it.
    string -- Compatibility and convenience functions for manipulating
C strings.
    term -- Terminal-related functions (currently limited to a getpass
function).
    testsupport -- Macros for mocking, unit tests, etc.
    thread -- Higher-level thread compatibility code
    time -- Higher-level time management code, including format
conversions and monotonic time
    tls -- Our wrapper around our TLS library
    trace -- Formerly src/trace -- a generic event tracing API
    wallclock -- Low-level time code, used by the log module.

  * To ensure that the dependency graph in src/common remains under
control, there is a tool that you can run called "make
check-includes".  It verifies that each module in Tor only includes
the headers that it is permitted to include, using a per-directory
".may_include" file.

  * The src/or/or.h header has been split into numerous smaller
headers.  Notably, many important structures are now declared in a
header called foo_st.h, where "foo" is the name of the structure.

  * The src/or directory, which had most of Tor's code, had been split
up into several directories.  This is still a work in progress:  This
code has not itself been refactored, and its dependency graph is still
a tangled web.  I hope we'll be working on that over the coming
releases, but it will take a while to do.

    The new top-level source directories are:

     src/core -- Code necessary to actually perform or use onion routing.
     src/feature -- Code used only by some onion routing
configurations, or only for a special purpose.
     src/app -- Top-level code to run, invoke, and configure the
lower-level code

   The new second-level source directories are:
     src/core/crypto -- High-level cryptographic protocols used in Tor
     src/core/mainloop -- Tor's event loop, connection-handling, and
traffic-routing code.
     src/core/or -- Parts related to handling onion routing itself
     src/core/proto -- support for encoding and decoding different
wire protocols

     src/feature/api -- Support for making Tor embeddable
     src/feature/client -- Functionality which only Tor clients need
     src/feature/control -- Controller implementation
     src/feature/dirauth -- Directory authority
     src/feature/dircache -- Directory cache
     src/feature/dirclient -- Directory client
     src/feature/dircommon -- Shared code between the other directory modules
     src/feature/hibernate -- Hibernating when Tor is out of bandwidth
or shutting down
     src/feature/hs -- v3 onion service implementation
     src/feature/hs_common -- shared code between both onion service
implementations
     src/feature/nodelist -- storing and accessing the list of relays on
the network.
     src/feature/relay -- code that only relay servers and exit servers need.
     src/feature/rend -- v2 onion service implementation
     src/feature/stats -- statistics and history

     src/app/config -- configuration and state for Tor
     src/app/main -- Top-level functions to invoke the rest or Tor.

  * The "tor" executable is now built in src/app/tor rather than src/or/tor.

  * There are more static libraries than before that you need to build
into your application if you want to embed Tor.  Rather than
maintaining this list yourself, I recommend that you run "make
show-libs" to have Tor emit a list of what you need to link.
