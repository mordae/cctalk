#!/usr/bin/make -f

tests = t-link

$(foreach t,${tests},$(eval ${t} = ../libcctalk.so ${t}.c cutest.h))
check += ${tests}

# EOF
