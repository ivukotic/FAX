#!/bin/bash

# g++ releaseFileCache.cxx -o releaseFileCache
# make --debug=b
#echo "TFile.AsyncPrefetching:   1" >> .rootrc
ls
voms-proxy-info -all

echo "version 2.60 -> full cost_matrix version Sep 19 2014"
#cp $X509_USER_PROXY mproxy.txt
echo -n "time> begin "; date 
#python WANtests.py
echo "starting info" > info.txt
python costMatrix.py >> info.txt 
echo -n "time> end "; date

#cat *.sh 
#cat *.sh >> info.txt
#cat AN*.log >> info.txt


echo "------------info.txt begin-------------"
cat info.txt
echo "------------info.txt end---------------"
