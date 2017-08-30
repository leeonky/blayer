#include <stdlib.h>
#include <stdio.h>
#include "decoder.h"

int main(int argc, char ** argv) {
	return decoder_main(argc, argv, stdin, stdout, stderr);
}

