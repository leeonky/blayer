#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "snubber.h"

int snubber_main(int argc, char **argv, FILE *stdin, FILE *stdout, FILE *stderr) {
	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	while ((read = getline(&line, &len, stdin)) != -1 &&
			strstr(line, "EXIT") != line)
		fprintf(stdout, "%s", line);

	free(line);
	return 0;
}
