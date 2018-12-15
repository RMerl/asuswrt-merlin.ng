#!/bin/bash

outfile=$RSYSLOG_OUT_LOG

echo "Starting with parameters: $@" >> $outfile

read log_line
while [[ -n "$log_line" ]]; do
    echo "Received $log_line" >> $outfile
    read log_line
done

echo "Terminating normally" >> $outfile
exit 0
