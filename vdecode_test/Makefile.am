noinst_PROGRAMS = vdecode.test
vdecode_test_SOURCES = main.c ../vdecode/vdecode.c vdecode_main_test.c vdecode_args_test.c
vdecode_test_LDADD = -lcunit -lcunitexd -lmock_ffmpeg -lwrpffp -lmock_sys -lbputil
vdecode_test_LDFLAGS = -L../mock_ffmpeg -L../wrpffp -L../mock_sys -L../bputil
