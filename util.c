/*
 * util.c
 */

#include "util.h"

/*
int main(int argc, char *argv[]) 
{
	char *data = "ABAA"; 
	int *malloc_int = (int *)(malloc(sizeof(int)));
	char *data_str;
	char *int_str;
	void *results;
//	printf("data: %s\n", data);
//	printf("%s\n", binary_to_hex(data, 4));
	data_str = "41 42 41 41 8";
	printf("converting to binary: %s\n", data_str); 
	results = hex_to_binary(data_str);
	printf("%p\n",results);
	printf("%s\n\n",(char *)results);

//	data_str = "41   41  42 4";
//	printf("converting to binary: %s", data_str);
//	hex_to_binary(data_str); 

//	data_str = "Aa KM";
//	hex_to_binary(data_str);

//	malloc_int = 128;
       	//printf("data: %ls\n", malloc_int); 	
//	int_str = binary_to_hex(&malloc_int, 4);
	int_str = "7A";
	printf("converting to binary:%s\n", int_str);
	printf("%s\n", (char *)hex_to_binary(int_str));
	printf("%d\n", *(int *)hex_to_binary(int_str));

	
//	malloc_int = 300;
//	int_str = binary_to_hex(&malloc_int, sizeof(int));
//	printf("%s\n", int_str);
//	hex_to_binary(int_str);


//	data = "ABCDEFGHIJK lmnop QRS tuv WX yz ";
//	printf("data: %s\n", data);
//	printf("%s\n", binary_to_hex(data, 32));
	

//	data = "ABCDEFGHIJK lmnop QRS tuv WX yz katie";
//	printf("data: %s\n", data);
//	printf("%s\n", binary_to_hex(data, 37));

	//binary_to_hex(NULL, 20);
	
//	hex_to_binary();
}
*/

/*
 * Return a point to a malloc(3) string that contains 
 * the hex representation of the binary data pointed to by data,
 * which has length n. 
 *
 * Note: Caller is responsible for calling free(3) 
 */
char *binary_to_hex(void *data, ssize_t n)
{
	int new_n;
	char *hex_str;
	int i, char_written;
	unsigned char *casted_data;

	// Create new size to include spaces, new lines, and hex digits
	new_n = n * 3; 

	hex_str = malloc(new_n);
	
	// Cast data pointer to dereference it
	casted_data = (unsigned char *) data;
	//printf("unsigned char: %u\n", *casted_data );
	//printf("size of unsigned char: %ld\n", sizeof(*casted_data));

	char_written = 0;
	for (i = 1; i <= n; i++) {
		
		// Index i - 1 because initialized i to 1
		snprintf(hex_str + char_written, new_n,"%02x", casted_data[i - 1]);
		char_written += 2;
		
		// Add \n every 16th pair
		if ((i % 16 == 0) & (i != 0)) {
			snprintf(hex_str + char_written, new_n, "\n");
		} else {
			snprintf(hex_str + char_written, new_n, " ");
		}
		char_written ++;
	}

	snprintf(hex_str + char_written, new_n, "\n");

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
	ssize_t bytes_expected;
	int leading_i, trailing_i;
	char curr_hex[3];
	void *results;
	unsigned char *curr_addr; 
	ssize_t bytes_used;
	unsigned int converted_hex;

	// Calculate reasonable size for malloc
	bytes_expected = strlen(hex) / 2;
	//printf("original size: %ld\n", bytes_expected);
	results = malloc(bytes_expected);
	curr_addr = (unsigned char *)results;
	
	trailing_i = 0;
	leading_i = 0;

	//printf("hex to binary: %s\n", hex);

	// Adding null byte to treat curr_hex as a str 
	curr_hex[2] = '\0';

	while ((hex[trailing_i] != '\0') && (hex[leading_i] != '\0')) {
		
		// trailing_i finds index of first non-space character
		while ((hex[trailing_i] == ' ') && (hex[trailing_i] != '\0')) {
			trailing_i ++;
		}	
		
		// leading_i finds index of next non-space character
		leading_i = trailing_i + 1;
		while ((hex[leading_i] == ' ') && (hex[leading_i] != '\0')) {
			leading_i ++; 
		}

		// Uneven amount of hex digits given 	
		if (hex[leading_i] == '\0' || hex[leading_i] == '\n') {
			//printf("(is uneven)\n");
			break;
			//return results; 
		}
		
		// Check characters are valid hex digits 
		if ((isxdigit(hex[trailing_i]) == 0) || (isxdigit(hex[leading_i]) == 0)) {
			free(results);
			//printf("INVALID HEX DIGIT\n");
			return NULL;
		}

		// Update current hex digits working with
		curr_hex[0] = hex[trailing_i];	
		curr_hex[1] = hex[leading_i];

		// Scan hex digits from the string
		sscanf(curr_hex, "%x", &converted_hex);
		*curr_addr = (unsigned char)converted_hex;
		//printf("%p: %x\n", (void *)curr_addr, *curr_addr);
		curr_addr += 1;

		// Increment indices
		trailing_i = leading_i + 1;
		leading_i = trailing_i + 1;
	}

	// Reallocate if have unused extra space in orginal malloc
	bytes_used = (curr_addr - (unsigned char *)results);
	//printf("addr gap: %ld\n", bytes_used);
	if (bytes_used < bytes_expected) {
	//	printf("REALLOCATING\n");
		results = realloc(results, bytes_used); 
	}

	return results;
}
