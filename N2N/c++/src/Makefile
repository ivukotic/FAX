all: XrdOucName2NameLFC.so xrd-lfc-test

LFC_LIB=/usr/lib64
LFC_INCLUDE=/usr/include/lfc

XRD_INCLUDE=/usr/include/xrootd
FLAGS=-D_REENTRANT -D_THREAD_SAFE -Wno-deprecated

SOURCES=XrdOucName2NameLFC.cc rucioN2N.cc rucioGetSitePrefix.cc
OBJECTS=XrdOucName2NameLFC.o  rucioN2N.o  rucioGetSitePrefix.o
HEADERS=XrdMsgStream.hh String.hh rucioN2N.hh

DEBUG=-g

XrdOucName2NameLFC.so: $(OBJECTS) Makefile
	g++ ${DEBUG} -shared -fPIC -Wl,-R${LFC_LIB} -L ${LFC_LIB} -o $@ $(OBJECTS) -llfc -ldl -ljson -lcurl

XrdOucName2NameLFC.o: XrdOucName2NameLFC.cc ${HEADERS} Makefile
	g++ ${DEBUG} ${FLAGS} -fPIC -I ${XRD_INCLUDE} -I ${LFC_INCLUDE} -c -o $@ $<

rucioN2N.o: rucioN2N.cc ${HEADERS} Makefile
	g++ ${DEBUG} ${FLAGS} -fPIC -I ${XRD_INCLUDE} -I ${LFC_INCLUDE} -c -o $@ $<

rucioGetSitePrefix.o: rucioGetSitePrefix.cc ${HEADERS} Makefile
	g++ ${DEBUG} ${FLAGS} -fPIC -I ${XRD_INCLUDE} -I ${LFC_INCLUDE} -c -o $@ $<

xrd-lfc-test: xrd-lfc-test.o Makefile
	g++ ${DEBUG} ${FLAGS} -fPIC -Wl,-R${LFC_LIB} -L ${LFC_LIB} -o $@ rucioN2N.o rucioGetSitePrefix.o $< -llfc -ldl -ljson -lcurl -lXrdClient

xrd-lfc-test.o: XrdOucName2NameLFC.cc ${HEADERS} Makefile
	g++ ${DEBUG} -DSTANDALONE ${FLAGS} -fPIC -I ${XRD_INCLUDE} -I ${LFC_INCLUDE} -c -o $@ $< 

clean:
	rm -vf xrd-lfc-test{,.o} XrdOucName2NameLFC.{o,so} rucioN2N.o rucioGetSitePrefix.o


