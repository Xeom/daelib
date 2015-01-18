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
LIBS=-

CC_FLAGS= -Iinclude -g -Wall
OBJ_FLAGS=$(CC_FLAGS) -c -fPIC
LIB_FLAGS=$(CC_FLAGS) $(LIBS) -shared -fPIC
PRG_FLAGS=$(CC_FLAGS) $(LIBS)
LIB_PRGRM=$(PRG_FLAGS) -L. -ldios

LIB_OBJS=#src/vector.o src/hashtable.o src/log.o src/loggers.o src/hash.o src/hashtable_vector.o
PUB_HEADERS=#include/store.h include/stores.h include/hash.h

# Default .o rule:
%.o: %.c
	@echo "Building $@."
	@$(CC) $(OBJ_FLAGS) $< -o $@

# Source and header deps.
#src/dassert.h: src/log.h src/loggers.h
#include/hash.h:
#src/hash.o: src/dassert.h include/hash.h
#src/hashtable.h:
#src/hashtable.o: src/dassert.h src/hashtable.h src/hashtable_backend.h
#src/hashtable_backend.h: src/hashtable.h
#src/hashtable_vector.c: src/hashtable_backend.h src/vector.h src/dassert.h
#src/log.h:
#src/log.o: src/dassert.h src/log.h src/vector.h
#src/loggers.h: src/log.h
#src/loggers.o: src/dassert.h src/log.h src/loggers.h
#src/vector.h:
#src/vector.o: src/dassert.h src/vector.h

#src/profile.o: src
#src/test.o: src


# The default behaviour is to build the libraries and examples.
all: #libdae.so libdae.a test profile

# Shared and static libraries:
libdae.so.$(VERSION): $(LIB_OBJS)
	@echo "Building daelib shared library."
	@$(CC) $(LIB_FLAGS) $(LIB_OBJS) -o $@

libdae.so: libdaeso.$(VERSION)
	@echo "Linking $@ to $@.$(VERSION)."
	@$(LN) -fs $@.$(VERSION) $@

libdae.a.$(VERSION): $(LIB_OBJS)
	@echo "Building daelib static library."
	@$(AR) rcs $@ $(LIB_OBJS)

libdae.a: libdae.a.$(VERSION)
	@echo "Linking $@ to $@.$(VERSION)."
	@$(LN) -fs $@.$(VERSION) $@

# Programs.
#test: src/test.o libdae.a
#	@echo "Building test program."
#	@$(CC) $(PRG_FLAGS) $< libdae.a -o $@

#profile: src/profile.o libdae.a
#	@echo "Building profiling program."
#	@$(CC) $(PRG_FLAGS) $< libdae.a -o $@

#run: test profile
#	@echo "Running ./test and ./profile."
#	@./test
#	@./profile

clean:
	@echo "Cleaning repository."
	rm -f $(LIB_OBJS) libdae.* #src/test.o src/profile.o

# Fake targets, not named after the output.
.PHONY: help targets all all_proxy clean

