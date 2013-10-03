#!/usr/bin/make -f

lib += libcctalk.so.0

libcctalk.so.0 = -Wl,-h,libcctalk.so.0 util.c proto.c

# EOF
