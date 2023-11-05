c: Copyright (C) Daniel Stenberg, <daniel@haxx.se>, et al.
SPDX-License-Identifier: curl
Long: speed-time
Short: y
Arg: <seconds>
Help: Trigger 'speed-limit' abort after this time
Category: connection
Example: --speed-limit 300 --speed-time 10 $URL
Added: 4.7
See-also: speed-limit limit-rate
Multi: single
---
If a transfer runs slower than speed-limit bytes per second during a
speed-time period, the transfer is aborted. If speed-time is used, the default
speed-limit is 1 unless set with --speed-limit.

This option controls transfers (in both directions) but does not affect slow
connects etc. If this is a concern for you, try the --connect-timeout option.
