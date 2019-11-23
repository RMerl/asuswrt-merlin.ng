#! /bin/bash
# source: readline-test.sh
# Copyright Gerhard Rieger and contributors (see file CHANGES)
# Published under the GNU General Public License V.2, see file COPYING

# script that simulates a simple program with authentication.
# is just for testing the readline features
# perform the test with something like:
# ./socat readline,history=$HOME/.history,noecho='^Password: ' system:./readline-test.sh,pty,setsid,ctty,stderr,sigint,sigquit,echo=0,raw


BANNER='readline feature test program'
USERPROMPT='Authentication required\nUsername: '
PWDPROMPT='Password: '
PROMPT='prog> '

# degenerated user database
CREDUSER="user"
CREDPASS="password"

if   [ $(echo "x\c") = "x" ]; then ECHO="echo"
elif [ $(echo -e "x\c") = "x" ]; then ECHO="echo -e"
fi

#trap "$ECHO $0 got SIGINT"  INT
trap "$ECHO $0 got SIGINT"  INT
trap "$ECHO $0 got SIGQUIT" QUIT

# print banner
$ECHO "$BANNER"

# on (some) ksh read -p does not mean prompt
$ECHO "$USERPROMPT\c"; read -r USERNAME
$ECHO "$PWDPROMPT\c"; read -rs PASSWORD
$ECHO

if [ "$USERNAME" != "$CREDUSER" -o "$PASSWORD" != "$CREDPASS" ]; then
    $ECHO "Authentication failed" >&2
    exit -1
fi

while $ECHO "$PROMPT\c"; read -r COMMAND; do
    if [ "$COMMAND" = "exit" ]; then
	break;
    fi
    $ECHO "executing $COMMAND"
done
