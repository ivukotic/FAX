#!/bin/zsh
export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase
source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh
source $AtlasSetup/scripts/asetup.sh 17.6.0,noTest
export X509_USER_PROXY=x509up_u20074
echo 'job:' $1, 'from:' $2
files=$(wc -l <inputFileListLarge)
echo 'input files:' $files
awk -v jo=$1 -v totjobs=$2 -v len=$files 'BEGIN {slice = len/totjobs; start = jo*slice; end = (jo+1)*slice;} NR > start && NR <= end {print}' inputFileListLarge > inputFileList
cat inputFileList
python filter-and-merge-d3pd.py  --in=inputFileList --out=SkimmedSlimmed_$1.root --tree=physics --var=branchesList --selection=file:cutCode
