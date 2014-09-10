void FAX_listFileContent(string fn){
    TFile *f = TFile::Open(fn.c_str());
    if (f){ 
        f->ls();
    } else {
        std::cout<<"File could not be opened."<<std::endl;
        exit(1);
    }
}

