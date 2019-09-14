#!/bin/bash

outfile=$RSYSLOG_OUT_LOG

status="OK"
echo "<= $status" >> $outfile
echo $status

in_transaction=false
fail_on_commit=false
retry_count=0

read line
while [[ -n "$line" ]]; do
    message=${line//$'\n'}
    echo "=> $message" >> $outfile

    if [[ "$message" == "BEGIN TRANSACTION" ]]; then
        in_transaction=true
        status="OK"
    elif [[ "$message" == "COMMIT TRANSACTION" ]]; then
        in_transaction=false
        if [[ $fail_on_commit == true ]]; then
            status="Error: could not commit transaction"
            fail_on_commit=false
        else
            status="OK"
        fi
    else
        if [[ $in_transaction == true ]]; then
            status="DEFER_COMMIT"
        else
            # Should not occur
            status="Error: received a message out of a transaction"
        fi
    fi

    # First command line argument ($1) indicates whether to test for negative
    # cases. If --failed_messages is specified, an error is returned for certain
    # messages, forcing them to be retried twice. If --failed_commits is
    # specified, the error is returned when committing the transaction.
    if [[ "$1" != "" && ($message == *04* || $message == *07*) ]]; then
        if [[ $retry_count < 2 ]]; then
            if [[ "$1" == "--failed_commits" ]]; then
                fail_on_commit=true
            else
                status="Error: could not process log message"
            fi
            let "retry_count++"
        else
            retry_count=0
        fi
    fi

    echo "<= $status" >> $outfile
    echo $status
    read line
done

exit 0
