export AtlasSetup=/afs/cern.ch/atlas/software/dist/AtlasSetup
alias asetup='source $AtlasSetup/scripts/asetup.sh'
asetup AtlasOffline,16.6.2,runtime,here
source /afs/cern.ch/sw/ganga/install/etc/setup-atlas.sh
export GANGA_CONFIG_PATH=$GANGA_CONFIG_PATH:GangaJEM/JEM.ini

