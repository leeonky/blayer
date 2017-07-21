#include "bputil/bputil.h"
#include "vplayer.h"
#include "wrpsdl/wrpsdl.h"

static int w, h, fmt;
static int lines[4];
static int64_t pts;
static int cid, cindex, align, size;

static int process_frame(shm_cbuf *cb, void *arg, io_stream *io_s) {
	printf("%d\n", cindex);
}

static int process_video(sdl_window *window, void *arg, io_stream *io_s) {
	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	while ((read = getline(&line, &len, stdin))!=-1 && strcmp(line, "EXIT\n")!=0) {
		sscanf(line, "video_frame:: width:%d height:%d format:%d pts:%lld align:%d cbuf:%d index:%d size:%d\n", &w, &h, &fmt, &pts, &align, &cid, &cindex, &size);
		shrb_load(cid, 4, size, NULL, process_frame, io_s);
	}
}

int vplayer_main(int argc, char **argv, FILE *stdin, FILE *stdout, FILE *stderr) {
	io_stream io_s = {stdin, stdout, stderr};
	return sdl_open_window("test", 0, 0, 800, 600, 0, NULL, process_video, &io_s);
}

