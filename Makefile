# dmenu - dynamic menu
# See LICENSE file for copyright and license details.

include config.mk

INC_DIR := include
SRC_DIR := src
OBJ_DIR := obj

SRCS := $(wildcard $(SRC_DIR)/*.c)
DEPS := $(wildcard $(INC_DIR)/*.h)
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

all: options dmenu stest

options:
	@echo dmenu build options:
	@echo "CCFLAGS  = $(CCFLAGS)"
	@echo "LDFLAGS  = $(LDFLAGS)"
	@echo "CC       = $(CC)"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $< -c -o $@ $(CCFLAGS) $(LDFLAGS)

dmenu: $(OBJ_DIR)/dmenu.o $(OBJ_DIR)/drw.o $(OBJ_DIR)/util.o
	$(CC) $^ -o $@ $(CCFLAGS) $(LDFLAGS)

stest: $(OBJ_DIR)/stest.o
	$(CC) -o $@ $(OBJ_DIR)/stest.o $(LDFLAGS)

clean:
	rm -f dmenu $(OBJ_DIR)/*

dist: clean
	mkdir -p dmenu-$(VERSION)
	cp -r LICENSE Makefile README config.mk \
		$(SRC_DIR) $(INC_DIR) $(OBJ_DIR) res dmenu-$(VERSION)
	tar -cf dmenu-$(VERSION).tar dmenu-$(VERSION)
	gzip dmenu-$(VERSION).tar
	rm -rf dmenu-$(VERSION)

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f dmenu dmenu_path dmenu_run stest $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/dmenu
	chmod 755 $(DESTDIR)$(PREFIX)/bin/dmenu_path
	chmod 755 $(DESTDIR)$(PREFIX)/bin/dmenu_run
	chmod 755 $(DESTDIR)$(PREFIX)/bin/stest
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	sed "s/VERSION/$(VERSION)/g" < res/doc/dmenu.1 > $(DESTDIR)$(MANPREFIX)/man1/dmenu.1
	chmod 644 $(DESTDIR)$(MANPREFIX)/man1/dmenu.1

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/dmenu \
			$(DESTDIR)$(MANPREFIX)/bin/dmenu_path \
			$(DESTDIR)$(MANPREFIX)/bin/dmenu_run \
			$(DESTDIR)$(MANPREFIX)/bin/stest \
			$(DESTDIR)$(MANPREFIX)/man1/dmenu.1

.PHONY: all options clean dist install uninstall
