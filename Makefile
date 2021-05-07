# Copyright © 2018-2021 Jakub Wilk <jwilk@jwilk.net>
# SPDX-License-Identifier: MIT

CFLAGS ?= -g -O2
CFLAGS += -Wall -Wextra

.PHONY: all
all: gentleshred

.PHONY: clean
clean:
	rm gentleshred

# vim:ts=4 sts=4 sw=4 noet
