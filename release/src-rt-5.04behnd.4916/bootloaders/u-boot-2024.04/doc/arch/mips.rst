.. SPDX-License-Identifier: GPL-2.0+

MIPS
====

Notes for the MIPS architecture port of U-Boot

Toolchains
----------

  * `ELDK < DULG < DENX <http://www.denx.de/wiki/DULG/ELDK>`_
  * `Embedded Debian -- Cross-development toolchains <http://www.emdebian.org/crosstools.html>`_
  * `Buildroot <http://buildroot.uclibc.org/>`_

Known Issues
------------

  * Cache incoherency issue caused by do_bootelf_exec() at cmd_elf.c

    Cache will be disabled before entering the loaded ELF image without
    writing back and invalidating cache lines. This leads to cache
    incoherency in most cases, unless the code gets loaded after U-Boot
    re-initializes the cache. The more common uImage 'bootm' command does
    not suffer this problem.

    [workaround] To avoid this cache incoherency:
       - insert flush_cache(all) before calling dcache_disable(), or
       - fix dcache_disable() to do both flushing and disabling cache.

  * Note that Linux users need to kill dcache_disable() in do_bootelf_exec()
    or override do_bootelf_exec() not to disable I-/D-caches, because most
    Linux/MIPS ports don't re-enable caches after entering kernel_entry.

TODOs
-----

  * Probe CPU types, I-/D-cache and TLB size etc. automatically
  * Secondary cache support missing
  * Initialize TLB entries redardless of their use
  * R2000/R3000 class parts are not supported
  * Limited testing across different MIPS variants
  * Due to cache initialization issues, the DRAM on board must be
    initialized in board specific assembler language before the cache init
    code is run -- that is, initialize the DRAM in lowlevel_init().
  * centralize/share more CPU code of MIPS32, MIPS64 and XBurst
  * support Qemu Malta
