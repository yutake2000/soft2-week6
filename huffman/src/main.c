#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "encode.h"
#include "decode.h"

int main(int argc, char **argv)
{
	if (argc != 4) {
		fprintf(stderr, "usage: %s (compress|expand) <input filename> <output filename>\n",argv[0]);
		exit(1);
	}

	if (strcmp(argv[1], "compress") == 0) {
		encode(argv[2], argv[3]);
	} else if(strcmp(argv[1], "expand") == 0) {
		expand(argv[2], argv[3]);
	}

	return EXIT_SUCCESS;
}
