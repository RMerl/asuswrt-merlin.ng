#!/bin/sh
#
# Ask a question and return the answer

QUESTION=$1
PARM=$2
TEXT_FILE=$3
while :; do
	echo -n "${QUESTION} [${PARM}] " >/dev/tty
	read ANSWER
	if [ "${ANSWER}" = "" ]; then
		break
	elif [ "${ANSWER}" = "?" ]; then
		cat build/${TEXT_FILE} >/dev/tty
	else
		echo ${ANSWER}
		break
	fi
done

