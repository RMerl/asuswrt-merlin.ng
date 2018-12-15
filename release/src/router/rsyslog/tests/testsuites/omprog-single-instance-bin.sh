#!/bin/bash

outfile=$RSYSLOG_OUT_LOG

# This line should appear only once in the output file for the test to pass:
echo "Starting" >> $outfile

# Write also to stderr (useful for testing the 'output' setting)
>&2 echo "[stderr] Starting"

echo "OK"

read line
while [[ -n "$line" ]]; do
    echo "Received $line" >> $outfile
    >&2 echo "[stderr] Received $line"
    echo "OK"
    read line
done

# This line should appear only once in the output file for the test to pass:
echo "Terminating" >> $outfile
>&2 echo "[stderr] Terminating"

exit 0
