#!/usr/bin/make -f

all:

include config.mk

pwd = $(shell pwd)
dirs = $(patsubst ${pwd}/%,%,$(abspath $(dir $(shell find * -name rules.mk))))

LD_LIBRARY_PATH := $(shell echo $(addprefix ${objdir},${dirs}) | sed 's/ /:/g'):${LD_LIBRARY_PATH}
export LD_LIBRARY_PATH

.SUFFIXES:

$(foreach bd,${dirs}, \
	$(eval dir = ${bd}/) \
	$(eval bdir = ${objdir}${dir}) \
	$(eval bin =) \
	$(eval sbin =) \
	$(eval check =) \
	$(eval lib =) \
	$(eval ar =) \
	$(eval inc =) \
	$(eval include ${dir}rules.mk) \
	$(eval all_bin += $(addprefix ${bdir},${bin})) \
	$(eval all_sbin += $(addprefix ${bdir},${sbin})) \
	$(eval all_check += $(addprefix ${bdir},${check})) \
	$(eval all_lib += $(addprefix ${bdir},${lib})) \
	$(eval all_ar += $(addprefix ${bdir},${ar})) \
	$(eval all_inc += $(addprefix ${dir},${inc})) \
	$(foreach i,${inc}, \
		$(eval $(addprefix ${dir},${i})-name = ${i}) \
	) \
	$(foreach t,${bin} ${sbin} ${check} ${lib} ${ar}, \
		$(eval ${bdir}${t}-src = $(patsubst ${pwd}/%,%,$(abspath $(addprefix ${dir},$(filter-out -%,$(${t})))))) \
		$(eval ${bdir}${t}-flags = $(patsubst ${pwd}/%,%,$(filter -%,$(${t})))) \
		$(eval objs = $(filter %.l,$(${bdir}${t}-src))) \
		$(eval PRECIOUS += $(addprefix ${objdir},$(objs:.l=.c))) \
		$(eval objs += $(filter %.c,$(${bdir}${t}-src))) \
		$(eval objs += $(filter %.cpp,$(${bdir}${t}-src))) \
		$(eval objs = $(objs:.c=.o)) \
		$(eval objs = $(objs:.cpp=.o)) \
		$(eval objs = $(objs:.l=.o)) \
		$(eval ${bdir}${t}-obj = $(addprefix ${objdir},${objs})) \
		$(eval libs = $(filter %.so,$(${bdir}${t}-src))) \
		$(eval ars = $(filter %.a,$(${bdir}${t}-src))) \
		$(eval ${bdir}${t}-deps += $(addprefix ${objdir},${libs})) \
		$(eval ${bdir}${t}-deps += $(addprefix ${objdir},${ars})) \
		$(eval ${bdir}${t}-deps += $(${bdir}${t}-obj)) \
		$(eval ${bdir}${t}-libs = $(foreach l,${libs},-L${objdir}$(dir ${l}) $(patsubst lib%.so,-l%,$(notdir ${l})))) \
		$(eval ${bdir}${t}-ars = $(addprefix ${objdir}, ${ars})) \
	) \
)

$(foreach l,${all_lib},\
	$(eval ${l}: $(${l}-obj) $(${l}-deps)) \
	$(eval liblink = $(basename ${l})) \
	$(eval all_liblink += ${liblink}) \
	$(eval ${liblink}: ${l}) \
)

$(foreach b,${all_bin} ${all_sbin} ${all_check} ${all_ar},\
	$(eval ${b}: $(${b}-obj) $(${b}-deps)) \
)

.PRECIOUS: ${PRECIOUS}

ifeq (${V},)
 q = @
else
 q =
endif

${all_liblink}:
	@echo " LN    $@ -> $(notdir $<)"
	${q}ln -sf $(notdir $<) $@

${all_lib}:
	@echo " LD    $@ $($@-libs) $($@-ars)"
	@mkdir -p $(dir $@)
	${q}${cc} -shared -o $@ $($@-obj) $($@-libs) $($@-ars) $($@-flags) ${cflags} ${ldflags}

${all_bin} ${all_sbin} ${all_check}:
	@echo " LD    $@ $($@-libs) $($@-ars)"
	@mkdir -p $(dir $@)
	${q}${cc} -o $@ $($@-obj) $($@-libs) $($@-ars) $($@-flags) ${cflags} ${ldflags}

${all_ar}:
	@echo " AR    $@"
	@mkdir -p $(dir $@)
	${q}rm -f $@
	${q}ar crs $@ $($@-obj)

${objdir}%.o: %.c ${all_inc}
	@echo " CC    $<"
	@mkdir -p $(dir $@)
	${q}${cc} -c -o $@ $< ${cppflags} ${cflags}

${objdir}%.o: %.cpp ${all_inc}
	@echo " C++   $<"
	@mkdir -p $(dir $@)
	${q}${cxx} -c -o $@ $< ${cppflags} ${cxxflags}

${objdir}%.o: ${objdir}%.c ${all_inc}
	@echo " CC    $<"
	@mkdir -p $(dir $@)
	${q}${cc} -c -o $@ $< ${cppflags} ${cflags}

${objdir}%.o: ${objdir}%.cpp ${all_inc}
	@echo " C++   $<"
	@mkdir -p $(dir $@)
	${q}${cxx} -c -o $@ $< ${cppflags} ${cxxflags}

${objdir}%.c: %.l
	@echo " RE2C  $<"
	@mkdir -p $(dir $@)
	${q}${re2c} -o $@ $<

all: ${all_lib} ${all_liblink} ${all_bin} ${all_sbin} ${all_ar}

test: check
check: ${all_check}
	@res=0; \
	for c in ${all_check}; do \
		${valgrind} $$c || res=$$?; \
	done; \
	exit $$res

clean:
	@echo " RM    ${objdir}"
	${q}${RM} -R ${objdir}

install:
	@$(foreach i,${all_bin}, \
		echo " INST  ${i} -> ${bindir}/$(notdir ${i})"; \
		install -m755 -D "${i}" "${DESTDIR}${bindir}/$(notdir ${i})"; \
	)
	@$(foreach i,${all_sbin}, \
		echo " INST  ${i} -> ${sbindir}/$(notdir ${i})"; \
		install -m755 -D "${i}" "${DESTDIR}${sbindir}/$(notdir ${i})"; \
	)
	@$(foreach i,${all_lib}, \
		echo " INST  ${i} -> ${libdir}/$(notdir ${i})"; \
		install -m755 -D "${i}" "${DESTDIR}${libdir}/$(notdir ${i})"; \
	)
	@$(foreach i,${all_ar}, \
		echo " INST  ${i} -> ${libdir}/$(notdir ${i})"; \
		install -m644 -D "${i}" "${DESTDIR}${libdir}/$(notdir ${i})"; \
	)
	@$(foreach i,${all_liblink}, \
		echo " LN    ${libdir}/$(notdir ${i}) -> $(shell readlink ${i})"; \
		cp -d "${i}" "${DESTDIR}${libdir}/$(notdir ${i})"; \
	)
	@$(foreach i,${all_inc}, \
		echo " INST  ${i} -> ${includedir}/$(${i}-name)"; \
		install -m644 -D "${i}" "${DESTDIR}${includedir}/$(${i}-name)"; \
	)

# EOF
