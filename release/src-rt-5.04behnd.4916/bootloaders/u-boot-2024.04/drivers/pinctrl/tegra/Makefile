# SPDX-License-Identifier: GPL-2.0
ifndef CONFIG_SPL_BUILD
ifdef CONFIG_TEGRA20
obj-y += pinctrl-tegra20.o
else
obj-y += pinctrl-tegra.o
endif
endif

obj-y += pinmux-common.o

obj-$(CONFIG_TEGRA20) += pinmux-tegra20.o funcmux-tegra20.o
obj-$(CONFIG_TEGRA30) += pinmux-tegra30.o funcmux-tegra30.o
obj-$(CONFIG_TEGRA114) += pinmux-tegra114.o funcmux-tegra114.o
obj-$(CONFIG_TEGRA124) += pinmux-tegra124.o funcmux-tegra124.o
obj-$(CONFIG_TEGRA210) += pinmux-tegra210.o funcmux-tegra210.o
