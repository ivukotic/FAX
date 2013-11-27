#!/bin/tcsh
set red=`./getBestRedirector.py`
echo "got $red"
#$(./getBestRedirector.py)
#sc=$status

#if ( $sc -eq 0 ) then
#    [[ $red != */ ]] && red="$red"/ 
#	echo "Setting STORAGEPREFIX to be $red"
#	export STORAGEPREFIX="$red"
#else
#	echo "problem in getting best redirector. Setting it to glrd.usatlas.org."
#    export STORAGEPREFIX="root://glrd.usatlas.org/"
#endif
