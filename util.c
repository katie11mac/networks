/*
 * util.c
 */

#include "util.h"

int main(int argc, char *argv[]) 
{
	char *data = "ABAA"; 
	int *malloc_int = (int *)malloc(sizeof(int));
	char *data_str;
	char *int_str;

	printf("data: %s\n", data);
	//data_str = binary_to_hex(data, 4);
	data_str = "41    41  42  41 ";
//	printf("converting to binary: %s", data_str); 
	hex_to_binary(data_str);

	data_str = "41   41  42 4";
//	printf("converting to binary: %s", data_str);
	hex_to_binary(data_str); 

	data_str = "Aa KM";
	hex_to_binary(data_str);

	malloc_int = 128;
       	//printf("data: %ls\n", malloc_int); 	
	//int_str = binary_to_hex(&malloc_int, 1);
	int_str = "80";
//	printf("converting to binary: %s", int_str);	

	hex_to_binary(int_str);


//	data = "ABCDEFGHIJK lmnop QRS tuv WX yz ";
//	printf("data: %s\n", data);
//	binary_to_hex(data, 32);
	

//	data = "ABCDEFGHIJK lmnop QRS tuv WX yz katie";
//	printf("data: %s\n", data);
//	binary_to_hex(data, 37);
	//binary_to_hex(NULL, 20);
	
//	hex_to_binary();
}

/*
 * Return a point to a malloc(3) string that contains 
 * the hex representation of the binary data pointed to by data,
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
	new_n = (n * 3) + new_lines; 
	//printf("new_lines: %d\nnew_n: %d\n", new_lines, new_n);

	hex_str = malloc(new_n);
	
	// Cast data pointer to dereference it
	casted_data = (unsigned char *) data;

	char_written = 0;
	for (i = 1; i <= n; i++) {
		
		// Index 1 - 1 because initialized i to 1
		snprintf(hex_str + char_written, new_n,"%02x ", casted_data[i - 1]);
		char_written += 3;
		
		// Add \n every 16th pair
		if ((i % 16 == 0) & (i != 0)) {
			snprintf(hex_str + char_written, new_n, "\n");
			char_written ++;
		}
	}

	snprintf(hex_str + char_written, new_n, "\n");
	//printf("final string: \'%s\'", hex_str); 
	//printf("\n"); 	

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
	int leading_i, trailing_i;
	char curr_hex[2];
	trailing_i = 0;
	leading_i = 0;

	printf("hex to binary: %s\n", hex);
	
	while ((hex[trailing_i] != '\0') & (hex[leading_i] != '\0')) {
		
		// trailing_i finds index of first non-space character
		while ((hex[trailing_i] == ' ') & (hex[trailing_i] != '\0')) {
			trailing_i ++;
		}	
		
		// leading_i finds index of next non-space character
		leading_i = trailing_i + 1;
		while ((hex[leading_i] == ' ') & (hex[leading_i] != '\0')) {
			leading_i ++; 
		}

		// Uneven amount of hex digits given 	
		if (hex[leading_i] == '\0') {
			printf("(is uneven)\n");
			return NULL; // CHANGE THIS RETURN STATEMENT
		}
		
		// Check characters are valid hex digits 
		if ((isxdigit(hex[trailing_i]) == 0) | (isxdigit(hex[leading_i]) == 0)) {
			// NEED TO FREE THE MALLOC-D BUFFER
			printf("INVALID HEX DIGIT\n");
			return NULL;
		}

		// Update current hex digits working with
		curr_hex[0] = hex[trailing_i];	
		curr_hex[1] = hex[leading_i];

		printf("%s ",curr_hex);

		// Increment indices
		trailing_i = leading_i + 1;
		leading_i = trailing_i + 1;;
	}

	printf("\n"); 

	return NULL;
}
