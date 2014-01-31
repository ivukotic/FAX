#!/bin/bash

python sleeper.py

echo "version 2013/01/28 11:35"
# this file should be used just to call real test script.
echo -n "time> begin "; date

# export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase
# source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh
# localSetupROOT --skipConfirm

# compiling releaseFileCache as staticly compiled one does not work on all sites.
g++ releaseFileCache.cxx -o releaseFileCache
g++ checkCache.cxx -o checkCache

echo -n "time> compiled cache cleaning tools "; date

echo "=o= compiling read.C"
make --debug=b
echo "=============== done"

ls

# asetup 17.7.3,noTest,x86_64-slc5-gcc43-opt
echo "now should have oracle_cx in it."

#python FDR.py > info.txt;
python FDR_spec.py > info.txt;
cat info.txt
echo "finished FDR part"
exit 0
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


if [ "${PANDA_SITE_NAME}" == "ANALY_ECDF_SL6" -o "${PANDA_SITE_NAME}" == "ANALY_GLASGOW" -o "${PANDA_SITE_NAME}" == "ANALY_LAPP" -o "${PANDA_SITE_NAME}" == "ANALY_CERN_XROOTD" -o "${PANDA_SITE_NAME}" == "ANALY_INFN-FRASCATI" ]; then
    echo -n "Starting DPM tests";
    testName='run-DPM.sh'
    ./$testName $v
fi

#if [ -d "/cvmfs" ]; then
      #     echo "resetting ROOT to 5.32 from cvmfs"
      #     export LD_LIBRARY_PATH=/cvmfs/atlas.cern.ch/repo/sw/software/i686-slc5-gcc43-opt/17.2.0/LCGCMT/LCGCMT_61c/InstallArea/i686-slc5-gcc43-opt/lib/
      #     export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase
      #     source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh
      #     source ${ATLAS_LOCAL_ROOT_BASE}/packageSetups/atlasLocalROOTSetup.sh --skipConfirm --rootVersion=5.32.01-i686-slc5-gcc4.3
      #     source ${ATLAS_LOCAL_ROOT_BASE}/packageSetups/atlasLocalGccSetup.sh --gccVersion gcc432_x86_64_slc5
      #     source ${ATLAS_LOCAL_ROOT_BASE}/packageSetups/atlasLocalPythonSetup.sh --pythonVersion=2.6.5p1-i686-slc5-gcc43
      #     testName='run-generalNTUP-TTC-test.sh'
      #     echo $testName
      #     echo "adding Async prefetch"
      #     echo TFile.AsyncPrefetching: 1 >> .rootrc
      #     for v in ${1//,/ };
      #       do
      #       if [ `echo "$v" | grep -c "NTUP"` -eq 1 ] ;
      # then
      # ./$testName $v;
      #       fi
      #     done
#fi

echo -n "time> finished DPM tests "; date
