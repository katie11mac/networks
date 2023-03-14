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

struct ether_frame {
	uint8_t dst[48];
	uint8_t src[48];
	uint8_t type[16];
	uint8_t data[12000]; // SHOULD WE SET IT TO THE MAX SIZE?
	uint8_t fcs[32]; // frame check sequence 
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

	int is_valid_frame = 1;
	

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
			//fflush(stdout);
			is_valid_frame = 0;
		} else if (frame_len > MAX_DATA_SIZE + METADATA_SIZE) {
			printf("ignoring %lu-byte frame (long)\n", frame_len);
			//fflush(stdout);
			is_valid_frame = 0;
		}

		// Verify FCS ONLY IF VALID FRAME


		// ONLY WANT TO PRECEDE IF THE PRECEDING CHECKS PASS !!!!! 
		
		if (is_valid_frame) {
			 // Interpret bits as ethernet frame
			 struct ether_frame *new_frame = (struct ether_frame *) frame;
			//printf("new_frame dst: %s\n", binary_to_hex(new_frame->dst, 6));
			
			// Check if frame is broadcast 
			

			// Check if frame is for sel
		}	

		free(data_as_hex);
    
	}

    if(frame_len < 0) {
        perror("read");
        exit(1);
    }

    return 0;
}

