# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2017 Google, Inc

obj-$(CONFIG_LOG_TEST) += log_test.o
obj-$(CONFIG_CMD_LOG) += log_filter.o

ifdef CONFIG_UT_LOG

obj-y += log_ut.o

ifdef CONFIG_SANDBOX
obj-$(CONFIG_LOG_SYSLOG) += syslog_test.o
obj-$(CONFIG_LOG_SYSLOG) += syslog_test_ndebug.o
endif

ifdef CONFIG_LOG
obj-y += pr_cont_test.o
obj-$(CONFIG_CONSOLE_RECORD) += cont_test.o
obj-y += pr_cont_test.o
else
obj-$(CONFIG_CONSOLE_RECORD) += nolog_test.o
obj-$(CONFIG_CONSOLE_RECORD) += nolog_ndebug.o
endif

endif # CONFIG_UT_LOG
