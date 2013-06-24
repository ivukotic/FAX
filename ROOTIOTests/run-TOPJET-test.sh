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

rm -rf BaseTree_ttbar
mkdir BaseTree_ttbar
rm -f input.txt
echo $1 > input.txt
#root -l -q -b machineInfo.C+
root -l -q -b "run_chain_ttbarSel.C++" >& tempinfo.txt
sed -e 's/Number of branches in the cache ...: /BRANCHESREAD=/g' <tempinfo.txt >info.txt
echo "FILENAME='"$1"'" >> info.txt
echo " --------- info.txt ----------"
cat info.txt
echo " -----------------------------"
python uploaderNEW.py "TOPJET 00-00-04 v1_1"

date >& info.txt

