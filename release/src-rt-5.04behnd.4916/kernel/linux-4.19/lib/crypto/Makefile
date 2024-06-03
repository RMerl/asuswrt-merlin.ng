# SPDX-License-Identifier: GPL-2.0

obj-y += libblake2s.o
libblake2s-y += blake2s.o blake2s-generic.o
ifneq ($(CONFIG_CRYPTO_MANAGER_DISABLE_TESTS),y)
libblake2s-y += blake2s-selftest.o
endif
