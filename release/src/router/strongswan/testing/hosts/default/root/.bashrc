# don't store duplicate entries in the history
export HISTCONTROL=erasedups
# use a simple prompt of host:pwd# (user is always root)
PS1='\h:\w\$ '
# set the terminal title to host:pwd
case $TERM in
xterm*)
	PROMPT_COMMAND='echo -ne "\033]0;${HOSTNAME}:${PWD}\007"'
	;;
esac

