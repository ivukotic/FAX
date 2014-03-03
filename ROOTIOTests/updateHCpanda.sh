#!/bin/zsh

source /afs/cern.ch/user/i/ivukotic/.zshrc

export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase
source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh

localSetupEmi
voms-proxy-init -voms atlas -pwstdin < /afs/cern.ch/user/i/ivukotic/gridlozinka.txt

asetup 17.2.7,single,here,slc5

source /afs/cern.ch/atlas/offline/external/GRID/DA/panda-client/latest/etc/panda/panda_setup.sh

cd /afs/cern.ch/user/i/ivukotic/FAX/ROOTIOTests


python updatePanda.py
python updatePandaWAN.py
python updatePandaDPM.py
