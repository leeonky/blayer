noinst_PROGRAMS = decoder.test
decoder_test_SOURCES = main.c ../decoder/decoder.c decoder_args_test.c
decoder_test_LDADD = -lcunit -lcunitexd -lmock_ffmpeg -lwrpffp -lmock_sys -lbputil
decoder_test_LDFLAGS = -L../mock_ffmpeg -L../wrpffp -L../mock_sys -L../bputil
