#!/bin/bash

function execute_and_show()
{
	cmd="$*"
	echo -n "$cmd"
	$cmd
	if [ $? -eq 0 ]; then
		echo " [ret: success]"
	else
		echo " [ret: failed]"
	fi
}

opt=$1
if [ "$opt" == "clean" ]; then
	rm client server
	exit 0
fi

execute_and_show g++ server.cpp -g -o xdserver
execute_and_show g++ client.cpp -g -o client -std=c++11 -lpthread

