c: Copyright (C) Daniel Stenberg, <daniel@haxx.se>, et al.
SPDX-License-Identifier: curl
Long: remote-time
Short: R
Help: Set the remote file's time on the local output
Category: output
Example: --remote-time -o foo $URL
Added: 7.9
See-also: remote-name time-cond
Multi: boolean
---
Makes curl attempt to figure out the timestamp of the remote file that is
getting downloaded, and if that is available make the local file get that same
timestamp.
