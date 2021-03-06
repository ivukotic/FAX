#!/bin/bash
A=$PWD
echo "Setting up gcc"
source /cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/Gcc/gcc472_x86_64_slc6/setup.sh
echo "Setting up ROOT"
cd /cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/root/5.34.09-x86_64-slc6-gcc4.7/
source bin/thisroot.sh
echo "Setting up xRootD"
source /cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/root/5.34.09-x86_64-slc6-gcc4.7/bin/setxrd.sh /cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/xrootd/3.2.7-x86_64-slc6/v3.2.7/ 2>&1 > /dev/null
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/xrootd/3.2.7-x86_64-slc6/v3.2.7/lib
cd $A

export X509_USER_PROXY=x509up_u20074

make

echo 'job:' $1, 'from:' $2
files=$(wc -l <inputFileListLarge)
echo 'input files:' $files
inputFile=$(awk -v jo=$1 'NR == jo {print}' inputFileListLarge)
cat inputFileList

./readWrite $inputFile \
root://faxbox.usatlas.org:1094//user/ilijav/rW_$1.root \
physics \
5 \
30

