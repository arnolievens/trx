################################################################################
# @author      : Arno Lievens (arnolievens@gmail.com)
# @file        : Makefile
# @created     : Thursday May 20, 2021 10:31:35 CEST
################################################################################

TARGET = trx
VERSION = 1.0
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man
LIBS =
INCL =
CC = gcc
CFLAGS = -Wall -std=gnu99 -pedantic

.PHONY: default all clean

default: $(TARGET)
all: default

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

%.o: %.c $(HEADERS)
	# $(CC) $(CFLAGS) -I$(INCL) -c $< -o $@
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	# $(CC) $(CFLAGS) $(OBJECTS) -I$(INCL) $(LIBS) -o $@
	$(CC) $(CFLAGS) $(OBJECTS) $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)

man:
	pandoc $(TARGET).md -s -t man -o $(TARGET).1
	sed -i "s/VERSION/${VERSION}/g" $(TARGET).1


install: all man
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f $(TARGET) ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/$(TARGET)
	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	cp -f $(TARGET).1 ${DESTDIR}${MANPREFIX}/man1/$(TARGET).1
	chmod 644 ${DESTDIR}${MANPREFIX}/man1/$(TARGET).1

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/$(TARGET)\
		${DESTDIR}${MANPREFIX}/man1/$(TARGET).1

