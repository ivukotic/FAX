#include "occi.h"
#include <stdlib.h>
#include <iostream>
#include "TApplication.h"
#include "TCanvas.h"
#include "TTree.h"
#include "TFile.h"
#include "TBranch.h"

using namespace std;
using namespace oracle::occi;


const string dateFormat("DD-MON-YY HH12:MI:SS");

class menu{

public:
    
    void init(){
        Environment *env = Environment::createEnvironment(Environment::OBJECT);
        
        string r="SELECT  name from project";
        getDS(r);
        int row=0;
        vector<string> tests;
        while (pSet->next()) {
            string test=pSet->getString(1);
            cout<<"test:"<<row<<"\t"<<test<<endl; row++;
            tests.push_back(pSet->getString(1));
        }
        cout<<"Please select which test you would like exported to a ROOT file: ";
        int sel;    
        cin>>sel;
        JT=tests[sel];
        f=new TFile((JT+".root").c_str(),"recreate");
        t=new TTree(JT.c_str(),JT.c_str());
        cout<<"intialize..."<<endl;

        t->Branch("site", &site );
        t->Branch("datetime",&datetime);
        t->Branch("filesystem", &filesystem );
        t->Branch("filename", &filename );
        t->Branch("cputime", &cputime );
        t->Branch("walltime", &walltime );
        t->Branch("hddtime", &hddtime );
        t->Branch("hddreads", &hddreads );
        t->Branch("vmem", &vmem );
        t->Branch("rss", &rss );
        t->Branch("events", &events );
        t->Branch("zipsize", &zipsize );
        t->Branch("totalsize", &totalsize );
        t->Branch("pandaid", &pandaid );
        t->Branch("cachesize", &cachesize );
        t->Branch("rootreads", &rootreads );
        t->Branch("rootbytesread", &rootbytesread );
        t->Branch("ethernetout", &ethernetout );
        t->Branch("ethernetin", &ethernetin );
        t->Branch("cpuload", &cpuload );
        t->Branch("cputype", &cputype );
        t->Branch("cores", &cores );
        t->Branch("HT", &HT );
        t->Branch("rootversion", &rootversion );
        t->Branch("rootbranch", &rootbranch );

        cout<<"BRANCHES CREATED"<<endl;
        run();
    }

    string JT;

private:

    string site;
    TDatime datetime;
    string filesystem;
    string filename;
    float cputime;
    float walltime;
    unsigned int hddtime;
    unsigned int hddreads;
    unsigned int vmem;
    unsigned int rss;
    unsigned int events;
    unsigned int zipsize;
    unsigned int totalsize;
    unsigned int pandaid;
    unsigned int cachesize;
    unsigned int rootreads;
    unsigned int rootbytesread;
    unsigned int ethernetout;
    unsigned int ethernetin;
    float cpuload;
    string cputype;
    int cores;
    bool HT;
    int rootversion;
    string rootbranch;



    TFile *f;
    TTree *t;

    void run(){
        string r="SELECT  site.name, result.created, site.storage, result.filename, result.cputime, result.walltime, storagemeasurements.time, storagemeasurements.reads, result.vmem, result.rss, root.events, root.zipsize, root.totalsize, result.pandaid, root.ttc, root.reads, root.bytesread, wn.networkout,  wn.networkin,  wn.load, cpu.name, cpu.cores, cpu.ht, rootversion.svnversion, rootversion.name from project, site, result, root, wn, rootversion, storagemeasurements, cpu where project.projectid=result.projectref and site.siteid=result.siteref and root.resultid=result.resultid and wn.resultid=result.resultid and rootversion.versionid=root.versionid and CPU.CPUID=wn.cpuid  and storagemeasurements.resultid=result.resultid and project.name='"+JT+"' order by site.name";
        getDS(r);
        int row=0;
        while (pSet->next()) {
            site=pSet->getString(1);
            if(!(row%100)){
                cout<<"row:"<<row<<"\t"<<site<<endl;
            } 
            row++;
            Date odate = pSet->getDate(2); 
            int y=0;unsigned int m, d, h, mi, s; 
            odate.getDate(y,m,d,h,mi,s);
            datetime.Set(y,m,d,h,mi,s);             
            // cout<<y<<"/"<<m<<"/"<<d<<endl;
            filesystem=pSet->getString(3);
            filename=pSet->getString(4);
            cputime=pSet->getFloat(5);
            walltime=pSet->getFloat(6);
            hddtime=pSet->getFloat(7);
            hddreads=pSet->getUInt(8);
            vmem=pSet->getUInt(9);
            rss=pSet->getUInt(10);
            events=pSet->getUInt(11);
            zipsize=pSet->getUInt(12);
            totalsize=pSet->getUInt(13);
            pandaid=pSet->getUInt(14);
            cachesize=pSet->getUInt(15);
            rootreads=pSet->getUInt(16);
            rootbytesread=pSet->getUInt(17);
            // ethernetout=pSet->getUInt(18);
            // ethernetin=pSet->getUInt(19);
            cpuload=pSet->getFloat(20);
            cputype=pSet->getString(21);
            cores=pSet->getUInt(22);
            HT=pSet->getUInt(23);
            rootversion=pSet->getUInt(24);
            rootbranch=pSet->getString(25);
            
            t->Fill();
        }

        cout<<"TREE FILLED"<<endl;
        t->Write();
        f->Close();
    }



    void getDS(string query){
        // cout<<query<<endl;
        Environment* m_env;
        m_env = Environment::createEnvironment();
        const char* dbname = "intr1-v.cern.ch:10121/intr.cern.ch";
        const char* user = "ATLAS_HCLOUDTEST";
        const char* pass = "HCLOUDTEST_2012_2";
        m_conn = m_env->createConnection(user, pass, dbname);
        m_stmt = m_conn->createStatement(query.c_str());
        status = m_stmt->status();
        if( status != Statement::PREPARED ) printf("The statement is not prepared.\n");
        try{
            status = m_stmt->execute();
            pSet = m_stmt->getResultSet();
        }catch (SQLException &sqlExcp){
            printf("menu SQL Exception\n");
        }
    }
 
    Connection* m_conn;
    Statement* m_stmt;
    Statement::Status status;
    ResultSet * pSet;
};



int main(int argc, char *argv[]){
    TApplication *theApp = new TApplication("App", &argc, argv);
    menu m;
    m.init();
    return 0;
}
