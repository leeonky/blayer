#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "iob.h"

int iob_add_handler(io_bus *iob, const iob_handler *handler) {
	iob->handlers[iob->handler_count++] = *handler;
}

static iob_handler *select_handler(io_bus *iob, const char *command) {
	iob_handler *handler = NULL;
	int i;
	for(i=0; i<iob->handler_count; ++i) {
		if(!strcmp(iob->handlers[i].command, command)) {
			handler = &iob->handlers[i];
			break;
		}
	}
	return handler;
}

static int process_command(io_bus *iob, io_stream *io_s) {
	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	while ((read = getline(&line, &len, io_s->stdin))!=-1 && strcmp(line, "EXIT\n")!=0) {
		char command[128];
		sscanf(line, "%s", command);

		const char *event_args = "";
		char *p = index(line, ' ');
		if(p) {
			event_args = p+1;
		}

		iob_handler *handler = select_handler(iob, command);
		if(handler) {
			handler->action(iob, command, event_args, handler->arg, io_s);
		} else {
			fprintf(io_s->stdout, "%s", line);
			fflush(io_s->stdout);
		}
	}

	free(line);
}

int iob_main(void *arg, int (*processer) (io_bus *, void *, io_stream *), io_stream *io_s) {
	io_bus iob = {};

	if(processer) {
		processer(&iob, arg, io_s);
	}
	process_command(&iob, io_s);
	return 0;
}

