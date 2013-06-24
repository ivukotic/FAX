#!/bin/bash

# cache size is in bytes and is positive
# special values: 0 - turned off
#                 -1 - default value (30MB) - this often does not work -set it manually

echo $1

treeToUse="CollectionTree"

if [ `echo "$1" | grep -c "NTUP_SMWZ"` -eq 1 ] ;
    then
    treeToUse="physics" 
fi
if [ `echo "$1" | grep -c "NTUP_SUSY"` -eq 1 ] ;
    then
    treeToUse="susy" 
fi
if [ `echo "$1" | grep -c "NTUP_TRIG"` -eq 1 ] ;
    then
    treeToUse="CollectionTree" 
fi
if [ `echo "$1" | grep -c "NTUP_TOPJET"` -eq 1 ] ;
    then
    treeToUse="physics" 
fi
echo "tree to use: $treeToUse"

#export XRD_DEBUGLEVEL=1

#export DCACHE_DEBUG=32
#export DCACHE_DEBUG_FILE=DCAPdbg.txt

echo "=o= 100% cache"
./releaseFileCache $1
echo "=o= check that cache is empty "
./checkCache $1 

echo -n "time> generalNTUP-TTC-test > File $1 released from cache "; date

echo "not running until debuged. " >& info.txt
exit

# root -l -q -b "readint.C++(\"$1\",\"$treeToUse\", 100, 30 )" >& info.txt
./readDirect $1 $treeToUse 100 30 >& info.txt
echo " --------- info.txt ----------"
cat info.txt
#cat DCAPdbg.txt; rm DCAPdbg.txt;
echo " -----------------------------"
python uploaderNEW.py "100% default cache" "100"
echo -n "time> generalNTUP-TTC-test > test 100,30 finished "; date

echo "=o= 10% cache"
./releaseFileCache $1
# root -l -q -b "readint.C++(\"$1\",\"$treeToUse\", 10, 30 )" >& info.txt
./readDirect $1 $treeToUse 10 30 >& info.txt
echo " --------- info.txt ----------"
cat info.txt
#cat DCAPdbg.txt; rm DCAPdbg.txt;
echo " -----------------------------"
python uploaderNEW.py "10% default cache" "10"
echo -n "time> generalNTUP-TTC-test > test 10,30 finished "; date

echo "=o= 10% no cache"
./releaseFileCache $1
# root -l -q -b "readint.C++(\"$1\",\"$treeToUse\", 10, 0 )" >& info.txt
./readDirect $1 $treeToUse 10 0 >& info.txt
echo " --------- info.txt ----------"
cat info.txt
#cat DCAPdbg.txt; rm DCAPdbg.txt;
echo " -----------------------------"
python uploaderNEW.py "10% no cache" "10"
echo -n "time> generalNTUP-TTC-test > test 10,0 finished "; date

echo "=o= 100% no cache"
./releaseFileCache $1
# root -l -q -b "readint.C++(\"$1\",\"$treeToUse\", 100, 0 )" >& info.txt
./readDirect $1 $treeToUse 100 0 >& info.txt
echo " --------- info.txt ----------"
cat info.txt
echo " -----------------------------"
python uploaderNEW.py "100% no cache" "100"
echo -n "time> generalNTUP-TTC-test > test 100,0 finished "; date


if [ `echo "$1" | grep -c "NTUP_SMWZ"` -eq 1 ] ;
   then
   ./releaseFileCache $1
   echo "=o= 100% events selected branches."
   # root -l -q -b "readint.C++(\"$1\",\"$treeToUse\", 100, 30, \"BranchesToBeRead_SMWZ.txt\" )" >& info.txt
   ./readDirect $1 $treeToUse 100 30 BranchesToBeRead_SMWZ.txt >& info.txt
   echo " --------- info.txt ----------"
   cat info.txt
#   cat DCAPdbg.txt; rm DCAPdbg.txt;
   echo " -----------------------------"
   python uploaderNEW.py "100% TTC 138 branches" "100"
   echo -n "time> generalNTUP-TTC-test > test 100,30 selected branches finished "; date

fi


export XRD_DEBUGLEVEL=0
export DCACHE_DEBUG=0
