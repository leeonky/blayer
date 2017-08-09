SUBDIRS = 

if !ENV_TEST
SUBDIRS += sys bputil iob wrpffp wrpsdl vdecode vplayer vbuf
endif

if !ENV_PRD
SUBDIRS += mock_ffmpeg mock_sys mock_sdl sys_test bputil_test iob_test wrpffp_test vdecode_test wrpsdl_test vbuf_test
endif

EXTRA_DIST = test.sh

AM_CFLAGS = -Wall -g -I$(top_srcdir)
