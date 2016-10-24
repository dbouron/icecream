lib_LTLIBRARIES = libicecc.la
libicecc_la_SOURCES = \
	job.cpp \
	exitcode.cpp \
	getifaddrs.cpp \
	logging.cpp \
	tempfile.c \
	platform.cpp \
	gcc.cpp \
	discover-sched.cpp \
	ping.cpp \
	end.cpp \
	get-cs.cpp \
	use-cs.cpp \
	get-native-env.cpp \
	use-native-env.cpp \
	compile-file.cpp \
	file-chunk.cpp \
	compile-result.cpp \
	job-begin.cpp \
	job-done.cpp \
	job-local-begin.cpp \
	job-local-done.cpp \
	login.cpp \
	conf-cs.cpp \
	stats.cpp \
	env-transfer.cpp \
	get-internal-status.cpp \
	mon-login.cpp \
	mon-get-cs.cpp \
	mon-job-begin.cpp \
	mon-job-done.cpp \
	mon-local-job-begin.cpp \
	mon-stats.cpp \
	text.cpp \
	status-text.cpp \
	verify-env.cpp \
	verify-env-result.cpp \
	blacklist-host-env.cpp \
	network.cpp

libicecc_la_LIBADD = \
	$(LZO_LDADD) \
	$(CAPNG_LDADD) \
	-ldl

libicecc_la_CFLAGS = -fPIC -DPIC
libicecc_la_CXXFLAGS = -fPIC -DPIC

icedir = $(includedir)/icecc
ice_HEADERS = \
	job.h \
	job.hxx \
	comm.h \
	logging.h

noinst_HEADERS = \
	exitcode.h \
	getifaddrs.h \
	logging.h \
	tempfile.h \
	platform.h \
	msg.h \
	channel.h \
	service.h \
	discover-sched.h \
	ping.h \
	end.h \
	get-cs.h \
	use-cs.h \
	get-native-env.h \
	use-native-env.h \
	compile-file.h \
	file-chunk.h \
	compile-result.h \
	job-begin.h \
	job-done.h \
	job-local-begin.h \
	job-local-done.h \
	login.h \
	conf-cs.h \
	stats.h \
	env-transfer.h \
	get-internal-status.h \
	mon-login.h \
	mon-get-cs.h \
	mon-job-begin.h \
	mon-job-done.h \
	mon-local-job-begin.h \
	mon-stats.h \
	text.h \
	status-text.h \
	verify-env.h \
	verify-env-result.h \
	blacklist-host-env.h \
	network.h


pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = icecc.pc
