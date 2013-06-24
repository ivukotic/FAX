#include "occi.h"
// #include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "TApplication.h"
#include "TH1F.h"
#include "TCanvas.h"
using namespace std;
using namespace oracle::occi;

const string dateFormat("DD-MON-YY HH12:MI:SS");

class coll{
    public:
        coll(int numb, string nam, string t ){number=numb;name=nam;typ=t;shown=false;}
        int number;
        string name;
        string typ;
        bool shown;
};

class menu{
public:
    
    menu(){
        m_print=false;
        c1= new TCanvas("test1","tes1",0,0,800,600);
        string q="select COLUMN_ID, COLUMN_NAME, DATA_TYPE from USER_TAB_COLUMNS where table_name='GRIDJOB'";
        cout<<"intialize..."<<endl;
        getDS(q);
        cout<<"got columns..."<<endl;
        while (pSet->next()){
            if (pSet->getInt(1)==1) continue;
            allColumns.push_back(coll(pSet->getInt(1),pSet->getString(2),pSet->getString(3)));
        }
        menuLoop();
    }
        
private:
    
    void menuLoop(){
        while (true){
            cout<<"\f--------------------------------\nVariables to show:"<<endl;
            for (i=allColumns.begin();i!=allColumns.end();i++){
                if (i->shown==1) cout<<"\t"<<i->name<<endl;
            }
            if (allCuts.size()){
                cout<<"---------------------------"<<endl;
                cout<<"Cuts applied: "<<endl;
                for (c=allCuts.begin();c!=allCuts.end();c++) cout<<"\t"<<*c<<endl;
            }
            cout<<"================================================================================================="<<endl;
            cout<<"Your choice: 0-Exit 1-AddVariable 2-RemoveVariable 3-AddCut 4-RemoveCut 5-Plot 6-Values 7-TogglePrintToPDF" <<endl;
            int choice;
            cin>>choice;
            if (choice==0) exit(0);
            if (choice==1) AddVariable();
            if (choice==2) RemoveVariable();
            if (choice==3) AddCut();
            if (choice==4) RemoveCut();
            if (choice==5) draw();
            if (choice==6) values();
            if (choice==7) {
                if (m_print==false) cout<<"printing to pdf file is ON."<<endl; else cout<<"printing to pdf file is OFF."<<endl;
                m_print=(!m_print);
            }
        }
    }
    
    string getQuery(){
        string r="SELECT ";
        for (i=allColumns.begin();i!=allColumns.end();i++){
            if (i->shown==1) r+= i->name+", ";
        }
        r.resize(r.size()-2);
        r+=" from gridjob ";
        if (allCuts.size()) r+=" where ";
        for (c=allCuts.begin();c!=allCuts.end();c++){
            r += *c;
            r += " AND ";
        }
        if (allCuts.size()) r.resize(r.size()-4);
        return r;
    }
    
    vector<coll> getColumns(){
        vector<coll> r;
        for (i=allColumns.begin();i!=allColumns.end();i++){
            if (i->shown==1) r.push_back(*i);
        }
        return r;
    }
    
    void AddVariable(){
        cout<<"\fNo.   Variable"<<endl;
        int co=0;
        int choice;
        for (i=allColumns.begin();i!=allColumns.end();i++){
            if (i->shown==0) cout<<co<<"\t"<<i->name<<endl; //"\t"<<i->typ<<endl;
            co++;
        }
        cout<<"-------------------------\n"<<"Please select column to add: ";
        cin>>choice;
        if (choice>-1 && choice<co) allColumns[choice].shown=1;
        return;
    }
    
    void RemoveVariable(){
        cout<<"\fNo.   Variable"<<endl;
        int co=0;
        int choice;
        for (i=allColumns.begin();i!=allColumns.end();i++){
            if (i->shown==1) cout<<co<<"\t"<<i->name<<endl;
            co++;
        }
        cout<<"-------------------------\n"<<"Please select column to remove: ";
        cin>>choice;
        if (choice>-1 && choice<co) allColumns[choice].shown=0;
        return;
    }
    
    void AddCut(){
        string cut;
        getline(cin, cut); // this jsut to avoid problem with buffer already filled with value 3
        cout<<"this is a freeform SQL cut. There is no filtering of this string. Please use responsably.\nYour cut: "<<endl;
        getline(cin, cut);
        allCuts.push_back(cut);
        return;
    }
    
    void RemoveCut(){
        cout<<"\fNo.    Cut"<<endl;
        int co=0;
        int choice;
        for (c=allCuts.begin();c!=allCuts.end();c++) {cout<<co<<"\t"<<(*c)<<endl;co++;}
        cout<<"------------------------\nPlease select cut to remove: ";
        cin>>choice;
        allCuts.erase(allCuts.begin()+choice);
        return;
    }
    
    void values(){
        cout<<"\fNo.   Variable"<<endl;
        int co=0;
        int choice;
        for (i=allColumns.begin();i!=allColumns.end();i++){
            if (!i->typ.compare("VARCHAR2")) cout<<co<<"\t"<<i->name<<endl; //"\t"<<i->typ<<endl;
            co++;
        }
        cout<<"-------------------------\nPlease select column to print possible values: ";
        cin>>choice;
        if (choice>-1 && choice<co) 
        if (!(allColumns[choice].typ).compare("VARCHAR2")) {
            getDS("SELECT UNIQUE "+allColumns[choice].name+ " from GRIDJOB ORDER BY "+allColumns[choice].name+ " DESC");
            while (pSet->next()) cout<<pSet->getString(1)<<endl;
            }
        return;
    }
    
    void getDS(string query){
        cout<<query<<endl;
        Environment* m_env;
        m_env = Environment::createEnvironment();
        const char* dbname = "intr1-v.cern.ch:10121/intr.cern.ch";
        const char* user = "ATLAS_ATHENAIOPERF";
        const char* pass = "Sophia2010";
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
        // m_conn->terminateStatement(m_stmt);
        // m_env->terminateConnection(m_conn);
    }
    
    void draw(){
        getDS(getQuery());
        vector<coll> colls = getColumns();
        
        TH1F g[30];
        int co=0;
        for (i=colls.begin();i!=colls.end();i++){
            g[co]=TH1F(i->name.c_str(),i->name.c_str(),100,0,5000);
            co++;
        }
        while (pSet->next()){
            int co=0;
            for (i=colls.begin();i!=colls.end();i++){
                if (i->typ.compare("NUMBER")==0) g[co].Fill(pSet->getFloat(co+1));
                // if (i->typ.compare("VARCHAR2")==0) g[co]->Fill(pSet->getString(co+1));
                co++;
            }
            //dateAsString=created.toText(dateFormat);
        }

        co=0;
        for (i=colls.begin();i!=colls.end();i++){
            if (!co) g[co].Draw(); else g[co].Draw("same");
            co++;
        }
        c1->Update();
        if (m_print==true){
            c1->Print("siteInfo.pdf");
        }
    }
    
    vector<coll> allColumns;
    vector<coll>::iterator i;
    vector<string> allCuts;
    vector<string>::iterator c;
    Connection* m_conn;
    Statement* m_stmt;
    Statement::Status status;
    ResultSet * pSet;
    TCanvas *c1;
    bool m_print;
};



int main(int argc, char *argv[]){
    TApplication *theApp = new TApplication("App", &argc, argv);
    menu m;
return 0;
}
