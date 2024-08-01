INCLUDES := $(INCLUDES)\
 -I../../plot/src\
 -I../../math/src

vpath %cpp ../../plot/src

OUTDIR := ../../plot/compgcc/bin
include $(DEPEND)

OBJECTS := $(OBJECTS) $(patsubst %.o, $(OUTDIR)/%.o,\
 CDiscretizedLine.o\
 CPGF.o\
 CPlot.o\
 CSplineFit.o\
 CContour.o\
)
