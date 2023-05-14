/* 
 * stack.c
 *
 * Network Configuration
 * - i0 on net0: 1.2.3.4/24
 * - i1 on net1: 5.6.7.8/24
 * - i2 on net2: 9.10.11.12/24
 * - i3 on net3: 13.14.15.16/24
 * - net4: 17.18.19.0/24 through gateway 13.14.15.17 
 * - net5: 11.11.0.0/16 through gateway 9.10.11.13 (no ARP though) 
 */

#include "stack.h"

struct interface interfaces[NUM_INTERFACES];
struct arp_entry arp_cache[NUM_ARP_ENTRIES];
struct route routing_table[NUM_ROUTES];
int fds[NUM_INTERFACES][2];
struct pollfd poll_fds[NUM_INTERFACES];
struct tcb connections[MAX_CONNECTIONS];
int num_connections = 0;


int main(int argc, char *argv[])
{

	int poll_results;

	init_fds();
	init_interfaces();
	init_poll_fds();
	init_routing_table();
	init_arp_cache(); 

	// Process frames until user terminates with Control-C
	while(1) {
		
		poll_results = poll(poll_fds, NUM_INTERFACES, INFTIM);
		
		if (poll_results == -1) {

			perror("poll failed");
		
		}
		
		for (int i = 0; i < NUM_INTERFACES; i++) {
			
			if (poll_fds[i].revents & POLLIN) {
		
				handle_ethernet_frame(&interfaces[i]);

			}	
		
		}

	}

    return 0;

}

/*
 * Initialize the fds for sending and receiving
 */
void init_fds() 
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

}


/*
 * Initialize interfaces with hardcoded values
 */
void init_interfaces() 
{
	
    // Interface 0 - WILL BE RECIEVING IN PART I
	//memcpy(interfaces[0].ether_addr, "\x01\x02\x03\x04\xff\xff", 6);
	memcpy(interfaces[0].ether_addr, "\x06\xdd\x79\xe0\x8b\x4d", 6);
	memcpy(interfaces[0].ip_addr, "\x01\x02\x03\x04", 4);
	interfaces[0].name = "i0";
	interfaces[0].in_fd = fds[0][0];
	interfaces[0].out_fd = fds[0][1];

    // Interface 1
    memcpy(interfaces[1].ether_addr, "\x05\x06\x07\x08\xff\xff", 6);
    memcpy(interfaces[1].ip_addr, "\x05\x06\x07\x08", 4);
	interfaces[1].name = "i1";
	interfaces[1].in_fd = fds[1][0];
	interfaces[1].out_fd = fds[1][1];

    // Interface 2
    memcpy(interfaces[2].ether_addr, "\x09\x0a\x0b\x0c\xff\xff", 6);
    memcpy(interfaces[2].ip_addr, "\x09\x0a\x0b\x0c", 4);
	interfaces[2].name = "i2";
	interfaces[2].in_fd = fds[2][0];
	interfaces[2].out_fd = fds[2][1];

    // Interface 3
    memcpy(interfaces[3].ether_addr, "\x0d\x0e\x0f\x10\xff\xff", 6);
    memcpy(interfaces[3].ip_addr, "\x0d\x0e\x0f\x10", 4);
	interfaces[3].name = "i3";
	interfaces[3].in_fd = fds[3][0];
	interfaces[3].out_fd = fds[3][1];

}


/*
 * Initialize each struct poll fd in poll_fds 
 * using each interfaces' in_fd
 */
void init_poll_fds()
{
	
	for (int i = 0; i < NUM_INTERFACES; i++) {
		
		poll_fds[i].fd = interfaces[i].in_fd;
		poll_fds[i].events = POLLIN;

	}

}

/*
 * Initialize routing table with hard coded routes 
 */
void init_routing_table() 
{	
	
	// Route 0 (interface 0)
	routing_table[0].iface = &interfaces[0];
	memcpy(routing_table[0].dst, "\x01\x02\x03\x00", 4);
	memcpy(routing_table[0].gateway, "\x00\x00\x00\x00", 4);
	memcpy(routing_table[0].genmask, "\xff\xff\xff\x00", 4);

	// Route 1 (interface 1)
	routing_table[1].iface = &interfaces[1];
	memcpy(routing_table[1].dst, "\x05\x06\x07\x00", 4);	
	memcpy(routing_table[1].gateway, "\x00\x00\x00\x00", 4);
	memcpy(routing_table[1].genmask, "\xff\xff\xff\x00", 4);
	
	// Route 2 (interface 2)
	routing_table[2].iface = &interfaces[2];
	memcpy(routing_table[2].dst, "\x09\x0a\x0b\x00", 4);
	memcpy(routing_table[2].gateway, "\x00\x00\x00\x00", 4);
	memcpy(routing_table[2].genmask, "\xff\xff\xff\x00", 4);

	// Route 3 (interface 3)
	routing_table[3].iface= &interfaces[3];
	memcpy(routing_table[3].dst, "\x0d\x0e\x0f\x00", 4);
	memcpy(routing_table[3].gateway, "\x00\x00\x00\x00", 4);
	memcpy(routing_table[3].genmask, "\xff\xff\xff\x00", 4);
	
	// Route 4 (to another router through interface 3)
	routing_table[4].iface = &interfaces[3];
	memcpy(routing_table[4].dst, "\x11\x12\x13\x00", 4);
	memcpy(routing_table[4].gateway, "\x0d\x0e\x0f\x11", 4);
	memcpy(routing_table[4].genmask, "\xff\xff\xff\x00", 4);

	// Route 5 (to another router through interface 2)
	routing_table[5].iface = &interfaces[2];
    memcpy(routing_table[5].dst, "\x0b\x0c\x00\x00", 4);
    memcpy(routing_table[5].gateway, "\x09\x0a\x0b\x0d", 4);
    memcpy(routing_table[5].genmask, "\xff\xff\x00\x00", 4);	

}


/*
 * Initialize arp cache with hard coded values 
 */
void init_arp_cache() 
{	
	
	// Device A
	memcpy(arp_cache[0].ether_addr, "\x11\x22\x33\x00\xff\xff", 6);
	memcpy(arp_cache[0].ip_addr, "\x01\x02\x03\x00", 4);

	// Device H
	memcpy(arp_cache[1].ether_addr, "\xdd\xee\xff\x00\xff\xff", 6);
	memcpy(arp_cache[1].ip_addr, "\x0d\x0e\x0f\x00", 4);

	// Router 1 (connected to I3) 
	memcpy(arp_cache[2].ether_addr, "\xdd\xee\xff\x11\xff\xff", 6);
	memcpy(arp_cache[2].ip_addr, "\x0d\x0e\x0f\x11", 4);
	
	// tap0 FreeBSD
	memcpy(arp_cache[3].ether_addr, "\x58\x9c\xfc\x00\x07\x6b", 6);
	memcpy(arp_cache[3].ip_addr, "\x01\x02\x03\x2d", 4);
}

/*
 * Check the integrity of an ethernet frame received on interface iface
 *
 * Return -1 for invalid frames
 * Return 0 otherwise
 */
int handle_ethernet_frame(struct interface *iface) 
{
	
	// Variables for frame
	uint8_t frame[ETHER_MAX_FRAME_SIZE];
    ssize_t frame_len;
	
	// Variables for processing received frame
	struct ether_header *curr_frame;
	uint8_t *payload;
	int payload_len;
	int ether_dst_addr_results;
	int check_fcs = 1;
	
	// Read frame from interface  
	frame_len = receive_ethernet_frame(iface->in_fd, frame);
	printf("\nreceived %ld-byte frame on interface 0\n", frame_len);
	
	// DEBUGGING
	printf("%s", binary_to_hex(frame, frame_len));

	// Set header information
	curr_frame = (struct ether_header *) frame;

	// Verify length of frame 
	if (is_valid_frame_len(frame_len) == 0) {
	
		return -1;

	}
	
	// Check Ethernet type 
	if (memcmp(curr_frame->type, ETHER_TYPE_ARP, 2) == 0) {
		check_fcs = 0;
	} else if (memcmp(curr_frame->type, ETHER_TYPE_IP, 2) == 0) {
		if (((struct ip_header *)(frame + sizeof(struct ether_header)))->protocol == IP_TCP_PROTOCOL) {
			check_fcs = 0;
		}
	}

	// Verify fcs
	if (check_fcs) {
		if (is_valid_fcs(frame, frame_len) == 0) {
	
			return -1;

		}
	}

	// Set payload 
	payload = frame + sizeof(struct ether_header);
	
	// Set the frame length 
	if (check_fcs) {
		//printf("CHECKING THE FCS\n");
		payload_len = frame_len - sizeof(struct ether_header) - ETHER_FCS_SIZE;
	} else {
		//printf("NOT CHECKING THE FCS\n");
		payload_len = frame_len - sizeof(struct ether_header);
		//printf("FRAME LENGTH: %ld (%lx)\n", frame_len, frame_len);
		//printf("PAYLOAD LENGTH: %d (%x)\n", payload_len, payload_len);
	}
	
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
 * Check length of an ethernet frame
 *
 * Return 1 if the frame has a valid length
 * Return 0 otherwise 
 */
int is_valid_frame_len(ssize_t frame_len) 
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
 * Check if initial fcs in Ethernet frame is correct by recalculating it 
 *
 * Return 1 if fcs is correct
 * Return 0 otherwise 
 */
int is_valid_fcs (uint8_t *frame, size_t frame_len) 
{

	size_t data_len;	
	uint32_t *fcs_ptr;
	uint32_t calculated_fcs;

	// Get data length and set fcs_ptr to given fcs 
	data_len = frame_len - sizeof(struct ether_header) - ETHER_FCS_SIZE; 
	fcs_ptr = (uint32_t *)(frame + frame_len - ETHER_FCS_SIZE);

	// Verify fcs
	calculated_fcs = crc32(0, frame, sizeof(struct ether_header) + data_len);
		
	if (calculated_fcs != *fcs_ptr) {
		
		printf("  ignoring %ld-byte frame (bad fcs: got 0x%08x, expected 0x%08x)\n", 
												frame_len, *fcs_ptr, calculated_fcs);
		return 0;
	
	}

	return 1;

}



/*
 * Check whether ethernet destination address in curr_frame is for interface iface
 *
 * Return 0 if frame was a braodcast
 * Return 1 if frame was for me (i.e. interfaces' MAC address)
 * Return -1 if frame was not for me 
 */
int check_ether_dst_addr(struct ether_header *curr_frame, ssize_t frame_len, struct interface iface) 
{

	// Check if frame is a broadcast 
	if (memcmp(curr_frame->dst, ETHER_BROADCAST_ADDR, 6) == 0) {
		
		printf("  received %lu-byte broadcast frame from %s", 
				frame_len, binary_to_hex(curr_frame->src, 6)); 
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
 * Copy data from new_ether_header and data to frame and set the fcs to create a 
 * new ethernet frame ready for sending. 
 *
 * Return the size of the constructed ethernet frame
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
 * Handle and check integrity of ARP packet received on interface iface
 *
 * Return -1 for invalid packets or packets with specifications not handling 
 * Return 0 otherwise
 */
int handle_arp_packet(uint8_t *src, struct interface *iface, uint8_t *packet, int packet_len) 
{
	
	// Variables for processing ARP types
	struct arp_header *curr_arp_header;
	uint16_t given_opcode;

	curr_arp_header = (struct arp_header *) packet;

	// Verify hardware type (only handling ethernet hardware) 
	if (memcmp(curr_arp_header->hardware_type, ARP_TYPE_ETHER, 2) == 0) {
		
		// Verify the hardware size for ethernet type
		if (curr_arp_header->hardware_size != ARP_ETHER_SIZE) { 
			
			printf("    ignoring arp packet with bad hardware size from %s", 
						 binary_to_hex(curr_arp_header->sender_mac_addr, 6));
			return -1;
		
		}

	} else {
		
		printf("    ignoring arp packet with incompatible hardware type from %s", 
							  binary_to_hex(curr_arp_header->sender_mac_addr, 6));
		return -1;
	
	}


	// Verify protocol type (only handling IPv4) 
	if (memcmp(curr_arp_header->protocol_type, ARP_TYPE_IP, 2) == 0) {
		
		// Verify the protocol size for ethernet type
		if (curr_arp_header->protocol_size != ARP_IP_SIZE) {
			
			printf("    ignoring arp packet with bad protocol size from %s", 
						 binary_to_hex(curr_arp_header->sender_mac_addr, 6));
			return -1;
		
		}

	} else {
		
		printf("    ignoring arp packet with incompatible protocol type from %s", 
							  binary_to_hex(curr_arp_header->sender_mac_addr, 6));
		return -1;
	
	}


	// Respond to requests 
	given_opcode = ntohs(curr_arp_header->opcode);
	
	// Verify the opcode is a request
	if (given_opcode == 1) {
		
		send_arp_reply(src, iface, packet, packet_len);		
		
	} else {
		
		printf("    ignoring arp packet (only receiving requests)\n");
		return -1;
	
	}

	return 0;

}

/*
 * Send ARP reply if the ARP request corresponds with interface iface
 *
 * Return 0 if sent a reply
 * Return -1 if did not send a reply 
 */
int send_arp_reply(uint8_t *src, struct interface *iface, uint8_t *packet, int packet_len) 
{
	
	struct arp_header *curr_arp_header;
	
	// Variables for sending a response
	struct ether_header new_ether_header;
	uint8_t frame[ETHER_MAX_FRAME_SIZE];
	size_t frame_len;

	curr_arp_header = (struct arp_header *) packet;

	// DO WE HAVE TO VERIFY ANYTHING ABOUT THE TARGET OR SOURCE IP ADDRS? 

	// Only respond to ARP requests that corresponds to my listening interface
	if (memcmp(curr_arp_header->target_ip_addr, iface->ip_addr, 4) == 0) {
		
		// Rewrite ethernet frame
		memcpy(new_ether_header.dst, src, 6);
		memcpy(new_ether_header.src, iface->ether_addr, 6);
		memcpy(new_ether_header.type, ETHER_TYPE_ARP, 2);

		// Set opcode to reply
		curr_arp_header->opcode = htons(0x0002);
		
		// Set target as the given sender
		memcpy(curr_arp_header->target_mac_addr, curr_arp_header->sender_mac_addr, 6);
		memcpy(curr_arp_header->target_ip_addr, curr_arp_header->sender_ip_addr, 4);
		
		// Set sender as the interface
		memcpy(curr_arp_header->sender_mac_addr, iface->ether_addr, 6);
		memcpy(curr_arp_header->sender_ip_addr, iface->ip_addr, 4);
		
		frame_len = compose_ether_frame(frame, &new_ether_header, packet, packet_len);

		// Send to corresponding fd for vde switch connected to that interface 
		send_ethernet_frame(iface->out_fd, frame, frame_len);
		
		return 0;
	
	}

	return -1;

}



/*
 * Handle and check integrity of IP packet received on interface iface 
 *
 * Return -1 if invalid 
 * Return 0 otherwise
 */
int handle_ip_packet(struct interface *iface, uint8_t *packet, int packet_len)
{

	struct ip_header *curr_ip_header;
	struct interface *local_interface;
	uint8_t *payload; 
	int payload_len;

	// Interpret data as IPv4 header
	curr_ip_header = (struct ip_header *) packet;
	
	//printf("    PACKET_LEN: %d %x\n", packet_len, packet_len);	
	//printf("    GIVEN PACKET LEN: %d %x\n", ntohs(curr_ip_header->total_length), ntohs(curr_ip_header->total_length));
	
	// Check if total length is correct 
	if (packet_len < ntohs(curr_ip_header->total_length)) {
		printf("    dropping packet from %u.%u.%u.%u (wrong length)\n", curr_ip_header->src_addr[0],
																	    curr_ip_header->src_addr[1],
																	    curr_ip_header->src_addr[2],
																	    curr_ip_header->src_addr[3]);

		return -1;
	
	}

	// Check if the header checksum is correct 
	if (is_valid_ip_checksum(curr_ip_header) == 0) {
		
		return -1;

	}
	
	// Check IHL 
	if (is_valid_ihl(curr_ip_header) == 0) {
		
		return -1;

	}
	

	// Check if provided recognized IP version (only IPv4)
	if (is_valid_ip_version(curr_ip_header) == 0) {
		
		return -1;

	}

	// Check if valid TTL 
	if (is_valid_ttl(curr_ip_header) == 0) {
		
		send_icmp_message(packet, packet_len, 11, 0);
		return -1;

	}

	// Set payload
	payload = packet + sizeof(struct ip_header);
	//payload_len = packet_len - sizeof(struct ip_header);
	payload_len = ntohs(curr_ip_header->total_length) - sizeof(struct ip_header);

	// Check ip destination 
	local_interface = determine_local_interface(curr_ip_header);


	// curr_ip_header destined for one of my interfaces
	if (local_interface != NULL) {
		
		printf("  delivering locally: received packet from %u.%u.%u.%u for %u.%u.%u.%u (%s)\n", curr_ip_header->src_addr[0],
																								curr_ip_header->src_addr[1],
																								curr_ip_header->src_addr[2],
																								curr_ip_header->src_addr[3], 
																							    local_interface->ip_addr[0], 
																							    local_interface->ip_addr[1],
																							    local_interface->ip_addr[2],
																							    local_interface->ip_addr[3], 
																								   local_interface->name); 
		
		// Received a TCP packet for local interface
		if (curr_ip_header->protocol == IP_TCP_PROTOCOL) {
			
			handle_tcp_packet(curr_ip_header->src_addr, curr_ip_header->dst_addr, payload, payload_len);
		
		}

		return 0;


	}
		
	// curr_ip_header not for one of my interfaces (needs routing) 
	return route_ip_packet(packet, packet_len, 0);

}


/*
 * Find correct route for IP packet and forward it accordingly 
 *
 * Return -1 if no route is found
 * Return 0 otherwise
 */
int route_ip_packet(uint8_t *packet, size_t packet_len, int is_process)
{
	
	// Variables for routing
	struct route *route_to_take;
	struct arp_entry *corresponding_arp_entry;

	// Variables for sending the IP packet to its next hop
	struct ether_header new_ether_header;
	uint8_t frame[ETHER_MAX_FRAME_SIZE];
	size_t frame_len;


	// Interpret packet as IP header
	struct ip_header *curr_ip_header = (struct ip_header *) packet;
			
	// Determine whether there is a valid route in routing table
	route_to_take = determine_route(curr_ip_header); 

	// No corresponding entry in routing table for IP packet
	if (route_to_take == NULL) {	

		printf("    dropping packet from %u.%u.%u.%u to %u.%u.%u.%u (no route)\n", curr_ip_header->src_addr[0],
																			       curr_ip_header->src_addr[1], 
																			       curr_ip_header->src_addr[2], 
																			       curr_ip_header->src_addr[3], 
																				   curr_ip_header->dst_addr[0], 
																			       curr_ip_header->dst_addr[1], 
																			       curr_ip_header->dst_addr[2], 
																			       curr_ip_header->dst_addr[3]);
		if (!is_process) {
			
			send_icmp_message(packet, packet_len, 3, 0);

		} 

		return -1;

	}

	// If gateway of route is 0.0.0.0 (meaning destination device is in one of networks router is connected to)
	// then we can send the ethernet frame directly to the device 
	if (memcmp(route_to_take->gateway, DIRECT_NETWORK_GATEWAY, 4) == 0) {	
		
		printf("    destination host is on attached network\n");
		corresponding_arp_entry = determine_mac_arp(curr_ip_header->dst_addr);
	
	// If gateway of route is not 0.0.0.0, then we have to send packet to another router 
	} else {
		
		printf("    packet must be routed\n");
		corresponding_arp_entry = determine_mac_arp(route_to_take->gateway);
		
	}

	// Could not find corresponding mac address for route
	if (corresponding_arp_entry == NULL) {
	
		printf("    dropping packet from %u.%u.%u.%u to %u.%u.%u.%u (no ARP)\n", curr_ip_header->src_addr[0],
																			     curr_ip_header->src_addr[1], 
																			     curr_ip_header->src_addr[2], 
																			     curr_ip_header->src_addr[3], 
																			     curr_ip_header->dst_addr[0], 
																			     curr_ip_header->dst_addr[1], 
																			     curr_ip_header->dst_addr[2], 
																			     curr_ip_header->dst_addr[3]);
		if (!is_process) {
		
			send_icmp_message(packet, packet_len, 3, 1);

		} 

		return -1;

	}

	
	// Write ethernet header for next hop
	memcpy(new_ether_header.dst, corresponding_arp_entry->ether_addr, 6);
	// Set frame src to corresponding leaving interface
	memcpy(new_ether_header.src, route_to_take->iface->ether_addr, 6); 
	memcpy(new_ether_header.type, ETHER_TYPE_IP, 2);

	// Update the TTL if it is not an ICMP message 
	if (!is_process) {
	
		curr_ip_header->ttl = curr_ip_header->ttl - 1;
		
		// Do not emit packet with TTL of 0
		if (is_valid_ttl(curr_ip_header) == 0) {
			
			// Restore IP header to have original TTL again
			curr_ip_header->ttl = curr_ip_header->ttl + 1;

			send_icmp_message(packet, packet_len, 11, 0);
			return -1;

		}

	}
	
	// Recalculate IP header checksum 
	curr_ip_header->header_checksum = 0;
	curr_ip_header->header_checksum = checksum(curr_ip_header, (curr_ip_header->version_and_ihl & 0x0f) * 4);

	frame_len = compose_ether_frame(frame, &new_ether_header, packet, packet_len);

	// Send to corresponding fd for vde switch connected to the interface
	send_ethernet_frame(route_to_take->iface->out_fd, frame, frame_len);	
	
	return 0;

}

/*      
 * Copy data from ip_header and payload to packet and set the checksum to create a 
 * new IP packet. 
 *  
 * Return the size of the constructed IP packet
 */ 
int compose_ip_packet(uint8_t *packet, struct ip_header *ip_header, uint8_t *payload, size_t payload_len) 
{

	int ihl; 
	uint16_t total_len; 
	uint16_t header_checksum; 

	ihl = (ip_header->version_and_ihl & 0xf) * 4;
	total_len = ihl + payload_len; 
	ip_header->total_length = htons(total_len);

	memcpy(packet, ip_header, ihl);
	memcpy(packet + ihl, payload, payload_len);

	header_checksum = checksum((struct ip_header *)packet, ihl);
	((struct ip_header *) packet)->header_checksum = header_checksum;

	return total_len;
}


/*
 * Check if inital checksum in IP header is correct by recalculating it.
 *
 * Return 1 if checksum is correct
 * Return 0 otherwise
 */
int is_valid_ip_checksum(struct ip_header *curr_ip_header)
{

	uint16_t given_checksum;
	uint8_t given_ihl;

	// Get given IHL 
	given_ihl = curr_ip_header->version_and_ihl & 0x0f; // low nibble

	// Get given ip checksum 
	given_checksum = curr_ip_header->header_checksum;
	
	// Reset the header checksum to recalculate it correctly
	curr_ip_header->header_checksum = 0;

	if (given_checksum != checksum(curr_ip_header, given_ihl * 4)) {
		
		printf("    dropping packet from %u.%u.%u.%u (bad IP header checksum)\n", curr_ip_header->src_addr[0], 
																				  curr_ip_header->src_addr[1], 
																				  curr_ip_header->src_addr[2], 
																				  curr_ip_header->src_addr[3]);
		
		return 0;
	
	}

	// Reset the header chucksum back (mainly needed for ICMP) 
	curr_ip_header->header_checksum = given_checksum;

	return 1;

}


/*
 * Check if given IHL is valid (greater than 5)
 *
 * Return 1 if IHL is greater than 5
 * Return 0 otherwise
 */
int is_valid_ihl(struct ip_header *curr_ip_header) 
{

	if ((curr_ip_header->version_and_ihl & 0x0f) < 5) {
		
		printf("    dropping packet from %u.%u.%u.%u (invalid IHL)\n", curr_ip_header->src_addr[0],
																	   curr_ip_header->src_addr[1],
																	   curr_ip_header->src_addr[2],
																	   curr_ip_header->src_addr[3]);
		return 0;
	
	}

	return 1;

}

/*
 * Check if given TTL is valid 
 *
 * Return 1 if TTL is valid
 * Return 0 otherwise
 */
int is_valid_ttl(struct ip_header *curr_ip_header) 
{

	if(curr_ip_header->ttl == 0) {
			
		printf("    dropping packet from %u.%u.%u.%u to %u.%u.%u.%u (TTL exceeded)\n", curr_ip_header->src_addr[0], 
																					   curr_ip_header->src_addr[1], 
																					   curr_ip_header->src_addr[2], 
																					   curr_ip_header->src_addr[3], 
																					   curr_ip_header->dst_addr[0], 
																					   curr_ip_header->dst_addr[1], 
																					   curr_ip_header->dst_addr[2], 
																					   curr_ip_header->dst_addr[3]);

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
int is_valid_ip_version(struct ip_header *curr_ip_header) 
{
	
	// Get given version (high nibble)
	if (((curr_ip_header->version_and_ihl & 0xf0) >> 4) != 4) {
		
		printf("    dropping packet from %u.%u.%u.%u (unrecognized IP version)\n", curr_ip_header->src_addr[0], 
																				   curr_ip_header->src_addr[1], 
																				   curr_ip_header->src_addr[2], 
																				   curr_ip_header->src_addr[3]);
		return 0;
	
	}

	return 1;

}


/*
 * Check IP destination of curr_ip_header. 
 * Determine whether the ip packet is for one of interfaces.
 *
 * Return pointer to interface it was destined for
 * Return NULL otherwise (meaning IP packet not destined for one of interfaces) 
 */
struct interface *determine_local_interface(struct ip_header *curr_ip_header) 
{
	
	for (int i = 0; i < NUM_INTERFACES; i++) {
		
		// Check if packet is for one of my interfaces 
		if (memcmp(curr_ip_header->dst_addr, interfaces[i].ip_addr, 4) == 0) {
	
			// Can return immediately since IP addresses are unique
			return &interfaces[i]; 
		
		} 
	
	}

	// curr_ip_header not for any of my interfaces
	return NULL;

}


/*
 * Convert uint8_t[4] to a uint32_t for bit comparison purposes.
 * Return the converted value. 
 */
uint32_t array_to_uint32(uint8_t array[4]) 
{
    
	uint32_t result = ((uint32_t)array[0] << 24) 
					| ((uint32_t)array[1] << 16) 
					| ((uint32_t)array[2] << 8) 
					| ((uint32_t)array[3]);
	
	return result;
}

/*
 * Determine the route the curr_ip_header should take. 
 * 
 * Return the pointer to the route the curr_ip_header should follow 
 * Return NULL if no interface has a matching route 
 */
struct route *determine_route(struct ip_header *curr_ip_header) 
{	

	uint32_t given_ip_dst_addr = array_to_uint32(curr_ip_header->dst_addr);

	uint32_t genmask_results = 0;
	int route_entry_num = -1;

	uint32_t curr_genmask;
	uint32_t curr_dst;


	for (int i = 0; i < NUM_ROUTES && routing_table[i].iface; i++) {
				
		curr_genmask = array_to_uint32(routing_table[i].genmask);
		curr_dst = array_to_uint32(routing_table[i].dst);
		
		if ((given_ip_dst_addr & curr_genmask) == curr_dst) {
	
			if (curr_genmask > genmask_results) {
				
				genmask_results = curr_genmask;
				route_entry_num = i;
			
			}

		}

	}
	
	if (route_entry_num == -1) {
		
		return NULL;
	
	} else {
		
		return &routing_table[route_entry_num];
	
	}

}


/*
 * Get the ether address corresponding with the ip_addr
 * by referring to the arp cache
 *
 * Returns pointer to arp_entry if found a successful ip/mac match
 * Returns NULL if it could not find a match
 */
struct arp_entry *determine_mac_arp(uint8_t *ip_addr)
{

	for (int i = 0; i < NUM_ARP_ENTRIES; i++) {
		
		// Found matching IP address
		if (memcmp(ip_addr, arp_cache[i].ip_addr, 4) == 0) {
		
			return &arp_cache[i];

		}
	
	}
	
	return NULL;

}

/*
 */
int send_ip_packet(uint8_t protocol, uint8_t dst_addr[4], uint8_t *payload, size_t payload_len) 
{
	struct ip_header new_ip_header;
	uint8_t ip_packet[MAX_IP_PACKET_SIZE]; 	
	int total_ip_len;
	struct interface src_interface;
	struct route *route_to_take;

	// Construct new IP packet
	memset(&new_ip_header, '\0', sizeof(struct ip_header));
	memcpy(&new_ip_header.version_and_ihl, IP_INITIAL_VERSION_AND_IHL, 1);
	new_ip_header.total_length = htons(sizeof(struct ip_header) + payload_len);
	new_ip_header.ttl = IP_INITIAL_TTL;
	new_ip_header.protocol = protocol;
	memcpy(new_ip_header.dst_addr, dst_addr, 4);

	// Determine the IP source (ie. which interface it will be leaving from) 
	route_to_take = determine_route(&new_ip_header); 
	
	if (route_to_take == NULL) {
		
		printf("    cannot send IP packet (no route)\n");
		return -1;
	
	}

	src_interface = *(route_to_take->iface);
	memcpy(new_ip_header.src_addr, src_interface.ip_addr, 4);
	
	total_ip_len = compose_ip_packet(ip_packet, &new_ip_header, payload, payload_len);

	// Route the newly constructed IP packet
	return route_ip_packet(ip_packet, total_ip_len, 1);

}

/*
 * Send ICMP message using the original_ip_packet (IP header + payload) 
 * for the specified type and code
 *
 * Return -1 if sending ICMP message fails
 */
int send_icmp_message(uint8_t *original_ip_packet, size_t original_ip_packet_len, uint8_t type, uint8_t code)
{
	
	struct icmp_header new_icmp_header; 
	uint8_t icmp_packet[sizeof(struct icmp_header) + ICMP_MAX_DATA_SIZE];
	size_t icmp_packet_len;
	size_t original_bytes_num;
	size_t original_ip_data_len = original_ip_packet_len - sizeof(struct ip_header);
	
	struct ip_header *original_ip_header = (struct ip_header *) original_ip_packet;

	printf("  sending ICMP message\n");

	// Determine how many bytes to read of the original data 
	if (original_ip_data_len < ICMP_IP_ORIGINAL_DATA_SIZE) {
		
		original_bytes_num = original_ip_data_len;
	
	} else {
		
		original_bytes_num = ICMP_IP_ORIGINAL_DATA_SIZE;
	
	}
	
	icmp_packet_len = sizeof(struct icmp_header) + sizeof(struct ip_header) + original_bytes_num;

	// Fill in ICMP header
	memset(&new_icmp_header, '\0', sizeof(struct icmp_header));
	new_icmp_header.type = type;
	new_icmp_header.code = code; 
	new_icmp_header.checksum = 0;
	new_icmp_header.unused = 0;
	
	// Calculate ICMP header checksum
	memcpy(icmp_packet, &new_icmp_header, sizeof(struct icmp_header));
	memcpy(icmp_packet + sizeof(struct icmp_header), original_ip_packet, sizeof(struct ip_header) + original_bytes_num);
	new_icmp_header.checksum = checksum(icmp_packet, icmp_packet_len); // does this need to be converted from hton
	memcpy(icmp_packet, &new_icmp_header, sizeof(struct icmp_header)); // Redundant
	
	return send_ip_packet(IP_ICMP_PROTOCOL, original_ip_header->src_addr, icmp_packet, icmp_packet_len);	

}


/*
 * Handle and check integrity of TCP packet 
 *
 * Return -1 for invalid packets 
 * Return 0 otherwise
 */
int handle_tcp_packet(uint8_t ip_src[4], uint8_t ip_dst[4], uint8_t *packet, int packet_len)
{
	
	struct tcp_header *curr_tcp_header;
	struct tcb *curr_tcb; 
	
	printf("    received TCP packet\n");
	
	curr_tcp_header = (struct tcp_header *) packet;

	// REMEMBER THAT WE ALREADY KNOW THIS PACKET IS FOR US!!! 

	// Verify port number 
	if (curr_tcp_header->dst_port != ntohs(TCP_LISTENING_PORT)) {
		
		printf("      dropping TCP packet (not listening on dst port)\n");
		return -1;

	}

	// See if connection already exists 
	if ((curr_tcb = determine_tcb(ip_src, ip_dst, curr_tcp_header)) == NULL) {
		
		// Add connection if it doesn't exist
		printf("      No existing connection found. Creating new connection.\n");
		curr_tcb = add_tcb(ip_src, ip_dst, curr_tcp_header);
	
	}
	
	// Could not create a new connection
	if (curr_tcb == NULL) {
		
		printf("      Dropping TCP packet?\n");
		return -1;
	
	}
	
	// Verify checksum  
	if (is_valid_tcp_checksum(curr_tcb, packet, packet_len) == 0) {
		
		return -1;	
	
	}
	
	/*
	// Verify sequence and acknowledgement numbers 
	if (is_valid_seq_and_ack(curr_tcb, curr_tcp_header) == 0) {
		
		return -1;
	
	}
	*/

	update_tcp_state(curr_tcb, packet, packet_len);

	// do something with determining the state 
	// in that determining the state we would also need to check the flags to determine what to do 
	
	return 0; 

}


/*
 * Determine whether a connection already exists based on 
 * IP src, IP dst, src port, and dst port of a TCP header
 *
 * Return pointer to tcb if one already exists
 * Return NULL if tcb does not yet exist 
 */
struct tcb *determine_tcb(uint8_t ip_src[4], uint8_t ip_dst[4], struct tcp_header *curr_tcp_header)
{	
	
	for (int i = 0; i < num_connections; i++) { 
		
		// Continue if given IP src, IP dst, src port, or dst port do no match the connection
		
		if (memcmp(ip_src, connections[i].ip_src, 4) != 0) {
			continue;
		}
		if (memcmp(ip_dst, connections[i].ip_dst, 4) != 0) {
			continue;
		}
		if (ntohs(curr_tcp_header->src_port) != connections[i].src_port) {
			continue;
		}
		if (ntohs(curr_tcp_header->dst_port) != connections[i].dst_port) {
			continue; 
		}
		
		// Reached end of conditionals and found a match
		return &connections[i];
	
	}
	
	return NULL;
}


/*
 * Add tcb to connections array 
 *
 * Return pointer to new tcb created 
 * Return NULL if MAX_CONECTIONS have already been reached
 *
 * NOTE: DOUBLE CHECK THE STATE CONNECTION SET TO AND WHETHER TO SET OTHER DEFAULT VALUES
 */
struct tcb *add_tcb(uint8_t ip_src[4], uint8_t ip_dst[4], struct tcp_header *curr_tcp_header)
{
	
	/*
	 * THINGS WRONG WITH THIS 
	 * - Not super space efficient (not overwriting closed connections)
	 * - Can't add past MAX_CONNECTIONS
	 */
	uint32_t random_seq_num; 

	// No more space for connections
	if (num_connections >= MAX_CONNECTIONS) {
		
		printf("        Reached max number of connections.\n");
		return NULL;
	
	}

	// Add new tcb at the end of the array
	memcpy(connections[num_connections].ip_src, ip_src, 4);
	memcpy(connections[num_connections].ip_dst, ip_dst, 4);
	connections[num_connections].src_port = ntohs(curr_tcp_header->src_port);
	connections[num_connections].dst_port = ntohs(curr_tcp_header->dst_port);
	
	// Set random sequence number 
    if(getrandom(&random_seq_num, sizeof(random_seq_num), 0) == -1) {
        perror("getrandom");
    }	

	connections[num_connections].seq_num = random_seq_num; // RANDOMLY GENERATE IT HERE? 
	connections[num_connections].ack_num = 0;
	connections[num_connections].state = LISTEN; // DOUBLE CHECK THIS???
	
	num_connections += 1;

	return &connections[num_connections - 1];

	// How should we determine where this new connection will go? 
	// Or should we check the connection state and see if any are closed? 
	// But does that mean that we have to initialized all the structs in the array to have a closed connection? 
	// Meaning that we would have to do that at the beginning of the code? 

}

/*
 * Calculate and return the tcp checksum of a tcp packet in network byte order 
 *
 * Return calculated checksum in network byte order
 */
uint16_t calculate_tcp_checksum(struct tcb *curr_tcb, uint8_t *curr_tcp_packet, int tcp_length)
{
	
	struct tcp_pseudo_header pseudo_header;
	struct tcp_header *curr_tcp_header;
	uint16_t calculated_checksum;
	int total_length = sizeof(struct tcp_pseudo_header) + tcp_length;
	// Edit total_length if there needs to be padding
	total_length = (total_length % 2) ? total_length : total_length + 1; 
	uint8_t tcp_text[total_length];

	memset(tcp_text, '\0', total_length);

	// Create a psuedo header for TCP packet
	memset(&pseudo_header, '\0', sizeof(struct tcp_pseudo_header));
	memcpy(pseudo_header.ip_src, curr_tcb->ip_src, 4);
	memcpy(pseudo_header.ip_dst, curr_tcb->ip_dst, 4);
	pseudo_header.zeros = 0;
	pseudo_header.ptcl = IP_TCP_PROTOCOL;
	//printf("      TCP LENGTH (checksum): %d (%x)\n", tcp_length, tcp_length);
	pseudo_header.tcp_length = htons(tcp_length); 
	// NOTE: We switch to network endianess bc tcp header will be in network endianess 
	
	// Interpret beginning of packet as TCP header 
	curr_tcp_header = (struct tcp_header *) curr_tcp_packet;

	// Reset the checksum 
	curr_tcp_header->checksum = 0;

	// Write psuedo header and TCP packet into the tcp_text buffer
	memcpy(tcp_text, &pseudo_header, sizeof(struct tcp_pseudo_header));
	memcpy(tcp_text + sizeof(struct tcp_pseudo_header), curr_tcp_packet, tcp_length);

	// Calculate the checksum using the data in tcp_text buffer 
	calculated_checksum = checksum(tcp_text, total_length);
	
	return calculated_checksum;

}

/*
 * Verify whether TCP checksum is correct 
 *
 * Return 1 if TCP checksum is correct 
 * Return 0 otherwise 
 *
 * NOTE: Double check padding 
 */
int is_valid_tcp_checksum(struct tcb *curr_tcb, uint8_t *curr_tcp_packet, int tcp_length)
{
	
	struct tcp_header *curr_tcp_header;
	uint16_t original_checksum;
	uint16_t calculated_checksum;
	int total_length = sizeof(struct tcp_pseudo_header) + tcp_length;
	// Edit total_length if there needs to be padding
	total_length = (total_length % 2) ? total_length : total_length + 1; 
		
	// Interpret beginning of packet as TCP header 
	curr_tcp_header = (struct tcp_header *) curr_tcp_packet;

	// Save original checksum
	original_checksum = curr_tcp_header->checksum;

	// Calculate the checksum using the data in tcp_text buffer 
	calculated_checksum = calculate_tcp_checksum(curr_tcb, curr_tcp_packet, tcp_length);

	//printf("ORIGINAL CHECKSUM:   %x\n", original_checksum);
	//printf("CALCULATED CHECKSUM: %x\n", calculated_checksum);

	if (calculated_checksum != original_checksum) {
		printf("        dropping TCP packet (bad checksum)\n");
		return 0;
	}

	return 1;

}

/*
 * Set the flags in given tcp_flags to reflect which flags were given in a TCP header
 *
 * Return void
 */
void set_tcp_flags(struct tcp_flags *flags, struct tcp_header *curr_tcp_header)
{
	// NOTE: Have else statements to ensure that previous flags do not roll over

	if (ntohs(curr_tcp_header->offset_reserved_control) & TCP_URG_FLAG) {
		flags->URG = 1;
	} else {
		flags->URG = 0;
	}

	if (ntohs(curr_tcp_header->offset_reserved_control) & TCP_ACK_FLAG) {
		flags->ACK = 1;
	} else {
		flags->ACK = 0;
	}

	if (ntohs(curr_tcp_header->offset_reserved_control) & TCP_PSH_FLAG) {
		flags->PSH = 1;
	} else {
		flags->PSH = 0;
	}
	
	if (ntohs(curr_tcp_header->offset_reserved_control) & TCP_RST_FLAG) {
		flags->RST = 1;
	} else {
		flags->RST = 0;
	}
	
	if (ntohs(curr_tcp_header->offset_reserved_control) & TCP_SYN_FLAG) {
		flags->SYN = 1;
	} else {
		flags->SYN = 0;
	}
	
	if (ntohs(curr_tcp_header->offset_reserved_control) & TCP_FIN_FLAG) {
		flags->FIN = 1;
	} else {
		flags->FIN = 0;
	}
	
}

/*
 * Verify whether the given sequence and acknowledgement numbers are the expected ones 
 *
 * Return 1 if they are the expected numbers or if received a SYN packet  
 * Return 0 otherwise 
 *
 * NOTE: The seq and ack numbers in the tcb should be the ones it expects next
 * CHECK THIS!!!! 
 */
int is_valid_seq_and_ack(struct tcb *curr_tcb, struct tcp_header *curr_tcp_header) 
{

	// If connection has not been established yet, do not verify seq or ack nums
	if ((curr_tcb->ack_num == 0)) {
		
		printf("      *NEW CONNECTION (no seq or ack checks)*\n");

	} else {
		// UNSURE OF THIS CHECK!!!!!
		if (curr_tcb->seq_num != ntohl(curr_tcp_header->seq_num)) {
		
			printf("ignoring TCP packet (unexpected sequence number)\n");
			return 0;
		
		}

		if (curr_tcb->ack_num != ntohl(curr_tcp_header->ack_num)) {
			
			printf("ignoring TCP packet (unexpected acknowledgement number)\n");
			return 0;
		
		}
	
	}

	return 1;

}


/*
 */
void update_tcp_state(struct tcb *curr_tcb, uint8_t *curr_tcp_packet, int packet_len) 
{
	
	char *print = "      current state: "; 
	struct tcp_header *curr_tcp_header = (struct tcp_header *)curr_tcp_packet;
	struct tcp_flags given_tcp_flags; 

	set_tcp_flags(&given_tcp_flags, curr_tcp_header);

	// MINDSET 
	// Check the current state 
	// look at if you received what diagram said 
	// do the action diagram says 
	// update state of the tcb
	switch (curr_tcb->state) {
		
		case LISTEN: {
			
			printf("%s LISTEN\n", print); 
			
			if (given_tcp_flags.SYN) {
				
				printf("      received SYN. sending SYN ACK.\n");
				send_tcp_packet(curr_tcb, TCP_SYN_FLAG | TCP_ACK_FLAG, curr_tcp_packet, packet_len, NULL, 0);
				curr_tcb->state = SYN_RECEIVED;

			}

		} break;
		
		case SYN_SENT: {
			printf("%s SYN_SENT\n", print);
		} break;

		case SYN_RECEIVED: {

			printf("%s SYN_RECEIVED\n", print);
			
			if (given_tcp_flags.ACK) {
			
				printf("      received ACK to my SYN. Moving to ESTABLISHED.\n");
				// send_tcp_packet(curr_tcb, TCP_ACK_FLAG, curr_tcp_packet, packet_len, NULL, 0);
				curr_tcb->state = ESTABLISHED;

			}

		} break;

		case ESTABLISHED: {
			
			printf("%s ESTABLISHED\n", print);
			
			// print out data 
			print_tcp_data(curr_tcp_packet, packet_len);
			
			send_tcp_packet(curr_tcb, TCP_ACK_FLAG, curr_tcp_packet, packet_len, NULL, 0);

			
		} break;

		case FIN_WAIT_1: {
			printf("%s FIN_WAIT_1\n", print);
		} break;

		case FIN_WAIT_2: {
			printf("%s FIN_WAIT_2\n", print);
		} break;

		case CLOSE_WAIT: {
			printf("%s CLOSE_WAIT\n", print);
		} break;

		case CLOSING: {
			printf("%s CLOSING\n", print);
		} break;

		case LAST_ACK: {
			printf("%s LAST_ACK\n", print);
		} break;
		
		case TIME_WAIT: {
			printf("%s TIME_WAIT\n", print);
		} break;

		case CLOSED: {
			printf("%s CLOSED\n", print);
		} break;

		// UNSURE ABOUT THIS DEFAULT CASE 
		default: 
			break;

	}

}

/*
 */
int send_tcp_packet(struct tcb *curr_tcb, uint8_t flags, uint8_t *original_tcp_packet, int original_packet_len, uint8_t *payload, size_t payload_len) 
{
	struct tcp_header new_tcp_header; 
	uint8_t tcp_packet[sizeof(struct tcp_header) + payload_len]; // ASSUMPTION: NO OPTIONS

	uint16_t new_offset_reserved_control;

	struct tcp_header *original_tcp_header = (struct tcp_header *) original_tcp_packet;
	uint16_t original_tcp_offset = ntohs(original_tcp_header->offset_reserved_control) >> 12; // DOUBLE CHECK!!!  
	//printf("      original tcp offset: %u\n", original_tcp_offset);
	uint32_t original_payload_len = original_packet_len - (original_tcp_offset * 4);
	//printf("      original payload len: %u\n", original_payload_len);

	/*
	 * PROBLEMS: 
	 * - What if you're not responding to a packet and you're just sending one out 
	 */

	// Set TCP ports
	new_tcp_header.src_port = original_tcp_header->dst_port;
	new_tcp_header.dst_port = original_tcp_header->src_port; 

	// Set sequence number
	if (flags & TCP_SYN_FLAG) {
		new_tcp_header.seq_num = htonl(curr_tcb->seq_num);
		original_payload_len += 1;
	} else {
		new_tcp_header.seq_num = original_tcp_header->ack_num;
	}

	// Set acknowledgement number
	new_tcp_header.ack_num = htonl(ntohl(original_tcp_header->seq_num) + original_payload_len); 

	// Set offset, reserved, and control (flags) 
	new_offset_reserved_control = TCP_DEFAULT_OFFSET << 12; 
	new_offset_reserved_control |= flags; 
	new_tcp_header.offset_reserved_control = htons(new_offset_reserved_control); 

	// Set the window 
	new_tcp_header.window = original_tcp_header->window; 

	// Set the urgent pointer 
	new_tcp_header.urgent_ptr = 0;

	// Copy the new header and given payload into tcp packet 
	memcpy(tcp_packet, &new_tcp_header, sizeof(struct tcp_header));

	if (payload_len > 0) {
		memcpy(tcp_packet + sizeof(struct tcp_header), payload, payload_len);
	}

	// Set the checksum 
	new_tcp_header.checksum = calculate_tcp_checksum(curr_tcb, tcp_packet, sizeof(tcp_packet)); // ASSUMPTION: NO OPTIONS
	memcpy(tcp_packet, &new_tcp_header, sizeof(struct tcp_header));

	// NEED TO UPDATE THE TCB
	// CONTAINS INFORMATION ABOUT US!!!! 
	curr_tcb->seq_num = ntohl(new_tcp_header.seq_num) + payload_len; // DOUBLE CHECK!!!!  
	curr_tcb->ack_num = ntohl(new_tcp_header.ack_num);
	
	printf("  sending TCP segment\n");
	return send_ip_packet(IP_TCP_PROTOCOL, curr_tcb->ip_src, tcp_packet, sizeof(tcp_packet));

}


/*
 */
void print_tcp_data(uint8_t *original_tcp_packet, int original_packet_len) 
{
	struct tcp_header *original_tcp_header = (struct tcp_header *) original_tcp_packet;
	uint16_t original_tcp_offset = ntohs(original_tcp_header->offset_reserved_control) >> 12;
	char tcp_data[original_packet_len - (original_tcp_offset * 4) + 1]; 

	// Entire TCP packet is just the header
	if (original_tcp_offset * 4 == original_packet_len) {
		
		printf("      No data in TCP packet.\n");

	// TCP packet has data 
	} else {
		
		printf("      TCP DATA: \n");
		snprintf(tcp_data, sizeof(tcp_data), "%s", original_tcp_packet + sizeof(struct tcp_header));
		printf("        %s", tcp_data);
	}

}


