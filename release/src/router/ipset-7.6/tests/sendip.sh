#!/bin/bash

# Save lineno for checking
wc -l /var/log/kern.log | cut -d ' ' -f 1 > .loglines
sendip "$@"

