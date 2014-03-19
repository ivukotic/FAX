You may find the same content here:
https://twiki.cern.ch/twiki/bin/viewauth/AtlasComputing/UsingFAXforEndUsersTutorial


How to use FAX - Tutorial 
=========================




 This is a step-by-step guide on how to use FAX that you may simply follow and copy-paste commands. If desired you may download all of it from: https://twiki.cern.ch/twiki/pub/AtlasComputing/UsingFAXforEndUsersTutorial/Tutorial.tar. If something does not work or you have any questions please feel free to contact atlas-adc-fax-operations@cernNOSPAMSPAMNOT.ch



1. Set up environment

    Setting up fax is trivially done using localSetupFAX:

    export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase
    source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh
    localSetupFAX
    
    FAX access endpoint is automatically setup for you and you can find it in the environment variable STORAGEPREFIX.

    You'll need a grid proxy (you need a valid ATLAS grid certificate):
    
    voms-proxy-init -voms atlas
    
    
    
2. Check availability of your dataset/datacontainer in FAX

    The tool to do that is called isDSinFAX.py and is available upon setting up localSetupFAX. Usage is trivial: just give it a dataset or datacontainer name. For each input dataset it will print number of FAX endpoints containing full and incomplete replicas. Example:

    isDSinFAX.py user.ilijav.HCtest.1


    There is one more tool doing the same task:
    fax_ls.py user.ilijav.HCtest.1
    

3. Find gLFN's of your files 

    Use tool fax-get-gLFNs.sh:

    fax-get-gLFNs.sh user.ilijav.HCtest.1  > my_list_of_gLFNS.txt 

    

4.  Simple file copy, open, inspection

    copy it localy:

    xrdcp $STORAGEPREFIX/atlas/rucio/user/ivukotic:group.test.hc.NTUP_SMWZ.root /tmp/myLocalCopy.root.1 

    Open and list file content of the file in ROOT. Start by setting up ROOT

    localSetupROOT

     root -l
          TFile *f = TFile::Open("root://fax.mwt2.org//atlas/rucio/user/ivukotic:group.test.hc.NTUP_SMWZ.root"); 
          if(f) f->ls();
          .q
    
    
5. Listing number of events in each DS file 
    listNumberOfEvents.C opens each of the files listed in a file and prints number of events in them
    
    create a file listing all files of a dataset:
    fax-get-gLFNs.sh user.flegger.MWT2.data12_8TeV.00212172.physics_Muons.merge.NTUP_SMWZ.f479_m1228_p1067_p1141_tid01007411_00 > MWT2_files.txt
    
    execute it by doing:
    root -l -b -q listNumberOfEvents.C

6. Reading-writing a file 
    compile the scripts:
    make

    remotely read from file 10% of events using 30MB of TTreeCache memory.
    ./readDirect root://fax.mwt2.org:1094//atlas/rucio/user/ivukotic:group.test.hc.NTUP_SMWZ.root physics 10 30 

    remotely read from file 10% of events using 30MB of TTreeCache memory and write to a remote xrootd server:
    ./readWrite root://fax.mwt2.org:1094//atlas/rucio/user/ivukotic:group.test.hc.NTUP_SMWZ.root  root://faxbox.usatlas.org:1094//  physics 10 30

7. Simple SkimSlim example 
    This example will use a standard Athena SkimSlim script filter-and-merge-d3pd.py 

    filter-and-merge-d3pd.py.txt: SkimSlim script. You get it in your path upon setting up Athena.
    branchesList: branchesList - contains list of all of branches we want in the output files
    cutCode: cutCode - a simple python function returning true for events we want to preserve
    inputFileList: inputFileList - list of all the input files. 

    Simply execute it:
    python filter-and-merge-d3pd.py  --in=inputFileList --out=SkimmedSlimmed.root --tree=physics --var=branchesList --selection=file:cutCode
    
8. SkimSlim example from T3 
 In this example we will submit just one simple job doing SkimSlim of an NTuple on a local batch system - condor.

    filter-and-merge-d3pd.py.txt: SkimSlim script. You get it in your path upon setting up Athena. If downloading it, you'll have to remove extension .txt added by TWiki.
    branchesList: contains list of all of branches we want in the output files.
    cutCode: a simple python function returning true for events we want to preserve.
    inputFileList: - list of all the input files.
    job.sub: job.sub - condor job description script.
    SkimSlimT3.sh: script executed at the worker node. 

    The SkimSlimT3.sh contains the commands we would normally use to start the job:

    #!/bin/zsh
    export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase
    source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh
    source $AtlasSetup/scripts/asetup.sh 17.6.0,noTest
    #here substitute your proxy
    export X509_USER_PROXY=x509up_u20074
    cat inputFileList
    python filter-and-merge-d3pd.py  --in=inputFileList --out=SkimmedSlimmed.root --tree=physics --var=branchesList --selection=file:cutCode

    The submit script is this:

    getenv         = False
    executable     = SkimSlimT3.sh
    output         = SkimSlimT3.out
    error          = SkimSlimT3.error
    log            = SkimSlimT3.log
    transfer_input_files = filter-and-merge-d3pd.py,/tmp/x509up_u20074,inputFileList,branchesList,cutCode
    universe       = vanilla
    #Requirements   = (regexp("^uc3.*", TARGET.Machine,"IM") == True)
    Requirements   = HAS_CVMFS =?= True
    queue 1

    It is important that in a line listing files to be transferred to a worker node you put full path to your grid proxy.

    To submit the job for execution do:
    condor_submit job.sub

    you may follow execution of your job by doing:
    condor_q YourUserName

9. A large SkimSlim example from T3 
    
    In this example we will submit a more complex condor job doing SkimSlim of an NTuple on a local batch system.

    You will need these files:

    filter-and-merge-d3pd.py.txt: SkimSlim script. You get it in your path upon setting up Athena. If downloading it, you'll have to remove extension .txt added by TWiki.
    branchesList: contains list of all of branches we want in the output files.
    cutCode: a simple python function returning true for events we want to preserve.
    inputFileListLarge: - list of all the input files.
    jobLarge.sub - condor job description script.
    SkimSlimLarge.sh: script executed at the worker node. 

    The SkimSlimLarge.sh now not only has commands that actually execute the code, but also extracts a part of the large input data files list into a new file which will be analyzed but the a specific job:

    #!/bin/zsh
    export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase
    source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh
    source $AtlasSetup/scripts/asetup.sh 17.6.0,noTest
    export X509_USER_PROXY=x509up_u20074
    echo 'job:' $1, 'from:' $2
    files=$(wc -l <inputFileListLarge)
    echo 'input files:' $files
    awk -v jo=$1 -v totjobs=$2 -v len=$files 'BEGIN {slice = len/totjobs; start = jo*slice; end = (jo+1)*slice;} NR > start && NR <= end {print}' inputFileListLarge > inputFileList
    cat inputFileList
    python filter-and-merge-d3pd.py  --in=inputFileList --out=SkimmedSlimmed_$1.root --tree=physics --var=branchesList --selection=file:cutCode

    This time we submit 10 jobs and name output files accordingly:

    Jobs = 10
    getenv         = False
    executable     = SkimSlimLarge.sh
    output         = SkimSlimLarge.out.$(Process)
    error          = SkimSlimLarge.error.$(Process)
    log            = SkimSlimLarge.log.$(Process)
    arguments = $(Process) $(Jobs)
    transfer_input_files = filter-and-merge-d3pd.py,/tmp/x509up_u20074,inputFileListLarge,branchesList,cutCode
    universe       = vanilla
    #Requirements   = (regexp("^uc3.*", TARGET.Machine,"IM") == True)
    Requirements   = HAS_CVMFS =?= True
    queue $(Jobs)

    It is important that in a line listing files to be transferred to a worker node you put full path to your grid proxy.

    To submit the job for execution do:
    condor_submit job.sub

    you may follow execution of your job by doing:
    condor_q YourUserName


10. Analysis-like example 

    This example will use condor batch queue to read a lot of events from a large number of files, does simple selection and writes selected events to new files. You will need the following files:

    ANALY_like.sh: ANALY_like.sh
    inputFileListLarge: inputFileListLarge
    jobLarge.sub: jobLarge.sub
    Makefile: Makefile
    readWrite.C: readWrite.C 

    This time we will not use Athena but setup the minimal ROOT/xrootd combination. Each worker node will compile the code. An output will go to a remote xrootd server.

    #!/bin/bash
    A=$PWD
    echo "Setting up gcc"
    source /cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/Gcc/gcc472_x86_64_slc6/setup.sh
    echo "Setting up ROOT"
    cd /cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/root/5.34.09-x86_64-slc6-gcc4.7/
    source bin/thisroot.sh
    echo "Setting up xRootD"
    source /cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/root/5.34.09-x86_64-slc6-gcc4.7/bin/setxrd.sh /cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/xrootd/3.2.7-x86_64-slc6/v3.2.7/ 2>&1 > /dev/null
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/xrootd/3.2.7-x86_64-slc6/v3.2.7/lib
    cd $A

    # replace with your proxy!
    export X509_USER_PROXY=x509up_u20074

    make

    echo 'job:' $1, 'from:' $2
    files=$(wc -l <inputFileListLarge)
    echo 'input files:' $files
    inputFile=$(awk -v jo=$1 'NR == jo {print}' inputFileListLarge)
    cat inputFileList

    ./readWrite $inputFile  root://faxbox.usatlas.org:1094//user/ilijav/rW_$1.root  physics  5  30

    This time we submit much more jobs:

    Jobs = 2232
    getenv         = False
    executable     = ANALY_like.sh
    output         = rW.out.$(Process)
    error          = rW.error.$(Process)
    log            = rW.log.$(Process)
    arguments = $(Process) $(Jobs)
    should_transfer_files = IF_NEEDED
    transfer_input_files = Makefile,readWrite.C,/tmp/x509up_u20074,inputFileListLarge
    universe       = vanilla
    Requirements   = HAS_CVMFS =?= True
    queue $(Jobs)



11. Using FAX from prun: Instead of giving   "--inDS myDataset"   option, provide it with  "--pfnList my_list_of_gLFNS.txt"



