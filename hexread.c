/*
 * hexread.c
 */

#include "util.h"

#define BUFF_SIZE 2048

int main(int argc, char *argv[])
{
	int i;
	char input_buffer[BUFF_SIZE];
	void *results;
	FILE *file;

	// No files provided
	if (argc == 1) {
		printf("No file provided. Please enter the hex you would like to be read:\n");
		
		if (fgets(input_buffer, BUFF_SIZE, stdin) == NULL) {
			printf("An error occurred\n"); // SHOULD I CHANGE THIS? 
			exit(0);
		}

		printf("%s", input_buffer);
		//results = hex_to_binary(input_buffer);
		// WHAT DO WE DO WITH THE RESULTS? 
	
	// Files provided
	} else {
		for (i = 1; i < argc; i++) {
		//	printf("argument %d: %s", i, argv[i]);	
			
			if ((file = fopen(argv[i], "r")) == NULL) {
				perror("fopen");
			} else {


				// read contents of the file 
				// pass contents of file to function 
			
				if (fclose(file) == EOF) {
					perror("fclose");
				}
			}
			
			printf("%s:\n", argv[i]);
			// WHAT DO WE DO WITH THE RESULTS? 
		}
	}
}
