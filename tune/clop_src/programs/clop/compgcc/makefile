include ../../_general/compgcc/options.mk
include ../../plot/compgcc/plot.mk
include ../../rclib/compgcc/rclib.mk
include ../../clop/compgcc/clop.mk

.PHONY: all clean clean-swig

all: bin/clop swig

ALLOBJECTS:=$(OBJECTS)

CLOPOBJECT:=../../clop/compgcc/bin/clop.o

CLOPOBJECTS:=$(OBJECTS) $(CLOPOBJECT)

ALLOBJECTS:=$(ALLOBJECTS) $(CLOPOBJECT)

bin/clop: $(CLOPOBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

#############################################################################
# SWIG
#############################################################################
include ../../_general/swig/swig.mk

#############################################################################
# clean
#############################################################################
clean: clean-swig
	rm -rvf `cat .gitignore`
	cd bin; rm -vf `cat .gitignore`

#############################################################################
# cppcheck
#############################################################################
cppcheck:
	cppcheck -D --enable=all -v $(INCLUDES) ../src

#############################################################################
# Automatic dependencies
#############################################################################
-include $(ALLOBJECTS:.o=.d)
