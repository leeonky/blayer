SUBDIRS = 

if !ENV_PRD
SUBDIRS += mock_ffmpeg bputil_test snubber_test pinf_test wrpffp_test vdecode_test
endif

if !ENV_TEST
SUBDIRS += bputil pinf snubber wrpffp vdecode
endif

EXTRA_DIST = test.sh

AM_CFLAGS = -Wall -g -I$(top_srcdir)
