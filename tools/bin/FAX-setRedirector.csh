#!/bin/tcsh

get_response()
{
echo $1
if ( $dontask = "y" ) then
        set do=y
else
        read inputline
        if ( $inputline = "a" ) then set do=y; set dontask=y; endif
        if ( $inputline = "y" ) then set do=y; endif
endif
}

set dontask=n

set deb=1
if ( $#argv == 1) then
    if ($argv[1] =~ "--quiet" || $argv[1] =~ "-q") then
        set deb=0
    endif
endif

if ( $FAXtoolsDir =~ "" ) then
    set FAXtoolsDir = "./"
endif

set red=`$FAXtoolsDir/bin/getBestRedirector.py`

set sc=$status
if ( $sc == 0 ) then
    if ($red !~ "*/" ) then 
        # echo "slash not found "
        set red="$red/" 
    endif
    
    if ( $deb > 0 ) then
       echo "Setting STORAGEPREFIX to be $red"
    endif
    
    setenv STORAGEPREFIX "$red"
else
    if ( $deb > 0 ) then
        echo "problem in getting best redirector. Setting it to glrd.usatlas.org."
    endif
    setenv STORAGEPREFIX "root://glrd.usatlas.org/"
endif    
