.. SPDX-License-Identifier: GPL-2.0+

qconfig - Querying CONFIG options
=================================

It is not possible to see all the CONFIG options used by a board without
building its `.config` file. This tool allows this to be done efficiently for
all boards, or a subset, writing the results to a unified database file.

This database can be queried, to find boards which used a certain combination
of options, to aid in discovering Kconfig options which imply others.

The tool also permits syncing of defconfigs, which corrects the ordering and
drops options which are implied by others.

Finally, it allows scanning the source code to look for inconsistencies in the
use of Kconfig options.

Installation
------------

You may need to install 'python3-asteval' for the 'asteval' module.

How does it work?
-----------------

When building a database (`-b`), this tool runs configuration and builds
include/autoconf.mk for every defconfig.  The config options defined in Kconfig
appear in the .config file (unless they are hidden because of unmet dependency.)
On the other hand, the config options defined by board headers are seen
in include/autoconf.mk.

When resyncing defconfigs (`-s`) the .config is synced by "make savedefconfig"
and the defconfig is updated with it.

For faster processing, this tool is multi-threaded.  It creates
separate build directories where the out-of-tree build is run.  The
temporary build directories are automatically created and deleted as
needed.  The number of threads are chosen based on the number of the CPU
cores of your system although you can change it via -j (--jobs) option.

Note that `*.config` fragments are not supported.

Toolchains
----------

Appropriate toolchains are necessary to generate include/autoconf.mk
for all the architectures supported by U-Boot.  Most of them are available
at the kernel.org site. This tool uses the same tools as
:doc:`../build/buildman`, so you can use `buildman --fetch-arch` to fetch
toolchains.


Examples
--------

To sync only X86 defconfigs::

   ./tools/qconfig.py -s -d <(grep -l X86 configs/*)

or::

   grep -l X86 configs/* | ./tools/qconfig.py -s -d -

To process CONFIG_CMD_FPGAD only for a subset of configs based on path match::

   ls configs/{hrcon*,iocon*,strider*} | \
       ./tools/qconfig.py -C CONFIG_CMD_FPGAD -d -


Finding boards with particular CONFIG combinations
--------------------------------------------------

You can use `qconfig.py` to figure out which boards have a CONFIG enabled, or
which do not. To use it, first build a database::

    ./tools/qconfig.py -b

Then you can run queries using the `-f` flag followed by a list of CONFIG terms.
Each term is CONFIG name, with or without a tilde (~) prefix. The tool searches
for boards which match the CONFIG name, or do not match if tilde is used. For
example, to find boards which enabled CONFIG_SCSI but not CONFIG_BLK::

    tools/qconfig.py -f SCSI ~BLK
    3 matches
    pg_wcom_seli8_defconfig highbank_defconfig pg_wcom_expu1_defconfig


Finding implied CONFIGs
-----------------------

Some CONFIG options can be implied by others and this can help to reduce
the size of the defconfig files. For example, CONFIG_X86 implies
CONFIG_CMD_IRQ, so we can put 'imply CMD_IRQ' under 'config X86' and
all x86 boards will have that option, avoiding adding CONFIG_CMD_IRQ to
each of the x86 defconfig files.

This tool can help find such configs. To use it, first build a database::

    ./tools/qconfig.py -b

Then try to query it::

   ./tools/qconfig.py -i CONFIG_I8042_KEYB
   CONFIG_I8042_KEYB found in 33/5155 defconfigs
   28 : CONFIG_X86
   28 : CONFIG_SA_PCIEX_LENGTH
   28 : CONFIG_HPET_ADDRESS
   28 : CONFIG_MAX_PIRQ_LINKS
   28 : CONFIG_I8254_TIMER
   28 : CONFIG_I8259_PIC
   28 : CONFIG_RAMBASE
   28 : CONFIG_IRQ_SLOT_COUNT
   28 : CONFIG_PCIE_ECAM_SIZE
   28 : CONFIG_APIC
   ...

This shows a list of config options which might imply CONFIG_I8042_KEYB along
with how many defconfigs they cover. From this you can see that CONFIG_X86
generally implies CONFIG_I8042_KEYB but not always (28 out of 35). Therefore,
instead of adding CONFIG_I8042_KEYB to
the defconfig of every x86 board, you could add a single imply line to the
Kconfig file::

    config X86
        bool "x86 architecture"
        ...
        imply CMD_EEPROM

That will cover 28 defconfigs and you can perhaps find another condition that
indicates that CONFIG_I8042_KEYB is not needed for the remaining 5 boards. Many
of the options listed are not suitable as they are not related. E.g. it would be
odd for CONFIG_RAMBASE to imply CONFIG_I8042_KEYB.

Using this search you can reduce the size of qconfig patches.

You can automatically add 'imply' statements in the Kconfig with the -a
option::

    ./tools/qconfig.py -s -i CONFIG_SCSI \
            -a CONFIG_ARCH_LS1021A,CONFIG_ARCH_LS1043A

This will add 'imply SCSI' to the two CONFIG options mentioned, assuming that
the database indicates that they do actually imply CONFIG_SCSI and do not
already have an 'imply SCSI'.

The output shows where the imply is added::

   18 : CONFIG_ARCH_LS1021A       arch/arm/cpu/armv7/ls102xa/Kconfig:1
   13 : CONFIG_ARCH_LS1043A       arch/arm/cpu/armv8/fsl-layerscape/Kconfig:11
   12 : CONFIG_ARCH_LS1046A       arch/arm/cpu/armv8/fsl-layerscape/Kconfig:31

The first number is the number of boards which can avoid having a special
CONFIG_SCSI option in their defconfig file if this 'imply' is added.
The location at the right is the Kconfig file and line number where the config
appears. For example, adding 'imply CONFIG_SCSI' to the 'config ARCH_LS1021A'
in arch/arm/cpu/armv7/ls102xa/Kconfig at line 1 will help 18 boards to reduce
the size of their defconfig files.

If you want to add an 'imply' to every imply config in the list, you can use::

    ./tools/qconfig.py -s -i CONFIG_SCSI -a all

To control which ones are displayed, use -I <list> where list is a list of
options (use '-I help' to see possible options and their meaning).

To skip showing you options that already have an 'imply' attached, use -A.

When you have finished adding 'imply' options you can regenerate the
defconfig files for affected boards with something like::

    git show --stat | ./tools/qconfig.py -s -d -

This will regenerate only those defconfigs changed in the current commit.
If you start with (say) 100 defconfigs being changed in the commit, and add
a few 'imply' options as above, then regenerate, hopefully you can reduce the
number of defconfigs changed in the commit.


Available options
-----------------

 --nocolour
   Disables colouring of output. This is normally used when writing to a
   terminal.

 -C, --commit
   Create a git commit with the changes when the operation is complete. A
   standard commit message is used which may need to be edited.

 -d, --defconfigs
  Specify a file containing a list of defconfigs to move.  The defconfig
  files can be given with shell-style wildcards. Use '-' to read from stdin.

 -f, --find
   Find boards with a given config combination

 -n, --dry-run
   Perform a trial run that does not make any changes.  It is useful to
   see what is going to happen before one actually runs it.

 -e, --exit-on-error
   Exit immediately if Make exits with a non-zero status while processing
   a defconfig file.

 -s, --force-sync
   Do "make savedefconfig" forcibly for all the defconfig files.
   If not specified, "make savedefconfig" only occurs for cases
   where at least one CONFIG was moved.

 -S, --spl
   Look for moved config options in spl/include/autoconf.mk instead of
   include/autoconf.mk.  This is useful for moving options for SPL build
   because SPL related options (mostly prefixed with CONFIG_SPL\_) are
   sometimes blocked by CONFIG_SPL_BUILD ifdef conditionals.

 -j, --jobs
   Specify the number of threads to run simultaneously.  If not specified,
   the number of threads is the same as the number of CPU cores.

 -r, --git-ref
   Specify the git ref to clone for building the autoconf.mk. If unspecified
   use the CWD. This is useful for when changes to the Kconfig affect the
   default values and you want to capture the state of the defconfig from
   before that change was in effect. If in doubt, specify a ref pre-Kconfig
   changes (use HEAD if Kconfig changes are not committed). Worst case it will
   take a bit longer to run, but will always do the right thing.

 -v, --verbose
   Show any build errors as boards are built

To see the complete list of supported options, run::

  tools/qconfig.py -h
