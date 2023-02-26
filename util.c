/*
 * util.c
 */

#include <stdio.h>
#include <stdlib.h>
#include "util.h"

int main(int argc, char *argv[]) 
{
	char *data = "ABAA"; 
	int *malloc_int = malloc(sizeof(int));
	
	printf("data: %s\n", data);
	printf("%s", binary_to_hex(data, 4));
	
	malloc_int = 128;
       	//printf("data: %ls\n", malloc_int); 	
	printf("%s", binary_to_hex(&malloc_int, 1)); 

	data = "ABCDEFGHIJK lmnop QRS tuv WX yz ";
	printf("data: %s\n", data);
	binary_to_hex(data, 32);
	

	data = "ABCDEFGHIJK lmnop QRS tuv WX yz katie";
	printf("data: %s\n", data);
	binary_to_hex(data, 37);
	//binary_to_hex(NULL, 20);
}

/*
 * Return a point to a malloc(3) string that contains 
 * the hex representation of the binary data pointed to by s,
 * which has length n. 
 *
 * Note: Caller is responsible for calling free(3) 
 */
char *binary_to_hex(void *data, ssize_t n)
{
	int new_n, new_lines;
	char *hex_str;
	int i, char_written;
	unsigned char *casted_data;

	// Find number of spaces and new lines needed 
	new_lines = (n / 16) + 1;
	new_n = (n * 3) + new_lines; // originally had the spaces - 1 but removed to count for null
	// might need to -1 still bc snprintf include space for null 
	// or leave it bc we'll have a trailing space lol

	//printf("new_lines: %d\nnew_n: %d\n", new_lines, new_n);

	hex_str = malloc(new_n);
	
	casted_data = (unsigned char *) data;

	char_written = 0;
	for (i = 1; i <= n; i++) {
		snprintf(hex_str + char_written, new_n,"%02x ", casted_data[i - 1]);
		char_written += 3;
		
		// % 15 bc start at 0	
		if (i % 16 == 0 && i != 0) {
			snprintf(hex_str + char_written, new_n, "\n");
			char_written ++;
		}
	}
	snprintf(hex_str + char_written, new_n, "\n");
	printf("final string: \'%s\'", hex_str); 
	printf("\n"); 	

	return hex_str;
}

/*
 * Return a pointer to a malloc(3) buffer that contains the 
 * binary representation of the bits hex data pointed to by hex, 
 * which itself will be NULL-terminated.
 *
 * Ignores whitespace, support both upper and lowercase hex digits, 
 * but if any non-hex characters are encountered, it immediately 
 * free(3) the buffer and returns NULL. 
 *
 * If the buffer contains an odd number of hex digits, 
 * it ignores the final one.
 *
 * Note: Caller is responsible for calling free(3)  
 */
void *hex_to_binary(char *hex)
{
	return NULL;
}
