#Author: Christopher McGinn - contact at chmc7718@colroado.edu or cffionn on skype for bugs
#2021.02.17

CXX = g++
#O3 for max optimization (go to 0 for debug)
CXXFLAGS = -Wall -O3 -Wextra -Wno-unused-local-typedefs -Wno-deprecated-declarations -std=c++11 -g
ifeq "$(GCCVERSION)" "1"
  CXXFLAGS += -Wno-error=misleading-indentation
endif

define SPHENIXADCDIRERR
 SPHENIXADCDIR is not set at all. Please set this environment variable to point to your build - this should be either
export SPHENIXADCDIR=$(PWD)
or
source setEnv.sh
if you have made appropriate changes.
For more, see README for full setup recommendations
endef

ifndef SPHENIXADCDIR
$(error "$(SPHENIXADCDIRERR)")	
endif

INCLUDE=-I$(SPHENIXADCDIR)
LIB=-L$(SPHENIXADCDIR)/lib

ROOT=`root-config --cflags --glibs`

MKDIR_BIN=mkdir -p $(SPHENIXADCDIR)/bin
MKDIR_LIB=mkdir -p $(SPHENIXADCDIR)/lib
MKDIR_OBJ=mkdir -p $(SPHENIXADCDIR)/obj
MKDIR_OUTPUT=mkdir -p $(SPHENIXADCDIR)/output
MKDIR_PDF=mkdir -p $(SPHENIXADCDIR)/pdfDir

all: mkdirBin mkdirLib mkdirObj mkdirOutput mkdirPdf obj/checkMakeDir.o obj/globalDebugHandler.o lib/libSPHENIXADC.so bin/sphenixADCProcessing.exe bin/sphenixTokenPassing.exe bin/testFileRead.exe

mkdirBin:
	$(MKDIR_BIN)

mkdirLib:
	$(MKDIR_LIB)

mkdirObj:
	$(MKDIR_OBJ)

mkdirOutput:
	$(MKDIR_OUTPUT)

mkdirPdf:
	$(MKDIR_PDF)

obj/checkMakeDir.o: src/checkMakeDir.C
	$(CXX) $(CXXFLAGS) -fPIC -c src/checkMakeDir.C -o obj/checkMakeDir.o $(INCLUDE)

obj/globalDebugHandler.o: src/globalDebugHandler.C
	$(CXX) $(CXXFLAGS) -fPIC -c src/globalDebugHandler.C -o obj/globalDebugHandler.o $(ROOT) $(INCLUDE)

lib/libSPHENIXADC.so:
	$(CXX) $(CXXFLAGS) -fPIC -shared -o lib/libSPHENIXADC.so obj/checkMakeDir.o obj/globalDebugHandler.o $(ROOT) $(INCLUDE)

bin/sphenixADCProcessing.exe: src/sphenixADCProcessing.C
	$(CXX) $(CXXFLAGS) src/sphenixADCProcessing.C -o bin/sphenixADCProcessing.exe $(ROOT) $(INCLUDE) $(LIB) -lSPHENIXADC

bin/sphenixTokenPassing.exe: src/sphenixTokenPassing.C
	$(CXX) $(CXXFLAGS) src/sphenixTokenPassing.C -o bin/sphenixTokenPassing.exe $(ROOT) $(INCLUDE) $(LIB) -lSPHENIXADC

bin/testFileRead.exe: src/testFileRead.C
	$(CXX) $(CXXFLAGS) src/testFileRead.C -o bin/testFileRead.exe $(ROOT) $(INCLUDE) $(LIB) -lSPHENIXADC

clean:
	rm -f ./*~
	rm -f ./#*#
	rm -f bash/*~
	rm -f bash/#*#
	rm -f bin/*.exe
	rm -rf bin
	rm -f include/*~
	rm -f include/#*#
	rm -f lib/*.so
	rm -rf lib
	rm -f obj/*.o
	rm -rf obj
	rm -f src/*~
	rm -f src/#*#
