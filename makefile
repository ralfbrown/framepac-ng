# Makefile for FramepaC-ng, using GCC 4+ under Unix/Linux
# Last change: 28mar2017

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

$(C)$(OBJ): ; $(CC) $(CFLAGS) -c $<

.cpp.C: ; ln -s $< $@

.SUFFIXES: $(OBJ) .C $(C) .cpp

#########################################################################
##!!end: definitions for GCC under Linux !!##
#########################################################################

#########################################################################
# define the package info and files to be used

PACKAGE=framepacng
RELEASE=0.01

# the object modules to be included in the library file
OBJS = allocator$(OBJ) array$(OBJ) bignum$(OBJ) bitvector$(OBJ) \
	bufbuilder_char$(OBJ) bwt$(OBJ) charget$(OBJ) cfile$(OBJ) \
	cluster$(OBJ) cluster_growseed$(OBJ) cluster_kmeans$(OBJ) \
	complex$(OBJ) critsect$(OBJ) filename$(OBJ) \
	float$(OBJ) frame$(OBJ) hashtable_u32u32$(OBJ) \
	init$(OBJ) random$(OBJ) \
	integer$(OBJ) jsonreader$(OBJ) list$(OBJ) map$(OBJ) matrix$(OBJ) \
	message$(OBJ) mmapfile$(OBJ) number$(OBJ) \
	object$(OBJ) objreader$(OBJ) rational$(OBJ) slab$(OBJ) \
	slabgroup$(OBJ) \
	sparsematrix$(OBJ) string$(OBJ) \
	sufarray_u32u32$(OBJ) \
	symbol$(OBJ) symboltable$(OBJ) synchevent$(OBJ) \
	threadpool$(OBJ) timer$(OBJ) trie$(OBJ) \
	vecsim_u32_dbl$(OBJ) vecsim_u32_flt$(OBJ) \
	vector_obj_dbl$(OBJ) vector_obj_flt$(OBJ) \
	vector_u32_dbl$(OBJ) vector_u32_flt$(OBJ) \
	basisvector_u32$(OBJ) progress$(OBJ) \
	wordcorpus_u32u32$(OBJ) wordcorpus_u32u40$(OBJ) \
	wordsplit$(OBJ)

# the header files needed by applications using this library
HEADERS = 

# the files to be included in the source distribution archive
DISTFILES= LICENSE COPYING makefile .gitignore *.C *.h framepac/*.h template/*.cc

# the library archive file for this module
LIBRARY = $(PACKAGE)$(LIB)

# the executable to be built for testing the package
TESTPROG = testfp

# the object modules needed to build the test program
TESTOBJS = $(TESTPROG)$(OBJ)

#########################################################################
## the generawl build rules

all: $(BINDIR)/$(TESTPROG)$(EXE)

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
	$(RM) *$(OBJ)
	$(RM) $(BINDIR)/$(TESTPROG)$(EXE)
#	$(RM) $(LIBRARY)

veryclean: clean
	$(RM) *.BAK
	$(RM) *.CKP *~
	$(RM) "#*#"

install: $(BINDIR)/$(TESTPROG)$(EXE)
	mkdir -p $(INSTALLDIR)
	$(CP) $(SCRIPTS) $(DATAFILES) $(BINDIR)/$(TESTPROG)$(EXE) $(INSTALLDIR)

system: $(BINDIR)/$(TESTPROG)$(EXE)

bootstrap:

strip:
	strip $(BINDIR)/$(TESTPROG)$(EXE)

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

$(BINDIR)/$(TESTPROG)$(EXE):
	echo FIXME

$(LIBRARY): $(OBJS)
	-$(RM) $(LIBRARY)
	$(LIBRARIAN) $(LIBFLAGS) $(LIBRARY) $(LIBOBJS)
	$(LIBINDEXER) $(LIBIDXFLAGS)

$(LIBINSTDIR)/$(LIBRARY): $(LIBRARY)
	$(CP) $(HEADERS) $(INSTDIR)
	$(CP) $(LIBRARY) $(LIBINSTDIR)

#########################################################################
## the dependencies for each module of the full package

$(TESTPROG)$(OBJ):
	echo FIXME

$(TESTPROG)$(C):
	echo FIXME: $(TOUCH) $@ \$(BITBUCKET)

allocator$(OBJ):	allocator$(C) framepac/memory.h
array$(OBJ):		array$(C) framepac/array.h
basisvector_u32$(OBJ):	basisvector_u32$(C) template/basisvector.cc
bignum$(OBJ):		bignum$(C) framepac/bignum.h
bitvector$(OBJ):	bitvector$(C) framepac/bitvector.h
bufbuilder_char$(OBJ):	bufbuilder_char$(C) template/bufbuilder.cc
bwt$(OBJ):		bwt$(C) framepac/config.h
charget$(OBJ):		charget$(C) framepac/charget.h
cfile$(OBJ):		cfile$(C) framepac/file.h framepac/string.h
cluster$(OBJ):		cluster$(C) framepac/cluster.h
cluster_agglom$(OBJ):	cluster_agglom$(C) framepac/cluster.h
cluster_growseed$(OBJ):	cluster_growseed$(C) framepac/cluster.h
cluster_incr$(OBJ):	cluster_incr$(C) framepac/cluster.h
cluster_kmeans$(OBJ):	cluster_kmeans$(C) framepac/cluster.h
complex$(OBJ):		complex$(C) framepac/complex.h
critsect$(OBJ):		critsect$(C) framepac/critsect.h
filename$(OBJ):		filename$(C) framepac/file.h
float$(OBJ):		float$(C) framepac/number.h
frame$(OBJ):		frame$(C) framepac/frame.h
hashtable_u32u32$(OBJ):	hashtable_u32u32$(C) template/hashtable.cc
init$(OBJ):		init$(C) framepac/init.h
integer$(OBJ):		integer$(C) framepac/number.h
jsonreader$(OBJ):	jsonreader$(C) framepac/objreader.h framepac/hashtable.h framepac/list.h \
			framepac/map.h
list$(OBJ):		list$(C) framepac/list.h framepac/init.h
map$(OBJ):		map$(C) framepac/map.h
matrix$(OBJ):		matrix$(C) framepac/matrix.h
message$(OBJ):		message$(C) framepac/message.h
mmapfile$(OBJ):		mmapfile$(C) framepac/mmapfile.h framepac/file.h
number$(OBJ):		number$(C) framepac/bignum.h framepac/rational.h
progress$(OBJ):		progress$(C) framepac/progress.h framepac/timer.h
object$(OBJ):		object$(C) framepac/object.h
objreader$(OBJ):	objreader$(C) framepac/objreader.h framepac/symbol.h framepac/bignum.h framepac/rational.h \
			framepac/list.h framepac/builder.h
random$(OBJ):		random$(C) framepac/random.h
rational$(OBJ):		rational$(C) framepac/rational.h
slab$(OBJ):		slab$(C) framepac/memory.h
slabgroup$(OBJ):	slabgroup$(C) framepac/memory.h
sparsematrix$(OBJ):	sparsematrix$(C) framepac/matrix.h
string$(OBJ):		string$(C) framepac/string.h
suffixarray$(OBJ):	suffixarray$(C) framepac/config.h
symbol$(OBJ):		symbol$(C) framepac/symbol.h
symboltable$(OBJ):	symboltable$(C) framepac/symbol.h
synchevent$(OBJ):	synchevent$(C) framepac/synchevent.h
threadpool$(OBJ):	threadpool$(C) framepac/threadpool.h
timer$(OBJ):		timer$(C) framepac/timer.h
trie$(OBJ):		trie$(C) framepac/trie.h
vecsim_u32_dbl$(OBJ):	vecsim_u32_dbl$(C) template/vecsim.cc
vecsim_u32_flt$(OBJ):	vecsim_u32_flt$(C) template/vecsim.cc
vector_obj_dbl$(OBJ):	vector_obj_dbl$(C) framepac/vector.h
vector_obj_flt$(OBJ):	vector_obj_flt$(C) framepac/vector.h
vector_u32_dbl$(OBJ):	vector_u32_dbl$(C) framepac/vector.h
vector_u32_flt$(OBJ):	vector_u32_flt$(C) framepac/vector.h
wordcorpus_u32u32$(OBJ): wordcorpus_u32u32$(C) template/wordcorpus.cc
wordcorpus_u32u40$(OBJ): wordcorpus_u32u40$(C) template/wordcorpus.cc
wordsplit$(OBJ):	wordsplit$(C) framepac/words.h

template/bufbuilder.cc:	framepac/builder.h
	$(TOUCH) $@ $(BITBUCKET)

template/hashtable.cc:	framepac/hashtable.h
	$(TOUCH) $@ $(BITBUCKET)

template/sufarray.cc:	framepac/sufarray.h framepac/bitvector.h
	$(TOUCH) $@ $(BITBUCKET)

template/vecsim.cc:	framepac/vecsim.h
	$(TOUCH) $@ $(BITBUCKET)

template/wordcorpus.cc:	framepac/wordcorpus.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/array.h:	framepac/object.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/atomic.h:	framepac/config.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/bignum.h:	framepac/number.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/bitvector.h:	framepac/object.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/builder.h:	framepac/string.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/bwt.h:		framepac/config.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/complex.h:	framepac/number.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/critsect.h:	framepac/config.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/file.h:	framepac/config.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/frame.h:	framepac/object.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/hashtable.h:	framepac/synchevent.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/list.h:	framepac/object.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/map.h:		framepac/object.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/matrix.h:	framepac/object.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/memory.h:	framepac/atomic.h framepac/init.h framepac/objectvmt.h
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

framepac/string.h:	framepac/object.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/symbol.h:	framepac/object.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/synchevent.h:	framepac/atomic.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/threadpool.h:	framepac/atomic.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/trie.h:	framepac/config.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/vecsim.h:	framepac/vector.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/vector.h:	framepac/object.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/words.h:	framepac/bidindex.h framepac/file.h
	$(TOUCH) $@ $(BITBUCKET)

framepac/wordcorpus.h:	framepac/bidindex.h framepac/builder.h framepac/byteorder.h \
		framepac/cstring.h framepac/file.h framepac/sufarray.h
	$(TOUCH) $@ $(BITBUCKET)

FramepaC.h:	framepac/config.h
	$(TOUCH) $@ $(BITBUCKET)

# End of Makefile #
