/*
 * stack.c
 *
 * Network Topology 
 *
 */

#include "stack.h"

struct interface *interfaces;
struct arp_entry *arp_cache;
struct route *routing_table;
int fds[NUM_INTERFACES][2];

int main(int argc, char *argv[])
{
	// Variables for vde_switch (one for each interface)
	char vde_path[20];
    int connect_to_remote_switch = 0;
    char *local_vde_cmd[] = { "vde_plug", NULL, NULL };
	char *remote_vde_cmd[] = { "ssh", "pjohnson@weathertop.cs.middlebury.edu",
                                      "/home/pjohnson/cs431/bin/vde_plug",
                                      NULL };
    char **vde_cmd = connect_to_remote_switch ? remote_vde_cmd : local_vde_cmd;

	
	// Connect to vde virtual switches for all interfaces	
	for (int i = 0; i < NUM_INTERFACES; i++) {
		
		sprintf(vde_path, "/tmp/net%d.vde", i);
		local_vde_cmd[1] = vde_path;
		vde_cmd = connect_to_remote_switch ? remote_vde_cmd : local_vde_cmd;
	
		// Connecting to vde virtual switch
		if (connect_to_vde_switch(fds[i], vde_cmd) < 0) {
			
			printf("Could not connect to switch, exiting.\n");
			exit(1);
		
		}
	
	}
	
	// Initialize interfaces, arp cache, and routing table
	init_interfaces(&interfaces);
	init_routing_table(&routing_table);
	init_arp_cache(&arp_cache); 

	// Process frames until user terminates with Control-C
	// (Assignment 3 Part I: Only listening on interface 0)
	while(1) {
	
		handle_ethernet_frame(&interfaces[0]);
	
	}

	free(interfaces);
	free(arp_cache);
	free(routing_table);

    return 0;

}

/*
 * Initialize interfaces with hardcoded values
 */
void init_interfaces(struct interface **interfaces) 
{
	
	if ((*interfaces = (struct interface *) malloc(NUM_INTERFACES * sizeof(struct interface))) == NULL) {
        printf("malloc failed\n");
		return;
	}

    // Interface 0 - WILL BE RECIEVING IN PART I
    memcpy((*interfaces)[0].ether_addr, "\x01\x02\x03\x04\xff\xff", 6);
    memcpy((*interfaces)[0].ip_addr, "\x01\x02\x03\x04", 4);
	(*interfaces)[0].name = "i0";
	(*interfaces)[0].in_fd = fds[0][0];
	(*interfaces)[0].out_fd = fds[0][1];

    // Interface 1
    memcpy((*interfaces)[1].ether_addr, "\x05\x06\x07\x08\xff\xff", 6);
    memcpy((*interfaces)[1].ip_addr, "\x05\x06\x07\x08", 4);
	(*interfaces)[1].name = "i1";
	(*interfaces)[1].in_fd = fds[1][0];
	(*interfaces)[1].out_fd = fds[1][1];

    // Interface 2
    memcpy((*interfaces)[2].ether_addr, "\x09\x0a\x0b\x0c\xff\xff", 6);
    memcpy((*interfaces)[2].ip_addr, "\x09\x0a\x0b\x0c", 4);
	(*interfaces)[2].name = "i2";
	(*interfaces)[2].in_fd = fds[2][0];
	(*interfaces)[2].out_fd = fds[2][1];

    // Interface 3
    memcpy((*interfaces)[3].ether_addr, "\x0d\x0e\x0f\x10\xff\xff", 6);
    memcpy((*interfaces)[3].ip_addr, "\x0d\x0e\x0f\x10", 4);
	(*interfaces)[3].name = "i3";
	(*interfaces)[3].in_fd = fds[3][0];
	(*interfaces)[3].out_fd = fds[3][1];

}


/*
 * Initialize routing table with hard coded routes 
 */
void init_routing_table(struct route **routing_table) 
{	
	
	if ((*routing_table = (struct route *) malloc(NUM_ROUTES * sizeof(struct route))) == NULL) {
		printf("malloc failed\n");
		return;
	}
	
	// Route 0 (interface 0)
	(*routing_table)[0].num_interface = 0;
	memcpy((*routing_table)[0].dst, "\x01\x02\x03\x00", 4);
	memcpy((*routing_table)[0].gateway, "\x00\x00\x00\x00", 4);
	memcpy((*routing_table)[0].genmask, "\xff\xff\xff\x00", 4);

	// Route 1 (interface 1)
	(*routing_table)[1].num_interface = 1;
	memcpy((*routing_table)[1].dst, "\x05\x06\x07\x00", 4);	
	memcpy((*routing_table)[1].gateway, "\x00\x00\x00\x00", 4);
	memcpy((*routing_table)[1].genmask, "\xff\xff\xff\x00", 4);
	
	// Route 2 (interface 2)
	(*routing_table)[2].num_interface = 2;
	memcpy((*routing_table)[2].dst, "\x09\x0a\x0b\x00", 4);
	memcpy((*routing_table)[2].gateway, "\x00\x00\x00\x00", 4);
	memcpy((*routing_table)[2].genmask, "\xff\xff\xff\x00", 4);

	// Route 3 (interface 3)
	(*routing_table)[3].num_interface = 3;
	memcpy((*routing_table)[3].dst, "\x0d\x0e\x0f\x00", 4);
	memcpy((*routing_table)[3].gateway, "\x00\x00\x00\x00", 4);
	memcpy((*routing_table)[3].genmask, "\xff\xff\xff\x00", 4);
	
	// Route 4 (to another router through interface 3)
	(*routing_table)[4].num_interface = 3;
	memcpy((*routing_table)[4].dst, "\x11\x12\x13\x00", 4);
	memcpy((*routing_table)[4].gateway, "\x0d\x0e\x0f\x11", 4);
	memcpy((*routing_table)[4].genmask, "\xff\xff\xff\x00", 4);

	// Route 5 (to another router through interface 2)
	(*routing_table)[5].num_interface = 2;
    memcpy((*routing_table)[5].dst, "\x0b\x0b\x00\x00", 4);
    memcpy((*routing_table)[5].gateway, "\x09\x0a\x0b\x0d", 4);
    memcpy((*routing_table)[5].genmask, "\xff\xff\x00\x00", 4);	

}


/*
 * Initialize arp cache with hard coded values 
 */
void init_arp_cache(struct arp_entry **arp_cache) 
{	
	
	if ((*arp_cache = (struct arp_entry *) malloc(NUM_ARP_ENTRIES * sizeof(struct arp_entry))) == NULL) {
		printf("malloc failed\n");
		return;
	}
	
	// Device A
	memcpy((*arp_cache)[0].ether_addr, "\x11\x22\x33\x00\xff\xff", 6);
	memcpy((*arp_cache)[0].ip_addr, "\x01\x02\x03\x00", 4);

	// Device H
	memcpy((*arp_cache)[1].ether_addr, "\xdd\xee\xff\x00\xff\xff", 6);
	memcpy((*arp_cache)[1].ip_addr, "\x0d\x0e\x0f\x00", 4);

	// Router 1 (connected to I3) 
	memcpy((*arp_cache)[2].ether_addr, "\xdd\xee\xff\x11\xff\xff", 6);
	memcpy((*arp_cache)[2].ip_addr, "\x0d\x0e\x0f\x11", 4);

}

/*
 * Received an ethernet frame from iface and check its integrity
 *
 * Return -1 for invalid frames
 * Return 0 otherwise
 */
int handle_ethernet_frame(struct interface *iface) 
{
	
	// Variables for frame
	uint8_t frame[1600];
    ssize_t frame_len;
	
	// Variables for processing recieved frames
	struct ether_header *curr_frame;
	ssize_t data_len;
	uint32_t *fcs_ptr;
	uint8_t *payload;
	int payload_len;
	int ether_dst_addr_results;

	// Read frame from interface !!!! NEED TO CHANGE THIS !!!! 
	frame_len = receive_ethernet_frame(iface->in_fd, frame);
	printf("received %ld-byte frame on interface 0\n", frame_len);

	// Set header information
	curr_frame = (struct ether_header *) frame;

	// Verify length of frame 
	if(is_valid_frame_length(frame_len) == 0) {
	
		return -1;

	}

	// Get data length and set ptr to given fcs 
	data_len = frame_len - sizeof(struct ether_header) - sizeof(*fcs_ptr); 
	fcs_ptr = (uint32_t *)(frame + sizeof(struct ether_header) + data_len);

	// Verify fcs
	if (is_valid_fcs(&frame, frame_len, data_len, *fcs_ptr) == 0) {
	
		return -1;

	}

	// Set payload 
	payload = frame + sizeof(struct ether_header);
	payload_len = frame_len - sizeof(struct ether_header) - ETHER_FCS_SIZE;
	
	// Acknowledge broadcasts and set ether_dst_addr_results
	ether_dst_addr_results = check_ether_dst_addr(curr_frame, frame_len, *iface);
	
	// Received broadcast
	if (ether_dst_addr_results == 0) {
		
		// Ethernet is ARP
		if (memcmp(curr_frame->type, ETHER_TYPE_ARP, 2) == 0) {
		
			printf("  has type ARP\n");
			handle_arp_packet(curr_frame->src, iface, payload, payload_len);	
			return 0;
		
		} else {
	
			printf("  ignoring %lu-byte frame (unrecognized type)\n", frame_len); 
			return -1; 
	
		}

	// Received ethernet frame for me  
	} else if (ether_dst_addr_results == 1) {
		
		// Ethernet is IPv4
		if (memcmp(curr_frame->type, ETHER_TYPE_IP, 2) == 0) {
		
			printf("  has type IP\n"); 	
			handle_ip_packet(iface, payload, payload_len);
			return 0;
	
		} else {
	
			printf("  ignoring %lu-byte frame (unrecognized type)\n", frame_len); 
			return -1; 
	
		}

	// Received ethernet frame not for me 
	} else {
		
		return -1;
	
	}
		
}




/*
 * Return 1 if the frame has a valid length and 0 otherwise 
 */
int is_valid_frame_length(ssize_t frame_len) 
{
	// Frame length too small
	if (frame_len < ETHER_MIN_FRAME_SIZE) {
		
		printf("  ignoring %lu-byte frame (short)\n", frame_len);
		
		return 0;
	
	// Frame length too large
	} else if (frame_len > ETHER_MAX_FRAME_SIZE) {
		
		printf("  ignoring %lu-byte frame (long)\n", frame_len);
		
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
		
		printf("  ignoring %ld-byte frame (bad fcs: got 0x%08x, expected 0x%08x)\n", frame_len, fcs, calculated_fcs);
		
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
int check_ether_dst_addr(struct ether_header *curr_frame, ssize_t frame_len, struct interface iface) 
{

	// Check if frame is a broadcast 
	if (memcmp(curr_frame->dst, ETHER_BROADCAST_ADDR, 6) == 0) {
		
		printf("  received %lu-byte broadcast frame from %s", frame_len, binary_to_hex(curr_frame->src, 6)); 
		
		return 0;
	
	// Check if frame is for the receiving interface 
	} else if (memcmp(curr_frame->dst, iface.ether_addr, 6) == 0) {
		
		// Can return immediately since MAC addresses unique
		return 1; 
	
	}
	
	// Frame is not for any of my interfaces
	printf("  ignoring %lu-byte frame (not for me)\n", frame_len); 
	
	return -1;

}

/*
 * Copy data from new_ether_header and data to frame to create a 
 * new ethernet frame ready for sending. 
 *
 * Return the size of the constructed ethernet frame.
 * Return -1 if the data is too large
 */
int compose_ether_frame(uint8_t *frame, struct ether_header *new_ether_header, uint8_t *data, size_t data_size) 
{
	uint32_t fcs; 

	if (data_size > ETHER_MAX_DATA_SIZE) {
		return -1;
	}

	memcpy(frame, new_ether_header, sizeof(*new_ether_header));
	memcpy(frame + sizeof(*new_ether_header), data, data_size);

	// Pad with zeros if data too short
	if (data_size < ETHER_MIN_DATA_SIZE) {
		memset(frame + sizeof(*new_ether_header) + data_size, '\0', ETHER_MIN_DATA_SIZE - data_size);
		data_size = ETHER_MIN_DATA_SIZE;
	}

	fcs = crc32(0, frame, sizeof(struct ether_header) + data_size);
	memcpy(frame + sizeof(*new_ether_header) + data_size, &fcs, ETHER_FCS_SIZE);

	return sizeof(*new_ether_header) + data_size + ETHER_FCS_SIZE;
}

/*
 * Handle and check integrity of ARP packets
 *
 * Return -1 for invalid packets or packets with specifications not handling 
 * Return 0 otherwise
 */
int handle_arp_packet(uint8_t *src, struct interface *iface, uint8_t *packet, int packet_length) 
{
	
	// Variables for processing ARP types
	struct arp_packet *curr_arp_packet;
	uint16_t given_opcode;

	// Variables for sending a response
	struct ether_header new_ether_header;
	uint8_t frame[ETHER_MAX_FRAME_SIZE];
	size_t frame_len;

	curr_arp_packet = (struct arp_packet *) packet;
	
	// Verify hardware type (only handling ethernet hardware) 
	if (memcmp(curr_arp_packet->hardware_type, "\x00\x01", 2) == 0) {
		
		// Verify the hardware size for ethernet type
		if (memcmp(&curr_arp_packet->hardware_size, "\x06", 1) != 0) {
			
			printf("    ignoring arp packet with bad hardware size from %s", binary_to_hex(curr_arp_packet->sender_mac_addr, 6));
			return -1;
		
		}

	} else {
		
		printf("    ignoring arp packet with incompatible hardware type from %s", binary_to_hex(curr_arp_packet->sender_mac_addr, 6));
		return -1;
	
	}


	// Verify protocol type (only handling IPv4) 
	if (memcmp(curr_arp_packet->protocol_type, "\x08\x00", 2) == 0) {
		
		// Verify the protocol size for ethernet type
		if (memcmp(&curr_arp_packet->protocol_size, "\x04", 1) != 0) {
			
			printf("    ignoring arp packet with bad protocol size from %s", binary_to_hex(curr_arp_packet->sender_mac_addr, 6));
			return -1;
		
		}

	} else {
		
		printf("    ignoring arp packet with incompatible protocol type from %s", binary_to_hex(curr_arp_packet->sender_mac_addr, 6));
		return -1;
	
	}
	
	// Respond to requests 
	given_opcode = ntohs(curr_arp_packet->opcode);
	
	// Verify the opcode is a request
	if (given_opcode == 1) {

		// DO WE HAVE TO VERIFY ANYTHING ABOUT THE TARGET OR SOURCE IP ADDRS? 

		// Only respond to ARP requests that corresponds to my listening interface
		if (memcmp(curr_arp_packet->target_ip_addr, iface->ip_addr, 4) == 0) {
			
			// Rewrite ethernet frame
			memcpy(new_ether_header.dst, src, 6);
			memcpy(new_ether_header.src, iface->ether_addr, 6);
			memcpy(new_ether_header.type, ETHER_TYPE_ARP, 2);

			// Set opcode to reply
			curr_arp_packet->opcode = htons(0x0002);
			
			// Set target as the given sender
			memcpy(curr_arp_packet->target_mac_addr, curr_arp_packet->sender_mac_addr, 6);
			memcpy(curr_arp_packet->target_ip_addr, curr_arp_packet->sender_ip_addr, 4);
			
			// Set sender as the interface
			memcpy(curr_arp_packet->sender_mac_addr, iface->ether_addr, 6);
			memcpy(curr_arp_packet->sender_ip_addr, iface->ip_addr, 4);
			
			frame_len = compose_ether_frame(frame, &new_ether_header, packet, packet_length);

			// Send to corresponding fd for vde switch connected to that interface 
			send_ethernet_frame(iface->out_fd, frame, frame_len);
			
			return 0;
		
		}

	} else {
		
		printf("    ignoring arp packet (only receiving requests)\n");
		return -1;
	
	}

	return 0;

}

/*
 * Handle and check integrity of IP packets 
 *
 * Return -1 if invalid 
 * Return 0 otherwise
 */
int handle_ip_packet(struct interface *iface, uint8_t *packet, int packet_length)
{

	struct ip_header *curr_packet;
	int ip_dst_addr_results;
			
	// Interpret data as IPv4
	curr_packet = (struct ip_header *) packet;
	
	// Check if total length is correct 
	if (packet_length != ntohs(curr_packet->total_length)) {
		
		printf("    dropping packet from %u.%u.%u.%u (wrong length)\n", curr_packet->src_addr[0],
																	    curr_packet->src_addr[1],
																	    curr_packet->src_addr[2],
																	    curr_packet->src_addr[3]);

		return -1;
	
	}

	// Check if the header checksum is correct 
	if (is_valid_ip_checksum(curr_packet) == 0) {	
		
		return -1;

	}
	
	// Check IHL 
	if (is_valid_ihl(curr_packet) == 0) {
		
		return -1;

	}
	

	// Check if provided recognized IP version (only IPv4)
	if (is_valid_ip_version(curr_packet) == 0) {
		
		return -1;

	}

	// Check if valid TTL 
	if(curr_packet->ttl == 0) {
		printf("    dropping packet from %u.%u.%u.%u to %u.%u.%u.%u (TTL exceeded)\n", curr_packet->src_addr[0], 
																					   curr_packet->src_addr[1], 
																					   curr_packet->src_addr[2], 
																					   curr_packet->src_addr[3], 
																					   curr_packet->dst_addr[0], 
																					   curr_packet->dst_addr[1], 
																					   curr_packet->dst_addr[2], 
																					   curr_packet->dst_addr[3]);

		//send_icmp_message(frame, frame_len, 11, 0, fds, interfaces);

		return -1;
	}


	// Get ip destination and check if it has a valid TTL accordingly 
		
	// Check ip destination 
	ip_dst_addr_results = check_ip_dst(curr_packet, interfaces);


	// curr_packet destined for one of my interfaces
	if (ip_dst_addr_results != -1) {
		
		printf("  delivering locally: received packet from %u.%u.%u.%u for %u.%u.%u.%u (interface %d)\n", curr_packet->src_addr[0],
																										  curr_packet->src_addr[1],
																										  curr_packet->src_addr[2],
																										  curr_packet->src_addr[3], 
																					    interfaces[ip_dst_addr_results].ip_addr[0], 
																					    interfaces[ip_dst_addr_results].ip_addr[1],
																					    interfaces[ip_dst_addr_results].ip_addr[2],
																					    interfaces[ip_dst_addr_results].ip_addr[3], 
																											   ip_dst_addr_results); 
		return 0;

	}
		
	// curr_packet not for one of my interfaces (needs routing) 
	return route_ip_packet(packet, packet_length);

}


/*
 * Find correct route for IP packet and forward it accordingly 
 *
 * Return -1 if no route is found
 * Return 0 otherwise
 */
int route_ip_packet(uint8_t *packet, size_t packet_length)
{
	
	int route_entry_num = -1; // Number entry in routing_table to take
	struct route route_to_take;
	int found_mac_addr = 0;
	uint8_t mac_dst[6];

	struct ether_header new_ether_header;
	uint8_t frame[ETHER_MAX_FRAME_SIZE];
	size_t frame_len;

	struct ip_header *curr_packet = (struct ip_header *) packet;

			
	// Determine whether there is a valid route in routing table
	route_entry_num = determine_route(curr_packet, interfaces, routing_table); 

	// No corresponding entry in routing table for IP packet
	if (route_entry_num == -1) {
		
		printf("    dropping packet from %u.%u.%u.%u to %u.%u.%u.%u (no route)\n", curr_packet->src_addr[0],
																			       curr_packet->src_addr[1], 
																			       curr_packet->src_addr[2], 
																			       curr_packet->src_addr[3], 
																				   curr_packet->dst_addr[0], 
																			       curr_packet->dst_addr[1], 
																			       curr_packet->dst_addr[2], 
																			       curr_packet->dst_addr[3]);
		
		//send_icmp_message(frame, frame_len, 3, 0, fds, interfaces);
		
		return -1;

	}

	// Found corresponding entry in routing table for IP packet, now find MAC address for next hop
	route_to_take = routing_table[route_entry_num];
	
	// If gateway of route is 0.0.0.0 (meaning destination device is in one of networks router is connected to)
	// then we can send the ethernet frame directly to the device 
	if (memcmp(route_to_take.gateway, DIRECT_NETWORK_GATEWAY, 4) == 0) {	
		
		printf("    destination host is on attached network\n");

		// Find dst mac address to the current packets ip dest in arp cache 
		found_mac_addr = determine_mac_from_ip(mac_dst, curr_packet->dst_addr, arp_cache);
	
	// If gateway of route is not 0.0.0.0, then we have to send packet to another router 
	} else {
		
		printf("    packet must be routed\n");

		// Find dst mac address to the current packets ip dest in arp cache 
		found_mac_addr = determine_mac_from_ip(mac_dst, route_to_take.gateway, arp_cache);
		
	}

	// Could not find corresponding mac address for route
	if (found_mac_addr == 0) {
		
		printf("    dropping packet from %u.%u.%u.%u to %u.%u.%u.%u (no ARP)\n", curr_packet->src_addr[0],
																			     curr_packet->src_addr[1], 
																			     curr_packet->src_addr[2], 
																			     curr_packet->src_addr[3], 
																			     curr_packet->dst_addr[0], 
																			     curr_packet->dst_addr[1], 
																			     curr_packet->dst_addr[2], 
																			     curr_packet->dst_addr[3]);
		
		//send_icmp_message(frame, frame_len, 3, 1, fds, interfaces);
		
		return -1;

	}

	
	// Rewrite the frame and forward it to next hop 
	
	// Set frame dst to mac address found in arp cache
	memcpy(new_ether_header.dst, mac_dst, 6);
	// Set frame src to corresponding leaving interface
	memcpy(new_ether_header.src, interfaces[route_to_take.num_interface].ether_addr, 6); 
	memcpy(new_ether_header.type, ETHER_TYPE_IP, 2);

	// Update the TTL 
	curr_packet->ttl = curr_packet->ttl - 1;

	// Recalculate IP header checksum since changed TTL
	curr_packet->header_checksum = 0;
	curr_packet->header_checksum = checksum(curr_packet, (curr_packet->version_and_ihl & 0x0f) * 4);

	frame_len = compose_ether_frame(frame, &new_ether_header, packet, packet_length);

	// Send to corresponding fd for vde switch connected to the interface
	send_ethernet_frame(fds[route_to_take.num_interface][1], frame, frame_len);	
	
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

	if (given_checksum != checksum(curr_packet, (given_ihl * 32) / 8)) {
		
		printf("    dropping packet from %u.%u.%u.%u (bad IP header checksum)\n", curr_packet->src_addr[0], 
																			  curr_packet->src_addr[1], 
																			  curr_packet->src_addr[2], 
																			  curr_packet->src_addr[3]);
		
		return 0;
	
	}

	// Reset the header chucksum back (mainly needed for ICMP) 
	curr_packet->header_checksum = given_checksum;

	return 1;

}


/*
 * Check if given IHL is valid (greater than 5)
 *
 * Return 1 if IHL is greater than 5
 * Return 0 otherwise
 */
int is_valid_ihl(struct ip_header *curr_packet) 
{

	if ((curr_packet->version_and_ihl & 0x0f) < 5) {
		
		printf("    dropping packet from %u.%u.%u.%u (invalid IHL)\n", curr_packet->src_addr[0],
																   curr_packet->src_addr[1],
																   curr_packet->src_addr[2],
																   curr_packet->src_addr[3]);
		return 0;
	
	}

	return 1;

}


/*
 * Check if given IP version is valid (is IPv4)
 *
 * Return 1 if IP version is 4
 * Return 0 otherwise
 */
int is_valid_ip_version(struct ip_header *curr_packet) 
{

	// Get given version (high nibble)
	if (((curr_packet->version_and_ihl & 0xf0) >> 4) != 4) {
		
		printf("    dropping packet from %u.%u.%u.%u (unrecognized IP version)\n", curr_packet->src_addr[0], 
																			   curr_packet->src_addr[1], 
																			   curr_packet->src_addr[2], 
																			   curr_packet->src_addr[3]);
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
int check_ip_dst(struct ip_header *curr_packet, struct interface *interfaces) 
{
	
	for (int i = 0; i < NUM_INTERFACES; i++) {
		
		// Check if packet is for one of my interfaces 
		if (memcmp(curr_packet->dst_addr, interfaces[i].ip_addr, 4) == 0) {
	
			// Can return immediately since IP addresses are unique
			return i; 
		
		} 
	
	}

	// curr_packet not for any of my interfaces
	return -1;

}


/*
 * Convert uint8_t array of 4 to a uint32_t for bit comparison purposes.
 * Return the converted value. 
 */
uint32_t array_to_uint32(uint8_t array[4]) 
{
    
	uint32_t result = ((uint32_t)array[0] << 24) | ((uint32_t)array[1] << 16) | ((uint32_t)array[2] << 8) | ((uint32_t)array[3]);
    
	return result;
}

/*
 * Determine the route the curr_packet should take. 
 * 
 * Return index of interface the curr_packet should follow 
 * Return -1 if no interface has a matching route 
 */
int determine_route(struct ip_header *curr_packet, struct interface *interfaces, struct route *routing_table) 
{	

	uint32_t given_ip_dst_addr = array_to_uint32(curr_packet->dst_addr);

	uint32_t genmask_results = 0;
	int route_entry_results = -1;

	uint32_t curr_genmask;
	uint32_t curr_dst;


	for (int i = 0; i < NUM_ROUTES; i++) {
				
		curr_genmask = array_to_uint32(routing_table[i].genmask);
		curr_dst = array_to_uint32(routing_table[i].dst);
		
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
 * Returns 1 if it found a successful ip/mac match
 * Returns 0 if it could not find a match
 */
int determine_mac_from_ip(uint8_t *mac_dst, uint8_t *ip_addr, struct arp_entry *arp_cache)
{

	for (int i = 0; i < NUM_ARP_ENTRIES; i++) {
		
		// Found matching IP address
		if (memcmp(ip_addr, arp_cache[i].ip_addr, 4) == 0) {
			
			memcpy(mac_dst, arp_cache[i].ether_addr, 6);
			
			return 1;
		
		}
	
	}
	
	return 0;

}

/*
 * Send an ICMP message for TLL exceeded, network unreachable or host unreachable using the original frame. 
 */
void send_icmp_message(uint8_t frame[1600], ssize_t frame_len, uint8_t type, uint8_t code, int (*fds)[2], struct interface *interfaces)
{
	// Set ethernet header information
	struct ether_header *curr_frame = (struct ether_header *) frame;
	
	// Set IP header information
	struct ip_header *curr_packet = (struct ip_header *) (frame + sizeof(struct ether_header));
	
	// Variables for sending ICMP 
	struct icmp_header new_icmp;
	uint32_t icmp_fcs;

	// Start overwriting the current frame to send an ICMP message 
	
	// Rewrite ethernet header 
	memcpy(curr_frame->dst, curr_frame->src, 6);
	memcpy(curr_frame->src, interfaces[RECEIVING_INTERFACE].ether_addr, 6);
	
	// Write to ICMP before rewriting IP header since we need that information 
	memset(&new_icmp, '\x00', sizeof(struct icmp_header)); // set new_icmp to avoid keeping unwanted data from original frame
	new_icmp.type = type;
	new_icmp.code = code;
	memcpy(&new_icmp.original_ip_header, curr_packet, sizeof(struct ip_header));
	memcpy(&new_icmp.original_data, (uint8_t *)curr_packet + sizeof(struct ip_header), sizeof(new_icmp.original_data));
	new_icmp.checksum = 0;
	new_icmp.checksum = checksum(&new_icmp, sizeof(struct icmp_header)); // does this need to be converted from hton
	memcpy(frame + sizeof(struct ether_header) + sizeof(struct ip_header), &new_icmp, sizeof(struct icmp_header));
	
	// Rewrite IP header
	memcpy(curr_packet->dst_addr, curr_packet->src_addr, 4);
	memcpy(curr_packet->src_addr, interfaces[RECEIVING_INTERFACE].ether_addr, 4);
	curr_packet->total_length = htons(sizeof(struct ip_header) + sizeof(struct icmp_header)); // CHECK THIS
	curr_packet->ttl = curr_packet->ttl - 1; // SINCE TTL EXCEEDED SHOULD THIS STILL BE new_ttl OR SHOULD IT RESET??? 
	curr_packet->protocol = 1;
	curr_packet->header_checksum = 0; // Reset for accurate recalculation
	curr_packet->header_checksum = checksum(&(*curr_packet), ((curr_packet->version_and_ihl & 0x0f) * 32) / 8);
	// IDENTIFICATION????
	
	// Recalculate FCS
	frame_len = sizeof(struct ether_header) + sizeof(struct ip_header) + sizeof(struct icmp_header) + sizeof(icmp_fcs);
	icmp_fcs = crc32(0, frame, frame_len - sizeof(icmp_fcs));
	memcpy(frame + frame_len - sizeof(icmp_fcs), &icmp_fcs, sizeof(icmp_fcs));

	// Send to corresponding fd for vde switch connected to that interface
	send_ethernet_frame(fds[RECEIVING_INTERFACE][1], frame, frame_len);
	
}



