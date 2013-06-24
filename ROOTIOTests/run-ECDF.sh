#!/bin/bash

# cache size is in bytes and is positive
# special values: 0 - turned off
#                 -1 - default value (30MB) - this often does not work -set it manually

if [ "${PANDA_SITE_NAME}" == "ANALY_ECDF" ]; then 
export DPM_HOST=gridpp09.ecdf.ed.ac.uk
export DPNS_HOST=gridpp09.ecdf.ed.ac.uk
filename="https://gridpp09.ecdf.ed.ac.uk/dpm/ecdf.ed.ac.uk/home/atlas/group.test.hc.NTUP_SMWZ.root"
filenamerfio="rfio:////dpm/ecdf.ed.ac.uk/home/atlas/group.test.hc.NTUP_SMWZ.root"
fi
if [ "${PANDA_SITE_NAME}" == "ANALY_GLASGOW" ]; then 
filename="https://svr025.gla.scotgrid.ac.uk/dpm/gla.scotgrid.ac.uk/home/atlas/atlasppsdisk/user.ilijav.HCtest.4/group.test.hc.NTUP_SMWZ.root"
filenamerfio="rfio:////dpm/gla.scotgrid.ac.uk/home/atlas/atlasppsdisk/user.ilijav.HCtest.4/group.test.hc.NTUP_SMWZ.root"
fi

treeToUse="physics"

unset HTTP_PROXY
unset http_proxy

# root -l -q -b "readint.C++(\"$filenamerfio\",\"$treeToUse\", 100, 30)" >& info.txt
./readDirect $filenamerfio $treeToUse 100 30 >& info.txt

echo " --------- info.txt ----------"
cat info.txt
echo " -----------------------------"
cp info.txt /exports/work/physics_ifp_gridpp_pool/DPMTests/info-Rfio-$(date +%Y%m%d%H%M%S).txt
#python uploaderDPM.py "DPM RFIO 100% default cache" "100"
echo -n "time> DPM-test > test 100,30 Rfio finished "; date

echo "resetting ROOT to 5.32 from cvmfs"
export LD_LIBRARY_PATH=/exports/work/atlas2/local/lib:/cvmfs/atlas.cern.ch/repo/sw/software/i686-slc5-gcc43-opt/17.2.0/LCGCMT/LCGCMT_61c/InstallArea/i686-slc5-gcc43-opt/lib/
export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase
source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh
source ${ATLAS_LOCAL_ROOT_BASE}/packageSetups/atlasLocalROOTSetup.sh --skipConfirm --rootVersion=5.32.01-i686-slc5-gcc4.3
source ${ATLAS_LOCAL_ROOT_BASE}/packageSetups/atlasLocalGccSetup.sh --gccVersion gcc432_x86_64_slc5 
source ${ATLAS_LOCAL_ROOT_BASE}/packageSetups/atlasLocalPythonSetup.sh --pythonVersion=2.6.5p1-i686-slc5-gcc43

echo "=o= DPM WebDav 100% cache"
#./releaseFileCache $1
#echo "=o= check that cache is empty "
#./checkCache $1 

root -l -q -b "readDPMWebDav.C++(\"$filename\",\"$treeToUse\", 100, 30,\"\",\"$X509_USER_PROXY\",\"$X509_CERT_DIR\")" >& info.txt
echo " --------- info.txt ----------"
cat info.txt
echo " -----------------------------"
#python uploaderDPM.py "DPM WebDav 100% default cache" "100"
cp info.txt /exports/work/physics_ifp_gridpp_pool/DPMTests/info-WebDav-$(date +%Y%m%d%H%M%S).txt
echo -n "time> DPM-test > test 100,30 WebDav finished "; date


echo "=o= DPM WebDav curl copy"
curl -v -E $X509_USER_PROXY --capath $X509_CERT_DIR -L $filename -o ./group.test.hc.NTUP_SMWZ.root 
#python uploaderDPM.py "DPM WebDav 100% default cache" "100"
echo -n "time> DPM-test > WebDav curl copy finished"; date
rm -f ./group.test.hc.NTUP_SMWZ.root



