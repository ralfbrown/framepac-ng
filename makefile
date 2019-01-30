# Makefile for FramepaC-ng, using GCC 4.8+ under Unix/Linux
# Last change: 30jan2019

#########################################################################
# define the locations of all the files

# the top of the source tree
TOP = .

# where to put the compiled library and required header files
INSTDIR = $(TOP)/include
LIBINSTDIR = $(TOP)/lib

INSTALLDIR ?= $(TOP)/bin

# where to put the compiled main program
BINDIR = $(TOP)/bin
SCRIPTDIR = $(TOP)/scripts

# where to look for header files from other modules when compiling
INCLUDEDIRS = -I. -I$(TOP)/include

#########################################################################
##!!begin: definitions for GCC under Linux !!##
#########################################################################
# define the compiler and its options

ifndef BUILD_DBG
### compile fully optimized for distribution
BUILD_DBG=0
### compile with debugging info
#BUILD_DBG=1
### compile with debugging info and all optimizations off
#BUILD_DBG=2
endif

# build statically-linked executable (1=yes, 0=no)
#STATIC?=1
STATIC?=0

# enable multi-threading? (1=yes, 0=no)
THREADS?=1
#THREADS?=0

ifndef PROFILE
#PROFILE=-pg
#PROFILE=-DPURIFY
endif

ifndef NODEBUG
#NODEBUG=-DNDEBUG
endif

ifndef GDB
#GDB = -ggdb3
endif

ifeq ($(SANE),1)
SANITIZE=-fsanitize=thread -fPIC
LINKSAN=-fPIC -pie
EXTRAOBJS=dynamic_annotations.o
else ifeq ($(SANE),2)
SANITIZE=-fsanitize=address -fno-omit-frame-pointer -DPURIFY
else ifeq ($(SANE),3)
SANITIZE=-fsanitize=leak -DPURIFY
else ifeq ($(SANE),4)
SANITIZE=-fsanitize=memory -fno-omit-frame-pointer
else ifeq ($(SANE),5)
# fasthash64, BiDirIndex, and SuffixArray make deliberate misaligned accesses
SANITIZE=-fsanitize=undefined -fno-sanitize=alignment
endif

ifndef CPU
## Uncomment the appropriate CPU type
### Pentium
#CPU=5
### PentiumPro or higher
#CPU=6
### AMD Athlon; not supported by GCC 2.x
#CPU=7
### AMD64/x86_64 CPUs in 64-bit mode; not supported by GCC 2.x
###    (AMD K8 [Opteron/Athlon64], newest PentiumIV with EM64t)
#CPU=8
### AMD64 "PhenomII" (K10) or newer
#CPU=10
### Let GCC auto-determine CPU type, but assume at least CPU=8 capabilities
CPU=99
endif

ifndef BITS
#BITS=32
BITS=64
endif

ifeq ($(THREADS),1)
  PTHREAD=-pthread
#-fopenmp
else
  PTHREAD=-DFrSINGLE_THREADED
endif

ifeq ($(STATIC),1)
  LINKTYPE=-static -z muldefs
else
  LINKTYPE=
endif

ifdef MAKE_SHAREDLIB
SHAREDLIB=-fPIC -DSHARED
endif

WARN=-Wall -Wextra -Wno-deprecated -Wshadow -Wcast-align -Wmissing-noreturn -Wmissing-format-attribute
#WARN += -Weffc++ -Wc++11-compat -Wzero-as-null-pointer-constant
WARN += -Wc++11-compat -Wzero-as-null-pointer-constant
#WARN += -Wunused-result (not on Doha)
#WARN += -Wno-multichar -Wpacked -Wdisabled-optimization -Wpadded

# explicitly force includes to check here first, to fix an incompatibility
#   with the templated iostreams (they don't have many of the functions
#   present in the old iostream)
#EXTRAINC=-I/usr/include/g++-3/

LINKBITS=-m$(BITS)
ifeq ($(CPU),99)
  # auto-detection, assuming at least AMD "K8" level of features (any
  #  x64 processor qualifies); requires GCC 4.2+
  CPUDEF=-march=native -D__886__ -D__BITS__=$(BITS)
else ifeq ($(CPU),10)
  # newest AMD chips: "Barcelona", PhenomII
  CPUDEF=-march=amdfam10 -D__886__ -D__BITS__=$(BITS)
else ifeq ($(CPU),8)
  CPUDEF=-march=k8 -msse -D__BITS__=$(BITS)
else ifeq ($(CPU),7)
  CPUDEF=-march=athlon-xp -mmmx
else ifeq ($(CPU),6)
  CPUDEF=-march=i$(CPU)86 -mtune=i$(CPU)86 -mmmx
else
  # we won't bother supporting anything less than a Pentium
  CPU=5
  CPUDEF=-march=i586 -mtune=i586
endif
ifneq ($(CPU),99)
CPUDEF += -D__$(CPU)86__
endif

#########################################################################
# external libraries

#########################################################################
# define the compiler and its options

ifdef USE_CLANG
CXX = clang
CCLINK = $(CXX)
#CFLAGS =
else
CXX = g++ --std=c++11
CCLINK = $(CXX)
CFLAGS = $(WARN) -ffunction-sections
endif

CFLAGS +=$(CPUDEF)
CFLAGS +=$(PTHREAD)
CFLAGS +=$(PROFILE)
CFLAGS +=$(NODEBUG)
CFLAGS +=$(LINKBITS) -pipe
CFLAGS +=$(EXTRAINC)
CFLAGS +=$(SANITIZE)
CFLAGS +=$(INCLUDEDIRS)
CFLAGS +=$(SHAREDLIB)
CFLAGS +=$(COMPILE_OPTS)
CFLAGEXE = -L$(LIBINSTDIR) $(PROFILE)
LINKFLAGS =$(LINKBITS)
LINKFLAGS +=$(LINKTYPE)
LINKFLAGS +=$(PTHREAD)
LINKFLAGS +=$(SANITIZE)
LINKFLAGS +=$(LINKSAN)

ifeq ($(BUILD_DBG),2)
  CFLAGS += -ggdb3 -O0 -fno-inline -g3
else ifeq ($(BUILD_DBG),1)
  CFLAGS += -ggdb3 -Og -g3 -fno-extern-tls-init
else
  CFLAGS += -O3 -fexpensive-optimizations -fno-extern-tls-init -g$(DBGLVL) $(GDB)
# CFLAGS += -fweb -ftracer -fgcse-sm -fgcse-las -fno-math-errno
endif

#########################################################################
# define the object module librarian and its options

LIBRARIAN = ar
LIBFLAGS = rucl
LIBOBJS = $(OBJS)

#########################################################################
# define the library indexer and its options

LIBINDEXER = ranlib
LIBIDXFLAGS = $(LIBRARY)

#########################################################################
# define file copy/deletion/etc. programs

RM = rm -f
CP = cp -p
ZIP = zip -q
ZIPFLAGS = -qo9
BITBUCKET = >&/dev/null
TOUCH = touch

#########################################################################
# define the various extensions in use

OBJ = .o
EXE =
LIB = .a
C = .C

#########################################################################
# define the required libraries in the proper format for the OS

USELIBS = -lcrypt -lrt $(SRILM_LIBS)
ifeq ($(THREADS),1)
#USELIBS += -lgomp
endif

#########################################################################
# define the default compile rule

$(C)$(OBJ): ; $(CXX) $(CFLAGS) -c -o $@ $<

build/%$(OBJ) : src/%$(C)
	@mkdir -p build
	$(CXX) $(CFLAGS) -c -o $@ $<

$(BINDIR)/%$(EXE) : tests/%$(OBJ)
	@mkdir -p $(BINDIR)
	$(CCLINK) $(LINKFLAGS) $(CFLAGEXE) -o $@ $< $(LIBRARY) $(USELIBS)

.cpp.C: ; ln -s $< $@

.SUFFIXES: $(OBJ) .C $(C) .cpp

#########################################################################
##!!end: definitions for GCC under Linux !!##
#########################################################################

#########################################################################
# define the package info and files to be used

PACKAGE=framepacng
RELEASE=0.13

# the object modules to be included in the library file
OBJS = \
	build/globaldata$(OBJ) \
	build/allocator$(OBJ) \
	build/argopt$(OBJ) \
	build/argopt_real$(OBJ) \
	build/argparser$(OBJ) \
	build/array$(OBJ) \
	build/as_string$(OBJ) \
	build/basisvector_u32$(OBJ) \
	build/basisvector_u32flt$(OBJ) \
	build/bidindex_cstr$(OBJ) \
	build/bignum$(OBJ) \
	build/bitvector$(OBJ) \
	build/bufbuilder_char$(OBJ) \
	build/bwt$(OBJ) \
	build/canonsent$(OBJ) \
	build/cfgfile$(OBJ) \
	build/cfile$(OBJ) \
	build/charget$(OBJ) \
	build/cluster_name$(OBJ) \
	build/cluster_u32_dbl$(OBJ) \
	build/cluster_u32_flt$(OBJ) \
	build/cluster_u32_u32$(OBJ) \
	build/cluster$(OBJ) \
	build/clusterinfo$(OBJ) \
	build/cognate$(OBJ) \
	build/complex$(OBJ) \
	build/confmatrix$(OBJ) \
	build/contextcoll_sym$(OBJ) \
	build/contextcoll_u32$(OBJ) \
	build/convert$(OBJ) \
	build/copyfile$(OBJ) \
	build/critsect$(OBJ) \
	build/cstring$(OBJ) \
	build/cstring_file$(OBJ) \
	build/dbllist$(OBJ) \
	build/fasthash64$(OBJ) \
	build/filename$(OBJ) \
	build/float$(OBJ) \
	build/frame$(OBJ) \
	build/hashset_obj$(OBJ) \
	build/hashset_sym$(OBJ) \
	build/hashset_u32$(OBJ) \
	build/hashtable_data$(OBJ) \
	build/hashtable_helper$(OBJ) \
	build/hashtable_objobj$(OBJ) \
	build/hashtable_objsz$(OBJ) \
	build/hashtable_symnul$(OBJ) \
	build/hashtable_symobj$(OBJ) \
	build/hashtable_symsz$(OBJ) \
	build/hashtable_u32u32$(OBJ) \
	build/hashtable_u32obj$(OBJ) \
	build/hazardptr$(OBJ) \
	build/init$(OBJ) \
	build/integer$(OBJ) \
	build/is_number$(OBJ) \
	build/jsonreader$(OBJ) \
	build/jsonwriter$(OBJ) \
	build/keylayout$(OBJ) \
	build/linebatch$(OBJ) \
	build/list$(OBJ) \
	build/listbuilder$(OBJ) \
	build/listutil$(OBJ) \
	build/loadfilelist$(OBJ) \
	build/map$(OBJ) \
	build/map_file$(OBJ) \
	build/matrix$(OBJ) \
	build/message$(OBJ) \
	build/mmapfile$(OBJ) \
	build/nonobject$(OBJ) \
	build/number$(OBJ) \
	build/object$(OBJ) \
	build/objreader$(OBJ) \
	build/prefixmatcher$(OBJ) \
	build/printf$(OBJ) \
	build/progress$(OBJ) \
	build/ptrie_u32$(OBJ) \
	build/random$(OBJ) \
	build/rational$(OBJ) \
	build/refarray$(OBJ) \
	build/set$(OBJ) \
	build/signal$(OBJ) \
	build/slab$(OBJ) \
	build/slabgroup$(OBJ) \
	build/smallalloc$(OBJ) \
	build/sparsematrix$(OBJ) \
	build/spelling$(OBJ) \
	build/string$(OBJ) \
	build/stringbuilder$(OBJ) \
	build/sufarray_u32u32$(OBJ) \
	build/sufarray_u32u40$(OBJ) \
	build/symbol$(OBJ) \
	build/symbolprop$(OBJ) \
	build/symboltable$(OBJ) \
	build/synchevent$(OBJ) \
	build/termvector$(OBJ) \
	build/texttransforms$(OBJ) \
	build/threadpool$(OBJ) \
	build/threshold$(OBJ) \
	build/timer$(OBJ) \
	build/trie$(OBJ) \
	build/trie_u32dbl$(OBJ) \
	build/trie_u32lst$(OBJ) \
	build/trie_u32u32$(OBJ) \
	build/vecsim_name$(OBJ) \
	build/vecsim_u32_dbl$(OBJ) \
	build/vecsim_u32_flt$(OBJ) \
	build/vecsim_u32_u32$(OBJ) \
	build/vecsimopt$(OBJ) \
	build/vector_obj_dbl$(OBJ) \
	build/vector_obj_flt$(OBJ) \
	build/vector_u32_dbl$(OBJ) \
	build/vector_u32_flt$(OBJ) \
	build/vector_u32_u32$(OBJ) \
	build/wordcorpus_u32u32$(OBJ) \
	build/wordcorpus_u32u40$(OBJ) \
	build/wordsplit$(OBJ) \
	build/wordsplit_eng$(OBJ)

TESTS = bin/argparser$(EXE)

# the header files needed by applications using this library
HEADERS = 

# the files to be included in the source distribution archive
DISTFILES= LICENSE COPYING makefile .gitignore *.h src/*.C framepac/*.h template/*.cc \
	tests/*.C tests/*.h tests/*.sh hopscotch/*.h

# the library archive file for this module
LIBRARY = $(PACKAGE)$(LIB)

# the executable(s) to be built for testing the package
TESTPROGS = \
	$(BINDIR)/argparser$(EXE) \
	$(BINDIR)/clustertest$(EXE) \
	$(BINDIR)/cogscore$(EXE) \
	$(BINDIR)/membench$(EXE) \
	$(BINDIR)/objtest$(EXE) \
	$(BINDIR)/parhash$(EXE) \
	$(BINDIR)/splitwords$(EXE) \
	$(BINDIR)/stringtest$(EXE) \
	$(BINDIR)/tpool$(EXE)

#########################################################################
## the general build rules

all: $(TESTPROGS)

help:
	@echo "The makefile for $(PACKAGE) understands the following targets:"
	@echo "  all       perform a complete build of library and test program"
	@echo "  lib       build the library only"
	@echo "  install   build and install the library"
	@echo "  system    install the test program in the top-level directory"
	@echo "  clean     erase intermediate files from the build"
	@echo "  veryclean erase all intermediate and backup files"
	@echo "  bootstrap install files needed by packages on which we depend"
	@echo "  tags      rebuild TAGS database"
	@echo "  tar       pack up all distribution files in a tar archive"
	@echo "  txz       pack up all distribution files in an xz-compressed tar file"
	@echo "  zip       pack up all distribution files in a zip archive"
	@echo "  help      you're looking at it..."

lib:	$(LIBRARY)

clean:
	$(RM) *$(OBJ) build/*$(OBJ) tests/*$(OBJ)
	$(RM) $(TESTPROGS)

veryclean: clean
	$(RM) *.BAK
	$(RM) *.CKP *~ */*~
	$(RM) "#*#"
	$(RM) $(LIBRARY)

install: $(BINDIR)/$(TESTPROG)$(EXE)
	@mkdir -p $(INSTALLDIR)
	$(CP) $(SCRIPTS) $(DATAFILES) $(BINDIR)/$(TESTPROG)$(EXE) $(INSTALLDIR)

system: $(TESTPROGS)

bootstrap:
	@mkdir -p build

strip:
	strip $(TESTPROGS)

tags:
	etags --c++ *.h *$(C) src/*$(C) framepac/*.h templates/*cc tests/*.h tests/*$(C)

tar:
	$(RM) $(PACKAGE).tar
	tar chf $(PACKAGE).tar $(DISTFILES)

zip:
	$(RM) frng-$(RELEASE).zip
	zip -qo9 frng-$(RELEASE).zip $(DISTFILES)

txz:
	$(RM) frng-$(RELEASE).txz
	tar -cf frng-$(RELEASE).txz --use-compress-program xz $(DISTFILES)

$(LIBRARY): $(OBJS)
	-$(RM) $(LIBRARY)
	$(LIBRARIAN) $(LIBFLAGS) $(LIBRARY) $(LIBOBJS)
	$(LIBINDEXER) $(LIBIDXFLAGS)

$(LIBINSTDIR)/$(LIBRARY): $(LIBRARY)
	$(CP) $(HEADERS) $(INSTDIR)
	$(CP) $(LIBRARY) $(LIBINSTDIR)

#########################################################################
## the dependencies for each module of the full package

$(BINDIR)/argparser$(EXE):	tests/argparser$(OBJ) $(LIBRARY)
$(BINDIR)/clustertest$(EXE):	tests/clustertest$(OBJ) $(LIBRARY)
$(BINDIR)/cogscore$(EXE):	tests/cogscore$(OBJ) $(LIBRARY)
$(BINDIR)/membench$(EXE):	tests/membench$(OBJ) $(LIBRARY)
$(BINDIR)/objtest$(EXE):	tests/objtest$(OBJ) $(LIBRARY)
$(BINDIR)/parhash$(EXE):	tests/parhash$(OBJ) $(LIBRARY)
$(BINDIR)/splitwords$(EXE):	tests/splitwords$(OBJ) $(LIBRARY)
$(BINDIR)/stringtest$(EXE):	tests/stringtest$(OBJ) $(LIBRARY)
$(BINDIR)/tpool$(EXE):		tests/tpool$(OBJ) $(LIBRARY)

build/allocator$(OBJ):		src/allocator$(C) framepac/atomic.h framepac/memory.h
build/argopt$(OBJ):		src/argopt$(C) template/argopt.cc
build/argopt_real$(OBJ):	src/argopt_real$(C) template/argopt.cc
build/argparser$(OBJ):		src/argparser$(C) framepac/argparser.h framepac/texttransforms.h
build/array$(OBJ):		src/array$(C) framepac/array.h framepac/fasthash64.h
build/as_string$(OBJ):		src/as_string$(C) framepac/as_string.h framepac/object.h framepac/texttransforms.h
build/basisvector_u32$(OBJ):	src/basisvector_u32$(C) template/basisvector.cc
build/basisvector_u32flt$(OBJ):	src/basisvector_u32flt$(C) template/basisvector.cc
build/bidindex_cstr$(OBJ):	src/bidindex_cstr$(C) template/bidindex.cc framepac/cstring.h
build/bignum$(OBJ):		src/bignum$(C) framepac/bignum.h
build/bitvector$(OBJ):		src/bitvector$(C) framepac/bitvector.h framepac/number.h framepac/fasthash64.h
build/bufbuilder_char$(OBJ):	src/bufbuilder_char$(C) template/bufbuilder.cc
build/bwt$(OBJ):		src/bwt$(C) framepac/config.h
build/canonsent$(OBJ):		src/canonsent$(C) framepac/texttransforms.h framepac/words.h
build/charget$(OBJ):		src/charget$(C) framepac/charget.h framepac/builder.h
build/cfgfile$(OBJ):		src/cfgfile$(C) framepac/configfile.h framepac/charget.h framepac/list.h \
				framepac/as_string.h framepac/message.h framepac/string.h framepac/symbol.h \
				framepac/texttransforms.h
build/cfile$(OBJ):		src/cfile$(C) framepac/file.h framepac/message.h framepac/stringbuilder.h \
				framepac/texttransforms.h
build/cluster$(OBJ):		src/cluster$(C) framepac/cluster.h framepac/message.h framepac/progress.h \
				framepac/signal.h framepac/texttransforms.h framepac/cstring.h framepac/convert.h \
				framepac/stringbuilder.h framepac/utility.h framepac/words.h
build/clusterinfo$(OBJ):	src/clusterinfo$(C) framepac/atomic.h framepac/cluster.h framepac/symboltable.h \
				framepac/cstring.h framepac/texttransforms.h
build/cluster_name$(OBJ):	src/cluster_name$(C) framepac/cluster.h
build/cluster_u32_dbl$(OBJ):	src/cluster_u32_dbl$(C) template/cluster_factory.cc
build/cluster_u32_flt$(OBJ):	src/cluster_u32_flt$(C) template/cluster_factory.cc
build/cluster_u32_u32$(OBJ):	src/cluster_u32_u32$(C) template/cluster_factory.cc
build/cognate$(OBJ):		src/cognate$(C) framepac/file.h framepac/spelling.h framepac/stringbuilder.h \
				framepac/cstring.h
build/complex$(OBJ):		src/complex$(C) framepac/complex.h framepac/fasthash64.h
build/confmatrix$(OBJ):		src/confmatrix$(C) framepac/spelling.h
build/contextcoll_sym$(OBJ):	src/contextcoll_sym$(C) template/contextcoll.cc
build/contextcoll_u32$(OBJ):	src/contextcoll_u32$(C) template/contextcoll.cc
build/convert$(OBJ):		src/convert$(C) framepac/convert.h
build/copyfile$(OBJ):		src/copyfile$(C) framepac/file.h
build/critsect$(OBJ):		src/critsect$(C) framepac/critsect.h
build/cstring$(OBJ):		src/cstring$(C) framepac/cstring.h framepac/fasthash64.h
build/cstring_file$(OBJ):	src/cstring_file$(C) framepac/cstring.h framepac/file.h
build/dbllist$(OBJ):		src/dbllist$(C) framepac/list.h
build/fasthash64$(OBJ):		src/fasthash64$(C) framepac/fasthash64.h
build/filename$(OBJ):		src/filename$(C) framepac/file.h framepac/texttransforms.h
build/float$(OBJ):		src/float$(C) framepac/number.h framepac/fasthash64.h
build/frame$(OBJ):		src/frame$(C) framepac/frame.h
build/globaldata$(OBJ):		src/globaldata$(C)
build/hashset_obj$(OBJ):	src/hashset_obj$(C) template/hashtable.cc
build/hashset_sym$(OBJ):	src/hashset_sym$(C) template/hashtable.cc
build/hashset_u32$(OBJ):	src/hashset_u32$(C) template/hashtable.cc
build/hashtable_data$(OBJ):	src/hashtable_data$(C) framepac/hashtable.h
build/hashtable_helper$(OBJ):	src/hashtable_helper$(C) framepac/hashtable.h framepac/hashhelper.h framepac/atomic.h
build/hashtable_objobj$(OBJ):	src/hashtable_objobj$(C) template/hashtable.cc
build/hashtable_objsz$(OBJ):	src/hashtable_objsz$(C) template/hashtable.cc
build/hashtable_symnul$(OBJ):	src/hashtable_symnul$(C) template/hashtable.cc
build/hashtable_symobj$(OBJ):	src/hashtable_symobj$(C) template/hashtable.cc
build/hashtable_symsz$(OBJ):	src/hashtable_symsz$(C) template/hashtable.cc
build/hashtable_u32u32$(OBJ):	src/hashtable_u32u32$(C) template/hashtable.cc template/hashtable_file.cc
build/hashtable_u32obj$(OBJ):	src/hashtable_u32obj$(C) template/hashtable.cc template/hashtable_file.cc
build/hazardptr$(OBJ):		src/hazardptr$(C) framepac/atomic.h
build/init$(OBJ):		src/init$(C) framepac/init.h framepac/symboltable.h framepac/hashhelper.h
build/integer$(OBJ):		src/integer$(C) framepac/number.h framepac/fasthash64.h
build/is_number$(OBJ):		src/is_number$(C) framepac/cstring.h
build/jsonreader$(OBJ):		src/jsonreader$(C) framepac/objreader.h framepac/stringbuilder.h framepac/list.h \
				framepac/map.h framepac/number.h framepac/cstring.h
build/jsonwriter$(OBJ):		src/jsonwriter$(C) framepac/file.h framepac/list.h
build/keylayout$(OBJ):		src/keylayout$(C) framepac/spelling.h
build/linebatch$(OBJ):		src/linebatch$(C) framepac/file.h
build/list$(OBJ):		src/list$(C) framepac/list.h framepac/fasthash64.h framepac/init.h
build/listbuilder$(OBJ):	src/listbuilder$(C) framepac/list.h framepac/string.h
build/listutil$(OBJ):		src/listutil$(C) framepac/list.h framepac/string.h
build/loadfilelist$(OBJ):	src/loadfilelist$(C) framepac/file.h framepac/list.h framepac/message.h
build/map$(OBJ):		src/map$(C) framepac/map.h framepac/fasthash64.h
build/map_file$(OBJ):		src/map_file$(C) framepac/map.h framepac/file.h
build/matrix$(OBJ):		src/matrix$(C) framepac/matrix.h
build/message$(OBJ):		src/message$(C) framepac/message.h framepac/texttransforms.h
build/mmapfile$(OBJ):		src/mmapfile$(C) framepac/mmapfile.h framepac/file.h
build/nonobject$(OBJ):		src/nonobject$(C) framepac/nonobject.h
build/number$(OBJ):		src/number$(C) framepac/bignum.h framepac/rational.h
build/object$(OBJ):		src/object$(C) framepac/object.h framepac/objreader.h framepac/cstring.h
build/objreader$(OBJ):		src/objreader$(C) framepac/objreader.h framepac/symboltable.h framepac/array.h \
			framepac/bignum.h framepac/bitvector.h framepac/map.h framepac/rational.h \
			framepac/list.h framepac/number.h framepac/stringbuilder.h framepac/termvector.h \
			framepac/texttransforms.h
build/prefixmatcher$(OBJ):	src/prefixmatcher$(C) framepac/utility.h
build/printf$(OBJ):		src/printf$(C) framepac/texttransforms.h
build/progress$(OBJ):		src/progress$(C) framepac/progress.h framepac/stringbuilder.h framepac/texttransforms.h
build/ptrie_u32$(OBJ):		src/ptrie_u32$(C) template/ptrie.cc
build/random$(OBJ):		src/random$(C) framepac/message.h framepac/random.h framepac/critsect.h
build/rational$(OBJ):		src/rational$(C) framepac/rational.h
build/refarray$(OBJ):		src/refarray$(C) framepac/array.h framepac/fasthash64.h framepac/random.h
build/set$(OBJ):		src/set$(C) framepac/set.h
build/signal$(OBJ):		src/signal$(C) framepac/signal.h framepac/message.h
build/slab$(OBJ):		src/slab$(C) framepac/memory.h
build/slabgroup$(OBJ):		src/slabgroup$(C) framepac/memory.h framepac/semaphore.h framepac/critsect.h
build/smallalloc$(OBJ):		src/smallalloc$(C) framepac/memory.h
build/sparsematrix$(OBJ):	src/sparsematrix$(C) framepac/matrix.h
build/spelling$(OBJ):		src/spelling$(C) framepac/spelling.h
build/string$(OBJ):		src/string$(C) framepac/string.h framepac/fasthash64.h
build/stringbuilder$(OBJ):	src/stringbuilder$(C) framepac/stringbuilder.h framepac/file.h
build/sufarray_u32u32$(OBJ):	src/sufarray_u32u32$(C) template/sufarray.cc template/sufarray_file.cc
build/sufarray_u32u40$(OBJ):	src/sufarray_u32u40$(C) template/sufarray.cc template/sufarray_file.cc \
				framepac/byteorder.h
build/symbol$(OBJ):		src/symbol$(C) framepac/symboltable.h framepac/nonobject.h framepac/fasthash64.h
build/symbolprop$(OBJ):		src/symbolprop$(C) framepac/frame.h framepac/list.h framepac/symbol.h
build/symboltable$(OBJ):	src/symboltable$(C) framepac/symboltable.h framepac/fasthash64.h \
				framepac/texttransforms.h
build/synchevent$(OBJ):		src/synchevent$(C) framepac/synchevent.h
build/termvector$(OBJ):		src/termvector$(C) template/termvector.cc
build/texttransforms$(OBJ):	src/texttransforms$(C) framepac/texttransforms.h
build/threadpool$(OBJ):		src/threadpool$(C) framepac/threadpool.h framepac/memory.h framepac/thread.h
build/threshold$(OBJ):		src/threshold$(C) framepac/threshold.h
build/timer$(OBJ):		src/timer$(C) framepac/timer.h framepac/texttransforms.h
build/trie$(OBJ):		src/trie$(C) framepac/trie.h
build/trie_u32dbl$(OBJ):	src/trie_u32dbl$(C) template/trie.cc
build/trie_u32lst$(OBJ):	src/trie_u32lst$(C) template/trie.cc framepac/list.h
build/trie_u32u32$(OBJ):	src/trie_u32u32$(C) template/trie.cc
build/vecsimopt$(OBJ):		src/vecsimopt$(C) framepac/vecsim.h
build/vecsim_name$(OBJ):	src/vecsim_name$(C) framepac/utility.h framepac/vecsim.h
build/vecsim_u32_dbl$(OBJ):	src/vecsim_u32_dbl$(C) template/vecsim_factory.cc
build/vecsim_u32_flt$(OBJ):	src/vecsim_u32_flt$(C) template/vecsim_factory.cc
build/vecsim_u32_u32$(OBJ):	src/vecsim_u32_u32$(C) template/vecsim_factory.cc
build/vector_obj_dbl$(OBJ):	src/vector_obj_dbl$(C) template/sparsevector.cc
build/vector_obj_flt$(OBJ):	src/vector_obj_flt$(C) template/sparsevector.cc
build/vector_u32_dbl$(OBJ):	src/vector_u32_dbl$(C) template/vector.cc template/densevector.cc \
				template/sparsevector.cc
build/vector_u32_flt$(OBJ):	src/vector_u32_flt$(C) template/vector.cc template/densevector.cc \
				template/sparsevector.cc
build/vector_u32_u32$(OBJ):	src/vector_u32_u32$(C) template/vector.cc template/densevector.cc \
				template/sparsevector.cc
build/wordcorpus_u24u32$(OBJ): 	src/wordcorpus_u32u40$(C) template/wordcorpus.cc template/concbuilder.cc
build/wordcorpus_u32u32$(OBJ): 	src/wordcorpus_u32u32$(C) template/wordcorpus.cc template/hashtable.cc \
				template/concbuilder.cc template/bufbuilder_file.cc template/hashtable_file.cc \
				template/sufarray_file.cc
build/wordcorpus_u32u40$(OBJ): 	src/wordcorpus_u32u40$(C) template/wordcorpus.cc
build/wordsplit$(OBJ):		src/wordsplit$(C) framepac/charget.h framepac/list.h framepac/stringbuilder.h \
				framepac/words.h
build/wordsplit_eng$(OBJ):	src/wordsplit_eng$(C) framepac/words.h

globaldata$(C):
	@mkdir -p build

template/argopt.cc:	framepac/argparser.h framepac/as_string.h framepac/texttransforms.h
	$(TOUCH) $@ $(BITBUCKET)

template/basisvector.cc: framepac/random.h framepac/basisvector.h
	$(TOUCH) $@ $(BITBUCKET)

template/bidindex.cc:	framepac/bidindex.h framepac/file.h framepac/message.h framepac/mmapfile.h
	$(TOUCH) $@ $(BITBUCKET)

template/bufbuilder.cc:	framepac/builder.h framepac/convert.h
	$(TOUCH) $@ $(BITBUCKET)

template/bufbuilder_file.cc:	framepac/builder.h framepac/file.h framepac/message.h
	$(TOUCH) $@ $(BITBUCKET)

template/cluster_agglom.cc:	framepac/cluster.h framepac/symboltable.h
	$(TOUCH) $@ $(BITBUCKET)

template/cluster_anneal.cc:	framepac/cluster.h
	$(TOUCH) $@ $(BITBUCKET)

template/cluster_dbscan.cc:	template/cluster.cc
	$(TOUCH) $@ $(BITBUCKET)

template/cluster_growseed.cc:	framepac/cluster.h framepac/progress.h framepac/vector.h
	$(TOUCH) $@ $(BITBUCKET)

template/cluster_incr.cc:	template/cluster.cc
	$(TOUCH) $@ $(BITBUCKET)

template/cluster_kmeans.cc:	framepac/cluster.h framepac/threadpool.h
	$(TOUCH) $@ $(BITBUCKET)

template/cluster_optics.cc:	template/cluster.cc
	$(TOUCH) $@ $(BITBUCKET)

template/cluster_snn.cc:	template/cluster.cc
	$(TOUCH) $@ $(BITBUCKET)

template/cluster_tight.cc:	framepac/cluster.h
	$(TOUCH) $@ $(BITBUCKET)

template/cluster_factory.cc: template/cluster.cc template/cluster_agglom.cc template/cluster_anneal.cc \
			template/cluster_dbscan.cc template/cluster_growseed.cc template/cluster_incr.cc \
			template/cluster_kmeans.cc template/cluster_optics.cc template/cluster_snn.cc \
			template/cluster_tight.cc
	$(TOUCH) $@ $(BITBUCKET)

template/cluster.cc:	framepac/cluster.h framepac/hashtable.h framepac/progress.h framepac/random.h \
			framepac/threadpool.h
	$(TOUCH) $@ $(BITBUCKET)

template/concbuilder.cc:	framepac/concbuilder.h template/bufbuilder.cc
	$(TOUCH) $@ $(BITBUCKET)

template/contextcoll.cc:	framepac/contextcoll.h
	$(TOUCH) $@ $(BITBUCKET)

template/densevector.cc:	framepac/vector.h template/bufbuilder.cc
	$(TOUCH) $@ $(BITBUCKET)

template/hashtable.cc:	framepac/hashtable.h framepac/message.h framepac/fasthash64.h
	$(TOUCH) $@ $(BITBUCKET)

template/hashtable_file.cc:	framepac/hashtable.h framepac/file.h framepac/message.h
	$(TOUCH) $@ $(BITBUCKET)

template/mtrie.cc:	framepac/trie.h
	$(TOUCH) $@ $(BITBUCKET)

template/pmtrie.cc:	framepac/trie.h
	$(TOUCH) $@ $(BITBUCKET)

template/ptrie.cc:	framepac/trie.h
	$(TOUCH) $@ $(BITBUCKET)

template/sufarray.cc:	framepac/sufarray.h framepac/bitvector.h
	$(TOUCH) $@ $(BITBUCKET)

template/sufarray_file.cc:	framepac/sufarray.h framepac/file.h framepac/message.h
	$(TOUCH) $@ $(BITBUCKET)

template/sparsevector.cc:	framepac/fasthash64.h framepac/vector.h template/bufbuilder.cc
	$(TOUCH) $@ $(BITBUCKET)

template/termvector.cc:	framepac/termvector.h framepac/charget.h
	$(TOUCH) $@ $(BITBUCKET)

template/trie.cc:	framepac/trie.h template/trienode.cc
	$(TOUCH) $@ $(BITBUCKET)

template/trienode.cc:	framepac/trie.h
	$(TOUCH) $@ $(BITBUCKET)

template/vecsim.cc:	framepac/vecsim.h
	$(TOUCH) $@ $(BITBUCKET)

template/vecsim_ct.cc:	framepac/vecsim.h
	$(TOUCH) $@ $(BITBUCKET)

template/vecsim_factory.cc:	framepac/message.h template/vecsim.cc template/vecsim_ct.cc
	$(TOUCH) $@ $(BITBUCKET)

template/vector.cc:	framepac/fasthash64.h framepac/vector.h
	$(TOUCH) $@ $(BITBUCKET)

template/vector_arith.cc:	template/vector.cc
	$(TOUCH) $@ $(BITBUCKET)

template/wordcorpus.cc:	framepac/wordcorpus.h framepac/mmapfile.h framepac/texttransforms.h framepac/words.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/argparser.h:	framepac/as_string.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/array.h:	framepac/object.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/as_string.h:	framepac/cstring.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/atomic.h:	framepac/config.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/basisvector.h:	framepac/vector.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/bidindex.h:	framepac/cstring.h framepac/hashtable.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/bignum.h:	framepac/number.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/bitvector.h:	framepac/object.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/builder.h:	framepac/config.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/bwt.h:	framepac/config.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/charget.h:	framepac/file.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/cluster.h:	framepac/array.h framepac/list.h framepac/symbol.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/complex.h:	framepac/number.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/configfile.h:	framepac/cstring.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/concbuilder.h:	framepac/builder.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/contextcoll.h:	framepac/hashtable.h framepac/vector.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/counter.h:	framepac/atomic.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/critsect.h:	framepac/config.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/cstring.h:	framepac/smartptr.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/file.h:	framepac/config.h framepac/cstring.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/frame.h:	framepac/object.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/hashhelper.h:	framepac/queue_mpsc.h framepac/semaphore.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/hashtable.h:	framepac/counter.h framepac/critsect.h framepac/init.h framepac/list.h framepac/number.h \
			framepac/symbol.h framepac/synchevent.h framepac/fasthash64.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/list.h:	framepac/object.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/map.h:		framepac/hashtable.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/matrix.h:	framepac/object.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/memory.h:	framepac/atomic.h framepac/init.h framepac/objectvmt.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/nonobject.h: framepac/object.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/number.h:	framepac/object.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/object.h:	framepac/cstring.h framepac/memory.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/objectvmt.h:	framepac/config.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/objreader.h:	framepac/object.h framepac/charget.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/perthread.h:	framepac/config.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/progress.h:	framepac/atomic.h framepac/cstring.h framepac/timer.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/random.h:	framepac/smartptr.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/rational.h:	framepac/number.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/semaphore.h:	framepac/config.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/set.h:	framepac/hashtable.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/spelling.h:	framepac/hashtable.h framepac/texttransforms.h framepac/trie.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/string.h:	framepac/object.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/stringbuilder.h:	framepac/builder.h framepac/cstring.h framepac/string.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/sufarray.h:	framepac/file.h framepac/mmapfile.h framepac/range.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/symbol.h:	framepac/string.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/symboltable.h:	framepac/hashtable.h framepac/init.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/synchevent.h:	framepac/atomic.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/termvector.h:	framepac/vector.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/texttransforms.h:	framepac/cstring.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/thread.h:	framepac/init.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/threadpool.h:	framepac/atomic.h framepac/critsect.h framepac/semaphore.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/timer.h:	framepac/cstring.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/trie.h:	framepac/config.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/utility.h:	framepac/list.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/vecsim.h:	framepac/vector.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/vector.h:	framepac/as_string.h framepac/critsect.h framepac/list.h framepac/symbol.h framepac/smartptr.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/words.h:	framepac/bidindex.h framepac/file.h framepac/string.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/wordcorpus.h:	framepac/bidindex.h framepac/builder.h framepac/byteorder.h \
		framepac/cstring.h framepac/file.h framepac/mmapfile.h framepac/sufarray.h
	$(TOUCH) $@ $(BITBUCKET)

FramepaC.h:		framepac/config.h framepac/argparser.h framepac/hashtable.h framepac/cluster.h \
		framepac/sufarray.h framepac/threadpool.h framepac/wordcorpus.h
	$(TOUCH) $@ $(BITBUCKET)

tests/argparser$(OBJ):	tests/argparser$(C) framepac/argparser.h
tests/clustertest$(OBJ): tests/clustertest$(C) framepac/argparser.h framepac/cluster.h framepac/file.h \
			framepac/message.h framepac/threadpool.h framepac/timer.h framepac/cstring.h
tests/cogscore$(OBJ):	tests/cogscore$(C) framepac/argparser.h framepac/spelling.h
tests/membench$(OBJ):	tests/membench$(C) framepac/argparser.h framepac/memory.h framepac/threadpool.h \
			framepac/timer.h
tests/objtest$(OBJ):	tests/objtest$(C) framepac/objreader.h framepac/symboltable.h
tests/parhash$(OBJ):	tests/parhash$(C) framepac/argparser.h framepac/fasthash64.h framepac/hashtable.h \
			framepac/message.h framepac/random.h framepac/symboltable.h framepac/texttransforms.h \
			framepac/threadpool.h framepac/timer.h framepac/cstring.h
tests/splitwords$(OBJ): tests/splitwords$(C) framepac/words.h framepac/argparser.h framepac/charget.h \
			framepac/cstring.h framepac/file.h
tests/stringtest$(OBJ):	tests/stringtest$(C) framepac/argparser.h framepac/string.h framepac/memory.h \
			framepac/threadpool.h framepac/timer.h
tests/tpool$(OBJ):	tests/tpool$(C) framepac/argparser.h framepac/random.h framepac/threadpool.h framepac/timer.h

# End of Makefile #
