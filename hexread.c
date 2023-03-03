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
				printf("%s:\n", argv[i]);
				convert_input(file);	
				if (fclose(file) == EOF) {
					perror("fclose");
				}
			}
			
		}
	}
}

/*
 * Reads hex strings from file and prints conversion to bytes to the screen 
 */
void convert_input(FILE *file) {
	char input_buffer[READ_SIZE + 1];
	void *results;
	
	// BUG: Retains memory of previous call if did not fill entire buff  
	// 	FIX: Added space for null byte at the end of malloc-d addr in util.c
	// BUG: Cannot handle two hex digits meant to be interpretted as a pair gets put in separate calls
	// BUG: Prints new line after every call to hex_to_binary
	
	// Read until EOF
	while (fgets(input_buffer, READ_SIZE + 1, file) != NULL) {
		results = hex_to_binary(input_buffer);
		if (results == NULL) {
			// BUG: Will convert to and print binary until reaches invalid hex digit
			printf("**PART OF INPUT NOT VALID HEX DIGITS**");
		} else {
			printf("%s", (char *)results);
			free(results);
		}
		printf("\n");
	}
	printf("\n");
}

