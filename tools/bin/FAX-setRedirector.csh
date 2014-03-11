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

set r=`grep export FAXgetBestRedirector | awk -F "export" '{print "set",$2}'`
echo "$r"
if ( $sc == 0 ) then
    eval "set $r"
else
    if ( $deb > 0 ) then
        echo "problem in getting best redirector. Setting it to glrd.usatlas.org."
    endif
    setenv STORAGEPREFIX "root://glrd.usatlas.org/"
endif

rm FAXgetBestRedirector