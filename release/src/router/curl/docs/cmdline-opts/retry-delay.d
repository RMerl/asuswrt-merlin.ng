c: Copyright (C) 1998 - 2022, Daniel Stenberg, <daniel@haxx.se>, et al.
SPDX-License-Identifier: curl
Long: retry-delay
Arg: <seconds>
Help: Wait time between retries
Added: 7.12.3
Category: curl
Example: --retry-delay 5 --retry $URL
See-also: retry
---
Make curl sleep this amount of time before each retry when a transfer has
failed with a transient error (it changes the default backoff time algorithm
between retries). This option is only interesting if --retry is also
used. Setting this delay to zero will make curl use the default backoff time.

If this option is used several times, the last one will be used.
