# Copyright © 2018-2021 Jakub Wilk <jwilk@jwilk.net>
# SPDX-License-Identifier: MIT

PREFIX = /usr/local
DESTDIR =

bindir = $(PREFIX)/bin

CFLAGS ?= -g -O2
CFLAGS += -Wall -Wextra
CFLAGS += -D_FILE_OFFSET_BITS=64

.PHONY: all
all: gentleshred

.PHONY: install
install: gentleshred
	install -d $(DESTDIR)$(bindir)
	install -m755 $(<) $(DESTDIR)$(bindir)/

.PHONY: clean
clean:
	rm gentleshred

# vim:ts=4 sts=4 sw=4 noet
