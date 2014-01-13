void listFileContent(){
	TFile *f = TFile::Open("root://fax.mwt2.org:1094//atlas/rucio/user/ivukotic:group.test.hc.NTUP_SMWZ.root");
	if (f)	f->ls();
}

