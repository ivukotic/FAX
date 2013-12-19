#!/bin/sh

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

echo "This test will try to get gLFNs for all the files of one dataset, would you like to proceed? (y|n): ";
read inputline
if [ "$inputline" == "y" ]
then
    kom="FAX-get-gLFNs.sh user.ilijav.HCtest.1"
    printf "Executing command:\n%s\n---------------------------------------------------------------------------------------------------------\n" "$kom"
    eval "$kom"
    echo "========================================================================================================"
else
    echo "Skipping this test."
fi


echo "This test will try to copy (using xrdcp) a small file to /dev/null of your computer, would you like to proceed? (y|n): ";
read inputline
if [ "$inputline" == "y" ]
then
    kom="xrdcp -d 1 $STORAGEPREFIX/atlas/rucio/user/ivukotic:group.test.hc.NTUP_SUSY.root -> /dev/null "
    printf "Executing command:\n%s\n---------------------------------------------------------------------------------------------------------\n" "$kom"
    eval $kom
    echo "========================================================================================================"
else
    echo "Skipping this test."
fi


echo "This test will set-up ROOT and list contet of a file, would you like to proceed? (y|n): ";
read inputline
if [ "$inputline" == "y" ]
then
    kom=" root -l -b -q \"FAX-listFileContent.C(\\\"$STORAGEPREFIX/atlas/rucio/user/ivukotic:group.test.hc.NTUP_SUSY.root\\\")\" "
    printf "Executing command:\n%s\n---------------------------------------------------------------------------------------------------------\n" "$kom"
    eval $kom
    echo "========================================================================================================"
else
    echo "Skipping this test."
fi