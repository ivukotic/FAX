#include <stdlib.h>

#include "Riostream.h"
#include "TROOT.h"
#include "TFile.h"
#include "TFileCollection.h"
#include "TNetFile.h"
#include "TRandom.h"
#include "TTree.h"
#include "TTreeCache.h"
#include "TTreePerfStats.h"
#include "TBranch.h"
#include "TClonesArray.h"
#include "TStopwatch.h"
#include "TTreeCacheUnzip.h"
#include "TProof.h"
#include "TProofLog.h"
#include "TProofMgr.h"
#include "TChain.h"
#include "TPaveText.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include "machineInfo.h"

int run_chain_ttbarSel_02_00()
{

  TStopwatch timer;
  bool checkHDD=true;
  long long diskS1[11];
  long long diskS2[11];
  long long netSt1[2];
  long long netSt2[2];

/*
  string str2("dcap://");
  size_t found;
  found=fn.find(str2);
  if (found!=string::npos){
        cout << "dcap - no disk accesses available." << endl;
        checkHDD=false;
                cout<<"FILESYSTEM='dcap'"<<endl;
  }

  str2="root://";
  found=fn.find(str2);
  if (found!=string::npos){
        cout << "xrootd - no disk accesses available." << endl;
        checkHDD=false;
        cout<<"FILESYSTEM='root'"<<endl;
  }

  if (checkHDD){
      char buf[2048];
      int count = readlink(fn.c_str(), buf, sizeof(buf));
      if (count >= 0) {
         buf[count] = '\0';
         printf("%s -> %s\n", fn.c_str(), buf);
      }
      fn = string(buf);

      diskSt(diskS1, fn);
  }
*/

  // gSystem->Load("libPhysics");

  Bool_t runProof = kFALSE;
  const char* server = "";
  Int_t nworkers = 0;

  TString runMode = "NonProof"; // "NonProof","Proof", "ProofLite"

  if (runMode == "Proof") {
    // case-1: PROOF server
    // ====================
    runProof = kTRUE;
    server = "bnlt3s01";
    nworkers = 7;
    // server = "acas1010";
    // nworkers = 24;

  } else if (runMode == "ProofLite") {
    // case-2: PROOF-Lite
    // ====================
    runProof = kTRUE;
    server = "";
    nworkers = 3;

  } else if (runMode == "NonProof") {
    // case-3: Non-Proof
    // ====================
    runProof = kFALSE;
    server = "";
    nworkers = -1;
  } else {
    cout << "Unknown runMode=" << runMode << ", exit now" << endl;
    gSystem->Exit(1);
  }

  TProof* proof = 0;

  // filename to list ntuple vars in analysis
  TString varsFilename = TString::Format("%s/list_varsUsed.txt", gSystem->WorkingDirectory());

  TString fout = TString::Format("%s/myOutput.root", gSystem->WorkingDirectory());
  gSystem->Setenv("OutFilename",fout.Data());

  const char* Cuts_Filename = "ttbarSel-Cuts.txt";
  gSystem->Setenv("Cuts_Filename",Cuts_Filename);
  gSystem->Setenv("VARSFilename", varsFilename.Data());
  //if (cut_GRL) gSystem->Setenv("XMLFilename",fxml.Data());

  TChain* chain = new TChain("physics");
  chain->SetCacheSize(10*1024*1024);
  chain->SetCacheLearnEntries(5);
  TString workDir(gSystem->WorkingDirectory());
  //TFileCollection fc("fc","list of input root files","input.txt");
  //chain->AddFileInfoList(fc.GetList());
  //chain->AddFileInfoList((TCollection*)&fc,1);
  ifstream input("input.txt");
  long totsize = 0;
  long zipsize = 0;
  string line;
  TFile* f = 0;
  if (input.is_open()) {
     while (input.good()) {
        getline(input,line);
        chain->AddFile(line.c_str());
        f = new TFile(line.c_str());
        if (f!=0) {
           totsize += f->GetSize();
        }
        else {cout << "cannot open " << line << endl;}
        delete f;
     }
  }
  else {cout << "cannot open input.txt" << endl;}
  //cout << "TOTALSIZE=" << totsize << endl;

  // chain->Add("root://xrd//data/user.hma.data10_7TeV.00159086.physics_L1Calo.merge.NTUP_JETMET.f275_p191.slimmed.V5.4/user.hma.data10_7TeV.00159086.physics_L1Calo.merge.NTUP_JETMET.f275_p191.slimmed.V5.4._00001.output.root.1");

/*
  chain->MakeSelector("BaseTree_ttbar");
  gSystem->Exec("mv BaseTree_ttbar.h BaseTree_ttbar.C BaseTree_ttbar/");

  gSystem->Exec("NewType=`sed -n 's/*jet_flavor_weight_SV0/*my_jet_flavor_weight/p' BaseTree_ttbar/BaseTree_ttbar.h`; sed \"/*my_jet_flavor_weight/c\\ $NewType\" ttbarSel.h-kept > ttbarSel.h-new; mv ttbarSel.h-new ttbarSel.h");
*/

  //+ListVarsUsed
  // you can comment the following block after the first running and there is 
  //  no new ntuple variables added into the analysis code.
  gSystem->Exec("./list_varsUsed.sh");
  //-ListVarsUsed

  gROOT->LoadMacro("BaseTree_ttbar/BaseTree_ttbar.C+");
  //gROOT->LoadMacro("ttbarSel.C+");
  if (runProof) {

    // force proof server uses the same ROOT version as client
    if (server != "") {
      // TString rootVer_dir = gSystem->Exec("echo $ROOTSYS | sed 's%.*root/\\([0-9].*\\)/.*/root%\\1%'");
      const char *rootVer_dir = gSystem->BaseName( gSystem->DirName( gSystem->DirName(gSystem->Getenv("ROOTSYS"))));
      TProof::AddEnvVar("PROOF_INITCMD", TString::Format("%s %s","echo source /usatlas/u/yesw2000/bin/root_set-slc5.sh",rootVer_dir) );
    }

    TProof::AddEnvVar("OutFilename",fout.Data());
    TProof::AddEnvVar("Cuts_Filename",Cuts_Filename);
    TString str_server(server);
    Bool_t IsProofLite = kFALSE;
    if (str_server.Length() == 0) {
       IsProofLite = kTRUE;
       if (nworkers>0) {
          str_server = TString::Format("workers=%d",nworkers);
       }
    }
    proof = TProof::Open(str_server.Data());
    if (!IsProofLite && nworkers>0) proof->SetParallel(nworkers);

    // for Proof-Lite
    if (IsProofLite) proof->SetParameter("PROOF_Packetizer","TPacketizer");

    //+Option: sub-mergers
    // you may like to enable sub-mergers in case that the merging process
    // takes a long time, set to Zero for dynamic setting number of sub-mergers.

    // proof->SetParameter("PROOF_UseMergers", 0);
    // proof->SetParameter("PROOF_UseMergers", 2);

    //-Option: sub-mergers

    // proof->Exec(".!hostname; echo ROOTSYS=$ROOTSYS");

    gSystem->Exec("gtar cfz BaseTree_ttbar.par BaseTree_ttbar/");
    proof->UploadPackage("BaseTree_ttbar.par");
    proof->EnablePackage("BaseTree_ttbar.par",kTRUE);

  }
  gSystem->AddIncludePath("-I./GRL/");
  gSystem->Load("GRL/libGoodRunsLists.so");

  timer.Start();
  //gSystem->Load("machineInfo_C.so");
  if (runProof) chain->SetProof();

  netSt(netSt2);
  TTree* thus = chain->GetTree();
  cout << "EVENTS="   << chain->GetEntries() << "  " << endl;
  //cout << "TOTALSIZE="<< chain->GetTotBytes()    << "  " << endl;
  //cout << "ZIPSIZE="  << chain->GetTree()->GetZipBytes()    << "  " << endl;
  cout << "CACHESIZE="<< 10*1024*1024            << "  " << endl;

  // you can use the option to control the printout in your code
  // and writing out the old tree for selected events.
  // chain->Process("ttbarSel.C+","Debug nPrint=1 No_selTree",5);
  // chain->Process("ttbarSel.C+","nPrint=200 No_selTree",20001);
  chain->Process("./ttbarSel.C+","nPrint=500 No_selTree");
  //chain->Process("./ttbarHarness.C+","nPrint=500 No_selTree");
  chain->PrintCacheStats();
  timer.Stop(); 
  cout<<"CPUTIME=" <<timer.CpuTime()<<endl;
  cout<<"WALLTIME="<<timer.RealTime()<<endl;
  //timer.Print();

  cout<<"ROOTVERSION="<<gROOT->GetSvnRevision()<<endl;
  cout<<"ROOTBRANCH='"<<gROOT->GetSvnBranch()<<"'"<<endl;
  getCPUusage();
  getMemoryInfo();
/*
  if (checkHDD){
     diskSt(diskS2, fn);
     cout<<"HDDREADS="<<diskS2[0]-diskS1[0]<<endl;
     cout<<"reads merged:  "<<diskS2[1]-diskS1[1]<<endl;
     cout<<"reads sectors: "<<diskS2[2]-diskS1[2]<<"   in Mb: "<<(diskS2[2]-diskS1[2])/2048<<endl;
     cout<<"HDDTIME="<<diskS2[3]-diskS1[3]<<endl;
  }
*/
  netSt(netSt2);
  cout<<"ETHERNETIN="<<netSt2[0]-netSt1[0]<<endl;
  cout<<"ETHERNETOUT="<<netSt2[1]-netSt1[1]<<endl;

  // write out log file on Proof master/worker nodes
  if (runProof) {
    TProofLog *pl = proof->GetManager()->GetSessionLogs();
    pl->Save("*", "file_with_all_logs.txt");
  }

  TFile outFile(fout.Data(),"UPDATE");

  // the parameters for event selection criteria are 
  // writen into the same output root file
  TPaveText cutText(0.1,0.1,0.9,0.9,"NDC");
  cutText.SetName("cutText");
  cutText.ReadFile(Cuts_Filename);
  cutText.Write();

  gSystem->Exit(0);
  return 0;
}
