# SPDX-License-Identifier: GPL-2.0
###
# scripts contains sources for various helper programs used throughout
# the kernel for the build process.
# ---------------------------------------------------------------------------

hostprogs-$(CONFIG_BUILD_BIN2C)		+= bin2c

always		:= $(hostprogs-y)

# Let clean descend into subdirs
subdir-	+= basic kconfig
subdir-$(CONFIG_DTC)	+= dtc
