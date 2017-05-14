#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "snubber.h"
#include "pinf/pinf.h"

int snubber_main(int argc, char **argv, FILE *stdin, FILE *stdout, FILE *stderr) {
	return pinf_main(stdin, stdout, stderr);
}
