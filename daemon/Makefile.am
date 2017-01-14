include $(top_srcdir)/local.am

sbin_PROGRAMS = iceccd

iceccd_SOURCES = \
	ncpus.c \
	main.cpp \
	serve.cpp \
	workit.cpp \
	environment.cpp \
	load.cpp \
	file_util.cpp \
	daemon.cpp \
	client.cpp

iceccd_LDADD = \
	$(top_builddir)/services/libicecc.la \
	$(top_builddir)/lib/misc/libmisc.la \
	$(LIB_KINFO) \
	$(CAPNG_LDADD)

noinst_HEADERS = \
	environment.h \
	load.h \
	ncpus.h \
	serve.h \
	workit.h \
	file_util.h \
	daemon.h \
	client.h \
	native-environment.h
