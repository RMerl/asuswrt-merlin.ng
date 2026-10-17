# SPDX-License-Identifier: GPL-2.0+

ifdef CONFIG_EXAMPLES

ifdef FTRACE
subdir-ccflags-y += -finstrument-functions -DFTRACE
endif

subdir-y += standalone
subdir-$(CONFIG_API) += api
endif
