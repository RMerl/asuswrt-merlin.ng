
In this article, I describe why I'm disabling
[seccomp](https://www.kernel.org/doc/Documentation/prctl/seccomp_filter.txt)
on Linux for the default
[acme-client](https://kristaps.bsd.lv/acme-client) build.

**tl;dr** seccomp filters differ significantly between Linux installs;
[acme-client](https://kristaps.bsd.lv/acme-client) terminates on
violations; users think software is crap and go back to other
fly-by-night Ruby or Perlthon client; users are insecure; I am sad.

**Edit**: people seem to be stumbling upon my usage of Alpine or that
this is a "rant".  As for the second, this is a technical article with
technical arguments --- not an op-ed.  As for the first, it's irrelevant
to the technical arguments.  The discussion covers Debian, Ubuntu, Arch,
and others.  It's Linux/seccomp, not Alpine, not Debian.

# Introduction

After implementing a sandbox for
[acme-client](https://kristaps.bsd.lv/acme-client) with
[pledge(2)](http://man.openbsd.org/pledge.2) (easy),
[capsicum(4)](https://www.freebsd.org/cgi/man.cgi?query=capsicum&sektion=4)
(harder, but still easy-ish), and
[sandbox\_init(3)](https://developer.apple.com/legacy/library/documentation/Darwin/Reference/ManPages/man3/sandbox_init.3.html)
(easy but arguably useless), I set out to add a Linux sandbox with
[libseccomp](https://github.com/seccomp/libseccomp).

This isn't the first system I've sandboxed
([kcgi](https://kristaps.bsd.lv/kcgi),
[sblg](https://kristaps.bsd.lv/sblg), etc.); but for seccomp, it will
probably be the last.

*Aside: what's a sandbox?*  A sandbox defines the resources an
application will use.  If it uses anything else, the program
terminates.  There are many sandboxes:
[pledge(2)](http://man.openbsd.org/pledge.2) on
[OpenBSD](http://www.openbsd.org),
[capsicum(4)](https://www.freebsd.org/cgi/man.cgi?query=capsicum&sektion=4)
on [FreeBSD](https://www.freebsd.org), and
[libseccomp](https://github.com/seccomp/libseccomp) implementing
[seccomp-bpf](https://www.kernel.org/doc/Documentation/prctl/seccomp_filter.txt)
on [Linux](https://www.kernel.org).

After implementing a filter that worked on a local x86\_64
[Alpine](https://alpinelinux.org) system (Alpine being the distribution
that went fastest from ISO to installing the compiler and CVS) and
releasing, I immediately began receiving violation reports (e.g.,
[issue/16](https://github.com/kristapsdz/acme-client-portable/issues/16))
for non-Alpine distributions and for non-x86\_64 architectures.

A filter violation is when a system call I didn't white-list is invoked.
Sometimes a system's C library would implement a function call
differently from Alpine (things like
[getaddrinfo(2)](http://man.openbsd.org/getaddrinfo.3) differ widely).
Sometimes an architecture would use different system calls for the same
function.  Reasons differed.

What disturbed me was not only that I couldn't reproduce --- each report
was from a non-[Alpine](https://alpinelinux.org) C library,
architecture, or distribution --- but that I couldn't verify that there
was a problem at all.  A crafty adversary could have requested a given
system call be white-listed while knowing he could exploit it.  The
burden would be on me to exactly reproduce the reporter's system to
verify the fix.  Anything but what I already had required spinning up
VirtualBox...

![This is quite applicable.](http://gifrific.com/wp-content/uploads/2012/08/Aint-Nobody-Got-Time-for-That.gif)

I'm not saying this happened --- I had no reason to distrust any bug
reports, and on the contrary thank everybody who took the time to do so!
Most system calls made sense.  The fact remains, however, that it's possible.

I consider this a serious problem.  You should, too.  But even if not,
there are other problems.

# The seccomp problem

Programming
[seccomp-bpf](https://www.kernel.org/doc/Documentation/prctl/seccomp_filter.txt)
(even helped via [libseccomp](https://github.com/seccomp/libseccomp))
is...

![Looking at sandbox-seccomp-filter.c...](http://i0.kym-cdn.com/entries/icons/original/000/016/986/xl1XYq8.jpg)

I don't have anything against libseccomp or seccomp-bpf's functionality;
however, as [I was once
told](https://mail-index.netbsd.org/tech-kern/2007/12/29/0007.html) many
years ago, "I find the documentation inscrutable".  Go ahead.
[RTFM](http://man7.org/linux/man-pages/man2/seccomp.2.html).
Or read
[sandbox-seccomp-filter.c](https://github.com/openssh/openssh-portable/blob/master/sandbox-seccomp-filter.c).

That, however, isn't the problem.  (It's "a" problem.) The problem is
that it's not portable *within Linux*: a new architecture means new (or
different) system calls; a new C library means new (or different) system
calls; and a new kernel version means new (or different?) system calls.

This is a problem with any capabilities system.  (See [Bugs Ex
Ante](https://www.youtube.com/watch?v=lqIXr_Or2s4) for a historical
perspective.) For some similar systems like
[systrace(4)](http://www.citi.umich.edu/u/provos/systrace/) on OpenBSD,
this wasn't so bad because OpenBSD's system is uniform: one C library,
one system call interface.

What makes seccomp problematic is what makes Linux popular: one can use the
[musl](https://www.musl-libc.org/) C library,
[glibc](https://www.gnu.org/software/libc/), or others.  One can use
x86\_64 or an embedded ARM system.  You can compile the kernel yourself
without swaths of otherwise-supported system calls --- and many
distributions do just that.  All of this is supported and even encouraged.

When I set out to implement a seccomp filter, I naïvely assumed that the
differences wouldn't be so great between Linux systems.  But this was
false.  Why?

First, system calls between hardware.  For example, some have `mmap`.
Some have `mmap2` (from a previous version of OpenSSH's filter, noted as
"EABI ARM only has `mmap2`").  Do I white-list both?  Just what is
`mmap2`?  [mmap2(2)](http://man7.org/linux/man-pages/man2/mmap2.2.html)
helpfully notes:

> On architectures where this system call is present, the glibc mmap()
> wrapper function invokes this system call rather than the mmap(2)
> system call.  

So I might *use* `mmap`, but the system call is `mmap2`?  Great.

This brings us to the second and larger problem.  The C library.  There
are several popular ones on Linux:
[glibc](https://www.gnu.org/software/libc),
[musl](https://www.musl-libc.org), [uClibc](https://uclibc.org/), etc.
Each of these is free to implement any standard function (like `mmap`,
above) in any way.  So while my code might say `read`, the C library
might also invoke `fstat`.  Great.

In general, section 2 calls (system calls) map evenly between system
call name and function name.  (Except as noted above... and maybe
elsewhere...)  However, section 3 is all over the place.  The strongest
differences were between big functions like
[getaddrinfo(2)](http://man.openbsd.org/getaddrinfo.3).

Then there's local modifications.  And not just between special embedded
systems.  But [Debian](https://www.debian.org/) and
[Arch](https://www.archlinux.org/), both using
[glibc](https://www.gnu.org/software/libc) and both on x86\_64, have
different kernels installed with different features.  Great.

Less great for me and seccomp.

# The ~~ego~~ developer problem

When I first released
[acme-client](https://kristaps.bsd.lv/acme-client), I assumed that users
would let me know if the system terminated with a violation.  What I
didn't consider was how much the seccomp filter would be violated even
though I carefully verified my test environment with a popular Linux on
a popular architecture.  Or how long it would take me to verify
violations and fix them (blindly).

In the end, I took a lot of my users' time (and those were the good
users, the ones you want, the ones who notify you of issues), I took a
lot of my own time tracking down system call violations, and I alienated
everybody else who just gave up when it failed.

What's more embarrassing is that, to most users who simply downloaded
the software, the application just *failed*.  Not *seccomp failed*, but
*[acme-client](https://kristaps.bsd.lv/acme-client) failed*.  Ouch.

Fact is, not everybody who really wants to use my software knows how to
program --- in fact, I'd guess that most don't at all, being that
[acme-client](https://kristaps.bsd.lv/acme-client) is for administering
certificates, not writing code.  To them, it was just crap.

So if my carefully-written and hardened program is meant to provide a
valuable security service, but it terminates due to seccomp violations
and users think it's crap and use in-secure applications --- what's the
point of sandboxing it at all?

![What's the point?](http://www.theimaginativeconservative.org/wp-content/uploads/2014/05/peter-sellers-as-dr-strangelove-1.jpg)

What solutions do I have?  If I stay with seccomp, I continue to
alienate general users with my "crappy" software, require skilled users
(where "skilled" is a function of programming) to jump through hoops,
and spend lots of my own time to interpret incoming violation reports
from said mystical "skilled" users.

# The Linux problem

Fact is, using seccomp is difficult unless

1. the system really is really really trivial (pure computation), which
   reduces the probability of users having violations;
2. the users are both skilled in programming and patient with
   violations, which reduces the problem that users will abandon the
   software; or
3. you have the resources to properly test on most systems before
   releasing.

It seems to me that seccomp is designed for big companies who can afford
the resources (like those rhyming with "doodle"), big open source
projects with ~~more intelligent developers and~~ testing
infrastructure like [OpenSSH](https://www.openssh.com/), or niche pure
computation systems.

That leaves [acme-client](https://kristaps.bsd.lv/acme-client) out of
the loop.

Is this a problem?  It depends on the software, I guess.  But it's not
*my* software that should scare you.  It's that mine is only one of many
potentially good pieces of software that will never be secured, and thus
will fall under your radar.  Or worse --- it's that the software you
*do* use, or need to use, isn't secure because the developer is in my
shoes.

Fortunately, there's always [OpenBSD](http://www.openbsd.org) and
[pledge(2)](http://man.openbsd.org/pledge.2)!  And if there are
alternatives, that means that the problems aren't insurmountable.  Which
makes it a Linux problem, because Linux suffers from seccomp's problems.

![You called?](https://www.linux.org.ru/gallery/4888769.jpg)

This article isn't supposed to be about OpenBSD's
[pledge(2)](http://man.openbsd.org/pledge.2).  Or even FreeBSD's
[capsicum(4)](https://www.freebsd.org/cgi/man.cgi?query=capsicum&sektion=4),
which is eminently more usable than either seccomp-bpf or libseccomp.

But really it kinda is, because it's important to show an example of how
*not* to go down the Linux/seccomp road.  (Also, I want to look
positive, when really I'm just sad and a little pissed.)

On both of these systems, I *know* that when I sandbox a given
capability, it will be sandboxed for all of my users.  Whether that
would remain if the C library were switchable is another story.
Moreover, going from zero to hero in implementing a filter for a
reasonably well-written piece of software isn't terribly difficult.
Give it a try!

# Solutions

I've now disabled the seccomp code in the [GNUmakefile](GNUmakefile) for
[acme-client](https://kristaps.bsd.lv/acme-client).

If you want to enable it, you're welcome to --- all of the code is still
there in [sandbox-seccomp.c](sandbox-seccomp.c) and fairly easy to read.  
Note that the debugging is enabled by default, and the functions it
isn't are safe to use in a signal context.  So if you use it a lot, that
might get annoying.

If you add system calls to the filter white-list, please notify me and
I'll make sure it makes its way into
[sandbox-seccomp.c](sandbox-seccomp.c) so you don't need to maintain a
downstream.

However, there's still the issue of trust.  Can I trust that what I put
in that code --- even if it's "only for experts" --- isn't a whole
waiting to be exploited?  Am I putting other skilled people at risk?
Can you trust what you see there, knowing it was provided by others?

There isn't an easy answer to this.

