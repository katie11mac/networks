/*
 * stack.c
 */

#include "stack.h"

int main(int argc, char *argv[])
{
    int fds[2];

	// Variables for vde_switch
    int connect_to_remote_switch = 0;
    char *local_vde_cmd[] = { "vde_plug", NULL };//"/tmp/net1.vde", NULL }; // ORIGINAL 2ND PARAM: NULL, NO 3RD
    char *remote_vde_cmd[] = { "ssh", "pjohnson@weathertop.cs.middlebury.edu",
                                      "/home/pjohnson/cs431/bin/vde_plug",
                                      NULL };
    char **vde_cmd = connect_to_remote_switch ? remote_vde_cmd : local_vde_cmd;
	
	// Variables for frame
	uint8_t frame[1600];
    ssize_t frame_len;
    char *data_as_hex;
	
	// Variables for processing recieved frames
	int is_valid = 1;
	uint8_t broadcast_addr[6];
	struct ether_header *curr_frame;
	ssize_t data_len;
	uint32_t *fcs_ptr;
	int is_ipv4 = 0;

	// Variables for processing IP frame
	int ether_dst_addr_results;
	struct ip_header *curr_packet;
	uint16_t given_checksum;
	uint8_t given_version;
	uint8_t given_ihl;
	int ip_dst_addr_results;
	int new_ttl;
	int needs_routing = 0;

	// Variables for our collection of interfaces 
	struct interface *interfaces;
	uint8_t num_interfaces = 4;

	// Variables for ARP cache 
	struct arp_entry *arp_cache;
	uint8_t num_arp_entries = 2;

	// Variables for routing table 
	struct route *routing_table;
	uint8_t num_routes = num_interfaces;


	// Set broadcast address 
	memcpy(broadcast_addr, "\xff\xff\xff\xff\xff\xff", 6);

	// Initialize interfaces, arp cache, and routing table
	init_interfaces(&interfaces, num_interfaces);
	init_routing_table(&routing_table, num_routes);
	init_arp_cache(&arp_cache, num_arp_entries); 

	// Connecting to vde virtual switch
    if(connect_to_vde_switch(fds, vde_cmd) < 0) {
        printf("Could not connect to switch, exiting.\n");
        exit(1);
    }

	// Process frames until user terminates with Control-C
    while((frame_len = receive_ethernet_frame(fds[0], frame)) > 0) {
        data_as_hex = binary_to_hex(frame, frame_len);

		// Verify length of frame 
		is_valid = is_valid_frame_length(frame_len); 	

		// If valid frame, interpret bits as ethernet frame 
		if (is_valid) {
			// Set header information
			curr_frame = (struct ether_header *) frame;
		}

		// Verify fcs
		if (is_valid) { 
			// Get data length and set ptr to given fcs 
			data_len = frame_len - sizeof(struct ether_header) - sizeof(*fcs_ptr); 
			fcs_ptr = (uint32_t *)(frame + sizeof(struct ether_header) + data_len);

			is_valid = is_valid_fcs(&frame, frame_len, data_len, *fcs_ptr);
		}

		// Verify type 
		if (is_valid) {
			// Ethernet is IPv4
			if (memcmp(curr_frame->type, "\x08\x00", 2) == 0) {
				is_ipv4 = 1;
			// Otherwise unrecognized type 
			} else {
				printf("ignoring %lu-byte frame (unrecognized type)\n", frame_len); 
				is_valid = 0;
			}
		}
		
		// If valid frame and IPv4, check if destination is my receiving interface (PART I) 
		if (is_valid && is_ipv4) {
			
			ether_dst_addr_results = check_ether_dst_addr(curr_frame, frame_len, broadcast_addr, interfaces, num_interfaces);

			// THIS IS CURRENTLY ONLY CHECKING IF FRAME WAS FOR ME (NOT BROADCAST)
			
			if (ether_dst_addr_results == 1) {
				// printf("\tUNWRAPPING ETHERNET FRAME FOR ME\n"); 
				
				// Interpret data as IPv4
				curr_packet = (struct ip_header *) (frame + sizeof(struct ether_header));
				
				// ---------------------------- MAKE THIS A FUNCTION ??? --------------------------------------
				// Check if total length is correct 
				if (((uint8_t *)fcs_ptr - (uint8_t *)curr_packet) != ntohs(curr_packet->total_length)) {
					printf("dropping packet from %u.%u.%u.%u (wrong length)\n", curr_packet->src_addr.part1, 
																				curr_packet->src_addr.part2, 
																				curr_packet->src_addr.part3, 
																				curr_packet->src_addr.part4);
					is_valid = 0;
				}

							
				// Check if the header checksum is correct 
				if (is_valid) {
					
					// Get given IHL 
					given_ihl = curr_packet->version_and_ihl & 0x0f; // low nibble

					// Get given ip checksum 
					given_checksum = curr_packet->header_checksum;
					
					// Reset the header checksum to recalculate it correctly
					curr_packet->header_checksum = 0;
				
					if (given_checksum != ip_checksum(curr_packet, (given_ihl * 32) / 8)) {
						printf("dropping packet from %u.%u.%u.%u (bad IP header checksum)\n", curr_packet->src_addr.part1, 
																				curr_packet->src_addr.part2, 
																				curr_packet->src_addr.part3, 
																				curr_packet->src_addr.part4);
						is_valid = 0;
					}
				}
				// --------------------------------------------------------------------------------------------

				// Check if provided recognized IP version (only IPv4)
				if (is_valid) {
				
					// Get given version
					given_version = (curr_packet->version_and_ihl & 0xf0) >> 4; // high nibble

					if (given_version != 4) {
						printf("dropping packet from %u.%u.%u.%u (unrecognized IP version)\n", curr_packet->src_addr.part1, 
																				curr_packet->src_addr.part2, 
																				curr_packet->src_addr.part3, 
																				curr_packet->src_addr.part4);
						is_valid = 0;
					}
				}
			

				// Get ip destination and check if it has a valid TTL accordingly 
				if (is_valid) {
					// Check ip destination 
					ip_dst_addr_results = check_ip_dst(curr_packet, interfaces, num_interfaces);
					new_ttl = curr_packet->ttl - 1;

					// curr_packet destined for one of my interfaces
					if (ip_dst_addr_results != -1) {
						// SHOULD I STILL CHECK THE TTL HERE TO MAKE SURE IT'S NOT LOWER THAN ZERO?
						printf("received packet from %u.%u.%u.%u for %u.%u.%u.%u (interface %d)\n", 
																	curr_packet->src_addr.part1,
																	curr_packet->src_addr.part2,
                                                                    curr_packet->src_addr.part3,
                                                                    curr_packet->src_addr.part4, 
																	interfaces[ip_dst_addr_results].ip_addr.part1, 
																	interfaces[ip_dst_addr_results].ip_addr.part2,
																	interfaces[ip_dst_addr_results].ip_addr.part3,
																	interfaces[ip_dst_addr_results].ip_addr.part4, 
																	ip_dst_addr_results); 

					// curr_packet not for one of my interfaces (needs routing) 
					} else {
						// TTL exceeded since it needs routing 
						if (new_ttl <= 0) {
							printf("dropping packet from %u.%u.%u.%u to %u.%u.%u.%u (TTL exceeded)\n", 
																				curr_packet->src_addr.part1, 
																				curr_packet->src_addr.part2, 
																				curr_packet->src_addr.part3, 
																				curr_packet->src_addr.part4, 
																				curr_packet->dst_addr.part1, 
																				curr_packet->dst_addr.part2, 
																				curr_packet->dst_addr.part3, 
																				curr_packet->dst_addr.part4);
							is_valid = 0;
						// TTL is valid 
						} else {
							needs_routing = 1;
						}
					}
				}

				if (is_valid && needs_routing) {
					printf("NEED TO ROUTE!!!\n");
					printf(" from %u.%u.%u.%u to %u.%u.%u.%u\n", curr_packet->src_addr.part1,
																 curr_packet->src_addr.part2,
																 curr_packet->src_addr.part3,
																 curr_packet->src_addr.part4,
																 curr_packet->dst_addr.part1,
															  	 curr_packet->dst_addr.part2,
																 curr_packet->dst_addr.part3,
																 curr_packet->dst_addr.part4);
				}


				// if dst is already here ... 
				// if needs to hop elsewhere ... 
				// DON'T FORGET ABOUT TTL 
			}
		}

		free(data_as_hex);
		printf("\n");
	}

    if(frame_len < 0) {
        perror("read");
        exit(1);
    }

    return 0;

	// CHECK WHERE WE NEED TO FREE INTERFACES, ROUTING TABLE, and CACHE !!!!!!!!!!!!
}

/*
 * Initialize interfaces with hardcoded values
 */
void init_interfaces(struct interface **interfaces, uint8_t num_interfaces) {

	if ((*interfaces = (struct interface *) malloc(num_interfaces * sizeof(struct interface))) == NULL) {
        printf("malloc failed\n");
		return;
	}

    // Interface 0 - WILL BE RECIEVING IN PART I
    memcpy((*interfaces)[0].ether_addr, "\x01\x02\x03\x04\xff\xff", 6);
    memcpy(&(*interfaces)[0].ip_addr, "\x01\02\03\04", 4);

    // Interface 1
    memcpy((*interfaces)[1].ether_addr, "\x05\x06\x07\x08\xff\xff", 6);
    memcpy(&(*interfaces)[1].ip_addr, "\x05\x06\x07\x08", 4);

    // Interface 2
    memcpy((*interfaces)[2].ether_addr, "\x09\x0a\x0b\x0c\xff\xff", 6);
    memcpy(&(*interfaces)[2].ip_addr, "\x09\x0a\x0b\x0c", 4);

    // Interface 3
    memcpy((*interfaces)[3].ether_addr, "\x0d\x0e\x0f\x10\xff\xff", 6);
    memcpy(&(*interfaces)[3].ip_addr, "\x0d\x0e\x0f\x10", 4);

}

/*
 * Initialize routing table with hard coded routes 
 */
void init_routing_table(struct route **routing_table, uint8_t num_routes) {
	
	if ((*routing_table = (struct route *) malloc(num_routes * sizeof(struct route))) == NULL) {
		printf("malloc failed\n");
		return;
	}

	// Route 0
	(*routing_table)[0].num_interface = 0;
	memcpy(&(*routing_table)[0].dst, "\x01\x02\x03\x00", 4);
	memcpy(&(*routing_table)[0].genmask, "\xff\xff\xff\x00", 4);

	// Route 1
	(*routing_table)[1].num_interface = 1;
	memcpy(&(*routing_table)[1].dst, "\x05\x06\x07\x00", 4);
	memcpy(&(*routing_table)[1].genmask, "\xff\xff\xff\x00", 4);
	
	// Route 2
	(*routing_table)[2].num_interface = 2;
	memcpy(&(*routing_table)[2].dst, "\x09\x0a\x0b\x00", 4);
	memcpy(&(*routing_table)[2].genmask, "\xff\xff\xff\x00", 4);

	// Route 3
	(*routing_table)[3].num_interface = 3;
	memcpy(&(*routing_table)[3].dst, "\x0d\x0e\x0f\x00", 4);
	memcpy(&(*routing_table)[3].genmask, "\xff\xff\xff\x00", 4);
}

/*
 * Initialize arp cache with hard coded values 
 */
void init_arp_cache(struct arp_entry **arp_cache, uint8_t num_arp_entries) {
	
	if ((*arp_cache = (struct arp_entry *) malloc(num_arp_entries * sizeof(struct arp_entry))) == NULL) {
		printf("malloc failed\n");
		return;
	}
	
	// ARP Entry 0
	memcpy((*arp_cache)[0].ether_addr, "\x11\x22\x33\x00\xff\xff", 6);
	memcpy(&(*arp_cache)[0].ip_addr, "\x01\x02\x03\x00", 4);

	// ARP Entry 1
	memcpy((*arp_cache)[1].ether_addr, "\xdd\xee\xff\x00\xff\xff", 6);
	memcpy(&(*arp_cache)[1].ip_addr, "\x0d\x0e\x0f\x00", 4);

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


int is_valid_fcs (uint8_t (*frame)[1600], size_t frame_len, ssize_t data_len, uint32_t fcs) 
{
	uint32_t calculated_fcs;

	// Verify fcs
	calculated_fcs = crc32(0, *frame, frame_len - sizeof(fcs));
		
	if (calculated_fcs != fcs) {
		printf("ignoring %ld-byte frame (bad fcs: got 0x%08x, expected 0x%08x)\n", frame_len, fcs, calculated_fcs);
		return 0;
	}

	return 1;

}



/*
 * Check whether destination address is for me (i.e. broadcast or interfaces' MAC address) 
 * and print appropriate messages. 
 *
 * Return 0 if frame was a braodcast
 * Return 1 if frame was for me (i.e. interfaces' MAC address)
 * Return -1 if frame was not for me 
 */
int check_ether_dst_addr(struct ether_header *curr_frame, ssize_t frame_len, uint8_t broadcast_addr[6], struct interface *interfaces, uint8_t num_interfaces) 
{
	int receiving_interface = 0;

	// Check if frame is a broadcast 
	if (memcmp(curr_frame->dst, broadcast_addr, 6) == 0) {
		printf("received %lu-byte broadcast frame from %s", frame_len, binary_to_hex(curr_frame->src, 6)); 
		return 0;
	
	// Check if frame is for the receiving interface 
	} else if (memcmp(curr_frame->dst, interfaces[receiving_interface].ether_addr, 6) == 0) {
		printf("DEBUGGING: received %lu-byte frame for me from %s", frame_len, binary_to_hex(curr_frame->src, 6)); 
		//printf("\tframe dest: %s\tinterface MAC: %s", binary_to_hex(curr_frame->dst, 6), binary_to_hex(interfaces[receiving_interface].ether_addr, 6));
		// Can return immediately since MAC addresses unique
		return 1; 
	}

	/* 
	// THIS NEEDS TO CHANGE SO THAT WE CHECK THAT IT'S ONLY THE DEVICE THAT IS RECIEVING
	for (int i = 0; i < num_interfaces; i++) {
		// Check if frame is for one of my interfaces 
		if (memcmp(curr_frame->dst, interfaces[i].ether_addr, 6) == 0) {
			//printf("received %lu-byte frame for me from %s", frame_len, binary_to_hex(curr_frame->src, 6)); 
			//printf("\tframe dest: %s\tinterface MAC: %s", binary_to_hex(curr_frame->dst, 6), binary_to_hex(interfaces[i].ether_addr, 6));
			// Can return immediately since MAC addresses unique
			return 1; 
		}
	}
	*/
	// Frame is not for any of my interfaces
	printf("ignoring %lu-byte frame (not for me)\n", frame_len); 
	return 0;
}


/*
 * Return index of interface it was destined for
 * Return -1 otherwise 
 */
int check_ip_dst(struct ip_header *curr_packet, struct interface *interfaces, uint8_t num_interfaces) {
	
	for (int i = 0; i < num_interfaces; i++) {
		// Check if packet is for one of my interfaces 
		if (compare_ip_addr_structs(curr_packet->dst_addr, interfaces[i].ip_addr) == 1) {
			return i; // Can return immediately bc IP addresses are unique 
		} 
	}

	// curr_packet not for any of my interfaces
	return -1;
}

/*
 * 
 */
int compare_ip_addr_structs(struct ip_address addr1, struct ip_address addr2) {
	
	if (addr1.part1 != addr2.part1) {
		return 0;
	}
	if (addr1.part2 != addr2.part2) {
		return 0;
	}
	if (addr1.part3 != addr2.part3) {
		return 0;
	}
	if (addr1.part4 != addr2.part4) {
		return 0;
	}

	return 1;
}
