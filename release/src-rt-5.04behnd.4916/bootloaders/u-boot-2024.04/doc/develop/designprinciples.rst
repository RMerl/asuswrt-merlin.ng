.. SPDX-License-Identifier: GPL-2.0+:

U-Boot Design Principles
========================

The 10 Golden Rules of U-Boot design
------------------------------------

Keep it Small
^^^^^^^^^^^^^

U-Boot is a Boot Loader, i.e. its primary purpose in the shipping
system is to load some operating system.
That means that U-Boot is
necessary to perform a certain task, but it's nothing you want to
throw any significant resources at. Typically U-Boot is stored in
relatively small NOR flash memory, which is expensive
compared to the much larger NAND devices often used to store the
operating system and the application.

At the moment, U-Boot supports boards with just 128 KiB ROM or with
256 KiB NOR flash. We should not easily ignore such configurations -
they may be the exception in among all the other supported boards,
but if a design uses such a resource-constrained hardware setup it is
usually because costs are critical, i. e. because the number of
manufactured boards might be tens or hundreds of thousands or even
millions...

A usable and useful configuration of U-Boot, including a basic
interactive command interpreter, support for download over Ethernet
and the capability to program the flash shall fit in no more than 128 KiB.

Keep it Fast
^^^^^^^^^^^^

The end user is not interested in running U-Boot. In most embedded
systems they are not even aware that U-Boot exists. The user wants to
run some application code, and that as soon as possible after switching
on their device.

It is therefore essential that U-Boot is as fast as possible,
especially that it loads and boots the operating system as fast as possible.

To achieve this, the following design principles shall be followed:

* Enable caches as soon and whenever possible

* Initialize devices only when they are needed within U-Boot, i.e. don't
  initialize the Ethernet interface(s) unless U-Boot performs a download over
  Ethernet; don't  initialize any IDE or USB devices unless U-Boot actually
  tries to load files from these, etc.  (and don't forget to shut down these
  devices after using them  - otherwise nasty things may happen when you try to
  boot your OS).

Also, building of U-Boot shall be as fast as possible.
This makes it easier to run a build for all supported configurations
or at least for all configurations of a specific architecture,
which is essential for quality assurance.
If building is cumbersome and slow, most people will omit
this important step.

Keep it Simple
^^^^^^^^^^^^^^

U-Boot is a boot loader, but it is also a tool used for board
bring-up, for production testing, and for other activities.

Keep it Portable
^^^^^^^^^^^^^^^^

U-Boot is a boot loader, but it is also a tool used for board
bring-up, for production testing, and for other activities that are
very closely related to hardware development. So far, it has been
ported to several hundreds of different boards on about 30 different
processor families - please make sure that any code you add can be
used on as many different platforms as possible.

Avoid assembly language whenever possible - only the reset code with
basic CPU initialization, maybe a static DRAM initialization and the C
stack setup should be in assembly.
All further initializations should be done in C using assembly/C
subroutines or inline macros. These functions represent some
kind of HAL functionality and should be defined consistently on all
architectures, e.g. basic MMU and cache control, stack pointer manipulation.
Non-existing functions should expand into empty macros or error codes.

Don't make assumptions about the environment where U-Boot is running.
It may be communicating with a human operator on directly attached
serial console, but it may be through a GSM modem as well, or driven
by some automatic test or control system. So don't output any fancy
control character sequences or similar.

Keep it Configurable
^^^^^^^^^^^^^^^^^^^^

Section "Keep it Small" already explains about the size restrictions
for U-Boot on one side. On the other side, U-Boot is a powerful tool
with many, many extremely useful features. The maintainer or user of
each board will have to decide which features are important to them and
what shall be included with their specific board configuration to meet
their current requirements and restrictions.

Please make sure that it is easy to add or remove features from a
board configuration, so everybody can make the best use of U-Boot on
their system.

If a feature is not included, it should not have any residual code
bloating the build.

Keep it Debuggable
^^^^^^^^^^^^^^^^^^

Of course debuggable code is a big benefit for all of us contributing
in one way or another to the development of the U-Boot project. But
as already mentioned in section "Keep it Portable" above, U-Boot is
not only a tool in itself, it is often also used for hardware
bring-up, so debugging U-Boot often means that we don't know if we are
tracking down a problem in the U-Boot software or in the hardware we
are running on. Code that is clean and easy to understand and to
debug is all the more important to many of us.

* One important feature of U-Boot is to enable output to the (usually serial)
  console as soon as possible in the boot process, even if this causes
  tradeoffs in other areas like memory footprint.

* All initialization steps shall print some "begin doing this" message before
  they actually start, and some "done" message when they complete. For example,
  RAM initialization and size detection may print a "RAM: " before they start,
  and "256 MB\\n" when done.  The purpose of this is that you can always see
  which initialization step was running if there should be any problem.  This
  is important not only during software development, but also for the service
  people dealing with broken hardware in the field.

* U-Boot should be debuggable with simple JTAG or BDM equipment.  It shall use
  a simple, single-threaded execution model.  Avoid any magic, which could
  prevent easy debugging even when only 1 or 2 hardware breakpoints are
  available.

Keep it Usable
^^^^^^^^^^^^^^

Please always keep in mind that there are at least three different
groups of users for U-Boot, with completely different expectations
and requirements:

* The end user of an embedded device just wants to run some application; they
  do not even want to know that U-Boot exists and only rarely interacts with
  it (for example to perform a reset to factory default settings etc.)

* System designers and engineers working on the development of the application
  and/or the operating system want a powerful tool that can boot from any boot
  device they can imagine, they want it fast and scriptable and whatever - in
  short, they want as many features supported as possible. And some more.

* The engineer who ports U-Boot to a new board and the board maintainer want
  U-Boot to be as simple as possible so porting it to and maintaining it on
  their hardware is easy for them.

* Make it easy to test. Add debug code (but don't re-invent the wheel - use
  existing macros like log_debug() or debug() depending on context).

Please always keep in mind that U-Boot tries to meet all these
different requirements.

Keep it Maintainable
^^^^^^^^^^^^^^^^^^^^

* Avoid ``#ifdefs`` where possible

* Use "weak" functions

* Always follow the :doc:`codingstyle` requirements.

Keep it Beautiful
^^^^^^^^^^^^^^^^^

* Keep the source code clean: strictly follow the :doc:`codingstyle`,
  keep lists (target names in the Makefiles, board names, etc.)
  alphabetically sorted, etc.

* Keep U-Boot console output clean: output only really necessary information,
  be terse but precise, keep output vertically aligned, do not use control
  character sequences (e.g. backspaces or \\r to do "spinning wheel" activity
  indicators), etc.

Keep it Open
^^^^^^^^^^^^

Contribute your work back to the whole community. Submit your changes
and extensions as patches to the U-Boot mailing list.

Lemmas from the golden rules
----------------------------

Generic Code is Good Code
^^^^^^^^^^^^^^^^^^^^^^^^^

New code shall be as generic as possible and added to the U-Boot
abstraction hierarchy as high as possible. As few code as possible shall be
added in board directories as people usually do not expect re-usable code
there.  Thus peripheral drivers should be put below
"drivers" even if they start out supporting only one specific
configuration.  Note that it is not a requirement for such a first
instance to be generic as genericity generally cannot be extrapolated
from a single data point.
