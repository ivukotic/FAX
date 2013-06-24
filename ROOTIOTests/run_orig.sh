#!/bin/bash

python sleeper.py

# this file should be used just to call real test script.
echo -n "time> begin "; date 
# compiling releaseFileCache as staticly compiled one does not work on all sites.
g++ releaseFileCache.cxx -o releaseFileCache
g++ checkCache.cxx -o checkCache

echo -n "time> compiled cache cleaning tools "; date

echo "=o= compiling read.C"
make --debug=b
echo "=============== done"

ls

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

#testName='run-TOPJET-test.sh'
#testName='run-TOPJET-02-00.sh'
#echo $testName
#for v in ${1//,/ };
#  do
#  if [ `echo "$v" | grep -c "NTUP_TOPJET"` -eq 1 ] ;
#      then
#      ./$testName $v
#  fi
#done

#echo -n "time> finished TOPJET test "; date


testName='run-generalAOD-athenad3pd-test.sh'
echo $testName
for v in ${1//,/ };
  do 
  if [ `echo "$v" | grep -c "AOD"` -eq 1 ] ;
      then
      echo ./$testName $v;
  fi
done

if [ -d "/cvmfs" ]; then 
    if [ "${PANDA_SITE_NAME}" == "ANALY_ECDF" -o "${PANDA_SITE_NAME}" == "ANALY_GLASGOW" ]; then 
	echo -n "Starting DPM tests";
	testName='run-DPM.sh'
	./$testName $v
    fi

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
fi

echo -n "time> finished all the tests "; date

