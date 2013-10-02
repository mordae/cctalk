#!/usr/bin/make -f

# Project version.
version = 0.1.0

# Where to put build products;
objdir = build/

# Current architecture;
arch = $(shell uname -m)

CC ?= gcc
CXX ?= g++
cc = ${CC}
cxx = ${CXX}

cppflags = -Wall -W -Werror -Wno-unused-parameter\
	   -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -D__STDC_ISO_10646__=200104L \
	   -Iinclude -DVERSION='"${version}"' \
	   -D_FORTIFY_SOURCE=2 ${CPPFLAGS}
cflags = -std=gnu99 -fPIC -O2 -g -fexceptions ${CFLAGS}
cxxflags = -fPIC -O2 -g -fexceptions ${CXXFLAGS}
ldflags = -Wl,--warn-shared-textrel,--fatal-warnings ${LDFLAGS}

valgrind = valgrind -q --tool=memcheck --leak-check=full --track-origins=yes

ifneq ($(wildcard arch/${arch}.mk),)
 $(info Loading arch/${arch}.mk)
 include arch/${arch}.mk
endif

lib64 = $(shell if uname -m | grep -q 64; then echo lib64; else echo lib; fi)

prefix = /usr
bindir = ${prefix}/bin
sbindir = ${prefix}/sbin
libdir = ${prefix}/${lib64}
includedir = ${prefix}/include

include $(wildcard deps/*.mk)
-include local.mk

# EOF
