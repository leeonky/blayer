noinst_PROGRAMS = libwrpsdl.test
libwrpsdl_test_SOURCES = main.c ../wrpsdl/wrpsdl.c sdl_window_test.c sdl_window_present.c sdl_init_audio_test.c sdl_reload_audio_test.c sdl_play_audio_test.c
libwrpsdl_test_LDADD = -lcunit -lmock_ffmpeg -lcunitexd -lmock_sdl -lbputil
libwrpsdl_test_LDFLAGS = -L../mock_ffmpeg -L../mock_sdl -L../bputil
