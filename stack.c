/*
 * stack.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include "util.h"
#include "cs431vde.h"

#define METADATA_SIZE 18
#define MIN_DATA_SIZE 46 
#define MAX_DATA_SIZE 1500

struct ether_header {
	uint8_t dst[6];
	uint8_t src[6];
	uint8_t type[2];
};

uint32_t crc32(uint32_t crc, const void *buf, size_t size);
int is_valid_frame_length(ssize_t frame_len); 
void check_dst_addr(struct ether_header *curr_frame, ssize_t frame_len, uint8_t broadcast_addr[6], uint8_t ether_addr[6]);

int main(int argc, char *argv[])
{
    int fds[2];

    uint8_t frame[1600];
    ssize_t frame_len;
    char *data_as_hex;
	
    int connect_to_remote_switch = 0;
    char *local_vde_cmd[] = { "vde_plug", NULL };
    char *remote_vde_cmd[] = { "ssh", "pjohnson@weathertop.cs.middlebury.edu",
                                      "/home/pjohnson/cs431/bin/vde_plug",
                                      NULL };
    char **vde_cmd = connect_to_remote_switch ? remote_vde_cmd : local_vde_cmd;

	int is_valid_frame = 1;
	uint8_t ether_addr[6]; 
	uint8_t broadcast_addr[6];
	struct ether_header *curr_frame;
	ssize_t data_len;
	uint32_t *fcs_ptr;
	uint32_t calculated_fcs;


	// Create ethernet/MAC address
	memcpy(ether_addr, "\x86\x46\x6c\x7e\xff\x1a", 6);

	// Set broadcast address 
	memcpy(broadcast_addr, "\xff\xff\xff\xff\xff\xff", 6);


	// Connecting to vde virtual switch
    if(connect_to_vde_switch(fds, vde_cmd) < 0) {
        printf("Could not connect to switch, exiting.\n");
        exit(1);
    }

	// Process frames until user terminates with Control-C
    while((frame_len = receive_ethernet_frame(fds[0], frame)) > 0) {
        data_as_hex = binary_to_hex(frame, frame_len);
        printf("received frame, length %ld:\n", frame_len);
        puts(data_as_hex);

		// Verify length of frame 
		is_valid_frame = is_valid_frame_length(frame_len); 	

		// If valid frame, interpret bits as ethernet frame 
		if (is_valid_frame) {

			// Set header information
			curr_frame = (struct ether_header *) frame;
			//printf("new_frame dst: %s\n", binary_to_hex(curr_frame->dst, 6));
			
			// Get data length
			data_len = frame_len - sizeof(struct ether_header) - sizeof(*fcs_ptr); 
			printf("data_len: %lu\n", data_len); 	
		
			// Set fcs 
			fcs_ptr = (uint32_t *)(frame + sizeof(struct ether_header) + data_len);
			printf("fcs_ptr value: %u\n", *fcs_ptr);

			// Verify fcs
			calculated_fcs = crc32(0, frame, frame_len - sizeof(*fcs_ptr));
			printf("calculated fcs: %u\n", calculated_fcs);
			
			if (calculated_fcs != *fcs_ptr) {
				printf("ignoring %ld-byte frame (bad fcs: got 0x%08x, expected 0x%08x)\n", frame_len, *fcs_ptr, calculated_fcs);
				is_valid_frame = 0;
			}

		}	
		
		// Check if destination is for me
		if (is_valid_frame) {
			check_dst_addr(curr_frame, frame_len, broadcast_addr, ether_addr);
		}

		free(data_as_hex);
	}

    if(frame_len < 0) {
        perror("read");
        exit(1);
    }

    return 0;
}


/*
 * Return 1 if the frame has a valid length and 0 otherwise 
 */
int is_valid_frame_length(ssize_t frame_len) 
{
	// Frame length too small
	if (frame_len < MIN_DATA_SIZE + METADATA_SIZE) {
		printf("ignoring %lu-byte frame (short)\n", frame_len);
		return 0;
	// Frame length too large
	} else if (frame_len > MAX_DATA_SIZE + METADATA_SIZE) {
		printf("ignoring %lu-byte frame (long)\n", frame_len);
		return 0;
	} else {
		return 1;
	}
}

/*
 * Check whether destination address is for me (i.e. broadcast or my MAC address) 
 * and print appropriate messages 
 */
void check_dst_addr(struct ether_header *curr_frame, ssize_t frame_len, uint8_t broadcast_addr[6], uint8_t ether_addr[6]) 
{
	// Check if frame is a broadcast 
	if (memcmp(curr_frame->dst, broadcast_addr, 6) == 0) {
		printf("received %lu-byte broadcast frame from %s\n", frame_len, binary_to_hex(curr_frame->src, 6)); 
	// Check if frame is for me 
	} else if (memcmp(curr_frame->dst, ether_addr, 6) == 0) {
		printf("received %lu-byte frame for me from %s\n", frame_len, binary_to_hex(curr_frame->src, 6)); 
	// Otherwise frame is not for me
	} else {
		printf("ignoring %lu-byte frame (not for me)\n", frame_len); 
	}
}
