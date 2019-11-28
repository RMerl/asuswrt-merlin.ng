#! /bin/sh
# source: mail.sh
# Copyright Gerhard Rieger and contributors (see file CHANGES)
# Published under the GNU General Public License V.2, see file COPYING

#set -vx

# This is an example for a shell script that can be fed to socat with exec.
# Its clue is that it does not use stdin/stdout for communication with socat,
# so you may feed the mail message via stdin to the script. The message should
# contain appropriate mail headers without continuation lines.
# socat establishes the connection to the SMTP server; the script performs the
# SMTP dialog and afterwards transfers the message body to the server.
# Lines with only a dot are not permitted - use two dots as escape.
# This script supports multiline answers from server, but not much more yet.

# Usage:  cat message.txt |socat exec:"mail.sh target@domain.com",fdin=3,fdout=4 tcp:mail.relay.org:25,crlf

while [ "$1" ]; do
    case "$1" in
    -f) shift; mailfrom="$1"; shift;;
    *) break;;
    esac
done

rcptto="$1"
[ -z "$1" ] && rcptto="root@loopback"
#server=$(expr "$rcptto" : '[^@]*@\(.*\)')
[ -z "$mailfrom" ] && mailfrom="$USER@$(hostname)"

# this function waits for a complete server message, checks if its status
# is in the permitted range (terminates session if not), and returns.
mail_chat () {
    local cmd="$1" 
    local errlevel="$2";  [ -z "$errlevel" ] && errlevel=300

    if [ "$cmd" ]; then  echo "> $cmd" >&2;  fi
    if [ -n "$cmd" ]; then echo "$cmd" >&4; fi
    while read status message <&3;
	(
	    case "$status" in
	    [0-9][0-9][0-9]-*) exit 0;;
	    [0-9][0-9][0-9]*) exit 1;;
	    *) exit 1;;
	    esac
	)
    do :; done
    if [ -z "$status" ]; then  echo smtp connection failed >&2; exit;  fi
    echo "< $status $message" >&2
    if [ "$status" -ge "$errlevel" ]; then
	echo $message >&2
	echo "QUIT" >&4; exit 1
    fi
}


# expect server greeting
mail_chat

mail_chat "HELO $(hostname)"

mail_chat "MAIL FROM: $mailfrom"

mail_chat "RCPT TO: $rcptto"

mail_chat "DATA" 400

while read l; do echo "$l" >&4; done
mail_chat "."

mail_chat "QUIT"

exit 0
