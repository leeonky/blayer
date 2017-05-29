#ifndef MOCK_FFMPEG_
#define MOCK_FFMPEG_

#include <libavformat/avformat.h>
#include <cunitexd.h>

extern_mock_void_function_0(av_register_all);
extern_mock_function_4(int, avformat_open_input, AVFormatContext **, const char *, AVInputFormat *, AVDictionary **);
extern_mock_function_2(int, avformat_find_stream_info, AVFormatContext *, AVDictionary **);
extern_mock_void_function_1(avformat_close_input, AVFormatContext **);

#endif
