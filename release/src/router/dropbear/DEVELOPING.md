# Developer Notes

## Building

See [INSTALL](INSTALL) for build instructions. 
[SMALL](SMALL) has hints for building smaller binaries, also see comments
in default_options.h.

## Debug printing

Set `#define DEBUG_TRACE 1` in localoptions.h to enable a `-v` option
for dropbear and dbclient. That prints various details of the session. For
development running `dropbear -F -E` is useful to run in the foreground. You
can set `#define DEBUG_NOFORK 1` to make dropbear a one-shot server, easy to 
run under a debugger.

## Random sources

Most cryptography requires a good random entropy source, both to generate secret
keys and in the course of a session. Dropbear uses the Linux kernel's
`getrandom()` syscall to ensure that the system RNG has been initialised before
using it. On some systems there is insufficient entropy gathered during early
boot - generating hostkeys then will block for some amount of time. 
Dropbear has a `-R` option to generate hostkeys upon the first connection 
as required - that will allow the system more time to gather entropy.

## Algorithms

Default algorithm lists are specified in [common-algo.c](common-algo.c).
They are in priority order, the client's first matching choice is used
(see rfc4253). 
Dropbear client has `-c` and `-m` arguments to choose which are enabled at
runtime (doesn't work for server as of June 2020).

Enabling/disabling algorithms is done in [localoptions.h](localoptions.h),
see [default_options.h](default_options.h).

## Non-root user

Dropbear server will run fine as a non-root user, allowing logins only for 
that user. Password authentication probably won't work (can't read shadow 
passwords). You will need to create hostkeys that are readable.

## Connection setup 

Dropbear implements first_kex_packet_follows to reduce 
handshake latency (rfc 4253 7.1). Some less common implementations don't 
handle that, it can be a cause of problems connecting. Note also that
Dropbear may send several ssh packets within a single TCP packet - it's just a 
stream.

