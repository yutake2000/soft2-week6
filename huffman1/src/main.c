#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "encode.h"
#include "decode.h"

char *expanded_dir = "expanded/";

int main(int argc, char **argv)
{
	if (argc < 3) {
		fprintf(stderr, "usage: %s (compress <output filename> <input filename...> | expand <input filename>) \n",argv[0]);
		exit(1);
	}

	if (strcmp(argv[1], "compress") == 0) {
		char *output_filename = argv[2];
		FILE *fout = fopen(output_filename, "wb");
		if (fout == NULL) {
			fprintf(stderr, "error: cannot open '%s'\n", output_filename);
			exit(1);
		}

		int nfiles = argc - 3;
		fwrite(&nfiles, sizeof(int), 1, fout);

		for (int i=3; i<argc; i++) {
			char *input_filename = argv[i];
			FILE *fin = fopen(input_filename, "rb");
			if (fin == NULL) {
				fprintf(stderr, "error: cannot open '%s'\n", input_filename);
				exit(1);
			}

			int filename_len = strlen(input_filename) + 1;
			fwrite(&filename_len, sizeof(int), 1, fout);
			fwrite(input_filename, sizeof(char), filename_len, fout);
			
			encode(fin);
			rewind(fin);
			compress(fin, fout);
			fclose(fin);

		}
		
		fclose(fout);
	} else if(strcmp(argv[1], "expand") == 0) {
		mkdir(expanded_dir, 0777);
		char *input_filename = argv[2];
		FILE *fin = fopen(input_filename, "rb");
		if (fin == NULL) {
			fprintf(stderr, "error: cannot open '%s'\n", input_filename);
			exit(1);
		}

		expand(fin);
	}

	return EXIT_SUCCESS;
}
