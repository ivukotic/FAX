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

$FAXtoolsDir/bin/FAX-get-best-redirector > FAXgetBestRedirector

cat FAXgetBestRedirector

set r=`grep export FAXgetBestRedirector | awk -F "STORAGEPREFIX=" '{print $2}'`
echo "$r"
if ( $r =~ "" ) then
    if ( $deb > 0 ) then
        echo "problem in getting best redirector. Setting it to glrd.usatlas.org."
    endif
    setenv STORAGEPREFIX "root://glrd.usatlas.org/"
else
    setenv STORAGEPREFIX $r/
endif

eval "rm -f FAXgetBestRedirector"
