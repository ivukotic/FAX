#!/bin/bash

# Parameters are destpandaname and url
destsite=$1
desturl=$2
desttag=$3

echo "------------------ Running test towards $destsite $desturl -----------------"
export DEST_SITE_NAME=$1

treeToUse="physics"
echo "resetting ROOT to default from cvmfs"
export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase
source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh 
#source ${ATLAS_LOCAL_ROOT_BASE}/packageSetups/atlasLocalROOTSetup.sh --skipConfirm 5.34.18-davix_p7-x86_64-slc6-gcc48-opt
source ${ATLAS_LOCAL_ROOT_BASE}/packageSetups/atlasLocalROOTSetup.sh --skipConfirm 
export LD_LIBRARY_PATH=./:$LD_LIBRARY_PATH

#if [ "${PANDA_SITE_NAME}" == "ANALY_CERN_XROOTD" ]; then
#echo "EOS tests"
#export COPY_TOOL=eos;
#filenameeos="root://eosatlas.cern.ch//eos/atlas/atlasdatadisk/user/ilijav/HCtest/user.ilijav.HCtest.1/group.test.hc.NTUP_SMWZ.root" 
#filenameeos="root://eosatlas.cern.ch//eos/atlas/atlasdatadisk/data12_8TeV/NTUP_SUSYSKIM/r4065_p1278_p1328_p1329/data12_8TeV.00209265.physics_JetTauEtmiss.merge.NTUP_SUSYSKIM.r4065_p1278_p1328_p1329_tid01106323_00/NTUP_SUSYSKIM.01106323._000003.root.1"

#./readDirect $filenameeos susy 100 30 >& info.txt
#echo " --------- info.txt ----------"
#cat info.txt
#echo " -----------------------------"
#python uploaderDPM.py "DPM Root Read 100% TTC" "100"
#echo -n "time> DPM-test > test 100,30 Eos finished "; date

#./readDirect $filenameeos susy 1 30 >& info.txt
#echo " --------- info.txt ----------"
#cat info.txt
#echo " -----------------------------"
#python uploaderDPM.py "DPM Root Read 1% TTC" "1"
#echo -n "time> DPM-test > test 1,30 Eos finished "; date

#./readDirect $filenameeos susy 1 0 >& info.txt
#echo " --------- info.txt ----------"
#cat info.txt
#echo " -----------------------------"
#python uploaderDPM.py "DPM Root Read NO Cache" "1"
#echo -n "time> DPM-test > test 1,0 Eos finished "; date


#filenameeos="root://eosatlas.cern.ch//eos/atlas/atlaseosdatadisk/rucio/user/flegger/60/e3/NTUP_SUSYSKIM.01106323._000003.WIGNER.root.1"
#export COPY_TOOL=eos-wigner;

#./readDirect $filenameeos susy 100 30 >& info.txt
#echo " --------- info.txt ----------"
#cat info.txt
#echo " -----------------------------"
#python uploaderDPM.py "DPM Root Read 100% TTC" "100"
#echo -n "time> DPM-test > test 100,30 Eos 2 finished "; date

#./readDirect $filenameeos susy 1 30 >& info.txt
#echo " --------- info.txt ----------"
#cat info.txt
#echo " -----------------------------"
#python uploaderDPM.py "DPM Root Read 1% TTC" "1"
#echo -n "time> DPM-test > test 1,30 Eos 2 finished "; date

#./readDirect $filenameeos susy 1 0 >& info.txt
#echo " --------- info.txt ----------"
#cat info.txt
#echo " -----------------------------"
#python uploaderDPM.py "DPM Root Read NO Cache" "1"
#echo -n "time> DPM-test > test 1,0 Eos 2 finished "; date
#fi

export COPY_TOOL=$desttag;
#put python to 2.6 in case its set more recent by root (for use with oracle in athena 17.6.0 release
source ${ATLAS_LOCAL_ROOT_BASE}/packageSetups/atlasLocalPythonSetup.sh --pythonVersion=2.6.5p1-x86_64-slc5-gcc43



echo "------------- Original proxy info --"
echo " voms-proxy-info --all"
voms-proxy-info --all
echo "-----------------------------------"
echo "-- Trying to fetch sakamurro ------"
#fn=`davix-ls -k --cert $X509_USER_PROXY --key $X509_USER_PROXY https://lxfsra04a04.cern.ch/dpm/cern.ch/home/dteam/sakamurro | sort -V | tail -n 1`
fn=`xrd lxfsra04a04.cern.ch dirlist dteam/sakamurro | sort -V -k 5 | tail -n 1 | awk '{ print $5 }'`

echo "-- sakamurro now is: $fn"
#davix-get --debug -k --cert $X509_USER_PROXY --key $X509_USER_PROXY https://lxfsra04a04.cern.ch/dpm/cern.ch/home/dteam/sakamurro/$fn $PWD/sakamurro
xrdcp -f -d 1 root://lxfsra04a04.cern.ch/$fn $PWD/sakamurro
echo "-----------------------------------"



export X509_USER_PROXY=$PWD/sakamurro

echo "------------ voms proxy info : -----------"
voms-proxy-info --all

echo "Grid Environment: "
echo "---> PROXY( $X509_USER_PROXY ) CADIR( $X509_CERT_DIR ) "
echo "-------------------------------------"
ulimit -c unlimited

# Testing connection to ElasticSearch cluster for debug
echo "------- Testing connection to ES cluster -------"
if [ -z "$var" ]; 
then export CAPATH=/etc/grid-security/certificates;
else export CAPATH=$X509_CERT_DIR;
fi
curl --cert $X509_USER_PROXY --key $X509_USER_PROXY --cacert $X509_USER_PROXY --capath $CAPATH -v -XGET 'https://dashb-es-dev.cern.ch:9203/hammer_dpm_0/'

#root -l -q -b "readint.C++(\"$filenamerfio\",\"$treeToUse\", 100, 30)" >& info.txt
#./readDirect $filenamexrootd $treeToUse 100 30 >& info.txt
root.exe -l -q -b "readDPMWebDav.C++(\"$desturl\",\"$treeToUse\", 100, 30,\"\",\"$X509_USER_PROXY\",\"$X509_CERT_DIR\")" >& info.txt
echo " --------- info.txt ----------"
cat info.txt
echo " -----------------------------"

echo " --- Uploading results to the DB ---"
python uploaderDPM.py "DPM Root Read 100% TTC" "100"
echo " --- Uploading results to ElasticSearch ---"
python uploaderDPMES.py "DPM Root Read 100% TTC"
echo -n "time> DPM-test > test 100,30 $desttag finished "; date

root.exe -l -q -b "readDPMWebDav.C++(\"$desturl\",\"$treeToUse\", 1, 30,\"\",\"$X509_USER_PROXY\",\"$X509_CERT_DIR\")" >& info.txt
echo " --------- info.txt ----------"
cat info.txt
echo " -----------------------------"
echo " --- Uploading results to the DB ---"
python uploaderDPM.py "DPM Root Read 1% TTC" "1"
echo " --- Uploading results to ElasticSearch ---"
python uploaderDPMES.py "DPM Root Read 1% TTC"
echo -n "time> DPM-test > test 1,30 $desttag finished "; date

rm -f ./group.test.hc.NTUP_SMWZ.root



