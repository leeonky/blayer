#include <stdio.h>
#include <stdlib.h>
#include <cunitexd.h>
#include "mock_ffmpeg/mock_ffmpeg.h"
#include "wrpffp/wrpffp.h"

SUITE_START("ffmpeg_resample_init_test");

static io_stream arg_io_s;
static ffmpeg_resampler arg_res;
static void *arg_arg;
static struct SwrContext *arg_swr_context;
static void *ret_buffer;

mock_function_3(int, resample_init_action, ffmpeg_resampler *, void *, io_stream *);

BEFORE_EACH() {
	init_subject("");
	arg_io_s.stdin = actxt.input_stream;
	arg_io_s.stdout = actxt.output_stream;
	arg_io_s.stderr = actxt.error_stream;

	arg_swr_context = (SwrContext *)&arg_swr_context;

	arg_arg = &arg_arg;

	ret_buffer = &ret_buffer;

	init_mock_function(swr_free, NULL);
	init_mock_function(resample_init_action, NULL);
	init_mock_function(av_free, NULL);
	return 0;
}

AFTER_EACH() {
	return close_subject();
}

SUBJECT(int) {
	return ffmpeg_init_resampler(arg_arg, resample_init_action, &arg_io_s);
}

SUITE_CASE("init resample and exit with out open and free") {
	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_EXPECT_CALLED_ONCE(resample_init_action);
	CUE_EXPECT_CALLED_WITH_PTR(resample_init_action, 2, arg_arg);
	CUE_EXPECT_CALLED_WITH_PTR(resample_init_action, 3, &arg_io_s);

	CUE_EXPECT_NEVER_CALLED(swr_free);
}

static int resample_init_action_open_resample(ffmpeg_resampler *rsp, void *arg, io_stream *io_s) {
	rsp->swr_context = arg_swr_context;
	rsp->buffer = ret_buffer;
	return 100;
}

static void swr_free_assert(SwrContext ** context) {
	CUE_ASSERT_PTR_EQ(*context, &arg_swr_context);
}

SUITE_CASE("should free swr when have opend swr") {
	init_mock_function(resample_init_action, resample_init_action_open_resample);
	init_mock_function(swr_free, swr_free_assert);

	CUE_ASSERT_SUBJECT_FAILED_WITH(100);

	CUE_EXPECT_CALLED_ONCE(swr_free);

	CUE_EXPECT_CALLED_ONCE(av_free);
	CUE_EXPECT_CALLED_WITH_PTR(av_free, 1, ret_buffer);
}

SUITE_END(ffmpeg_resample_init_test);

SUITE_START("ffmpeg_resample_reload_test");
static int arg_sample_rate, ret_out_channels, ret_in_channels, ret_out_buffer_size, ret_in_buffer_size, arg_align;
static uint64_t arg_in_channels_layout, arg_out_channels_layout;
static enum AVSampleFormat arg_in_format, arg_out_format;
static ffmpeg_resampler arg_resampler;
static audio_frames arg_in_afs, arg_out_afs;

static struct SwrContext *stub_swr_alloc_set_opts(struct SwrContext *s, int64_t out_ch_layout, enum AVSampleFormat out_sample_fmt, int out_sample_rate, int64_t in_ch_layout, enum AVSampleFormat in_sample_fmt, int in_sample_rate, int log_offset, void *log_ctx) {
	return arg_swr_context;
}

static int stub_av_get_channel_layout_nb_channels(uint64_t layout) {
	if(layout == arg_out_channels_layout)
		return ret_out_channels;
	return ret_in_channels;
}

static int stub_av_samples_get_buffer_size(int *lines, int channels, int samples, enum AVSampleFormat format, int align) {
	if(channels == ret_out_channels)
		return ret_out_buffer_size;
	return ret_in_buffer_size;
}

mock_function_3(int, reload_resampler_action, ffmpeg_resampler *, void *, io_stream *);

static void *stub_av_malloc(size_t s) {
	return ret_buffer;
}

BEFORE_EACH() {
	init_subject("");
	arg_io_s.stdin = actxt.input_stream;
	arg_io_s.stdout = actxt.output_stream;
	arg_io_s.stderr = actxt.error_stream;

	arg_sample_rate = 48000;
	arg_in_channels_layout = AV_CH_LAYOUT_5POINT1;
	arg_in_format = AV_SAMPLE_FMT_U8;
	ret_in_channels = 3;
	arg_align = 1;

	arg_out_channels_layout = AV_CH_LAYOUT_4POINT0;
	arg_out_format = AV_SAMPLE_FMT_S16;
	ret_out_channels = 5;

	ret_buffer = &ret_buffer;

	ret_out_buffer_size = 1024;
	ret_in_buffer_size = 1024;

	init_mock_function(swr_alloc_set_opts, stub_swr_alloc_set_opts);
	init_mock_function(swr_init, NULL);
	init_mock_function(swr_free, NULL);
	init_mock_function(reload_resampler_action, NULL);
	init_mock_function(av_get_channel_layout_nb_channels, stub_av_get_channel_layout_nb_channels);
	init_mock_function(av_malloc, stub_av_malloc);
	init_mock_function(av_free, NULL);
	init_mock_function(av_samples_get_buffer_size, stub_av_samples_get_buffer_size);

	arg_arg = &arg_arg;

	return 0;
}

AFTER_EACH() {
	return close_subject();
}

SUBJECT(int) {
	return ffmpeg_reload_resampler(&arg_resampler, &arg_in_afs, arg_out_channels_layout, arg_out_format, &arg_out_afs, arg_arg, reload_resampler_action, &arg_io_s);
}

SUITE_CASE("first reload with params") {
	arg_in_afs.sample_rate = arg_sample_rate;
	arg_in_afs.layout = arg_in_channels_layout;
	arg_in_afs.format = arg_in_format;
	arg_in_afs.count = 128;
	arg_in_afs.buffer_samples = 20;
	arg_in_afs.align = arg_align;
	arg_resampler.swr_context = NULL;

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_EXPECT_CALLED_ONCE(swr_alloc_set_opts);
	CUE_EXPECT_CALLED_WITH_PTR(swr_alloc_set_opts, 1, NULL);
	CUE_EXPECT_CALLED_WITH_INT(swr_alloc_set_opts, 2, arg_out_channels_layout);
	CUE_EXPECT_CALLED_WITH_INT(swr_alloc_set_opts, 3, arg_out_format);
	CUE_EXPECT_CALLED_WITH_INT(swr_alloc_set_opts, 4, arg_sample_rate);
	CUE_EXPECT_CALLED_WITH_INT(swr_alloc_set_opts, 5, arg_in_channels_layout);
	CUE_EXPECT_CALLED_WITH_INT(swr_alloc_set_opts, 6, arg_in_format);
	CUE_EXPECT_CALLED_WITH_INT(swr_alloc_set_opts, 7, arg_sample_rate);
	CUE_EXPECT_CALLED_WITH_INT(swr_alloc_set_opts, 8, 0);
	CUE_EXPECT_CALLED_WITH_PTR(swr_alloc_set_opts, 8, NULL);

	CUE_EXPECT_CALLED_ONCE(swr_init);
	CUE_EXPECT_CALLED_WITH_PTR(swr_init, 1, arg_swr_context);

	CUE_EXPECT_CALLED_ONCE(reload_resampler_action);
	CUE_EXPECT_CALLED_WITH_PTR(reload_resampler_action, 1, &arg_resampler);
	CUE_EXPECT_CALLED_WITH_PTR(reload_resampler_action, 2, arg_arg);
	CUE_EXPECT_CALLED_WITH_PTR(reload_resampler_action, 3, &arg_io_s);

	CUE_ASSERT_EQ(arg_out_afs.sample_rate, arg_sample_rate);
	CUE_ASSERT_EQ(arg_out_afs.layout, arg_out_channels_layout);
	CUE_ASSERT_EQ(arg_out_afs.channels, ret_out_channels);
	CUE_ASSERT_EQ(arg_out_afs.format, arg_out_format);
	CUE_ASSERT_EQ(arg_out_afs.count, 128);
	CUE_ASSERT_EQ(arg_out_afs.buffer_samples, arg_in_afs.buffer_samples);

	CUE_EXPECT_CALLED_ONCE(av_malloc);
	CUE_EXPECT_CALLED_WITH_INT(av_malloc, 1, ret_out_buffer_size);

	CUE_EXPECT_CALLED_ONCE(av_samples_get_buffer_size);
	CUE_EXPECT_CALLED_WITH_PTR(av_samples_get_buffer_size, 1, NULL);
	CUE_EXPECT_CALLED_WITH_INT(av_samples_get_buffer_size, 2, ret_out_channels);
	CUE_EXPECT_CALLED_WITH_INT(av_samples_get_buffer_size, 3, arg_in_afs.buffer_samples);
	CUE_EXPECT_CALLED_WITH_INT(av_samples_get_buffer_size, 4, arg_out_format);
	CUE_EXPECT_CALLED_WITH_INT(av_samples_get_buffer_size, 5, arg_in_afs.align);

	CUE_ASSERT_EQ(arg_resampler.sample_rate, arg_sample_rate);
	CUE_ASSERT_EQ(arg_resampler.align, arg_align);
	CUE_ASSERT_EQ(arg_resampler.buffer_size, ret_out_buffer_size);
	CUE_ASSERT_EQ(arg_resampler.out_layout, arg_out_channels_layout);
	CUE_ASSERT_EQ(arg_resampler.out_format, arg_out_format);
	CUE_ASSERT_EQ(arg_resampler.in_layout, arg_in_channels_layout);
	CUE_ASSERT_EQ(arg_resampler.in_format, arg_in_format);
	CUE_ASSERT_PTR_EQ(arg_resampler.buffer, ret_buffer);
}

static struct SwrContext *stub_swr_alloc_set_opts_failed(struct SwrContext *s, int64_t out_ch_layout, enum AVSampleFormat out_sample_fmt, int out_sample_rate, int64_t in_ch_layout, enum AVSampleFormat in_sample_fmt, int in_sample_rate, int log_offset, void *log_ctx) {
	return NULL;
}

SUITE_CASE("failed to alloc context") {
	arg_resampler.swr_context = NULL;
	init_mock_function(swr_alloc_set_opts, stub_swr_alloc_set_opts_failed);

	CUE_ASSERT_SUBJECT_FAILED_WITH(-1);

	CUE_EXPECT_NEVER_CALLED(swr_init);

	CUE_EXPECT_NEVER_CALLED(av_malloc);

	CUE_EXPECT_NEVER_CALLED(reload_resampler_action);

	CUE_EXPECT_NEVER_CALLED(swr_free);

	CUE_ASSERT_STDERR_EQ("Error[libwrpffp]: failed to alloc SwrContext\n");
}

static int stub_swr_init_failed(SwrContext *s) {
	return -1;
}

SUITE_CASE("failed to init context") {
	arg_resampler.swr_context = NULL;
	init_mock_function(swr_init, stub_swr_init_failed);
	init_mock_function(swr_free, swr_free_assert);

	CUE_ASSERT_SUBJECT_FAILED_WITH(-1);

	CUE_EXPECT_NEVER_CALLED(av_malloc);

	CUE_EXPECT_NEVER_CALLED(reload_resampler_action);

	CUE_EXPECT_CALLED_ONCE(swr_free);

	CUE_ASSERT_STDERR_EQ("Error[libwrpffp]: -1\n");
}

SUITE_CASE("failed to av alloc") {
	arg_resampler.swr_context = NULL;
	init_mock_function(av_malloc, NULL);

	CUE_ASSERT_SUBJECT_FAILED_WITH(-1);

	CUE_EXPECT_NEVER_CALLED(reload_resampler_action);

	CUE_EXPECT_CALLED_ONCE(swr_free);

	CUE_ASSERT_STDERR_EQ("Error[libwrpffp]: failed to alloc buffer\n");
}

SUITE_CASE("no need to alloc buffer") {
}

SUITE_CASE("reload same convertion with last params, should not close and open") {
	arg_resampler.swr_context = arg_swr_context;

	arg_resampler.sample_rate = arg_in_afs.sample_rate = arg_sample_rate;
	arg_resampler.in_layout = arg_in_afs.layout = arg_in_channels_layout;
	arg_resampler.in_format = arg_in_afs.format = arg_in_format;
	arg_resampler.out_layout = arg_out_channels_layout;
	arg_resampler.out_format = arg_out_format;

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_EXPECT_NEVER_CALLED(swr_alloc_set_opts);

	CUE_EXPECT_CALLED_ONCE(reload_resampler_action);
	CUE_EXPECT_CALLED_WITH_PTR(reload_resampler_action, 1, &arg_resampler);
	CUE_EXPECT_CALLED_WITH_PTR(reload_resampler_action, 2, arg_arg);
	CUE_EXPECT_CALLED_WITH_PTR(reload_resampler_action, 3, &arg_io_s);
}

SUITE_CASE("load diff convertion with last params") {
	arg_resampler.swr_context = arg_swr_context;
	arg_resampler.out_layout = 0;
	arg_resampler.out_format = 0;

	arg_in_afs.sample_rate = arg_sample_rate;
	arg_in_afs.layout = arg_in_channels_layout;
	arg_in_afs.format = arg_in_format;
	arg_in_afs.count = 128;
	init_mock_function(swr_free, swr_free_assert);

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_EXPECT_CALLED_ONCE(swr_alloc_set_opts);
	CUE_EXPECT_CALLED_WITH_PTR(swr_alloc_set_opts, 1, NULL);
	CUE_EXPECT_CALLED_WITH_INT(swr_alloc_set_opts, 2, arg_out_channels_layout);
	CUE_EXPECT_CALLED_WITH_INT(swr_alloc_set_opts, 3, arg_out_format);
	CUE_EXPECT_CALLED_WITH_INT(swr_alloc_set_opts, 4, arg_sample_rate);
	CUE_EXPECT_CALLED_WITH_INT(swr_alloc_set_opts, 5, arg_in_channels_layout);
	CUE_EXPECT_CALLED_WITH_INT(swr_alloc_set_opts, 6, arg_in_format);
	CUE_EXPECT_CALLED_WITH_INT(swr_alloc_set_opts, 7, arg_sample_rate);
	CUE_EXPECT_CALLED_WITH_INT(swr_alloc_set_opts, 8, 0);
	CUE_EXPECT_CALLED_WITH_PTR(swr_alloc_set_opts, 8, NULL);

	CUE_EXPECT_CALLED_ONCE(swr_init);
	CUE_EXPECT_CALLED_WITH_PTR(swr_init, 1, arg_swr_context);

	CUE_EXPECT_CALLED_ONCE(reload_resampler_action);
	CUE_EXPECT_CALLED_WITH_PTR(reload_resampler_action, 1, &arg_resampler);
	CUE_EXPECT_CALLED_WITH_PTR(reload_resampler_action, 2, arg_arg);
	CUE_EXPECT_CALLED_WITH_PTR(reload_resampler_action, 3, &arg_io_s);

	CUE_ASSERT_EQ(arg_out_afs.sample_rate, arg_sample_rate);
	CUE_ASSERT_EQ(arg_out_afs.layout, arg_out_channels_layout);
	CUE_ASSERT_EQ(arg_out_afs.channels, ret_out_channels);
	CUE_ASSERT_EQ(arg_out_afs.format, arg_out_format);
	CUE_ASSERT_EQ(arg_out_afs.count, 128);

	CUE_ASSERT_EQ(arg_resampler.sample_rate, arg_sample_rate);
	CUE_ASSERT_EQ(arg_resampler.out_layout, arg_out_channels_layout);
	CUE_ASSERT_EQ(arg_resampler.out_format, arg_out_format);
	CUE_ASSERT_EQ(arg_resampler.in_layout, arg_in_channels_layout);
	CUE_ASSERT_EQ(arg_resampler.in_format, arg_in_format);
}

SUITE_CASE("in and out format is the same") {
	arg_resampler.swr_context = NULL;

	arg_out_channels_layout = arg_in_channels_layout;
	arg_out_format = arg_in_format;
	arg_in_afs.sample_rate = arg_sample_rate;
	arg_in_afs.layout = arg_in_channels_layout;
	arg_in_afs.format = arg_in_format;

	CUE_ASSERT_SUBJECT_SUCCEEDED();

	CUE_EXPECT_NEVER_CALLED(swr_alloc_set_opts);

	CUE_EXPECT_NEVER_CALLED(swr_init);

	CUE_EXPECT_NEVER_CALLED(swr_free);

	CUE_EXPECT_CALLED_ONCE(reload_resampler_action);
	CUE_EXPECT_CALLED_WITH_PTR(reload_resampler_action, 1, &arg_resampler);
	CUE_EXPECT_CALLED_WITH_PTR(reload_resampler_action, 2, arg_arg);
	CUE_EXPECT_CALLED_WITH_PTR(reload_resampler_action, 3, &arg_io_s);
}

SUITE_END(ffmpeg_resample_reload_test);

SUITE_START("ffmpeg_resample_test");

static void *in_buffer, *out_buffer;

mock_function_3(int, resample_aciton, ffmpeg_resampler *, void *, io_stream *);

SUBJECT(int) {
	/*return ffmpeg_resample(&arg_resampler, in_buffer, out_buffer, arg_arg, resample_aciton, &arg_io_s);*/
}

SUITE_CASE("resampled: convert and copy") {
}

SUITE_CASE("same format resample, no need to do anything") {
}

SUITE_END(ffmpeg_resample_test);
