#!/usr/bin/make
# Makefile for Daelib.
# See ./README for more information.

all_proxy: all

# For those lost souls.
help:
	@echo 'Daelib: Library with common utilities for C programs.     '
	@echo 'This builds both shared and static versions of the        '
	@echo 'library, along with example programs. Type `make targets` '
	@echo 'to get a list of targets and descriptions.                '

# For quick references.
targets:
	@echo 'Targets:'
	@echo ' all        | Build all targets.                          '
	@echo ' libdae.so  | Build the shared library.                   '
	@echo ' libdae.a   | Build the statically linked library.        '
	@echo ' examples   | Build all examples.                         '
	@echo ' targets    | List targets and descriptions.              '
	@echo ' help       | Help message.                               '
	@echo ' clean      | Clean the repository.                       '

# Current version.
VERSION=0.0.1

# Toolchain programs.
CC=gcc
LD=ld
AR=ar
LN=ln

# Paths and filenames.
LIBID=dae
LIBNAME=lib$(LIBID)

LIB=lib
BIN=bin
SRC=src
INC=$(SRC)/include
TEST=$(SRC)/test

# Default prefix.
PREFIX=/usr

# CC flags.
LIBS=

CC_FLAGS= -I$(INC) -g -Wall
OBJ_FLAGS=$(CC_FLAGS) -c -fPIC
LIB_FLAGS=$(CC_FLAGS) $(LIBS) -shared -fPIC
PRG_FLAGS=$(CC_FLAGS) $(LIBS)
LIB_PRGRM=$(PRG_FLAGS) -L$(LIB) -l$(LIB)

# Objects and headers.
LIB_OBJS_REL= vector.o hashtable.o log.o loggers.o hashtable_vector.o
LIB_OBJS= $(addprefix $(SRC)/, $(LIB_OBJS_REL))

TEST_OBJS_REL = profile.o test.o
TEST_OBJS= $(addprefix $(TEST)/, $(TEST_OBJS_REL))

PUB_HEADERS_REL= assert.h log.h loggers.h vector.h hashtable.h hashtable_backend.h
PUB_HEADERS= $(addprefix $(INC)/, $(PUB_HEADERS_REL))

# Default .o rule:
%.o: %.c
	@echo "Building $@."
	@$(CC) $(OBJ_FLAGS) $< -o $@

# Source and header deps.
$(INC)/log.h:
$(SRC)/log.o: $(INC)/assert.h $(INC)/log.h $(INC)/vector.h
$(INC)/loggers.h: $(INC)/log.h
$(SRC)/loggers.o: $(INC)/assert.h $(INC)/log.h $(INC)/loggers.h

$(INC)/assert.h: $(INC)/log.h $(INC)/loggers.h

$(INC)/vector.h:
$(SRC)/vector.o: $(INC)/assert.h $(INC)/vector.h

$(INC)/hashtable.h:
$(SRC)/hashtable.o: $(INC)/assert.h $(INC)/hashtable.h $(INC)/hashtable_backend.h
$(INC)/hashtable_backend.h: $(INC)/hashtable.h
$(SRC)/hashtable_vector.o: $(INC)/hashtable_backend.h $(INC)/vector.h $(INC)/assert.h

$(TEST)/profile.o: $(SRC) $(INC)
$(TEST)/test.o: $(SRC) $(INC)

# Shared and static libraries:
LIBN=$(LIB)/$(LIBNAME)

$(LIBN).so.$(VERSION): $(LIB_OBJS) | $(LIB)
	@echo "Building $(LIBNAME) shared library."
	@$(CC) $(LIB_FLAGS) $(LIB_OBJS) -o $@

$(LIBN).so: $(LIBN).so.$(VERSION) | $(LIB)
	@echo "Linking $@ to $@.$(VERSION)."
	@$(LN) -fs $(notdir $<) $@

$(LIBN).a.$(VERSION): $(LIB_OBJS) | $(LIB)
	@echo "Building $(LIBNAME) static library."
	@$(AR) rcs $@ $(LIB_OBJS)

$(LIBN).a: $(LIBN).a.$(VERSION) | $(LIB)
	@echo "Linking $@ to $@.$(VERSION)."
	@$(LN) -fs $(notdir $<) $@

# Examples / test programs:
examples: $(BIN)/test $(BIN)/profile

$(BIN)/test: $(TEST)/test.o $(LIBN).a | $(BIN)
	@echo "Building test program."
	@$(CC) $(PRG_FLAGS) $^ -o $@

$(BIN)/profile: $(TEST)/profile.o $(LIBN).a | $(BIN)
	@echo "Building profiling program."
	@$(CC) $(PRG_FLAGS) $^ -o $@

# Directories.
$(BIN):
	@echo "Creating bin directory."
	@mkdir -p $(BIN)

$(LIB):
	@echo "Creating library directory."
	@mkdir -p $(LIB)

$(PREFIX)/lib:
	@echo "Creating library directory in $(PREFIX)."
	@mkdir -p $(PREFIX)/lib

$(PREFIX)/include/daelib:
	@echo "Creating include directory in $(PREFIX)."
	@mkdir -p $(PREFIX)/include/daelib

# Testing.
run: $(BIN)/test $(BIN)/profile
	@echo "Running test and profile programs."
	@$(BIN)/test
	@$(BIN)/profile

# Install/remove.
install: $(LIBN).so $(LIBN).a $(INC) | $(PREFIX)/lib $(PREFIX)/include/daelib
	@echo "Installing into $(PREFIX)."
	@cp -r $(INC)/* $(PREFIX)/include/daelib
	@cp -r $(LIB)/* $(PREFIX)/lib

uninstall:
	@echo "Uninstalling from root $(PREFIX)."
	@rm -rf $(PREFIX)/include/daelib
	@rm -f $(PREFIX)/lib/$(LIBNAME).*

# Clean.
clean:
	@echo "Cleaning repository."
	@rm -f $(LIB_OBJS) $(TEST_OBJS)
	@rm -rf $(LIB) $(BIN)

# The default behaviour is to build the libraries and examples.
all: $(LIBN).so $(LIBN).a examples

# Fake targets, not named after the output.
.PHONY: help targets all all_proxy run clean
