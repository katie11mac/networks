#include "checksum.h"

/* 
 * Compute Internet Checksum for count bytes beginning at location addr.
 *
 * Resource: https://www.rfc-editor.org/rfc/rfc1071 (Section 4.1)
 */
uint16_t checksum(void *addr, uint32_t count) 
{
    uint16_t checksum;
    register uint32_t sum = 0;

    while (count > 1)  {
		// This is the inner loop
		sum += *((uint16_t *) addr);
		addr = (uint16_t *) addr + 1; 
		count -= 2;
    }

    // Add left-over byte, if any 
    if (count > 0) {
       sum += *((uint8_t *) addr);
    }

    //  Fold 32-bit sum to 16 bits 
    while (sum>>16) {
        sum = (sum & 0xffff) + (sum >> 16);
    }

    checksum = ~sum;
    return checksum;    
}
