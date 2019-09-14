#!/bin/bash

outfile=$RSYSLOG_OUT_LOG

function handle_sigterm {
    echo "Received SIGTERM" >> $outfile
}
trap "handle_sigterm" SIGTERM

echo "Starting" >> $outfile

# Tell rsyslog we are ready to start processing messages
echo "OK"

read log_line
while [[ -n "$log_line" ]]; do
    echo "Received $log_line" >> $outfile

    # Tell rsyslog we are ready to process the next message
    echo "OK"

    read log_line
done

echo "Terminating unresponsively" >> $outfile

# Terminate with a very long sleep, so omprog will kill this process when
# the closeTimeout (which we have configured to a short value) is reached.
# (Note hat the sleep subprocess itself will not be killed.)
sleep 150s

exit 0
