all: XrdOucName2NameLFC.so

ifeq ($(strip $(XRD_INC)),)
    XRD_INC=/usr/include/xrootd
endif

ifeq ($(strip $(XRD_LIB)),)
XRD_LIB=/usr/lib64
endif

FLAGS=-D_REENTRANT -D_THREAD_SAFE -Wno-deprecated

SOURCES=XrdOucName2NameLFC.cc rucioN2N.cc rucioGetSitePrefix.cc
OBJECTS=XrdOucName2NameLFC.o  rucioN2N.o  rucioGetSitePrefix.o
HEADERS=XrdMsgStream.hh String.hh rucioN2N.hh

DEBUG=-g

XrdOucName2NameLFC.so: $(OBJECTS) Makefile
	g++ ${DEBUG} -shared -fPIC -o $@ $(OBJECTS) -ldl -ljson -lssl -L $(XRD_LIB) -lXrdPosix

XrdOucName2NameLFC.o: XrdOucName2NameLFC.cc ${HEADERS} Makefile
	g++ ${DEBUG} ${FLAGS} -fPIC -I ${XRD_INC} -c -o $@ $<

rucioN2N.o: rucioN2N.cc ${HEADERS} Makefile
	g++ ${DEBUG} ${FLAGS} -fPIC -I ${XRD_INC} -c -o $@ $<

rucioGetSitePrefix.o: rucioGetSitePrefix.cc ${HEADERS} Makefile
	g++ ${DEBUG} ${FLAGS} -fPIC -I ${XRD_INC} -c -o $@ $<

xrd-lfc-test: xrd-lfc-test.o Makefile
	g++ ${DEBUG} ${FLAGS} -fPIC -Wl -o $@ rucioN2N.o rucioGetSitePrefix.o $< -ldl -ljson -L $(XRD_LIB) -lXrdPosix

xrd-lfc-test.o: XrdOucName2NameLFC.cc ${HEADERS} Makefile
	g++ ${DEBUG} -DSTANDALONE ${FLAGS} -fPIC -I ${XRD_INC} -c -o $@ $< 

clean:
	rm -vf xrd-lfc-test{,.o} XrdOucName2NameLFC.{o,so} rucioN2N.o rucioGetSitePrefix.o


