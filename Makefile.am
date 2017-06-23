SUBDIRS = 

if !ENV_PRD
SUBDIRS += mock_ffmpeg snubber_test pinf_test wrpffp_test vdecode_test
endif

if !ENV_TEST
SUBDIRS += pinf snubber wrpffp vdecode
endif

EXTRA_DIST = test.sh

AM_CFLAGS = -Wall -g -I$(top_srcdir)
