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

echo "This test will try to get gLFNs for all the files of one dataset, would you like to proceed? (y|n): ";
set inputline="$<"
if ( $inputline == "y" )  then
    set kom="FAX-get-gLFNs.sh user.ilijav.HCtest.1"
    printf "Executing command:\n%s\n---------------------------------------------------------------------------------------------------------\n" "$kom"
    eval "$kom"
    echo "========================================================================================================"
else
    echo "Skipping this test."
endif


echo "This test will try to copy (using xrdcp) a small file to /dev/null of your computer, would you like to proceed? (y|n): ";
set inputline="$<"
if ( $inputline == "y" ) then
    set kom="xrdcp -d 1 $STORAGEPREFIX/atlas/rucio/user/ivukotic:group.test.hc.NTUP_SUSY.root -> /dev/null "
    printf "Executing command:\n%s\n---------------------------------------------------------------------------------------------------------\n" "$kom"
    eval $kom
    echo "========================================================================================================"
else
    echo "Skipping this test."
endif


echo "This test will set-up ROOT and list contet of a file, would you like to proceed? (y|n): ";
set inputline="$<"
if ( $inputline == "y" ) then
    #export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase
    #source ${ATLAS_LOCAL_ROOT_BASE}/packageSetups/atlasLocalROOTSetup.sh --quiet
	set fn=$STORAGEPREFIX/atlas/rucio/user/ivukotic:group.test.hc.NTUP_SUSY.root
	echo $fn
	set cfn='"'$fn'"'
    echo $cfn
	set kom="root -l -b -q 'FAX-listFileContent.C("$cfn")' "
    echo $kom
	printf "Executing command:\n%s\n---------------------------------------------------------------------------------------------------------\n" "$kom"
    eval $kom
    echo "========================================================================================================"
else
    echo "Skipping this test."
endif