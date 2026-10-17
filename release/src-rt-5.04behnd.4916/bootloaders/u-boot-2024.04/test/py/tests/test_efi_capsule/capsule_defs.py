# SPDX-License-Identifier:      GPL-2.0+

"""Directoreis used for authentication and capsule tests."""

# Directories
CAPSULE_DATA_DIR = '/EFI/CapsuleTestData'
CAPSULE_INSTALL_DIR = '/EFI/UpdateCapsule'

# v1.5.1 or earlier of efitools has a bug in sha256 calculation, and
# you need build a newer version on your own.
# The path must terminate with '/' if it is not null.
EFITOOLS_PATH = ''
