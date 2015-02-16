#updated by kobe, 2007,4,25
#---------------------------------------------------------------------------------------------
OUTPUTFILES=yajsc libyajsc.so yajsc.a
OS=`uname`
CF_CYGWIN="-II:/cygwin/usr/include/boost-1_33_1"
CF_LINUX="-I/usr/local/include/boost-1_36"
LB_CYGWIN="-lboost_regex-gcc-mt-s -llibboost_serialization-gcc"
LB_LINUX="-L/usr/local/lib -lboost_regex -lboost_serialization-gcc41-mt"
CXXFLAGS=$(shell if [[ $(OS) == Linux* ]];then echo $(CF_LINUX);else echo $(CF_CYGWIN);fi)
LIBS=$(shell if [[ $(OS) == Linux* ]];then echo $(LB_LINUX);else echo $(LB_CYGWIN);fi)
CXX=g++ -O2 -g -Wall
#---------------------------------------------------------------------------------------------

all: $(OUTPUTFILES)

.SUFFIXES: .o .cpp .hpp
#---------------------------------------------------------------------------------------------

yajsc: Main.o SymbolTable.o Common.o Compiler.o Heap.o Simulation.o Error.o BinaryCode.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)
yajsc.a: Yajsc.o Main.o SymbolTable.o Common.o Compiler.o Heap.o Simulation.o Error.o BinaryCode.o
	ar rc $@ $^
libyajsc.so: libyajsc.o Main.o SymbolTable.o Common.o Compiler.o Heap.o Simulation.o Error.o BinaryCode.o
	$(CXX) -shared -Wl,-soname,$@ -o $@ $^ $(LIBS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<
#---------------------------------------------------------------------------------------------

clean:
	rm -rf *.exe *.o *.so *.a
