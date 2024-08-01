INCLUDES := $(INCLUDES)\
 -I../../rclib/src/util\
 -I../../rclib/src/conui\
 -I../../rclib/src/io\
 -I../../_general

vpath %cpp ../../rclib/src/util
vpath %cpp ../../rclib/src/conui
vpath %cpp ../../rclib/src/io
vpath %cpp ../../rclib/src/misc

OUTDIR := ../../rclib/compgcc/bin
include $(DEPEND)

OBJECTS := $(OBJECTS) $(patsubst %.o, $(OUTDIR)/%.o,\
 CTimeIO.o\
 ReadLineToString.o\
 chtime.o\
 clktimer.o\
 consolui.o\
 justtext.o\
 mykbhit.o\
 mybeep.o\
 readstr.o\
 userflag.o\
 version.o\
)

# Win32 optional classes
ifeq ($(GUI),win32)
INCLUDES := $(INCLUDES)\
 -I../../rclib/src/wingui

vpath %cpp ../../rclib/src/wingui

OBJECTS := $(OBJECTS) $(patsubst %.o, $(OUTDIR)/%.o,\
 instance.o\
 subwin.o\
 wincui.o\
 window.o\
 CBrushCache.o\
)

#No directx with mingw (yet)
ifneq ($(DIRECTX),no)
OBJECTS := $(OBJECTS) $(patsubst %.o, $(OUTDIR)/%.o,\
 dderror.o\
)
endif

endif

ifeq ($(GUI),x11)
INCLUDES := $(INCLUDES)\
 -I../../rclib/src/xgui

vpath %cpp ../../rclib/src/xgui

OBJECTS := $(OBJECTS) $(patsubst %.o, $(OUTDIR)/%.o,\
 CXWindow.o\
)

LDFLAGS:=$(LDFLAGS) -lX11 -L/usr/X11R6/lib -L/usr/X11R6/lib64
endif
