############################################################################
#
# options.mk
#
# makefile options
#
# Remi Coulom
#
# November, 1996
#
# Options:
#  GUI=win32,x11,nogui (default = nogui)
#  COMPILER=cl,icl,icc,gcc,clang (default = gcc)
#  THREADS=PTHREADS,WIN32THREADS,MACTHREADS (default=PTHREADS)
#
############################################################################
ifndef GUI
 -include ../../_general/compgcc/default_gui.mk
 ifndef GUI
  GUI:=nogui
 endif
endif

ifndef COMPILER
 -include ../../_general/compgcc/default_compiler.mk
 ifndef COMPILER
  COMPILER:=gcc
 endif
endif

ifndef THREADS
 -include ../../_general/compgcc/default_threads.mk
 ifndef THREADS
  THREADS:=PTHREADS
 endif
endif

ifndef OSTYPE
 UNAME := $(shell uname -o 2>/dev/null)
 ifeq ($(UNAME),Cygwin)
  OSTYPE:=cygwin
 endif
 ifeq ($(UNAME),Msys)
  OSTYPE:=msys
 endif
 UNAME := $(shell uname)
 ifeq ($(UNAME),Darwin)
  OSTYPE:=darwin
 endif
endif


############################################################################
# compiler-independent stuff
############################################################################
CFLAGS =\
	$(OPTIMISATIONS)\
	$(WARNINGS)\
	$(DEBUG)\
	$(OTHEROPTIONS)\
	$(INCLUDES)\
	$(DEFINES)

DEPEND = ../../_general/compgcc/dep.mk

DEFINES := -D$(THREADS) -DGUI=$(GUI)

############################################################################
# cl/icl
############################################################################
INTELMSVC:=no
ifeq ($(COMPILER),cl)
 CC:=cl
 OPTIMISATIONS := -Ox -DCDECL=__cdecl -EHsc
 OTHEROPTIONS := $(OTHEROPTIONS) -D_WIN32_WINNT=0x400 -Drestrict=
# OTHEROPTIONS := $(OTHEROPTIONS) /Zi #debug
 INTELMSVC:=yes
endif
ifeq ($(COMPILER),icl)
 CC:=icl
 OPTIMISATIONS := -Ox -DCDECL=__cdecl -EHsc
 INTELMSVC:=yes
endif
ifeq ($(INTELMSVC),yes)
 GUI := win32
 OTHEROPTIONS := $(OTHEROPTIONS) -nologo -Zc:forScope -MT

# LDFLAGS := kernel32.lib user32.lib gdi32.lib comdlg32.lib comctl32.lib winmm.lib
# LDFLAGS := $(LDFLAGS) ddraw.lib
 LDFLAGS := $(LDFLAGS) -MT -link -STACK:10000000
 DEFINES := $(DEFINES) -DKBHIT_WIN32 -DCLOCK_FTIME -D_WIN32

############################################################################
# gcc/icc/clang
############################################################################
else
 ifeq ($(COMPILER),icc)
  CC = icc
#  OPTIMISATIONS = -O3 -march=pentium4
#  OPTIMISATIONS = -O3 -march=athlon64 -msse2 -mfpmath=sse
  OPTIMISATIONS = -O3 -no-prec-div -fp-model fast=2 -fast-transcendentals -msse2 -restrict
#  WARNINGS = -vec_report0 -Wall -diag-disable 654,981,383,869,1572 -Weffc++ -diag-disable 2012,2022,2015,2017,2021 -Werror
  WARNINGS = -w1 -vec-report0 -diag-disable 654
  DEBUG =
  LDFLAGS := -Kc++ -lm
  SWIG_OPTIONS := -w1 -O0
 else
  # test for the presence of ccache
  CCACHE := $(shell ccache -V 2>/dev/null | head -1 | colrm 7)
  ifeq ($(COMPILER),clang)
   CC = $(CCACHE) clang++ -Qunused-arguments -fcolor-diagnostics
  else
   CC = $(CCACHE) g++
  endif
#  OPTIMISATIONS := -O3 -fomit-frame-pointer -march=athlon64 -msse2 -mfpmath=sse
#  OPTIMISATIONS := -O3 -fomit-frame-pointer -march=pentium4
#  OPTIMISATIONS := -O3 -mfpmath=sse -msse2 -fomit-frame-pointer -ffast-math
  SWIG_OPTIONS := -Wall -O0 -fno-strict-aliasing
  OPTIMISATIONS := -O3 -ffast-math -msse2
  OTHEROPTIONS := -pthread
  LDFLAGS :=
  DEFINES := $(DEFINES) -Drestrict=__restrict__

#  OPTIMISATIONS := -O3 -pg
#  LDFLAGS = -pg

  WARNINGS := -Wall -ansi -pedantic -Wno-unused-parameter -Wno-long-long -Wconversion
  ifeq ($(OSTYPE),darwin)
   WARNINGS := $(WARNINGS) -Wshorten-64-to-32
  endif
  DEBUG := $(DEBUG) -g

  #
  # Stack debug: uncomment either one of these two lines
  #
#  DEBUG := $(DEBUG) -fstack-check -fstack-protector
  OPTIMISATIONS := $(OPTIMISATIONS) -fomit-frame-pointer

  ifneq ($(OSTYPE),cygwin)
   ifneq ($(OSTYPE),msys)
    OTHEROPTIONS := $(OTHEROPTIONS) -fpic
   endif
  endif
 endif

 #
 # Stuff installed with MacPorts
 #
 ifeq ($(OSTYPE),darwin)
#  INCLUDES := $(INCLUDES) -idirafter/opt/local/include
  INCLUDES := $(INCLUDES) -I/opt/local/include
  LDFLAGS := $(LDFLAGS) -L/opt/local/lib
 endif
 
 #
 # MSYS
 #
 ifeq ($(OSTYPE),msys)
  GUI := nogui
  DEFINES := $(DEFINES) -DKBHIT_WIN32
  INCLUDES := $(INCLUDES) -isystemc:/boost_1_42_0
  THREADS := WIN32THREADS
  CC := g++ # ccache sometimes crashes
 endif

 #
 # KBHIT
 #
 ifeq ($(GUI),win32)
  DEFINES := $(DEFINES) -DKBHIT_WIN32
 else
  ifeq ($(THREADS),WIN32THREADS)
   DEFINES := $(DEFINES) -DKBHIT_WIN32
  else
   DEFINES := $(DEFINES) -DKBHIT_UNIX
  endif
 endif

 #
 # CLOCK
 #
 DEFINES := $(DEFINES) -DCLOCK_FTIME
 
 #
 # gcc can compile win32 or X11
 #
 ifeq ($(GUI),win32)
  DEFINES := $(DEFINES) -D_WIN32 -DNOMINMAX
  LDFLAGS := $(LDFLAFS) -lgdi32 -lcomdlg32 -lcomctl32 -lwinmm
 else
  ifeq ($(GUI),x11)
   DEFINES := $(DEFINES) -DX11_AVAILABLE
   INCLUDES := $(INCLUDES) -I/usr/X11R6/include
  endif
 endif

 #CC=../../_general/bounds_checking/bc-g++

 #
 # threads
 #
 ifeq ($(THREADS),PTHREADS)
  LDFLAGS := $(LDFLAGS) -pthread
 endif
endif

############################################################################
# SWIG
############################################################################
SWIG_CC_FLAGS = \
 $(SWIG_OPTIONS)\
 -I/usr/include/python3.12\
 $(DEBUG)\
 $(OTHEROPTIONS)\
 $(INCLUDES)\
 $(DEFINES)

SWIG_FLAGS = -c++ -python $(INCLUDES) $(DEFINES)

SWIG_LD_FLAGS :=
ifeq ($(UNAME),Darwin)
 SWIG_LD_FLAGS := $(SWIG_LD_FLAGS) -lpython
endif
