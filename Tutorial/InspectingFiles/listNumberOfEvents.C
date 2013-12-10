#include <iostream>
#include <fstream>
#include <string>
using namespace std;

void listNumberOfEvents(){
	int total, filesOpened;
	ifstream infile;
	infile.open ("../MWT2_files.txt");
	while (!infile.eof()){
		string sLine = "";
		getline(infile, sLine);
		cout << sLine << endl;
		TFile *f = TFile::Open(sLine.c_str());
		if (f) {
			TTree *t=(TTree*) f->Get("physics");
			int entries=t->GetEntries();
			cout<<" "<<entries<<endl;
			total += entries;
		}

                filesOpened++;
                if (filesOpened>10) break;
	}
	cout<<"Total: "<<total<<endl;
	infile.close();

}

