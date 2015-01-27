#ifndef TIOPERFSTATS_H
#define TIOPERFSTATS_H


#include "TVirtualPerfStats.h"


class TIOPerfStat : public TVirtualPerfStats{
public:

    TIOPerfStat(TVirtualPerfStats* current_element) :
        next(current_element),
        cumulated_time(0),
        cumulated_size(0),
        io_calls(0){

    }

    virtual ~TIOPerfStat(){
        if(gPerfStats == this){
            gPerfStats = next;
        }
    }

    virtual void SimpleEvent(EEventType type){
        if(next)
            next->SimpleEvent(type);

    }

     virtual void PacketEvent(const char *slave, const char *slavename, const char *filename,
                              Long64_t eventsprocessed, Double_t latency,
                              Double_t proctime, Double_t cputime,
                              Long64_t bytesRead){
        if(next){
            next->PacketEvent(slave, slavename, filename, eventsprocessed, latency, proctime, cputime, bytesRead);
        }

    }

     virtual void FileEvent(const char *slave, const char *slavename, const char *nodename,
                            const char *filename, Bool_t isStart){
        if(next){
            next->FileEvent(slave, slavename, nodename, filename, isStart);
        }
    }

     virtual void FileOpenEvent(TFile *file, const char *filename, Double_t start){
        if(next){
            next->FileOpenEvent(file, filename, start);
        }
    }

     virtual void FileReadEvent(TFile *file, Int_t len, Double_t start){

        Double_t end = TTimeStamp();
        end -= start;
        if(end > 0){
            cumulated_time+= end;
        }
        cumulated_size +=len;
        io_calls +=1;

        if(gDebug > 2){
            Info("TIOPerfStat", " TraceI/O: read %d Bytes in %lf",  len, end);
        }

        if(next){
            next->FileReadEvent(file, len, start);
        }
    }

     virtual void UnzipEvent(TObject *tree, Long64_t pos, Double_t start, Int_t complen, Int_t objlen){
        if(next){
            next->UnzipEvent(tree, pos, start, complen, objlen);
        }
    }

     virtual void RateEvent(Double_t proctime, Double_t deltatime,
                            Long64_t eventsprocessed, Long64_t bytesRead){
        if(next){
            next->RateEvent(proctime, deltatime, eventsprocessed, bytesRead);
        }
    }

     virtual void SetBytesRead(Long64_t num){
        if(next){
            next->SetBytesRead(num);
        }
    }

     virtual Long64_t GetBytesRead() const {
        if(next){
            return next->GetBytesRead();
        }
        return 0;
    }

     virtual void SetNumEvents(Long64_t num){
        if(next){
            next->SetNumEvents(num);
        }
    }

     virtual Long64_t GetNumEvents() const{
        if(next){
            return next->GetNumEvents();
        }
        return 0;
    }


    void PrintStats() const{
        std::cout << "TotalIOTime=" << cumulated_time << std::endl;
        std::cout << "TotalIOSize=" << cumulated_size << std::endl;
        std::cout << "TotialIOCalls=" << io_calls << std::endl;
    }
    
    
    void RegisterTree(TTree* t){
		   next = (TVirtualPerfStats*) t->GetPerfStats();
		   t->SetPerfStats(this);
		   
		   gPerfStats = this;
	}

private:
    TVirtualPerfStats* next;
    Double_t cumulated_time;
    Double_t cumulated_size;
    Long64_t io_calls;
};


#endif // TIOPERFSTATS_H
