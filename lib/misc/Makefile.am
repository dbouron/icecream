include $(top_srcdir)/local.am

noinst_LTLIBRARIES = libmisc.la

libmisc_la_CFLAGS = -fPIC -DPIC
libmisc_la_CXXFLAGS = -fPIC -DPIC

libmisc_la_SOURCES = \
	md5.c \
	logging.cpp \
	tempfile.c \
	platform.cpp \
	gcc.cpp \
	exitcode.cpp

noinst_HEADERS = \
	platform-compatibility.h \
	platform-compatibility.hxx \
	ignore-result.h \
	ignore-result.hxx \
	md5.h \
	logging.h \
	tempfile.h \
	platform.h \
	gcc.h \
	exitcode.h

icedir = $(includedir)/icecc
ice_HEADERS = \
	logging.h
