all: XrdOucName2NameLFC.so xrd-lfc-test

LFC_LIB=/usr/lib64
LFC_INCLUDE=/usr/include/lfc

XRD_INCLUDE=/usr/include/xrootd
FLAGS=-D_REENTRANT -D_THREAD_SAFE -Wno-deprecated

SOURCE=XrdOucName2NameLFC.cc
HEADERS=XrdMsgStream.hh String.hh

DEBUG=-g

XrdOucName2NameLFC.so: XrdOucName2NameLFC.o Makefile
	g++ ${DEBUG} -shared -fPIC -Wl,-R${LFC_LIB} -L ${LFC_LIB} -o $@ $< -llfc -ldl

XrdOucName2NameLFC.o: ${SOURCE} ${HEADERS} Makefile
	g++ ${DEBUG} ${FLAGS} -fPIC -I ${XRD_INCLUDE} -I ${LFC_INCLUDE} -c -o $@ $<

xrd-lfc-test: xrd-lfc-test.o Makefile
	g++ ${DEBUG} ${FLAGS} -fPIC -Wl,-R${LFC_LIB} -L ${LFC_LIB} -o $@ $< -llfc -ldl

xrd-lfc-test.o: ${SOURCE} ${HEADERS} Makefile
	g++ ${DEBUG} -DSTANDALONE ${FLAGS} -fPIC -I ${XRD_INCLUDE} -I ${LFC_INCLUDE} -c -o $@ $< 


clean:
	rm -vf xrd-lfc-test{,.o} XrdOucName2NameLFC.{o,so}


