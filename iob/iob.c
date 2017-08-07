#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "iob.h"

int iob_add_handler(io_bus *iob, const iob_handler *handler) {
	if(iob->handler_count == sizeof(iob->handlers)/sizeof(iob_handler))
		return -1;
	iob->handlers[iob->handler_count++] = *handler;
	return 0;
}

static iob_handler *select_handler(io_bus *iob, const char *command) {
	int handler_count = iob->handler_count;
	iob_handler *handler;
	while(handler_count--)
		if(!strcmp((handler = &iob->handlers[handler_count])->command, command))
			return handler;
	return NULL;
}

static void parse_line(const char *line, const char **command, const char **event_args) {
	static char buf[128];

	buf[0] = '\0';
	if(1==sscanf(line, "%s", buf)) {
		*command = buf;
		char *p = index(line, ' ');
		if(p) {
			*event_args = p+1;
		} else {
			*event_args = "";
		}
	}
}

static void process_command(io_bus *iob, io_stream *io_s) {
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	int handler_count = iob->handler_count;

	while ((read = getline(&line, &len, io_s->stdin))!=-1 && strcmp(line, "EXIT\n")!=0) {
		const char *command, *event_args;
		parse_line(line, &command, &event_args);

		iob_handler *handler = select_handler(iob, command);
		if(handler && handler->action) {
			handler->action(iob, command, event_args, handler->arg, io_s);
		} else {
			fprintf(io_s->stdout, "%s", line);
			fflush(io_s->stdout);
		}
	}
	free(line);

	while(handler_count--)
		if(iob->handlers[handler_count].close)
			iob->handlers[handler_count].close(iob, iob->handlers[handler_count].arg, io_s);
}

int iob_main(void *arg, int (*processer) (io_bus *, void *, io_stream *), io_stream *io_s) {
	int res = 0;
	io_bus iob = {};

	if(processer) {
		res = processer(&iob, arg, io_s);
	}

	if(!res) {
		process_command(&iob, io_s);
	}
	return res;
}

