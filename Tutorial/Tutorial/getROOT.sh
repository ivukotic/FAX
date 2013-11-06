##############  getROOT.sh #################
#!/bin/bash
A=$PWD
echo "Setting up gcc"
source /cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/Gcc/current/setup.sh
echo "Setting up ROOT"
cd /cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/root/5.34.07-x86_64-slc5-gcc4.3/
source bin/thisroot.sh
echo "Setting up xRootD"
source /cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/root/5.34.07-x86_64-slc5-gcc4.3/bin/setxrd.sh /cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/xrootd/3.2.4-x86_64-slc5/v3.2.4/ 2>&1 > /dev/null
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/xrootd/3.2.4-x86_64-slc5/v3.2.4/lib
cd $A
