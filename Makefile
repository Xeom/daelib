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
	@echo ' libdae.so | Build the shared library.                    '
	@echo ' libdae.a  | Build the statically linked library.         '
	@echo ' examples   | Build all examples.                         '
	@echo ' targets    | List targets and descriptions.              '
	@echo ' help       | Help message.                               '
	@echo ' clean      | Clean the repository.                       '

# Current version.
VERSION=0.0.1

# Sane defaults.
CC=gcc
LD=ld
AR=ar
LN=ln

# Flags.
LIB=dae
LIBFILE=lib$(LIB)
LIBNAME=$(LIB)lib
LIBS=

SRC=src
INC=$(SRC)/include
TEST=$(SRC)/test

CC_FLAGS= -I$(INC) -g -Wall
OBJ_FLAGS=$(CC_FLAGS) -c -fPIC
LIB_FLAGS=$(CC_FLAGS) $(LIBS) -shared -fPIC
PRG_FLAGS=$(CC_FLAGS) $(LIBS)
LIB_PRGRM=$(PRG_FLAGS) -L. -l$(LIB)

LIB_OBJS_REL= vector.o hashtable.o log.o loggers.o hashtable_vector.o
LIB_OBJS= $(addprefix $(SRC)/, $(LIB_OBJS_REL))

TEST_OBJS_REL = profile.o test.o
TEST_OBJS= $(addprefix $(TEST)/, $(TEST_OBJS_REL))

PUB_HEADERS= assert.h log.h loggers.h vector.h hashtable.h hashtable_backend.h

# Default .o rule:
%.o: %.c
	@echo "Building $@."
	@$(CC) $(OBJ_FLAGS) $< -o $@

# Source and header deps.
$(INC)/assert.h: $(INC)/log.h $(INC)/loggers.h
$(INC)/hashtable.h:
$(SRC)/hashtable.o: $(INC)/assert.h $(INC)/hashtable.h $(INC)/hashtable_backend.h
$(INC)/hashtable_backend.h: $(INC)/hashtable.h
$(SRC)/hashtable_vector.o: $(INC)/hashtable_backend.h $(INC)/vector.h $(INC)/assert.h
$(INC)/log.h:
$(SRC)/log.o: $(INC)/assert.h $(INC)/log.h $(INC)/vector.h
$(INC)/loggers.h: $(INC)/log.h
$(SRC)/loggers.o: $(INC)/assert.h $(INC)/log.h $(INC)/loggers.h
$(INC)/vector.h:
$(SRC)/vector.o: $(INC)/assert.h $(INC)/vector.h

$(TEST)/profile.o: $(SRC) $(INC)
$(TEST)/test.o: $(SRC) $(INC)


# The default behaviour is to build the libraries and examples.
all: $(LIBFILE).so $(LIBFILE).a test profile

# Shared and static libraries:
$(LIBFILE).so.$(VERSION): $(LIB_OBJS)
	@echo "Building $(LIBNAME) shared library."
	@$(CC) $(LIB_FLAGS) $(LIB_OBJS) -o $@

$(LIBFILE).so: $(LIBFILE).so.$(VERSION)
	@echo "Linking $@ to $@.$(VERSION)."
	@$(LN) -fs $@.$(VERSION) $@

$(LIBFILE).a.$(VERSION): $(LIB_OBJS)
	@echo "Building $(LIBNAME) static library."
	@$(AR) rcs $@ $(LIB_OBJS)

$(LIBFILE).a: $(LIBFILE).a.$(VERSION)
	@echo "Linking $@ to $@.$(VERSION)."
	@$(LN) -fs $@.$(VERSION) $@

# Programs.
test: $(TEST)/test.o $(LIBFILE).a
	@echo "Building test program."
	@$(CC) $(PRG_FLAGS) $< $(LIBFILE).a -o $@

profile: $(TEST)/profile.o $(LIBFILE).a
	@echo "Building profiling program."
	@$(CC) $(PRG_FLAGS) $< $(LIBFILE).a -o $@

run: test profile
	@echo "Running ./test and ./profile."
	@./test
	@./profile

clean:
	@echo "Cleaning repository."
	rm -f $(LIB_OBJS) $(LIBFILE).* $(TEST)/test.o $(TEST)/profile.o

# Fake targets, not named after the output.
.PHONY: help targets all all_proxy run clean
