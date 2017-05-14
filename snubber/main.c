#include <stdlib.h>
#include <stdio.h>
#include "snubber.h"

int main(int argc, char ** argv) {
	return snubber_main(argc, argv, stdin, stdout, stderr);
}
