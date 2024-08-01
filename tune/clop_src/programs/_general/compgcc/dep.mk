$(OUTDIR)/%.d: %.c
	@echo Updating dependencies for $(@D)/$*.o
#	@echo $(patsubst %.o:,$(@D)/%.o $@:,$(shell gcc -MM $(CFLAGS) $<)) > $@
	@$(SHELL) -ec 'gcc -MM -msse $(INCLUDES) $(DEFINES) $< \
	| sed '\''s!$*.o!$(@D)/& $@!g'\'' > $@'

$(OUTDIR)/%.d: %.cpp
	@echo Updating dependencies for $(@D)/$*.o
#	@echo $(patsubst %.o:,$(@D)/%.o $@:,$(shell gcc -MM $(CFLAGS) $<)) > $@
	@$(SHELL) -ec 'gcc -MM -msse $(INCLUDES) $(DEFINES) $< \
	| sed '\''s!$*.o!$(@D)/& $@!g'\'' > $@'

ifeq ($(INTELMSVC),yes)
$(OUTDIR)/%.o: %.cpp
	@echo compiling with $(CC): $@
	@$(CC) $< $(CFLAGS) -c -Fo$@

$(OUTDIR)/%.o: %.c
	@echo compiling with $(CC): $@
	@$(CC) $< $(CFLAGS) -c -Fo$@

$(OUTDIR)/%.o: %.rc
	rc $(RCFLAGS) -fo$@ $<
else
$(OUTDIR)/%.s: %.cpp
	@echo compiling with $(CC): $@
	@$(CC) $< $(CFLAGS) -c -g -S -fverbose-asm -o $@

$(OUTDIR)/%.o: %.cpp
	@echo compiling with $(CC): $@
	@$(CC) $< $(CFLAGS) -c -o $@

$(OUTDIR)/%.o: %.c
	@echo compiling with $(CC): $@
	@$(CC) $< $(CFLAGS) -c -o $@

$(OUTDIR)/%.o: %.rc
	windres $(WINDRESFLAGS) -i $< -o $@
endif
