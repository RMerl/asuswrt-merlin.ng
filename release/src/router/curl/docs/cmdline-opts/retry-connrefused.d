c: Copyright (C) Daniel Stenberg, <daniel@haxx.se>, et al.
SPDX-License-Identifier: curl
Long: retry-connrefused
Help: Retry on connection refused (use with --retry)
Added: 7.52.0
Category: curl
Example: --retry-connrefused --retry 7 $URL
See-also: retry retry-all-errors
Multi: boolean
---
In addition to the other conditions, consider ECONNREFUSED as a transient
error too for --retry. This option is used together with --retry.
