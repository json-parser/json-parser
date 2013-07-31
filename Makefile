
VERSION_MAJOR = 1.0
VERSION := 1.0.0

ifeq ($(shell uname),Darwin)
	SO_EXT := dylib
else
	SO_EXT := so
endif

ifeq (x$(PREFIX),x)
	PREFIX := /usr/local
endif

SO_NAME := libjsonparser.$(SO_EXT).$(VERSION_MAJOR)
REAL_NAME = libjsonparser.$(SO_EXT).$(VERSION)

all: libjsonparser.a libjsonparser.$(SO_EXT)

libjsonparser.a: json.o
	$(AR) rcs libjsonparser.a json.o

libjsonparser.so: json.o
	$(CC) -shared -Wl,-soname,$(SO_NAME) -o libjsonparser.so

libjsonparser.dylib: json.o
	$(CC) -dynamiclib json.o -o libjsonparser.dylib

json.o:
	$(CC) $(CFLAGS) -c json.c

clean:
	rm -f libjsonparser.$(SO_EXT) libjsonparser.a json.o

install: libjsonparser.$(SO_EXT) libjsonparser.a
	@echo -----
	@echo Installing shared library: $(PREFIX)/lib/libjsonparser.$(SO_EXT)
	@install -d $(PREFIX)/lib
	@install -m 0755 libjsonparser.$(SO_EXT) $(PREFIX)/lib/$(REAL_NAME)
	@rm -f $(PREFIX)/lib/$(SO_NAME)
	@ln -s $(REAL_NAME) $(PREFIX)/lib/$(SO_NAME)
	@rm -f $(PREFIX)/lib/libjsonparser.$(SO_EXT)
	@ln -s $(SO_NAME) $(PREFIX)/lib/libjsonparser.$(SO_EXT)
	@echo Installing static library: $(PREFIX)/lib/libjsonparser.a
	@install -m 0755 libjsonparser.a $(PREFIX)/lib/libjsonparser.a
	@echo Installing header file: $(PREFIX)/include/json.h
	@install -d $(PREFIX)/include
	@install -m 0644 ./json.h $(PREFIX)/include/json.h
	@echo -----
	@echo Compiler flags: -I$(PREFIX)/include
	@echo Linker flags: -L$(PREFIX)/lib -ljsonparser
	@echo ------

.PHONY: all clean install

