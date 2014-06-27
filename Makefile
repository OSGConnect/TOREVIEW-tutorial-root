RC     := root-config
ifeq ($(shell which $(RC) 2>&1 | sed -ne "s@.*/$(RC)@$(RC)@p"),$(RC))
MKARCH := $(wildcard $(shell $(RC) --etcdir)/Makefile.arch)
endif
ifneq ($(MKARCH),)
include $(MKARCH)
else
#include $(ROOTSYS)/test/Makefile.arch
endif

ALIBS = $(LIBS) -lTreePlayer

#------------------------------------------------------------------------------
INSPO       = inspector.$(ObjSuf)
INSPS       = inspector.$(SrcSuf)
INSP        = inspector$(ExeSuf)

OBJS          = $(INSPO)
PROGRAMS      = $(INSP)

#------------------------------------------------------------------------------

.SUFFIXES: .$(SrcSuf) .$(ObjSuf) .$(DllSuf)

all:            $(PROGRAMS)


$(INSP):      $(INSPO)
		$(LD) $(LDFLAGS) $^ $(ALIBS) $(OutPutOpt)$@
		$(MT_EXE)
		@echo "$@ done"
