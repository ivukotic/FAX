RC     := root-config
ifeq ($(shell which $(RC) 2>&1 | sed -ne "s@.*/$(RC)@$(RC)@p"),$(RC))
MKARCH := $(wildcard $(shell $(RC) --etcdir)/Makefile.arch)
endif
ifneq ($(MKARCH),)
include $(MKARCH)
else
include $(ROOTSYS)/test/Makefile.arch
endif
# -include ../MyConfig.mk

ALIBS = $(LIBS) -lTreePlayer

#------------------------------------------------------------------------------
READO       = read.$(ObjSuf)
READS       = read.$(SrcSuf)
READ        = readDirect$(ExeSuf)

READCO       = readWrite.$(ObjSuf)
READCS       = readWrite.$(SrcSuf)
READC        = readWrite$(ExeSuf)

OBJS          = $(READO) $(READCO)
PROGRAMS      = $(READ) $(READC)

#------------------------------------------------------------------------------

.SUFFIXES: .$(SrcSuf) .$(ObjSuf) .$(DllSuf)

all:            $(PROGRAMS)


$(READ):      $(READO)
		$(LD) $(LDFLAGS) $^ $(ALIBS) $(OutPutOpt)$@
		$(MT_EXE)
		@echo "$@ done"

$(READC):      $(READCO)
		$(LD) $(LDFLAGS) $^ $(ALIBS) $(OutPutOpt)$@
		$(MT_EXE)
		@echo "$@ done"

