#include "mock_ffmpeg.h"

mock_void_function_0(av_register_all);
mock_function_4(int, avformat_open_input, AVFormatContext **, const char *, AVInputFormat *, AVDictionary **);
mock_function_2(int, avformat_find_stream_info, AVFormatContext *, AVDictionary **);
mock_void_function_1(avformat_close_input, AVFormatContext **);
