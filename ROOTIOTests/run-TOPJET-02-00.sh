#!/bin/bash

# cache size is in bytes and is positive
# special values: 0 - turned off
#                 -1 - default value (30MB) - this often does not work -set it manually

echo $1

date >& info.txt

treeToUse="physics"

if [ `echo "$1" | grep -c "NTUP_TOPJET"` -eq 1 ] ;
    then
    treeToUse="physics" 
fi
echo "tree to use: $treeToUse"

date >& info.txt

echo "=o= run TOPJET analysis"
./releaseFileCache $1
echo "=o= check that cache is empty "
./checkCache $1 > cacheContent.txt

date >& info.txt

#rm -rf BaseTree_ttbar
#mkdir BaseTree_ttbar
#rm -f input.txt
#echo $1 > input.txt
#root -l -q -b "run_chain_ttbarSel.C++" >& tempinfo.txt
workdir=${PWD}

rm -rf MakeSelectorAdvancedExample
tar -xvf MakeSelectorAdvancedExample-00-02-00.tar
cd MakeSelectorAdvancedExample
echo "LORG Working in " $PWD
#source $AtlasSetup/scripts/asetup.sh 17.2.0.1,here,64
source build-GRL.sh
echo $1 > input.txt
ln -s $workdir/machineInfo.h .
#ln -s $workdir/ttbarHarness.C .
#ln -s $workdir/ttbarHarness.h .
#export PATH=${PATH}:${PWD}/GoodRunsLists
#export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}/GoodRunsLists/Standalone
macro="run_chain_ttbarSel_02_00.C"
echo "Running $workdir/$macro"
root -l -q -b "$workdir/${macro}++" >& tempinfo.txt

sed -e 's/Number of branches in the cache ...: /BRANCHESREAD=/g' \
    -e 's/Reading............................: /ROOTBYTESREAD=/g' \
    -e 's/bytes in /\nROOTREADS=/g' \
    -e 's/ transactions/\n/g' \
    <tempinfo.txt >info.txt

echo "FILENAME='"$1"'" >> info.txt
echo " --------- info.txt ----------"
cat info.txt
echo " -----------------------------"
python $workdir/uploaderNEW.py "TOPJET 00-02-00 v1"

date >& info.txt

echo "Returning to " $workdir
cd $workdir
