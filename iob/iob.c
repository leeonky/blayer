#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "iob.h"

int iob_main(void *arg, int (*processer) (io_bus *, void *, io_stream *), io_stream *io_s) {
	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	while ((read = getline(&line, &len, io_s->stdin))!=-1 && strcmp(line, "EXIT\n")!=0) {
		fprintf(io_s->stdout, "%s", line);
		fflush(io_s->stdout);
	}

	free(line);
	return 0;
}

