#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pinf.h"

int pinf_main(FILE *stdin, FILE *stdout, FILE *stderr) {
	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	while ((read = getline(&line, &len, stdin))!=-1 && strcmp(line, "EXIT\n")!=0) {
		fprintf(stdout, "%s", line);
	}

	free(line);
	return 0;
}

