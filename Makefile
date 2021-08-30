################################################################################
# @author      : Arno Lievens (arnolievens@gmail.com)
# @file        : Makefile
# @created     : Thursday May 20, 2021 10:31:35 CEST
################################################################################

TARGET       = trx
DESCRIPTION  = connect to serial devices from cli flags or file
VERSION      = 0.11
AUTHOR       = arnolievens@gmail.com
DATE         = May 2021

DEST_DIR     = /usr
PREFIX       = /local/bin
MAN_PREFIX   = /local/share/man/man1

SRC_DIR      = ./src
INC_DIR      = ./include
BIN_DIR      = ./bin
BUILD_DIR    = ./build
MAN_DIR      = ./man

.PHONY: clean install uninstall dpkg

SOURCES     := $(shell find $(SRC_DIR) -name *.c)
OBJECTS     := $(addprefix $(BUILD_DIR)/,$(notdir $(SOURCES:.c=.o)))
DEPS        := $(OBJS:.o=.d)

LIBS         =
INCLUDES     =

CC           = gcc

CFLAGS       = -std=gnu99 -pedantic -Wextra -Wall -Wundef -Wshadow \
			   -Wpointer-arith -Wcast-align -Wstrict-prototypes \
			   -Wstrict-overflow=5 -Wwrite-strings -Wcast-qual \
			   -Wswitch-default -Wswitch-enum -Wconversion -Wunreachable-code

LDFLAGS      =

# debian dpkg control file
define DEBIAN_CONTROL
Package: $(TARGET)
Version: $(VERSION)
section: custom
priority: optional
architecture: all
essential: no
installed-size: 1024
Depends: $(DEPENDENCIES)
maintainer: $(AUTHOR)
description: $(DESCRIPTION)
endef
export DEBIAN_CONTROL

all: $(BIN_DIR)/$(TARGET) man

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/$(TARGET): $(OBJECTS)
	mkdir -p $(BIN_DIR)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

man:
	mkdir -p $(MAN_DIR)
	printf "%s\n%s\n%s\n\n" \
		"% $(shell echo $(TARGET) | tr a-z A-Z)(1) $(TARGET) $(VERSION)" \
		"% $(AUTHOR)" \
		"% $(DATE)" > doc/$(TARGET).tmp
	pandoc doc/$(TARGET).tmp doc/$(TARGET).md -s -t man -o $(MAN_DIR)/$(TARGET).1
	rm -f doc/$(TARGET).tmp

clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(BIN_DIR)
	rm -rf $(MAN_DIR)
	rm -rf $(TARGET)
	rm  -f $(TARGET).deb

install: all
	mkdir -p $(DEST_DIR)$(PREFIX)
	cp -f $(BIN_DIR)/$(TARGET) $(DEST_DIR)$(PREFIX)/$(TARGET)
	chmod 755 $(DEST_DIR)$(PREFIX)/$(TARGET)
	mkdir -p $(DEST_DIR)$(MAN_PREFIX)
	cp -f $(MAN_DIR)/$(TARGET).1 $(DEST_DIR)$(MAN_PREFIX)/$(TARGET).1
	chmod 644 $(DEST_DIR)$(MAN_PREFIX)/$(TARGET).1

uninstall:
	rm -f $(DEST_DIR)$(PREFIX)/$(TARGET)
	rm -f $(DEST_DIR)$(MAN_PREFIX)/$(TARGET).1

dpkg: all
	mkdir -p $(TARGET)$(DEST_DIR)$(PREFIX)
	cp -f $(BIN_DIR)/$(TARGET) $(TARGET)$(DEST_DIR)$(PREFIX)/$(TARGET)
	chmod 755 $(TARGET)$(DEST_DIR)$(PREFIX)/$(TARGET)
	mkdir -p $(TARGET)$(DEST_DIR)$(MAN_PREFIX)
	cp -f $(MAN_DIR)/$(TARGET).1 $(TARGET)$(DEST_DIR)$(MAN_PREFIX)/$(TARGET).1
	chmod 644 $(TARGET)$(DEST_DIR)$(MAN_PREFIX)/$(TARGET).1
	mkdir -p $(TARGET)/DEBIAN
	@echo "$$DEBIAN_CONTROL" > $(TARGET)/DEBIAN/control
	dpkg-deb --build $(TARGET) $(TARGET).deb

