 export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase
 source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh --quiet

 localSetupFAX --rootVersion=current-SL6 --xrootdVersion=current-SL6

 voms-proxy-init -voms atlas
