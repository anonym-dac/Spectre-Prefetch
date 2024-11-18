###############################################################################
# Makefile for boom-attacks
###########################
# Makes baremetal executables to run on BOOM
###############################################################################

# Folders
SRC  := src
OBJ  := obj
BIN  := bin
DUMP := dump
DEP  := dep


CC:=gcc
OBJDUMP:=objdump -S


CFLAGS    = -Wno-unused-function -Wno-unused-variable -Wno-unused-but-set-variable

LDFLAGS:=-static 

DEPFLAGS  = -MT $@ -MMD -MP -MF $(DEP)/$*.d

# Programs to compile
PROGRAMS =  SpectrePrefetchV1 SpectrePrefetchV2
BINS     = $(addprefix $(BIN)/,$(addsuffix .x86,$(PROGRAMS)))
DUMPS    = $(addprefix $(DUMP)/,$(addsuffix .dump,$(PROGRAMS)))


all: $(BINS) $(DUMPS)

dumps: $(DUMPS)

$(OBJ)/%.o: $(SRC)/%.c
	@mkdir -p $(OBJ)
	@mkdir -p $(DEP)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

# Build executable


$(BIN)/%.x86: $(OBJ)/%.o  
	@mkdir -p $(BIN)
	$(CC) $< -o $@


# Build dump
$(DUMP)/%.dump: $(BIN)/%.x86
	@mkdir -p $(DUMP)
	$(OBJDUMP) -D $< > $@

# Keep the temporary .o files
.PRECIOUS: $(OBJ)/%.o

# Remove all generated files
clean:
	rm -rf $(BIN) $(OBJ) $(DUMP) $(DEP)
.PHONY: all clean
# Include dependencies
-include $(addprefix $(DEP)/,$(addsuffix .d,$(PROGRAMS)))
