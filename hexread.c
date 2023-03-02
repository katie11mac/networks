/*
 * hexread.c
 */

#include "util.h"

#define READ_SIZE 16

void convert_input(FILE *file);

int main(int argc, char *argv[])
{
	int i;
	FILE *file;

	// No files provided
	if (argc == 1) {
		printf("No file provided. Please enter the hex you would like to be read and enter EOF when you are done:\n");
		convert_input(stdin);		
	
	// Files provided
	} else {
		for (i = 1; i < argc; i++) {
			
			if ((file = fopen(argv[i], "r")) == NULL) {
				perror("fopen");
			} else {
				convert_input(file);	
				if (fclose(file) == EOF) {
					perror("fclose");
				}
			}
			
		}
	}
}

void convert_input(FILE *file) {
	char input_buffer[READ_SIZE + 1];
	void *results;
	
	// BUG: memory is never cleared 
	// BUG: when two hex digits separated by many spaces are sent in separate calls
	while (fgets(input_buffer, READ_SIZE + 1, file) != NULL) {
		results = hex_to_binary(input_buffer);
		if (results == NULL) {
			// BUG: will convert and print binary until reaches invalid hex digit
			printf("**PART OF INPUT NOT VALID HEX DIGITS**");
			break;
		} else {
			printf("%s", (char *)results);
			// should probably memset here, but don't know appropriate size
			//free(results);
		}	
		printf("\n");
	}

