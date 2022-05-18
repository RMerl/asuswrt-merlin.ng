![Continuous Integration](https://github.com/jirka-h/haveged/workflows/Continuous%20Integration/badge.svg)

Haveged, an entropy source

IMPORTANT UPDATE

Starting from Linux kernel v5.4, the HAVEGED inspired algorithm has been included in the Linux kernel (see the [LKML article]( https://lore.kernel.org/lkml/alpine.DEB.2.21.1909290010500.2636@nanos.tec.linutronix.de/T/) and the Linux Kernel [commit](https://github.com/torvalds/linux/commit/50ee7529ec4500c88f8664560770a7a1b65db72b)). Additionally, since v5.6, as soon as the CRNG (the Linux cryptographic-strength random number generator) gets ready, `/dev/random` does not block on reads anymore (see [this commit](https://github.com/torvalds/linux/commit/30c08efec8884fb106b8e57094baa51bb4c44e32)).

I'm happy that these changes made it into the mainline kernel. It's pleasing to see that the main idea behind HAVEGED has sustained time test - it was published already in 2003 [here.](https://www.irisa.fr/caps/projects/hipsor/publications/havege-tomacs.pdf) I'm also glad that the HAVEGE algorithm is being further explored and examined - see the [CPU Jitter Random Number Generator.](https://www.chronox.de/jent.html)

Please note that while the mainline Linux Kernel and HAVEGED are using the same concept to generate the entropy (utilizing the CPU jitter) the implementation is completely different. In this sense, HAVEGED can be viewed as another entropy source. 

It means that HAVEGED **service** is now less relevant. However, it's still useful in the following situations, when you
* need randomness early in the boot process, before the CRNG in the Linux kernel gets fully initialized.
* want to deploy an additional entropy source. HAVEGED now inserts entropy into the kernel every 60 seconds, regardless of the entropy level reported by Linux Kernel. It does not affect the `/dev/random` read speed but it diversifies the entropy sources, making the Linux Kernel CRNG more robust. 
* you are looking for userspace RNG to generate random numbers. See `man -S8 haveged` for examples or try running `haveged -n 0 | pv > /dev/null`
* and last but not least, most Linux installations are still running on the older kernel versions.

In any case, I will keep maintaining the HAVEGED project. The userspace application, as well as the haveged library, are not affected in any way by changes in the Linux kernel.

INTRODUCTION

Complete documentation on haveged can be found at http://www.issihosts.com/haveged/

Linux provides device interfaces (/dev/random and /dev/urandom) to a pool of
random numbers collected from system interrupt service routines. On some
systems, especially on those systems with high needs or limited user
interaction, the standard collection mechanism cannot meet demand. In those
cases, an adequate supply of random numbers can be maintained by feeding
additional entropy into /dev/random pool via a file system interface. The
haveged daemon was created to fulfill this function using random data generated
by the HAVEGE algorithm.

The HAVEGE algorithm is based upon the indirect effects of unrelated hardware
events on the instruction timing of a calculation that is sensitive to processor
features such as branch predictors and instruction/data access mechanisms.
Samples from a high-resolution timer are input into the algorithm to produce a
stream of random data in a collection buffer. The algorithm requires no special
privilege other than access to a high resolution timer, as provided by hardware
instruction or a system call.

The HAVEGE mechanism is implemented in C using in-line assembly only where
direct hardware access is needed. On modern compilers, compiler intrinsics are
used to replace much if not all in-line assembly. The haveged implementation of
HAVEGE adds two crucial features: automated tuning of the algorithm to an
environment and a run time facility to verify the data generated in the
collection buffer.

The haveged collection loop is tuned at run-time to match the size of the hosts
L1 data and instruction caches. The size determination is made on the basis of
the best fit to the following (low to high):

1. as a compiled default
2. as determined by cpuid,  if available
3. as determined by the /sys file system,  if available
4. as specified by initialization parameters.

Run-time verification of collection buffer contents is based upon the
methodology from the proposed AIS-31 standard from BSI, the German Federal
Office for Information Security (Bundesamt für Sicherheit in der
Informationstechnik). In the suggested configuration, haveged passes most of the
requirements for a NTG.1 class device described in version two of the AIS-31
specification.

The haveged implementation also provides an experimental feature to multiplex
havege collections over multiple cpu cores. Interprocess coordination in this
configuration sacrifices a significant fraction of haveged throughput in
exchange for a cpu load spread over a number of processors.

All of the above features are contained within the haveged-devel sub package of
haveged. The haveged package can be built with libtool to expose the devel sub
package or without libtool if the devel library is not needed. In either case,
the executable built contains a file system interface to haveged-devel features
and an optional facility to feed havege output into the random device. The build
system also provides "check" targets to test the output of the executable's
random number generator through the file system interface.

TESTING haveged

Testing via build check targets has been part of the haveged distribution since
the project moved to automake. Original tests were limited to a quick evaluation
implemented as an adaptation of the open source ent tool from Fourmilab and a
more thorough evaluation based upon the NIST SP800 test suite. Run time testing
was added to haveged version 1.5 as part of an effort to loosely follow the RNG
testing AIS-31 framework of the German Common Criteria organization, BSI.

The significant features of the AIS-31 framework are:

Run-time tests are broken into 2 groups, Procedure A containing 5 tests, and
Procedure B containing 3 tests. Procedure A consists of performing a 48-bit
disjointedness test on 64K sequences, followed by 257 repetitions of the four
FIPS-140-1 tests and an auto-correlation test on successive 2000 bit sequences.
Procedure B performs distribution tests for 10,000 occurrences of 1, 2, 3, 4 bit
runs in successive samples, followed by a entropy estimate based upon on another
256000+2560 bit sample. A sample must pass all individual tests to pass the
procedure. An ideal RNG is expected to pass Procedure A with a probability of
0.9987 and pass Procedure B with with a probability of 0.9998. One retry will be
attempted to recover from the the failure of a single test in either procedure.
The probability an ideal generator would fail the retry is nill.

Special features of the haveged implementation:

1) A failure of either procedure is a fatal error. In the case of haveged, the
   instance will terminate.
2) The tests operate directly on the collection buffer after a fill. Performance
   costs are acceptable except for the auto-correlation test, test5. An option
   to skip test5 on some repetitions mitigates this problem.
3) Tests can take place at start up (a "tot" test) or continuously as specified
   by a haveged parameter. In the continuous case, there is no implied alignment
   between the collection buffer and the testing context. Collection buffer
   contents will be released for consumption provided the buffer does not contain
   a failed individual test.
4) The size of the input required to complete procedure B depends on the content.
   This means there is no fixed alignment of the test input in the collection
   buffer.
5) Procedure retries are logged. Extended information is available with -v3. Retries
   are expected (see failure rates above) but normally only seen with output
   ranges north of a few GB.

More detailed information on the adaptation of the BIS framework can be found
at http://www.issihosts.com/haveged/ais31.html

BUILDING haveged

This package originated on "Enterprise Linux 5" systems (RHEL 5 / CentOS 5 / SL
5), but every effort has been made to retain and broaden the hardware support of
the original HAVEGE implementation. The package uses the automake build system.
By default, the build uses a libtool build to expose haveged-devel. The
contrib/build directory contains a build.sh script to toggle the libtool
requirement and recover from some autotools failures. Change directory to the
build directory and type ./build.sh for options.

The configure process uses hardware detection via config.sub or the configure
"-host" command line argument. The configure "host" variable is used to select
in-line assembly or compiler intrinsics appropriate to the build target.

Currently supported hosts are:

1. x86
2. ia64
3. powerpc
4. s390
5. sparc
6. sparclite
7. generic

The generic host type is provided for those systems without user level access to
a high-resolution system timer. In this case, the --enable-clock_gettime option
is set to 'yes'.

The following build options are available to "./configure":

1.  --enable-clock_gettime (default 'no' for recognized hosts)
2.  --enable-daemon (default 'yes' if Linux)
3.  --enable-diagnostic (default 'no')
4.  --enable-nistest (default 'no' but recommended)
5.  --enable-olt (default 'yes')
6.  --enable-threads (experimental)
7.  --enable-tune (default 'yes')

Detailed option information is available by typing "./configure --help". For
options xxx that take "yes/no" arguments, --disable-xxx may be used as the
inverse of --enable-xxx.

If --enable-clock_gettime() is 'yes', the clock_gettime(CLOCK_MONOTONIC) system
call will be used as a timer source. This option defaults to 'yes' for generic
host builds and 'no' otherwise. This option may proved useful if access to time
hardware is privileged. Due to variability of clock_gettime() implementations,
the adequacy of the clock_gettime() resolution cannot be known until run time.
It is strongly advised to use run-time testing if this option is used.

If --enable-daemon is 'yes', ioctl access required to the random device and
read-write access to the /proc virtual file system is required. The daemon may
be run in the foreground or fork into the background based upon a command line
argument. The daemon interface targets the 2.6 and later kernel and may not work
on 2.4 kernels due to difference in the random interface between those two
kernel versions. The change in the proc file system from pool size expressed in
bytes to pool size expressed in bits has been taken into account, other changes
may be required. This option is 'no' when diagnostic modes are enabled. If
the option is no the executable is installed in the user bin directory instead
of the sbin directory.

If --enable-diagnostic is 'yes', the capture and inject diagnostic interfaces
are enabled. The capture or inject diagnostic may be enabled singly by setting
the option to 'capture' or 'inject'. A setting for any value other than 'no'
for this option forces --enable-daemon=no. See DIAGNOSTICS below for details.

The --enable-nistest option enables more thorough testing for the check target.
See CHECKING for details.

The --enable-olt option is provided to suppress the entire online test facility.
This option is provided for systems with a very limited resource budget and the
ability to thoroughly test the RNG output by other means. When enabled, the online
test system tests the output of the haveged random number generator using AIS-31
test procedures A and B. Either or both tests may be run as a total failure check
(a "tot" test) at initialization and/or continuously during all subsequent haveged
operation - See the man page and the description at
http://www.issihosts.com/haveged/ais31.html for further information.

The --enable-threads option is an experimental prototype for running multiple
collection threads in a single haveged instance. The goal is to create a
multi-core haveged that would spread collection overhead more evenly over the
available cpu resources.

The --enable-tune option allows the on-line tuning facility to be suppressed.
This is intended for systems with special needs and or a limited resource budget.
Setting the option to 'yes' enables both the cpuid and virtual file system methods,
a value of 'no' suppresses both methods. Individual tuning methods can be selected
by setting the option to either 'cpuid' or 'vfs'. Note that the 'cpuid' method
is always conditional on host type and will not be present if the hardware
architecture does not support the instruction.

The haveged build does not modify CFLAGS and DFLAGS, so for example:

CFLAGS="-fpic -DGENERIC_DCACHE=32" LDFLAGS="-z now" ./configure --disable-tune

would configure the build for a manually-tuned, hardened daemon with a default
data cache size of 32kb.


CHECKING haveged

The build check target provides two test procedures for the build.

1. A "quick" check based upon and adaptation of the public domain ENT program.
   The "entest" program uses the ENT sources to subject a sample to the following:

    a) The Chi-Square result must fall within acceptable bounds (>1% and <99 %)
    b) The entropy/character must exceed a minimum (7.5)
    c) The arithmetic mean must exceed a minimum (127.0)
    d) The monte-carlo approximation of PI must lie within error bounds (.5%)
    e) The Sequential Correlation Coefficient must be below a minimum (.8)

   The program provides a pass-fail indication and an optional display of the
   results to stdout. The entest checks are quite weak and intended only as a
   quick sanity check.

2. An adaptation of the NIST Statistical Test Suite as adapted by Oliver
   Rochecouste of irisa.fr as part of the original havege project. More that 400
   tests are performed in a typical run. The program provides as pass-fail
   indication with detailed results reported in the nist.out file in the
   nist directory.  You will need sit down with SP800-*.pdf available from the
   NIST to review the detailed results. AIS31 provides recommendations for the
   NIST test suite as 'additional tests'. See testing documentation at
   http://www.issihosts.com/haveged/ais31.html for further information.

The "quick" test is always part of the check target. The NIST suite is run only
when --enable-nistest is 'yes'.

Both checks function the same way, haveged is run to collect a sample file in
the test directory which is then analyzed by the test program. A pass-fail return
is given in both cases, additional information is written to stdout. The input
samples and the nist.out report are deleted by the clean make target.

Both test mechanisms are statistical and even a fully functional random number
generator will experience occasional failures. It is not uncommon to see one or
two failures in the NIST suite and the entest will occasionally fail with a small
sample size (usually the Chi-Square test barks). Early haveged releases used a
entest sample size of 1MB, this has been increased to 16MB because failures with
that sample size were all too common. A 16MB sample will also deplete and refill
the haveged collection area to exercise all buffer logic.

Users are encouraged to run their own external tests. The --number==0 option is
a convenient means to pipe haveged output into external suites such as Dieharder,
the TESTU01 batteries, or PractRand.


RUNNING haveged

The following invocation arguments are always available:

   --buffer     , -b [] Buffer size [KW] - default : 128
   --data       , -d [] Data cache size [KB], with fallback to 16
   --inst       , -i [] Instruction cache size [KB], with fallback to 16
   --file       , -f [] Sample output file - default: 'sample', '-' for stdout
   --number     , -n [] Output size in [k|m|g|t]. 0 = unlimited to stdout
   --verbose    , -v [] Verbose mask 0=none,1=summary,2=retries,4=timing,8=loop,16=code,32=test
   --help       , -h    This help

The "-b", "-d", "-i" options are needed only in special cases. Generator output
should be validated after changes to these values.

If haveged is built with online testing enabled, the following is present

   --onlinetest , -o [] [t<x>][c<x>] x=[a[n][w]][b[w]] 't'ot, 'c'ontinuous, default: ta8b"

The default configuration executes the "tot" test using AIS procedure B. At the completion
of the tot test, the buffer is reloaded and any continuous tests will be run before
data becomes available.

If haveged is built with threads support, the following is present

   --threads    , -t [] Number of threads

If daemon interface is enabled, the following options are available:

   --Foreground , -F    Run daemon in foreground, do not fork and detach,
   --pid        , -p [] The location of the daemon pid file, default: /var/run/haveged.pid
   --run        , -r [] 0=daemon,1=config info,>1=Write <r>KB sample file
   --write      , -w [] Set write_wakeup_threshold [bits]

If haveged is running detached, diagnostic output is written to syslog, otherwise
output is written to stderr.

If the daemon interface is enabled, non-zero "-r" options are used to test the
haveged random number generator; the random number generator will be configured,
the initial data collection pass will be executed, configuration details will be
written to stdout, and a "-r" KB sample of output will be written to the sample
output file for all "-r" > 1. The "-n" option provides a more friendly version
of r > 1.

If the daemon interface and --run==1 is specified, or if --run is not available
and --number is not specified a summary of build, tuning, and execution
is displayed and haveged exits without generating data:

<prog>: ver: <ver>; arch: <arch>; vend: <vend>, build: (<opts>); collect: <collect>
<prog>: cpu: <cpu> (<tune>);data: <data> (<tune>); inst: <inst> (<tune>); idx: <idx>; sz: <sz>
<prog>: tot tests(<spec>): <score>; continuous tests(<spec>):<score> last entropy estimate <ent>
<prog>: fills: <fills>, generated: <total>

where
    <prog>     build:  program name - normally haveged
    <ver>      build:  package version
    <arch>     build:  architecture: ia64, generic, ppc, s390, sparc, sparclite, x86
    <vend>     build:  vendor string of host from cpuid if available else 'generic'
    <opts>     build:  version of gcc used and build option flags - see below.
    <collect>  tuning: collection buffer size
    <cpu>      tuning: (sources list for cpu info, see below)
    <data>     tuning: size, (sources list for L1 data cache)
    <inst>     tuning: size, (sources list for L1 instruction cache)
    <idx>      tuning: collector loops used/collector loops available
    <sz>       tuning: collector size used/collector size available.
    <tune>     tuning: tuning sources - see below
    <spec>     exec:   tests to be executed in --onlinetest format
    <ent>      exec:   last entropy estimate from procedure B.
    <score>    exec:   pass/fail counts for AIS test procedures
    <fills>    exec:   number of times buffer was filled
    <total>    exec:   number of bytes output

build option flags represent the ./configure options as:
    C=clock_gettime, D=diagnostic I=tune with cpuid, M=multi-core, T=online tests, V=tune with vfs

tuning sources are:
    D=default value, P=instance parameter, C=cpuid present,
    H=hyperthreading, A=AMD cpuid, A5=AMD fn5, A6=AMD fn6, A8=AMD fn8
    L2=Intel has leaf2, L4=Intel has leaf4, B=Intel leaf b,
    4=intel leaf4, V=virtual file system available
    VS=/sys/devices/system/cpu/cpu%d/cache/index<n>/level,
    VO=/sys/devices/system/cpu/online, VI=/proc/cpuinfo
    VC=/sys/devices/system/cpu

Sources displayed in parenthesis are white space separated lists of the above
tokens. The "collector * used/collector *available" values indicate the fit of
the haveged collection loop to the L1 instruction cache.

Items marked 'exec:' above have meaning only when data is generated. The display
of test results is customized to match the test options specified. The last few
lines of the build/tuning summary (those items marked 'exec:' above) also
appear by themselves as the 'exec summary' in other circumstances.

In other circumstances, the completion of initialization is announced by a banner
written to the log:

"<prog>: starting up"
"Writing <n> byte output to <file>"

The first line above is used when running as a daemon, the second otherwise. If
one or more online tests are running, test failures are logged as:

<prog>: AIS-31 <stage> procedure <proc>: <action> <bytes> bytes fill <fill>

where
    <stage>   is either 'tot' or 'continuous'
    <proc>    is either 'A' or 'B'
    <action>  is either 'retry' or 'fail'
    <bytes>   is number of bytes processed in procedure before failure
    <fill>    is the number of times the buffer was filled

The exec summary is logged upon error or signal terminations. Other log output is
controlled by --verbose:

A --verbose bit mask to obtain additional diagnostic information:

0x01      Show exec summary on termination, retry summary
0x02      Show online test retry details
0x04      Show timing for collections
0x08      Show collection loop characteristics
0x10      Show code offsets
0x20      Show all online test completion details

The "--write" option will set proc/sys/kernel/random/write_wakeup_threshold to
the given value. This is useful because this threshold is very small on some
systems. A minimum of 1024 is recommended.

The file system interface supports file creation of up data setups up to 16tb or
can be part of a piped command set. See the man(8) page for examples.

DIAGNOSTIC haveged build

The diagnostic build is a special version of the non-daemon configuration with a
specialized --run option. The --run levels 0 and 1 correspond duplicate the
equivalent application functions. One or more of the addition commands may be
included depending on the --enable-diagnostic value chosen.

capture (--run 2)
   Run the RNG but collect the timer inputs in a separate buffer. The normal
   RNG output is discarded and replaced by the timer inputs. Requires additional
   buffering, performance is poor because 8 times as many fills are required to
   generate same amount of output.

inject tics (--run 4)
   Replace timer source with file input. Requires additional buffering, source
   must account for generator warmup.

inject data (--run 8)
   Bypass the normal RNG completely and read input directly into the output
   buffer. Generator warmup is skipped. This option is useful for validating
   the online test suite.

Knowledge of haveged internals is needed to use the special features effectively
and interpret the results. The diagnostic build identifies itself as
'havege_diagnostic' in -help display and other outputs.


INSTALLATION

If the daemon interface is not enabled, the install places the executable in
automake's bin_PROGRAMS directory and provides a man(8) page. A man(3) page
is provided for the libtool build. If the daemon interface is enabled, the
executable is installed in automake's sbin_PROGRAMS directory.

EXTRAS

The contrib directory contains bits and pieces that are not integrated into the
distribution. Currently this directory contains build related utilities in
the build directory and an unorganized collection of some of the tools used
to analyze haveged in the diags directory.

The script contrib/build/build.sh organizes the build utilities. The script
provides methods to toggle the use of libtool in the build in particularly
problematic environments, a bootstrap to recover from automake error states
and the option to build and run a devel sample program after the devel
sub package has been built.

Several sample package spec files are provided as contrib/build/*.spec. The
havege_sample.c source demonstrates usage of the havege.h API.
