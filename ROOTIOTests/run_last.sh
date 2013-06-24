#!/bin/bash

#python sleeper.py

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
  echo $v 
  if [ `echo "$v" | grep -c "NTUP"` -eq 1 ] ;
      then
      ./$testName $v;
  fi
done

echo -n "time> finished general NTUP TTC tests "; date

#echo $testName
#for v in ${1//,/ };
#  do
#  if [ `echo "$v" | grep -c "NTUP_TOPJET"` -eq 1 ] ;
#      then
#      ./$testName $v
#  fi
#done

#echo -n "time> finished TOPJET test "; date


#testName='run-generalAOD-athenad3pd-test.sh'
#echo $testName
#for v in ${1//,/ };
#  do 
#  if [ `echo "$v" | grep -c "AOD"` -eq 1 ] ;
#      then
#      echo ./$testName $v;
#  fi
#done

#if [ -d "/cvmfs" ]; then 
#    if [ "${PANDA_SITE_NAME}" == "ANALY_ECDF" -o "${PANDA_SITE_NAME}" == "ANALY_GLASGOW" ]; then 
#	echo -n "Starting DPM tests";
#	testName='run-DPM.sh'
#	./$testName $v
#    fi
#fi

#     testName='run-generalNTUP-TTC-test.sh'
#     echo $testName
#     echo "adding Async prefetch"    
#     echo TFile.AsyncPrefetching: 1 >> .rootrc 
#     for v in ${1//,/ };
#       do 
#       if [ `echo "$v" | grep -c "NTUP"` -eq 1 ] ;
#       then
#            ./$testName $v;
#       fi
#     done


echo -n "time> finished all the tests "; date

