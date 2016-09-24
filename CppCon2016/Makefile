STD	  	 := -std=c++11
CXXFLAGS := -Wall -Wextra 
BASEARCH := -m64 -mfpmath=sse

.DEFAULT_GOAL = all
.PHONY = all ideal cstyle clean dist-clean dump

SRCS := calc_sum.cpp gen_files.cpp streaming.cpp
OBJS := $(patsubst %.cpp, %.o, $(SRCS))
EXEC := calc_sum gen_files

all: Makefile $(EXEC)

# Intel doesn't support -mfma
intrinsics: CXXFLAGS += -DINTRINSICS $(if $(findstring icc, $(CXX)), -march=core-avx2, -mfma)
intrinsics: streaming

cstyle: CXXFLAGS += -DCSTYLE
cstyle: streaming

streaming: streaming.o
	@ $(CXX) $(OPT) $(BASEARCH) $(ARCH) $< -lm -o $@

$(EXEC): %: %.o
	@ $(CXX) $(OPT) $(BASEARCH) $(ARCH) $< -o $@

gen_files.o : gen_files.cpp Makefile
	@ $(CXX) $(STD) $(CXXFLAGS) $(BASEARCH) $(ARCH) -march=native -O3 -c -o $@ $<

calc_sum.o: calc_sum.cpp Makefile
	@ $(CXX) $(STD) $(CXXFLAGS) $(BASEARCH) $(ARCH) -msse -march=x86-64 -O0 -c -o $@ $<

%.o : %.cpp Makefile
	$(CXX) $(STD) $(CXXFLAGS) $(OPT) $(BASEARCH) $(ARCH) -c -o $@ $<

clean:
	@ $(RM) $(OBJS) streaming

dist-clean: clean
	@ $(RM) $(EXEC)

dump:
	@ objdump -D -j .text -M intel streaming
