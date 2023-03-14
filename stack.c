/*
 * stack.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include "util.h"
#include "cs431vde.h"
//#include "crc32.c"

#define METADATA_SIZE 18
#define MIN_DATA_SIZE 46 
#define MAX_DATA_SIZE 1500

struct ether_header {
	uint8_t dst[6];
	uint8_t src[6];
	uint8_t type[2];
};

int
main(int argc, char *argv[])
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

	// CREATED VARIABLES
	int is_valid_frame = 1;
	uint8_t ether_addr[6]; // 86:46:6c:7e:ff:1a
	struct ether_header *curr_frame;
	ssize_t data_len;
	// NEED A VARIABLE TO HOLD THE DATA
	uint8_t fcs[4]; // frame check sequence 
	uint32_t calculated_fcs;


	// Create ethernet/MAC address
	memcpy(ether_addr, "\x86\x46\x6c\x7e\xff\x1a", 6);
//	printf("ether_addr: %x:%x:%x:%x:%x:%x\n", ether_addr[0], ether_addr[1], ether_addr[2], ether_addr[3], ether_addr[4], ether_addr[5]);

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
		if (frame_len < MIN_DATA_SIZE + METADATA_SIZE) {
			// IS THERE A BETTER WAY TO PRINT THESE MESSAGES?? 
			printf("ignoring %lu-byte frame (short)\n", frame_len);
			is_valid_frame = 0;
		} else if (frame_len > MAX_DATA_SIZE + METADATA_SIZE) {
			printf("ignoring %lu-byte frame (long)\n", frame_len);
			is_valid_frame = 0;
		}

		// Only precede if valid frame 
		if (is_valid_frame) {

			// Interpret bits as ethernet frame

			// Set header information
			curr_frame = (struct ether_header *) frame;
			printf("new_frame dst: %s\n", binary_to_hex(curr_frame->dst, 6));
			
			// Set data information
			data_len = frame_len - sizeof(struct ether_header) - sizeof(fcs); 
			printf("data_len: %lu\n", data_len); 	
			
			// Set fcs (frame check sequence)  
			for (int i = 0; i < sizeof(fcs); i++) {
				*(fcs + i) = frame[sizeof(struct ether_header) + data_len + i];
			}
			printf("FCS: %x %x %x %x\n", fcs[0], fcs[1], fcs[2], fcs[3]);

			// Verify FCS
			//calculated_fcs = crc32(0, frame, frame_len - sizeof(fcs));

		}	

			// Check if frame is broadcast 
			

			// Check if frame is for sel
		free(data_as_hex);
    
	}

    if(frame_len < 0) {
        perror("read");
        exit(1);
    }

    return 0;
}

