# SPDX-License-Identifier:      GPL-2.0+

"""Constants used for secure boot test."""

# Owner guid
GUID = '11111111-2222-3333-4444-123456789abc'

# v1.5.1 or earlier of efitools has a bug in sha256 calculation, and
# you need build a newer version on your own.
# The path must terminate with '/'.
EFITOOLS_PATH = ''

# "--addcert" option of sbsign must be available, otherwise
# you need build a newer version on your own.
# The path must terminate with '/'.
SBSIGN_PATH = ''
