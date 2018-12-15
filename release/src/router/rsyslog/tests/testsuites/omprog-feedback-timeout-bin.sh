#!/bin/bash

outfile=$RSYSLOG_OUT_LOG

echo "Starting" >> $outfile
echo "<= OK" >> $outfile
echo "OK"

just_started=true

read line
while [[ -n "$line" ]]; do
    message=${line//$'\n'}
    echo "=> $message" >> $outfile

    if [[ $message == *02* ]]; then
        # Test partial reads from pipe
        echo "<= OK" >> $outfile
        printf 'O'
        printf 'K'
        printf '\n'
    elif [[ $message == *04* ]]; then
        if [[ $just_started == false ]]; then
            # Force a restart due to 'confirmTimeout' (2 seconds) exceeded
            echo "<= (timeout)" >> $outfile
            ./msleep 10000
        else
            # When the message is retried (just after restart), confirm it correctly
            echo "<= OK" >> $outfile
            echo "OK"
        fi
    elif [[ $message == *07* ]]; then
        # Test keep-alive feature for long-running processing (more than 2 seconds)
        echo "<= ........OK" >> $outfile
        for i in {1..8}; do
            ./msleep 500
            printf '.'
        done
        printf 'OK\n'
    else
        echo "<= OK" >> $outfile
        echo "OK"
    fi

    just_started=false
    read line
done

exit 0
