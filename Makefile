################################################################################
# @author      : Arno Lievens (arnolievens@gmail.com)
# @file        : Makefile
# @created     : Thursday May 20, 2021 10:31:35 CEST
################################################################################

TARGET = trx
VERSION = 1.0
AUTHOR = Arno Lievens
DATE = May 2021
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man
LIBS =
INCL =
CC = gcc
CFLAGS = -std=gnu99 -pedantic -Wextra -Wall -Wundef -Wshadow -Wpointer-arith \
		 -Wcast-align -Wstrict-prototypes -Wstrict-overflow=5 -Wwrite-strings \
		 -Wcast-qual -Wswitch-default -Wswitch-enum -Wconversion \
		 -Wunreachable-code

.PHONY: default all clean

default: $(TARGET)
all: default

OBJECTS = $(patsubst src/%.c, src/%.o, $(wildcard src/*.c))
HEADERS = $(wildcard src/*.h)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LIBS) -o $@

clean:
	-rm -f src/*.o
	-rm -f $(TARGET)

man:
	printf "%s\n%s\n%s\n\n" \
		"% $(shell echo $(TARGET) | tr a-z A-Z)(1) $(TARGET) $(VERSION)" \
		"% $(AUTHOR)" \
		"% $(DATE)" > doc/$(TARGET).tmp
	pandoc doc/$(TARGET).tmp doc/$(TARGET).md -s -t man -o $(TARGET).1
	rm -f doc/$(TARGET).tmp


install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f $(TARGET) ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/$(TARGET)
	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	cp -f $(TARGET).1 ${DESTDIR}${MANPREFIX}/man1/$(TARGET).1
	chmod 644 ${DESTDIR}${MANPREFIX}/man1/$(TARGET).1

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/$(TARGET)\
		${DESTDIR}${MANPREFIX}/man1/$(TARGET).1
