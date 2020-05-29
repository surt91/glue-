TARGET	= glue++
DOC 	= manual.pdf

CXXFLAGS = -std=c++11 -fexceptions -pipe

CPP	 := $(wildcard *.cpp)

OBJ	 = $(CPP:%.cpp=obj/%.o)
DEP	 = $(CPP:%.cpp=dep/%.d)

# diagnostics color is introduced with gcc 4.9, test if our gcc knows it
# http://stackoverflow.com/a/17947005/1698412
GCC_GTEQ_490 := $(shell expr `gcc -dumpversion | sed -e 's/\.\([0-9][0-9]\)/\1/g' -e 's/\.\([0-9]\)/0\1/g' -e 's/^[0-9]\{3,4\}$$/&00/'` \>= 40900)
ifeq "$(GCC_GTEQ_490)" "1"
    CXXFLAGS += -fdiagnostics-color=auto
endif

VERSION := $(shell git describe --tags --always)

release: CXXFLAGS += -O3 -flto -mtune=native -mtune=corei7 -fno-strict-aliasing
release: VERSION += release
release: all
silent: CXXFLAGS += -DNLOG
silent: VERSION += silent
silent: release
debug: CXXFLAGS += -g -Og
debug: VERSION += debug
debug: all

CXXFLAGS += -DVERSION="\"$(VERSION)\""
CXXFLAGS += -fopenmp

# for clang sanitizers
#CXX = clang++
#CXXFLAGS = -O1 -g -fsanitize=address -fno-omit-frame-pointer
#CXXFLAGS = -fsanitize=memory -fno-omit-frame-pointer -g -O2 -fsanitize-memory-track-origins
#CXXFLAGS = -fsanitize=undefined -O2

WARNLEVEL= -Wall -Wextra -Wpedantic -Werror -Wno-unknown-pragmas -Wno-unused-result
#WARNLEVEL += -Wfloat-equal -Wshadow -Wextra-semi -Wdocumentation -Wdeprecated
#WANRLEVEL += -Weverything -Wno-c++98-compat -Wno-c++98-compat-bind-to-temporary-copy -Wno-padded

LNDIRS  =

INCLUDES = -Itclap/include -Ikissfft

CXXFLAGS += $(INCLUDES)

LIBS = -lm -lz kissfft/libkissfft.a
LFLAGS	= $(LNDIRS) $(LIBS)

all: $(DEP) $(TARGET)

.DELETE_ON_ERROR:
.PHONY: clean proper

MAKEFILE_TARGETS_WITHOUT_INCLUDE := clean proper
ifeq ($(filter $(MAKECMDGOALS),$(MAKEFILE_TARGETS_WITHOUT_INCLUDE)),)
-include $(DEP)
endif

obj dep:
	mkdir $@

# perfect dependencies, see https://www.gnu.org/software/make/manual/make.pdf
dep/%.d: %.cpp | dep
	@echo Dep: $@
	@mkdir -p $(@D)
	@set -e; rm -f $@; \
	$(CXX) $(WARNLEVEL) $(CXXFLAGS) -MM $< > $@.$$$$; \
	sed 's,\($(*F)\)\.o[ :]*,obj/$*.o $@: ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

obj/%.o: %.cpp | obj
	@mkdir -p $(@D)
	$(CXX) -c $(WARNLEVEL) $(CXXFLAGS) $< -o $@

$(TARGET): $(OBJ) kissfft/libkissfft.a
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ) $(LFLAGS)

kissfft/libkissfft.a:
	$(MAKE) -C kissfft

doc/mathjax.zip:
	mkdir -p doc/html/
	wget -c https://codeload.github.com/mathjax/MathJax/zip/master -O doc/mathjax.zip

doc/mathjax: | doc/mathjax.zip
	unzip -qud doc/ doc/mathjax.zip
	mv doc/MathJax-master doc/mathjax

doc: $(DOC)
$(DOC): doc.conf *.cpp *.hpp | doc/mathjax
	doxygen doc.conf
	$(MAKE) -C doc/latex clean
	$(MAKE) -C doc/latex
	cp doc/latex/refman.pdf $@

proper:
	rm -rf $(OBJ)

clean: proper
	rm -rf dep
	rm -rf $(TARGET)
