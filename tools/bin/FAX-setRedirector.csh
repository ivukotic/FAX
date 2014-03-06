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

if ( $deb > 0 ) then
    echo $red
endif

set r=`echo $red | awk -F "export" '{print "set",$2}'`   
echo "$r" 
if ( $sc == 0 ) then
    eval $r
else
    if ( $deb > 0 ) then
        echo "problem in getting best redirector. Setting it to glrd.usatlas.org."
    endif
    setenv STORAGEPREFIX "root://glrd.usatlas.org/"
endif    
