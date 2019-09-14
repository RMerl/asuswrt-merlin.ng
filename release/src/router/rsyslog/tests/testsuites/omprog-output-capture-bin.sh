#!/bin/bash

echo "[stdout] Starting"
>&2 echo "[stderr] Starting"

read log_line
while [[ -n "$log_line" ]]; do
    echo "[stdout] Received $log_line"
    >&2 echo "[stderr] Received $log_line"
    read log_line
done

echo "[stdout] Terminating normally"
>&2 echo "[stderr] Terminating normally"

exit 0
