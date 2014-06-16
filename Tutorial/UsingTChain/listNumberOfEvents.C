#include <iostream>
#include <fstream>
#include <string>
using namespace std;

void listNumberOfEvents(){
	int total, filesOpened;
	ifstream infile;
	infile.open ("../MWT2_files.txt");
    TChain ch;
	while (!infile.eof()){
		string sLine = "";
		getline(infile, sLine);
		cout << sLine << endl;
        ch.AddFile(sLine.c_str(),TChain::kBigNumber,"physics");         
        filesOpened++;
        if (filesOpened>10) break;
    }
    infile.close();
    
	int entries=ch.GetEntries();
	cout<<"Entries: "<<entries<<endl;

}

