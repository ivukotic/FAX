#define ttbarSel_cxx
// The class definition in ttbarSel.h has been generated automatically
// by the ROOT utility TTree::MakeSelector(). This class is derived
// from the ROOT class TSelector. For more information on the TSelector
// framework see $ROOTSYS/README/README.SELECTOR or the ROOT User Manual.

// The following methods are defined in this file:
//    Begin():        called everytime a loop on the tree starts,
//                    a convenient place to create your histograms.
//    SlaveBegin():   called after Begin(), when on PROOF called only on the
//                    slave servers.
//    Process():      called for each event, in this function you decide what
//                    to read and fill your histograms.
//    SlaveTerminate: called at the end of the loop on the tree, when on PROOF
//                    called only on the slave servers.
//    Terminate():    called at the end of the loop on the tree,
//                    a convenient place to draw/fit your histograms.
//
// To use this file, try the following session on your Tree T:
//
// Root > T->Process("ttbarSel.C")
// Root > T->Process("ttbarSel.C","some options")
// Root > T->Process("ttbarSel.C+")
//

#include "ttbarSel.h"
#include <TH2.h>
#include <TStyle.h>
#include <Riostream.h>
#include <TSystem.h>
#include <TRegexp.h>


Bool_t ExtractParVal(TString &str_line, Float_t &parVal);

void ttbarSel::Init(TTree *tree)
{
  cout << "Come to ttbarSel::Init" << endl;

  selTree = tree;

  // Check for existing tps
  if (ps!=0) {
    cout << "Error: TTreePerfStats not properly cleaned up!" << endl;
  }
  // Now make new tps
  else if (ps==0) {
    cout << "Creating TTreePerfStats for TTree " << tree << endl;
    ps = new TTreePerfStats( "ioperf", tree );
  }

  BaseTree_ttbar::Init(tree);

  tree->SetCacheSize(10*1024*1024);
  //tree->SetCacheLearnEntries(100);

  if(!newTree){
    if (fFile != NULL) fFile->cd();
    if (fWrite_selTree) {
      fselTree = (TTree*)tree->CloneTree(0);
      fselTree->SetName("fselTree");
      fselTree->SetTitle("Selected events");
    }

    // new tree and its new branch variables
    newTree = new TTree("newTree","info of reconstructed ttbar evts");

    newTree->Branch("Wjj_n",    &m_Wjj_n, "Wjj_n/I");
    // newTree->Branch("Wjj_mode", &m_Wjj_mode);
    newTree->Branch("Wjj_mass", &m_Wjj_mass);
    newTree->Branch("Wjj_pt",   &m_Wjj_pt);
    newTree->Branch("Wjj_eta",  &m_Wjj_eta);
    newTree->Branch("Wjj_delR", &m_Wjj_delR);

    if (fselTree) newTree->Branch("Wjj_j1_i",   &m_Wjj_j1_i);
    newTree->Branch("Wjj_j1_pt",   &m_Wjj_j1_pt);
    newTree->Branch("Wjj_j1_eta",  &m_Wjj_j1_eta);
    newTree->Branch("Wjj_j1_wt",   &m_Wjj_j1_wt);

    if (fselTree) newTree->Branch("Wjj_j2_i",   &m_Wjj_j2_i);
    newTree->Branch("Wjj_j2_pt",   &m_Wjj_j2_pt);
    newTree->Branch("Wjj_j2_eta",  &m_Wjj_j2_eta);
    newTree->Branch("Wjj_j2_wt",   &m_Wjj_j2_wt);
 
    newTree->Branch("Wlv_n",    &m_Wlv_n, "Wlv_n/I");
    newTree->Branch("Wlv_mode", &m_Wlv_mode);
    newTree->Branch("Wlv_mass", &m_Wlv_mass);
    newTree->Branch("Wlv_pt",   &m_Wlv_pt);
    newTree->Branch("Wlv_eta",  &m_Wlv_eta);
    newTree->Branch("Wlv_delR", &m_Wlv_delR);
    newTree->Branch("Wlv_pt_nu",    &m_Wlv_pt_nu);
    newTree->Branch("Wlv_eta_nu",   &m_Wlv_eta_nu);
    // newTree->Branch("Wlv_phi_nu",   &m_Wlv_phi_nu);
    if (fselTree) newTree->Branch("Wlv_lep_i", &m_Wlv_lep_i);

    newTree->Branch("top_n",      &m_top_n,  "top_n/I");
    newTree->Branch("top_lvb_n",  &m_top_lvb_n,  "top_lvb_n/I");
    newTree->Branch("top_jjb_n",  &m_top_jjb_n,  "top_jjb_n/I");
    newTree->Branch("top_mode", &m_top_mode);
    newTree->Branch("top_W_i",  &m_top_W_i);
    if (fselTree) newTree->Branch("top_b_i",  &m_top_b_i);
    newTree->Branch("top_mass", &m_top_mass);
    newTree->Branch("top_pt",   &m_top_pt);
    newTree->Branch("top_eta",  &m_top_eta);
    // newTree->Branch("top_phi", &m_top_phi);
    newTree->Branch("top_delR", &m_top_delR);
    newTree->Branch("top_b_pt", &m_top_b_pt);
    newTree->Branch("top_b_eta", &m_top_b_eta);
    // newTree->Branch("top_b_phi", &m_top_b_phi);
    newTree->Branch("top_b_WT", &m_top_b_WT);
 
    newTree->Branch("ttbar_n",&m_ttbar_n, "ttbar_n/I");
    // newTree->Branch("ttbar_mode", &m_ttbar_mode);
    newTree->Branch("ttbar_mass", &m_ttbar_mass);
    newTree->Branch("ttbar_pt",   &m_ttbar_pt);
    newTree->Branch("ttbar_eta",  &m_ttbar_eta);
    // newTree->Branch("ttbar_phi",  &m_ttbar_phi);
    newTree->Branch("ttbar_delR", &m_ttbar_delR);

    newTree->Branch("ttbar_lvb_i",     &m_ttbar_lvb_i);
    newTree->Branch("ttbar_jjb_i",     &m_ttbar_jjb_i);

    if (fFile == NULL) {
      if (fselTree) fOutput->Add(fselTree);
      fOutput->Add(newTree);
    } else {
      // content in TProofOutput need not "to be Added" into to fOutput
      if (fselTree) fselTree->AutoSave();
      newTree->AutoSave();
    }
  } else {
    if (fselTree) tree->CopyAddresses(fselTree);
  }

}

void ttbarSel::Begin(TTree * /*tree*/)
{
   // The Begin() function is called at the start of the query.
   // When running with PROOF Begin() is only called on the client.
   // The tree argument is deprecated (on PROOF 0 is passed).

  cout << "Come to ttbarSel::Begin" << endl;

  TString option = GetOption();


}


Bool_t ttbarSel::Notify() {
  cout << "Notify file change: " << fileindex << endl;
  fileindex++;

  ftotsize += selTree->GetTree()->GetTotBytes();
  fzipsize += selTree->GetTree()->GetZipBytes();

  my_jet_flavor_weight = jet_flavor_weight_SV0;
  return kTRUE;
}

void ttbarSel::InitVectAndHist()
{
  if (fDebug) cout <<"Beginning of ttbarSel::InitVectAndHist" << endl;

  // initialize branch variables
  m_Wjj_lvec = new vector<TLorentzVector>;
  m_Wjj_n    = 0;
  // m_Wjj_mode = new vector<int>;
  m_Wjj_mass = new vector<double>;
  m_Wjj_pt   = new vector<double>;
  m_Wjj_eta  = new vector<double>;
  m_Wjj_delR = new vector<double>;
  m_Wjj_j1_i    = new vector<int>;
  m_Wjj_j2_i    = new vector<int>;
  m_Wjj_j1_pt   = new vector<double>;
  m_Wjj_j1_eta  = new vector<double>;
  m_Wjj_j1_wt   = new vector<double>;
  m_Wjj_j2_pt   = new vector<double>;
  m_Wjj_j2_eta  = new vector<double>;
  m_Wjj_j2_wt   = new vector<double>;

  m_Wlv_lvec  = new vector<TLorentzVector>;
  m_Wlv_n     = 0;
  m_Wlv_lep_i = new vector<int>;
  m_Wlv_mode  = new vector<int>;
  m_Wlv_mass  = new vector<double>;
  m_Wlv_pt    = new vector<double>;
  m_Wlv_eta   = new vector<double>;
  m_Wlv_delR  = new vector<double>;
  m_Wlv_pt_nu    = new vector<double>;
  m_Wlv_eta_nu   = new vector<double>;
  // m_Wlv_phi_nu   = new vector<double>;

  m_top_lvec = new vector<TLorentzVector>;
  m_top_n = 0;
  m_top_lvb_n = 0;
  m_top_jjb_n = 0;
  m_top_mode = new vector<int>;
  m_top_W_i  = new vector<int>;
  m_top_b_i  = new vector<int>;
  m_top_mass = new vector<double>;
  m_top_pt   = new vector<double>;
  m_top_eta  = new vector<double>;
  // m_top_phi  = new vector<double>;
  m_top_delR = new vector<double>;
  m_top_b_pt = new vector<double>;
  m_top_b_eta = new vector<double>;
  // m_top_b_phi = new vector<double>;
  m_top_b_WT  = new vector<double>;

  m_ttbar_n = 0;
  // m_ttbar_mode = new vector<int>;
  m_ttbar_mass =  new vector<double>;
  m_ttbar_pt   = new vector<double>;
  m_ttbar_eta  = new vector<double>;
  // m_ttbar_phi  = new vector<double>;
  m_ttbar_delR = new vector<double>;
  m_ttbar_lvb_i    = new vector<int>;
  m_ttbar_jjb_i    = new vector<int>;


  // Initialize hists
  TDirectory *savedir = gDirectory;

  if (fFile) fFile->cd();

  h1_nEl = new TH1F("h1_nEl","No(elecs)", 30, -0.5, 29.5);
  if (!fFile) fOutput->Add(h1_nEl);

  h1_nMu = new TH1F("h1_nMu","No(muon)", 10, -0.5, 9.5);
  if (!fFile) fOutput->Add(h1_nMu);

  h1_nJet = new TH1F("h1_nJet","No(jet)", 100, -0.5, 99.5);
  if (!fFile) fOutput->Add(h1_nJet);

  h1_nWlv = new TH1F("h1_nWlv","No(W->lv)", 60, -0.5, 59.5);
  if (!fFile) fOutput->Add(h1_nWlv);

  h1_nWjj = new TH1F("h1_nWjj","No(W->jj)", 20, -0.5, 19.5);
  if (!fFile) fOutput->Add(h1_nWjj);

  h1_m_Wjj = new TH1F("h1_m_Wjj","mass(W->jj), GeV", 120, 50, 110);
  if (!fFile) fOutput->Add(h1_m_Wjj);

  h1_nTop_lvb = new TH1F("h1_nTop_lvb","No(Top->lvb)", 20, -0.5, 19.5);
  if (!fFile) fOutput->Add(h1_nTop_lvb);

  h1_m_lvb = new TH1F("h1_m_lvb","mass(Top->lvb), GeV", 120, 140, 200);
  if (!fFile) fOutput->Add(h1_m_lvb);

  h1_nTop_jjb = new TH1F("h1_nTop_jjb","No(Top->jjb)", 10, -0.5, 9.5);
  if (!fFile) fOutput->Add(h1_nTop_jjb);

  h1_m_jjb = new TH1F("h1_m_jjb","mass(Top->jjb), GeV", 120, 140, 200);
  if (!fFile) fOutput->Add(h1_m_jjb);

  h1_nttbar = new TH1F("h1_nttbar","No(ttbar)", 20, -0.5, 19.5);
  if (!fFile) fOutput->Add(h1_nttbar);

  h1_m_ttbar = new TH1F("h1_m_ttbar","mass(Top->ttbar), GeV", 120, 300, 1500);
  if (!fFile) fOutput->Add(h1_m_ttbar);

  gDirectory = savedir;


  if (fDebug) cout <<"End of ttbarSel::InitVectAndHist" << endl;
}

void ttbarSel::SlaveBegin(TTree * /*tree*/)
{
   // The SlaveBegin() function is called after the Begin() function.
   // When running with PROOF SlaveBegin() is called on each slave server.
   // The tree argument is deprecated (on PROOF 0 is passed).

  cout << "Come to ttbarSel::SlaveBegin" << endl;

  TString option = GetOption();

  if (option.Contains("No_fselTree") || option.Contains("NO_fselTree"))
     fWrite_selTree = kFALSE;

  option.ToUpper();
  if (option.Contains("DEBUG")) fDebug = kTRUE;

  Int_t idx_nPrint = option.Index("NPRINT=");
  if (idx_nPrint >= 0) {
    TString opt_nPrint = option(idx_nPrint+strlen("NPRINT="), option.Length());
    if (!opt_nPrint.IsDigit()) {
       TRegexp re("[^0-9]");
       Int_t idx_nonD = opt_nPrint.Index(re);
       opt_nPrint.Remove(idx_nonD);
    }
    if (fDebug) {
      cout << "option = " << option << ", idx_nPrint=" << idx_nPrint << ", opt_nPrint=" << opt_nPrint << endl;
    }
    fPrintEveryNevts = opt_nPrint.Atoi();
    cout << "Override fPrintEveryNevts = " << fPrintEveryNevts << endl;
   }

  if (fTimer==NULL) fTimer = new TStopwatch();
  fTimer->Start();

  TDirectory *savedir = gDirectory;
  const char *outFilename = gSystem->Getenv("OutFilename");
  const char *outFilename_base = gSystem->BaseName(outFilename);

  if (outFilename) {
    fProofFile = new TProofOutputFile(outFilename_base,"M");
    fProofFile->SetOutputFileName(outFilename);

    if (!(fFile = fProofFile->OpenFile("RECREATE"))) {
      Warning("SlaveBegin", "problems creating file: %s/%s",
      fProofFile->GetDir(), fProofFile->GetFileName());
    } else {
      fFile->cd();
      fFile->ls();
    }
  }

  gDirectory = savedir;

  InitVectAndHist();

  InitCriteria();
}

void ttbarSel::InitCriteria()
{
  // Initialize criteria parameters

  if (fDebug) cout <<"Beginning of ttbarSel::InitCriteria" << endl;

  // default value
  cJet_MinE = 0;
  cEl_MinE  = 0;
  cMu_MinE  = 0;
  cW_MassWin = 10*GeV;
  cTop_MassWin = 20*GeV;
  cJet_bWeightMin = 5.0;

  const char *Cuts_Filename = gSystem->Getenv("Cuts_Filename");
  if (Cuts_Filename) {
    ifstream cutFile("BaseTree_ttbar/ttbarSel-Cuts.txt");
    const int linesize = 255;
    char curline[linesize];
    TString str_line;

    while (cutFile.getline(curline,linesize)) {
      str_line = curline;

      if (str_line.BeginsWith("Jet_MinE")) {
         ExtractParVal(str_line,cJet_MinE);
         cJet_MinE *= GeV;
         if (fDebug) cout << "Setting cJet_MinE = " << cJet_MinE << endl;

      } else if (str_line.BeginsWith("El_MinE")) {
         ExtractParVal(str_line,cEl_MinE);
         cEl_MinE *= GeV;
         if (fDebug) cout << "Setting cEl_MinE = " << cEl_MinE << endl;

      } else if (str_line.BeginsWith("Mu_MinE")) {
         ExtractParVal(str_line,cMu_MinE);
         cMu_MinE *= GeV;
         if (fDebug) cout << "Setting cMu_MinE = " << cMu_MinE << endl;

      } else if (str_line.BeginsWith("W_MassWin")) {
         ExtractParVal(str_line,cW_MassWin);
         cW_MassWin *= GeV;
         if (fDebug) cout << "Setting cW_MassWin = " << cW_MassWin << endl;

      } else if (str_line.BeginsWith("Top_MassWin")) {
         ExtractParVal(str_line,cTop_MassWin);
         cTop_MassWin *= GeV;
         if (fDebug) cout << "Setting cTop_MassWin = " << cTop_MassWin << endl;

      } else if (str_line.BeginsWith("Jet_bWeightMin")) {
         ExtractParVal(str_line,cJet_bWeightMin);
         if (fDebug) cout << "Setting cJet_bWeightMin = " << cJet_bWeightMin << endl;

      } else {
        continue;
      }

    }

    cutFile.close();
  }

  if (fDebug) {
     cout << "cJet_MinE(MeV) = " << cJet_MinE
          << ", cW_MassWin(MeV)=" << cW_MassWin << endl;
  }

  if (fDebug) cout <<"End of ttbarSel::InitCriteria" << endl;
}

void ttbarSel::InitEvent()
{
  if (fDebug) cout <<"Beginning of ttbarSel::InitEvent" << endl;

  m_Wjj_lvec->clear();
  m_Wjj_n = 0;
  // m_Wjj_mode->clear();
  m_Wjj_mass->clear();
  m_Wjj_pt->clear();
  m_Wjj_eta->clear();
  m_Wjj_delR->clear();
  m_Wjj_j1_i->clear();
  m_Wjj_j2_i->clear();
  m_Wjj_j1_pt->clear();
  m_Wjj_j1_eta->clear();
  m_Wjj_j1_wt->clear();
  m_Wjj_j2_pt->clear();
  m_Wjj_j2_eta->clear();
  m_Wjj_j2_wt->clear();

  m_Wlv_lvec->clear();
  m_Wlv_n = 0;
  m_Wlv_lep_i->clear();
  m_Wlv_mode->clear();
  m_Wlv_mass->clear();
  m_Wlv_pt->clear();
  m_Wlv_eta->clear();
  m_Wlv_delR->clear();
  m_Wlv_pt_nu->clear();
  m_Wlv_eta_nu->clear();
  // m_Wlv_phi_nu->clear();

  m_top_lvec->clear();
  m_top_n = 0;
  m_top_lvb_n = 0;
  m_top_jjb_n = 0;
  m_top_mode->clear();
  m_top_W_i->clear();
  m_top_b_i->clear();
  m_top_mass->clear();
  m_top_pt->clear();
  m_top_eta->clear();
  // m_top_phi->clear();
  m_top_delR->clear();
  m_top_b_pt->clear();
  m_top_b_eta->clear();
  // m_top_b_phi->clear();
  m_top_b_WT->clear();

  m_ttbar_n = 0;
  // m_ttbar_mode->clear();
  m_ttbar_mass->clear();
  m_ttbar_pt->clear();
  m_ttbar_eta->clear();
  // m_ttbar_phi->clear();
  m_ttbar_delR->clear();
  m_ttbar_lvb_i->clear();
  m_ttbar_jjb_i->clear();

  if (fDebug) cout <<"End of ttbarSel::InitEvent" << endl;
}

Bool_t ttbarSel::Process(Long64_t entry)
{
   // The Process() function is called for each entry in the tree (or possibly
   // keyed object in the case of PROOF) to be processed. The entry argument
   // specifies which entry in the currently loaded tree is to be processed.
   // It can be passed to either ttbarSel::GetEntry() or TBranch::GetEntry()
   // to read either all or the required parts of the data. When processing
   // keyed objects with PROOF, the object is already loaded and is available
   // via the fObject pointer.
   //
   // This function should contain the "body" of the analysis. It can contain
   // simple or elaborate selection criteria, run algorithms on the data
   // of the event and typically fill histograms.
   //
   // The processing can be stopped by calling Abort().
   //
   // Use fStatus to set the return value of TTree::Process().
   //
   // The return value is currently not used.

  if (fDebug) cout << "Come to ttbarSel::Process" << endl;

  b_el_n->GetEntry(entry);
  h1_nEl->Fill(el_n);

  b_mu_n->GetEntry(entry);
  h1_nMu->Fill(mu_n);

  b_jet_n->GetEntry(entry);
  h1_nJet->Fill(jet_n);

  if(entry%fPrintEveryNevts==0)  cout << "Event #" << entry
    << ", el_n=" << el_n
    << "; jet_n=" << jet_n << endl;

  InitEvent();

  if (jet_n>1 || el_n+mu_n>0) {
    GetEntry(entry);

    Recon_Wjj();
    h1_nWjj->Fill(m_Wjj_n);

    Recon_Wlv_allLep();
    h1_nWlv->Fill(m_Wlv_n);

    Recon_ttbar();
    h1_nttbar->Fill(m_ttbar_n);
  }

  if (fDebug) cout << "In ttbarSel::Proess, m_ttbar_n = " << m_ttbar_n << endl;

  if (m_ttbar_n>0) {
     if (fselTree) fselTree->Fill();
     newTree->Fill();
  }

  if (fDebug) cout << "End of ttbarSel::Proess" << endl;
  return kTRUE;
}

void ttbarSel::SlaveTerminate()
{
   // The SlaveTerminate() function is called after all entries or objects
   // have been processed. When running with PROOF SlaveTerminate() is called
   // on each slave server.

  cout << "Come to ttbarSel::SlaveTerminate" << endl;

  skimcount += newTree->GetEntriesFast();

   if (fFile) {
     TDirectory *savedir = gDirectory;
     fFile->cd();
     fFile->Write("",TObject::kOverwrite);
     // savedir->cd();
     fProofFile->Print();
     fOutput->Add(fProofFile);

     // fselTree->SetDirectory(0);
     gDirectory = savedir;
     fFile->Close();
   }

   if (fTimer) {
      fTimer->Stop(); fTimer->Print();
      fTimer->Delete();
      fTimer = 0;
   }
}

void ttbarSel::Terminate()
{
   // The Terminate() function is the last function to be called during
   // a query. It always runs on the client, it can be used to present
   // the results graphically or save the results to file.

  cout << "Come to ttbarSel::Terminate" << endl;

  cout<<"TOTALSIZE="<<ftotsize<<endl;
  cout<<"ZIPSIZE="  <<fzipsize<<endl;
  cout<<"EVENTSREAD="<<skimcount<<endl;

  selTree->PrintCacheStats();

  if (ps) {
    //cout << "Almost Finished with " << ps << endl;
    //ps->Finish();
    cout << "Time to print" << endl;
    ps->Print();
    cout << "ROOTBYTESREAD = " << ps->GetBytesRead() << endl;
    cout << "DISKTIME      = " << ps->GetDiskTime() << endl;
    //cout << "BRANCHESREAD       = " << ps->GetNleaves() << endl;
    cout << "CACHESIZE     = " << ps->GetTreeCacheSize() << endl;
    cout << "ROOTREADS     = " << ps->GetReadCalls() << endl;
    cout << "unzip time    = " << ps->GetUnzipTime() << endl;
    cout << "Time to delete" << endl;
    delete ps;
    ps = 0;
  }
  else {cout << "TTreePerf not made" << endl;}

/*
  TIter next(fOutput);
  TObject *obj;

  while (obj=next()) {
    cout << "obj->Class=" << obj->IsA()->GetName() << endl;
    obj->Write();
  }
*/

}

//___________________________________________________________________
Bool_t ttbarSel::Recon_Wjj()
{
  // select 2 jets from container
  // check that their mass is consisten with the W mass
  // create the W->jj composite
  // fill m_jj histogram
  // insert W candidate into the W container

  if (fDebug) cout << "Come to ttbarSel::Recon_Wjj" << endl;

  Bool_t ret = kFALSE;

  if (jet_n<1) return ret;

  TLorentzVector lvec_W, lvec_j1, lvec_j2;
  Double_t mass_W, pt_W, eta_W, delR_W;
  Double_t pt_j1, pt_j2, eta_j1, eta_j2, wt_j1, wt_j2;

  for (Int_t j1=0; j1<jet_n; j1++) {
    if (jet_E->at(j1)<cJet_MinE) continue;
    lvec_j1.SetPtEtaPhiE( jet_pt->at(j1), jet_eta->at(j1), 
                          jet_phi->at(j1), jet_E->at(j1) );

    for (Int_t j2=j1+1; j2<jet_n; j2++) {
      if (jet_E->at(j2)<cJet_MinE) continue;
      lvec_j2.SetPtEtaPhiE( jet_pt->at(j2), jet_eta->at(j2), 
                            jet_phi->at(j2), jet_E->at(j2) );
      lvec_W = lvec_j1 + lvec_j2;

      mass_W = lvec_W.M();
      pt_W   = lvec_W.Pt();
      eta_W  = lvec_W.Eta();
      delR_W = lvec_j1.DeltaR(lvec_j2);

      pt_j1  = lvec_j1.Pt();
      pt_j2  = lvec_j2.Pt();
      eta_j1 = lvec_j1.Eta();
      eta_j2 = lvec_j2.Eta();
      wt_j1  = my_jet_flavor_weight->at(j1);
      wt_j2  = my_jet_flavor_weight->at(j2);

      if (fDebug) cout << "mass_W = " << mass_W << ", delR_W = " << delR_W << endl;

      h1_m_Wjj->Fill(mass_W/GeV);

      if (fabs(mass_W-WMass)<cW_MassWin) {
        m_Wjj_n++;
        // m_Wjj_mode->push_back(2);  // mode=2 for W->jj
        m_Wjj_mass->push_back(mass_W);
        m_Wjj_pt->push_back(pt_W);
        m_Wjj_eta->push_back(eta_W);
        m_Wjj_delR->push_back(delR_W);

        m_Wjj_lvec->push_back(lvec_W);
        m_Wjj_j1_i->push_back(j1);
        m_Wjj_j2_i->push_back(j2);
        m_Wjj_j1_pt->push_back(pt_j1);
        m_Wjj_j2_pt->push_back(pt_j2);
        m_Wjj_j1_eta->push_back(eta_j1);
        m_Wjj_j2_eta->push_back(eta_j2);
        m_Wjj_j1_wt->push_back(wt_j1);
        m_Wjj_j2_wt->push_back(wt_j2);

      }

    }
  }
  
  ret = kTRUE;
  return ret;

}

//___________________________________________________________________
Bool_t ttbarSel::Recon_Wlv_allLep()
{
  if (fDebug) cout << "Come to ttbarSel::Recon_Wlv_allLep" << endl;

  Bool_t ret = kFALSE;

  if (el_n + mu_n < 1) return ret;

  // W->enu
  if (fDebug) cout << "Going to reconstruct W->enu" << endl;
  Recon_Wlv_oneTypeLep(10, cEl_MinE, el_n, el_E, el_pt, el_eta, el_phi);

  // W->munu
  if (fDebug) cout << "Going to reconstruct W->munu" << endl;
  Recon_Wlv_oneTypeLep(20, cEl_MinE, mu_n, mu_E, mu_pt, mu_eta, mu_phi);

  ret = kTRUE;

  return ret;
}


//___________________________________________________________________
Bool_t ttbarSel::Recon_Wlv_oneTypeLep(Int_t mode, Float_t &lep_MinE,
       Int_t &lep_n,
       vector<float>* &lep_E, vector<float>* &lep_pt,
       vector<float>* &lep_eta, vector<float>* &lep_phi)
{
  if (fDebug) cout << "Come to ttbarSel::Recon_Wlv_oneTypeLep" << endl;

  Bool_t ret = kFALSE;

  vector<TLorentzVector*> pvec_lvec_nu;
  TLorentzVector *p_lvec_nu;
  TLorentzVector lvec_lep, lvec_W, lvec_nu;
  Double_t mass_W, pt_W, eta_W, delR_W;
  Double_t pt_nu, eta_nu, phi_nu;

  Double_t pxMiss = MET_RefFinal_em_etx;
  Double_t pyMiss = MET_RefFinal_em_ety;

  // W -> lep nu
  for (Int_t lep_i=0; lep_i<lep_n; lep_i++) {
    if (lep_E->at(lep_i)<lep_MinE) continue;
    lvec_lep.SetPtEtaPhiE( lep_pt->at(lep_i),  lep_eta->at(lep_i), 
                           lep_phi->at(lep_i), lep_E->at(lep_i) );
    if ( BuildNu_fromWMass(lvec_lep,pxMiss,pyMiss,pvec_lvec_nu) ) {
      Int_t nu_n = pvec_lvec_nu.size();
      for (Int_t nu_i=0; nu_i<nu_n; nu_i++) {
        p_lvec_nu = pvec_lvec_nu.at(nu_i);
        lvec_W = lvec_lep + *p_lvec_nu;
        mass_W = lvec_W.M();
        pt_W   = lvec_W.Pt();
        eta_W  = lvec_W.Eta();
        if (lvec_lep.Pt()>0 && lvec_nu.Pt()>0)
           delR_W = lvec_lep.DeltaR(*p_lvec_nu);
        else
           delR_W = -99.;

        if (fDebug) cout << "mass_W(lnu) = " << mass_W << ", delR_W = " << delR_W << endl;

        pt_nu = p_lvec_nu->Pt();
        if (pt_nu>0) {
           eta_nu = p_lvec_nu->Eta();
           phi_nu = p_lvec_nu->Phi();
        } else {
           eta_nu = -99.;
           phi_nu = -99.;
        }

        if (fabs(mass_W-WMass)<cW_MassWin) {
          m_Wlv_n++;
          m_Wlv_mode->push_back(mode);
          m_Wlv_mass->push_back(mass_W);
          m_Wlv_pt->push_back(pt_W);
          m_Wlv_eta->push_back(eta_W);
          m_Wlv_delR->push_back(delR_W);
          m_Wlv_pt_nu->push_back(pt_nu);
          m_Wlv_eta_nu->push_back(eta_nu);
          // m_Wlv_phi_nu->push_back(phi_nu);

          m_Wlv_lvec->push_back(lvec_W);
          m_Wlv_lep_i->push_back(lep_i);
        }

      }
    }
  }

  ret = kTRUE;

  return ret;
}


//___________________________________________________________________
Bool_t ttbarSel::Recon_top_lvb()
{
  // reconstruct top->Wb->lvb using existing W->lv
  //              ***********

  if (fDebug) cout << "Come to ttbarSel::Recon_top_lvb" << endl;

  Bool_t ret = kFALSE;

  if (m_Wlv_n<1 || jet_n<1) return ret;

  TLorentzVector lvec_top, lvec_W, lvec_b;
  Double_t mass_top, pt_top, eta_top, delR_top;
  // Double_t phi_top;
  Double_t b_wt;

  for (Int_t j=0; j<jet_n; j++) {
    if (jet_E->at(j) < cJet_MinE) continue;
    b_wt = my_jet_flavor_weight->at(j);
    if (b_wt < cJet_bWeightMin) continue;

    lvec_b.SetPtEtaPhiE( jet_pt->at(j), jet_eta->at(j), 
                         jet_phi->at(j), jet_E->at(j) );

    for (Int_t i_W=0; i_W<m_Wlv_n; i_W++) {
      lvec_W = m_Wlv_lvec->at(i_W);

      lvec_top = lvec_b + lvec_W;

      mass_top = lvec_top.M();
      h1_m_lvb->Fill(mass_top/GeV);

      if (fabs(mass_top-TopMass)<cTop_MassWin) {
        pt_top  = lvec_top.Pt();
        eta_top = lvec_top.Eta();
        // phi_top = lvec_top.Phi();
        delR_top = lvec_W.DeltaR(lvec_b);

        m_top_n++;
        m_top_lvb_n++;
        m_top_lvec->push_back(lvec_top);
        m_top_mode->push_back(1);  // 1 for lvb
        m_top_W_i->push_back(i_W);
        m_top_b_i->push_back(j);
        m_top_mass->push_back(mass_top);
        m_top_pt->push_back(pt_top);
        m_top_eta->push_back(eta_top);
        // m_top_phi->push_back(phi_top);
        m_top_delR->push_back(delR_top);
        m_top_b_pt->push_back(jet_pt->at(j));
        m_top_b_eta->push_back(jet_eta->at(j));
        m_top_b_WT->push_back(b_wt);

      }

    }
  }

  ret = kTRUE;

  return ret;
}

//___________________________________________________________________
Bool_t ttbarSel::Recon_top_jjb()
{
  // reconstruct top->Wb->jjb using existing W->jj
  //             ************
  //

  if (fDebug) cout << "Come to ttbarSel::Recon_top_jjb" << endl;

  Bool_t ret = kFALSE;

  if (m_Wjj_n<1 || jet_n<3) return ret;

  TLorentzVector lvec_top, lvec_W, lvec_b;
  Double_t mass_top, pt_top, eta_top, delR_top;
  // Double_t phi_top;
  Double_t b_wt;
  Int_t j1_W, j2_W;

  if (fDebug) cout << "jet_n = " << jet_n << endl;

  for (Int_t j=0; j<jet_n; j++) {
    if (jet_E->at(j)<cJet_MinE) continue;
    b_wt = my_jet_flavor_weight->at(j);
    if (fDebug) cout << "j = " << j << ", cJet_bWeightMin = " << cJet_bWeightMin
                     << ", b_wt=" << b_wt << endl;
    if (b_wt < cJet_bWeightMin) continue;

    lvec_b.SetPtEtaPhiE( jet_pt->at(j), jet_eta->at(j), 
                         jet_phi->at(j), jet_E->at(j) );

    for (Int_t i_W=0; i_W<m_Wjj_n; i_W++) {
      lvec_W = m_Wjj_lvec->at(i_W);
      j1_W = m_Wjj_j1_i->at(i_W);
      j2_W = m_Wjj_j2_i->at(i_W);
      if (j==j1_W || j==j2_W) continue;

      lvec_top = lvec_b + lvec_W;

      mass_top = lvec_top.M();
      h1_m_jjb->Fill(mass_top/GeV);

      if (fabs(mass_top-TopMass)<cTop_MassWin) {
        pt_top  = lvec_top.Pt();
        eta_top = lvec_top.Eta();
        // phi_top = lvec_top.Phi();
        delR_top = lvec_W.DeltaR(lvec_b);

        m_top_n++;
        m_top_jjb_n++;
        m_top_lvec->push_back(lvec_top);
        m_top_mode->push_back(2);  // 2 for jjb
        m_top_W_i->push_back(i_W);
        m_top_b_i->push_back(j);
        m_top_mass->push_back(mass_top);
        m_top_pt->push_back(pt_top);
        m_top_eta->push_back(eta_top);
        // m_top_phi->push_back(phi_top);
        m_top_delR->push_back(delR_top);
        m_top_b_pt->push_back(jet_pt->at(j));
        m_top_b_eta->push_back(jet_eta->at(j));
        m_top_b_WT->push_back(b_wt);

      }

    }
  }

  if (fDebug) cout << "End of ttbarSel::Recon_top_jjb" << endl;

  ret = kTRUE;

  return ret;
}


//___________________________________________________________________
Bool_t ttbarSel::Recon_ttbar()
{
  // reconstruct ttbar->Wb, Wb->jjb, lvb
  // where W's have already been reconstructed
  // save information for those ttbar passing top-quark mass cut

  if (fDebug) cout << "Come to ttbarSel::Recon_ttbar" << endl;

  Bool_t ret = kFALSE;

  if (fDebug) cout << "m_Wjj_n = " << m_Wjj_n << ", m_Wlv_n = " << m_Wlv_n
                   << ", jet_n=" << jet_n << endl;

  if (m_Wjj_n<0 || m_Wlv_n<0 || jet_n<4) return ret;

  // First reconstruct top->lvb and jjb
  // ==================================

  // t->Wb, W->lv
  Recon_top_lvb();
  h1_nTop_lvb->Fill(m_top_lvb_n);

  // t->Wb, W->jj
  Recon_top_jjb();
  h1_nTop_jjb->Fill(m_top_jjb_n);

  // Now reconstruct ttbar->lvb + jjb
  // ================================
  if (fDebug) cout << "m_top_lvb_n = " << m_top_lvb_n << ", m_top_jjb_n = " << m_top_jjb_n << endl;
  if (m_top_lvb_n < 1 || m_top_jjb_n < 1) return ret;
  if (fDebug) cout << "Now reconstruct ttbar -> lvb + jjb" << endl;

  TLorentzVector lvec_ttbar, lvec_top_lvb, lvec_top_jjb;
  Double_t mass_ttbar, pt_ttbar, eta_ttbar, delR_ttbar;
  // Double_t phi_ttbar;
  Int_t idx_b_lvb, b_idx_jjb, idx_W_jjb, j1_jjb, j2_jjb;

  for (Int_t t1=0; t1<m_top_n; t1++) {
    if (m_top_mode->at(t1) != 1) continue;  // lvb only
    lvec_top_lvb = m_top_lvec->at(t1);
    idx_b_lvb = m_top_b_i->at(t1);

    for (Int_t t2=0; t2<m_top_n; t2++) {
      if (m_top_mode->at(t2) != 2) continue;  // jjb only
      b_idx_jjb = m_top_b_i->at(t2);
      idx_W_jjb = m_top_W_i->at(t2);
      j1_jjb    = m_Wjj_j1_i->at(idx_W_jjb);
      j2_jjb    = m_Wjj_j2_i->at(idx_W_jjb);

      if (fDebug) cout << "checking overlap in ttbar"
                       << "\n  bJet(lvb)=" << idx_b_lvb
                       << "; j1(jjb)=" << j1_jjb << ", j2(jjb)=" << j2_jjb
                       << ", bJet(jjb)=" << b_idx_jjb<< endl;

      if (idx_b_lvb==b_idx_jjb || idx_b_lvb==j1_jjb || idx_b_lvb==j2_jjb) continue;

      lvec_top_jjb = m_top_lvec->at(t2);

      lvec_ttbar = lvec_top_lvb + lvec_top_jjb;

      mass_ttbar = lvec_ttbar.M();
      pt_ttbar = lvec_ttbar.Pt();
      eta_ttbar = lvec_ttbar.Eta();
      // phi_ttbar = lvec_ttbar.Phi();
      delR_ttbar = lvec_top_lvb.DeltaR(lvec_top_jjb);

      h1_m_ttbar->Fill(mass_ttbar/GeV);
      
      m_ttbar_n++;
      m_ttbar_mass->push_back(mass_ttbar);
      m_ttbar_pt->push_back(pt_ttbar);
      m_ttbar_eta->push_back(eta_ttbar);
      m_ttbar_delR->push_back(delR_ttbar);
      m_ttbar_lvb_i->push_back(t1);
      m_ttbar_jjb_i->push_back(t2);

    }
  }
  
  ret = kTRUE;
  return ret;
}


//___________________________________________________________________
Bool_t ttbarSel::BuildNu_fromWMass(const TLorentzVector& lvec_lep,
                 const double pxMiss, const double pyMiss,
                 vector<TLorentzVector*>& pvec_lvec_nu)
{
  static TLorentzVector lvec_nu1, lvec_nu2;

  if (fDebug) cout << "Come to ttbarSel::BuildNu_fromWMass" << endl;

  // if (lvec_lep == NULL || pvec_lvec_nu == NULL) return kFALSE;

  // clear output
  pvec_lvec_nu.clear();

  // solve the quadratic equation
  
  double ptMiss = sqrt (pxMiss*pxMiss + pyMiss*pyMiss);

  double alpha = pow(WMass,2) + pow((pxMiss+lvec_lep.Px()),2) + 
                 pow((pyMiss+lvec_lep.Py()),2) - pow(lvec_lep.E(),2);
  
  double beta = 0.5 * ( alpha-pow(ptMiss,2)+pow(lvec_lep.Pz(),2) );

  double inv_par2 = 1. / ( pow(lvec_lep.E(),2)-pow(lvec_lep.Pz(),2) );
  
  double gamma = -( beta*beta - ( pow(lvec_lep.E(),2)*pow(ptMiss,2) ) )
                 * inv_par2;

  double lambda = 2*beta*lvec_lep.Pz() * inv_par2;

  double delta = pow(lambda,2)-4*gamma;

  // if no solution
  if ( delta < 0 ){
    return kFALSE;
  }

  delta = sqrt(delta);
  
  // instantiate Neutrino

  double pz = (lambda-delta)/2.0;
  double e  = sqrt(pxMiss*pxMiss+pyMiss*pyMiss+pz*pz);

  lvec_nu1.SetXYZT(pxMiss,pyMiss,pz,e);
  pvec_lvec_nu.push_back(&lvec_nu1);

  if (fDebug) cout << "nu1 in W->lnu, E = " << lvec_nu1.E() << ", pt = " << lvec_nu1.Pt() << endl;

  if ( delta == 0 ) return true;

  pz = (lambda+delta)/2.0;
  e  = sqrt(pxMiss*pxMiss+pyMiss*pyMiss+pz*pz);

  lvec_nu2.SetXYZT(pxMiss,pyMiss,pz,e);
  pvec_lvec_nu.push_back(&lvec_nu2);

  if (fDebug) cout << "nu2 in W->lnu, E = " << lvec_nu2.E() << ", pt = " << lvec_nu2.Pt() << endl;

  return kTRUE;
}

//___________________________________________________________________
Bool_t ExtractParVal(TString &str_line, Float_t &parVal)
{
  static TRegexp re("[:=]");
  TString str_par;

  Bool_t ret = kFALSE;

  Int_t idx_delim = str_line.Index(re);
  str_par = str_line(idx_delim+1, str_line.Length());
  if (str_par.IsFloat()) {
     parVal = str_par.Atof();
     ret = kTRUE;
  } else {
     cout << "Warning! Unknown value " << str_par.Data() << " in Jet_MinE" << endl;
     ret = kFALSE;
  }

  return ret;
}
