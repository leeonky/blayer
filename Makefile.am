SUBDIRS = 

if !ENV_TEST
SUBDIRS += bputil pinf snubber wrpffp wrpsdl vdecode
endif

if !ENV_PRD
SUBDIRS += mock_ffmpeg mock_sys mock_sdl bputil_test snubber_test pinf_test wrpffp_test vdecode_test wrpsdl_test
endif

EXTRA_DIST = test.sh

AM_CFLAGS = -Wall -g -I$(top_srcdir)
