#!/bin/bash

python sleeper.py

echo "version 2013/01/28 11:35"
# this file should be used just to call real test script.
echo -n "time> begin "; date

export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase
source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh
localSetupROOT --skipConfirm

# compiling releaseFileCache as staticly compiled one does not work on all sites.
g++ releaseFileCache.cxx -o releaseFileCache
ls releaseFileCache
g++ checkCache.cxx -o checkCache
ls checkCache
echo -n "time> compiled cache cleaning tools "; date

echo "=o= compiling read.C"
make --debug=b
echo "=============== done"

ls

./doFDR.sh > info.txt

cat info.txt
echo "finished FDR part"
#exit 0

testName='run-generalNTUP-TTC-test.sh'
echo $testName
for v in ${1//,/ };
  do
  if [ `echo "$v" | grep -c "NTUP"` -eq 1 ] ;
      then
      ./$testName $v;
  fi
done

echo -n "time> finished general NTUP TTC tests "; date

