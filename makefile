# Makefile for FramepaC-ng, using GCC 4.8+ under Unix/Linux
# Last change: 29mar2018

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
#SANITIZE=-fsanitize=thread -fPIE -DHELGRIND
SANITIZE=-fsanitize=thread -fPIE -DPURIFY -DDYNAMIC_ANNOTATIONS_ENABLED=1
LINKSAN=-pie
EXTRAOBJS=dynamic_annotations.o
else ifeq ($(SANE),2)
SANITIZE=-fsanitize=address -fno-omit-framepointer -DPURIFY
else ifeq ($(SANE),3)
SANITIZE=-fsanitize=leak -DPURIFY
else ifeq ($(SANE),4)
SANITIZE=-fsanitize=memory -fno-omit-framepointer
else ifeq ($(SANE),5)
SANITIZE=-fsanitize=undefined
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

CC = g++ --std=c++11
CCLINK = $(CC)
CFLAGS = $(WARN) -ffunction-sections

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
CFLAGEXE = -L$(LIBINSTDIR) $(PROFILE) -o $@
LINKFLAGS =$(LINKBITS)
LINKFLAGS +=$(LINKTYPE)
LINKFLAGS +=$(PTHREAD)
LINKFLAGS +=$(SANITIZE)
LINKFLAGS +=$(LINKSAN)

ifeq ($(BUILD_DBG),2)
  CFLAGS += -ggdb3 -O0 -fno-inline -g3
else ifeq ($(BUILD_DBG),1)
  CFLAGS += -ggdb3 -O -g3 -fno-extern-tls-init
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

$(C)$(OBJ): ; $(CC) $(CFLAGS) -c -o $@ $<

.cpp.C: ; ln -s $< $@

.SUFFIXES: $(OBJ) .C $(C) .cpp

#########################################################################
##!!end: definitions for GCC under Linux !!##
#########################################################################

#########################################################################
# define the package info and files to be used

PACKAGE=framepacng
RELEASE=0.03

# the object modules to be included in the library file
OBJS = allocator$(OBJ) array$(OBJ) bignum$(OBJ) \
	argopt$(OBJ) argopt_real$(OBJ) argparser$(OBJ) \
	bidindex_cstr$(OBJ) bitvector$(OBJ) \
	bufbuilder_char$(OBJ) bwt$(OBJ) canonsent$(OBJ) \
	charget$(OBJ) cfile$(OBJ) cognate$(OBJ) confmatrix$(OBJ) \
	clusterinfo$(OBJ) \
	cluster_u32_dbl$(OBJ) cluster_u32_flt$(OBJ) cluster_u32_u32$(OBJ) \
	complex$(OBJ) critsect$(OBJ) cstring$(OBJ) filename$(OBJ) \
	fasthash64$(OBJ) \
	float$(OBJ) frame$(OBJ) hazardptr$(OBJ) \
	hashset_obj$(OBJ) hashset_sym$(OBJ) hashset_u32$(OBJ) \
	hashtable_objobj$(OBJ) hashtable_objsz$(OBJ) \
	hashtable_symnul$(OBJ) hashtable_symobj$(OBJ) hashtable_symsz$(OBJ) \
	hashtable_u32u32$(OBJ) hashtable_data$(OBJ) \
	hashtable_helper$(OBJ) \
	init$(OBJ) linebatch$(OBJ) random$(OBJ) \
	integer$(OBJ) jsonreader$(OBJ) list$(OBJ) listbuilder$(OBJ) \
	listutil$(OBJ) loadfilelist$(OBJ) map$(OBJ) matrix$(OBJ) \
	message$(OBJ) mmapfile$(OBJ) number$(OBJ) \
	nonobject$(OBJ) object$(OBJ) objreader$(OBJ) \
	prefixmatcher$(OBJ) \
	printf$(OBJ) ptrie_u32$(OBJ) refarray$(OBJ) \
	rational$(OBJ) set$(OBJ) slab$(OBJ) \
	slabgroup$(OBJ) smallalloc$(OBJ) \
	sparsematrix$(OBJ) spelling$(OBJ) \
	string$(OBJ) stringbuilder$(OBJ) \
	sufarray_u32u32$(OBJ) sufarray_u32u40$(OBJ) \
	symbol$(OBJ) symboltable$(OBJ) synchevent$(OBJ) \
	termvector$(OBJ) texttransforms$(OBJ) \
	threadpool$(OBJ) threshold$(OBJ) timer$(OBJ) \
	trie$(OBJ) trie_u32dbl$(OBJ) trie_u32u32$(OBJ) \
	vecsimopt$(OBJ) vecsim_name$(OBJ) \
	vecsim_u32_dbl$(OBJ) vecsim_u32_flt$(OBJ) vecsim_u32_u32$(OBJ) \
	vector_obj_dbl$(OBJ) vector_obj_flt$(OBJ) \
	vector_u32_dbl$(OBJ) vector_u32_flt$(OBJ) vector_u32_u32$(OBJ) \
	basisvector_u32$(OBJ) progress$(OBJ) \
	wordcorpus_u32u32$(OBJ) wordcorpus_u32u40$(OBJ) \
	wordsplit$(OBJ) \
	globaldata$(OBJ)

TESTS = bin/argparser$(EXE)

# the header files needed by applications using this library
HEADERS = 

# the files to be included in the source distribution archive
DISTFILES= LICENSE COPYING makefile .gitignore *.C *.h framepac/*.h template/*.cc tests/*.C tests/*.h tests/*.sh \
	hopscotch/*.h

# the library archive file for this module
LIBRARY = $(PACKAGE)$(LIB)

# the executable(s) to be built for testing the package
TESTPROGS = $(BINDIR)/argparser$(EXE) $(BINDIR)/membench$(EXE) $(BINDIR)/objtest$(EXE) $(BINDIR)/parhash$(EXE) \
	 $(BINDIR)/stringtest$(EXE) $(BINDIR)/tpool$(EXE)

#########################################################################
## the generawl build rules

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
	$(RM) *$(OBJ) tests/*$(OBJ)
	$(RM) $(TESTPROGS)

veryclean: clean
	$(RM) *.BAK
	$(RM) *.CKP *~
	$(RM) "#*#"
	$(RM) $(LIBRARY)

install: $(BINDIR)/$(TESTPROG)$(EXE)
	mkdir -p $(INSTALLDIR)
	$(CP) $(SCRIPTS) $(DATAFILES) $(BINDIR)/$(TESTPROG)$(EXE) $(INSTALLDIR)

system: $(TESTPROGS)

bootstrap:

strip:
	strip $(TESTPROGS)

tags:
	etags --c++ *.h *$(C) \
		$(TOP)/wordclus/*.h $(TOP)/wordclus/*$(C) \
		$(TOP)/framepac/*.h $(TOP)/framepac/*$(C)

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
		@ mkdir -p $(BINDIR)
		$(CCLINK) $(LINKFLAGS) $(CFLAGEXE) $< $(LIBRARY) $(USELIBS)

$(BINDIR)/membench$(EXE):	tests/membench$(OBJ) $(LIBRARY)
		@ mkdir -p $(BINDIR)
		$(CCLINK) $(LINKFLAGS) $(CFLAGEXE) $< $(LIBRARY) $(USELIBS)

$(BINDIR)/objtest$(EXE):	tests/objtest$(OBJ) $(LIBRARY)
		@ mkdir -p $(BINDIR)
		$(CCLINK) $(LINKFLAGS) $(CFLAGEXE) $< $(LIBRARY) $(USELIBS)

$(BINDIR)/parhash$(EXE):	tests/parhash$(OBJ) $(LIBRARY)
		@ mkdir -p $(BINDIR)
		$(CCLINK) $(LINKFLAGS) $(CFLAGEXE) $< $(LIBRARY) $(USELIBS)

$(BINDIR)/stringtest$(EXE):	tests/stringtest$(OBJ) $(LIBRARY)
		@ mkdir -p $(BINDIR)
		$(CCLINK) $(LINKFLAGS) $(CFLAGEXE) $< $(LIBRARY) $(USELIBS)

$(BINDIR)/tpool$(EXE):		tests/tpool$(OBJ) $(LIBRARY)
		@ mkdir -p $(BINDIR)
		$(CCLINK) $(LINKFLAGS) $(CFLAGEXE) $< $(LIBRARY) $(USELIBS)

allocator$(OBJ):	allocator$(C) framepac/atomic.h framepac/memory.h
argopt$(OBJ):		argopt$(C) template/argopt.cc
argopt_real$(OBJ):	argopt_real$(C) template/argopt.cc
argparser$(OBJ):	argparser$(C) framepac/argparser.h
array$(OBJ):		array$(C) framepac/array.h framepac/fasthash64.h
basisvector_u32$(OBJ):	basisvector_u32$(C) template/basisvector.cc
bidindex_cstr$(OBJ):	bidindex_cstr$(C) framepac/bidindex.h
bignum$(OBJ):		bignum$(C) framepac/bignum.h
bitvector$(OBJ):	bitvector$(C) framepac/bitvector.h framepac/number.h framepac/fasthash64.h
bufbuilder_char$(OBJ):	bufbuilder_char$(C) template/bufbuilder.cc
bwt$(OBJ):		bwt$(C) framepac/config.h
canonsent$(OBJ):	canonsent$(C) framepac/stringbuilder.h framepac/texttransforms.h
charget$(OBJ):		charget$(C) framepac/charget.h
cfile$(OBJ):		cfile$(C) framepac/file.h framepac/stringbuilder.h framepac/texttransforms.h
clusterinfo$(OBJ):	clusterinfo$(C) framepac/cluster.h
cluster_u32_dbl$(OBJ):	cluster_u32_dbl$(C) template/cluster_factory.cc
cluster_u32_flt$(OBJ):	cluster_u32_flt$(C) template/cluster_factory.cc
cluster_u32_u32$(OBJ):	cluster_u32_u32$(C) template/cluster_factory.cc
cognate$(OBJ):		cognate$(C) framepac/spelling.h
complex$(OBJ):		complex$(C) framepac/complex.h framepac/fasthash64.h
confmatrix$(OBJ):	confmatrix$(C) framepac/spelling.h
critsect$(OBJ):		critsect$(C) framepac/critsect.h
cstring$(OBJ):		cstring$(C) framepac/cstring.h framepac/fasthash64.h
fasthash64$(OBJ):	fasthash64$(C) framepac/fasthash64.h
filename$(OBJ):		filename$(C) framepac/file.h framepac/texttransforms.h
float$(OBJ):		float$(C) framepac/number.h framepac/fasthash64.h
frame$(OBJ):		frame$(C) framepac/frame.h
globaldata$(OBJ):	globaldata$(C)
hashset_obj$(OBJ):	hashset_obj$(C) template/hashtable.cc
hashset_sym$(OBJ):	hashset_sym$(C) template/hashtable.cc
hashset_u32$(OBJ):	hashset_u32$(C) template/hashtable.cc
hashtable_data$(OBJ):	hashtable_data$(C) framepac/hashtable.h
hashtable_helper$(OBJ):	hashtable_helper$(C) framepac/hashtable.h framepac/atomic.h
hashtable_objobj$(OBJ):	hashtable_objobj$(C) template/hashtable.cc
hashtable_objsz$(OBJ):	hashtable_objsz$(C) template/hashtable.cc
hashtable_symnul$(OBJ):	hashtable_symnul$(C) template/hashtable.cc
hashtable_symobj$(OBJ):	hashtable_symobj$(C) template/hashtable.cc
hashtable_symsz$(OBJ):	hashtable_symsz$(C) template/hashtable.cc
hashtable_u32u32$(OBJ):	hashtable_u32u32$(C) template/hashtable.cc
hazardptr$(OBJ):	hazardptr$(C) framepac/atomic.h
init$(OBJ):		init$(C) framepac/init.h framepac/symboltable.h
integer$(OBJ):		integer$(C) framepac/number.h framepac/fasthash64.h
jsonreader$(OBJ):	jsonreader$(C) framepac/objreader.h framepac/stringbuilder.h framepac/list.h \
			framepac/map.h framepac/number.h
jsonwriter$(OBJ):	jsonwriter$(C) framepac/file.h framepac/list.h
linebatch$(OBJ):	linebatch$(C) framepac/file.h
list$(OBJ):		list$(C) framepac/list.h framepac/fasthash64.h framepac/init.h
listbuilder$(OBJ):	listbuilder$(C) framepac/list.h framepac/string.h
listutil$(OBJ):		listutil$(C) framepac/list.h framepac/string.h
loadfilelist$(OBJ):	loadfilelist$(C) framepac/file.h framepac/list.h framepac/message.h
map$(OBJ):		map$(C) framepac/map.h framepac/fasthash64.h
matrix$(OBJ):		matrix$(C) framepac/matrix.h
message$(OBJ):		message$(C) framepac/message.h framepac/texttransforms.h
mmapfile$(OBJ):		mmapfile$(C) framepac/mmapfile.h framepac/file.h
nonobject$(OBJ):	nonobject$(C) framepac/nonobject.h
number$(OBJ):		number$(C) framepac/bignum.h framepac/rational.h
object$(OBJ):		object$(C) framepac/object.h
objreader$(OBJ):	objreader$(C) framepac/objreader.h framepac/symboltable.h framepac/array.h \
			framepac/bignum.h framepac/bitvector.h framepac/map.h framepac/rational.h \
			framepac/list.h framepac/number.h framepac/stringbuilder.h framepac/termvector.h \
			framepac/texttransforms.h
prefixmatcher$(OBJ):	prefixmatcher$(C) framepac/utility.h
printf$(OBJ):		printf$(C) framepac/texttransforms.h
progress$(OBJ):		progress$(C) framepac/progress.h framepac/timer.h
ptrie_u32$(OBJ):	ptrie_u32$(C) template/ptrie.cc
random$(OBJ):		random$(C) framepac/random.h
rational$(OBJ):		rational$(C) framepac/rational.h
refarray$(OBJ):		refarray$(C) framepac/array.h framepac/fasthash64.h framepac/random.h
set$(OBJ):		set$(C) framepac/set.h
slab$(OBJ):		slab$(C) framepac/memory.h
slabgroup$(OBJ):	slabgroup$(C) framepac/memory.h
smallalloc$(OBJ):	smallalloc$(C) framepac/memory.h
sparsematrix$(OBJ):	sparsematrix$(C) framepac/matrix.h
spelling$(OBJ):		spelling$(C) framepac/spelling.h
string$(OBJ):		string$(C) framepac/string.h framepac/fasthash64.h
stringbuilder$(OBJ):	stringbuilder$(C) framepac/stringbuilder.h framepac/file.h
sufarray_u32u32$(OBJ):	sufarray_u32u32$(C) template/sufarray.cc
sufarray_u32u40$(OBJ):	sufarray_u32u40$(C) template/sufarray.cc framepac/byteorder.h
symbol$(OBJ):		symbol$(C) framepac/symbol.h framepac/nonobject.h framepac/fasthash64.h
symboltable$(OBJ):	symboltable$(C) framepac/symboltable.h framepac/fasthash64.h framepac/texttransforms.h
synchevent$(OBJ):	synchevent$(C) framepac/synchevent.h
termvector$(OBJ):	termvector$(C) template/termvector.cc
texttransforms$(OBJ):	texttransforms$(C) framepac/texttransforms.h
threadpool$(OBJ):	threadpool$(C) framepac/threadpool.h framepac/thread.h
threshold$(OBJ):	threshold$(C) framepac/threshold.h
timer$(OBJ):		timer$(C) framepac/timer.h
trie$(OBJ):		trie$(C) framepac/trie.h
trie_u32dbl$(OBJ):	trie_u32dbl$(C) template/trie.cc
trie_u32u32$(OBJ):	trie_u32u32$(C) template/trie.cc
vecsimopt$(OBJ):	vecsimopt$(C) framepac/vecsim.h
vecsim_name$(OBJ):	vecsim_name$(C) framepac/utility.h framepac/vecsim.h
vecsim_u32_dbl$(OBJ):	vecsim_u32_dbl$(C) template/vecsim_factory.cc
vecsim_u32_flt$(OBJ):	vecsim_u32_flt$(C) template/vecsim_factory.cc
vecsim_u32_u32$(OBJ):	vecsim_u32_u32$(C) template/vecsim_factory.cc
vector_obj_dbl$(OBJ):	vector_obj_dbl$(C) framepac/vector.h
vector_obj_flt$(OBJ):	vector_obj_flt$(C) framepac/vector.h
vector_u32_dbl$(OBJ):	vector_u32_dbl$(C) framepac/vector.h template/sparsevector.cc
vector_u32_flt$(OBJ):	vector_u32_flt$(C) framepac/termvector.h template/sparsevector.cc
vector_u32_u32$(OBJ):	vector_u32_u32$(C) framepac/termvector.h template/sparsevector.cc
wordcorpus_u32u32$(OBJ): 	wordcorpus_u32u32$(C) template/wordcorpus.cc
wordcorpus_u32u40$(OBJ): 	wordcorpus_u32u40$(C) template/wordcorpus.cc
wordsplit$(OBJ):	wordsplit$(C) framepac/words.h

template/argopt.cc:	framepac/argparser.h
	$(TOUCH) $@ $(BITBUCKET)

template/bufbuilder.cc:	framepac/builder.h
	$(TOUCH) $@ $(BITBUCKET)

template/cluster_agglom.cc:	framepac/cluster.h
	$(TOUCH) $@ $(BITBUCKET)

template/cluster_anneal.cc:	framepac/cluster.h
	$(TOUCH) $@ $(BITBUCKET)

template/cluster_dbscan.cc:	template/cluster.cc
	$(TOUCH) $@ $(BITBUCKET)

template/cluster_growseed.cc:	template/cluster.cc
	$(TOUCH) $@ $(BITBUCKET)

template/cluster_incr.cc:	template/cluster.cc
	$(TOUCH) $@ $(BITBUCKET)

template/cluster_kmeans.cc:	framepac/cluster.h framepac/threadpool.h
	$(TOUCH) $@ $(BITBUCKET)

template/cluster_optics.cc:	template/cluster.cc
	$(TOUCH) $@ $(BITBUCKET)

template/cluster_tight.cc:	template/cluster.cc
	$(TOUCH) $@ $(BITBUCKET)

template/cluster_factory.cc: template/cluster.cc template/cluster_agglom.cc template/cluster_anneal.cc \
			template/cluster_dbscan.cc template/cluster_growseed.cc template/cluster_incr.cc \
			template/cluster_kmeans.cc template/cluster_optics.cc template/cluster_tight.cc
	$(TOUCH) $@ $(BITBUCKET)

template/cluster.cc:	framepac/cluster.h framepac/threadpool.h
	$(TOUCH) $@ $(BITBUCKET)

template/hashtable.cc:	framepac/hashtable.h framepac/message.h framepac/fasthash64.h
	$(TOUCH) $@ $(BITBUCKET)

template/mtrie.cc:	framepac/trie.h
	$(TOUCH) $@ $(BITBUCKET)

template/pmtrie.cc:	framepac/trie.h
	$(TOUCH) $@ $(BITBUCKET)

template/ptrie.cc:	framepac/trie.h
	$(TOUCH) $@ $(BITBUCKET)

template/sufarray.cc:	framepac/sufarray.h framepac/bitvector.h
	$(TOUCH) $@ $(BITBUCKET)

template/sparsevector.cc:	framepac/vector.h
	$(TOUCH) $@ $(BITBUCKET)

template/termvector.cc:	framepac/termvector.h framepac/charget.h framepac/fasthash64.h
	$(TOUCH) $@ $(BITBUCKET)

template/trie.cc:	framepac/trie.h
	$(TOUCH) $@ $(BITBUCKET)

template/trienode.cc:	framepac/trie.h
	$(TOUCH) $@ $(BITBUCKET)

template/vecsim.cc:	framepac/vecsim.h
	$(TOUCH) $@ $(BITBUCKET)

template/vecsim_ct.cc:	framepac/vecsim.h
	$(TOUCH) $@ $(BITBUCKET)

template/vecsim_factory.cc:	template/vecsim.cc template/vecsim_ct.cc
	$(TOUCH) $@ $(BITBUCKET)

template/wordcorpus.cc:	framepac/wordcorpus.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/array.h:	framepac/object.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/atomic.h:	framepac/config.h
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

framepac/counter.h:	framepac/atomic.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/critsect.h:	framepac/config.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/file.h:	framepac/config.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/frame.h:	framepac/object.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/hashtable.h:	framepac/counter.h framepac/init.h framepac/list.h framepac/number.h framepac/queue_mpsc.h \
			framepac/semaphore.h framepac/symbol.h framepac/synchevent.h
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

framepac/object.h:	framepac/memory.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/objectvmt.h:	framepac/config.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/objreader.h:	framepac/object.h framepac/charget.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/perthread.h:	framepac/config.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/progress.h:	framepac/atomic.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/rational.h:	framepac/number.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/semaphore.h:	framepac/config.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/set.h:	framepac/hashtable.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/spelling.h:	framepac/hashtable.h framepac/texttransforms.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/string.h:	framepac/object.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/stringbuilder.h:	framepac/builder.h framepac/string.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/symbol.h:	framepac/string.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/symboltable.h:	framepac/hashtable.h framepac/init.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/synchevent.h:	framepac/atomic.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/termvector.h:	framepac/vector.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/thread.h:	framepac/init.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/threadpool.h:	framepac/atomic.h framepac/critsect.h framepac/semaphore.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/trie.h:	framepac/config.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/vecsim.h:	framepac/vector.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/vector.h:	framepac/list.h framepac/symbol.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/words.h:	framepac/bidindex.h framepac/file.h framepac/string.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/wordcorpus.h:	framepac/bidindex.h framepac/builder.h framepac/byteorder.h \
		framepac/cstring.h framepac/file.h framepac/sufarray.h
	$(TOUCH) $@ $(BITBUCKET)

FramepaC.h:	framepac/config.h
	$(TOUCH) $@ $(BITBUCKET)

tests/argparser$(OBJ):	tests/argparser$(C) framepac/argparser.h
tests/membench$(OBJ):	tests/membench$(C) framepac/argparser.h framepac/memory.h framepac/threadpool.h \
			framepac/timer.h
tests/objtest$(OBJ):	tests/objtest$(C) framepac/objreader.h framepac/symboltable.h
tests/parhash$(OBJ):	tests/parhash$(C) framepac/argparser.h framepac/fasthash64.h framepac/hashtable.h \
			framepac/message.h framepac/random.h framepac/symboltable.h framepac/texttransforms.h \
			framepac/threadpool.h framepac/timer.h
tests/stringtest$(OBJ):	tests/stringtest$(C) framepac/argparser.h framepac/string.h framepac/memory.h \
			framepac/threadpool.h framepac/timer.h
tests/tpool$(OBJ):	tests/tpool$(C) framepac/argparser.h framepac/random.h framepac/threadpool.h framepac/timer.h

# End of Makefile #
