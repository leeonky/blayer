SUBDIRS = 

if !ENV_PRD
SUBDIRS += snubber_test
endif

if !ENV_TEST
SUBDIRS += snubber
endif

EXTRA_DIST = test.h test.sh

AM_CFLAGS = -I$(top_srcdir)
