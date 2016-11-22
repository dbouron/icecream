test: test-full

test-prepare:
	if test -x /sbin/setcap; then \
	    sudo /sbin/setcap cap_sys_chroot+ep ${sbindir}/iceccd ; \
	elif which filecap >/dev/null 2>/dev/null; then \
	    sudo filecap ${sbindir}/iceccd sys_chroot ; \
	else \
	    exit 1 ; \
	fi

test-full: test-prepare
	$(MAKE) test-run

test-run:
	results=`realpath -s ${builddir}/results` && builddir2=`realpath -s ${builddir}` && cd ${srcdir} && ./test.sh ${prefix} $$results --builddir=$$builddir2

# Automake's conditionals are dumb and adding 'test-run: clangplugin' would make it warn about
# being defined in two contexts, even though in this context it's harmless and intended.
test-run: @HAVE_CLANG_DEVEL_DEP@

clangplugin: ${builddir}/clangplugin.so

# It appears there's no way to force libtool to create a shared library if AC_DISABLE_SHARED is used
# in configure (and it is, as icecream uses static libs). So just do it manually. If this matters
# for anybody, feel free to do this in a better way.
${builddir}/clangplugin.so: clangplugin.cpp
	$(CXX) -shared -fPIC -o $@ $^ -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -fno-rtti

clean: clean-clangplugin

clean-clangplugin:
	rm -f ${builddir}/clangplugin.so

include $(top_srcdir)/local.am

TESTS = $(check_PROGRAMS)

check_PROGRAMS = \
	test-args \
	test-services \
	test-network \
	test-platform-compatibility

test_args_SOURCES = test-args.cpp
test_args_LDADD = \
	$(top_builddir)/client/libclient.a \
	$(top_builddir)/services/libicecc.la \
	$(LIBRSYNC) \
	$(GTEST_LDADD) \
	$(PTHREAD_LDADD)

test_services_SOURCES = test-services.cpp
test_services_LDADD = \
	$(top_builddir)/services/libicecc.la \
	$(GTEST_LDADD) \
	$(PTHREAD_LDADD)

test_network_SOURCES = test-network.cpp
test_network_LDADD = \
	$(top_builddir)/services/libicecc.la \
	$(GTEST_LDADD) \
	$(PTHREAD_LDADD)

test_platform_compatibility_SOURCES = test-platform-compatibility.cpp
test_platform_compatibility_LDADD = \
	$(GTEST_LDADD) \
	$(PTHREAD_LDADD)
