Generic TPL framework
=====================

Overview
--------

TPL---Third Program Loader.

Due to the SPL on some boards(powerpc mpc85xx) has a size limit and cannot
be compatible with all the external device(e.g. DDR). So add a tertiary
program loader (TPL) to enable a loader stub loaded by the code from the
SPL. It loads the final uboot image into DDR, then jump to it to begin
execution. Now, only the powerpc mpc85xx has this requirement and will
implemente it.

Keep consistent with SPL, with this framework almost all source files for a
board can be reused. No code duplication or symlinking is necessary anymore.

How it works
------------

There has been a directory $(srctree)/spl which contains only a Makefile. The
Makefile is shared by SPL and TPL.

The object files are built separately for SPL/TPL and placed in the
directory spl/tpl. The final binaries which are generated are
u-boot-{spl|tpl}, u-boot-{spl|tpl}.bin and u-boot-{spl|tpl}.map.

During the TPL build a variable named CONFIG_TPL_BUILD is exported in the
make environment and also appended to CPPFLAGS with -DCONFIG_TPL_BUILD.

The SPL options are shared by SPL and TPL, the board config file should
determine which SPL options to choose based on whether CONFIG_TPL_BUILD
is set. Source files can be compiled for TPL with options choosed in the
board config file.

For example:

spl/Makefile:
LIBS-$(CONFIG_SPL_LIBCOMMON_SUPPORT) += common/libcommon.o

CONFIG_SPL_LIBCOMMON_SUPPORT is defined in board config file:
#ifdef CONFIG_TPL_BUILD
#define CONFIG_SPL_LIBCOMMON_SUPPORT
#endif
