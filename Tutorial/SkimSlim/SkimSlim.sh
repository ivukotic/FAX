#!/bin/zsh
export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase
source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh
source $AtlasSetup/scripts/asetup.sh 17.6.0,noTest
python filter-and-merge-d3pd.py  --in=inputFileList --out=SkimmedSlimmed.root --tree=physics --var=branchesList --selection=file:cutCode
