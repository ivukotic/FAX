#!/bin/tcsh

set deb=1
if ( $#argv == 1) then
    if ($argv[1] =~ "--quiet" || $argv[1] =~ "-q") then
        set deb=0
    endif
endif

if ( $FAXtoolsDir =~ "" ) then
    set FAXtoolsDir = "./"
endif

set red=`$FAXtoolsDir/bin/FAX-get-best-redirector`

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
