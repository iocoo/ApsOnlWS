

include $(FEHOME)/mak/platform.mak
include $(FEHOME)/mak/database.mak

PRGOBJS =stdsoap2.o \
	 soapC.o \
	 soapServer.o \
	 StlmDateSvr.o  \
         Ultil.o \
         WtLog.o 

PRGTARG =ApsOnlWS_MT
PRGLIBS =-lpthread -ldbs -lzlog
#-L$(FEHOME)/lib3rd/lib -locilib
PRGDEFS = 

debug all: debugexec
release: releaseexec

# DO NOT modify any code below!!!

releasedynamic debugdynamic releasestatic debugstatic releaseexec debugexec clean:
	@make -f $(FEHOME)/mak/mkstand.mak $@ TARGET="$(PRGTARG)" OBJS="$(PRGOBJS)" LIBS="$(PRGLIBS)" 
