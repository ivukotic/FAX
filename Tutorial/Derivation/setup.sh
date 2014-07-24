 export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase
 source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh --quiet

 localSetupFAX --rootVersion=current-SL6 --xrootdVersion=current-SL6

 voms-proxy-init -voms atlas

echo "creating input file list"
fax-get-gLFNs valid2.117050.PowhegPythia_P2011C_ttbar.digit.AOD.e2657_s1933_s1964_r5534/ > inputDataList.txt

asetup 19.1.X.Y-VAL,rel_4,AtlasDerivation,here

Reco_tf.py --inputAODFile inputXAOD.pool.root --outputDAODFile output --reductionConf TEST1 TEST2 TEST3 TEST4
