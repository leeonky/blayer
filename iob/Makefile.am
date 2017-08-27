lib_LTLIBRARIES = libiob.la
libiob_la_SOURCES = iob.c vfs.c afs.c
include_HEADERS = iob.h vfs.h afs.h
libiob_la_LIBADD = -lsys -lbputil
libiob_la_LDFLAGS = -shared -fPIC -L../bputil -L../sys
