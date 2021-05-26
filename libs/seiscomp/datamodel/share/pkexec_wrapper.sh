#!/bin/bash

pid=-1

# the cleanup function will be the exit point
cleanup () {
	if [ $pid -ne -1 ]
	then
		kill $pid
	fi

}

if [ $# -eq 0 ]; then
    echo "Usage: pkexec_wrapper.sh <cmd>"
	exit 0
fi

trap cleanup EXIT ERR INT TERM

cmd=$1

pkttyagent -p $(echo $$) --fallback &
pid=`echo $!`

pkexec "$@"
