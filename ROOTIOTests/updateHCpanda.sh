#!/bin/zsh
source /afs/cern.ch/user/i/ivukotic/.zshrc

source /afs/cern.ch/project/gd/LCG-share/current/etc/profile.d/grid_env.sh 
voms-proxy-init -voms atlas -pwstdin < /afs/cern.ch/user/i/ivukotic/gridlozinka.txt

export AtlasSetup=/afs/cern.ch/atlas/software/releases/17.0.4/AtlasSetup
alias asetup='source $AtlasSetup/scripts/asetup.sh'

source $AtlasSetup/scripts/asetup.sh rel_0,noTest
source /afs/cern.ch/atlas/offline/external/GRID/DA/panda-client/latest/etc/panda/panda_setup.sh

cd /afs/cern.ch/user/i/ivukotic/ROOTIOTests


python updatePanda.py
#python updatePandaWAN.py
#python updatePandaDPM.py
