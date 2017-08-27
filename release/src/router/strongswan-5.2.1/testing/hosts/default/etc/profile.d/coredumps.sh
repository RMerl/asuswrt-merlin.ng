#!/bin/sh -e

ulimit -c unlimited >/dev/null 2>&1
install -m 1777 -d /var/local/dumps >/dev/null 2>&1
echo "/var/local/dumps/core.%e.%p" > /proc/sys/kernel/core_pattern
