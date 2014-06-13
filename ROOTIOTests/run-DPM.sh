#!/bin/bash

# These are the urls towards which we want to run the test
urls=("https://lxfsra04a04.cern.ch/dpm/cern.ch/home/atlas/group.test.hc.NTUP_SMWZ.root" \
"root://lxfsra04a04.cern.ch:1094//atlas/group.test.hc.NTUP_SMWZ.root" \
"https://vm-dcache-deploy6.desy.de:2880/dteam/group.test.hc.NTUP_SMWZ.root" \
"https://gridpp09.ecdf.ed.ac.uk/dpm/ecdf.ed.ac.uk/home/atlas/group.test.hc.NTUP_SMWZ.root" \
"root://gridpp09.ecdf.ed.ac.uk//dpm/ecdf.ed.ac.uk/home/atlas/group.test.hc.NTUP_SMWZ.root" \
"https://atlasdisk1.lnf.infn.it/dpm/lnf.infn.it/home/atlas/group.test.hc.NTUP_SMWZ.root" \
"root://eosatlas.cern.ch//eos/atlas/atlaseosdatadisk/rucio/user/flegger/60/e3/NTUP_SUSYSKIM.01106323._000003.WIGNER.root.1" \
"https://dpmrc-head.civ.zcu.cz/dpm/zcu.cz/home/dteam/group.test.hc.NTUP_SMWZ.root")

# These are the more friendly dest names
destnames=("CERN_DPM_TRUNK" "CERN_DPM_TRUNK" "DESY_DCACHETEST" "GRIDPP_DPM_EPELTEST" "GRIDPP_DPM_EPELTEST" "LNF_DPM_EPELTEST" "CERN_EOS" "ZCU_DPM_RC")

# And this is a human readable word that tells us what we tested
desttag=("DPMhttps" "DPMxrootd" "DCACHEhttps" "DPMhttps" "DPMxrootd" "DPMhttps" "EOSxrootd" "DPMhttps")

for i in 0 1 2 3 4 5 6 7; do
 dest=${urls[$i]}
 dtag=${desttag[$i]}
 dsite=${destnames[$i]}
 echo "------------------ Starting run-DPMsingle.sh $dsite \"$dest\" $dtag -----------------"
 ./run-DPMsingle.sh $dsite "$dest" $dtag
done


rm -f ./group.test.hc.NTUP_SMWZ.root



