#!/bin/bash

outfile=$RSYSLOG_OUT_LOG

status="OK"
echo "<= $status" >> $outfile
echo "$status"

retry_count=0

read line
while [[ -n "$line" ]]; do
    message=${line//$'\n'}
    echo "=> $message" >> $outfile

    if [[ $message == *04* || $message == *07* ]]; then
        if [[ $retry_count < 2 ]]; then
            status="Error: could not process log message"
            let "retry_count++"
        else
            status="OK"
            retry_count=0
        fi
    else
        status="OK"
    fi

    echo "<= $status" >> $outfile
    echo "$status"
    read line
done

exit 0
