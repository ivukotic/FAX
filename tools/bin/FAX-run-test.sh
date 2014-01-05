#!/bin/sh

get_response()
{
echo "$1 (y|n|a): " 
if [ $dontask = "y" ]
then
        do=y
else
        read inputline
		if [ $inputline = "y" ]; then do=y; else do=n; fi
        if [ $inputline = "a" ]; then do=y; dontask=y; fi
fi
}

dontask=n
res=""

if [ -z "$X509_USER_PROXY" ]
then
    proxyfn=/tmp/x509up_u$(id -u)
else
    proxyfn=$X509_USER_PROXY
fi

if [ ! -f $proxyfn ] 
then
    echo "To run the tests you will need a valide voms proxy. Please set it up (voms-proxy-init -voms atlas)."
    exit 0
fi

get_response "This test will try to get gLFNs for all the files of one dataset, would you like to proceed?";

if [ $do = "y" ]
then
    kom="FAX-get-gLFNs.sh user.ilijav.HCtest.1"
    printf "Executing command:\n%s\n---------------------------------------------------------------------------------------------------------\n" "$kom"
    eval "$kom"
    if [ "$?" -eq 0 ]
    then
        echo "TEST RESULT - OK"
        res=0$res
    else
        echo "TEST RESULT - FAIL"
        res=1$res
    fi
    echo "========================================================================================================"
else
    echo "Skipping this test."
    res=x$res
fi


get_response "This test will try to copy (using xrdcp) a small file to /dev/null of your computer, would you like to proceed?";

if [ $do = "y" ]
then
    kom="xrdcp -d 1 $STORAGEPREFIX/atlas/rucio/user/ivukotic:group.test.hc.NTUP_SUSY.root -> /dev/null "
    printf "Executing command:\n%s\n---------------------------------------------------------------------------------------------------------\n" "$kom"
    eval $kom
    if [ "$?" -eq 0 ]
    then
        echo "TEST RESULT - OK"
        res=0$res
    else
        echo "TEST RESULT - FAIL"
        res=1$res
    fi
    echo "========================================================================================================"
else
    echo "Skipping this test."
    res=x$res
fi


get_response  "This test will set-up ROOT and list content of a file, would you like to proceed?";

if [ $do = "y" ]
then
	source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh --quiet
	source ${ATLAS_LOCAL_ROOT_BASE}/packageSetups/atlasLocalROOTSetup.sh --quiet
    SCRIPT=$(readlink -f "$0")
    SCRIPTPATH=$(dirname "$SCRIPT")
    kom=" root -l -b -q \"$SCRIPTPATH/FAX-listFileContent.C(\\\"$STORAGEPREFIX/atlas/rucio/user/ivukotic:group.test.hc.NTUP_SUSY.root\\\")\" "
    printf "Executing command:\n%s\n---------------------------------------------------------------------------------------------------------\n" "$kom"
    eval $kom
    if [ "$?" -eq 0 ]
    then
        echo "TEST RESULT - OK"
        res=0$res
    else
        echo "TEST RESULT - FAIL"
        res=1$res
    fi
    echo "========================================================================================================"
else
    echo "Skipping this test."
    res=x$res
fi

echo "Uploading test results..."
java -jar $SCRIPTPATH/FAX2Google-0.0.1-jar-with-dependencies.jar $res > /dev/null
echo "Done."