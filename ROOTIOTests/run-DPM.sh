#!/bin/bash

# cache size is in bytes and is positive
# special values: 0 - turned off
#                 -1 - default value (30MB) - this often does not work -set it manually
if [ "${PANDA_SITE_NAME}" == "ANALY_LAPP" ]; then 
export SITEDPMHOST=lapp-se99.in2p3.fr
export SITEDOMAIN=in2p3.fr
fi
if [ "${PANDA_SITE_NAME}" == "ANALY_ECDF" ]; then 
export SITEDPMHOST=gridpp09.ecdf.ed.ac.uk
export SITEDOMAIN=ecdf.ed.ac.uk
fi
if [ "${PANDA_SITE_NAME}" == "ANALY_GLASGOW" ]; then 
export SITEDPMHOST=dev013-v6.gla.scotgrid.ac.uk
export SITEDOMAIN=gla.scotgrid.ac.uk
fi
if [ "${PANDA_SITE_NAME}" == "ANALY_CERN_XROOTD" ]; then 
export SITEDPMHOST=lxfsra04a04.cern.ch
export SITEDOMAIN=cern.ch
fi

if [ "${PANDA_SITE_NAME}" == "ANALY_INFN-FRASCATI" ]; then
export SITEDPMHOST=atlasdisk1.lnf.infn.it
export SITEDOMAIN=lnf.infn.it
fi


filename="http://$SITEDPMHOST/dpm/$SITEDOMAIN/home/atlas/group.test.hc.NTUP_SMWZ.root"
filenamexrootd="root://$SITEDPMHOST//dpm/$SITEDOMAIN/home/atlas/group.test.hc.NTUP_SMWZ.root"

export DPM_HOST=$SITEDPMHOST
export DPNS_HOST=$SITEDPMHOST
#filenamehttp="http://$SITEDPMHOST/dpm/$SITEDOMAIN/home/atlas/group.test.hc.NTUP_SMWZ.root"
#filenamerfio="rfio:////dpm/$SITEDOMAIN/home/atlas/group.test.hc.NTUP_SMWZ.root"

treeToUse="physics"
echo "resetting ROOT to default from cvmfs"
export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase
source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh
source ${ATLAS_LOCAL_ROOT_BASE}/packageSetups/atlasLocalROOTSetup.sh --skipConfirm 

#ln -s $LCG_LOCATION/lib64/libdpm.so libshift.so.2.1
#ln -s $LCG_LOCATION/lib64/liblcgdm.so
export LD_LIBRARY_PATH=./:$LD_LIBRARY_PATH

if [ "${PANDA_SITE_NAME}" == "ANALY_CERN_XROOTD" ]; then
echo "EOS tests"
export COPY_TOOL=eos;
#filenameeos="root://eosatlas.cern.ch//eos/atlas/atlasdatadisk/user/ilijav/HCtest/user.ilijav.HCtest.1/group.test.hc.NTUP_SMWZ.root" 
filenameeos="root://eosatlas.cern.ch//eos/atlas/atlasdatadisk/data12_8TeV/NTUP_SUSYSKIM/r4065_p1278_p1328_p1329/data12_8TeV.00209265.physics_JetTauEtmiss.merge.NTUP_SUSYSKIM.r4065_p1278_p1328_p1329_tid01106323_00/NTUP_SUSYSKIM.01106323._000003.root.1"

./readDirect $filenameeos $treeToUse 100 30 >& info.txt
echo " --------- info.txt ----------"
cat info.txt
echo " -----------------------------"
python uploaderDPM.py "DPM Root Read 100% TTC" "100"
echo -n "time> DPM-test > test 100,30 Eos finished "; date

./readDirect $filenameeos $treeToUse 1 30 >& info.txt
echo " --------- info.txt ----------"
cat info.txt
echo " -----------------------------"
python uploaderDPM.py "DPM Root Read 1% TTC" "1"
echo -n "time> DPM-test > test 1,30 Eos finished "; date

filenameeos="root://eosatlas.cern.ch//eos/atlas/atlasdatadisk/data12_8TeV/NTUP_SUSYSKIM/r4065_p1278_p1328_p1329/data12_8TeV.00209265.physics_JetTauEtmiss.merge.NTUP_SUSYSKIM.r4065_p1278_p1328_p1329_tid01106323_00/NTUP_SUSYSKIM.01106323._000003.WIGNER.root.1"

export COPY_TOOL=eos-wigner;

./readDirect $filenameeos $treeToUse 1 30 >& info.txt
echo " --------- info.txt ----------"
cat info.txt
echo " -----------------------------"
python uploaderDPM.py "DPM Root Read 1% TTC" "1"
echo -n "time> DPM-test > test 1,30 Eos 2 finished "; date

fi

export COPY_TOOL=xrdcp;
#root -l -q -b "readint.C++(\"$filenamerfio\",\"$treeToUse\", 100, 30)" >& info.txt
#./readDirect $filenamexrootd $treeToUse 100 30 >& info.txt
root.exe -l -q -b "readDPMWebDav.C++(\"$filenamexrootd\",\"$treeToUse\", 100, 30,\"\",\"$X509_USER_PROXY\",\"$X509_CERT_DIR\")" >& info.txt
echo " --------- info.txt ----------"
cat info.txt
echo " -----------------------------"
python uploaderDPM.py "DPM Root Read 100% TTC" "100"
echo -n "time> DPM-test > test 100,30 Xrootd finished "; date

root.exe -l -q -b "readDPMWebDav.C++(\"$filenamexrootd\",\"$treeToUse\", 1, 30,\"\",\"$X509_USER_PROXY\",\"$X509_CERT_DIR\")" >& info.txt
echo " --------- info.txt ----------"
cat info.txt
echo " -----------------------------"
python uploaderDPM.py "DPM Root Read 1% TTC" "1"
echo -n "time> DPM-test > test 1,30 Xrootd finished "; date

#export LD_LIBRARY_PATH=/cvmfs/atlas.cern.ch/repo/sw/software/x86_64-slc5-gcc43-opt/17.6.0/LCGCMT/LCGCMT_63/InstallArea/x86_64-slc5-gcc43-opt/lib/
#source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh
#source ${ATLAS_LOCAL_ROOT_BASE}/packageSetups/atlasLocalROOTSetup.sh --skipConfirm
#source ${ATLAS_LOCAL_ROOT_BASE}/packageSetups/atlasLocalGccSetup.sh gcc436_x86_64_slc5
#source ${ATLAS_LOCAL_ROOT_BASE}/packageSetups/atlasLocalPythonSetup.sh --pythonVersion=2.6.5p1-i686-slc5-gcc43
which root.exe
unset HTTP_PROXY
unset http_proxy
echo "=o= DPM WebDav no cache"
export COPY_TOOL=https

root.exe -l -q -b "readDPMWebDav.C++(\"$filename\",\"$treeToUse\", 100, 30,\"\",\"$X509_USER_PROXY\",\"$X509_CERT_DIR\")" >& info.txt
echo " --------- info.txt ----------"
cat info.txt
echo " -----------------------------"
python uploaderDPM.py "DPM Root Read 100% TTC" "100"
echo -n "time> DPM-test > test 100,0 WebDav finished "; date

root.exe -l -q -b "readDPMWebDav.C++(\"$filename\",\"$treeToUse\", 1, 30,\"\",\"$X509_USER_PROXY\",\"$X509_CERT_DIR\")" >& info.txt
echo " --------- info.txt ----------"
cat info.txt
echo " -----------------------------"
python uploaderDPM.py "DPM Root Read 1% TTC" "1"
echo -n "time> DPM-test > test 1,0 WebDav finished "; date


#echo "=o= checking out ROOT"
#curl -s ftp://root.cern.ch/root/root_v5.32.03.source.tar.gz > root_v5.32.03.source.tar.gz
#echo "=o= gunzip ROOT"
#gunzip -q root_v5.32.03.source.tar.gz
#echo "=o= untar" 
#tar xf root_v5.32.03.source.tar
#rm -f root_v5.32.03.source.tar
#echo "=o= patch" 
#cp root-webfile-https-1.patch root/net/net/src
#cd root/net/net/src
#patch -p0 < root-webfile-https-1.patch
#cd ../../../
#echo "=o= configuring it"
#./configure linux --disable-x11 --disable-mysql --disable-opengl --disable-asimage --disable-fftw3 --disable-tmva --disable-python --enable-builtin-freetype
#make --quiet
#. bin/thisroot.sh
#cd ..
#printenv

#echo "=o= DPM WebDav plain http"
#export COPY_TOOL=http
#root.exe -l -q -b "readDPMWebDav.C++(\"$filenamehttp\",\"$treeToUse\", 100, 0,\"\",\"$X509_USER_PROXY\",\"$X509_CERT_DIR\",100)" >& info.txt
#echo " --------- info.txt ----------"
#cat info.txt
#echo " -----------------------------"
#python uploaderDPM.py "DPM WebDav 100 Ev" "100"
#echo -n "time> DPM-test > test 100,0 WebDav finished "; date

#echo "=o= DPM WebDav curl copy"
#curl -v -E $X509_USER_PROXY --capath $X509_CERT_DIR -L $filename -o ./group.test.hc.NTUP_SMWZ.root 
#python uploaderDPM.py "DPM Copy" "100"
#echo -n "time> DPM-test > WebDav curl copy finished"; date
rm -f ./group.test.hc.NTUP_SMWZ.root



