void listFileContent(){
	TFile *f = TFile::Open("root://fax.mwt2.org:1094//atlas/dq2/user/ilijav/HCtest/user.ilijav.HCtest.1/group.test.hc.NTUP_SMWZ.root");
	if (f)	f->ls();
}

