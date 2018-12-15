#!/bin/bash

outfile=$RSYSLOG_OUT_LOG
terminate=false

function handle_sigusr1 {
    echo "Received SIGUSR1, will terminate after the next message" >> $outfile
    >&2 echo "[stderr] Received SIGUSR1, will terminate after the next message"
    terminate=true
}
trap "handle_sigusr1" SIGUSR1

function handle_sigterm {
    echo "Received SIGTERM, terminating" >> $outfile
    >&2 echo "[stderr] Received SIGTERM, terminating"
    exit 1
}
trap "handle_sigterm" SIGTERM

echo "Starting" >> $outfile

# Write also to stderr (useful for testing the 'output' setting)
>&2 echo "[stderr] Starting"

# Tell rsyslog we are ready to start processing messages
echo "OK"

read log_line
while [[ -n "$log_line" ]]; do
    echo "Received $log_line" >> $outfile
    >&2 echo "[stderr] Received $log_line"

    if [[ $terminate == true ]]; then
        # Terminate prematurely by closing pipe, without confirming the message
        echo "Terminating without confirming the last message" >> $outfile
        >&2 echo "[stderr] Terminating without confirming the last message"
        exit 1
    fi

    # Tell rsyslog we are ready to process the next message
    echo "OK"

    read log_line
done

echo "Terminating normally" >> $outfile
>&2 echo "[stderr] Terminating normally"

exit 0
