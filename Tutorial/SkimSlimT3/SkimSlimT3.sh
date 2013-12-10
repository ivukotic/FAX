#!/bin/zsh
export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase
source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh
source $AtlasSetup/scripts/asetup.sh 18.0.0,noTest
export X509_USER_PROXY=x509up_u20074
cat inputFileList
python filter-and-merge-d3pd.py  --in=inputFileList --out=SkimmedSlimmed.root --tree=physics --var=branchesList --selection=file:cutCode
