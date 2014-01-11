#!/bin/tcsh

if ( $?X509_USER_PROXY ) then
    set proxyfn=$X509_USER_PROXY
else
	set proxyfn=/tmp/x509up_u$(id -u)
endif

if ( ! -e $proxyfn ) then
    echo "To run the tests you will need a valide voms proxy. Please set it up (voms-proxy-init -voms atlas)."
    exit 0
endif

set res=""

echo "This test will try to get gLFNs for all the files of one dataset, would you like to proceed? (y|n): ";
set inputline="$<"
if ( $inputline == "y" )  then
    set kom="FAX-get-gLFNs.sh user.ilijav.HCtest.1"
    printf "Executing command:\n%s\n---------------------------------------------------------------------------------------------------------\n" "$kom"
    eval "$kom"
    if ($status) then
        echo "TEST RESULT - FAIL"
        set res="1$res"
    else
        echo "TEST RESULT - OK"
        set res="0$res"
    endif        
    echo "========================================================================================================"
else
    echo "Skipping this test."
    set res="x$res"
endif


echo "This test will try to copy (using xrdcp) a small file to /dev/null of your computer, would you like to proceed? (y|n): ";
set inputline="$<"
if ( $inputline == "y" ) then
    set kom="xrdcp -d 1 $STORAGEPREFIX/atlas/rucio/user/ivukotic:group.test.hc.NTUP_SUSY.root -> /dev/null "
    printf "Executing command:\n%s\n---------------------------------------------------------------------------------------------------------\n" "$kom"
    eval $kom
    if ($status) then
        echo "TEST RESULT - FAIL"
        set res="1$res"
    else
        echo "TEST RESULT - OK"
        set res="0$res"
    endif 
    echo "========================================================================================================"
else
    echo "Skipping this test."
    set res="x$res"
endif


echo "This test will set-up ROOT and list content of a file, would you like to proceed? (y|n): ";
set inputline="$<"
if ( $inputline == "y" ) then
	source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.csh --quiet
	source ${ATLAS_LOCAL_ROOT_BASE}/packageSetups/atlasLocalROOTSetup.csh --quiet	
	set fn=$STORAGEPREFIX/atlas/rucio/user/ivukotic:group.test.hc.NTUP_SUSY.root
	echo $fn
	set cfn='"'$fn'"'
    echo $cfn
    
    set SCRIPT=`readlink -f "$0"`
    set SCRIPTPATH=`dirname "$SCRIPT"`
    
	set kom="root -l -b -q '"$SCRIPTPATH/"FAX-listFileContent.C("$cfn")' "
    echo $kom
	printf "Executing command:\n%s\n---------------------------------------------------------------------------------------------------------\n" "$kom"
    eval $kom
    if ($status) then
        echo "TEST RESULT - FAIL"
        set res="1$res"
    else
        echo "TEST RESULT - OK"
        set res="0$res"
    endif 
    echo "========================================================================================================"
else
    echo "Skipping this test."
    set res="x$res"
endif

echo "Uploading test results..."
java -jar $SCRIPTPATH/FAX2Google-0.0.2-jar-with-dependencies.jar $res > /dev/null
echo "Done."
