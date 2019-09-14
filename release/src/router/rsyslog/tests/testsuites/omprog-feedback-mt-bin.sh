#!/bin/bash

outfile=$RSYSLOG_OUT_LOG

status="OK"
echo $status

retried=false

read line
while [[ -n "$line" ]]; do
    message=${line//$'\n'}

    if [[ $((RANDOM % 100)) < $1 ]]; then
        status="Error: could not process log message"
        retried=true
    else
        if [[ $retried == true ]]; then
            echo "=> $message (retried)" >> $outfile
            retried=false
        else
            echo "=> $message" >> $outfile
        fi
        status="OK"
    fi

    echo $status
    read line
done

exit 0
