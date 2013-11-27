#!/bin/bash

deb=1
if [ "$#" -eq 1 ]; then
    if [ $1 = "--quiet" -o $argv[1] = "-q" ]; then
        deb=0
    fi
fi

if [ "$FAXtoolsDir" = "" ]; then
    FAXtoolsDir="./"
fi

red=$($FAXtoolsDir/bin/getBestRedirector.py)
sc=$?

if [ $sc -eq 0 ]; then
    [[ $red != */ ]] && red="$red"/ 
	if [ $deb -eq 1 ]; then
        echo "Setting STORAGEPREFIX to be $red"
    fi
	export STORAGEPREFIX="$red"
else
    if [ $deb -eq 1 ]; then
    	echo "problem in getting best redirector. Setting it to glrd.usatlas.org."
    fi
    export STORAGEPREFIX="root://glrd.usatlas.org/"
fi
