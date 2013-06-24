#!/bin/bash

# g++ releaseFileCache.cxx -o releaseFileCache
make --debug=b
ls
#echo "TFile.AsyncPrefetching:   1" >> .rootrc

echo "version 2.30 -> full cost_matrix version"
#cp $X509_USER_PROXY mproxy.txt
echo -n "time> begin "; date 
#python WANtests.py
echo "starting info\n" > info.txt
python costMatrix.py >> info.txt 
echo -n "time> end "; date

#cat *.sh 
#cat *.sh >> info.txt
#cat AN*.log >> info.txt


echo "------------info.txt begin-------------"
cat info.txt
echo "------------info.txt end---------------"
