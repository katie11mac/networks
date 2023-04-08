/*
 * stack.c
 */

#include "stack.h"

int main(int argc, char *argv[])
{
    int fds[2];

	// Variables for vde_switch
    int connect_to_remote_switch = 0;
    char *local_vde_cmd[] = { "vde_plug", NULL };
    char *remote_vde_cmd[] = { "ssh", "pjohnson@weathertop.cs.middlebury.edu",
                                      "/home/pjohnson/cs431/bin/vde_plug",
                                      NULL };
    char **vde_cmd = connect_to_remote_switch ? remote_vde_cmd : local_vde_cmd;
	
	// Variables for frame
	uint8_t frame[1600];
    ssize_t frame_len;
    char *data_as_hex;
	
	// Variables for processing recieved frames
	int is_valid_frame = 1;
	uint8_t broadcast_addr[6];
	struct ether_header *curr_frame;
	ssize_t data_len;
	uint32_t *fcs_ptr;
	uint32_t calculated_fcs;

	// Variables for our collection of interfaces 
	struct interface *interfaces;
	uint8_t num_interfaces = 4;

	// ---------- TRIED PUTTING THIS IN A FUNCTION, BUT GOT FUNKY RESULTS ----------
	if ((interfaces = (struct interface *) malloc(num_interfaces * sizeof(struct interface))) == NULL) {
		printf("malloc failed\n");
	}
	
	// Interface 0 - WILL BE RECIEVING IN PART I
	memcpy(interfaces[0].ether_addr, "\x01\x02\x03\x04\xff\xff", 6);
	memcpy(&interfaces[0].ip_addr, "\x01\02\03\04", 4);

	// Interface 1
	memcpy(interfaces[1].ether_addr, "\x05\x06\x07\x08\xff\xff", 6);
	memcpy(&interfaces[1].ip_addr, "\x05\x06\x07\x08", 4);

	// Interface 2
	memcpy(interfaces[2].ether_addr, "\x09\x0a\x0b\x0c\xff\xff", 6);
	memcpy(&interfaces[2].ip_addr, "\x09\x0a\x0b\x0c", 4);
	
	// Interface 3
	memcpy(interfaces[3].ether_addr, "\x0d\x0e\x0f\x10\xff\xff", 6);
	memcpy(&interfaces[3].ip_addr, "\x0d\x0e\x0f\x10", 4);
	//-----------------------------------------------------------------------------

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
        //printf("received frame, length %ld:\n", frame_len);
        //puts(data_as_hex);

		// Verify length of frame 
		is_valid_frame = is_valid_frame_length(frame_len); 	

		// If valid frame, interpret bits as ethernet frame 
		if (is_valid_frame) {

			// Set header information
			curr_frame = (struct ether_header *) frame;
			//printf("new_frame dst: %s\n", binary_to_hex(curr_frame->dst, 6));
			
			// Get data length
			data_len = frame_len - sizeof(struct ether_header) - sizeof(*fcs_ptr); 
			//printf("data_len: %lu\n", data_len); 	
		
			// Set fcs 
			fcs_ptr = (uint32_t *)(frame + sizeof(struct ether_header) + data_len);
			//printf("fcs_ptr value: %u\n", *fcs_ptr);

			// Verify fcs
			calculated_fcs = crc32(0, frame, frame_len - sizeof(*fcs_ptr));
			//printf("calculated fcs: %u\n", calculated_fcs);
			
			if (calculated_fcs != *fcs_ptr) {
				printf("ignoring %ld-byte frame (bad fcs: got 0x%08x, expected 0x%08x)\n", frame_len, *fcs_ptr, calculated_fcs);
				is_valid_frame = 0;
			}
		}	
		
		// Check if destination is for any of my interfaces
		if (is_valid_frame) {

			check_dst_addr(curr_frame, frame_len, broadcast_addr, interfaces, num_interfaces); 
			//printf("jfdksl");
			//check_dst_addr(curr_frame, frame_len, broadcast_addr, ether_addr);
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
 * Check whether destination address is for me (i.e. broadcast or interfaces' MAC address) 
 * and print appropriate messages. 
 *
 * Return 0 if frame was a braodcast
 * Return 1 if frame was for me (i.e. interfaces' MAC address)
 * Return -1 if frame was not for me 
 */
int check_dst_addr(struct ether_header *curr_frame, ssize_t frame_len, uint8_t broadcast_addr[6], struct interface *interfaces, uint8_t num_interfaces) 
{
	// Check if frame is a broadcast 
	if (memcmp(curr_frame->dst, broadcast_addr, 6) == 0) {
		printf("received %lu-byte broadcast frame from %s", frame_len, binary_to_hex(curr_frame->src, 6)); 
		return 1;
	}
	
	for (int i = 0; i < num_interfaces; i++) {
		// Check if frame is for one of my interfaces 
		if (memcmp(curr_frame->dst, interfaces[i].ether_addr, 6) == 0) {
			printf("received %lu-byte frame for me from %s", frame_len, binary_to_hex(curr_frame->src, 6)); 
			printf("\tframe dest: %s\tinterface MAC: %s", binary_to_hex(curr_frame->dst, 6), binary_to_hex(interfaces[i].ether_addr, 6));
			// Can return immediately since MAC addresses unique
			return 1; 
		}
	}
	
	// Frame is not for any of my interfaces
	printf("ignoring %lu-byte frame (not for me)\n", frame_len); 
	return 0;
}
