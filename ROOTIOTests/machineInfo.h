//#include "/usr/include/stdio.h"
//#include "/usr/lib/x86_64-redhat-linux4E/include/stdio.h"
//#include "/usr/include/c++/4.4.4/tr1/stdio.h"

#include "Riostream.h"
#include "TROOT.h"
#include "TFile.h"
#include "TNetFile.h"
#include "TRandom.h"
#include "TTree.h"
#include "TTreeCache.h"
#include "TTreePerfStats.h"
#include "TBranch.h"
#include "TClonesArray.h"
#include "TStopwatch.h"
#include "TTreeCacheUnzip.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

void diskSt(long long * diskStat, string fn){
  
    // first finding base directory for the file
    int pos=fn.find('/',2);
    string dis;
    dis=fn.substr(0, pos);
    cout<<"\nmount point from fn: "<<dis<<endl;
    
    // finding which device has this mount
    string na, nb, nc, nd;
    int i1;    
    ifstream mounts ("/proc/mounts", ios::in);
    if (mounts.is_open()){
        while(!mounts.eof()){
            mounts>>na>>nb>>nc>>nd>>i1>>i1;
            if (nb.compare(dis)==0)  {
                cout<<na<<"\t"<<nb<<"\t";
                if (na.substr(0,5).compare("/dev/")==0)
                    na=na.substr(5);
                cout<<na<<endl;    
                break;
            }
        }
    }
    
	ifstream file ("/proc/diskstats", ios::in);
	if (file.is_open()){
		while(!file.eof()){
			string name="";
			file>>i1>>i1>>name;
            // cout<<name<<"\t";
            for (int c=0;c<11;c++) {
                file>>diskStat[c]; 
                // cout<<diskStat[c]<<"\t";
            }
            // cout<<endl;    
            if (!name.compare(na)) {
                // for (int c=0;c<11;c++) cout<<diskStat[c]<<"\t"; cout<<endl;
                break;
			    };
		} 
	}
	file.close();
}

void getMemoryInfo(){
    ifstream file ("/proc/self/status", ios::in);
    int i;
	if (file.is_open()){
		while(!file.eof()){
			string name="";
			file>>name;
            // cout<<name<<"\t";
            if (!name.compare("VmPeak:")) {file>>i;cout<<"VMEM="<<i<<endl;}
            if (!name.compare("VmRSS:")) {file>>i;cout<<"RSS="<<i<<endl;file.close();return;}	    
		} 
	}
	file.close();
}


void getCPUusage(){
    FILE *pf;
    char data[48];
    pf=popen("uptime |awk -F'average:' '{ print $2}'  | sed s/,//g | awk '{ print $3}' ","r");
    if(!pf){
        fprintf(stderr, "Could not open pipe for output.\n");
        return;
    }
    fgets(data, 48 , pf);
    pclose(pf);
    cout<<"CPUSTATUS="<<data<<endl;
}

void netSt(long long *netStat){
    system("netstat -in > nstat.txt");
    ifstream file("nstat.txt", ios::in);
    int i;netStat[0]=0;netStat[1]=0;
	if (file.is_open()){
		while(!file.eof()){
			string name="";
			file>>name;
            if (!name.compare("eth0")) {
                file>>i>>i>>netStat[0]>>i>>i>>i>>netStat[1]; 
                break;
			    }
		}
	}
	
    system("rm nstat.txt");
	file.close();
}

void machineInfo() {
  //getCPUusage();
  //getMemoryInfo();
}
