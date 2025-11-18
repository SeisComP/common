#!/bin/bash

pid=-1

# the cleanup function will be the exit point
cleanup () {
	if [ $pid -ne -1 ]
	then
		kill $pid &>/dev/null
	fi

}

if [ $# -eq 0 ]; then
    echo "Usage: pkexec_wrapper.sh <cmd>"
	exit 0
fi

trap cleanup EXIT ERR INT TERM

cmd=$1

which pkexec &> /dev/null
if [ $? -ne 0 ]; then
    echo "Error: pkexec command not found"
    exit 1
fi

which pkttyagent &> /dev/null
if [ $? -ne 0 ]; then
    echo "Error: pkttyagent command not found"
    exit 1
fi

# The fallback agent can only be started when the device /dev/tty
# is available and usable. If this script is started from the
# desktop the tty device may not be available.
if  !(test "$(ps -p "$$" -o tty=)" = "?"); then
    pkttyagent -p $(echo $$) --fallback &
    pid=`echo $!`
fi

pkexec "$@"
