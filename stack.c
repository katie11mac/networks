/*
 * stack.c
 */

#include "stack.h"

int main(int argc, char *argv[])
{
	// Variables for our collection of interfaces 
	struct interface *interfaces;
	uint8_t num_interfaces = 4;

	// Variables for ARP cache 
	struct arp_entry *arp_cache;
	uint8_t num_arp_entries = 3;

	// Variables for routing table 
	struct route *routing_table;
	uint8_t num_routes = 6;

	// Variables for vde_switch (one for each interface)
    int fds[num_interfaces][2];
	char vde_path[20];
    int connect_to_remote_switch = 0;
    char *local_vde_cmd[] = { "vde_plug", NULL, NULL };
	char *remote_vde_cmd[] = { "ssh", "pjohnson@weathertop.cs.middlebury.edu",
                                      "/home/pjohnson/cs431/bin/vde_plug",
                                      NULL };
    char **vde_cmd = connect_to_remote_switch ? remote_vde_cmd : local_vde_cmd;
	
	// Variables for frame
	uint8_t frame[1600];
    ssize_t frame_len;
    char *data_as_hex;
	
	// Variables for processing recieved frames
	int is_valid = 1; // Whether it is corrupted and has valid values
	struct ether_header *curr_frame;
	ssize_t data_len;
	uint32_t *fcs_ptr;
	int is_ipv4 = 0;
	int is_arp = 0;

	// Variables for processing IP frame
	int ether_dst_addr_results;
	struct ip_header *curr_packet;
	uint8_t given_version;
	int ip_dst_addr_results;
	int new_ttl;
	int needs_routing = 0;
	int route_entry_num = -1; // Number entry in routing_table to take
	struct route route_to_take;
	struct ip_address direct_network_gateway;
	uint8_t mac_dst[6];
	int found_mac_addr = 0;

	// Variables for processing ARP types
	struct arp_packet *curr_arp_packet;

	// Set direct network gateway value
	memcpy(&direct_network_gateway, "\x00\x00\x00\x00", 4);
	

	// Initialize interfaces, arp cache, and routing table
	init_interfaces(&interfaces, num_interfaces);
	init_routing_table(&routing_table, num_routes);
	init_arp_cache(&arp_cache, num_arp_entries); 

	// Connect to vde virtual switches for all interfaces	
	for (int i = 0; i < num_interfaces; i++) {
		
		sprintf(vde_path, "/tmp/net%d.vde", i);
		local_vde_cmd[1] = vde_path;
		vde_cmd = connect_to_remote_switch ? remote_vde_cmd : local_vde_cmd;
	
		// Connecting to vde virtual switch
		if (connect_to_vde_switch(fds[i], vde_cmd) < 0) {
			printf("Could not connect to switch, exiting.\n");
			exit(1);
		}
	}
	

	// Process frames until user terminates with Control-C
	// (Assignment 3 Part I: Only listening on interface 0)
    while ((frame_len = receive_ethernet_frame(fds[0][0], frame)) > 0) {
		
		// Reset variables
		is_ipv4 = 0;
		is_arp = 0;
		needs_routing = 0;
		route_entry_num = -1;
		found_mac_addr = 0;
		
		// Save data received
		data_as_hex = binary_to_hex(frame, frame_len);

		// Set header information
		curr_frame = (struct ether_header *) frame;

		// Verify length of frame 
		is_valid = is_valid_frame_length(frame_len); 	

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
			// Ethernet is ARP 
			} else if (memcmp(curr_frame->type, "\x08\x06", 2) == 0) {
				is_arp = 1;
			// Otherwise unrecognized type 
			} else {
				printf("ignoring %lu-byte frame (unrecognized type)\n", frame_len); 
				is_valid = 0;
			}
		}
		
		// Acknowledge broadcasts and set ether_dst_addr_results
		if (is_valid) {
			ether_dst_addr_results = check_ether_dst_addr(curr_frame, frame_len, interfaces, num_interfaces);
		}
		
		if (is_valid && is_arp) {
			
			printf("RECEIVED ARP PACKET\n");
			curr_arp_packet = (struct arp_packet *) (frame + sizeof(struct ether_header));
			
			// Verify hardware type and size (only handling ethernet hardware) 
			if (is_valid) {
			
				// WANT TO DOUBLE CHECK IF THIS IS THE BEHAVIOR WE WANT!!!!! 
				// Only want to handle ethernet hardware type
				if (memcmp(curr_arp_packet->hardware_type, "\x00\x01", 2) == 0) {
					
					// Verify the hardware size for ethernet type
					if (memcmp(&curr_arp_packet->hardware_size, "\x06", 1) != 0) {
						printf("ignoring arp packet with bad hardware size from %s", binary_to_hex(curr_arp_packet->sender_mac_addr, 6));
						is_valid = 0;
					}

				} else {
					printf("ignoring arp packet with incompatible hardware type from %s", binary_to_hex(curr_arp_packet->sender_mac_addr, 6));
					is_valid = 0;
				}

			}

			// Verify protocol type and size (only handling IPv4) 
            if (is_valid) {
            
                // WANT TO DOUBLE CHECK IF THIS IS THE BEHAVIOR WE WANT!!!!! 
                // Only want to handle IPv4 protocol type
                if (memcmp(curr_arp_packet->protocol_type, "\x08\x00", 2) == 0) {
                    
                    // Verify the hardware size for ethernet type
                    if (memcmp(&curr_arp_packet->protocol_size, "\x04", 1) != 0) {
                        printf("ignoring arp packet with bad protocol size from %s", binary_to_hex(curr_arp_packet->sender_mac_addr, 6));
                        is_valid = 0;
                    }

                } else {
                    printf("ignoring arp packet with incompatible protocol type from %s", binary_to_hex(curr_arp_packet->sender_mac_addr, 6));
                    is_valid = 0;
                }

            }

		}

		// -------------------------- IP PACKETS -----------------------------------------
		// If valid frame and IPv4, check if destination is my receiving interface (PART I) 
		if (is_valid && is_ipv4) {
			

			// THIS IS CURRENTLY ONLY CHECKING IF FRAME WAS FOR ME (NOT BROADCAST)
			
			// Frame is for me 
			if (ether_dst_addr_results == 1) {
				
				// Interpret data as IPv4
				curr_packet = (struct ip_header *) (frame + sizeof(struct ether_header));
				

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
					is_valid = is_valid_ip_checksum(curr_packet);
				}
				
				// Check IHL and whether it is greater than 5
				if (is_valid) {
				
					if ((curr_packet->version_and_ihl & 0x0f) < 5) {
						printf("dropping packet from %u.%u.%u.%u (invalid IHL)\n", curr_packet->src_addr.part1, 
																				   curr_packet->src_addr.part2, 
																				   curr_packet->src_addr.part3, 
																				   curr_packet->src_addr.part4);
						is_valid = 0; 
					}
				}
				

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


				// Check whether have route for packet in routing table
				if (is_valid && needs_routing) {
					/*
					printf("DEBUGGING: NEED TO ROUTE FROM %u.%u.%u.%u to %u.%u.%u.%u\n", curr_packet->src_addr.part1,
																						 curr_packet->src_addr.part2,
																						 curr_packet->src_addr.part3,
																						 curr_packet->src_addr.part4,
																						 curr_packet->dst_addr.part1,
																						 curr_packet->dst_addr.part2,
																						 curr_packet->dst_addr.part3,
																						 curr_packet->dst_addr.part4);
					*/
					// Determine whether there is a valid route in routing table
					route_entry_num = determine_route(curr_packet, interfaces, num_interfaces, routing_table, num_routes); 

					// No corresponding entry in routing table for IP packet
					if (route_entry_num == -1) {
						printf("dropping packet from %u.%u.%u.%u to %u.%u.%u.%u (no route)\n",
																		curr_packet->src_addr.part1,
																		curr_packet->src_addr.part2,
																		curr_packet->src_addr.part3,
																		curr_packet->src_addr.part4,
																		curr_packet->dst_addr.part1,
																		curr_packet->dst_addr.part2,
																		curr_packet->dst_addr.part3,
																		curr_packet->dst_addr.part4);
						// SHOULD I SET is_valid or needs_routing HERE??? 
						is_valid = 0;

					// Found corresponding entry in routing table for IP packet
					} else {
						//printf("FINDING MAC ADDRESS FOR NEXT HOP\n");
						
						route_to_take = routing_table[route_entry_num];
						
						// Get MAC dst
						// If gateway of route is 0.0.0.0 (meaning destination device is in one of networks router is connected to)
						// then we can send the ethernet frame directly to the device 
						if (compare_ip_addr_structs(route_to_take.gateway, direct_network_gateway) == 1) {
							//printf("ROUTE IS DIRECTLY CONNECTED MEANING WE CAN SEND THE PACKET TO ITS FINAL DESTINATION\n");
							
							// Find dst mac address to the current packets ip dest in arp cache 
							found_mac_addr = determine_mac_from_ip(mac_dst, curr_packet->dst_addr, arp_cache, num_arp_entries);
						
						// If gateway of route is not 0.0.0.0, then we have to send packet to another router 
						} else {
							//printf("ROUTE IS NOT DIRECTLY CONNECTED MEANING WE HAVE TO SEND IT TO ANOTHER ROUTER\n");	
							
							// Find dst mac address to the current packets ip dest in arp cache 
							found_mac_addr = determine_mac_from_ip(mac_dst, route_to_take.gateway, arp_cache, num_arp_entries);
							
						}

						// Could not find corresponding mac address for route
						if (found_mac_addr == 0) {
							printf("dropping packet from %u.%u.%u.%u to %u.%u.%u.%u (no ARP)\n",
                                                                        curr_packet->src_addr.part1,
                                                                        curr_packet->src_addr.part2,
                                                                        curr_packet->src_addr.part3,
                                                                        curr_packet->src_addr.part4,
                                                                        curr_packet->dst_addr.part1,
                                                                        curr_packet->dst_addr.part2,
                                                                        curr_packet->dst_addr.part3,
                                                                        curr_packet->dst_addr.part4);
						}

					}
					
					// INSIDE THE is_valid && needs_routing conditional
					// Rewrite the frame and forward it to next hop 
					//		Note: Rewriting over received frame since we want to keep the IP packet in tact
					if ((route_entry_num != -1) && (found_mac_addr == 1)) {
						//printf("NEED TO AND CAN FORWARD THIS PACKET TO NEXT HOP or FINAL DESTINATION\n");
						//printf("FORWARDING TO INTERFACE %d\n", route_to_take.num_interface);
						
						// Set frame src to corresponding leaving interface
						memcpy(curr_frame->src, interfaces[route_to_take.num_interface].ether_addr, 6); 
						
						// Set frame dst to mac address found in arp cache
						memcpy(curr_frame->dst, mac_dst, 6);

						// Update the TTL 
						curr_packet->ttl = new_ttl;

						// Recalculate IP header checksum since changed TTL
						curr_packet->header_checksum = ip_checksum(&(*curr_packet), ((curr_packet->version_and_ihl & 0x0f) * 32) / 8);
						
						// Recalculate FCS since changed data in frame 
						*fcs_ptr = crc32(0, frame, frame_len - sizeof(*fcs_ptr));
						
						// Send to corresponding fd for vde switch connected to the interface
						send_ethernet_frame(fds[route_to_take.num_interface][1], frame, frame_len);	
					}
				}
			}
		}
		
		// NEED TO RESET A HANDFUL OF BOOLEAN VARIABLES
		free(data_as_hex);
		printf("\n");
	}

	free(interfaces);
	free(arp_cache);
	free(routing_table);

    if (frame_len < 0) {
        perror("read");
        exit(1);
    }

    return 0;

}

/*
 * Initialize interfaces with hardcoded values
 */
void init_interfaces(struct interface **interfaces, uint8_t num_interfaces) 
{
	if ((*interfaces = (struct interface *) malloc(num_interfaces * sizeof(struct interface))) == NULL) {
        printf("malloc failed\n");
		return;
	}

    // Interface 0 - WILL BE RECIEVING IN PART I
    memcpy((*interfaces)[0].ether_addr, "\x01\x02\x03\x04\xff\xff", 6);
    memcpy(&(*interfaces)[0].ip_addr, "\x01\x02\x03\x04", 4);

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
void init_routing_table(struct route **routing_table, uint8_t num_routes) 
{	
	if ((*routing_table = (struct route *) malloc(num_routes * sizeof(struct route))) == NULL) {
		printf("malloc failed\n");
		return;
	}
	
	// Route 0 (interface 0)
	(*routing_table)[0].num_interface = 0;
	memcpy(&(*routing_table)[0].dst, "\x01\x02\x03\x00", 4);
	memcpy(&(*routing_table)[0].gateway, "\x00\x00\x00\x00", 4);
	memcpy(&(*routing_table)[0].genmask, "\xff\xff\xff\x00", 4);

	// Route 1 (interface 1)
	(*routing_table)[1].num_interface = 1;
	memcpy(&(*routing_table)[1].dst, "\x05\x06\x07\x00", 4);	
	memcpy(&(*routing_table)[1].gateway, "\x00\x00\x00\x00", 4);
	memcpy(&(*routing_table)[1].genmask, "\xff\xff\xff\x00", 4);
	
	// Route 2 (interface 2)
	(*routing_table)[2].num_interface = 2;
	memcpy(&(*routing_table)[2].dst, "\x09\x0a\x0b\x00", 4);
	memcpy(&(*routing_table)[2].gateway, "\x00\x00\x00\x00", 4);
	memcpy(&(*routing_table)[2].genmask, "\xff\xff\xff\x00", 4);

	// Route 3 (interface 3)
	(*routing_table)[3].num_interface = 3;
	memcpy(&(*routing_table)[3].dst, "\x0d\x0e\x0f\x00", 4);
	memcpy(&(*routing_table)[3].gateway, "\x00\x00\x00\x00", 4);
	memcpy(&(*routing_table)[3].genmask, "\xff\xff\xff\x00", 4);
	
	// Route 4 (to another router through interface 3)
	(*routing_table)[4].num_interface = 3;
	memcpy(&(*routing_table)[4].dst, "\x11\x12\x13\x00", 4);
	memcpy(&(*routing_table)[4].gateway, "\x0d\x0e\x0f\x11", 4);
	memcpy(&(*routing_table)[4].genmask, "\xff\xff\xff\x00", 4);

	// Route 5 (to another router through interface 2)
	(*routing_table)[5].num_interface = 2;
    memcpy(&(*routing_table)[5].dst, "\x0b\x0b\x00\x00", 4);
    memcpy(&(*routing_table)[5].gateway, "\x09\x0a\x0b\x0d", 4);
    memcpy(&(*routing_table)[5].genmask, "\xff\xff\x00\x00", 4);	
}

/*
 * Initialize arp cache with hard coded values 
 */
void init_arp_cache(struct arp_entry **arp_cache, uint8_t num_arp_entries) 
{	
	if ((*arp_cache = (struct arp_entry *) malloc(num_arp_entries * sizeof(struct arp_entry))) == NULL) {
		printf("malloc failed\n");
		return;
	}
	
	// Device A
	memcpy((*arp_cache)[0].ether_addr, "\x11\x22\x33\x00\xff\xff", 6);
	memcpy(&(*arp_cache)[0].ip_addr, "\x01\x02\x03\x00", 4);

	// Device H
	memcpy((*arp_cache)[1].ether_addr, "\xdd\xee\xff\x00\xff\xff", 6);
	memcpy(&(*arp_cache)[1].ip_addr, "\x0d\x0e\x0f\x00", 4);

	// Router 1 (connected to I3) 
	memcpy((*arp_cache)[2].ether_addr, "\xdd\xee\xff\x11\xff\xff", 6);
	memcpy(&(*arp_cache)[2].ip_addr, "\x0d\x0e\x0f\x11", 4);

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
 * Check if initial fcs in Ethernet frame is correct by recalculating it. 
 *
 * Return 1 if fcs is correct
 * Return 0 otherwise 
 */
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
int check_ether_dst_addr(struct ether_header *curr_frame, ssize_t frame_len, struct interface *interfaces, uint8_t num_interfaces) 
{
	int receiving_interface = 0;

	// Check if frame is a broadcast 
	if (memcmp(curr_frame->dst, BROADCAST_ADDR, 6) == 0) {
		printf("received %lu-byte broadcast frame from %s", frame_len, binary_to_hex(curr_frame->src, 6)); 
		return 0;
	
	// Check if frame is for the receiving interface 
	} else if (memcmp(curr_frame->dst, interfaces[receiving_interface].ether_addr, 6) == 0) {
		//printf("DEBUGGING: received %lu-byte frame for me from %s", frame_len, binary_to_hex(curr_frame->src, 6)); 
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
 * Check if inital checksum in IP header is correct by recalculating it.
 *
 * Return 1 if checksum is correct
 * Return 0 otherwise
 */
int is_valid_ip_checksum(struct ip_header *curr_packet)
{
	uint16_t given_checksum;
	uint8_t given_ihl;

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
		return 0;
	}

	return 1;

}




/*
 * Check IP destination of IP packet. Determine whether IP packet is for one of interfaces.
 *
 * Return index of interface it was destined for
 * Return -1 otherwise (meaning IP packet not destined for one of interfaces) 
 */
int check_ip_dst(struct ip_header *curr_packet, struct interface *interfaces, uint8_t num_interfaces) 
{
	
	for (int i = 0; i < num_interfaces; i++) {
		// Check if packet is for one of my interfaces 
		if (compare_ip_addr_structs(curr_packet->dst_addr, interfaces[i].ip_addr) == 1) {
			// Can return immediately since IP addresses are unique
			return i; 
		} 
	}

	// curr_packet not for any of my interfaces
	return -1;
}



/*
 * Compare two ip address structs. 
 *
 * Return 1 if all four parts of the ip address are the same 
 * Return 0 otherwise
 */
int compare_ip_addr_structs(struct ip_address addr1, struct ip_address addr2) 
{
	
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



/*
 * Convert ip_struct to a uint32_t for bit comparison purposes.
 * Return the converted value. 
 */
uint32_t convert_ip_addr_struct(struct ip_address ip) 
{

	uint32_t result = 0; 

	result = result | ((uint32_t)ip.part1 << 24);
	result = result | ((uint32_t)ip.part2 << 16);
	result = result | ((uint32_t)ip.part3 << 8);
	result = result | (uint32_t)ip.part4;

	return result;

}


/*
 * Determine the route the curr_packet should take. 
 * 
 * Return index of interface the curr_packet should follow 
 * Return -1 if no interface has a matching route 
 */
int determine_route(struct ip_header *curr_packet, struct interface *interfaces, uint8_t num_interfaces, struct route *routing_table, uint8_t num_routes) 
{	

	uint32_t given_ip_dst_addr = convert_ip_addr_struct(curr_packet->dst_addr);
	
	uint32_t genmask_results = 0;
	int route_entry_results = -1;

	uint32_t curr_genmask;
	uint32_t curr_dst;


	for (int i = 0; i < num_routes; i++) {
				
		curr_genmask = convert_ip_addr_struct(routing_table[i].genmask);
		curr_dst = convert_ip_addr_struct(routing_table[i].dst);
		
		if ((given_ip_dst_addr & curr_genmask) == curr_dst) {
			if (curr_genmask > genmask_results) {
				genmask_results = curr_genmask;
				route_entry_results = i;
			}
		}

	}

	return route_entry_results;
}


/*
 * Sets mac_dst to the appropriate mac destination 
 *
 * Returns 1 if it found a 
 * Returns 0 if it could not find a route
 */
int determine_mac_from_ip(uint8_t *mac_dst, struct ip_address ip_addr, struct arp_entry *arp_cache, uint8_t num_arp_entries)
{

	for (int i = 0; i < num_arp_entries; i++) {
		
		// Found matching IP address
		if (compare_ip_addr_structs(ip_addr, arp_cache[i].ip_addr) == 1) {
			memcpy(mac_dst, arp_cache[i].ether_addr, 6);
			printf("FOUND MATCHING DEVICE\n");
			return 1;
		}
	
	}
	
	return 0;

}






