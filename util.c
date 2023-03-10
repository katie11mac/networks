/*
 * util.c
 */

#include "util.h"

/*
 * Return a point to a malloc(3) string that contains 
 * the hex representation of the binary data pointed to by data,
 * which has length n. 
 *
 * Note: Caller is responsible for calling free(3) 
 */
char *binary_to_hex(void *data, ssize_t n)
{
	ssize_t new_n;
	char *hex_str;
	int i;
	ssize_t bytes_written;
	uint8_t *casted_data;
	uint8_t low_nibble;
	uint8_t high_nibble;

	// Create new size to include spaces, new lines, and hex digits
	// Added 1 for null 
	new_n = (n * 3) + 1; 

	hex_str = malloc(new_n);
	
	// Cast data pointer to dereference it
	casted_data = (uint8_t *) data;

	bytes_written = 0;

	for (i = 0; i < n; i++) {
		
		// Grab high_nibble from current addr
		high_nibble = (casted_data[i] & 0xf0) >> 4;

		// Convert high_nibble and put in memory
		// Referenced ASCI table
		if (high_nibble < 10) {
			*(hex_str + bytes_written) = (char)(high_nibble + '0');
		} else {
			*(hex_str + bytes_written) = (char)(high_nibble + 'A' - 10);
		}
		bytes_written++;

		// Grab low_nibble from current addr 
		low_nibble = casted_data[i] & 0x0f;

		// Convert low_nibble and put in memory
		if (low_nibble < 10) {
			*(hex_str + bytes_written) = (char)(low_nibble + '0');
		} else {
			*(hex_str + bytes_written) = (char)(low_nibble + 'A' - 10);
		}
		bytes_written++;

		// Add \n if last byte in data or if 16th pair
		if (i == n - 1) {
			*(hex_str + bytes_written) = '\n';
		} else if (((i + 1) % 16 == 0) && (i != 0)) {
			*(hex_str + bytes_written) = '\n';
		} else {
			*(hex_str + bytes_written) = ' ';
		}
		bytes_written ++;
		
	}

	*(hex_str + bytes_written) = '\0';

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
	char curr_hex[2];
	void *results;
	uint8_t *curr_addr; 
	ssize_t bytes_used;
	uint8_t converted_hex;

	ssize_t high_nibble_val;
	ssize_t low_nibble_val;

	// Calculate reasonable size for malloc 
	// +1 for debugging hexread (if previous input longer, adds rest of its bytes to shorter version)
	bytes_expected = (strlen(hex) / 2) + 1;
	results = malloc(bytes_expected);
	// Reset the memory
	results = memset(results, '\0', bytes_expected);
	
	// Initialize moving pointer
	curr_addr = (uint8_t *)results;
	
	trailing_i = 0;
	leading_i = 0;

	while ((hex[trailing_i] != '\0') && (hex[leading_i] != '\0')) {
		
		// trailing_i finds index of first non-space character
		while ((isspace(hex[trailing_i]) != 0) && (hex[trailing_i] != '\0')) {
			trailing_i ++;
		}	
		
		// leading_i finds index of next non-space character
		leading_i = trailing_i + 1;
		while ((isspace(hex[leading_i]) != 0) && (hex[leading_i] != '\0')) {
			leading_i ++; 
		}

		// Uneven amount of hex digits given 
		if (hex[leading_i] == '\0') {
			break;
		}
		
		// Check characters are valid hex digits 
		if ((isxdigit(hex[trailing_i]) == 0) || (isxdigit(hex[leading_i]) == 0)) {
			free(results);
			return NULL;
		}

		// Update current hex digits working with
		curr_hex[0] = hex[trailing_i];	
		curr_hex[1] = hex[leading_i];

		// Get value of the hex digit itself 
		// 	Could insert checks if they return -1, 
		// 	but we check if they're hex digits before hand
		high_nibble_val = hex_digit_to_binary(curr_hex[0]);
		low_nibble_val = hex_digit_to_binary(curr_hex[1]);
		
		// Need to shift high_nibble to first four hex digits 
		high_nibble_val = high_nibble_val << 4; 
		
		// Combine high and low nibble for full converted value
		converted_hex = high_nibble_val | low_nibble_val;

		// Store converted hex in malloc-ed memory
		*curr_addr = (uint8_t)converted_hex;
		curr_addr += 1;

		// Increment indices
		trailing_i = leading_i + 1;
		leading_i = trailing_i + 1;
	}

	// Reallocate if have unused extra space in orginal malloc
	// Want to retain extra \0 at the end, so +1
	bytes_used = (curr_addr - (uint8_t *)results) + 1;
	
	if (bytes_used < bytes_expected) {
		results = realloc(results, bytes_used);
	}
	
	return results;
}

/*
 * Given a hex digit in a string, return its binary value
 *
 * Return -1 if not a valid hex digit
 */
ssize_t hex_digit_to_binary (char hex_digit) {
	// Note: Opposite to the binary_to_hex conversion 
	if ((hex_digit >= '0') && (hex_digit <= '9')) {
		return hex_digit - '0';
	} else if ((hex_digit >= 'A') && (hex_digit <= 'F')) {
		return hex_digit - 'A' + 10;
	} else if ((hex_digit >= 'a') && (hex_digit <= 'f')) {
		return hex_digit - 'f' + 10;
	} else {
		return -1;
	}
}
