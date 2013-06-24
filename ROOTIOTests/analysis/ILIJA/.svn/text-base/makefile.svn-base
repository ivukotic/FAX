include $(ROOTSYS)/etc/Makefile.arch
-include ../MyConfig.mk


LDFLAGS += -Wall
LIBOS += -L/afs/cern.ch/project/oracle/@sys/10205/lib/ -locci -lclntsh
INCL += -I/afs/cern.ch/project/oracle/@sys/10205/rdbms/public/ -I$(ROOTSYS)/include

SITES       = site.cxx
SITE        = site$(ExeSuf)
eToRS		= exportToROOT.cxx
eToR		= exportToROOT$(ExeSuf)

PROGRAMS      = $(SITE) $(eToR)

.SUFFIXES: .$(SrcSuf) .$(ObjSuf) .$(DllSuf)

all:            $(PROGRAMS)


$(SITE):      site.o
	$(LD) $(LDFLAGS) $^ $(LIBOS) $(LIBS)  $(OutPutOpt) $@ $(MT_EXE)
	@echo "$@ done"

site.o:	$(SITES)
	$(LD)  $^ $(INCL) -c


$(eToR):      exportToROOT.o
	$(LD) $(LDFLAGS) $^ $(LIBOS) $(LIBS)  $(OutPutOpt) $@ $(MT_EXE)
	@echo "$@ done"

exportToROOT.o:	$(eToRS)
	$(LD)  $^ $(INCL) -c

clean:
	rm *.o 
	rm site
	rm exportToROOT
