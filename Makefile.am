SUBDIRS = 

if !ENV_PRD
SUBDIRS += snubber_test pinf_test vdecode_test
endif

if !ENV_TEST
SUBDIRS += pinf snubber vdecode
endif

EXTRA_DIST = test.h test.sh

AM_CFLAGS = -Wall -I$(top_srcdir)
