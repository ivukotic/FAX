#!/bin/bash

# g++ releaseFileCache.cxx -o releaseFileCache
# make --debug=b
#echo "TFile.AsyncPrefetching:   1" >> .rootrc
ls

export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase
#source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh
#localSetupFAX
#voms-proxy-info -all

echo "version 2.90 -> back to no localSetupFAX. Sep 21 2014"

echo -n "time> begin "; date 
echo "starting info" > info.txt
python costMatrix.py >> info.txt 
echo -n "time> end "; date

#cat *.sh 
#cat *.sh >> info.txt
#cat AN*.log >> info.txt


echo "------------info.txt begin-------------"
cat info.txt
echo "------------info.txt end---------------"
