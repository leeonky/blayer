noinst_PROGRAMS = libwrpffp.test
libwrpffp_test_SOURCES = main.c ../wrpffp/wrpffp.c ffmpeg_main_test.c ffmpeg_stream_test.c ffmpeg_decoder_test.c ffmpeg_frame_test.c ffmpeg_audio_decode_test.c ffmpeg_resample_test.c
libwrpffp_test_LDADD = -lcunit -lmock_ffmpeg -lcunitexd -lbputil -lmock_sys
libwrpffp_test_LDFLAGS = -L../mock_ffmpeg -L../bputil -L../mock_sys
