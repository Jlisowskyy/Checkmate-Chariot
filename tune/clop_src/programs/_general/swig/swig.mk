SWIG_DIR:=../swig
IFILE:=$(shell ls $(SWIG_DIR)/*.i)
IFILE_BASENAME:=$(shell basename $(IFILE) .i)
WRAPFILE:=$(SWIG_DIR)/$(IFILE_BASENAME)_wrap
MODULE:=$(shell echo `grep module $(IFILE)` | sed -e "s/%module \(.*\)/\\1/")

clean-swig:
	cd $(SWIG_DIR); rm -rvf `cat .gitignore`

# Inefficient but portable method for cygwin and msys
#############################################################################
USE_SETUP=false
SETUP_OPTIONS=
COMPILE_OPTIONS=

ifeq ($(OSTYPE),cygwin)
 USE_SETUP=true
endif

ifeq ($(OSTYPE),msys)
 USE_SETUP=true
 SETUP_OPTIONS:=--compile=mingw32
endif

ifeq ($(USE_SETUP),true)
SWIG_TARGET:= $(SWIG_DIR)/_$(MODULE).so
SWIG_TMP:=\
 $(SWIG_DIR)/swig_opts.tmp\
 $(SWIG_DIR)/extra_objects.tmp\
 $(SWIG_DIR)/extra_compile_args.tmp\
 $(SWIG_DIR)/extra_link_args.tmp

$(SWIG_DIR)/swig_opts.tmp:
	@echo making $@...
	@echo $(SWIG_FLAGS) >$@

$(SWIG_DIR)/extra_objects.tmp:
	@echo making $@...
	@echo $(OBJECTS) >$@

$(SWIG_DIR)/extra_compile_args.tmp:
	@echo making $@...
	@echo $(CFLAGS) -O0 >$@

$(SWIG_DIR)/extra_link_args.tmp:
	@echo making $@...
	@echo $(LDFLAGS) >$@

swig: $(SWIG_TARGET)

$(SWIG_DIR)/setup.py: ../../_general/swig/gen_setup ../../_general/swig/setup_template.py
	cd $(SWIG_DIR); ../../_general/swig/gen_setup >$@
	chmod a+x $@

$(SWIG_TARGET): $(SWIG_TMP) $(OBJECTS) $(IFILE) $(SWIG_DIR)/setup.py
	@echo Building $(SWIG_TARGET) ...
	cd $(SWIG_DIR); ./setup.py build_ext --inplace --force $(SETUP_OPTIONS)
	touch $@

# This is more efficient
#############################################################################
else
swig: $(SWIG_DIR)/_$(MODULE).so

$(WRAPFILE).cxx: $(IFILE)
	swig $(SWIG_FLAGS) $<
	cd $(SWIG_DIR); pwd; ../../_general/swig/check-pythonappend.sh $(IFILE_BASENAME)

$(WRAPFILE).d:
	swig $(SWIG_FLAGS) -MM $(IFILE) >$@

-include $(WRAPFILE).d

$(WRAPFILE).o: $(WRAPFILE).cxx
	$(CC) $(SWIG_CC_FLAGS) -c -o $@ $^

$(SWIG_DIR)/_$(MODULE).so: $(WRAPFILE).o $(OBJECTS)
	$(CC) -shared -o $@ $^ $(LDFLAGS) $(SWIG_LD_FLAGS)
endif
