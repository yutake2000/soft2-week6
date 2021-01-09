#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "encode.h"
#include "decode.h"

int main(int argc, char **argv)
{
	if (argc < 4) {
		fprintf(stderr, "usage: %s (compress|expand) <input filename> <output filename>\n",argv[0]);
		exit(1);
	}

	if (strcmp(argv[1], "compress") == 0) {
		char *input_filename = argv[2];
		char *output_filename = argv[3];
		FILE *fin = fopen(input_filename, "rb");
		if (fin == NULL) {
			fprintf(stderr, "error: cannot open '%s'\n", input_filename);
			exit(1);
		}

		FILE *fout = fopen(output_filename, "wb");
		if (fout == NULL) {
			fprintf(stderr, "error: cannot open '%s'\n", output_filename);
			exit(1);
		}

		encode(fin);
		rewind(fin);
		compress(fin, fout);

		fclose(fin);
		fclose(fout);
	} else if(strcmp(argv[1], "expand") == 0) {
		char *input_filename = argv[2];
		char *output_filename = argv[3];
		FILE *fin = fopen(input_filename, "rb");
		if (fin == NULL) {
			fprintf(stderr, "error: cannot open '%s'\n", input_filename);
			exit(1);
		}

		FILE *fout = fopen(output_filename, "wb");
		if (fout == NULL) {
			fprintf(stderr, "error: cannot open '%s'\n", output_filename);
			exit(1);
		}

		expand(fin, fout);
	}

	return EXIT_SUCCESS;
}
