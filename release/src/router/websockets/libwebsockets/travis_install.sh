#!/bin/bash

if [ "$COVERITY_SCAN_BRANCH" == 1 ]; then exit; fi

if [ "$TRAVIS_OS_NAME" == "linux" ];
then
	sudo apt-get update -qq

	if [ "$LWS_METHOD" == "libev" ];
	then
		sudo apt-get install -y -qq libev-dev;
	fi
fi

if [ "$TRAVIS_OS_NAME" == "osx" ];
then
	if [ "$LWS_METHOD" == "libev" ];
	then
		brew install libev;
	fi
fi

	
