# SPDX-License-Identifier: GPL-2.0+
# Copyright (C) 2023 PHYTEC Messtechnik GmbH
# Author: Teresa Remmet <t.remmet@phytec.de>

ifdef CONFIG_SPL_BUILD
# necessary to create built-in.o
obj- := __dummy__.o
endif

obj-y += phytec_som_detection.o
obj-$(CONFIG_ARCH_IMX8M) += imx8m_som_detection.o
