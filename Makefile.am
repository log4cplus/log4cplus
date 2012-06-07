ACLOCAL_AMFLAGS = -I m4
EXTRA_DIST = ChangeLog log4cplus.pc.in
SUBDIRS = include src loggingserver tests
if QT
SUBDIRS += qt4debugappender
endif
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = log4cplus.pc
