/*
 * hexdump.c 
 */

#include "util.h"

#define READ_SIZE 16

void convert_stdin(void);
void convert_file(FILE *file); 

int main(int argc, char *argv[])
{
	int i;
	FILE *file;

	// No files provided
	if (argc == 1) {
		printf("No file provided. Please enter the data you would like to be dumped and enter EOF when you are done:\n");
		convert_stdin();

	// Files provided	
	} else {
		for (i = 1; i < argc; i++) {
			if ((file = fopen(argv[i], "r")) == NULL) {
				perror("fopen");
			} else {
				printf("%s:\n", argv[i]);
				convert_file(file);
				if (fclose(file) == EOF) {
					perror("fclose");
				}
			}
		}
	}
}

/*
 * Read from stdin, convert input to hex, and print to screen
 */
void convert_stdin(void) {
	// +1 for the null byte
	char input_buffer[READ_SIZE + 1];
	char *results;
	
	// Read until EOF 
	while (fgets(input_buffer, READ_SIZE + 1, stdin) != NULL) {
		results = binary_to_hex(input_buffer, strlen(input_buffer));
		if (results != NULL) {
			printf("%s", results);
			free(results);
		}	
	}

	printf("\n"); 
}

/*
 * Read bytes from file, convert bytes to hex, and print to screen
 */
void convert_file(FILE *file) {
	char input_buffer[READ_SIZE];
	char *results;
	ssize_t bytes_read; 
	
	// Keep calling fread until it no longer fills the buffer entirely
	while ((bytes_read = fread(input_buffer, 1, READ_SIZE, file)) == READ_SIZE) {
		results = binary_to_hex(input_buffer, bytes_read);
		if (results != NULL) {
			printf("%s", results);
			free(results);
		}	
	}
	
	// Convert the last bytes read left in the buffer
	results = binary_to_hex(input_buffer, bytes_read);
	if (results != NULL) {
		printf("%s", results);
		free(results);	
	}
	
}
