include $(top_srcdir)/local.am

bin_PROGRAMS = icecc
bin_SCRIPTS = icecc-create-env

noinst_LIBRARIES = libclient.a
libclient_a_SOURCES = \
        arg.cpp \
        cpp.cpp \
        local.cpp \
        remote.cpp \
        util.cpp \
        safeguard.cpp

icecc_SOURCES = \
	main.cpp
icecc_LDADD = \
	libclient.a \
	$(top_builddir)/services/libicecc.la \
	$(top_builddir)/lib/misc/libmisc.la \
	$(LIBRSYNC)

noinst_HEADERS = \
	client.h \
	util.h \
	remote.h \
	arg.h \
	cpp.h \
	local.h \
	safeguard.h

AM_CPPFLAGS += -DPLIBDIR=\"$(pkglibexecdir)\"

EXTRA_DIST = icecc-create-env

install-exec-local:
	$(mkinstalldirs) $(DESTDIR)$(bindir)
	rm -f $(DESTDIR)$(bindir)/icerun
	$(LN_S) $(bindir)/icecc $(DESTDIR)$(bindir)/icerun
	$(mkinstalldirs) $(DESTDIR)$(pkglibexecdir)
	rm -f $(DESTDIR)$(pkglibexecdir)/icecc-create-env
	$(LN_S) $(bindir)/icecc-create-env $(DESTDIR)$(pkglibexecdir)/icecc-create-env

	$(mkinstalldirs) $(DESTDIR)$(pkglibexecdir)/bin
	for link in g++ gcc c++ cc $(CLANG_SYMLINK_WRAPPERS); do \
		rm -f $(DESTDIR)$(pkglibexecdir)/bin/$$link ;\
		$(LN_S) $(bindir)/icecc $(DESTDIR)$(pkglibexecdir)/bin/$$link ;\
	done

uninstall-local:
	rm $(DESTDIR)$(bindir)/icerun
	rm $(DESTDIR)$(pkglibexecdir)/icecc-create-env

	for link in g++ gcc c++ cc $(CLANG_SYMLINK_WRAPPERS); do \
		rm $(DESTDIR)$(pkglibexecdir)/bin/$$link ;\
	done
