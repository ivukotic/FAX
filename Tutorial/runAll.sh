echo "*********************************************************************"
echo "*********************** Getting Proxy *******************************"
echo "*********************************************************************"
source ./setup.sh


echo "*********************************************************************"
echo "*********************** Is DS in FAX ? ******************************"
echo "*********************************************************************"

./whereIsTheDS.sh


echo "*********************************************************************"
echo "*********************** creating file lists *************************"
echo "*********************************************************************"

./createFileList.sh

echo "*********************************************************************"
echo "*********************** Getting one file ****************************"
echo "*********************************************************************"
cd GettingFile
./gettingFile.sh
cd ..

echo "*********************************************************************"
echo "*********************** Reading file using ROOT *********************"
echo "*********************************************************************"
cd InspectingFile
./inspectingFile.sh
cd ..

echo "*********************************************************************"
echo "*********************** Reading multiple files **********************"
echo "*********************************************************************"
cd InspectingFiles
./inspectingFiles.sh
cd ..

echo "*********************************************************************"
echo "*********************** Reading and Writing Files *******************"
echo "*********************************************************************"
cd ReadingWritingFile
./readingFile.sh
./readWriteFile.sh
cd ..

echo "*********************************************************************"
echo "*********************** Skimming Sliming interactively **************"
echo "*********************************************************************"
cd SkimSlim
./SkimSlim.sh
cd ..

echo "*********************************************************************"
echo "*********************** Skimming Sliming in condor ******************"
echo "*********************************************************************"
cd SkimSlimT3
./skimSlimT3.sh
cd ..

echo "*********************************************************************"
echo "*********************** Large Skim Slim campaign ********************"
echo "*********************************************************************"
cd SkimSlimT3Large
./skimSlimT3Large.sh
cd ..

echo "*********************************************************************"
echo "*********************** ALL DONE ************************************"
echo "*********************************************************************"
